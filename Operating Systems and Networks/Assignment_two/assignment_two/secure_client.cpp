//////////////////////////////////////////////////////////////////////////////////////////////
// TCP CrossPlatform CLIENT v.1.0 (towards IPV6 ready)
// compiles using GCC 
//
// References: https://msdn.microsoft.com/en-us/library/windows/desktop/ms738520(v=vs.85).aspx
//             http://long.ccaba.upc.edu/long/045Guidelines/eva/ipv6.html#daytimeServer6
//             Andre Barczak's tcp client codes
//
// Author: Napoleon Reyes, Ph.D.
//         Massey University, Albany  
//////////////////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_PORT "1234" 
#define USE_IPV6 true  // if set to false, IPv4; assignment will be marked with IPv6

#if defined __unix__ || defined __APPLE__
  #include <unistd.h>
  #include <errno.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <iostream>
#elif defined __WIN32__
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <iostream>
  #define WSVERS MAKEWORD(2,2)
  WSADATA wsadata;
#endif

#include <boost/multiprecision/cpp_int.hpp>      // Big integers
#include <random>
#include <vector>
#include <cstdint>       // uint8_t
#include <sstream>

using boost::multiprecision::cpp_int;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////

//*** RSA-CBC INSERT START ***

// Fast modular
cpp_int modexp(cpp_int base, cpp_int exp, const cpp_int& mod) {
    cpp_int result = 1;
    while (exp > 0) {
        if ((exp & 1) != 0) result = (result * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

// CA public key known to client
const cpp_int CA_e = cpp_int("2113");
const cpp_int CA_n = cpp_int("190933");

// Parse decimal string → cpp_int
cpp_int parse_int(const string& s) {
    return cpp_int(s);
}
//*** RSA-CBC INSERT END ***

int main(int argc, char *argv[]) {
    //*******************************************************************
    // Initialization
    //*******************************************************************
    char portNum[12];
#if defined __unix__ || defined __APPLE__
    int s;
#elif defined _WIN32
    SOCKET s;
#endif

    #define BUFFER_SIZE 200 
    #define SEGMENT_SIZE 70

    char send_buffer[BUFFER_SIZE], receive_buffer[BUFFER_SIZE];
    int n, bytes;
    char serverHost[NI_MAXHOST], serverService[NI_MAXSERV];

#if defined _WIN32
    // WSSTARTUP
    int err = WSAStartup(WSVERS, &wsadata);
    if(err!=0){ WSACleanup(); printf("WSAStartup failed: %d\n",err); exit(1); }
    if(LOBYTE(wsadata.wVersion)!=2 || HIBYTE(wsadata.wVersion)!=2){
        printf("Winsock 2.2 not supported\n"); WSACleanup(); exit(1);
    } else printf("Winsock 2.2 initialized.\n");
#endif

    //*******************************************************************
    // Resolve server address
    //*******************************************************************
    struct addrinfo *result=nullptr, hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family   = USE_IPV6?AF_INET6:AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if(argc==3){
        sprintf(portNum,"%s",argv[2]);
        printf("Using port: %s\n",portNum);
        if(getaddrinfo(argv[1],portNum,&hints,&result)!=0){
            perror("getaddrinfo"); exit(1);
        }
    } else {
        printf("Defaulting to 127.0.0.1:%s\n",DEFAULT_PORT);
        sprintf(portNum,"%s",DEFAULT_PORT);
        if(getaddrinfo("127.0.0.1",portNum,&hints,&result)!=0){
            perror("getaddrinfo"); exit(1);
        }
    }

    //*******************************************************************
    // Create socket & connect
    //*******************************************************************
    s = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
#if defined __unix__ || defined __APPLE__
    if(s<0){ perror("socket"); freeaddrinfo(result); exit(1); }
#elif defined _WIN32
    if(s==INVALID_SOCKET){ printf("socket failed: %d\n",WSAGetLastError()); freeaddrinfo(result); WSACleanup(); exit(1); }
#endif

    if(connect(s,result->ai_addr,result->ai_addrlen)!=0){
        perror("connect"); freeaddrinfo(result);
#if defined _WIN32
        WSACleanup();
#endif
        exit(1);
    }
    freeaddrinfo(result);
    printf("Connected to server.\n");

    //*******************************************************************
    // RSA-CBC PROTOCOL START
    //*******************************************************************

    // 1) Receive encrypted cert dCA(e_srv,n_srv)
    bytes = recv(s,receive_buffer,BUFFER_SIZE,0);
    if(bytes<=0){ perror("recv cert"); exit(1); }
    receive_buffer[bytes]=0;
    cout<<"Secure Client: received certificate: "<<receive_buffer<<endl;

    // 2) Decrypt cert with CA public → server_e, server_n
    string cert_str(receive_buffer);
    istringstream iss(cert_str);
    
    // Parse
    cpp_int enc_e, enc_n;
    if (!(iss >> enc_e >> enc_n)) {
        cerr << "Error parsing certificate: '" << cert_str << "'" << endl;
        exit(1);
    }

    // Decrypte
    cpp_int server_e = modexp(enc_e, CA_e, CA_n);
    cpp_int server_n = modexp(enc_n, CA_e, CA_n);

    cout << "Secure Client: decrypted server pubkey (e,n): "
     << server_e << ", " << server_n << endl;
    // 3) Send ACK 226
    const char* ack226="ACK 226 public key received\r\n";
    send(s,ack226,strlen(ack226),0);
    cout<<"Sent: ACK 226 public key received\n";

    // 4) Generate NONCE and IV-byte
    mt19937_64 rng(random_device{}());
    uint64_t mx = (server_n.convert_to<uint64_t>()-1);
    uniform_int_distribution<uint64_t> dist(1,mx);
    cpp_int nonce = dist(rng);
    uint8_t prev_byte = (uint8_t)(nonce.convert_to<unsigned long>() & 0xFF);
    cout<<"Secure Client: NONCE = "<<nonce<<endl;
    cpp_int enc_nonce = modexp(nonce,server_e,server_n);
    string enc_nonce_str = enc_nonce.convert_to<string>() + "\r\n";
    send(s,enc_nonce_str.c_str(),enc_nonce_str.size(),0);
    cout<<"Sent encrypted NONCE\n";

    // 5) Receive ACK 220 nonce OK
    bytes = recv(s,receive_buffer,BUFFER_SIZE,0);
    if(bytes<=0){ perror("recv ACK nonce"); exit(1); }
    receive_buffer[bytes]=0;
    cout<<"Secure Client: received "<<receive_buffer<<endl;

    //*******************************************************************
    // Main loop: RSA-CBC encrypt & send (byte-wise)
    //*******************************************************************
    cout<<"\n--- You may now send messages ('.' to quit) ---\n";
    while(true){
        printf("Type here: ");
        if(!fgets(send_buffer,SEGMENT_SIZE,stdin)) break;
        if(send_buffer[0]=='.') break;
        // strip newline
        send_buffer[strlen(send_buffer)-1]=0;

        // encrypt byte-wise
        vector<cpp_int> cipher_blocks;
        for(unsigned char ch: string(send_buffer)){
            uint8_t m_byte = ch;
            uint8_t x_byte = m_byte ^ prev_byte;     // CBC XOR
            cpp_int x = cpp_int(x_byte);
            cpp_int c = modexp(x, server_e, server_n);
            cipher_blocks.push_back(c);
            prev_byte = (uint8_t)(c.convert_to<unsigned long>() & 0xFF);
        }
        // build output
        string out;
        for(auto &c: cipher_blocks){
            out += c.convert_to<string>() + " ";
        }
        out += "\r\n";

        bytes = send(s,out.c_str(),out.size(),0);
        if(bytes<=0){ perror("send encrypted"); exit(1); }
        cout<<"Secure Client: sent encrypted message\n";

        // receive echo
        n=0;
        while(true){
            bytes = recv(s,&receive_buffer[n],1,0);
            if(bytes<=0){ perror("recv echo"); exit(1); }
            if(receive_buffer[n]=='\n'){ receive_buffer[n]=0; break; }
            if(receive_buffer[n]!='\r') ++n;
        }
        cout<<"Secure Client: received echo: "<<receive_buffer<<endl;
    }

    cout<<"\nClient shutting down...\n";
#if defined __unix__ || defined __APPLE__
    close(s);
#elif defined _WIN32
    closesocket(s);
    WSACleanup();
#endif

    return 0;
}
