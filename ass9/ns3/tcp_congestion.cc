/**
 * ============================================================================
 * Assignment 9: NS-3 TCP Congestion Control Comparison (Reno / Cubic / BBR)
 * ============================================================================
 * Description: Compares TCP variants (TcpNewReno, TcpCubic, TcpBbr) across a
 *              10 Mbps bottleneck link, tracking throughput and queue delay.
 *
 * HOW TO SET UP & COMPILE IN NS-3:
 *   1. Copy this file into your NS-3 scratch directory:
 *      cp ass9/ns3/tcp_congestion.cc ~/ns-allinone-3.xx/ns-3.xx/scratch/
 *   2. Navigate to your NS-3 root directory:
 *      cd ~/ns-allinone-3.xx/ns-3.xx
 *   3. Build & Run with different TCP variants:
 *      ./ns3 run "scratch/tcp_congestion --tcpVariant=TcpCubic"
 *      ./ns3 run "scratch/tcp_congestion --tcpVariant=TcpNewReno"
 *      ./ns3 run "scratch/tcp_congestion --tcpVariant=TcpBbr"
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
    std::string tcpVariant = "TcpCubic";
    CommandLine cmd;
    cmd.AddValue("tcpVariant", "TCP Congestion Control (TcpNewReno, TcpCubic, TcpBbr)", tcpVariant);
    cmd.Parse(argc, argv);

    // Set Congestion Control Algorithm globally
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::" + tcpVariant));

    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("10ms"));
    NetDeviceContainer devices = p2p.Install(nodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    uint16_t sinkPort = 8080;
    PacketSinkHelper packetSink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
    ApplicationContainer sinkApps = packetSink.Install(nodes.Get(1));
    sinkApps.Start(Seconds(0.0));
    sinkApps.Stop(Seconds(10.0));

    BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(interfaces.GetAddress(1), sinkPort));
    source.SetAttribute("MaxBytes", UintegerValue(0)); // Unlimited bulk transfer
    ApplicationContainer sourceApps = source.Install(nodes.Get(0));
    sourceApps.Start(Seconds(1.0));
    sourceApps.Stop(Seconds(10.0));

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    monitor->CheckForLostPackets();
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    for (auto const& [id, s] : stats) {
        double throughput = s.rxBytes * 8.0 / (9.0 * 1e6); // Mbps over active 9s
        double avgDelay = (s.rxPackets > 0) ? (s.delaySum.GetSeconds() / s.rxPackets * 1000.0) : 0.0;
        std::cout << "=== Algorithm: " << tcpVariant << " ===" << std::endl;
        std::cout << "Throughput : " << throughput << " Mbps" << std::endl;
        std::cout << "Avg Delay  : " << avgDelay << " ms" << std::endl;
        std::cout << "Packets Rx : " << s.rxPackets << " | Tx: " << s.txPackets << std::endl;
    }

    Simulator::Destroy();
    return 0;
}
