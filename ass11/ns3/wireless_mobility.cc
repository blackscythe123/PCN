/**
 * ============================================================================
 * Assignment 11: NS-3 Heterogeneous Wireless Network (802.11n) with Mobility & PDR
 * ============================================================================
 * Description: Implements Wi-Fi network with 1 static Access Point (AP) and 4 mobile
 *              Station (STA) nodes moving under RandomWalk2d mobility. Measures
 *              Throughput, Packet Delivery Ratio (PDR %), and Delay using FlowMonitor.
 *
 * HOW TO SET UP & COMPILE IN NS-3:
 *   1. Copy this file into your NS-3 scratch directory:
 *      cp ass11/ns3/wireless_mobility.cc ~/ns-allinone-3.xx/ns-3.xx/scratch/
 *   2. Navigate to your NS-3 root directory:
 *      cd ~/ns-allinone-3.xx/ns-3.xx
 *   3. Build & Run:
 *      ./ns3 run scratch/wireless_mobility
 *
 * HOW TO STOP:
 *   - Automatic Stop: Simulator stops automatically at t = 10.0 seconds (Simulator::Stop(Seconds(10.0))).
 *   - Abort simulation early: Press [Ctrl + C] in the terminal running ./ns3.
 * ============================================================================
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // 1 AP node and 4 Mobile Station (STA) nodes
    NodeContainer wifiApNode;
    wifiApNode.Create(1);

    NodeContainer wifiStaNodes;
    wifiStaNodes.Create(4);

    // 802.11n Wi-Fi PHY and Channel
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211n_2_4GHZ);

    WifiMacHelper mac;
    Ssid ssid = Ssid("SSN-CSE-WIFI");

    // Configure STA nodes
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    NetDeviceContainer staDevices = wifi.Install(phy, mac, wifiStaNodes);

    // Configure AP node
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevices = wifi.Install(phy, mac, wifiApNode);

    // Mobility: Constant position for AP
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 0.0)); // AP at origin
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNode);

    // Mobility: RandomWalk2d for STA nodes within 50x50m box
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)),
                             "Speed", StringValue("ns3::ConstantRandomVariable[Constant=2.0]"));
    mobility.Install(wifiStaNodes);

    // Install IP Stack
    InternetStackHelper stack;
    stack.Install(wifiApNode);
    stack.Install(wifiStaNodes);

    Ipv4AddressHelper address;
    address.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer staInterfaces = address.Assign(staDevices);
    Ipv4InterfaceContainer apInterface = address.Assign(apDevices);

    // Traffic: UDP PacketSink on AP
    uint16_t port = 9;
    PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp = sink.Install(wifiApNode.Get(0));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(10.0));

    // Traffic: STA nodes send UDP traffic to AP
    OnOffHelper onoff("ns3::UdpSocketFactory", InetSocketAddress(apInterface.GetAddress(0), port));
    onoff.SetAttribute("DataRate", StringValue("1Mbps"));
    onoff.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps = onoff.Install(wifiStaNodes);
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(10.0));

    // FlowMonitor for PDR and Throughput
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    monitor->CheckForLostPackets();
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    for (auto const& [id, s] : stats) {
        double throughput = s.rxBytes * 8.0 / (9.0 * 1e6); // Mbps over active 9s
        double pdr = (s.txPackets > 0) ? ((double)s.rxPackets / s.txPackets * 100.0) : 0.0;
        double avgDelay = (s.rxPackets > 0) ? (s.delaySum.GetSeconds() / s.rxPackets * 1000.0) : 0.0;
        std::cout << "Flow " << id << " | Tx: " << s.txPackets << " | Rx: " << s.rxPackets
                  << " | Throughput: " << throughput << " Mbps"
                  << " | PDR: " << pdr << " %"
                  << " | Delay: " << avgDelay << " ms" << std::endl;
    }

    Simulator::Destroy();
    return 0;
}
