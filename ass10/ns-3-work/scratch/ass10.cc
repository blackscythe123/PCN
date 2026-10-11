// Assignment 10 - Comparative Evaluation of Routing Protocols in NS-3
// (RIP, "OSPF", AODV) under normal / congested / link-failure conditions.
//
// Topology (same graph for every protocol, a diamond so a failed link has an
// alternate path):
//
//        n0                                   n1
//          \                                 /
//           r1 ---- r2 ----                r3
//          /          \                   /  \
//        n2             ---- r4 --------      n3
//
// Links: n0-r1, n2-r1, r1-r2, r1-r4, r2-r3, r4-r3, r3-n1, r3-n3
//
// n0 -> n1 is the primary CBR/UDP flow. In the "congestion" condition a
// second simultaneous flow n2 -> n3 is added. In the "failure" condition the
// r1-r2 link is brought down mid-simulation, forcing traffic via r1-r4-r3.
//
// --protocol=RIP|OSPF|AODV selects the routing approach:
//   RIP   - ns-3's built-in distance-vector RipHelper on the 4 routers.
//   OSPF  - ns-3 has no OSPF module; Ipv4GlobalRoutingHelper (a link-state
//           shortest-path algorithm, the same family OSPF belongs to) is
//           used as the closest available stand-in. This is a documented
//           simplification, not a real OSPF implementation.
//   AODV  - a genuine on-demand ad-hoc routing protocol. AODV needs a
//           broadcast-capable link for route discovery, so for this run
//           every link in the SAME graph is built with CSMA instead of
//           point-to-point (still one link = one 2-node segment, so the
//           multi-hop structure is identical either way).
//
// FlowMonitor gives Packet Delivery Ratio and end-to-end delay. "Routing
// table updates" are counted from each protocol's own NS_LOG control-message
// output (RIP route-update sends, AODV RREQ/RREP/RERR sends, or the number
// of global-routing recomputations for the OSPF stand-in).

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/aodv-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RoutingCompare");

