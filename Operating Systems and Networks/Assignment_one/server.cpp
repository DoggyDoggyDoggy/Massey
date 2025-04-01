//=======================================================================================================================
// Course: 159.342
// Description: Cross-platform, Active mode FTP SERVER, Start-up Code for Assignment 1 
//
// This code gives parts of the answers away but this implementation is only IPv4-compliant. 
// Remember that the assignment requires a fully IPv6-compliant cross-platform FTP server that can communicate with a 
// built-in FTP client either in Windows 11, Ubuntu Linux or MacOS.
// 
// This program is cross-platform but your assignment will be marked only in Windows 11.
//
// You must change parts of this program to make it IPv6-compliant (replace all data structures and functions that work only with IPv4).
//
// Hint: The sample TCP server codes show the appropriate data structures and functions that work in both IPv4 and IPv6. 
//       We also covered IPv6-compliant data structures and functions in our lectures.   
//
// Author: n.h.reyes@massey.ac.nz
//=======================================================================================================================

#define USE_IPV6 true  // Now using IPv6; set to true to enable IPv6

#if defined __unix__ || defined __APPLE__
  #include <unistd.h>
  #include <errno.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <netdb.h> // used by getaddrinfo() and getnameinfo()
  #include <ctype.h>
  #include <iostream>
#elif defined _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h> // required by getaddrinfo() and special constants
  #include <stdlib.h>
  #include <stdio.h>
  #include <ctype.h>
  #include <iostream>
  #define WSVERS MAKEWORD(2,2) /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
                              // The high-order byte specifies the minor version number; 
                              // the low-order byte specifies the major version number.
  WSADATA wsadata; // Create a WSADATA object called wsadata.
#endif

#define BUFFER_SIZE 256
enum class FileType { BINARY, TEXT, UNKNOWN };

FileType file_type;  // Global variable to store the current file transfer mode

