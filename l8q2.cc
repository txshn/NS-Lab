/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015, IMDEA Networks Institute
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Hany Assasa <hany.assasa@gmail.com>
.*
 * This is a simple example to test TCP over 802.11n (with MPDU aggregation enabled).
 *
 * Network topology:
 *
 *   Ap    STA
 *   *      *
 *   |      |
 *   n1     n2
 *
 * In this example, an HT station sends TCP packets to the access point.
 * We report the total throughput received during a window of 100ms.
 * The user can specify the application data rate and choose the variant
 * of TCP i.e. congestion control algorithm to use.
 */

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/mobility-module.h"
#include "ns3/packet-sink.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/tcp-westwood.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-module.h"


NS_LOG_COMPONENT_DEFINE ("wifi-tcp");

using namespace ns3;

Ptr <PacketSink> sink;                         /* Pointer to the packet sink application */
uint64_t lastTotalRx = 0;                     /* The value of the last total received bytes */
//double averageThroughput = 0;                 /* The average throughput in last 10 seconds */

void
CalculateThroughput() {
    Time now = Simulator::Now();                                         /* Return the simulator's virtual time. */
    double cur =
            (sink->GetTotalRx() - lastTotalRx) * (double) 8 / 1e5;     /* Convert Application RX Packets to MBits. */
//    averageThroughput = (sink->GetTotalRx() * (double) 8) / (1e6 * now.GetSeconds());
    //Create a file to store throughput values
//    std::ofstream outfile;
//    outfile.open("throughput.txt", std::ios_base::app);
//    outfile << now.GetSeconds() << " " << averageThroughput << std::endl;
//    std::cout << "Average throughput: " << averageThroughput << " Mbit/s" << std::endl;
    std::cout << now.GetSeconds() << "s: \t" << cur << " Mbit/s" << std::endl;
    lastTotalRx = sink->GetTotalRx();
    Simulator::Schedule(MilliSeconds(100), &CalculateThroughput);
}

int
main(int argc, char *argv[]) {
    uint32_t payloadSize = 1472;                       /* Transport layer payload size in bytes. */
    std::string dataRate = "100Mbps";                  /* Application layer datarate. */
    std::string tcpVariant = "TcpNewReno";             /* TCP variant type. */
    std::string phyRate = "HtMcs7";                    /* Physical layer bitrate. */
    double simulationTime = 10;                        /* Simulation time in seconds. */
    bool pcapTracing = false;                          /* PCAP Tracing is enabled or not. */

    /* Command line argument parser setup. */
    CommandLine cmd(__FILE__);
    cmd.AddValue("payloadSize", "Payload size in bytes", payloadSize);
    cmd.AddValue("dataRate", "Application data ate", dataRate);
    cmd.AddValue("tcpVariant", "Transport protocol to use: TcpNewReno, "
                               "TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
                               "TcpBic, TcpYeah, TcpIllinois, TcpWestwood, TcpWestwoodPlus, TcpLedbat ", tcpVariant);
    cmd.AddValue("phyRate", "Physical layer bitrate", phyRate);
    cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
    cmd.AddValue("pcap", "Enable/disable PCAP Tracing", pcapTracing);
    cmd.Parse(argc, argv);

    tcpVariant = std::string("ns3::") + tcpVariant;
    // Select TCP variant
    if (tcpVariant.compare("ns3::TcpWestwoodPlus") == 0) {
        // TcpWestwoodPlus is not an actual TypeId name; we need TcpWestwood here
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpWestwood::GetTypeId()));
        // the default protocol type in ns3::TcpWestwood is WESTWOOD
        Config::SetDefault("ns3::TcpWestwood::ProtocolType", EnumValue(TcpWestwood::WESTWOODPLUS));
    } else {
        TypeId tcpTid;
        NS_ABORT_MSG_UNLESS(TypeId::LookupByNameFailSafe(tcpVariant, &tcpTid), "TypeId " << tcpVariant << " not found");
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TypeId::LookupByName(tcpVariant)));
    }

    /* Configure TCP Options */
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));

    WifiMacHelper wifiMac;
    WifiHelper wifiHelper;
    wifiHelper.SetStandard(ns3::WIFI_STANDARD_80211p);

    /* Set up Legacy Channel */
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel", "Frequency", DoubleValue(5e9));

    /* Setup Physical Layer */
    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());
    wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");
    wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                       "DataMode", StringValue(phyRate),
                                       "ControlMode", StringValue("HtMcs0"));

    NodeContainer networkNodes;
    networkNodes.Create(2);
    Ptr <Node> apWifiNode = networkNodes.Get(0);
    Ptr <Node> staWifiNode = networkNodes.Get(1);

    /* Configure AP */
    Ssid ssid = Ssid("network");
    wifiMac.SetType("ns3::ApWifiMac",
                    "Ssid", SsidValue(ssid));

    NetDeviceContainer apDevice;
    apDevice = wifiHelper.Install(wifiPhy, wifiMac, apWifiNode);

    /* Configure STA */
    wifiMac.SetType("ns3::StaWifiMac",
                    "Ssid", SsidValue(ssid));

    NetDeviceContainer staDevices;
    staDevices = wifiHelper.Install(wifiPhy, wifiMac, staWifiNode);

    /* Mobility model */
    MobilityHelper mobility;
    Ptr <ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 0.0));
    positionAlloc->Add(Vector(1.0, 1.0, 0.0));

    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(apWifiNode);
