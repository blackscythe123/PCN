// Assignment 9 - TCP Congestion Control Algorithms (Reno / Cubic / BBR) in NS-3
// Scenario: VANET - 5 vehicles send data through a Roadside Router across the
// "Internet" (bottleneck link) to a Traffic Control Server.
//
//   Vehicle0 --|
//   Vehicle1 --|
//   Vehicle2 --|---- Roadside Router ====Internet(bottleneck)==== Server
//   Vehicle3 --|
//   Vehicle4 --|
//
// Each vehicle runs one saturating TCP (BulkSend) flow to the server. The TCP
// congestion-control algorithm used by every flow is picked with --tcpVariant
// (Reno/Cubic/Bbr) so the SAME program produces all three runs being compared.
// FlowMonitor gives per-flow throughput/delay/loss; a small trace hooks each
// flow's congestion window over time; Jain's fairness index is computed from
// the five flows' throughputs.

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpVariantsVanet");

static std::ofstream cwndFile;

static void
CwndTrace(uint32_t oldCwnd, uint32_t newCwnd)
{
    cwndFile << Simulator::Now().GetSeconds() << " " << newCwnd << std::endl;
}

static void
ConnectCwndTraces(uint32_t numFlows)
{
    for (uint32_t i = 0; i < numFlows; i++) {
        Config::ConnectWithoutContextFailSafe(
            "/NodeList/" + std::to_string(i) + "/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
            MakeCallback(&CwndTrace));
    }
}

int
main(int argc, char *argv[])
{
    std::string tcpVariant = "Reno"; // Reno | Cubic | Bbr
    uint32_t numVehicles = 5;
    double simTime = 20.0;

    CommandLine cmd;
    cmd.AddValue("tcpVariant", "TCP congestion control: Reno, Cubic or Bbr", tcpVariant);
    cmd.AddValue("numVehicles", "Number of vehicle (TCP source) nodes", numVehicles);
    cmd.AddValue("simTime", "Simulation duration (s)", simTime);
    cmd.Parse(argc, argv);

    std::string tid = "ns3::TcpNewReno";
    if (tcpVariant == "Cubic") tid = "ns3::TcpCubic";
    else if (tcpVariant == "Bbr") tid = "ns3::TcpBbr";
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TypeId::LookupByName(tid)));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1024));
    Config::SetDefault("ns3::TcpSocketState::EnablePacing", BooleanValue(true)); // needed for correct TcpBbr behaviour

    cwndFile.open("cwnd-" + tcpVariant + ".dat");

    // Nodes: 0..numVehicles-1 = vehicles, numVehicles = roadside router, numVehicles+1 = server
    NodeContainer vehicles;
    vehicles.Create(numVehicles);
    NodeContainer routerServer;
    routerServer.Create(2);
    Ptr<Node> router = routerServer.Get(0);
    Ptr<Node> server = routerServer.Get(1);

    InternetStackHelper internet;
    internet.InstallAll();

    // Access links: vehicle <-> router (wireless-uplink-like, 5Mbps/5ms)
    PointToPointHelper access;
    access.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    access.SetChannelAttribute("Delay", StringValue("5ms"));

    // Bottleneck link: router <-> server, representing the shared Internet path
    PointToPointHelper bottleneck;
    bottleneck.SetDeviceAttribute("DataRate", StringValue("8Mbps"));
    bottleneck.SetChannelAttribute("Delay", StringValue("20ms"));

    Ipv4AddressHelper addr;
    Ipv4InterfaceContainer serverIf;
    for (uint32_t i = 0; i < numVehicles; i++) {
        NetDeviceContainer link = access.Install(vehicles.Get(i), router);
        addr.SetBase(("10.1." + std::to_string(i) + ".0").c_str(), "255.255.255.0");
        addr.Assign(link);
    }
    NetDeviceContainer rsLink = bottleneck.Install(router, server);
    addr.SetBase("10.2.0.0", "255.255.255.0");
    serverIf = addr.Assign(rsLink);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // One TCP sink on the server per vehicle flow (distinct ports)
    uint16_t basePort = 5000;
    ApplicationContainer sinkApps, sourceApps;
    for (uint32_t i = 0; i < numVehicles; i++) {
        uint16_t port = basePort + i;
        PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
        ApplicationContainer s = sink.Install(server);
        s.Start(Seconds(0.0));
        s.Stop(Seconds(simTime));
        sinkApps.Add(s);

        BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(serverIf.GetAddress(1), port));
        source.SetAttribute("MaxBytes", UintegerValue(0));
        ApplicationContainer src = source.Install(vehicles.Get(i));
        src.Start(Seconds(1.0 + 0.1 * i)); // slight stagger so flows don't start bit-identical
        src.Stop(Seconds(simTime));
        sourceApps.Add(src);
    }

    Simulator::Schedule(Seconds(1.2), &ConnectCwndTraces, numVehicles);

    FlowMonitorHelper flowmonHelper;
    Ptr<FlowMonitor> monitor = flowmonHelper.InstallAll();

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    double activeTime = simTime - 1.0;

    std::cout << "===== TCP variant: " << tcpVariant << " =====" << std::endl;
    double sumThr = 0.0, sumThrSq = 0.0;
    uint32_t n = 0;
    for (auto &kv : stats) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(kv.first);
        FlowMonitor::FlowStats s = kv.second;
        if (t.destinationPort < basePort || t.destinationPort >= basePort + numVehicles) continue; // skip ACK-direction flows
        double throughputMbps = (s.rxBytes * 8.0) / activeTime / 1e6;
        double avgDelayMs = (s.rxPackets > 0) ? (s.delaySum.GetSeconds() / s.rxPackets) * 1000 : 0;
        double lossPct = (s.txPackets > 0) ? (double)s.lostPackets / s.txPackets * 100 : 0;
        std::cout << "Flow " << t.sourceAddress << " -> port " << t.destinationPort
                   << " | Throughput=" << throughputMbps << "Mbps"
                   << " | AvgDelay=" << avgDelayMs << "ms"
                   << " | Loss=" << lossPct << "%" << std::endl;
        sumThr += throughputMbps;
        sumThrSq += throughputMbps * throughputMbps;
        n++;
    }
    double fairness = (n > 0 && sumThrSq > 0) ? (sumThr * sumThr) / (n * sumThrSq) : 0.0;
    std::cout << "Jain's Fairness Index (" << n << " flows) = " << fairness << std::endl;

    cwndFile.close();
    Simulator::Destroy();
    return 0;
}
