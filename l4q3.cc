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
#include "ns3/csma-star-helper.h"
 

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Lab4_Q2");

int main(int argc, char *argv[]){

    CommandLine cmd;
    cmd.Parse (argc, argv);
 
    Time::SetResolution (Time::NS);
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(8);

    NodeContainer c0 = NodeContainer(nodes.Get(0), nodes.Get(1), nodes.Get(2));
    NodeContainer c1 = NodeContainer(nodes.Get(2),nodes.Get(3), nodes.Get(4));
    NodeContainer c2 = NodeContainer(nodes.Get(4), nodes.Get(5));
    NodeContainer c3 = NodeContainer(nodes.Get(5), nodes.Get(6), nodes.Get(7));

    PointToPointHelper ptp;
    ptp.SetDeviceAttribute("DataRate", StringValue("50Mbps"));
    ptp.SetChannelAttribute("Delay", StringValue("5ms"));

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("50Mbps"));
    csma.SetChannelAttribute("Delay", StringValue("5ms"));


    NetDeviceContainer lan0 = csma.Install(c0);
    NetDeviceContainer lan1 = csma.Install(c1);
    NetDeviceContainer pt45 = ptp.Install(c2);
    NetDeviceContainer lan2 = csma.Install(c3);

    InternetStackHelper stk;
    stk.InstallAll();

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0","255.255.255.0");
    Ipv4InterfaceContainer ipterfs0 = ipv4.Assign(lan0);
    ipv4.SetBase("10.1.2.0","255.255.255.0");
    Ipv4InterfaceContainer ipterfs1 = ipv4.Assign(lan1);
    ipv4.SetBase("10.1.3.0","255.255.255.0");
    Ipv4InterfaceContainer ipterfs2 = ipv4.Assign(pt45);
    ipv4.SetBase("10.1.4.0","255.255.255.0");
    Ipv4InterfaceContainer ipterfs3 = ipv4.Assign(lan2);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    //start multicasting
    Ipv4Address multicastSource("10.1.1.1");
    Ipv4Address multicastGroup("225.1.2.4");

    Ipv4StaticRoutingHelper multicast;
    Ptr<Node> multicastRouter = nodes.Get(2);
    Ptr<NetDevice> inputIf = lan0.Get(2);
    NetDeviceContainer outputDevices;
    outputDevices.Add(lan1.Get(0));

    multicast.AddMulticastRoute(multicastRouter, multicastSource, multicastGroup, inputIf, outputDevices);

    Ptr<Node> Sender = nodes.Get(0);
    Ptr<NetDevice> SenderIf = lan0.Get(0);
    multicast.SetDefaultMulticastRoute(Sender, SenderIf);

    uint16_t multicastPort = 9;

    OnOffHelper onoff ("ns3::UdpSocketFactory", Address(InetSocketAddress(multicastGroup, multicastPort)));
    onoff.SetConstantRate(DataRate("255b/s"));
    onoff.SetAttribute("PacketSize", UintegerValue(128));

    ApplicationContainer srcC = onoff.Install(nodes.Get(0));
    srcC.Start(Seconds(1.0));
    srcC.Stop(Seconds(10.0));

    PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), multicastPort));

    ApplicationContainer sinkC = sink.Install(nodes.Get(5));
    sinkC.Start(Seconds(2.0));
    sinkC.Stop(Seconds(10.0));

    AnimationInterface anim("test.xml");
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}