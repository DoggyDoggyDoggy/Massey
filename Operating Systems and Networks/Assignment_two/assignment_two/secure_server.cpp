//////////////////////////////////////////////////////////////////////////////////////////////
// TCP SECURE SERVER GCC (IPV6 ready)
//
//
// References: https://msdn.microsoft.com/en-us/library/windows/desktop/ms738520(v=vs.85).aspx
//             http://long.ccaba.upc.edu/long/045Guidelines/eva/ipv6.html#daytimeServer6
//
// Author: Napoleon Reyes, Ph.D.
//         Massey University, Albany  
//////////////////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_PORT "1234" 
#define USE_IPV6 true  // if false → IPv4; assignment marked with IPv6

#if defined __unix__ || defined __APPLE__
  #include <unistd.h>
  #include <errno.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <netdb.h> // getnameinfo()
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

#include <boost/multiprecision/cpp_int.hpp>  // big integers
#include <sstream>
#include <vector>
#include <algorithm>  // std::remove
#include <cstdint>    // uint8_t

using boost::multiprecision::cpp_int;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////

//*** RSA-CBC INSERT START ***

// Fast modular exponentiation (RSA)
cpp_int modexp(cpp_int base, cpp_int exp, const cpp_int& mod) {
    cpp_int result = 1;
    while (exp > 0) {
        if ((exp & 1) != 0) result = (result * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

// XOR for CBC chaining
cpp_int xor_block(const cpp_int& a, const cpp_int& b) {
    return a ^ b;
}

// Server's RSA key pair (replace with yours)
const cpp_int SERVER_e = cpp_int("3");
const cpp_int SERVER_d = cpp_int("16971");
const cpp_int SERVER_n = cpp_int("25777");

// CA's private key for signing server certificate
const cpp_int CA_d     = cpp_int("74297");
const cpp_int CA_n     = cpp_int("190933");

// Parse decimal string → cpp_int
cpp_int parse_int(const string& s) {
    return cpp_int(s);
}

// cpp_int → string
string to_string(const cpp_int& x) {
    return x.convert_to<string>();
}
//*** RSA-CBC INSERT END ***

//////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
    //********************************************************************
    // SOCKET INIT
    //********************************************************************
#if defined __unix__ || defined __APPLE__
    int s, ns;
#elif defined __WIN32__
    SOCKET s, ns;
#endif

    struct sockaddr_storage clientAddress;
    char clientHost[NI_MAXHOST], clientService[NI_MAXSERV];
    char portNum[NI_MAXSERV];

    cpp_int nonce;
    uint8_t prev_byte = 0;

#if defined __WIN32__
    int err = WSAStartup(WSVERS, &wsadata);
    if (err != 0) { printf("WSAStartup failed: %d\n", err); return 1; }
    if (LOBYTE(wsadata.wVersion)!=2 || HIBYTE(wsadata.wVersion)!=2) {
        printf("Winsock 2.2 not supported\n"); WSACleanup(); return 1;
    } else printf("Winsock 2.2 initialized.\n");
#endif

    //********************************************************************
    // ADDRINFO SETUP
    //********************************************************************
    struct addrinfo *result = nullptr, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = USE_IPV6 ? AF_INET6 : AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags    = AI_PASSIVE;

    if (argc == 2) {
        strcpy(portNum, argv[1]);
        printf("Using port %s\n", portNum);
        if (getaddrinfo(NULL, argv[1], &hints, &result) != 0) {
            perror("getaddrinfo"); return 1;
        }
    } else {
        strcpy(portNum, DEFAULT_PORT);
        printf("Using DEFAULT_PORT = %s\n", portNum);
        if (getaddrinfo(NULL, DEFAULT_PORT, &hints, &result) != 0) {
            perror("getaddrinfo"); return 1;
        }
    }

    //********************************************************************
    // SOCKET & BIND
    //********************************************************************
    s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
#if defined __unix__ || defined __APPLE__
    if (s < 0) { perror("socket"); freeaddrinfo(result); return 1; }
#elif defined __WIN32__
    if (s == INVALID_SOCKET) {
        printf("socket error: %d\n", WSAGetLastError());
        freeaddrinfo(result); WSACleanup(); return 1;
    }
#endif

    if (bind(s, result->ai_addr, (int)result->ai_addrlen) != 0) {
#if defined __unix__ || defined __APPLE__
        perror("bind"); close(s);
#elif defined __WIN32__
        printf("bind error: %d\n", WSAGetLastError());
        closesocket(s); WSACleanup();
#endif
        freeaddrinfo(result); return 1;
    }
    freeaddrinfo(result);

    //********************************************************************
    // LISTEN
    //********************************************************************
    if (listen(s, SOMAXCONN) != 0) {
#if defined __unix__ || defined __APPLE__
        perror("listen"); close(s);
#elif defined __WIN32__
        printf("listen error: %d\n", WSAGetLastError());
        closesocket(s); WSACleanup();
#endif
        return 1;
    }

    printf("<<<SERVER>>> listening on port %s\n", portNum);

    //********************************************************************
    // MAIN LOOP
    //********************************************************************
    while (true) {
        socklen_t addrlen = sizeof(clientAddress);
        ns = accept(s, (struct sockaddr*)&clientAddress, &addrlen);
#if defined __unix__ || defined __APPLE__
        if (ns < 0) { perror("accept"); break; }
#elif defined __WIN32__
        if (ns == INVALID_SOCKET) {
            printf("accept error: %d\n", WSAGetLastError());
            break;
        }
#endif

        getnameinfo((struct sockaddr*)&clientAddress, addrlen,
                    clientHost, sizeof(clientHost),
                    clientService, sizeof(clientService),
                    NI_NUMERICHOST);
        printf("Client connected: %s:%s\n", clientHost, clientService);

        // 1) Send signed cert (e,n) encrypted by CA_d
        {
            cpp_int enc_e = modexp(SERVER_e, CA_d, CA_n);
            cpp_int enc_n = modexp(SERVER_n, CA_d, CA_n);
            string cert = to_string(enc_e) + " " + to_string(enc_n) + "\r\n";
            send(ns, cert.c_str(), (int)cert.size(), 0);
            cout<<"Sent certificate: "<<cert;
        }

        // 2) Recv ACK 226 public key received
        {
            char buf[256];
            int b = recv(ns, buf, sizeof(buf)-1, 0);
            if (b <= 0) { perror("recv ACK226"); goto CLEAN; }
            buf[b] = '\0';
            cout<<"Received: "<<buf;
        }

        // 3) Recv encrypted NONCE, strip CRLF, decrypt, set prev_byte
        {
            char buf[1024];
            int b = recv(ns, buf, sizeof(buf)-1, 0);
            if (b <= 0) { perror("recv nonce"); goto CLEAN; }
            buf[b] = '\0';
            string s_enc(buf);
            // strip CR and LF
            s_enc.erase(remove(s_enc.begin(), s_enc.end(), '\r'), s_enc.end());
            s_enc.erase(remove(s_enc.begin(), s_enc.end(), '\n'), s_enc.end());

            cpp_int enc_nonce = parse_int(s_enc);
            nonce = modexp(enc_nonce, SERVER_d, SERVER_n);
            // CBC IV is only low byte:
            prev_byte = (uint8_t)(nonce.convert_to<unsigned long>() & 0xFF);
            cout<<"Decrypted NONCE: "<<nonce<<endl;
        }

        // 4) Send ACK 220 nonce OK
        {
            const char* ack220 = "ACK 220 nonce OK\r\n";
            send(ns, ack220, (int)strlen(ack220), 0);
            cout<<"Sent ACK 220 nonce OK\n";
        }

        // 5) Communication: recv CBC blocks → decrypt → echo
        while (true) {
            // read one line of ciphertext
            string line;
            char c;
            while (true) {
                int b = recv(ns, &c, 1, 0);
                if (b <= 0) goto CLEAN;
                if (c == '\n') break;
                if (c != '\r') line.push_back(c);
            }
            if (line.empty()) continue;

            // parse space-separated big ints
            istringstream iss(line);
            vector<cpp_int> cblocks;
            string tok;
            while (iss >> tok) {
                cblocks.push_back(parse_int(tok));
            }

            // decrypt CBC byte-wise
            string plain;
            for (auto &cb : cblocks) {
                // RSA decrypt to big integer x
                cpp_int x = modexp(cb, SERVER_d, SERVER_n);
                // extract low byte of x
                uint8_t x_byte = (uint8_t)(x.convert_to<unsigned long>() & 0xFF);
                // CBC XOR at byte level
                uint8_t m_byte = x_byte ^ prev_byte;
                plain.push_back((char)m_byte);
                // update prev_byte from low byte of ciphertext
                prev_byte = (uint8_t)(cb.convert_to<unsigned long>() & 0xFF);
            }

            cout<<"Decrypted message: "<<plain<<endl;

            // echo back
            string reply = "Echo: " + plain + "\r\n";
            send(ns, reply.c_str(), (int)reply.size(), 0);
        }

    CLEAN:
#if defined __unix__ || defined __APPLE__
        close(ns);
#elif defined __WIN32__
        closesocket(ns);
#endif
        cout<<"Client disconnected\n\n";
    }

    // Shutdown
#if defined __unix__ || defined __APPLE__
    close(s);
#elif defined __WIN32__
    closesocket(s);
    WSACleanup();
#endif

    return 0;
}
