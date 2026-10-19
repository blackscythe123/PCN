/**
 * ============================================================================
 * Assignment 10: NS-3 Dynamic Routing Protocols & Link Failure Simulation
 * ============================================================================
 * Description: Simulates diamond topology (4 nodes) with dynamic routing.
 *              Injects link failure between n1-n3 at t=5.0s, triggers routing
 *              table recomputation, and verifies traffic rerouting via n2-n3.
 *
 * HOW TO SET UP & COMPILE IN NS-3:
 *   1. Copy this file into your NS-3 scratch directory:
 *      cp ass10/ns3/routing_protocols.cc ~/ns-allinone-3.xx/ns-3.xx/scratch/
 *   2. Navigate to your NS-3 root directory:
 *      cd ~/ns-allinone-3.xx/ns-3.xx
 *   3. Build & Run:
 *      ./ns3 run scratch/routing_protocols
 *
 * HOW TO STOP:
 *   - Automatic Stop: Simulator stops automatically at t = 15.0 seconds (Simulator::Stop(Seconds(15.0))).
 *   - Abort simulation early: Press [Ctrl + C] in the terminal running ./ns3.
 * ============================================================================
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // 4 Nodes forming a diamond topology:
    //      n1
    //    /    \
    //  n0      n3
    //    \    /
    //      n2
    NodeContainer c;
    c.Create(4);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer d01 = p2p.Install(c.Get(0), c.Get(1));
    NetDeviceContainer d13 = p2p.Install(c.Get(1), c.Get(3));
    NetDeviceContainer d02 = p2p.Install(c.Get(0), c.Get(2));
    NetDeviceContainer d23 = p2p.Install(c.Get(2), c.Get(3));

    InternetStackHelper stack;
    stack.Install(c);

    Ipv4AddressHelper ip;
    ip.SetBase("10.1.1.0", "255.255.255.0"); ip.Assign(d01);
    ip.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer if13 = ip.Assign(d13);
    ip.SetBase("10.1.3.0", "255.255.255.0"); ip.Assign(d02);
    ip.SetBase("10.1.4.0", "255.255.255.0"); ip.Assign(d23);

    // Enable Global Dynamic Routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Receiver on n3
    uint16_t port = 9;
    UdpServerHelper server(port);
    ApplicationContainer serverApp = server.Install(c.Get(3));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(15.0));

    // Sender on n0
    UdpClientHelper client(if13.GetAddress(1), port);
    client.SetAttribute("MaxPackets", UintegerValue(1500));
    client.SetAttribute("Interval", TimeValue(MilliSeconds(10)));
    client.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApp = client.Install(c.Get(0));
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(15.0));

    // Simulate Link Failure between n1 and n3 at t=5.0s
    Simulator::Schedule(Seconds(5.0), &PointToPointNetDevice::SetReceiveEnable,
                        d13.Get(0)->GetObject<PointToPointNetDevice>(), false);
    // Recompute routing tables dynamically
    Simulator::Schedule(Seconds(5.1), &Ipv4GlobalRoutingHelper::RecomputeRoutingTables);

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Stop(Seconds(15.0));
    Simulator::Run();

    monitor->CheckForLostPackets();
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    for (auto const& [id, s] : stats) {
        double throughput = s.rxBytes * 8.0 / (13.0 * 1e6); // Mbps over active 13s
        double pdr = (s.txPackets > 0) ? ((double)s.rxPackets / s.txPackets * 100.0) : 0.0;
        std::cout << "Flow " << id << " | Tx: " << s.txPackets << " | Rx: " << s.rxPackets
                  << " | Lost: " << s.lostPackets << " | Throughput: " << throughput << " Mbps"
                  << " | PDR: " << pdr << " %" << std::endl;
    }

    Simulator::Destroy();
    return 0;
}
