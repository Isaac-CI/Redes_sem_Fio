#ifndef NS3_UDP_ISS_APPLICATION_H
#define NS3_UDP_ISS_APPLICATION_H
#include "ns3/socket.h"
#include "libRedes.h"
#include "ns3/application.h"

using namespace ns3;

namespace ns3
{
    class ISS : public Application, public LibRedes
    {
        public:
            ISS ();
            ISS (Ipv4Address, Ipv4Address, Ipv4Address, Ipv4Address, Ipv4Address, Ipv4Address, 
                    Ipv4Address, Ipv4Address, int);
            virtual ~ISS ();

            static TypeId GetTypeId ();
            virtual TypeId GetInstanceTypeId () const;

            void ISSCallback(Ptr<Socket>);

            void SendPacket (Ptr<Packet> packet, Ipv4Address destination, uint16_t port);

        private:
        
            void SetupReceiveSocket (Ptr<Socket> socket, Ipv4Address addr);
            virtual void StartApplication ();
            
            ns3::Callback<void, Ptr<Socket>> cb;
            int id; // identificador do nó. Caso seja sensor, vai de 1 até 6. Caso seja servidor, id = 10. Caso seja intermediário entre servidor e gateway, é 11. Se for o intermediário entre servidor e sensores é 12 e por fim, caso seja o gateway, id é 13.
            Ipv4Address addrSensors[6]; // Array de endereços dos sensores
            Ipv4Address addrServer;  //Endereço do servidor
            Ipv4Address m_addr; // endereço do nó cuja aplicação está instalada
            Ptr<Socket> receiver_socket; /**< A socket to receive data */
            Ptr<Socket> sender_socket; /**< A socket to listen on a specific port */
    };
}

#endif