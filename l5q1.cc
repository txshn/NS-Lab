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
#include "ns3/packet-sink.h"
 

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Lab4_Q2");

int main(int argc, char *argv[]){

    CommandLine cmd;
    cmd.Parse (argc, argv);

    uint32_t maxBytes = 0;
 
    Time::SetResolution (Time::NS);
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(5);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("50Mbps"));
    csma.SetChannelAttribute("Delay", StringValue("5ms"));

    NetDeviceContainer lan0 = csma.Install(nodes);

    InternetStackHelper stk;
    stk.InstallAll();

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ipterfs0 = ipv4.Assign(lan0);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    //Simulate a Bulk Sender for TCP
    BulkSendHelper src("ns3::TcpSocketFactory", InetSocketAddress(ipterfs0.GetAddress(4), 9));
    src.SetAttribute("MaxBytes", UintegerValue(maxBytes));
    ApplicationContainer srcApps = src.Install(nodes.Get(0));
    srcApps.Start(Seconds(1.0));
    srcApps.Stop(Seconds(10.0));

    //Simulate a reciever using PacketSinkHelper
    PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9));
    ApplicationContainer sinkApps = sink.Install(nodes.Get(4));
    sinkApps.Start(Seconds(2.0));
    sinkApps.Stop(Seconds(10.0));

    AnimationInterface anim("test.xml");
    Simulator::Stop (Seconds (10.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}