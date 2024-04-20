//=======================================================================================================================
// Course: 159.342
// Description: Cross-platform, Active mode FTP SERVER, Start-up Code for
// Assignment 1
//
// This code gives parts of the answers away but this implementation is only
// IPv4-compliant. Remember that the assignment requires a fully IPv6-compliant
// cross-platform FTP server that can communicate with a built-in FTP client
// either in Windows 10, Ubuntu Linux or MacOS.
//
// This program is cross-platform but your assignment will be marked only in
// Windows 10.
//
// You must change parts of this program to make it IPv6-compliant (replace all
// data structures and functions that work only with IPv4).
//
// Hint: The sample TCP server codes show the appropriate data structures and
// functions that work in both IPv4 and IPv6.
//       We also covered IPv6-compliant data structures and functions in our
//       lectures.
//
// Author: n.h.reyes@massey.ac.nz
//=======================================================================================================================
#define USE_IPV6 true // enable Ipv6
#if defined __unix__ || defined __APPLE__
#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netdb.h> //used by getnameinfo()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>

#elif defined __WIN32__
#include <iostream>
#include <stdio.h>
//#include <stdlib.h>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h> //required by getaddrinfo() and special constants
#include <fstream>
#define WSVERS MAKEWORD(2, 2)

using namespace std;

WSADATA wsadata; // Create a WSADATA object called wsadata.

#endif

enum class FileType { BINARY, TEXT, UNKNOWN };

FileType file_type;

#define DEFAULT_PORT "1234"

//********************************************************************
// MAIN
//********************************************************************
int main(int argc, char *argv[]) {
  //********************************************************************
  // INITIALIZATION - SOCKET library
  //********************************************************************

  file_type = FileType::UNKNOWN;

#if defined __unix__ || defined __APPLE__
  // nothing to do here

#elif defined _WIN32
  //********************************************************************
  // WSSTARTUP - Window only
  //********************************************************************

  int err = WSAStartup(WSVERS, &wsadata);

  if (err != 0) {
    WSACleanup();
    printf("WSAStartup failed with error: %d\n", err);
    exit(1);
  }

  if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2) {
    printf("Could not find a usable version of Winsock.dll\n");
    WSACleanup();
    exit(1);
  }

#endif

  struct sockaddr_storage
      clientAddress; // IPV6-compatible - address information
  struct sockaddr_storage
      clientAddress_act; // IPV6-compatible - address information
  memset(&clientAddress_act, 0, sizeof(clientAddress_act));

  struct sockaddr_in clientAddress_act_Ipv4; // IPv4 - PORT
  socklen_t addr_len;

  char clientHost[NI_MAXHOST];    // IPV6-compatible - client IP addr buffer
  char clientService[NI_MAXSERV]; // IPV6-compatible - client port buffer

  //********************************************************************
  // set the socket address structure.
  //********************************************************************
  struct addrinfo *result = NULL; // initialize for using getaddrinfo()
  struct addrinfo hints;
  int iResult;  // result of getaddrinfo()

#if defined __unix__ || defined __APPLE__
  int s, ns;               // socket declaration
  int ns_data, s_data_act; // socket declaration
#elif defined _WIN32
  SOCKET s, ns;               // socket declaration
  SOCKET ns_data, s_data_act; // data socket declaration
#endif

#define BUFFER_SIZE 500
#define RBUFFER_SIZE 450

  char send_buffer[BUFFER_SIZE], receive_buffer[BUFFER_SIZE];
  int n, bytes, addrlen;
  char portNum[NI_MAXSERV]; // NI_MAXSERV = 32

  memset(
      &send_buffer, 0,
      BUFFER_SIZE); // initialize buffer
  memset(&receive_buffer, 0, RBUFFER_SIZE); // initialize receive buffer

#if defined __unix__ || defined __APPLE__
  ns_data = -1;
#elif defined _WIN32
  ns_data = INVALID_SOCKET;
