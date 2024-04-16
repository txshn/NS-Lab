#include <iostream>
#include <fstream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/csma-module.h"
 

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Lab2_Q1");

int main(int argc, char *argv[]){
    CommandLine cmd;
    cmd.Parse (argc, argv);
 
    Time::SetResolution (Time::NS);
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(5);

    NodeContainer lan = NodeContainer(nodes.Get(1), nodes.Get(2), nodes.Get(3), nodes.Get(4));

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("50Mbps"));
    csma.SetChannelAttribute("Delay", StringValue("2ms"));

    PointToPointHelper ptp;
    ptp.SetChannelAttribute("Delay", StringValue("5ms"));
    ptp.SetDeviceAttribute("DataRate", StringValue("10Mbps"));

    NetDeviceContainer lan0 = csma.Install(lan);
    NetDeviceContainer p2p0 = ptp.Install(nodes.Get(0),nodes.Get(1));

    InternetStackHelper stk;
    stk.InstallAll();

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ipterfs0 = ipv4.Assign(p2p0);
    ipv4.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer ipterfs1 = ipv4.Assign(lan0);
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    uint16_t multicastPort = 9;

    OnOffHelper onoff ("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address("255.255.255.255"), multicastPort)));
    onoff.SetConstantRate(DataRate("500kb/s"));
    onoff.SetAttribute("PacketSize", UintegerValue(128));

    ApplicationContainer srcC = onoff.Install(nodes.Get(1));
    srcC.Start(Seconds(1.0));
    srcC.Stop(Seconds(10.0));

    PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), multicastPort));

    ApplicationContainer sinkC = sink.Install(nodes.Get(4));
    sinkC.Add(sink.Install(nodes.Get(0)));
    sinkC.Start(Seconds(2.0));
    sinkC.Stop(Seconds(10.0));

    AnimationInterface anim("test.xml");
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}