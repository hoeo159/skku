#include <iostream>
#include <string>
#include <map>
#include <vector>

using namespace std;

#define MAX_SIZE 65536
#define MAX_LEN 4000

// tcp flag
#define RESERV 0xF00
#define CWR 0x80
#define ECE 0x40
#define URG 0x20
#define ACK 0x10
#define PSH 0x8
#define RST 0x4
#define SYN 0x2
#define FIN 0x1

u_int packetCount = 0;
u_int maxTcpPayload = 0;
u_int maxUdpPayload = 0;

u_int headerIPv6[] = {0, 43, 44, 50, 51, 60, 135};
u_int appSegmentNum[] = {21, 22, 23, 25, 53, 80, 443, 5353};
string appSegmentName[] = {"FTP", "SSH", "TELNET", "SMTP", "DNS", "HTTP", "HTTPS", "MDNS"};

class Packet
{
private:
    int32_t actlen;
    int32_t caplen;
    size_t psec;
    size_t pmsec;

    int32_t SrcMac[6];
    int32_t DstMac[6];
    int32_t type; // 0x0800 ipv4, 0x0806 ARP, 0x86DD ipv6, 0x9000 Loopback
    int32_t SrcIP[8];
    int32_t DstIP[8];

    int32_t version;
    int32_t trafficClass;
    int32_t flowLabel;
    int32_t payloadLength;
    int32_t nextHeader; // 1 : ICMPv4, 4 : IPv4, 6 : TCP, 17 : UDP, 41 : IPv6 58 : ICMPv6
    int32_t hopLimit;

    void printIP();
public:
    Packet();
    int32_t getHeader(u_char*);
    int32_t strippingPacket(u_char*, int32_t, FILE*);
    int32_t getPayloadLength();
    void skipParsing(u_char*, FILE*);
    void printInfo();
};

class UdpPacket
{
private:
    int32_t SrcPort;
    int32_t DstPort;
    int32_t length;
    int32_t payloadLength;
    int32_t checkSum;
public:
    UdpPacket();
    void parsingUDP(u_char*, int32_t, FILE*);
    void printInfo();
};

class TcpPacket
{
private:
    int32_t SrcPort;
    int32_t DstPort;
    int32_t seqNum;
    int32_t ackNum;
    int32_t headerLength;
    int32_t flag;
    int32_t wndSize;
    int32_t checkSum;
    int32_t payloadLength;
    vector<string> optList;
    
    void printSegType();
public:
    TcpPacket();
    void parsingTcp(u_char*, int32_t, FILE*);
    void printInfo();
};

UdpPacket::UdpPacket()
    :SrcPort(0), DstPort(0), length(0), payloadLength(0), checkSum(0) 
    {}

TcpPacket::TcpPacket()
    :SrcPort(0), DstPort(0), seqNum(0), ackNum(0), headerLength(0), flag(0),
    wndSize(0), checkSum(0), payloadLength(0)
    {}

Packet::Packet()
    :actlen(0), caplen(0), type(0), psec(0), pmsec(0),
    flowLabel(0), trafficClass(0), payloadLength(0), nextHeader(0), hopLimit(0)
    {}

int32_t Packet::getPayloadLength(){
    return payloadLength;
}

int32_t Packet::getHeader(u_char *buff)
{   
    for(int i = 15; i >= 12; i--){
        actlen += (buff[i] << (i * 8));
    }
    for(int i = 11; i >= 8; i--){
        caplen += (buff[i] << (i * 8));
    }
    for(int i = 7; i >= 4; i--){
        pmsec += (buff[i] << (i * 8));
    }
    for(int i = 3; i >= 0; i--){
        psec += (buff[i] << (i * 8));
    }

    if(actlen != caplen){
        exit(-1);
    }

    return caplen;
}