#endif

  int active = 0;

  printf("\n============================================\n");
  printf("   << 159.342 Cross-platform FTP Server Ver 2.0 >>");
  printf("\n============================================\n");
  printf("   submitted by: Greshka Lao, Any (Hoi Yi) Kwok");
  printf("\n           date:     22/04/24");
  printf("\n============================================\n");

  //********************************************************************
  // SOCKET
  //********************************************************************

  memset(&hints, 0, sizeof(struct addrinfo)); // initialize address info

  if (USE_IPV6) {
    hints.ai_family = AF_INET6;
  } else { // IPV4
    hints.ai_family = AF_INET;
  }

  hints.ai_socktype = SOCK_STREAM; // for TCP
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  // Resolve the local address and port to be used by the server
  if (argc == 2) { // if we input port number
    iResult = getaddrinfo(NULL, argv[1], &hints, &result);
    sprintf(portNum, "%s", argv[1]);

  } else { //use default port 1234
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    sprintf(portNum, "%s", DEFAULT_PORT);
    printf("\nUsing DEFAULT_PORT = %s\n", portNum);
  }

  if (iResult != 0) {
    printf("getaddrinfo failed: %d\n", iResult);

#if defined _WIN32
    WSACleanup();
#endif
    return 1;
  }

#if defined __unix__ || defined __APPLE__
  s = -1;
#elif defined _WIN32
  s = INVALID_SOCKET;
#endif

  // SOCKET for the server to listen for client connections
  s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

#if defined __unix__ || defined __APPLE__
  if (s < 0) {
    printf("socket failed\n");
    freeaddrinfo(result);
  }
