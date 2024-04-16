#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main ()
{
    // Create nodes for the wired network
    NodeContainer wiredNodes;
    wiredNodes.Create(3);

    // Create nodes for the wireless network
    NodeContainer wirelessNodes;
    wirelessNodes.Create(2);

    // Create a point-to-point link between wired nodes
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
    NetDeviceContainer wiredDevices;
    wiredDevices = p2p.Install(wiredNodes.Get(0), wiredNodes.Get(1));

    // Install CSMA net devices on the third wired node
    CsmaHelper csma;
    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(wiredNodes.Get(2));

    // Create wireless channel and PHY
    WifiHelper wifi;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211b);
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
    NetDeviceContainer wifiDevices;
    wifiDevices = wifi.Install(wifiChannel.Create(), wifiMac, wirelessNodes);

    // Mobility
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(5.0),
                                  "DeltaY", DoubleValue(10.0),
                                  "GridWidth", UintegerValue(3),
                                  "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wiredNodes);
    mobility.Install(wirelessNodes);

    // Install internet stack on all nodes
    InternetStackHelper stack;
    stack.Install(wiredNodes);
    stack.Install(wirelessNodes);

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer wiredInterfaces;
    wiredInterfaces = address.Assign(wiredDevices);
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer wirelessInterfaces;
    wirelessInterfaces = address.Assign(wifiDevices);

    // Enable global routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Create a packet sink on node 1 of the wireless network
    PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", InetSocketAddress(wirelessInterfaces.GetAddress(1), 9));
    ApplicationContainer sinkApps = sinkHelper.Install(wirelessNodes.Get(1));
    sinkApps.Start(Seconds(0.0));
    sinkApps.Stop(Seconds(10.0));

    // Create a UDP client application and install it on node 0 of the wired network
    UdpEchoClientHelper client("10.1.1.2", 9);
    client.SetAttribute("MaxPackets", UintegerValue(1));
    client.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    client.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps = client.Install(wiredNodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    // Enable PCAP tracing
    p2p.EnablePcapAll("wired-topology");
    wifi.EnablePcapAll("wireless-topology");

    // Create animation XML
    AnimationInterface anim("network-animation.xml");

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}