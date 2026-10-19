/**
 * ============================================================================
 * Assignment 7: NS-3 Point-to-Point UDP Latency vs Throughput Simulation
 * ============================================================================
 * Description: Simulates a 2-node point-to-point wired topology (5 Mbps, 2ms delay).
 *              Generates UDP traffic, installs FlowMonitor, and measures:
 *              - Throughput (Mbps)
 *              - Packet Delivery Ratio (PDR %)
 *              - Average Latency / Delay (ms)
 *
 * HOW TO SET UP & COMPILE IN NS-3:
 *   1. Copy this file into your NS-3 scratch directory:
 *      cp ass7/ns3/p2p_latency.cc ~/ns-allinone-3.xx/ns-3.xx/scratch/
 *   2. Navigate to your NS-3 root directory:
 *      cd ~/ns-allinone-3.xx/ns-3.xx
 *   3. Build & Run through the NS-3 runner:
 *      ./ns3 run scratch/p2p_latency
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

    // 1. Create 2 Nodes (Sender n0, Receiver n1)
    NodeContainer nodes;
    nodes.Create(2);

    // 2. Point-to-Point Channel (5 Mbps DataRate, 2ms Channel Delay)
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer devices = p2p.Install(nodes);

    // 3. Install TCP/IP Stack & IP Assignment
    InternetStackHelper stack;
    stack.Install(nodes);
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // 4. Install UDP Server on Node 1 (port 9)
    uint16_t port = 9;
    UdpServerHelper server(port);
    ApplicationContainer serverApp = server.Install(nodes.Get(1));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // 5. Install UDP Client on Node 0 (Sends 1024-byte packets)
    UdpClientHelper client(interfaces.GetAddress(1), port);
    client.SetAttribute("MaxPackets", UintegerValue(1000));
    client.SetAttribute("Interval", TimeValue(MilliSeconds(10)));
    client.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApp = client.Install(nodes.Get(0));
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));

    // 6. FlowMonitor for Latency, Throughput & PDR
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    // 7. Simulation Run & Automatic Stop
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    monitor->CheckForLostPackets();
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    for (auto const& [id, s] : stats) {
        double throughput = s.rxBytes * 8.0 / (8.0 * 1e6); // Mbps over active 8s
        double pdr = (s.txPackets > 0) ? ((double)s.rxPackets / s.txPackets * 100.0) : 0.0;
        double avg_delay = (s.rxPackets > 0) ? (s.delaySum.GetSeconds() / s.rxPackets * 1000.0) : 0.0;

        std::cout << "--- Flow " << id << " Metrics ---" << std::endl;
        std::cout << "Tx Packets : " << s.txPackets << " | Rx Packets: " << s.rxPackets << std::endl;
        std::cout << "Throughput : " << throughput << " Mbps" << std::endl;
        std::cout << "Packet Delivery Ratio (PDR) : " << pdr << " %" << std::endl;
        std::cout << "Average Latency / Delay      : " << avg_delay << " ms" << std::endl;
    }

    Simulator::Destroy();
    return 0;
}
