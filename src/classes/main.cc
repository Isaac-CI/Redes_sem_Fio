#include "m_gateway-app.h"
#include "m_intermediate-gateway-app.h"
#include "m_intermediate-sensor-app.h"
#include "m_sensor-app.h"
#include "m_server-app.h"
#include "libRedes.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/applications-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/netanim-module.h"
#include "ns3/trace-helper.h"


#define AP_ADDRESS "10.1.1.0"

using namespace ns3;

int main(void)
{
    LogComponentEnable("ServerApp", LOG_LEVEL_ALL);
    LogComponentEnable("SensorApp", LOG_LEVEL_ALL);
    LogComponentEnable("ISS", LOG_LEVEL_ALL);
    LogComponentEnable("IGS", LOG_LEVEL_ALL);
    LogComponentEnable("GatewayApp", LOG_LEVEL_ALL);

    LibRedes handler = LibRedes();

    handler.loadFile();

    NodeContainer sensorNodes;
    sensorNodes.Create(6);

    NodeContainer intermediateNodes;
    intermediateNodes.Create(2);

    NodeContainer serverNode;
    serverNode.Create(1);

    NodeContainer gatewayNode;
    gatewayNode.Create(1);

    //Create WIFI helper
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);

    //Create WIFI helpers for layers 1 and 2
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    //Create WIFI helpers for MAC addressing
    WifiMacHelper mac;
    Ssid ssid = Ssid("ns-3-ssid");

    //Create a WIFI device container for the structured network
    NetDeviceContainer sensorDevices, serverDevice, gatewayDevice, intermediateDevices;
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    sensorDevices = wifi.Install(phy, mac, sensorNodes);
    intermediateDevices = wifi.Install(phy, mac, intermediateNodes);
    serverDevice = wifi.Install(phy, mac, serverNode);
    gatewayDevice = wifi.Install(phy, mac, gatewayNode);


    // ----------------------- NODE MOBILITY SECTION ------------------------------------------

    MobilityHelper sensorMobility, intermediateMobility, serverMobility, gatewayMobility;

    Ptr<ConstantPositionMobilityModel> gatewayMobilityModel = CreateObject<ConstantPositionMobilityModel>();
    double xCoord = 0.0;
    double yCoord = 0.0;
    double zCoord = 0.0;
    gatewayMobilityModel->SetPosition(ns3::Vector(xCoord, yCoord, zCoord));
    gatewayNode.Get(0)->AggregateObject(gatewayMobilityModel);

    Ptr<ConstantPositionMobilityModel> serverMobilityModel = CreateObject<ConstantPositionMobilityModel>();
    xCoord = 10.0;
    yCoord = 10.0;
    serverMobilityModel->SetPosition(ns3::Vector(xCoord, yCoord, zCoord));
    serverNode.Get(0)->AggregateObject(serverMobilityModel);

    Ptr<ConstantPositionMobilityModel> gatewayServerIntermediateMobilityModel = CreateObject<ConstantPositionMobilityModel>();
    xCoord = 5.0;
    yCoord = 5.0;
    gatewayServerIntermediateMobilityModel->SetPosition(ns3::Vector(xCoord, yCoord, zCoord));
    intermediateNodes.Get(0)->AggregateObject(gatewayServerIntermediateMobilityModel);

    Ptr<ConstantPositionMobilityModel> serverShelfIntermediateMobilityModel = CreateObject<ConstantPositionMobilityModel>();
    xCoord = 15.0;
    yCoord = 10.0;
    serverShelfIntermediateMobilityModel->SetPosition(ns3::Vector(xCoord, yCoord, zCoord));
    intermediateNodes.Get(1)->AggregateObject(serverShelfIntermediateMobilityModel);

    std::vector<Ptr<ConstantPositionMobilityModel>> sensorMobilityModels;
    xCoord = 17.5;
    yCoord = 12.5;
    uint shelfGroup = 1;

    for(uint idx = 0; idx < sensorNodes.GetN(); idx++){
        Ptr<ConstantPositionMobilityModel> sensorMobilityModel = CreateObject<ConstantPositionMobilityModel>();
        sensorMobilityModels.push_back(sensorMobilityModel);
        sensorMobilityModels[idx]->SetPosition(Vector(xCoord, yCoord, zCoord));
        sensorNodes.Get(idx)->AggregateObject(sensorMobilityModels[idx]);

        zCoord = shelfGroup%2==0 ? 0.0 : zCoord + 1.0 ;
        yCoord = shelfGroup%2==0 ? yCoord : yCoord - 2.5;
        shelfGroup = shelfGroup%2==0 ? 1 : shelfGroup + 1 ;
    }


    //Install Internet Stacks on each node
    InternetStackHelper stack;
    stack.Install(serverNode);
    stack.Install(intermediateNodes);
    stack.Install(gatewayNode);
    stack.Install(sensorNodes);

    //Create an Address Helper

    Ipv4AddressHelper address;
    address.SetBase(AP_ADDRESS, "255.255.255.0");
    
    Ipv4InterfaceContainer sensorInterfaces = address.Assign(sensorDevices);
    Ipv4InterfaceContainer intermediateInterfaces = address.Assign(intermediateDevices);
    Ipv4InterfaceContainer serverInterface = address.Assign(serverDevice);
    Ipv4InterfaceContainer gatewayInterface = address.Assign(gatewayDevice);

    // Aplicação
    Packet::EnablePrinting();

    //Instala em cada nó sua devida aplicação
    ApplicationContainer apps;
    // Instala SensorApp em cada nó sensor
    for(uint32_t i = 0; i < sensorNodes.GetN(); i++) {
        Ptr<SensorApp> udp_sensor = Create<SensorApp> (sensorInterfaces.GetAddress(i),
                                                intermediateInterfaces.GetAddress(1),
                                                i + 1, handler);
        sensorNodes.Get(i)->AddApplication(udp_sensor);
        apps.Add(udp_sensor);
    }

    Ptr<ServerApp> server_app = Create<ServerApp>(serverInterface.GetAddress(0),
                                                intermediateInterfaces.GetAddress(0),
                                                intermediateInterfaces.GetAddress(1),
                                                10, handler);
    serverNode.Get(0)->AddApplication(server_app);
    apps.Add(server_app);

    Ptr<IGS> IGS_app = Create<IGS>(intermediateInterfaces.GetAddress(0),
                                    serverInterface.GetAddress(0),
                                    gatewayInterface.GetAddress(0),
                                    11, handler);
    intermediateNodes.Get(0)->AddApplication(IGS_app);
    apps.Add(IGS_app);


    Ptr<ISS> ISS_app = Create<ISS>(intermediateInterfaces.GetAddress(1),
                                    serverInterface.GetAddress(0),
                                    sensorInterfaces.GetAddress(0), 
                                    sensorInterfaces.GetAddress(1), 
                                    sensorInterfaces.GetAddress(2), 
                                    sensorInterfaces.GetAddress(3), 
                                    sensorInterfaces.GetAddress(4), 
                                    sensorInterfaces.GetAddress(5),
                                    12);
    intermediateNodes.Get(1)->AddApplication(ISS_app);
    apps.Add(ISS_app);

    Ptr<GatewayApp> gateway_app = Create<GatewayApp>(gatewayInterface.GetAddress(0),
                                                    intermediateInterfaces.GetAddress(0),     
                                                    13, handler);
    gatewayNode.Get(0)->AddApplication(gateway_app);
    apps.Add(gateway_app);

    
    Ptr<ServerApp> udp_server = DynamicCast <ServerApp> (apps.Get(6));
    Ptr<GatewayApp> udp_gateway = DynamicCast <GatewayApp> (apps.Get(9));
    uint8_t* buffer = (uint8_t*)malloc(sizeof(LibRedes::messageData));
    uint8_t* buffer2 = (uint8_t*)malloc(sizeof(LibRedes::messageData));
    for(uint8_t i = 0; i < 10; i++){
        buffer[0] = 10; // Server
        buffer[1] = 0; // Broadcast
        buffer[2] = 0; // Verifica status dos sensores
        buffer[3] = 0; // não importa, fica em 0
        Ptr<Packet> packet1 = Create<Packet>(buffer, sizeof(LibRedes::messageData));
        Simulator::Schedule(Seconds(i + 2), &ServerApp::SendPacket, udp_server, packet1, intermediateInterfaces.GetAddress(1), 5500);

        buffer2[0] = 13; // Gateway
        buffer2[1] = 10; // Server
        buffer2[2] = handler.gateway_commands[i]; // Verifica status dos sensores
        buffer2[3] = handler.gateway_target[i]; 
        Ptr<Packet> packet2 = Create<Packet>(buffer2, sizeof(LibRedes::messageData));
        Simulator::Schedule(Seconds(i + 1.5), &GatewayApp::SendPacket, udp_gateway, packet2, intermediateInterfaces.GetAddress(0), 5500);
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(12.0));
    
    Simulator::Stop(Seconds(15.0));


    ns3::AnimationInterface anim("animation.xml");
    anim.EnableIpv4RouteTracking("routing.xml", Seconds(0.0), Seconds(12.0), Seconds(1.0));

    for (uint32_t i = 0; i < sensorNodes.GetN(); ++i){
        anim.UpdateNodeDescription(sensorNodes.Get(i), "Sensor " + std::to_string(i));
        anim.UpdateNodeColor(sensorNodes.Get(i), 0, 255, 0);
        anim.UpdateNodeSize(sensorNodes.Get(i), 0.5, 0.5);
    }

    xCoord = 17.5;
    yCoord = 12.5;
    shelfGroup = 1;

    for(uint idx = 0; idx < sensorNodes.GetN(); idx++){
        anim.SetConstantPosition(sensorNodes.Get(idx), xCoord, yCoord, zCoord);
        zCoord = shelfGroup%2==0 ? 0.0 : zCoord + 1.0 ;
        yCoord = shelfGroup%2!=0 ? yCoord : yCoord - 2.5;
        shelfGroup = shelfGroup%2==0 ? 1 : shelfGroup + 1 ;
    }

    for (uint32_t i = 0; i < intermediateNodes.GetN(); ++i){
        anim.UpdateNodeDescription(intermediateNodes.Get(i), "Intermediate " + std::to_string(i));
        anim.UpdateNodeColor(intermediateNodes.Get(i), 255, 165, 0);
        anim.UpdateNodeSize(intermediateNodes.Get(i), 1.0, 1.0);
    }
    anim.SetConstantPosition(intermediateNodes.Get(0), 5.0, 5.0, 0.0);
    anim.SetConstantPosition(intermediateNodes.Get(1), 15.0, 10.0, 0.0);

    anim.UpdateNodeDescription(serverNode.Get(0), "Server");
    anim.UpdateNodeColor(serverNode.Get(0), 255, 0, 0);
    anim.UpdateNodeSize(serverNode.Get(0), 2.0, 2.0);
    anim.SetConstantPosition(serverNode.Get(0), 10.0, 10.0, 0.0);

    anim.UpdateNodeDescription(gatewayNode.Get(0), "Gateway");
    anim.UpdateNodeColor(gatewayNode.Get(0), 0, 0, 255);
    anim.UpdateNodeSize(gatewayNode.Get(0), 1.5, 1.5);
    anim.SetConstantPosition(gatewayNode.Get(0), 0.0, 0.0, 0.0);

    ns3::Simulator::Run();
    ns3::Simulator::Destroy();

    return 0;
}