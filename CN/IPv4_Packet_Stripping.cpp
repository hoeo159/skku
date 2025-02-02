#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;

u_int packetCount = 0;

#define MAX_SIZE 65536
#define MTU 4000
#define DF 2
#define MF 1

map<int, string> Protocol = {
    {1, "ICMP"},
    {2, "IGMP"},
    {3, "IP/IP"},
    {6, "TCP"},
    {17, "UDP"},
    {41, "IPv6"},
    {47, "GRE"},
    {89, "OSPF"},
    {115, "L2TP"},
};

class Packet
{
private:
    int32_t actlen;
    int32_t caplen;
    int32_t headlen;
    int32_t totallen;
    int32_t fragOffset;
    int32_t identification;

    int32_t SrcMac[6];
    int32_t DstMac[6];
    int32_t type; // 0x0800 ipv4, 0x0806 ARP, 0x86DD ipv6, 0x9000 Loopback
    int32_t SrcIP[4];
    int32_t DstIP[4];

    int32_t flag;
    int32_t TTL;
    
    string protocol;
    int32_t tos;

    size_t psec;
    size_t pmsec;

    void printMac();
    void printIP();
public:
    Packet();
    int32_t getHeader(u_char*);
    void strippingPacket(u_char*, int32_t, FILE*);
    void printInfo();
};

Packet::Packet()
    :actlen(0), caplen(0), headlen(0), totallen(0),
    fragOffset(0), identification(0), type(0), flag(0), TTL(0), psec(0), pmsec(0)    {}

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
    
    if(actlen > MTU){
        printf("error len : %d MTU : %d\n", caplen, MTU);
        exit(-1);
    }
    if(actlen < caplen){
        exit(-1);
    }

    return caplen;
}

void Packet::strippingPacket(u_char* buff, int32_t size, FILE* fp)
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
    fread(buff, 1, 1, fp);  // version, header length
    headlen = (buff[0] & 0x0F) * 4; // header unit is word

    fread(buff, 1, 1, fp); // Type of Service
    tos = (buff[0] & 0xFC) >> 2;

    fread(buff, 1, 2, fp); // total packet length
    for(int i = 0; i < 2; i++){
        totallen += (buff[i] << (1 - i) * 8);
    }
    fread(buff, 1, 2, fp); // identifier
    for(int i = 0; i < 2; i++){
        identification += (buff[i] << (1 - i) * 8);
    }

    fread(buff, 1, 2, fp); // flag and fragment offset
    int fragment = 0;
    for(int i = 0; i < 2; i++){
        fragment += (buff[i] << (1 - i) * 8);
    }
    fragOffset = fragment & 0x1FFF;
    flag = (fragment & 0xE000) >> 13;

    fread(buff, 1, 1, fp);
    TTL = buff[0];
    fread(buff, 1, 1, fp);
    int protocolid = buff[0];
    protocol = Protocol[protocolid];

    fread(buff, 1, 2, fp);
    int checksum  = 0;
    for(int i = 0; i < 2; i++){
        checksum += (buff[i] << (1 - i) * 8);
    }

    fread(buff, 1, 4, fp);
    for(int i = 0; i < 4; i++){
        SrcIP[i] = buff[i];
    }
    fread(buff, 1, 4, fp);
    for(int i = 0; i < 4; i++){
        DstIP[i] = buff[i];
    }

    fseek(fp, -34, SEEK_CUR);
    // Skip protocol details for IP payload
    fseek(fp, size, SEEK_CUR);

    printf("\n");
    return;
}

void Packet::printMac(){
    cout << "Destination Mac : ";
    for(int i = 0; i < 6; i++){
        printf("%02x", DstMac[i]);
        if(i != 5)
            printf(":");
    }
    cout << endl;
    
    cout << "Source Mac : ";
    for(int i = 0; i < 6; i++){
        printf("%02x", SrcMac[i]);
        if(i != 5)
            printf(":");
    }
    cout << endl;
    return;
}

void Packet::printIP(){
    cout << "Destination IP : ";
    for(int i = 0; i < 4; i++){
        printf("%d", DstIP[i]);
        if(i != 3)
            printf(".");
    }
    cout << endl;

    cout << "Source IP : ";
    for(int i = 0; i < 4; i++){
        printf("%d", SrcIP[i]);
        if(i != 3)
            printf(".");
    }
    cout << endl;
    return;
}

void Packet::printInfo()
{
    // Korean time
    cout << "epoch time : " << (psec / 3600 + 9) % 24 << ":"
    << (psec % 3600) / 60 << ":" << (psec % 60);
    printf(".%06ld\n", pmsec);
    cout << "captured length : " << caplen << " byte(s)\n";
    cout << "actual length : " << actlen << " byte(s)\n";
    cout << "IP header length : " << headlen << " bytes(s)\n\n";
    printMac();
    printIP();
    printf("\n");
    cout << "Protocol : " << protocol << endl;
    cout << "Identification : " << identification << endl;
    cout << "DF : " << (static_cast<bool>(flag & DF) ? "Set" : "Unset") << endl;
    cout << "MF : " << (static_cast<bool>(flag & MF) ? "Set" : "Unset") << endl;
    cout << "TTL(Time to Live) : " << TTL << endl;
    cout << "Type of Service : " << tos << endl;
    return;
}

vector<Packet*> p;

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
        int32_t len = 0;
        
        size_t readSize = fread(pbuff, 1, 16, fp);
        if(readSize == 0){
            break;
        }

        printf("### frame number %d ###\n", ++packetCount);

        len = packet->getHeader(pbuff);
        packet->strippingPacket(pbuff, len, fp);
        packet->printInfo();
        
        p.push_back(packet);

        cout << "\n";
    }

    fclose(fp);
    return 0;
}