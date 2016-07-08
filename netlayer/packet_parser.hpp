#pragma once

#include <cstdint>
#include <string>
//#include <iostream>
//#include <iomanip>
#include <arpa/inet.h>

/*uint16_t ntohs(uint16_t a) {
    return (a<<8) ^ (a>>8);
}

uint16_t htons(uint16_t a) {
    return ntohs(a);
}

uint32_t ntohl(uint32_t a) {
    a = (a<<16) ^ (a>>16);
    a = ((a&0x00FF00FF) << 8) ^ ((a&0xFF00FF00) >> 8);
    return a;
}

uint32_t htonl(uint32_t a) {
    return ntohl(a);
}*/

struct EthernetHead {
    uint8_t  dstAddr[6];
    uint8_t  srcAddr[6];
    uint16_t type;
};

struct Ip4Head {
    uint8_t  ver;  // 4 bits version and 4 bits internet header length, 4-byte
    uint8_t  dsf; //Differentiated Services Field
    uint16_t totLen; //len of Ip4Head and its load, byte
    uint16_t id; //random
    uint16_t flags; // 3 bits flags and 13 bits fragment-offset
    uint8_t  ttl;
    uint8_t  protocol; //TCP=6
    uint16_t checksum;
    uint32_t srcAddr;
    uint32_t dstAddr;
    
    Ip4Head() {
        ver = 0x45; //ipv4, length=5
        dsf = 0x00;
        totLen = sizeof(Ip4Head);
        id = 0x0000; //TODO: should be random
        flags = 0x4000; //no fragment
        ttl = 64;
        protocol = 6; //TCP
        checksum = 0;
        srcAddr = 0;
        dstAddr = 0;
    }
    
    void HostNetworkConvert() {
        totLen = ntohs(totLen);
        id = ntohs(id);
        flags = ntohs(flags);
        checksum = ntohs(checksum);
        
        srcAddr = ntohl(srcAddr);
        dstAddr = ntohl(dstAddr);
    }
    
    void computeChecksum() {
        checksum = 0;
        
        uint32_t res = 0;
        for (int i = 0; i < (sizeof(Ip4Head) >> 1); ++ i) {
            uint16_t chunk = ntohs( ( reinterpret_cast<uint16_t * const>(this))[i] );
            res += static_cast<uint32_t>(chunk);
        }
        res = (res >> 16) + (res & 0xffff);
        res = (res >> 16) + (res & 0xffff);
        
        checksum = htons( static_cast<uint16_t>(0xffff ^ res) );
    }
};

struct TcpHead {
    uint16_t    srcPort;
    uint16_t    dstPort;
    uint32_t    synNo;
    uint32_t    ackNo;
    uint16_t    flags; //4-bit length of header(4byte), 12bit flag
    uint16_t    windowSize;
    uint16_t    checksum;
    uint16_t    urgentPointer;
    
    static const uint16_t FlagFin = 0x01;
    static const uint16_t FlagSyn = 0x02;
    static const uint16_t FlagRst = 0x04;
    static const uint16_t FlagPsh = 0x08;
    static const uint16_t FlagAck = 0x10;
    static const uint16_t FlagUrg = 0x20;
    
    TcpHead() {
        srcPort = 0;
        dstPort = 0;
        synNo = 0; //TODO: random
        ackNo = 0;
        flags = (sizeof(TcpHead)/4) << 4;
        windowSize = 14600;
        checksum = 0;
        urgentPointer = 0;
    }
    
    void HostNetworkConvert() {
        srcPort = ntohs(srcPort);
        dstPort = ntohs(dstPort);
        flags = ntohs(flags);
        windowSize = ntohs(windowSize);
        checksum = ntohs(checksum);
        urgentPointer = ntohs(urgentPointer);
        
        synNo = ntohl(synNo);
        ackNo = ntohl(ackNo);
    }
    
    void computeChecksum(const Ip4Head& ip, const uint8_t *rdata = nullptr, const uint16_t dataLen = 0) {
        checksum = 0;
        
        /*uint8_t kbuffers[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
        *(uint32_t *)(kbuffers + 0) = ip.srcAddr;
        *(uint32_t *)(kbuffers + 4) = ip.dstAddr;
        kbuffers[8] = 0;
        kbuffers[9] = ip.protocol;
        *(uint16_t *)(kbuffers + 10) = htons( uint16_t( sizeof(TcpHead) + dataLen ) );*/
        
        uint32_t res = 0;
        res += ntohs(ip.srcAddr >> 16);
        res += ntohs(ip.srcAddr & 0xffff);
        res += ntohs(ip.dstAddr >> 16);
        res += ntohs(ip.dstAddr & 0xffff);
        res += ntohs(uint16_t(ip.protocol) << 8);
        res += uint16_t(sizeof(TcpHead) + dataLen);
        
        /*using namespace std;
        for (int i = 0; i < sizeof(kbuffers); ++ i) {
            uint8_t c = (reinterpret_cast<uint8_t*>(&kbuffers))[i];
            cout << setw(3) << hex << int(c);
            if ((i & 0xf) == 0xf) cout << endl;
        }
        cout << endl;*/
        
        /*uint32_t res = 0;
        for (int i = 0; i < 12; i += 2) {
            uint16_t chunk = ntohs( *(uint16_t *)(kbuffers + i) );
            res += (uint32_t)(chunk);
        }*/
        //cout << res << endl;
        
        for (int i = 0; i < (sizeof(TcpHead) >> 1); ++ i) {
            uint16_t chunk = ntohs( ( reinterpret_cast<uint16_t *>(this))[i] );
            res += static_cast<uint32_t>(chunk);
        }
        
        for (int i = 0; i < (dataLen / 2); ++ i) {
            uint16_t chunk = ntohs( ( reinterpret_cast<const uint16_t *>(rdata))[i] );
            res += static_cast<uint32_t>(chunk);
        }
        
        res = (res >> 16) + (res & 0xffff);
        res = (res >> 16) + (res & 0xffff);
        
        checksum = htons( static_cast<uint16_t>(0xffff ^ res) );
    }
};