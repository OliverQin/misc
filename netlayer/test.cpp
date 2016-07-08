#include "packet_parser.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

int main() {
    cout << sizeof(EthernetHead) << endl;
    cout << sizeof(Ip4Head) << endl;
    cout << sizeof(TcpHead) << endl;
    
    Ip4Head iphd;
   
    iphd.totLen = 60;
    iphd.id = 41777;
    iphd.srcAddr = 0xc0a810df;
    iphd.dstAddr = 0x7ae45ffa;
    
    iphd.HostNetworkConvert();
    iphd.computeChecksum();
    //iphd.HostNetworkConvert();
    //cout << iphd.checksum << endl;
    
    for (int i = 0; i < sizeof(Ip4Head); ++ i) {
        uint8_t c = (reinterpret_cast<uint8_t*>(&iphd))[i];
        cout << setw(3) << hex << int(c);
        if ((i & 0xf) == 0xf) cout << endl;
    }
    cout << endl;
    
    TcpHead tcp;
       
    tcp.srcPort = 23734;
    tcp.dstPort = 80;
    tcp.synNo = 0x0e2806c6;
    tcp.ackNo = 0x0;
    tcp.flags = 0xa002;
    tcp.windowSize = 29200;
    
    uint8_t restData[20] = {
        0x02, 0x04, 0x05, 0xb4, 
        0x04, 0x02, 0x08, 0x0a, 
        0x00, 0x5d, 0xa5, 0xf7, 
        0x00, 0x00, 0x00, 0x00, 
        0x01, 0x03, 0x03, 0x07};
    
    tcp.HostNetworkConvert();
    cout << "conv done" << endl;
    tcp.computeChecksum(iphd, restData, 20);
    cout << "chs done" << endl;
    
    for (int i = 0; i < sizeof(tcp); ++ i) {
        uint8_t c = (reinterpret_cast<uint8_t*>(&tcp))[i];
        cout << setw(3) << hex << int(c);
        if ((i & 0xf) == 0xf) cout << endl;
    }
    cout << endl;
    
    return 0;
}
