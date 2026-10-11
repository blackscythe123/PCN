// Assignment 7 - Experiment 2: UDP Traffic and Port-Based Application Delivery
// Topology: Node1 ---- P2P link ---- Node2
// Two independent UDP flows (port 5000 and port 6000) share the same pair of
// IP addresses; only the destination port tells the receiver which
// application a packet belongs to. Link latency and the number of active
// flows are passed on the command line so the same binary can reproduce
// every row of both required result tables.

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("UdpPortDelivery");

int main(int argc, char *argv[])
{
    std::string delay = "2ms";   // link propagation delay (varied per run)
    bool twoFlows = false;       // false = only Flow 1 (Sec.3), true = both (Sec.5/7)
    double simTime = 5.0;        // seconds each flow is active

    CommandLine cmd;
    cmd.AddValue("delay", "Point-to-point propagation delay", delay);
    cmd.AddValue("twoFlows", "Enable the second UDP flow (port 6000)", twoFlows);
    cmd.Parse(argc, argv);

    // 1. Two nodes, one point-to-point link
    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue(delay));
    NetDeviceContainer devices = p2p.Install(nodes);

    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);
    Ipv4Address node2Addr = interfaces.GetAddress(1);

    // 2 & 3. Flow 1 (Application 1): UdpClient on Node1 -> UdpServer on Node2, port 5000
    uint16_t port1 = 5000;
    UdpServerHelper server1(port1);
    ApplicationContainer serverApp1 = server1.Install(nodes.Get(1));
    serverApp1.Start(Seconds(0.0));
    serverApp1.Stop(Seconds(simTime + 1.0));

    UdpClientHelper client1(node2Addr, port1);
    client1.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF));
    client1.SetAttribute("PacketSize", UintegerValue(512));
    client1.SetAttribute("Interval", TimeValue(MicroSeconds(4096))); // 512B/4.096ms = 1 Mbps
    ApplicationContainer clientApp1 = client1.Install(nodes.Get(0));
    clientApp1.Start(Seconds(1.0));
    clientApp1.Stop(Seconds(simTime + 1.0));

    // 4. Flow 2 (Application 2): second UdpClient/UdpServer pair on port 6000
    uint16_t port2 = 6000;
    ApplicationContainer serverApp2, clientApp2;
    if (twoFlows) {
        UdpServerHelper server2(port2);
        serverApp2 = server2.Install(nodes.Get(1));
        serverApp2.Start(Seconds(0.0));
        serverApp2.Stop(Seconds(simTime + 1.0));

        UdpClientHelper client2(node2Addr, port2);
        client2.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF));
        client2.SetAttribute("PacketSize", UintegerValue(512));
        client2.SetAttribute("Interval", TimeValue(MicroSeconds(4096)));
        clientApp2 = client2.Install(nodes.Get(0));
        clientApp2.Start(Seconds(1.0));
        clientApp2.Stop(Seconds(simTime + 1.0));
    }

    // 6. PCAP capture on both interfaces (view with Wireshark)
    p2p.EnablePcapAll("udp-port-based");

    Simulator::Stop(Seconds(simTime + 1.0));
    Simulator::Run();

    Ptr<UdpServer> srv1 = DynamicCast<UdpServer>(serverApp1.Get(0));
    double thr1 = (srv1->GetReceived() * 512.0 * 8.0) / simTime / 1e6;
    std::cout << "delay=" << delay << " Flow1(port5000) received=" << srv1->GetReceived()
              << " throughput=" << thr1 << "Mbps";

    if (twoFlows) {
        Ptr<UdpServer> srv2 = DynamicCast<UdpServer>(serverApp2.Get(0));
        double thr2 = (srv2->GetReceived() * 512.0 * 8.0) / simTime / 1e6;
        std::cout << " Flow2(port6000) received=" << srv2->GetReceived()
                  << " throughput=" << thr2 << "Mbps";
    }
    std::cout << std::endl;

    Simulator::Destroy();
    return 0;
}