//    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
//                              "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)));
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(staWifiNode);

    Ptr <ConstantVelocityMobilityModel> mob = staWifiNode->GetObject<ConstantVelocityMobilityModel>();
    mob->SetVelocity(Vector(25, 0, 0));

    /* Internet stack */
    InternetStackHelper stack;
    stack.Install(networkNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterface;
    apInterface = address.Assign(apDevice);
    Ipv4InterfaceContainer staInterface;
    staInterface = address.Assign(staDevices);

    /* Populate routing table */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    /* Install TCP Receiver on the access point */
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9));
    ApplicationContainer sinkApp = sinkHelper.Install(apWifiNode);
    sink = StaticCast<PacketSink>(sinkApp.Get(0));

    /* Install TCP/UDP Transmitter on the station */
    OnOffHelper server("ns3::TcpSocketFactory", (InetSocketAddress(apInterface.GetAddress(0), 9)));
    server.SetAttribute("PacketSize", UintegerValue(payloadSize));
    server.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    server.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    server.SetAttribute("DataRate", DataRateValue(DataRate(dataRate)));
    ApplicationContainer serverApp = server.Install(staWifiNode);

    /* Start Applications */
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(simulationTime + 1));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(simulationTime + 1));
    Simulator::Schedule(Seconds(1.1), &CalculateThroughput);

    /* Enable Traces */
    if (pcapTracing) {
        wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        wifiPhy.EnablePcap("AccessPoint", apDevice);
        wifiPhy.EnablePcap("Station", staDevices);
    }

//    wifiPhy.EnableAscii("wifi-phy-rx-trace", apDevice);
//    wifiPhy.EnableAscii("wifi-phy-tx-trace", staDevices);

    wifiPhy.EnableAsciiAll("wifiPhy-l8r1-25.tr");
//    wifiPhy.EnableAscii("wifi-phy-tx-trace", apDevice);

    FlowMonitorHelper flowmon;
    Ptr <FlowMonitor> monitor;
    monitor = flowmon.Install(staWifiNode);
    monitor = flowmon.Install(apWifiNode);

    /* Start Simulation */
    Simulator::Stop(Seconds(simulationTime + 1));
    Simulator::Run();

    monitor->CheckForLostPackets();
    Ptr <Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
    std::map <FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); ++i) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
        std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
        std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
        std::cout << "  Goodput: " << i->second.rxBytes * 8.0 / 5.0 / (1024 * 1024) << " Mbps\n";
//        std::cout << "Throughput: " << i->second.rxBytes * 8.0 /
//                                       (i->second.timeLastRxPacket.GetSeconds() -
//                                        i->second.timeFirstTxPacket.GetSeconds())
//                                       / 1024 << " Kbps";
        std::cout << "  Packet Loss Ratio: "
                  << (i->second.txPackets - i->second.rxPackets) * 100 / (double) i->second.txPackets << " %\n";
        //std::cout << "  Packet Dropped: " << i->second.packetsDropped  << "%\n";
        std::cout << "  mean Delay: " << i->second.delaySum.GetSeconds() * 1000 / i->second.rxPackets << " ms\n";
        std::cout << "  mean Jitter: " << i->second.jitterSum.GetSeconds() * 1000 / (i->second.rxPackets - 1)
                  << " ms\n";
        std::cout << "  mean Hop count: " << 1 + i->second.timesForwarded / (double) i->second.rxPackets << "\n";
    }

    double averageThroughput = ((sink->GetTotalRx() * 8) / (1e6 * simulationTime));

    Simulator::Destroy();

//    if (averageThroughput < 50) {
//        NS_LOG_ERROR("Obtained throughput is not in the expected boundaries!");
//        exit(1);
//    }
    std::cout << "\nAverage throughput: " << averageThroughput << " Mbit/s" << std::endl;
    return 0;
}