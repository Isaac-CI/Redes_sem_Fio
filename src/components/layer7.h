#ifndef NS3_UDP_ARQ_APPLICATION_H
#define NS3_UDP_ARQ_APPLICATION_H
#include "ns3/socket.h"
#include "libRedes.h"
#include "ns3/application.h"

using namespace ns3;

namespace ns3
{
    class Layer7 : public Application, public LibRedes
    {
        public:
            Layer7 ();
            Layer7 (Ipv4Address, Ipv4Address, Ipv4Address, Ipv4Address, Ipv4Address, Ipv4Address, 
                    Ipv4Address, Ipv4Address, Ipv4Address, Ipv4Address, int, LibRedes);
            Layer7 (Ipv4Address, Ipv4Address, Ipv4Address, Ipv4Address, Ipv4Address, Ipv4Address, 
                    Ipv4Address, Ipv4Address, Ipv4Address, Ipv4Address, int, LibRedes, std::vector<bool>);
            virtual ~Layer7 ();

            static TypeId GetTypeId ();
            virtual TypeId GetInstanceTypeId () const;
            typedef struct{
                uint8_t source;
                uint8_t dest;
                uint8_t command;
                uint8_t payload;
            } messageData;

            void SensorCallback(Ptr<Socket>);
            void ServerCallback(Ptr<Socket>);
            void GatewayCallback(Ptr<Socket>);
            void IGSCallback(Ptr<Socket>);
            void ITSCallback(Ptr<Socket>);

            std::vector<bool> server_state_table;
            bool sensor_state;

            void SendPacket (Ptr<Packet> packet, Ipv4Address destination, uint16_t port);

        private:
        
            void SetupReceiveSocket (Ptr<Socket> socket, Ipv4Address addr);
            virtual void StartApplication ();
            
            ns3::Callback<void, Ptr<Socket>> cb;
            int id; // identificador do nó. Caso seja sensor, vai de 1 até 6. Caso seja servidor, id = 10. Caso seja intermediário entre servidor e gateway, é 11. Se for o intermediário entre servidor e sensores é 12 e por fim, caso seja o gateway, id é 13.
            Ipv4Address addrITS; // endereço do nó intermediário entre servidor e sensores
            Ipv4Address addrIGS; // endereço do nó intermediário entre servidor e gateway
            Ipv4Address addrGateway; // endereço do nó gateway
            Ipv4Address addrSensors[6]; // Array de endereços dos sensores
            Ipv4Address addrServer;  //Endereço do servidor
            Ptr<Socket> receiver_socket; /**< A socket to receive data */
            Ptr<Socket> sender_socket; /**< A socket to listen on a specific port */
    };
}

#endif