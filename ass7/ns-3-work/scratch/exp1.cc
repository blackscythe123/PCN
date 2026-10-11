// Assignment 7 - Experiment 1: Simulation of a Simple Network using NS-3
// Topology:  n0 --- n1 --- n2
//                    |
//                    n3
// Flow 1: n0 -> n2 (UDP, 1 Mbps)   Flow 2: n3 -> n0 (UDP, 500 Kbps)
// Reports: total bytes received, throughput per flow, average latency per
// flow (computed with plain Tx/Rx trace callbacks -- no FlowMonitor used).

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <queue>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleNetwork");

// FIFO queues of send timestamps; since each flow travels a single ordered
// point-to-point path with no other traffic sharing its queue slot order,
// packets arrive in the order they were sent, so matching front-of-queue
// Tx time with each Rx event gives the correct per-packet delay.
static std::queue<Time> txQ1, txQ2;
static double totalDelay1 = 0, totalDelay2 = 0;
static uint32_t rxCount1 = 0, rxCount2 = 0;

static void TxTrace1(Ptr<const Packet> p) { txQ1.push(Simulator::Now()); }
static void TxTrace2(Ptr<const Packet> p) { txQ2.push(Simulator::Now()); }

static void RxTrace1(Ptr<const Packet> p, const Address &a)
{
    if (!txQ1.empty()) {
        totalDelay1 += (Simulator::Now() - txQ1.front()).GetSeconds();
        txQ1.pop();
        rxCount1++;
    }
}
static void RxTrace2(Ptr<const Packet> p, const Address &a)
{
    if (!txQ2.empty()) {
        totalDelay2 += (Simulator::Now() - txQ2.front()).GetSeconds();
        txQ2.pop();
        rxCount2++;
    }
}

int main(int argc, char *argv[])
{
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // 1. Create the four nodes
    NodeContainer nodes;
    nodes.Create(4);

    // 2. Create the three point-to-point links (5 Mbps, 2 ms)
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer d01 = p2p.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer d12 = p2p.Install(nodes.Get(1), nodes.Get(2));
    NetDeviceContainer d32 = p2p.Install(nodes.Get(3), nodes.Get(2));

    // 3. Internet stack on all nodes
    InternetStackHelper internet;
    internet.Install(nodes);

    // 4. Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i01 = address.Assign(d01);
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer i12 = address.Assign(d12);
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer i32 = address.Assign(d32);

    // 5. Routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // 6. Flow 1: n0 -> n2, 1 Mbps
    uint16_t port1 = 5000;
    PacketSinkHelper sink1("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port1));
    ApplicationContainer sinkApp1 = sink1.Install(nodes.Get(2));
    sinkApp1.Start(Seconds(1.0));
    sinkApp1.Stop(Seconds(10.0));

    OnOffHelper source1("ns3::UdpSocketFactory", InetSocketAddress(i12.GetAddress(1), port1));
    source1.SetAttribute("DataRate", StringValue("1Mbps"));
    source1.SetAttribute("PacketSize", UintegerValue(512));
    // Keep the source continuously "on" (default OnOffHelper alternates 1s
    // on / 1s off, which would halve the effective rate below the 1 Mbps we
    // actually want to demonstrate).
    source1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=100]"));
    source1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    ApplicationContainer sourceApp1 = source1.Install(nodes.Get(0));
    sourceApp1.Start(Seconds(2.0));
    sourceApp1.Stop(Seconds(10.0));

    // Flow 2: n3 -> n0, 500 Kbps
    uint16_t port2 = 6000;
    PacketSinkHelper sink2("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port2));
    ApplicationContainer sinkApp2 = sink2.Install(nodes.Get(0));
    sinkApp2.Start(Seconds(1.0));
    sinkApp2.Stop(Seconds(10.0));

    OnOffHelper source2("ns3::UdpSocketFactory", InetSocketAddress(i01.GetAddress(0), port2));
    source2.SetAttribute("DataRate", StringValue("500Kbps"));
    source2.SetAttribute("PacketSize", UintegerValue(512));
    source2.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=100]"));
    source2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    ApplicationContainer sourceApp2 = source2.Install(nodes.Get(3));
    sourceApp2.Start(Seconds(3.0));
    sourceApp2.Stop(Seconds(10.0));

    // 7. Hook Tx/Rx trace sources for latency measurement (no FlowMonitor)
    sourceApp1.Get(0)->TraceConnectWithoutContext("Tx", MakeCallback(&TxTrace1));
    sinkApp1.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&RxTrace1));
    sourceApp2.Get(0)->TraceConnectWithoutContext("Tx", MakeCallback(&TxTrace2));
    sinkApp2.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&RxTrace2));

    // 8. Run simulation for 10 seconds
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    Ptr<PacketSink> sinkPtr1 = DynamicCast<PacketSink>(sinkApp1.Get(0));
    Ptr<PacketSink> sinkPtr2 = DynamicCast<PacketSink>(sinkApp2.Get(0));

    double activeTime1 = 10.0 - 2.0; // Flow 1 active window
    double activeTime2 = 10.0 - 3.0; // Flow 2 active window
    double thr1 = (sinkPtr1->GetTotalRx() * 8.0) / activeTime1 / 1e6; // Mbps
    double thr2 = (sinkPtr2->GetTotalRx() * 8.0) / activeTime2 / 1e6; // Mbps

    std::cout << "\n===== Flow 1 (n0 -> n2) =====\n";
    std::cout << "Total bytes received : " << sinkPtr1->GetTotalRx() << " bytes\n";
    std::cout << "Throughput           : " << thr1 << " Mbps\n";
    std::cout << "Avg packet latency    : "
              << (rxCount1 ? (totalDelay1 / rxCount1) * 1000 : 0) << " ms\n";

    std::cout << "\n===== Flow 2 (n3 -> n0) =====\n";
    std::cout << "Total bytes received : " << sinkPtr2->GetTotalRx() << " bytes\n";
    std::cout << "Throughput           : " << thr2 << " Mbps\n";
    std::cout << "Avg packet latency    : "
              << (rxCount2 ? (totalDelay2 / rxCount2) * 1000 : 0) << " ms\n";

    std::cout << "\n===== Comparison =====\n";
    std::cout << "Flow 1 offers " << (thr1 > thr2 ? "higher" : "lower")
              << " throughput than Flow 2 because its source rate (1 Mbps) is"
              << (thr1 > thr2 ? " higher" : " lower")
              << " than Flow 2's (500 Kbps); both stay well within the 5 Mbps"
              << " link capacity so latency for both is dominated by the fixed"
              << " 2 ms-per-hop propagation delay rather than queuing.\n";

    Simulator::Destroy();
    return 0;
}
