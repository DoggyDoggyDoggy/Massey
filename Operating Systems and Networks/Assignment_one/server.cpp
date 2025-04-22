//=======================================================================================================================
// Course: 159.342
// Description: Cross-platform, Active mode FTP SERVER, Start-up Code for Assignment 1 
//
// This code gives parts of the answers away but this implementation is only IPv4-compliant. 
// Remember that the assignment requires a fully IPv6-compliant cross-platform FTP server that can communicate with a 
// built-in FTP client either in Windows 11, Ubuntu Linux or MacOS.
// 
// This program is cross-platform but your assignment will be marked only on Windows 11.
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
  #include <netdb.h>      // used by getaddrinfo() and getnameinfo()
  #include <ctype.h>
  #include <iostream>
#elif defined _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>   // required by getaddrinfo() and special constants
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

FileType file_type;


#if defined _WIN32
#define strcasecmp _stricmp
#endif

//====================================================================
// MAIN
//====================================================================
int main(int argc, char *argv[]) {

    file_type = FileType::UNKNOWN;

    bool loggedIn = false;
    char authUser[256] = {0};

#if defined __unix__ || defined __APPLE__
   
#elif defined _WIN32
    int err = WSAStartup(WSVERS, &wsadata);
    if (err != 0) {
         WSACleanup();
         printf("WSAStartup failed with error: %d\n", err);
         exit(1);
    }
#endif
    struct addrinfo hints, *result = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = (USE_IPV6 ? AF_INET6 : AF_INET); 
    hints.ai_socktype = SOCK_STREAM;   // TCP stream socket.
    hints.ai_protocol = IPPROTO_TCP;     // TCP protocol.
    hints.ai_flags = AI_PASSIVE;         

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
    
    
    printf("\n============================================\n");
    printf("   << 159.342 Cross-platform FTP Server >>\n");
    printf("   submitted by: Denys Pedan#23011350 \n");
    printf("   date:09/04/2025\n");
    printf("============================================\n");

    //--------------------------------------------------------------------------------------
    // Use sockaddr_storage instead of sockaddr_in for remote addresses (IPv6 compliant).
    //--------------------------------------------------------------------------------------
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen = sizeof(remoteaddr);

    
    struct sockaddr_storage local_data_addr_act;
#if defined __unix__ || defined __APPLE__
    int s_data_act, ns_data;
    s_data_act = -1;
#elif defined _WIN32
    SOCKET s_data_act, ns_data;
    s_data_act = INVALID_SOCKET;
#endif

    char send_buffer[BUFFER_SIZE], receive_buffer[BUFFER_SIZE];
    int active = 0;  // Flag indicating that a data connection has been established
    int n, bytes;

    //--------------------------------------------------------------------------------------
    // MAIN LOOP - Accept connections
    //--------------------------------------------------------------------------------------
    int count = 0;
    while (1) { // Start of MAIN LOOP
        // Reset authentication status for each new connection.
        loggedIn = false;
        memset(authUser, 0, sizeof(authUser));

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
        count = snprintf(send_buffer, BUFFER_SIZE, "220 FTP Server ready.\r\n");
        if(count >= 0 && count < BUFFER_SIZE) {
            send(ns, send_buffer, strlen(send_buffer), 0);
        }

        //----------------------------------------------------------------------------------
        // Communication loop: Process commands from the client.
        //----------------------------------------------------------------------------------
        // Reset active flag for data connection
        active = 0;
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
                printf("\nClient has gracefully exited.\n");
                break;
            }
            printf("[DEBUG INFO] Command received: '%s\\r\\n'\n", receive_buffer);
            
            memset(send_buffer, 0, BUFFER_SIZE);
            //----------------------------------------------------------------------------------
            // Process FTP commands.
            //----------------------------------------------------------------------------------
            
            
            if(strncmp(receive_buffer, "USER", 4) == 0) {
                // Tokenize the command by spaces.
                char *token = strtok(receive_buffer, " ");
                // Skip the first token ("USER")
                token = strtok(NULL, " ");
                // If the next token is also "USER", skip it.
                if(token && strcasecmp(token, "USER") == 0) {
                    token = strtok(NULL, " ");
                }
                if(token) {
                    // Compare the provided username with "napoleon" (case-sensitive check).
                    if(strcmp(token, "napoleon") == 0) {
                        strcpy(authUser, token);
                        snprintf(send_buffer, BUFFER_SIZE, "331 Password required.\r\n");
                    } else {
                        snprintf(send_buffer, BUFFER_SIZE, "530 Not logged in.\r\n");
                    }
                } else {
                    snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in parameters.\r\n");
                }
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            // PASS command: Verify password if previous USER was "napoleon" and password equals "342".
            if(strncmp(receive_buffer, "PASS", 4) == 0) {
                char password[256];
                if(sscanf(receive_buffer, "PASS %s", password) == 1) {
                    if(strcmp(authUser, "napoleon") == 0 && strcmp(password, "342") == 0) {
                        loggedIn = true;
                        snprintf(send_buffer, BUFFER_SIZE, "230 Login successful.\r\n");
                    } else {
                        snprintf(send_buffer, BUFFER_SIZE, "530 Login incorrect.\r\n");
                    }
                } else {
                    snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in parameters.\r\n");
                }
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            // SYST command: Return system type.
            if(strncmp(receive_buffer, "SYST", 4) == 0) {
                snprintf(send_buffer, BUFFER_SIZE, "215 Windows 64-bit\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            // TYPE command: Set transfer mode (I for binary, A for ASCII)
            if(strncmp(receive_buffer, "TYPE", 4) == 0) {
                char objType;
                int scannedItems = sscanf(receive_buffer, "TYPE %c", &objType);
                if(scannedItems < 1) {
                    snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in arguments\r\n");
                    send(ns, send_buffer, strlen(send_buffer), 0);
                    printf("[DEBUG INFO] <-- %s\n", send_buffer);
                    continue;
                }
                switch(toupper(objType)) {
                    case 'I':  
                        file_type = FileType::BINARY;
                        printf("Using binary mode to transfer files.\n");
                        snprintf(send_buffer, BUFFER_SIZE, "200 Command OK.\r\n");
                        break;
                    case 'A':  
                        file_type = FileType::TEXT;
                        printf("Using ASCII mode to transfer files.\n");
                        snprintf(send_buffer, BUFFER_SIZE, "200 Command OK.\r\n");
                        break;
                    default:
                        snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in arguments.\r\n");
                        break;
                }
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            // STOR command: Not implemented.
            if(strncmp(receive_buffer, "STOR", 4) == 0) {
                printf("Unrecognised command (STOR not implemented).\n");
                snprintf(send_buffer, BUFFER_SIZE, "502 Command not implemented.\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            // RETR command: Retrieve (download) a file.
            if(strncmp(receive_buffer, "RETR", 4) == 0) {
                if(!loggedIn) {
                    snprintf(send_buffer, BUFFER_SIZE, "530 Not logged in.\r\n");
                    send(ns, send_buffer, strlen(send_buffer), 0);
                    continue;
                }
                // Parse the filename from the command "RETR <filename>"
                char filename[256];
                if(sscanf(receive_buffer, "RETR %s", filename) != 1) {
                    snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in parameters.\r\n");
                    send(ns, send_buffer, strlen(send_buffer), 0);
                    continue;
                }
                // Open the requested file (binary mode if file_type==BINARY, otherwise text mode)
                FILE *f = NULL;
                if(file_type == FileType::BINARY) {
                    f = fopen(filename, "rb");
                } else {
                    f = fopen(filename, "r");
                }
                if(!f) {
                    snprintf(send_buffer, BUFFER_SIZE, "550 File not found or access denied.\r\n");
                    send(ns, send_buffer, strlen(send_buffer), 0);
                    continue;
                }
                // Check if a data connection has been established (via EPRT)
                if(!active) {
                    snprintf(send_buffer, BUFFER_SIZE, "425 Use EPRT to establish data connection.\r\n");
                    send(ns, send_buffer, strlen(send_buffer), 0);
                    fclose(f);
                    continue;
                }
                // Send preliminary reply on control connection
                snprintf(send_buffer, BUFFER_SIZE, "150 Opening data connection for file transfer.\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                // Transfer file contents over the data connection (s_data_act)
                char fileBuffer[1024];
                size_t readBytes;
                while((readBytes = fread(fileBuffer, 1, sizeof(fileBuffer), f)) > 0) {
                    send(s_data_act, fileBuffer, readBytes, 0);
                }
                fclose(f);
#if defined __unix__ || defined __APPLE__
                close(s_data_act);
#elif defined _WIN32
                closesocket(s_data_act);
#endif
                active = 0; // Reset active flag after transfer
                snprintf(send_buffer, BUFFER_SIZE, "226 File transfer complete.\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                continue;
            }
            // OPTS command: Always return unrecognized command.
            if(strncmp(receive_buffer, "OPTS", 4) == 0) {
                printf("Unrecognised command (OPTS not implemented).\n");
                snprintf(send_buffer, BUFFER_SIZE, "502 Command not implemented.\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            // EPRT command: Parse client data connection parameters and establish active data connection.
            if(strncmp(receive_buffer, "EPRT", 4) == 0) {
                // Expected format: EPRT |<protocol>|<address>|<port>|
                char *p = strchr(receive_buffer, '|');
                if(!p) {
                    snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in parameters.\r\n");
                    send(ns, send_buffer, strlen(send_buffer), 0);
                    continue;
                }
                p++; // Move past first '|'
                char protocol[10], addrStr[256], portStr[10];
                char *token = strtok(p, "|");
                if(token) {
                    strcpy(protocol, token);
                    token = strtok(NULL, "|");
                    if(token) {
                        strcpy(addrStr, token);
                        token = strtok(NULL, "|");
                        if(token) {
                            strcpy(portStr, token);
                        } else {
                            snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in parameters.\r\n");
                            send(ns, send_buffer, strlen(send_buffer), 0);
                            continue;
                        }
                    } else {
                        snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in parameters.\r\n");
                        send(ns, send_buffer, strlen(send_buffer), 0);
                        continue;
                    }
                } else {
                    snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in parameters.\r\n");
                    send(ns, send_buffer, strlen(send_buffer), 0);
                    continue;
                }
                // Use getaddrinfo() to resolve the client's data address.
                struct addrinfo dataHints, *dataResult = NULL;
                memset(&dataHints, 0, sizeof(dataHints));
                dataHints.ai_family = (USE_IPV6 ? AF_INET6 : AF_INET);
                dataHints.ai_socktype = SOCK_STREAM;
                dataHints.ai_protocol = IPPROTO_TCP;
                iResult = getaddrinfo(addrStr, portStr, &dataHints, &dataResult);
                if(iResult != 0) {
                    snprintf(send_buffer, BUFFER_SIZE, "425 Can't open data connection: %s\r\n", gai_strerror(iResult));
                    send(ns, send_buffer, strlen(send_buffer), 0);
                    continue;
                }
                // Create the data socket.
#if defined __unix__ || defined __APPLE__
                s_data_act = socket(dataResult->ai_family, dataResult->ai_socktype, dataResult->ai_protocol);
                if (s_data_act < 0) {
                    snprintf(send_buffer, BUFFER_SIZE, "425 Can't open data connection.\r\n");
                    send(ns, send_buffer, strlen(send_buffer), 0);
                    freeaddrinfo(dataResult);
                    continue;
                }
#elif defined _WIN32
                s_data_act = socket(dataResult->ai_family, dataResult->ai_socktype, dataResult->ai_protocol);
                if (s_data_act == INVALID_SOCKET) {
                    snprintf(send_buffer, BUFFER_SIZE, "425 Can't open data connection.\r\n");
                    send(ns, send_buffer, strlen(send_buffer), 0);
                    freeaddrinfo(dataResult);
                    continue;
                }
#endif
                // Connect to the client's provided data address.
                if (connect(s_data_act, dataResult->ai_addr, (int)dataResult->ai_addrlen) != 0) {
                    snprintf(send_buffer, BUFFER_SIZE, "425 Can't open data connection.\r\n");
                    send(ns, send_buffer, strlen(send_buffer), 0);
#if defined __unix__ || defined __APPLE__
                    close(s_data_act);
#elif defined _WIN32
                    closesocket(s_data_act);
#endif
                    freeaddrinfo(dataResult);
                    continue;
                }
                freeaddrinfo(dataResult);
                active = 1; // Mark that data connection is active.
                snprintf(send_buffer, BUFFER_SIZE, "200 EPRT command successful.\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                continue;
            }
            // PORT command: For IPv6, advise to use EPRT.
            if(strncmp(receive_buffer, "PORT", 4) == 0) {
                snprintf(send_buffer, BUFFER_SIZE, "522 Use EPRT for IPv6 connections.\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                continue;
            }
            // LIST/NLST command: Send directory listing over data connection.
            if((strncmp(receive_buffer, "LIST", 4)==0) || (strncmp(receive_buffer, "NLST", 4)==0)) {
                if(!loggedIn) {
                    snprintf(send_buffer, BUFFER_SIZE, "530 Not logged in.\r\n");
                    send(ns, send_buffer, strlen(send_buffer), 0);
                    continue;
                }
                if(!active) {
                    snprintf(send_buffer, BUFFER_SIZE, "425 Use EPRT to establish data connection.\r\n");
                    send(ns, send_buffer, strlen(send_buffer), 0);
                    continue;
                }
    #if defined __unix__ || defined __APPLE__
                int ret = system("ls -la > tmp.txt");
    #elif defined _WIN32
                int ret = system("dir > tmp.txt");
    #endif
                FILE *fin = fopen("tmp.txt", "r");
                if(!fin) {
                    snprintf(send_buffer, BUFFER_SIZE, "550 Failed to open directory listing.\r\n");
                    send(ns, send_buffer, strlen(send_buffer), 0);
                    continue;
                }
                snprintf(send_buffer, BUFFER_SIZE, "150 Opening data connection for directory listing.\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                char temp_buffer[80];
                // Send directory listing over data connection.
                while (fgets(temp_buffer, sizeof(temp_buffer), fin) != NULL) {
                    send(s_data_act, temp_buffer, strlen(temp_buffer), 0);
                }
                fclose(fin);
    #if defined __unix__ || defined __APPLE__
                close(s_data_act);
    #elif defined _WIN32
                closesocket(s_data_act);
    #endif
                active = 0;
                snprintf(send_buffer, BUFFER_SIZE, "226 Directory send OK.\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                continue;
            }
            // QUIT command: Close the connection.
            if(strncmp(receive_buffer, "QUIT", 4) == 0) {
                printf("Quit command received.\n");
                snprintf(send_buffer, BUFFER_SIZE, "221 Connection closing.\r\n");
                send(ns, send_buffer, strlen(send_buffer), 0);
                printf("[DEBUG INFO] <-- %s\n", send_buffer);
                break;
            }
            // If command is not recognized, send error message.
            snprintf(send_buffer, BUFFER_SIZE, "502 Command not implemented.\r\n");
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
