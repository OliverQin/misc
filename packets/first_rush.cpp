#include <cstdlib>
#include <iostream>
#include <string>
#include <cstdio>
#include <random>
#include <ctime>
#include <unordered_set>
#include <arpa/inet.h>
#include <pcap.h>


#include "../bitshuffler/bit_shuffler.hpp"
#include "../netlayer/packet_parser.hpp"

using namespace std;

std::mt19937 rng( time(NULL) );

void convertMacToNet(const char* mac, char *net) {
    uint32_t counter = 0, value = 0;
    for (const char *i = mac; (*i)!=0; ++ i) {
        
        if ( (*i) <= 'F' && (*i) >= 'A')
            value += (*i)-'A'+10;
        
        else if ( (*i) <= 'f' && (*i) >= 'a')
            value += (*i)-'a'+10;
        
        else if ( (*i) <= '9' && (*i) >= '0')
            value += (*i)-'0';
        else
            continue;
        
        ++ counter;
        if (counter == 2) {
            *(net++) = uint8_t(value);
            value = 0;
            counter = 0;
        }
        else
            value *= 16;
        
    }
}

std::string generatePayload(const char* srcMac, const char* dstMac, uint32_t srcIp, uint32_t dstIp, uint16_t dstPort) {
    EthernetHead a;
    Ip4Head b;
    TcpHead c;
    
    convertMacToNet(dstMac, (char *)a.dstAddr);
    convertMacToNet(srcMac, (char *)a.srcAddr);
    
    b.totLen += sizeof(c);
    b.id = rng() & 0xffff;
    b.srcAddr = srcIp;
    b.dstAddr = dstIp;
        
    c.srcPort = rng() % 55379 + 1024;
    c.dstPort = dstPort;
    c.synNo = rng();
    c.flags |= c.FlagSyn;
    
    b.HostNetworkConvert();
    b.computeChecksum();
    
    c.HostNetworkConvert();
    c.computeChecksum(b);
    
    std::string sa( (char*)(&a), sizeof(a) );
    std::string sb( (char*)(&b), sizeof(b) );
    std::string sc( (char*)(&c), sizeof(c) );
    
    return sa+sb+sc;
}

struct Combined {
    u_char data[54];
    /*EthernetHead ether;
    Ip4Head ip4;
    TcpHead tcp;*/
};

int main(int argc, char **argv)
{
    unordered_set<uint32_t> glbset;
    
    //cout << sizeof(Combined) << endl;
    static_assert( sizeof(Combined) == 54, "What?" );
    
    BitShuffler<24> bsmain(time(0));
    
    char *dev; 
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* descr;
    const u_char *packet;
    struct pcap_pkthdr hdr;     /* pcap.h */
    Combined *combHd;
    struct ether_header *eptr;  /* net/ethernet.h */
    const int BUFSIZE = 14+20+20;
    
    uint32_t GlobalIp, ip1, ip2, ip3, ip4;

    if (argc != 6) {
        cerr << "Usage: " << argv[0] << " if srcMac dstMac srcIp dstPort" << endl;
        return 1;
    }
    
    sscanf(argv[4], "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
    GlobalIp = (ip1<<24) | (ip2<<16) | (ip3<<8) | ip4;
    
    descr = pcap_open_live(argv[1],BUFSIZE,1, 2 /*ms*/,errbuf);

    if(descr == NULL)
    {
        printf("pcap_open_live(): %s\n",errbuf);
        exit(1);
    }

    uint32_t sentIp = 0;
    uint32_t waiting = 0;
    const uint32_t MaxSent = 0x1000000;
    uint16_t TargetPort = atoi( argv[5] );
    
    while (sentIp < MaxSent) {
        for (uint32_t i = 0; i < 5000 && sentIp < MaxSent; ++ i) {
            uint32_t nd = bsmain.getNumber(sentIp++);
            std::string out = generatePayload( argv[2], argv[3], GlobalIp, (10<<24) | nd, TargetPort );
            
            pcap_sendpacket(descr, (const u_char*)out.c_str(), out.size());
        }
        cerr << "\r" << (sentIp*100) / double(MaxSent) << '%';
        
        while (1) {
            packet = pcap_next(descr,&hdr);

            if(packet == NULL)
            {/* dinna work *sob* */
                //printf("Didn't grab packet\n");
                if (sentIp < MaxSent || waiting++ <= 2500)
                    break;
            }
            
            else if (hdr.len >= sizeof(Combined)) {
                combHd = (Combined*)packet;
                
                Ip4Head ipGot = *(Ip4Head*)(combHd->data + 14);
                ipGot.HostNetworkConvert();
                TcpHead tcpGot = *(TcpHead*)(combHd->data + 34);//combHd ->tcp;
                tcpGot.HostNetworkConvert(); //host
                
                if ((ipGot.srcAddr >> 24) == 10 &&
                    tcpGot.srcPort == TargetPort && 
                    (tcpGot.flags & tcpGot.FlagSyn) && 
                    (tcpGot.flags & tcpGot.FlagAck) && 
                    ((tcpGot.flags & tcpGot.FlagRst) == 0) &&
                    glbset.count(ipGot.srcAddr) == 0) {
                    glbset.insert( ipGot.srcAddr );
                
                    cout << (ipGot.srcAddr>>24) << '.' 
                    << ((ipGot.srcAddr>>16)&0xff) << '.' 
                    <<  ((ipGot.srcAddr>>8)&0xff) << '.' 
                    <<  (ipGot.srcAddr&0xff) << ':' << TargetPort << endl;
                    
                }
                
                
            }
        }
        
        
    }
    return 0;
}