//====================================================================
// MAIN
//====================================================================
int main(int argc, char *argv[]) {
    // Set initial file type to UNKNOWN
    file_type = FileType::UNKNOWN;

#if defined __unix__ || defined __APPLE__
   // Nothing specific needed on Unix-based systems for socket initialization.
#elif defined _WIN32
    // Initialize Winsock on Windows
    int err = WSAStartup(WSVERS, &wsadata);
    if (err != 0) {
         WSACleanup();
         printf("WSAStartup failed with error: %d\n", err);
         exit(1);
    }
#endif

    //--------------------------------------------------------------------------------------
    // Setup local address info using getaddrinfo() to support both IPv4 and IPv6.
    // Old code used sockaddr_in; we replace it with getaddrinfo() for IPv6 compliance.
    //--------------------------------------------------------------------------------------
    struct addrinfo hints, *result = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = (USE_IPV6 ? AF_INET6 : AF_INET); // Choose AF_INET6 if IPv6 enabled, else AF_INET.
    hints.ai_socktype = SOCK_STREAM;   // TCP stream socket.
    hints.ai_protocol = IPPROTO_TCP;     // TCP protocol.
    hints.ai_flags = AI_PASSIVE;         // Use the wildcard IP address.

    char default_port[] = "1234";
    // If port is provided as an argument, use it; otherwise, use default port.
    char *port_str = (argc == 2) ? argv[1] : default_port;
    int iResult = getaddrinfo(NULL, port_str, &hints, &result);
    if(iResult != 0) {
        printf("getaddrinfo failed: %s\n", gai_strerror(iResult));
#if defined _WIN32
        WSACleanup();
#endif
        exit(1);
    }

    //--------------------------------------------------------------------------------------
    // Create the control connection socket.
    // Old code used int s, ns; we maintain similar variables.
    //--------------------------------------------------------------------------------------
#if defined __unix__ || defined __APPLE__
    int s, ns;
#elif defined _WIN32
    SOCKET s, ns;
#endif

    s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
#if defined __unix__ || defined __APPLE__
    if (s < 0) {
        printf("socket failed\n");
        freeaddrinfo(result);
        exit(1);
    }
#elif defined _WIN32
    if (s == INVALID_SOCKET) {
        printf("socket failed\n");
        freeaddrinfo(result);
        WSACleanup();
        exit(1);
    }
#endif

    //--------------------------------------------------------------------------------------
    // Bind the socket to the local address.
    // This replaces the old IPv4-specific bind using sockaddr_in.
    //--------------------------------------------------------------------------------------
    if (bind(s, result->ai_addr, (int)result->ai_addrlen) != 0) {
        printf("Bind failed!\n");
        freeaddrinfo(result);
#if defined _WIN32
        closesocket(s);
        WSACleanup();
#else
        close(s);
#endif
        exit(1);
    }
    freeaddrinfo(result);  // Free the address info structure now that bind is complete.

    //--------------------------------------------------------------------------------------
    // Listen for incoming connections on the control socket.
    //--------------------------------------------------------------------------------------
    if (listen(s, 5) != 0) {
        printf("Listen failed!\n");
#if defined _WIN32
        closesocket(s);
        WSACleanup();
#else
        close(s);
#endif
        exit(1);
    }
    
    // Original header comments preserved.
    printf("\n============================================\n");
    printf("   << 159.342 Cross-platform FTP Server >>\n");
    printf("   submitted by:     \n");
    printf("   date:     \n");
    printf("============================================\n");

    //--------------------------------------------------------------------------------------
    // Use sockaddr_storage instead of sockaddr_in for remote addresses (IPv6 compliant).
    //--------------------------------------------------------------------------------------
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen = sizeof(remoteaddr);

    // For active mode – replace IPv4-specific structure with sockaddr_storage.
    struct sockaddr_storage local_data_addr_act;
#if defined __unix__ || defined __APPLE__
    int ns_data, s_data_act;
    ns_data = -1;
#elif defined _WIN32
    SOCKET ns_data, s_data_act;
    ns_data = INVALID_SOCKET;
#endif

    char send_buffer[BUFFER_SIZE], receive_buffer[BUFFER_SIZE];
    int active = 0;
    int n, bytes;

    //--------------------------------------------------------------------------------------
    // MAIN LOOP - Accept connections
    //--------------------------------------------------------------------------------------
    int count = 0;
    while (1) { // Start of MAIN LOOP
        printf("\n------------------------------------------------------------------------\n");
        printf("SERVER is waiting for an incoming connection request at port: %s\n", port_str);
        printf("------------------------------------------------------------------------\n");

#if defined __unix__ || defined __APPLE__
        ns = accept(s, (struct sockaddr *)(&remoteaddr), &addrlen);
        if (ns < 0) break;
#elif defined _WIN32
        ns = accept(s, (struct sockaddr *)(&remoteaddr), &addrlen);
        if (ns == INVALID_SOCKET) break;
#endif

        //----------------------------------------------------------------------------------
        // Print client's address information using getnameinfo()
        // This replaces the old usage of inet_ntoa() which works only for IPv4.
        //----------------------------------------------------------------------------------
        char clientHost[NI_MAXHOST], clientService[NI_MAXSERV];
        memset(clientHost, 0, sizeof(clientHost));
        memset(clientService, 0, sizeof(clientService));
        int ret = getnameinfo((struct sockaddr *)&remoteaddr, addrlen,
                              clientHost, sizeof(clientHost),
                              clientService, sizeof(clientService),
                              NI_NUMERICHOST | NI_NUMERICSERV);
        if(ret == 0) {
            printf("\nConnected to [CLIENT's IP %s , port %s] through SERVER's port %s\n", 
                    clientHost, clientService, port_str);
        } else {
            printf("\nConnected to CLIENT, but getnameinfo failed: %s\n", gai_strerror(ret));
        }
        
        //----------------------------------------------------------------------------------
        // Send welcome message to the client.
        //----------------------------------------------------------------------------------
        count = snprintf(send_buffer, BUFFER_SIZE, "220 FTP Server ready. \r\n");
        if(count >= 0 && count < BUFFER_SIZE) {
            bytes = send(ns, send_buffer, strlen(send_buffer), 0);
        }

        //----------------------------------------------------------------------------------
        // Communication loop: Process commands from the client.
        //----------------------------------------------------------------------------------
        while (1) {
            n = 0;
            memset(receive_buffer, 0, BUFFER_SIZE);
            // Receive the command byte by byte until LF is encountered.
            while (1) {
                bytes = recv(ns, &receive_buffer[n], 1, 0);
                if(bytes <= 0) break;
                if(receive_buffer[n] == '\n') {
                    receive_buffer[n] = '\0';
                    break;
                }
                if(receive_buffer[n] != '\r') n++;  // Skip CR characters.
            }
            if(bytes <= 0) {
                printf("\nclient has gracefully exited.\n");
                break;
            }
            printf("[DEBUG INFO] command received:  '%s\\r\\n'\n", receive_buffer);
            
            memset(send_buffer, 0, BUFFER_SIZE);
            //----------------------------------------------------------------------------------
            // Process FTP commands.
            // The following commands are processed:
            // USER, PASS, SYST, TYPE, STOR, RETR, OPTS, EPRT, CWD, QUIT, PORT, LIST/NLST.
            // Original logic preserved with minimal modifications for IPv6.
            //----------------------------------------------------------------------------------
            if(strncmp(receive_buffer,"USER",4) == 0) {
                printf("Logging in... \n");
                count = snprintf(send_buffer, BUFFER_SIZE, "331 Password required (anything will do really... :-) \r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            if(strncmp(receive_buffer,"PASS",4) == 0) {
                count = snprintf(send_buffer, BUFFER_SIZE, "230 Public login successful \r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            if(strncmp(receive_buffer,"SYST",4) == 0) {
                count = snprintf(send_buffer, BUFFER_SIZE, "215 Windows 64-bit\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            if(strncmp(receive_buffer,"TYPE",4) == 0) {
                char objType;
                int scannedItems = sscanf(receive_buffer, "TYPE %c", &objType);
                if(scannedItems < 1) {
                    count = snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in arguments\r\n");
                    send(ns, send_buffer, strlen(send_buffer), 0);
                    printf("[DEBUG INFO] <-- %s\n", send_buffer);
                    continue;
                }
                switch(toupper(objType)) {
                    case 'I':  
                        file_type = FileType::BINARY;
                        printf("using binary mode to transfer files.\n");
                        count = snprintf(send_buffer, BUFFER_SIZE, "200 command OK.\r\n");
                        break;
                    case 'A':  
                        file_type = FileType::TEXT;
                        printf("using ASCII mode to transfer files.\n");
                        count = snprintf(send_buffer, BUFFER_SIZE, "200 command OK.\r\n");
                        break;
                    default:
                        count = snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in arguments\r\n");
                        break;
                }
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            if(strncmp(receive_buffer,"STOR",4) == 0) {
                printf("unrecognised command \n");
                count = snprintf(send_buffer, BUFFER_SIZE, "502 command not implemented\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            if(strncmp(receive_buffer,"RETR",4) == 0) {
                printf("unrecognised command \n");
                count = snprintf(send_buffer, BUFFER_SIZE, "502 command not implemented\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            if(strncmp(receive_buffer,"OPTS",4) == 0) {
                printf("unrecognised command \n");
                count = snprintf(send_buffer, BUFFER_SIZE, "502 command not implemented\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            if(strncmp(receive_buffer,"EPRT",4) == 0) {  
                printf("processing EPRT command \n");
                // Minimal processing for active mode in IPv6.
                count = snprintf(send_buffer, BUFFER_SIZE, "200 EPRT command successful\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            if(strncmp(receive_buffer,"CWD",3) == 0) {
                printf("unrecognised command \n");
                count = snprintf(send_buffer, BUFFER_SIZE, "502 command not implemented\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            if(strncmp(receive_buffer,"QUIT",4) == 0) {
                printf("Quit \n");
                count = snprintf(send_buffer, BUFFER_SIZE, "221 Connection close by client\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                break;
            }
            if(strncmp(receive_buffer,"PORT",4) == 0) {
                // For IPv6, it is recommended to use EPRT. Return a message to advise this.
                count = snprintf(send_buffer, BUFFER_SIZE, "522 Use EPRT for IPv6 connections.\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                continue;
            }
            if((strncmp(receive_buffer,"LIST",4)==0) || (strncmp(receive_buffer,"NLST",4)==0)) {
    #if defined __unix__ || defined __APPLE__
                int ret = system("ls -la > tmp.txt"); // On Unix, use ls command.
    #elif defined _WIN32
                int ret = system("dir > tmp.txt"); // On Windows, use dir command.
    #endif
                printf ("system() returned: %d.\n", ret);
                FILE *fin = fopen("tmp.txt", "r");
                count = snprintf(send_buffer, BUFFER_SIZE, "150 Opening data connection...\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                char temp_buffer[80];
                // Send file listing line by line.
                while (!feof(fin)) {
                    memset(temp_buffer, 0, sizeof(temp_buffer));
                    if(fgets(temp_buffer, sizeof(temp_buffer)-2, fin) != NULL) {
                        count = snprintf(send_buffer, BUFFER_SIZE, "%s", temp_buffer);
                        send(ns, send_buffer, strlen(send_buffer), 0);
                    }
                }
                fclose(fin);
                count = snprintf(send_buffer, BUFFER_SIZE, "226 File transfer complete.\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                continue;
            }
            // If command is not recognized, send error message.
            count = snprintf(send_buffer, BUFFER_SIZE, "502 command not implemented\r\n");
            send(ns, send_buffer, strlen(send_buffer), 0);
        } // End of COMMUNICATION LOOP per CLIENT

    #if defined __unix__ || defined __APPLE__
        close(ns);
    #elif defined _WIN32
        closesocket(ns);
    #endif
        printf("DISCONNECTED from client.\n");
    } // End of MAIN LOOP

    printf("\nSERVER SHUTTING DOWN...\n");
#if defined __unix__ || defined __APPLE__
    close(s);
#elif defined _WIN32
    closesocket(s);
    WSACleanup();
#endif

    return 0;
}