int32_t Packet::strippingPacket(u_char* buff, int32_t size, FILE* fp)
{
    fread(buff, 1, 6, fp); // dst MAC
    for(int i = 0; i < 6; i++){
        DstMac[i] = buff[i];
    }
    fread(buff, 1, 6, fp); // src MAC
    for(int i = 0; i < 6; i++){
        SrcMac[i] = buff[i];
    }
    fread(buff, 1, 2, fp); // type
    for(int i = 0; i < 2; i++){
        type += (buff[i] << (1 - i) * 8);
    }

    if(type == 0x86dd){

    }
    else{
        fseek(fp, size - 14, SEEK_CUR);
        return 0;
    }

    fread(buff, 1, 4, fp);  
    version = buff[0] >> 4;

    int32_t tmp = 0;

    for(int i = 0; i < 4; i++){
        tmp += (buff[i] << (3 - i) * 8);
    }

    trafficClass = (tmp & 0xFF00000) >> 20;
    flowLabel = (tmp & 0xFFFFF);

    fread(buff, 1, 2, fp);
    for(int i = 0; i < 2; i++){
        payloadLength += (buff[i] << (1 - i) * 8);
    }

    fread(buff, 1, 2, fp);
    nextHeader = buff[0];
    hopLimit = buff[1];

    fread(buff, 1, 16, fp);
    for(int i = 0; i < 16; i += 2){
        int32_t tmp = 0;
        tmp = ((buff[i] << 8) + buff[i + 1]);
        SrcIP[i / 2] = tmp;
    }

    fread(buff, 1, 16, fp);
    for(int i = 0; i < 16; i += 2){
        int32_t tmp = 0;
        tmp = ((buff[i] << 8) + buff[i + 1]);
        DstIP[i / 2] = tmp;
    }

    return nextHeader;
}

void Packet::skipParsing(u_char* buff, FILE* fp){
    fseek(fp, payloadLength, SEEK_CUR);
    return;
}

void Packet::printInfo()
{
    // Korea time
    cout << "epoch time : " << (psec / 3600 + 9) % 24 << ":"
    << (psec % 3600) / 60 << ":" << (psec % 60);
    printf(".%06ld\n", pmsec);
    cout << "captured length : " << caplen << " byte(s)\n";
    cout << "actual length : " << actlen << " byte(s)\n";
    cout << "IPv6 header length : " << 40 << " byte(s)\n";
    printIP();
    printf("Traffic class : 0x%02x\n", trafficClass);
    printf("Flow label : 0x%05x\n", flowLabel);
    cout << "Payload length : " << payloadLength << " byte(s)\n";
    return;
}

void Packet::printIP(){
    cout << "Source      IP : ";
    for(int i = 0; i < 8; i++){
        printf("%04x", SrcIP[i]);
        if(i != 7)
            printf(":");
    }
    cout << endl;

    cout << "Destination IP : ";
    for(int i = 0; i < 8; i++){
        printf("%04x", DstIP[i]);
        if(i != 7)
            printf(":");
    }
    cout << endl;
    return;
}

void UdpPacket::parsingUDP(u_char* buff, int32_t size, FILE* fp){
    fread(buff, 1, 2, fp);
    for(int i = 0; i < 2; i++){
        SrcPort += (buff[i] << (1 - i) * 8);
    }
    fread(buff, 1, 2, fp);
    for(int i = 0; i < 2; i++){
        DstPort += (buff[i] << (1 - i) * 8);
    }
    fread(buff, 1, 2, fp);
    for(int i = 0; i < 2; i++){
        length += (buff[i] << (1 - i) * 8);
    }
    fread(buff, 1, 2, fp);
    for(int i = 0; i < 2; i++){
        checkSum += (buff[i] << (1 - i) * 8);
    }
    payloadLength = length - 8;

    if(maxUdpPayload < payloadLength){
        maxUdpPayload = payloadLength;
    }

    fseek(fp, payloadLength , SEEK_CUR);
    return;
}

void TcpPacket::parsingTcp(u_char* buff, int32_t size, FILE* fp){
    fread(buff, 1, 2, fp);
    for(int i = 0; i < 2; i++){
        SrcPort += (buff[i] << (1 - i) * 8);
    }
    fread(buff, 1, 2, fp);
    for(int i = 0; i < 2; i++){
        DstPort += (buff[i] << (1 - i) * 8);
    }
    fread(buff, 1, 4, fp);
    for(int i = 0; i < 4; i++){
        seqNum += (buff[i] << (3 - i) * 8);
    }
    fread(buff, 1, 4, fp);
    for(int i = 0; i < 4; i++){
        ackNum += (buff[i] << (3 - i) * 8);
    }
    fread(buff, 1, 2, fp);
    headerLength = (buff[0] >> 4) * 4;

    int optflag = 0;
    if(headerLength > 20)
        optflag = 1;

    for(int i = 0; i < 2; i++){
        flag += (buff[i] << (1 - i) * 8);
    }
    fread(buff, 1, 2, fp);
    for(int i = 0; i < 2; i++){
        wndSize += (buff[i] << (1 - i) * 8);
    }
    fread(buff, 1, 2, fp);
    for(int i = 0; i < 2; i++){
        checkSum += (buff[i] << (1 - i) * 8);
    }
    fread(buff, 1, 2, fp); // urgent pointer

    int optlen = headerLength - 20;
    fread(buff, 1, optlen, fp);
    for(int idx = 0; idx < optlen; idx++){
        int kind = buff[idx];
        if(kind == 1) // No Op
            continue;
        else{
            idx++;
            int l = buff[idx];
            idx += (l - 2);
            if(kind == 2){ // Maximum Segment Size
                optList.push_back("Maximum Segment Size");
            }
            if(kind == 3){ // Window Scale
                optList.push_back("Window Scale");
            }
            if(kind == 4){ // SACK-permit
                optList.push_back("SACK-permit");
            }
            if(kind == 5){ // SACK
                optList.push_back("SACK");
            }
            if(kind == 8){ // timestamp
                optList.push_back("timestamp");
            }
        }
    }

    payloadLength = size - headerLength;

    if(maxTcpPayload < payloadLength){
        maxTcpPayload = payloadLength;
    }

    fseek(fp, payloadLength, SEEK_CUR);
    return;
}

