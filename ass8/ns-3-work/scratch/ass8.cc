// Assignment 8 - Simulation and Performance Analysis of TCP and UDP Traffic
// Topology: 4 end nodes through 2 intermediate routers (dumbbell shape)
//
//   n0 ----10Mbps/2ms---- r1 ====bottleneck==== r2 ----10Mbps/2ms---- n3
//   n1 ----10Mbps/2ms---- r1                    r2 ----10Mbps/2ms---- n2
//
//   TCP flow: n0 -> n3   (BulkSend, 1024-byte segments)
//   UDP flow: n1 -> n2   (OnOff, 512-byte packets, configurable rate)
//
// The bottleneck bandwidth, propagation delay, DropTail queue size and UDP
// rate are all CommandLine arguments so ONE binary reproduces every run of
// Experiments 1-4 (bottleneck bw / UDP rate / queue size / prop delay).
// Packet loss, average end-to-end delay, throughput and an approximate TCP
// retransmission rate are computed per flow using FlowMonitor.

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpUdpBottleneck");

int main(int argc, char *argv[])
{
    std::string bottleneckBw = "2Mbps";
    std::string bottleneckDelay = "10ms";
    uint32_t queueSize = 50;
    std::string udpRate = "1Mbps";
    double simTime = 20.0;

    CommandLine cmd;
    cmd.AddValue("bottleneckBw", "Bottleneck (r1-r2) link bandwidth", bottleneckBw);
    cmd.AddValue("bottleneckDelay", "Bottleneck (r1-r2) propagation delay", bottleneckDelay);
    cmd.AddValue("queueSize", "DropTail queue size in packets", queueSize);
    cmd.AddValue("udpRate", "UDP source data rate", udpRate);
    cmd.AddValue("simTime", "Simulation duration (s)", simTime);
    cmd.Parse(argc, argv);

    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1024));

    // Nodes: 0=n0 1=n1 2=r1 3=r2 4=n2 5=n3
    NodeContainer nodes;
    nodes.Create(6);
    Ptr<Node> n0 = nodes.Get(0), n1 = nodes.Get(1);
    Ptr<Node> r1 = nodes.Get(2), r2 = nodes.Get(3);
    Ptr<Node> n2 = nodes.Get(4), n3 = nodes.Get(5);

    // Access links: 10 Mbps, 2 ms
    PointToPointHelper access;
    access.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    access.SetChannelAttribute("Delay", StringValue("2ms"));

    // Bottleneck link: variable bandwidth/delay, DropTail queue of queueSize
    PointToPointHelper bottleneck;
    bottleneck.SetDeviceAttribute("DataRate", StringValue(bottleneckBw));
    bottleneck.SetChannelAttribute("Delay", StringValue(bottleneckDelay));
    bottleneck.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize",
                         QueueSizeValue(QueueSize(std::to_string(queueSize) + "p")));

    NetDeviceContainer d_n0r1 = access.Install(n0, r1);
    NetDeviceContainer d_n1r1 = access.Install(n1, r1);
    NetDeviceContainer d_r1r2 = bottleneck.Install(r1, r2);
    NetDeviceContainer d_r2n2 = access.Install(r2, n2);
    NetDeviceContainer d_r2n3 = access.Install(r2, n3);

    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper addr;
    addr.SetBase("10.0.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i_n0r1 = addr.Assign(d_n0r1);
    addr.SetBase("10.0.2.0", "255.255.255.0");
    Ipv4InterfaceContainer i_n1r1 = addr.Assign(d_n1r1);
    addr.SetBase("10.0.3.0", "255.255.255.0");
    Ipv4InterfaceContainer i_r1r2 = addr.Assign(d_r1r2);
    addr.SetBase("10.0.4.0", "255.255.255.0");
    Ipv4InterfaceContainer i_r2n2 = addr.Assign(d_r2n2);
    addr.SetBase("10.0.5.0", "255.255.255.0");
    Ipv4InterfaceContainer i_r2n3 = addr.Assign(d_r2n3);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // TCP flow: n0 -> n3
    uint16_t tcpPort = 9001;
    PacketSinkHelper tcpSink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), tcpPort));
    ApplicationContainer tcpSinkApp = tcpSink.Install(n3);
    tcpSinkApp.Start(Seconds(0.0));
    tcpSinkApp.Stop(Seconds(simTime));

    BulkSendHelper tcpSource("ns3::TcpSocketFactory", InetSocketAddress(i_r2n3.GetAddress(1), tcpPort));
    tcpSource.SetAttribute("MaxBytes", UintegerValue(0)); // send continuously
    ApplicationContainer tcpSourceApp = tcpSource.Install(n0);
    tcpSourceApp.Start(Seconds(1.0));
    tcpSourceApp.Stop(Seconds(simTime));

    // UDP flow: n1 -> n2
    uint16_t udpPort = 9002;
    PacketSinkHelper udpSink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), udpPort));
    ApplicationContainer udpSinkApp = udpSink.Install(n2);
    udpSinkApp.Start(Seconds(0.0));
    udpSinkApp.Stop(Seconds(simTime));

    OnOffHelper udpSource("ns3::UdpSocketFactory", InetSocketAddress(i_r2n2.GetAddress(1), udpPort));
    udpSource.SetAttribute("DataRate", StringValue(udpRate));
    udpSource.SetAttribute("PacketSize", UintegerValue(512));
    udpSource.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=100]"));
    udpSource.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    ApplicationContainer udpSourceApp = udpSource.Install(n1);
    udpSourceApp.Start(Seconds(1.0));
    udpSourceApp.Stop(Seconds(simTime));

    // NS-3 packet-level tracing (pcap on every point-to-point interface)
    access.EnablePcapAll("ass8-trace");

    // FlowMonitor: per-flow loss / delay / throughput without hand-written traces
    FlowMonitorHelper flowmonHelper;
    Ptr<FlowMonitor> monitor = flowmonHelper.InstallAll();

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    double activeTime = simTime - 1.0;

    for (auto &kv : stats) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(kv.first);
        FlowMonitor::FlowStats s = kv.second;
        std::string proto = (t.protocol == 6) ? "TCP" : (t.protocol == 17) ? "UDP" : "OTHER";
        double throughputMbps = (s.rxBytes * 8.0) / activeTime / 1e6;
        double avgDelayMs = (s.rxPackets > 0) ? (s.delaySum.GetSeconds() / s.rxPackets) * 1000 : 0;
        double lossPct = (s.txPackets > 0) ? (double)s.lostPackets / s.txPackets * 100 : 0;
        // Approximate retransmission rate: extra IP-layer transmissions beyond
        // what was actually delivered, expressed as a percentage of Tx packets
        // (a segment is only retransmitted after being lost, so this tracks
        // retransmission overhead directly from FlowMonitor's Tx/Rx counts).
        double retransPct = (proto == "TCP" && s.txPackets > 0)
                                 ? (double)(s.txPackets - s.rxPackets) / s.txPackets * 100
                                 : 0.0;

        std::cout << "[" << proto << "] " << t.sourceAddress << ":" << t.sourcePort
                  << " -> " << t.destinationAddress << ":" << t.destinationPort
                  << " | Tx=" << s.txPackets << " Rx=" << s.rxPackets
                  << " Lost=" << s.lostPackets << " (" << lossPct << "%)"
                  << " | Throughput=" << throughputMbps << "Mbps"
                  << " | AvgDelay=" << avgDelayMs << "ms";
        if (proto == "TCP") {
            std::cout << " | RetransRate=" << retransPct << "%";
        }
        std::cout << std::endl;
    }

    Simulator::Destroy();
    return 0;
}
