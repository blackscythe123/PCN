/**
 * ============================================================================
 * Assignment 8: NS-3 Dumbbell Bottleneck - TCP vs UDP Traffic Competition
 * ============================================================================
 * Description: Implements a 6-node dumbbell topology where competing TCP BulkSend
 *              and UDP OnOff CBR flows share a single constrained bottleneck link
 *              (1 Mbps, 20ms delay, DropTailQueue limit of 10 packets).
 *
 * HOW TO SET UP & COMPILE IN NS-3:
 *   1. Copy this file into your NS-3 scratch directory:
 *      cp ass8/ns3/tcp_udp_bottleneck.cc ~/ns-allinone-3.xx/ns-3.xx/scratch/
 *   2. Navigate to your NS-3 root directory:
 *      cd ~/ns-allinone-3.xx/ns-3.xx
 *   3. Build & Run:
 *      ./ns3 run scratch/tcp_udp_bottleneck
 *
 * HOW TO STOP:
 *   - Automatic Stop: Simulator stops automatically at t = 10.0 seconds (Simulator::Stop(Seconds(10.0))).
 *   - Abort simulation early: Press [Ctrl + C] in the terminal running ./ns3.
 * ============================================================================
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Nodes: 0:TCP_Src, 1:UDP_Src, 2:Router1, 3:Router2, 4:TCP_Sink, 5:UDP_Sink
    NodeContainer c;
    c.Create(6);

    InternetStackHelper stack;
    stack.Install(c);

    // Access Links (10 Mbps, 2ms)
    PointToPointHelper access;
    access.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    access.SetChannelAttribute("Delay", StringValue("2ms"));

    // Bottleneck Link between R1(2) and R2(3) (1 Mbps, 20ms, DropTail Queue = 10 packets)
    PointToPointHelper bottleneck;
    bottleneck.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    bottleneck.SetChannelAttribute("Delay", StringValue("20ms"));
    bottleneck.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", StringValue("10p"));

    NetDeviceContainer d02 = access.Install(c.Get(0), c.Get(2));
    NetDeviceContainer d12 = access.Install(c.Get(1), c.Get(2));
    NetDeviceContainer d23 = bottleneck.Install(c.Get(2), c.Get(3));
    NetDeviceContainer d34 = access.Install(c.Get(3), c.Get(4));
    NetDeviceContainer d35 = access.Install(c.Get(3), c.Get(5));

    Ipv4AddressHelper ip;
    ip.SetBase("10.1.1.0", "255.255.255.0"); ip.Assign(d02);
    ip.SetBase("10.1.2.0", "255.255.255.0"); ip.Assign(d12);
    ip.SetBase("10.1.3.0", "255.255.255.0"); ip.Assign(d23);
    ip.SetBase("10.1.4.0", "255.255.255.0"); ip.Assign(d34);
    ip.SetBase("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer if35 = ip.Assign(d35);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // 1. TCP Application (BulkSend from node 0 to node 4)
    uint16_t tcpPort = 8080;
    PacketSinkHelper tcpSink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), tcpPort));
    ApplicationContainer sinkApp = tcpSink.Install(c.Get(4));
    sinkApp.Start(Seconds(0.5));
    sinkApp.Stop(Seconds(10.0));

    BulkSendHelper tcpSource("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address("10.1.4.2"), tcpPort));
    ApplicationContainer srcApp = tcpSource.Install(c.Get(0));
    srcApp.Start(Seconds(1.0));
    srcApp.Stop(Seconds(10.0));

    // 2. UDP Application (OnOff CBR from node 1 to node 5)
    uint16_t udpPort = 9090;
    PacketSinkHelper udpSink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), udpPort));
    udpSink.Install(c.Get(5)).Start(Seconds(0.5));

    OnOffHelper udpSource("ns3::UdpSocketFactory", InetSocketAddress(if35.GetAddress(1), udpPort));
    udpSource.SetAttribute("DataRate", StringValue("800Kbps"));
    udpSource.SetAttribute("PacketSize", UintegerValue(512));
    ApplicationContainer udpApp = udpSource.Install(c.Get(1));
    udpApp.Start(Seconds(2.0));
    udpApp.Stop(Seconds(10.0));

    // FlowMonitor
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    monitor->CheckForLostPackets();
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    for (auto const& [id, s] : stats) {
        double throughput = s.rxBytes * 8.0 / (9.0 * 1e6); // Mbps over active 9s
        double pdr = (s.txPackets > 0) ? ((double)s.rxPackets / s.txPackets * 100.0) : 0.0;
        std::cout << "Flow " << id << " | Tx: " << s.txPackets << " | Rx: " << s.rxPackets
                  << " | Lost: " << s.lostPackets << " | Throughput: " << throughput << " Mbps"
                  << " | PDR: " << pdr << " %" << std::endl;
    }

    Simulator::Destroy();
    return 0;
}