void UdpPacket::printInfo(){
    cout << "(Udp Info)\n";
    cout << "Source      Port : " << SrcPort << "\n";
    cout << "Destination Port : " << DstPort << "\n";
    cout << "Payload Length : " << payloadLength << " byte(s)\n";
    for(int i = 0; i < sizeof(appSegmentNum) / sizeof(appSegmentNum[0]); i++){
        if(appSegmentNum[i] == SrcPort || appSegmentNum[i] == DstPort){
            cout << "Application Type : " << appSegmentName[i] << endl;
            break;
        }
    }
    return;
}

void TcpPacket::printSegType(){
    cout << "Segment Type : ";
    if(flag & URG)
        cout << "U ";
    if(flag & ACK)
        cout << "A ";
    if(flag & PSH)
        cout << "P ";
    if(flag & RST)
        cout << "R ";
    if(flag & SYN)
        cout << "S ";
    if(flag & FIN)
        cout << "F ";
    cout << endl;
    return;
}

void TcpPacket::printInfo(){
    cout << "(Tcp Info)\n";
    cout << "Source      Port : " << SrcPort << "\n";
    cout << "Destination Port : " << DstPort << "\n";
    cout << "Acknowledgement number : " << ackNum << "\n";
    cout << "Payload Length : "  << payloadLength << " byte(s)\n";
    if(payloadLength > 0){
        cout << "Starting Sequence Number : " << seqNum << "\n";
        cout << "Ending Sequence Number : " << seqNum + payloadLength << "\n";
    }
    cout << "Advertising Window Size : " << wndSize << "\n";
    printSegType();
    if(!optList.empty()){
        cout << "Tcp Option List" << endl;
        for(auto i : optList)
            cout << "\t" << i << "\n";
    }
    for(int i = 0; i < sizeof(appSegmentNum) / sizeof(appSegmentNum[0]); i++){
        if(appSegmentNum[i] == SrcPort || appSegmentNum[i] == DstPort){
            cout << "Application Type : " << appSegmentName[i] << endl;
            break;
        }
    }
}

int main(int argc, char* argv[]){
    if(argc < 2){
        cout << "input pcap file name!\n";
        exit(-1);
    }
    string filename = argv[1];
    FILE *fp = fopen(filename.c_str(), "r");
    u_char pbuff[MAX_SIZE] = {0, };
    
    if(fp == NULL){
        perror("fopen error ");
        exit(-1);
    }

    fseek(fp, 24, SEEK_CUR);

    while(true){
        Packet *packet = new Packet;
        int32_t len = 0, paylen = 0;
        int32_t flag = 0;
        
        size_t readSize = fread(pbuff, 1, 16, fp);
        if(readSize == 0){
            break;
        }

        packetCount++;
        len = packet->getHeader(pbuff);
        flag = packet->strippingPacket(pbuff, len, fp);
        paylen = packet->getPayloadLength();

        if(flag){
            printf("\n### frame number %d ###\n", packetCount);
            packet->printInfo();
        }

        if(flag == 0x11){
            UdpPacket *udppacket = new UdpPacket;
            udppacket->parsingUDP(pbuff, paylen, fp);
            udppacket->printInfo();
        }
        else if(flag == 0x6){
            TcpPacket *tcpPacket = new TcpPacket;
            tcpPacket->parsingTcp(pbuff, paylen, fp);
            tcpPacket->printInfo();
        }
        else{
            packet->skipParsing(pbuff, fp);
        }

        // if(packetCount > 14)
        //     break;
    }

    fclose(fp);

    printf("\nMax TCP payload size : %d\n", maxTcpPayload);
    printf("Max UDP payload size : %d\n", maxUdpPayload);

    return 0;
}