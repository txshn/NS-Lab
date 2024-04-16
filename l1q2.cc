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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("l2q2");

int main(int argc, char *argv[]){
    
    CommandLine cmd;
    cmd.Parse (argc, argv);
  
    Time::SetResolution (Time::NS);
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(4);

    PointToPointHelper p2p;
    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("50p"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    NetDeviceContainer n0n1 = p2p.Install(nodes.Get(0), nodes.Get(1));

    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("50p"));
    p2p.SetChannelAttribute("Delay", StringValue("3ms"));
    p2p.SetDeviceAttribute("DataRate", StringValue("8Mbps"));
    NetDeviceContainer n1n2 = p2p.Install(nodes.Get(1), nodes.Get(2));

    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("3p"));
    p2p.SetChannelAttribute("Delay", StringValue("4ms"));
    p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    NetDeviceContainer n2n3 = p2p.Install(nodes.Get(2), nodes.Get(3));

    InternetStackHelper stk;
    stk.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ipterfs0 = ipv4.Assign(n0n1);
    ipv4.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer ipterfs1 = ipv4.Assign(n1n2);
    ipv4.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer ipterfs2 = ipv4.Assign(n2n3);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(nodes.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(ipterfs0.GetAddress(0), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (16));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (5.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApp0 = echoClient.Install(nodes.Get(1));
    clientApp0.Start(Seconds(2.0));
    clientApp0.Stop(Seconds(10.0));

    ApplicationContainer clientApp1 = echoClient.Install(nodes.Get(2));
    clientApp1.Start(Seconds(2.0));
    clientApp1.Stop(Seconds(10.0));

    ApplicationContainer clientApp2 = echoClient.Install(nodes.Get(3));
    clientApp2.Start(Seconds(2.0));
    clientApp2.Stop(Seconds(10.0));

    AnimationInterface anim("test.xml");
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}