#elif defined _WIN32
  // check for errors in socket allocation
  if (s == INVALID_SOCKET) {
    printf("Error at socket(): %d\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    exit(1); // return 1;
  }
#endif

  //********************************************************************
  // BIND
  //********************************************************************
  iResult = bind(s, result->ai_addr, (int)result->ai_addrlen);

// if error is detected, then clean-up
#if defined __unix__ || defined __APPLE__
  if (iResult == -1) {
    printf("\nbind failed\n");
    freeaddrinfo(result);
    close(s); // close socket
#elif defined _WIN32
  if (iResult == SOCKET_ERROR) {
    printf("bind failed with error: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(s);
    WSACleanup();
#endif
    return 1;
  }

  freeaddrinfo(result); // free the memory allocated by the getaddrinfo function for the server's address, as it is no longer needed

  //********************************************************************
  // LISTEN
  //********************************************************************
#if defined __unix__ || defined __APPLE__
  if (listen(s, SOMAXCONN) == -1) {
#elif defined _WIN32
  if (listen(s, SOMAXCONN) == SOCKET_ERROR) {
#endif

#if defined __unix__ || defined __APPLE__
    printf("\nListen failed\n");
    close(s);
#elif defined _WIN32
    printf("Listen failed with error: %d\n", WSAGetLastError());
    closesocket(s);
    WSACleanup();
#endif

    exit(1);
  }

  //********************************************************************
  // INFINITE LOOP
  //********************************************************************
  int count = 0;
  //====================================================================================
  while (1) { // Start of MAIN LOOP
    //====================================================================================
    addrlen = sizeof(clientAddress); // IPv4 & IPv6-compliant

    printf("\n------------------------------------------------------------------------\n");
    printf("SERVER is waiting for an incoming connection request at port:%s", portNum);
    printf("\n------------------------------------------------------------------------\n");

#if defined __unix__ || defined __APPLE__
    ns = accept(s, (struct sockaddr *)(&clientAddress),
                (socklen_t *)&addrlen);
#elif defined _WIN32
    ns = accept(s, (struct sockaddr *)(&clientAddress), &addrlen);
#endif

#if defined __unix__ || defined __APPLE__
    if (ns == -1) {
      printf("\naccept failed\n");
      close(s);
      return 1;
    } else {
      int returnValue;
      memset(clientHost, 0, sizeof(clientHost));
      memset(clientService, 0, sizeof(clientService));

      returnValue = getnameinfo(
          (struct sockaddr *)&clientAddress, addrlen,    // addr infor
          clientHost, sizeof(clientHost), clientService, // ip address
          sizeof(clientService), NI_NUMERICHOST);        // port

      if (returnValue != 0) {
        printf("\nError detected: getnameinfo() failed \n");
        exit(1);
      } else {
        printf("\n============================================================================\n");
        printf(
            "\nConnected to [Client's IP %s, port %s] through SERVER's port %s",
            clientHost, clientService, portNum);
        printf("\n============================================================================\n");
      }
    }
#elif defined _WIN32

    if (ns == INVALID_SOCKET) {
      printf("accept failed: %d\n", WSAGetLastError());
      closesocket(s);
      WSACleanup();
      return 1;

    } else {
      DWORD returnValue;
      memset(clientHost, 0, sizeof(clientHost));
      memset(clientService, 0, sizeof(clientService));

      returnValue = getnameinfo(
          (struct sockaddr *)&clientAddress, addrlen, clientHost,
          sizeof(clientHost), clientService, sizeof(clientService),NI_NUMERICHOST);
      if (returnValue != 0) {
        printf("\nError detected: getnameinfo() failed with error#%d\n", WSAGetLastError());
        exit(1);
      } else {
        printf("\n============================================================================\n");
        printf(
            "\nConnected to [Client's IP %s, port %s] through SERVER's port %s", clientHost, clientService, portNum);
        printf("\n============================================================================\n");
      }
    }

#endif

    //********************************************************************
    // Respond with welcome message
    //*******************************************************************
    count = snprintf(send_buffer, BUFFER_SIZE, "220 FTP Server ready. \r\n");
    if (count >= 0 && count < BUFFER_SIZE) {
      bytes = send(ns, send_buffer, strlen(send_buffer), 0);
    }

    //********************************************************************
    // COMMUNICATION LOOP per CLIENT
    //********************************************************************

    while (1) {
      // Clear buffer
      memset(receive_buffer, 0, RBUFFER_SIZE);
      n = 0;
      while (1) {

        //********************************************************************
        // RECEIVE MESSAGE AND THEN FILTER IT
        //********************************************************************
        bytes = recv(ns, &receive_buffer[n], 1, 0); // receive byte by byte...

        // 1st error checking before handling data
#if defined __unix__ || defined __APPLE__
        if ((bytes == -1) || (bytes == 0))
          break;
#elif defined _WIN32
        if ((bytes == SOCKET_ERROR) || (bytes == 0))
          break;
#endif

        if (receive_buffer[n] == '\n') { /*end on a LF*/
          receive_buffer[n] = '\0';
          break;
        }
        if (receive_buffer[n] != '\r')
          n++; /*Trim CRs*/
      }

      if (bytes == 0)
        printf("\nclient has gracefully exited.\n");

// 2nd error checking after receiving data
#if defined __unix__ || defined __APPLE__
      if ((bytes == -1) || (bytes == 0))
        break;
#elif defined _WIN32
      if ((bytes == SOCKET_ERROR) || (bytes == 0))
        break;
#endif
      printf("[DEBUG INFO] command received:  '%s\\r\\n' \n", receive_buffer);

      //********************************************************************
      // PROCESS COMMANDS/REQUEST FROM USER
      //********************************************************************

      char username[BUFFER_SIZE];
      char password[BUFFER_SIZE];

      if (strncmp(receive_buffer, "USER", 4) == 0) {
        // this resets the char buffer of both variables.
        memset(username, '\0', sizeof(username));
        memset(password, '\0', sizeof(password));
        int i = 5; // extract from the 5th char (skip "USER " command)
        while (receive_buffer[i] != '\0' && i < BUFFER_SIZE) {
          username[i - 5] = receive_buffer[i];
          i++;
        }
        printf("%s\n", username);
        printf("Logging in... \n");
        count =
            snprintf(send_buffer, BUFFER_SIZE, "331 Password required \r\n");
        if (count >= 0 && count < BUFFER_SIZE) {
          bytes = send(ns, send_buffer, strlen(send_buffer), 0);
        }
        printf("[DEBUG INFO] <-- %s\n", send_buffer);
        if (bytes < 0)
          break;
      }
      //---
      if (strncmp(receive_buffer, "PASS", 4) == 0) {
        int i = 5;
        while (receive_buffer[i] != '\0' && i < BUFFER_SIZE) {
          password[i - 5] = receive_buffer[i];
          i++;
        }
        printf("%s\n", password);
        // we are only allowing one user to log in successfully, as per the details below:
        char user[] = "napoleon";
        char pass[] = "342";
        if (strcmp(username, user) == 0 &&
            strcmp(password, pass) == 0) { // both username and password have to be correct in order to sign in.
          count = snprintf(send_buffer, BUFFER_SIZE,
                           "230 Public login sucessful \r\n");
        } else {
          count = snprintf(send_buffer, BUFFER_SIZE,
                           "530-User cannot log in. \r\n");
        }
        if (count >= 0 && count < BUFFER_SIZE) {
          bytes = send(ns, send_buffer, strlen(send_buffer), 0);
        }
        printf("[DEBUG INFO] <-- %s\n", send_buffer);
        if (!(strcmp(username, user) == 0 && strcmp(password, pass) == 0)) {
          break;
        }
        if (bytes < 0)
          break;
      }
      //---

      if (strncmp(receive_buffer, "SYST", 4) == 0) {
        printf("Information about the system \n");
        count = snprintf(send_buffer, BUFFER_SIZE,
                         ""
                         "\n");
        if (count >= 0 && count < BUFFER_SIZE) {
          bytes = send(ns, send_buffer, strlen(send_buffer), 0);
        }
        printf("[DEBUG INFO] <-- %s\n", send_buffer);
        if (bytes < 0)
          break;
      }

      //---
      if (strncmp(receive_buffer, "TYPE", 4) == 0) {
        bytes = 0;
        printf("<--TYPE command received.\n\n");

        char objType;
        int scannedItems = sscanf(receive_buffer, "TYPE %c", &objType);
        if (scannedItems < 1) {
          count = snprintf(send_buffer, BUFFER_SIZE,
                           "501 Syntax error in arguments\r\n");
          if (count >= 0 && count < BUFFER_SIZE) {
            bytes = send(ns, send_buffer, strlen(send_buffer), 0);
          }
          printf("[DEBUG INFO] <-- %s\n", send_buffer);
          if (bytes < 0)
            break;
        }

        switch (toupper(objType)) {
        case 'I':
          file_type = FileType::BINARY; // image file
          printf("using BINARY mode to transfer files.\n");
          count = snprintf(send_buffer, BUFFER_SIZE, "200 command OK.\r\n");
          if (count >= 0 && count < BUFFER_SIZE) {
            bytes = send(ns, send_buffer, strlen(send_buffer), 0);
          }
          printf("[DEBUG INFO] <-- %s\n", send_buffer);
          if (bytes < 0) break;
          break;

        case 'A':
          file_type = FileType::TEXT; // text file
          printf("using ASCII mode to transfer files.\n");
          count = snprintf(send_buffer, BUFFER_SIZE, "200 command OK.\r\n");
          if (count >= 0 && count < BUFFER_SIZE) {
            bytes = send(ns, send_buffer, strlen(send_buffer), 0);
          }
          printf("[DEBUG INFO] <-- %s\n", send_buffer);

          if (bytes < 0) break;
          break;

        default:
          count = snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in arguments\r\n");
          if (count >= 0 && count < BUFFER_SIZE) {
            bytes = send(ns, send_buffer, strlen(send_buffer), 0);
          }
          printf("[DEBUG INFO] <-- %s\n", send_buffer);
          if (bytes < 0) break;
          break;
        }
      }
      //---
      if (strncmp(receive_buffer, "STOR", 4) == 0) {
        printf("unrecognised command \n");
        count = snprintf(send_buffer, BUFFER_SIZE, "502 command not implemented\r\n");
        if (count >= 0 && count < BUFFER_SIZE) {
          bytes = send(ns, send_buffer, strlen(send_buffer), 0);
        }
        printf("[DEBUG INFO] <-- %s\n", send_buffer);
        if (bytes < 0) break;
      }
      //---
      if (strncmp(receive_buffer, "RETR", 4) == 0) {

        if (file_type == FileType::UNKNOWN) {
          printf("ERROR - Unknown file type\n");
          count = snprintf(send_buffer, BUFFER_SIZE, "550 Please Select Data Mode before transfer. \r\n");
          if (count >= 0 && count < BUFFER_SIZE) {
            bytes = send(ns, send_buffer, strlen(send_buffer), 0);
          }
          printf("[DEBUG INFO] <-- %s\n", send_buffer);
          if (bytes < 0) break;
        } else if (file_type == FileType::TEXT) {
          // we are assuming that image files can only be transferred if FileType is set to binary.
          printf("TEXT file \n");
          count = snprintf(send_buffer, BUFFER_SIZE, "550 DO NOT support Text file transfer. \r\n");
          if (count >= 0 && count < BUFFER_SIZE) {
            bytes = send(ns, send_buffer, strlen(send_buffer), 0);
          }
          printf("[DEBUG INFO] <-- %s\n", send_buffer);
          if (bytes < 0) break;
        } else {
          printf("BINARY file \n");
          count = snprintf(send_buffer, BUFFER_SIZE, "150 opening Binary mode data connection. \r\n");
          if (count >= 0 && count < BUFFER_SIZE) {
            bytes = send(ns, send_buffer, strlen(send_buffer), 0);
            printf("[DEBUG INFO] <-- %s\n", send_buffer);
          }
          if (bytes < 0) break;
          printf("Connecting to client\n");
          s_data_act = socket(clientAddress_act.ss_family, SOCK_STREAM, 0);
          // connection failed
          if (connect(s_data_act, (struct sockaddr *)&clientAddress_act, addr_len) != 0) {
            printf("Error connecting to client\n");
            #if defined __unix__ || defined __APPLE__
              close(s_data_act);
            #elif defined _WIN32
              closesocket(s_data_act);
            #endif
          }
          else {

            char filename[BUFFER_SIZE];
            memset(filename, '\0', sizeof(filename));

            char extension[BUFFER_SIZE];
            memset(extension, '\0', sizeof(extension));

            // Extract filename and file extension

            int i = 5;  // extract from the 5th char (skip "RETR " command)
            int j = 0;  // full file name
            int k = -1;  // file extension
            while (receive_buffer[i] != '\0' && i < BUFFER_SIZE) {
              //filename[i-5] = receive_buffer[i];
              filename[j] = receive_buffer[i];
              if (receive_buffer[i] == '.') {
                k = 0;
              } else if ( k >= 0) {
                extension[k] = receive_buffer[i];
                k++;
              }
              i++;
              j++;
            }

            printf("The filename is %s\n", filename);
            printf("The extension is %s\n", extension);

            bool is_image = false;
            if (strcmp(extension, "jpg") == 0 || strcmp(extension, "jpeg") == 0 ||
                strcmp(extension, "bmp") == 0 || strcmp(extension, "gif") == 0 ||
                strcmp(extension, "tiff") == 0 || strcmp(extension, "png") == 0) {
              is_image = true;
            }

            if (is_image) {
              ifstream retr_file(filename, ios::binary);

              if (!retr_file.is_open()) {
                #if defined __unix__ || defined __APPLE__
                  close(s_data_act);
                #elif defined _WIN32
                  closesocket(s_data_act);
                #endif

                count = snprintf(send_buffer, BUFFER_SIZE, "450 cannot access file. \r\n");
                if (count >= 0 && count < BUFFER_SIZE) {
                  bytes = send(ns, send_buffer, strlen(send_buffer), 0);
                  printf("[DEBUG INFO] <-- %s\n", send_buffer);
                }
                if (bytes < 0) break;
              }
              else {
                char buffer[500];
                while (!retr_file.eof()) {
                  retr_file.read(buffer, sizeof(buffer));
                  int bytes_read = retr_file.gcount();
                  send(s_data_act, buffer, bytes_read, 0);
                }
                retr_file.close();

                #if defined __unix__ || defined __APPLE__
                 close(s_data_act);
                #elif defined _WIN32
                 closesocket(s_data_act);
                #endif

                count = snprintf(send_buffer, BUFFER_SIZE, "226 File transfer complete. \r\n");

                if (count >= 0 && count < BUFFER_SIZE) {
                  bytes = send(ns, send_buffer, strlen(send_buffer), 0);
                  printf("[DEBUG INFO] <-- %s\n", send_buffer);
                }
                if (bytes < 0) break;
              }
            }
            else {
              #if defined __unix__ || defined __APPLE__
                close(s_data_act);
              #elif defined _WIN32
                closesocket(s_data_act);
              #endif

              count = snprintf(send_buffer, BUFFER_SIZE, "450 Unsupported file format. \r\n");
              if (count >= 0 && count < BUFFER_SIZE) {
                bytes = send(ns, send_buffer, strlen(send_buffer), 0);
              }
              if (bytes < 0) break;
            }
          }
        }
      }

      //---
      if (strncmp(receive_buffer, "OPTS", 4) == 0) {
        count =
            snprintf(send_buffer, BUFFER_SIZE, "550 unrecognized command\r\n");
        if (count >= 0 && count < BUFFER_SIZE) {
          bytes = send(ns, send_buffer, strlen(send_buffer), 0);
        }
        printf("[DEBUG INFO] <-- %s\n", send_buffer);
        if (bytes < 0)
          break;
      }
      //---
      if (strncmp(receive_buffer, "CWD", 3) == 0) {
        printf("unrecognised command \n");
        count = snprintf(send_buffer, BUFFER_SIZE, "502 command not implemented\r\n");
        if (count >= 0 && count < BUFFER_SIZE) {
          bytes = send(ns, send_buffer, strlen(send_buffer), 0);
        }
        printf("[DEBUG INFO] <-- %s\n", send_buffer);
        if (bytes < 0)
          break;
      }
      //---
      if (strncmp(receive_buffer, "QUIT", 4) == 0) {
        printf("Quit \n");
        count = snprintf(send_buffer, BUFFER_SIZE, "221 Connection close by client\r\n");
        if (count >= 0 && count < BUFFER_SIZE) {
          bytes = send(ns, send_buffer, strlen(send_buffer), 0);
        }
        printf("[DEBUG INFO] <-- %s\n", send_buffer);
        if (bytes < 0)
          break;
      }
      //---
      if (strncmp(receive_buffer, "PORT", 4) == 0) {
        // We decide to keep the Ipv4 stuff as Port should be ONLY specific to IPv4 addresses
        s_data_act = socket(AF_INET, SOCK_STREAM, 0);
        int act_port[2];
        int act_ip[4], port_dec;
        char ip_decimal[NI_MAXHOST];
        printf("===================================================\n");
        printf("\tActive FTP mode, the client is listening... \n");
        active = 1; // flag for active connection

        int scannedItems = sscanf(receive_buffer, "PORT %d,%d,%d,%d,%d,%d",
                                  &act_ip[0], &act_ip[1], &act_ip[2],
                                  &act_ip[3], &act_port[0], &act_port[1]);

        if (scannedItems < 6) {
          count = snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in arguments \r\n");
          if (count >= 0 && count < BUFFER_SIZE) {
            bytes = send(ns, send_buffer, strlen(send_buffer), 0);
          }
          printf("[DEBUG INFO] <-- %s\n", send_buffer);
          if (bytes < 0) break;
        }

        clientAddress_act_Ipv4.sin_family = AF_INET; // Port should be specific to IPv4
        count = snprintf(ip_decimal, NI_MAXHOST, "%d.%d.%d.%d", act_ip[0], act_ip[1], act_ip[2], act_ip[3]);

        if (!(count >= 0 && count < BUFFER_SIZE)) break;

        printf("\tCLIENT's IP is %s\n", ip_decimal);
        clientAddress_act_Ipv4.sin_addr.s_addr = inet_addr(ip_decimal); // Port should be specific to IPv4
        port_dec = act_port[0];
        port_dec = port_dec << 8;
        port_dec = port_dec + act_port[1];
        printf("\tCLIENT's Port is %d\n", port_dec);
        printf("===================================================\n");

        clientAddress_act_Ipv4.sin_port = htons(port_dec); // Port should be specific to IPv4

        count = snprintf(send_buffer, BUFFER_SIZE, "200 PORT Command successful\r\n");
        if (count >= 0 && count < BUFFER_SIZE) {
          bytes = send(ns, send_buffer, strlen(send_buffer), 0);
          printf("[DEBUG INFO] <-- %s\n", send_buffer);
        }
      }
      //---
      if (strncmp(receive_buffer, "EPRT", 4) == 0) {  // available for BOTH IPv4 and Ipv6

        printf("===================================================\n");
        printf("\tActive FTP mode, the client is listening... \n");

        int af;
        int port;
        char ip_address[INET6_ADDRSTRLEN];
        active = 1; // Flag to active mode

        // Parse the EPRT command (pattern: EPRT |protocol|ip|port| , protocol: 1= Ipv4, 2 = Ipv6)
        int scannedItems = sscanf(receive_buffer, "EPRT |%d|%[^|]|%d|", &af, ip_address, &port);

        if (scannedItems < 3 ||
            (af != 1 && af != 2)) { // scannedItems < 3 => not enough parameter, af !=1 or !=2 => wrong protocol
          count = snprintf(send_buffer, BUFFER_SIZE,"501 Syntax error in arguments\r\n");
          if (count >= 0 && count < BUFFER_SIZE) {
            bytes = send(ns, send_buffer, strlen(send_buffer), 0);
          }
          printf("[DEBUG INFO] <-- %s\n", send_buffer);
          if (bytes < 0) break;
        } else {
          if (af == 1) { // IPv4
            struct sockaddr_in *clientAddress_act_ipv4 =
                (struct sockaddr_in *)&clientAddress_act;
            clientAddress_act_ipv4->sin_family = AF_INET;
            clientAddress_act_ipv4->sin_port = htons(port);
            inet_pton(AF_INET, ip_address, &(clientAddress_act_ipv4->sin_addr));
            addr_len = sizeof(struct sockaddr_in);
          } else if (af == 2) { // IPv6
            struct sockaddr_in6 *clientAddress_act_ipv6 =
                (struct sockaddr_in6 *)&clientAddress_act;
            clientAddress_act_ipv6->sin6_port = htons(port);
            clientAddress_act_ipv6->sin6_family = AF_INET6;
             inet_pton(AF_INET6, ip_address,
                      &(clientAddress_act_ipv6->sin6_addr));
            if (inet_pton(AF_INET6, ip_address,
                          &(clientAddress_act_ipv6->sin6_addr)) <= 0) {
              printf("ERROR on transfering Ipv6.....");
            }
            addr_len = sizeof(struct sockaddr_in6);
          }

          count = snprintf(send_buffer, BUFFER_SIZE,"200 EPRT Command successful\r\n");
          if (count >= 0 && count < BUFFER_SIZE) {
            bytes = send(ns, send_buffer, strlen(send_buffer), 0);
            printf("[DEBUG INFO] <-- %s\n", send_buffer);
          }
          if (bytes < 0) break;
        }
      }
      // ---

      // technically, LIST is different from NLST,but we make them the same here
      if ((strncmp(receive_buffer, "LIST", 4) == 0) || (strncmp(receive_buffer, "NLST", 4) == 0)) {
        printf("===================================================\n");
        printf("\tOpening data socket..... \n");
        s_data_act = socket(clientAddress_act.ss_family, SOCK_STREAM, 0);

        if (s_data_act == INVALID_SOCKET) {
          printf("Error at socket(): %d\n", WSAGetLastError());
      #if defined _WIN32
          WSACleanup(); // only for window
      #endif
        }

        else {
          iResult = connect(s_data_act, (struct sockaddr *)&clientAddress_act, sizeof(clientAddress_act));
          if (iResult != 0) { // connection fail
            count = snprintf(
                send_buffer, BUFFER_SIZE,"425 Data Socket Connection Failure... \r\n");
            if (count >= 0 && count < BUFFER_SIZE) {
              bytes = send(ns, send_buffer, strlen(send_buffer), 0);
              printf("[DEBUG INFO] <-- %s\n", send_buffer);
            }

#if defined __unix__ || defined __APPLE__
            close(s_data_act);
#elif defined _WIN32
            closesocket(s_data_act);
#endif
            break;
          }

          else {
            printf("Connected to client\n");
            count = snprintf( send_buffer, BUFFER_SIZE, "150 opening ASCII mode data connection\n");
            if (count >= 0 && count < BUFFER_SIZE) {
              bytes = send(ns, send_buffer, strlen(send_buffer), 0);
              printf("[DEBUG INFO] <-- %s\n", send_buffer);
            }
          }
#if defined __unix__ || defined __APPLE__
          int i = system("ls -la > tmp.txt"); // save list to a txt file, return
                                              // 0 if success
#elif defined _WIN32
          int i = system("dir > tmp.txt"); // save list to a txt file, return 0 if success
#endif
          printf("The value returned by system() was: %d.\n", i);

          FILE *fin;

          fin = fopen("tmp.txt", "r"); // open tmp.txt file in read mode
          char temp_buffer[500];
          printf("transferring file...\n");
          while (!feof(fin)) {
            strcpy(temp_buffer, "");
            if (fgets(temp_buffer, 498, fin) != NULL) {
              count = snprintf(send_buffer, BUFFER_SIZE, "%s", temp_buffer);
              if (count >= 0 && count < BUFFER_SIZE) {
                if (active == 0){
                  send(ns_data, send_buffer, strlen(send_buffer), 0);
                } else {
                  send(s_data_act, send_buffer, strlen(send_buffer), 0);
                }
              }
            }
          }
          fclose(fin);

#if defined __unix__ || defined __APPLE__
          if (active == 0)
            close(ns_data);
          else
            close(s_data_act);

#elif defined _WIN32
          if (active == 0)
            closesocket(ns_data);
          else
            closesocket(s_data_act);
#endif

          count = snprintf(send_buffer, BUFFER_SIZE,"226 File transfer complete. \r\n");
          if (count >= 0 && count < BUFFER_SIZE) {
            bytes = send(ns, send_buffer, strlen(send_buffer), 0);
            printf("[DEBUG INFO] <-- %s\n", send_buffer);
          }
        }
      }
      //system("del tmp.txt");

      //---
      //=================================================================================
    } // End of COMMUNICATION LOOP per CLIENT
      //=================================================================================

    //********************************************************************
    // CLOSE SOCKET
    //********************************************************************

#if defined __unix__ || defined __APPLE__
    close(ns);
#elif defined _WIN32
    iResult = shutdown(ns, SD_SEND);
    if (iResult == SOCKET_ERROR) {
      printf("shutdown failed with error: %d\n", WSAGetLastError());
      closesocket(ns);
      WSACleanup();
      exit(1);
    }
    closesocket(ns);
#endif
    printf(
        "DISCONNECTED from %s\n",
        inet_ntop(AF_INET6, &clientAddress, send_buffer, sizeof(send_buffer)));

    //====================================================================================
  } // End of MAIN LOOP
  //====================================================================================

  printf("\nSERVER SHUTTING DOWN...\n");

#if defined __unix__ || defined __APPLE__
  close(s); // close listening socket

#elif defined _WIN32
  closesocket(s); // close listening socket
  WSACleanup();   // call WSACleanup when done using the Winsock dll
#endif
  return 0;
}