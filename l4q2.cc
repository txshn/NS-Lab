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

    uint32_t nSpokes = 8;
    CsmaHelper lan0;
    lan0.SetChannelAttribute("DataRate", StringValue("50Mbps"));
    lan0.SetChannelAttribute("Delay", StringValue("5ms"));
    
    CsmaStarHelper star(nSpokes, lan0);

    InternetStackHelper stk;
    star.InstallStack(stk);

    star.AssignIpv4Addresses(Ipv4AddressHelper("10.1.1.0", "255.255.255.0"));

    uint16_t port = 50000;
    Address hubLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
    PacketSinkHelper pktS("ns3::TcpSocketFactory", hubLocalAddress);
    ApplicationContainer hubApp = pktS.Install(star.GetHub());
    hubApp.Start(Seconds(1.0));
    hubApp.Stop(Seconds(10.0));

    OnOffHelper onOff("ns3::TcpSocketFactory", Address(InetSocketAddress(Ipv4Address("255.255.255.255"), 9)));
    onOff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onOff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer spokeApps;
    for(uint32_t i = 0; i < star.SpokeCount(); ++i){
        AddressValue remoteAddress(InetSocketAddress(star.GetHubIpv4Address(i), port));
        onOff.SetAttribute("Remote", remoteAddress);
        spokeApps.Add(onOff.Install(star.GetSpokeNode(i)));
    }
    spokeApps.Start(Seconds(1.0));
    spokeApps.Stop(Seconds(10.0));
    //star.Bounding(0,0,100,100);

    AnimationInterface anim("test.xml");
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}