int
main(int argc, char *argv[])
{
    std::string protocol = "RIP";     // RIP | OSPF | AODV
    std::string condition = "normal"; // normal | congestion | failure
    double simTime = 20.0;

    CommandLine cmd;
    cmd.AddValue("protocol", "Routing approach: RIP, OSPF or AODV", protocol);
    cmd.AddValue("condition", "Network condition: normal, congestion or failure", condition);
    cmd.AddValue("simTime", "Simulation duration (s)", simTime);
    cmd.Parse(argc, argv);

    if (protocol == "RIP") {
        LogComponentEnable("Rip", LOG_LEVEL_DEBUG);
    } else if (protocol == "AODV") {
        LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_LOGIC);
    }

    bool useCsma = (protocol == "AODV");

    NodeContainer hosts;   // 0=n0 1=n1 2=n2 3=n3
    hosts.Create(4);
    NodeContainer routers; // 0=r1 1=r2 2=r3 3=r4
    routers.Create(4);
    Ptr<Node> n0 = hosts.Get(0), n1 = hosts.Get(1), n2 = hosts.Get(2), n3 = hosts.Get(3);
    Ptr<Node> r1 = routers.Get(0), r2 = routers.Get(1), r3 = routers.Get(2), r4 = routers.Get(3);

    // --- Internet stack + routing protocol selection ---
    InternetStackHelper hostStack;
    InternetStackHelper routerStack;
    if (protocol == "RIP") {
        RipHelper rip;
        routerStack.SetRoutingHelper(rip);
    } else if (protocol == "AODV") {
        AodvHelper aodv;
        hostStack.SetRoutingHelper(aodv);
        routerStack.SetRoutingHelper(aodv);
    }
    // OSPF stand-in and RIP hosts both use the default (global routing capable) stack.
    hostStack.Install(hosts);
    routerStack.Install(routers);

    // --- Links: each "link" is one 2-node segment, CSMA for AODV else P2P ---
    auto makeLink = [&](Ptr<Node> a, Ptr<Node> b, Ipv4AddressHelper &addr) -> Ipv4InterfaceContainer {
        NetDeviceContainer dev;
        if (useCsma) {
            CsmaHelper csma;
            csma.SetChannelAttribute("DataRate", StringValue("1Mbps"));
            csma.SetChannelAttribute("Delay", StringValue("2ms"));
            dev = csma.Install(NodeContainer(a, b));
        } else {
            PointToPointHelper p2p;
            p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
            p2p.SetChannelAttribute("Delay", StringValue("2ms"));
            dev = p2p.Install(a, b);
        }
        Ipv4InterfaceContainer iface = addr.Assign(dev);
        addr.NewNetwork();
        return iface;
    };

    Ipv4AddressHelper addr;
    addr.SetBase("10.0.0.0", "255.255.255.0");
    makeLink(n0, r1, addr);
    makeLink(n2, r1, addr);
    Ipv4InterfaceContainer r1r2 = makeLink(r1, r2, addr);
    makeLink(r1, r4, addr);
    makeLink(r2, r3, addr);
    makeLink(r4, r3, addr);
    Ipv4InterfaceContainer r3n1 = makeLink(r3, n1, addr);
    Ipv4InterfaceContainer r3n3 = makeLink(r3, n3, addr);

    if (protocol == "OSPF") {
        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }
    // RIP hosts (n0..n3) are not running RIP themselves; give them a default
    // route toward their attached router, as ns-3's own RIP examples do.
    if (protocol == "RIP") {
        Ipv4StaticRoutingHelper staticRouting;
        Ptr<Ipv4> ipv4;
        ipv4 = n0->GetObject<Ipv4>();
        staticRouting.GetStaticRouting(ipv4)->SetDefaultRoute(r1r2.GetAddress(0), 1);
        ipv4 = n2->GetObject<Ipv4>();
        staticRouting.GetStaticRouting(ipv4)->SetDefaultRoute(r1r2.GetAddress(0), 1);
        ipv4 = n1->GetObject<Ipv4>();
        staticRouting.GetStaticRouting(ipv4)->SetDefaultRoute(r3n1.GetAddress(0), 1);
        ipv4 = n3->GetObject<Ipv4>();
        staticRouting.GetStaticRouting(ipv4)->SetDefaultRoute(r3n3.GetAddress(0), 1);
    }

    // --- CBR/UDP traffic ---
    uint16_t port1 = 6000, port2 = 6001;
    PacketSinkHelper sink1("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port1));
    ApplicationContainer sinkApp1 = sink1.Install(n1);
    sinkApp1.Start(Seconds(0.0));
    sinkApp1.Stop(Seconds(simTime));

    OnOffHelper source1("ns3::UdpSocketFactory", InetSocketAddress(r3n1.GetAddress(1), port1));
    source1.SetAttribute("DataRate", StringValue("500Kbps"));
    source1.SetAttribute("PacketSize", UintegerValue(512));
    source1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=100]"));
    source1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    ApplicationContainer sourceApp1 = source1.Install(n0);
    sourceApp1.Start(Seconds(2.0));
    sourceApp1.Stop(Seconds(simTime));

    if (condition == "congestion") {
        PacketSinkHelper sink2("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port2));
        ApplicationContainer sinkApp2 = sink2.Install(n3);
        sinkApp2.Start(Seconds(0.0));
        sinkApp2.Stop(Seconds(simTime));

        OnOffHelper source2("ns3::UdpSocketFactory", InetSocketAddress(r3n3.GetAddress(1), port2));
        source2.SetAttribute("DataRate", StringValue("500Kbps"));
        source2.SetAttribute("PacketSize", UintegerValue(512));
        source2.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=100]"));
        source2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        ApplicationContainer sourceApp2 = source2.Install(n2);
        sourceApp2.Start(Seconds(2.0));
        sourceApp2.Stop(Seconds(simTime));
    }

    if (condition == "failure") {
        double failTime = simTime / 2.0;
        Ptr<Ipv4> ipv4R1 = r1->GetObject<Ipv4>();
        Ptr<Ipv4> ipv4R2 = r2->GetObject<Ipv4>();
        int32_t ifR1 = ipv4R1->GetInterfaceForAddress(r1r2.GetAddress(0));
        int32_t ifR2 = ipv4R2->GetInterfaceForAddress(r1r2.GetAddress(1));
        Simulator::Schedule(Seconds(failTime), &Ipv4::SetDown, ipv4R1, ifR1);
        Simulator::Schedule(Seconds(failTime), &Ipv4::SetDown, ipv4R2, ifR2);
        if (protocol == "OSPF") {
            Simulator::Schedule(Seconds(failTime + 0.1), &Ipv4GlobalRoutingHelper::RecomputeRoutingTables);
        }
    }

    FlowMonitorHelper flowmonHelper;
    Ptr<FlowMonitor> monitor = flowmonHelper.InstallAll();

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    std::cout << "===== protocol=" << protocol << " condition=" << condition << " =====" << std::endl;
    for (auto &kv : stats) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(kv.first);
        FlowMonitor::FlowStats s = kv.second;
        if (t.destinationPort != port1 && t.destinationPort != port2) continue;
        double pdr = (s.txPackets > 0) ? (double)s.rxPackets / s.txPackets * 100 : 0;
        double avgDelayMs = (s.rxPackets > 0) ? (s.delaySum.GetSeconds() / s.rxPackets) * 1000 : 0;
        std::cout << "Flow " << t.sourceAddress << " -> port " << t.destinationPort
                   << " | Tx=" << s.txPackets << " Rx=" << s.rxPackets
                   << " | PDR=" << pdr << "%"
                   << " | AvgDelay=" << avgDelayMs << "ms" << std::endl;
    }

    Simulator::Destroy();
    return 0;
}
