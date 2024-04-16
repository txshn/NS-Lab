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
    nodes.Create(3);

    NodeContainer csmaNodes;
    csmaNodes.Add(nodes.Get(1));
    csmaNodes.Add(nodes.Get(2));
    csmaNodes.Create(4);

    PointToPointHelper ptp;
    ptp.SetChannelAttribute("Delay", StringValue("5ms"));
    ptp.SetDeviceAttribute("DataRate",StringValue("10Mbps"));
    NetDeviceContainer nd01 = ptp.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer nd12 = ptp.Install(nodes.Get(1),nodes.Get(2));

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("10Mbps"));
    csma.SetChannelAttribute("Delay", StringValue("5ms"));
    NetDeviceContainer cD16 = csma.Install(csmaNodes);

    InternetStackHelper stk;
    stk.InstallAll();

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ipterfs0 = ipv4.Assign(nd01);
    ipv4.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer ipterfs1 = ipv4.Assign(nd12);
    ipv4.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer ipterfs2 = ipv4.Assign(cD16);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(nodes.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(ipterfs0.GetAddress(0), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (16));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (5.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApp0 = echoClient.Install(csmaNodes.Get(4));
    clientApp0.Start(Seconds(2.0));
    clientApp0.Stop(Seconds(10.0));

    AsciiTraceHelper ascii;
    ptp.EnableAsciiAll(ascii.CreateFileStream("Lab2_Q1.tr"));

    AnimationInterface anim("test.xml");
    Simulator::Run();
    Simulator::Destroy();

   return 0;

}