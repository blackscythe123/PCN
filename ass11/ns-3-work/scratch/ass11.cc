// Assignment 11 - Performance Analysis of a Heterogeneous Wireless Network
// One mobile node talks UDP to a fixed gateway/coordinator node over one of
// three wireless technologies, while moving with RandomWaypoint mobility at
// a configurable speed.
//
// NS-3 has no Bluetooth module, and modelling full Zigbee (802.15.4 PHY/MAC
// plus 6LoWPAN for IP) would add a second, incompatible code path just for
// one technology. Instead all three technologies are built on ns-3's real
// WifiNetDevice (ad-hoc), each configured with the transmit power, PHY rate
// and packet size that actually characterise it, so the comparative signal
// strength / throughput / loss / delay trends across technologies and
// mobility speeds stay meaningful with one simple, uniform program:
//   Wi-Fi      : 802.11g, 20 dBm tx power, 54 Mbps, 1024 B packets.
//   Bluetooth  : 802.11b, 4 dBm tx power (~Bluetooth Class 2), 1 Mbps, 200 B.
//   Zigbee     : 802.11b, -10 dBm tx power (802.15.4's very low Tx power),
//                1 Mbps, 100 B packets (802.15.4's small MTU).
// This is a documented simplification, not a claim of protocol-exact
// Bluetooth/Zigbee behaviour.

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HeterogeneousWireless");

static double g_signalSum = 0.0;
static uint32_t g_signalCount = 0;

static void
SnifferRx(Ptr<const Packet> packet, uint16_t channelFreqMhz, WifiTxVector txVector,
          MpduInfo aMpdu, SignalNoiseDbm signalNoise, uint16_t staId)
{
    g_signalSum += signalNoise.signal;
    g_signalCount++;
}

int
main(int argc, char *argv[])
{
    std::string tech = "WiFi"; // WiFi | Bluetooth | Zigbee
    double speed = 1.0;        // m/s
    double simTime = 30.0;

    CommandLine cmd;
    cmd.AddValue("tech", "Wireless technology: WiFi, Bluetooth or Zigbee", tech);
    cmd.AddValue("speed", "Mobile node speed (m/s)", speed);
    cmd.AddValue("simTime", "Simulation duration (s)", simTime);
    cmd.Parse(argc, argv);

    double txPowerDbm;
    std::string dataMode;
    uint32_t packetSize;
    std::string wifiStandardStr;
    if (tech == "WiFi") {
        txPowerDbm = 20.0;
        dataMode = "ErpOfdmRate54Mbps";
        packetSize = 1024;
        wifiStandardStr = "80211g";
    } else if (tech == "Bluetooth") {
        txPowerDbm = 4.0;
        dataMode = "DsssRate1Mbps";
        packetSize = 200;
        wifiStandardStr = "80211b";
    } else { // Zigbee
        txPowerDbm = -10.0;
        dataMode = "DsssRate1Mbps";
        packetSize = 100;
        wifiStandardStr = "80211b";
    }

    NodeContainer nodes;
    nodes.Create(2); // 0 = gateway (fixed), 1 = mobile
    Ptr<Node> gateway = nodes.Get(0);
    Ptr<Node> mobile = nodes.Get(1);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());
    phy.Set("TxPowerStart", DoubleValue(txPowerDbm));
    phy.Set("TxPowerEnd", DoubleValue(txPowerDbm));

    WifiHelper wifi;
    wifi.SetStandard(wifiStandardStr == "80211g" ? WIFI_STANDARD_80211g : WIFI_STANDARD_80211b);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                  "DataMode", StringValue(dataMode),
                                  "ControlMode", StringValue(dataMode));

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    // Gateway is fixed. The mobile node starts right next to it and moves
    // away in a straight line at a constant --speed (m/s) -- a simple,
    // deterministic mobility pattern (a vehicle/user driving away from its
    // access point/coordinator) that cleanly shows how increasing speed
    // increases distance-over-time, and therefore degrades signal quality
    // and delivery, rather than depending on where a RandomWaypoint model
    // happens to wander.
    MobilityHelper gwMobility;
    gwMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    gwMobility.Install(gateway);
    gateway->GetObject<MobilityModel>()->SetPosition(Vector(0.0, 0.0, 0.0));

    MobilityHelper mobMobility;
    mobMobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobMobility.Install(mobile);
    Ptr<ConstantVelocityMobilityModel> cvmm = mobile->GetObject<ConstantVelocityMobilityModel>();
    cvmm->SetPosition(Vector(2.0, 0.0, 0.0));
    cvmm->SetVelocity(Vector(speed, 0.0, 0.0));

    InternetStackHelper internet;
    internet.Install(nodes);
    Ipv4AddressHelper addr;
    addr.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ifaces = addr.Assign(devices);

    uint16_t port = 7000;
    PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp = sink.Install(gateway);
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(simTime));

    OnOffHelper source("ns3::UdpSocketFactory", InetSocketAddress(ifaces.GetAddress(0), port));
    source.SetAttribute("DataRate", StringValue("200Kbps"));
    source.SetAttribute("PacketSize", UintegerValue(packetSize));
    source.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=100]"));
    source.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    ApplicationContainer sourceApp = source.Install(mobile);
    sourceApp.Start(Seconds(1.0));
    sourceApp.Stop(Seconds(simTime));

    Ptr<WifiNetDevice> gwWifiDev = DynamicCast<WifiNetDevice>(devices.Get(0));
    gwWifiDev->GetPhy()->TraceConnectWithoutContext("MonitorSnifferRx", MakeCallback(&SnifferRx));

    FlowMonitorHelper flowmonHelper;
    Ptr<FlowMonitor> monitor = flowmonHelper.InstallAll();

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    double activeTime = simTime - 1.0;

    std::cout << "===== tech=" << tech << " speed=" << speed << "m/s =====" << std::endl;
    for (auto &kv : stats) {
        FlowMonitor::FlowStats s = kv.second;
        double throughputKbps = (s.rxBytes * 8.0) / activeTime / 1e3;
        double avgDelayMs = (s.rxPackets > 0) ? (s.delaySum.GetSeconds() / s.rxPackets) * 1000 : 0;
        double lossPct = (s.txPackets > 0) ? (double)(s.txPackets - s.rxPackets) / s.txPackets * 100 : 0;
        double avgSignalDbm = (g_signalCount > 0) ? g_signalSum / g_signalCount : 0;
        std::cout << "Tx=" << s.txPackets << " Rx=" << s.rxPackets
                   << " | Throughput=" << throughputKbps << "Kbps"
                   << " | PacketLoss=" << lossPct << "%"
                   << " | AvgDelay=" << avgDelayMs << "ms"
                   << " | AvgSignal=" << avgSignalDbm << "dBm" << std::endl;
    }

    Simulator::Destroy();
    return 0;
}
