#include <iostream>
#include <fstream>
#include <string>
#include "ns3/point-to-point-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("l1q1");

int main(int argc, char *argv[]){

    CommandLine cmd;
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnabl("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogCompenentEnable("TcpEchoClientApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(3);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("Datarate", StringValue("1Mbps"));
    p2p.SetChannel Attribute("Delay", StringValue("5ms"));

    NetDeviceContainer nd01 = p2p.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer nd12 = p2p.Install(nodes.Get(1), nodes.Get(2));

    InternetStackHelper stk;
    stk.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    ipv4.Assign(nd01);
    Ipv4InterfaceContainer ipterfs = ipv4.Assign(nd12);

    UdpEchoServerHelper echoserver(9);

    AplicationContainer serverApps = echoServer.Install(nodes.Get(2));

    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    
    UdpEchoClientHelper echoClient(ipterfs.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApp0 = echoClient.Install(nodes.Get(0));
    clientApp0.Start(Seconds(2.0));
    clientApp0.Stop(Seconds(10.0));

    AnimationInterface anim("test.xml");
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
