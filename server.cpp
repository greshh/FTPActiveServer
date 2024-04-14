
//=======================================================================================================================
// Course: 159.342
// Description: Cross-platform, Active mode FTP SERVER, Start-up Code for Assignment 1 
//
// This code gives parts of the answers away but this implementation is only IPv4-compliant. 
// Remember that the assignment requires a fully IPv6-compliant cross-platform FTP server that can communicate with a 
// built-in FTP client either in Windows 10, Ubuntu Linux or MacOS.
// 
// This program is cross-platform but your assignment will be marked only in Windows 10.
//
// You must change parts of this program to make it IPv6-compliant (replace all data structures and functions that work only with IPv4).
//
// Hint: The sample TCP server codes show the appropriate data structures and functions that work in both IPv4 and IPv6. 
//       We also covered IPv6-compliant data structures and functions in our lectures.   
//
// Author: n.h.reyes@massey.ac.nz
//=======================================================================================================================

//#define USE_IPV6 false  //if set to false, IPv4 addressing scheme will be used; you need to set this to true to enable IPv6 later on.  The assignment will be marked using IPv6!
#define USE_IPV6 true  // enable Ipv6
#define InetNtopA inet_ntop

#if defined __unix__ || defined __APPLE__
  #include <unistd.h>
  #include <errno.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <netdb.h> //used by getnameinfo()
  #include <iostream>
  #include <cstring>

#elif defined __WIN32__
  #include <winsock2.h>
  #include <ws2tcpip.h> //required by getaddrinfo() and special constants
  #include <stdlib.h>
  #include <stdio.h>
  #include <iostream>
  #include <string>
 // For WSSTATUP(WINDOW) //
  #define WSVERS MAKEWORD(2,2) /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
                    //The high-order byte specifies the minor version number; 
                    //the low-order byte specifies the major version number.
  WSADATA wsadata; //Create a WSADATA object called wsadata.
  // For WSSTATUP(WINDOW) //
#endif

#define BUFFER_SIZE 256
enum class FileType{BINARY, TEXT, UNKNOWN};

FileType file_type;




// Reconnection after Error
void reconnect(int& s, sockaddr_in6& localaddr){
    //close the current socket(In Error)
#if defined __unix__ || defined __APPLE__
    close(s);
#elif defined _WIN32
    closesocket(s);
#endif

    // reconnected to a new socket
    s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
#if defined __unix__ || defined __APPLE__ // for cross planform
    if (s <0) {
         			 printf("socket failed\n");
         		 }
#elif defined _WIN32
    //check for errors in socket allocation
    if (s == INVALID_SOCKET) {
        printf("Error at socket(): %d\n", WSAGetLastError());
        //freeaddrinfo(result);
        WSACleanup();
        exit(1);//return 1;
    }
#endif

    if (bind(s,(struct sockaddr *)(&localaddr),sizeof(localaddr)) != 0) { //old programming style, needs replacing
        printf("Bind failed!\n");
        exit(0);
    }

    listen(s, 5);
}

//********************************************************************
//MAIN
//********************************************************************
int main(int argc, char *argv[]) {
//********************************************************************
// INITIALIZATION
//********************************************************************
	   
file_type = FileType::UNKNOWN;

#if defined __unix__ || defined __APPLE__
   //nothing to do here

#elif defined _WIN32

	   int err = WSAStartup(WSVERS, &wsadata);

		 if (err != 0) {
		      WSACleanup();
			  // Tell the user that we could not find a usable WinsockDLL
		      printf("WSAStartup failed with error: %d\n", err);
		      exit(1);
		 }
#endif


    // struct sockaddr_in localaddr,remoteaddr;  //ipv4 only, needs replacing
    // struct sockaddr_in local_data_addr_act;   //ipv4 only, needs replacing
    struct sockaddr_in6 localaddr,remoteaddr;  //changed to ipv6-compatible customized struct
    struct sockaddr_in6 local_data_addr_act;   //changed to ipv6-compatible customized struct

#if defined __unix__ || defined __APPLE__

		 int s,ns;                //socket declaration
		 int ns_data, s_data_act; //socket declaration
#elif defined _WIN32			 
		 SOCKET s,ns;                //socket declaration
		 SOCKET ns_data, s_data_act; //socket declaration
#endif

		 char send_buffer[BUFFER_SIZE],receive_buffer[BUFFER_SIZE];
		
         // ns_data=INVALID_SOCKET;

#if defined __unix__ || defined __APPLE__
		  ns_data = -1;
#elif defined _WIN32
		  ns_data= INVALID_SOCKET;
#endif

		 int active=0;
		 int n,bytes,addrlen;
		 
		 printf("\n============================================\n");
		 printf("   << 159.342 Cross-platform FTP Server Ver 1.2 >>");
		 printf("\n============================================\n");
		 printf("   submitted by: Greshka Lao, Any (Hoi Yi) Kwok");
		 printf("\n       due date: 22/04/24");
		 printf("\n============================================\n");
	
		 
		 memset(&localaddr,0,sizeof(localaddr));//clean up the structure
		 memset(&remoteaddr,0,sizeof(remoteaddr));//clean up the structure
		 
//********************************************************************
//SOCKET
//********************************************************************

        /* Not Ipv6 and Not cross-platform
         s = socket(AF_INET, SOCK_STREAM, 0); //old programming style, needs replacing
         if (s <0) {
			 printf("socket failed\n");
		 }
		 localaddr.sin_family = AF_INET;
         */

		 // set the socket address structure.
		 struct addrinfo *result = NULL;
		 struct addrinfo hints;
		 int iResult;

		 memset(&hints, 0, sizeof(struct addrinfo));

		 if (USE_IPV6) {
			 hints.ai_family = AF_INET6;
		 } else { // IPV4
			 hints.ai_family = AF_INET;
		 }

		 hints.ai_socktype = SOCK_STREAM;
		 hints.ai_protocol = IPPROTO_TCP;

		 char portNum[NI_MAXSERV];

		 s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP); // changed to Ipv6 and enforce  TCP Proto

         #if defined __unix__ || defined __APPLE__ // for cross planform
             if (s <0) {
         			 printf("socket failed\n");
         		 }
         #elif defined _WIN32
            //check for errors in socket allocation
                     if (s == INVALID_SOCKET) {
                          printf("Error at socket(): %d\n", WSAGetLastError());
                          //freeaddrinfo(result);
                          WSACleanup();
                          exit(1);//return 1;
                      }
         #endif
         localaddr.sin6_family = AF_INET6;// changed to Ipv6

		 //CONTROL CONNECTION:  port number = content of argv[1]
		 if (argc == 2) {
			 //localaddr.sin_port = htons((u_short)atoi(argv[1]));
             // ipv4 only, needs replacing. In our lectures, we have an elegant way of resolving the local address and port to be used by the server.
             // (I don't know whatway he said so I just simplily change the code to Ipv6's one...... in the same style.... )
             localaddr.sin6_port = htons((u_short)atoi(argv[1]));

			 iResult = getaddrinfo(NULL, argv[1], &hints, &result);
			 sprintf(portNum, "%s", argv[1]);
			 printf("\nargv[1] = %s\n", argv[1]);
		 }
		 else {
			 //localaddr.sin_port = htons(1234);//default listening port //ipv4 only, needs replacing
             localaddr.sin6_port = htons(1234); // Default listening port

			 iResult = getaddrinfo(NULL, "1234", &hints, &result); 
			 sprintf(portNum, "%s", "1234");
		 }
		 //localaddr.sin_addr.s_addr = INADDR_ANY;//server address should be local, old programming style, needs replacing
         localaddr.sin6_addr = in6addr_any; // Bind to all available interfaces
		 
//********************************************************************
//BIND
//********************************************************************
		 iResult = bind(s, result->ai_addr, (int)result->ai_addrlen);

		 //if error is detected, then clean-up
#if defined __unix__ || defined __APPLE__
   if (iResult == -1) {
      printf( "\nbind failed\n"); 
      freeaddrinfo(result);
      close(s);//close socket
#elif defined _WIN32
    if (iResult == SOCKET_ERROR) {
      printf("bind failed with error: %d\n", WSAGetLastError());
      freeaddrinfo(result);
      closesocket(s);
      WSACleanup();
#endif       
      exit(0);
    }
		 
//********************************************************************
//LISTEN
//********************************************************************
		 listen(s,5);
		
//********************************************************************
//INFINITE LOOP
//********************************************************************
		 int count=0;
         int reconnectCount = 0;
         const int maxReconnect = 5;
		 //====================================================================================
		 while (1) {//Start of MAIN LOOP
		 //====================================================================================
			    addrlen = sizeof(remoteaddr);
//********************************************************************
//NEW SOCKET newsocket = accept  //CONTROL CONNECTION
//********************************************************************
			 printf("\n------------------------------------------------------------------------\n");
			 //printf("SERVER is waiting for an incoming connection request at port:%d", ntohs(localaddr.sin_port));
             printf("SERVER is waiting for an incoming connection request at port:%d", ntohs(localaddr.sin6_port));
			 printf("\n------------------------------------------------------------------------\n");

#if defined __unix__ || defined __APPLE__ 
			 ns = accept(s,(struct sockaddr *)(&remoteaddr), (socklen_t*)&addrlen); 
#elif defined _WIN32 
			 ns = accept(s,(struct sockaddr *)(&remoteaddr), &addrlen); 
#endif

			 //if (ns < 0 ) break;
             if (ns<0) {
                 if (reconnectCount < maxReconnect){
                     reconnectCount++;
                     printf("Reconnecting : %d/%d attemps\n", reconnectCount, maxReconnect);
                     reconnect(reinterpret_cast<int &>(s), localaddr);
                     continue;
                 } else {
                     printf("Maximum Reconnection Reached. Automatically Exit.\n");
                     break;
                 }
             } else {
                 reconnectCount=0;
             }
				 
			 printf("\n============================================================================\n");
	 		 //printf("connected to [CLIENT's IP %s , port %d] through SERVER's port %d",
			 //        inet_ntoa(remoteaddr.sin_addr),ntohs(remoteaddr.sin_port),ntohs(localaddr.sin_port)); //ipv4 only, needs replacing
             printf("connected to [CLIENT's IP %s , port %d] through SERVER's port %d",
                    inet_ntop(AF_INET6, &remoteaddr.sin6_addr, send_buffer, sizeof(send_buffer)),
                    ntohs(remoteaddr.sin6_port),ntohs(localaddr.sin6_port));
			 printf("\n============================================================================\n");
			 //printf("detected CLIENT's port number: %d\n", ntohs(remoteaddr.sin_port));

			 //printf("connected to CLIENT's IP %s at port %d of SERVER\n",
			 //    inet_ntoa(remoteaddr.sin_addr),ntohs(localaddr.sin_port));
			 
			 //printf("detected CLIENT's port number: %d\n", ntohs(remoteaddr.sin_port));
//********************************************************************
//Respond with welcome message
//*******************************************************************
			 count=snprintf(send_buffer,BUFFER_SIZE,"220 FTP Server ready. \r\n");
			 if(count >=0 && count < BUFFER_SIZE){
			 	 bytes = send(ns, send_buffer, strlen(send_buffer), 0);

			 }

			//********************************************************************
			//COMMUNICATION LOOP per CLIENT
			//********************************************************************

			 while (1) {
				 
				 n = 0;

				 
				 while (1) {
//********************************************************************
//RECEIVE MESSAGE AND THEN FILTER IT
//********************************************************************
				   bytes = recv(ns, &receive_buffer[n], 1, 0);//receive byte by byte...

					 if ((bytes < 0) || (bytes == 0)) break;
					 if (receive_buffer[n] == '\n') { /*end on a LF*/
						 receive_buffer[n] = '\0';
						 break;
					 }
					 if (receive_buffer[n] != '\r') n++; /*Trim CRs*/
				 
				 } 
				 
                 if(bytes == 0) printf("\nclient has gracefully exited.\n"); //2022

				 if ((bytes < 0) || (bytes == 0)) break;

				 printf("[DEBUG INFO] command received:  '%s\\r\\n' \n", receive_buffer);

//********************************************************************
//PROCESS COMMANDS/REQUEST FROM USER
//********************************************************************	
				 char username[BUFFER_SIZE];
				 char password[BUFFER_SIZE];
				  			 
				 if (strncmp(receive_buffer,"USER",4)==0)  {
					 // this resets the char buffer of both variables.
					 memset(username, '\0', sizeof(username));
					 memset(password, '\0', sizeof(password));
					 int i = 5;
					 while (receive_buffer[i] != '\0' && i < BUFFER_SIZE) {
						username[i-5] = receive_buffer[i];
						i++;
					 }
					 printf("%s\n", username);
					 printf("Logging in... \n");
					 count=snprintf(send_buffer,BUFFER_SIZE,"331 Password required \r\n");
					 if(count >=0 && count < BUFFER_SIZE){
					    bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 }
					 printf("[DEBUG INFO] <-- %s\n", send_buffer);
					 if (bytes < 0) break;
				 }
				 //---
				 if (strncmp(receive_buffer,"PASS",4)==0)  {
					 int i = 5;
					 while (receive_buffer[i] != '\0' && i < BUFFER_SIZE) {
						password[i-5] = receive_buffer[i];
						i++;
					 }
					 printf("%s\n", password);
					 // we are only allowing one user to log in successfully, as per the details below:
					 char user[] = "napoleon";
					 char pass[] = "342";
					 if (strcmp(username, user) == 0 && strcmp(password, pass) == 0) { // both username and password have to be correct in order to sign in.
						 count = snprintf(send_buffer, BUFFER_SIZE, "230 Public login sucessful \r\n");
					 } else {
						 count = snprintf(send_buffer, BUFFER_SIZE, "530-User cannot log in. \r\n");
					 }				 
					 if(count >=0 && count < BUFFER_SIZE){
					    bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 }
					 printf("[DEBUG INFO] <-- %s\n", send_buffer);
					 if (!(strcmp(username, user) == 0 && strcmp(password, pass) == 0)) { break; }
					 if (bytes < 0) break;
				 }
				 //---
				 if (strncmp(receive_buffer,"SYST",4)==0)  {
					 printf("Information about the system \n");
					 count=snprintf(send_buffer,BUFFER_SIZE,"215 Windows 64-bit\r\n");					 
					 if(count >=0 && count < BUFFER_SIZE){
					    bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 }
					 printf("[DEBUG INFO] <-- %s\n", send_buffer);
					 if (bytes < 0) break;
				 }
				 
				 //---
				 if (strncmp(receive_buffer,"TYPE",4)==0)  {

				 	     bytes=0;
				       printf("<--TYPE command received.\n\n");

				       char objType;
				       int scannedItems = sscanf(receive_buffer, "TYPE %c", &objType);
				       if(scannedItems < 1) {
				       				count=snprintf(send_buffer,BUFFER_SIZE,"501 Syntax error in arguments\r\n");
				       				if(count >=0 && count < BUFFER_SIZE){
				                 bytes = send(ns, send_buffer, strlen(send_buffer), 0);  
				              }
				              printf("[DEBUG INFO] <-- %s\n", send_buffer);           
				              if (bytes < 0) break;             
				       }

				       switch(toupper(objType)){
                      case 'I':  file_type = FileType::BINARY;
                      	         printf("using binary mode to transfer files.\n");
																 count=snprintf(send_buffer,BUFFER_SIZE,"200 command OK.\r\n");																 
																 if(count >=0 && count < BUFFER_SIZE){
																    bytes = send(ns, send_buffer, strlen(send_buffer), 0);
																 }
																 printf("[DEBUG INFO] <-- %s\n", send_buffer);
																 if (bytes < 0) break;

																 break;
											case 'A':  file_type = FileType::TEXT;
                      	         printf("using ASCII mode to transfer files.\n");
																 count=snprintf(send_buffer,BUFFER_SIZE,"200 command OK.\r\n");																 
																 if(count >=0 && count < BUFFER_SIZE){
																   bytes = send(ns, send_buffer, strlen(send_buffer), 0);
																 }
																 printf("[DEBUG INFO] <-- %s\n", send_buffer);
																 
																 if (bytes < 0) break;

																 break;		
											default:
																count=snprintf(send_buffer,BUFFER_SIZE,"501 Syntax error in arguments\r\n");
																if(count >=0 && count < BUFFER_SIZE){
				              					   bytes = send(ns, send_buffer, strlen(send_buffer), 0);  
				              					}
				              					printf("[DEBUG INFO] <-- %s\n", send_buffer);           
				              				  if (bytes < 0) break;  
				              				  break;
				         } 					 			 
					 
				 }
				 //---
				 if (strncmp(receive_buffer,"STOR",4)==0)  {
					 printf("unrecognised command \n");
					 count=snprintf(send_buffer,BUFFER_SIZE,"502 command not implemented\r\n");					 
					 if(count >=0 && count < BUFFER_SIZE){
					    bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 }
					 printf("[DEBUG INFO] <-- %s\n", send_buffer);
					 if (bytes < 0) break;
				 }
				 //---
				 if (strncmp(receive_buffer,"RETR",4)==0)  {
					 printf("unrecognised command \n");
					 count=snprintf(send_buffer,BUFFER_SIZE,"502 command not implemented\r\n");					 
					 if(count >=0 && count < BUFFER_SIZE){
					    bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 }
					 printf("[DEBUG INFO] <-- %s\n", send_buffer);
					 if (bytes < 0) break;
				 }
				 //---
				 if (strncmp(receive_buffer,"OPTS",4)==0)  {
					 count = snprintf(send_buffer, BUFFER_SIZE, "550 unrecognized command\r\n");
					 if(count >=0 && count < BUFFER_SIZE){
					    bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 }
					 printf("[DEBUG INFO] <-- %s\n", send_buffer);
					 if (bytes < 0) break;
				 }
				 //---
				 if (strncmp(receive_buffer,"EPRT",4)==0)  {  //more work needs to be done here
					 printf("unrecognised command \n");
					 count=snprintf(send_buffer,BUFFER_SIZE,"502 command not implemented\r\n");					 
					 if(count >=0 && count < BUFFER_SIZE){
					    bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 }
					 printf("[DEBUG INFO] <-- %s\n", send_buffer);
					 if (bytes < 0) break;
				 }
				 //---
				 if (strncmp(receive_buffer,"CWD",3)==0)  {
					 printf("unrecognised command \n");
					 count=snprintf(send_buffer,BUFFER_SIZE,"502 command not implemented\r\n");					 
					 if(count >=0 && count < BUFFER_SIZE){
					   bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 }
					 printf("[DEBUG INFO] <-- %s\n", send_buffer);
					 if (bytes < 0) break;
				 }
				 //---
				 if (strncmp(receive_buffer,"QUIT",4)==0)  {
					 printf("Quit \n");
					 count=snprintf(send_buffer,BUFFER_SIZE,"221 Connection close by client\r\n");					 
					 if(count >=0 && count < BUFFER_SIZE){
					    bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					 }
					 printf("[DEBUG INFO] <-- %s\n", send_buffer);
					 if (bytes < 0) break;
					 
				 }
				 //---
				 if(strncmp(receive_buffer,"PORT",4)==0) {
					 s_data_act = socket(AF_INET, SOCK_STREAM, 0);
					 //local variables
					 //unsigned char act_port[2];
					 int act_port[2];
					 int act_ip[4], port_dec;
					 char ip_decimal[NI_MAXHOST];
					 printf("===================================================\n");
					 printf("\tActive FTP mode, the client is listening... \n");
					 active=1;//flag for active connection
					 
					 int scannedItems = sscanf(receive_buffer, "PORT %d,%d,%d,%d,%d,%d",
							&act_ip[0],&act_ip[1],&act_ip[2],&act_ip[3],
					      &act_port[0],&act_port[1]);
					 
					 if(scannedItems < 6) {
		       	count=snprintf(send_buffer,BUFFER_SIZE,"501 Syntax error in arguments \r\n");						
		       	if(count >=0 && count < BUFFER_SIZE){
						   bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					  }
						printf("[DEBUG INFO] <-- %s\n", send_buffer);
					  if (bytes < 0) break;
			      
		       }
					 
					 /*
                     local_data_addr_act.sin_family=AF_INET;//local_data_addr_act  //ipv4 only, needs to be replaced.
					 count=snprintf(ip_decimal,NI_MAXHOST, "%d.%d.%d.%d", act_ip[0], act_ip[1], act_ip[2],act_ip[3]);
					  */
                     local_data_addr_act.sin6_family=AF_INET6;//local_data_addr_act
					 count=snprintf(ip_decimal,NI_MAXHOST, "%d.%d.%d.%d", act_ip[0], act_ip[1], act_ip[2],act_ip[3]);

					 if(!(count >=0 && count < BUFFER_SIZE)) break;

                     /*
					 printf("\tCLIENT's IP is %s\n",ip_decimal);  //IPv4 format
					 local_data_addr_act.sin_addr.s_addr=inet_addr(ip_decimal);  //ipv4 only, needs to be replaced.
					 port_dec=act_port[0];
					 port_dec=port_dec << 8;
					 port_dec=port_dec+act_port[1];
					 printf("\tCLIENT's Port is %d\n",port_dec);
					 printf("===================================================\n");
					 
					 local_data_addr_act.sin_port=htons(port_dec); //ipv4 only, needs to be replaced
                      */

                     // Print the client's IP address (IPv6 format)
                     char ip_buffer[INET6_ADDRSTRLEN]; // Buffer to hold the IPv6 address string
                     inet_ntop(AF_INET6, &local_data_addr_act.sin6_addr, ip_buffer, INET6_ADDRSTRLEN);
                     printf("connected to [CLIENT's IP %s , port %d] through SERVER's port %d\n",
                            ip_buffer, ntohs(remoteaddr.sin6_port), ntohs(localaddr.sin6_port));
                     printf("===================================================\n");
                     local_data_addr_act.sin6_port=htons(port_dec);


           // Note: the following connect() function is not correctly placed.  It works, but technically, as defined by
           // the protocol, connect() should occur in another place.  Hint: carefully inspect the lecture on FTP, active operations 
           // to find the answer. 
					 if (connect(s_data_act, (struct sockaddr *)&local_data_addr_act, (int) sizeof(struct sockaddr)) != 0){
						 //printf("trying connection in %s %d\n",inet_ntoa(local_data_addr_act.sin_addr),ntohs(local_data_addr_act.sin_port));
                         printf("trying connection in %s %d\n",
                                inet_ntop(AF_INET6, &local_data_addr_act.sin6_addr, ip_buffer, INET6_ADDRSTRLEN),
                                ntohs(local_data_addr_act.sin6_port));
                         count=snprintf(send_buffer,BUFFER_SIZE, "425 Something is wrong, can't start active connection... \r\n");
						 if(count >=0 && count < BUFFER_SIZE){
						   bytes = send(ns, send_buffer, strlen(send_buffer), 0);

						   printf("[DEBUG INFO] <-- %s\n", send_buffer);
						 }

#if defined __unix__ || defined __APPLE__ 
						 close(s_data_act);
#elif defined _WIN32	
						 closesocket(s_data_act);
#endif	 

					 }
					 else {
						 count=snprintf(send_buffer,BUFFER_SIZE, "200 PORT Command successful\r\n");
						 if(count >=0 && count < BUFFER_SIZE){
						   bytes = send(ns, send_buffer, strlen(send_buffer), 0);
						   printf("[DEBUG INFO] <-- %s\n", send_buffer);
						   printf("Connected to client\n");
						 }
					 }

				 }
				 //---				 

				 //technically, LIST is different than NLST,but we make them the same here
				 if ( (strncmp(receive_buffer,"LIST",4)==0) || (strncmp(receive_buffer,"NLST",4)==0))   {
#if defined __unix__ || defined __APPLE__ 

					int i=system("ls -la > tmp.txt");//change that to 'dir', so windows can understand
			
#elif defined _WIN32	

					 int i=system("dir > tmp.txt");
#endif					 
					 printf ("The value returned by system() was: %d.\n",i);

					 FILE *fin;

           fin=fopen("tmp.txt","r");//open tmp.txt file

					 snprintf(send_buffer,BUFFER_SIZE,"125 Transfering... \r\n");
					 snprintf(send_buffer,BUFFER_SIZE,"150 Opening ASCII mode data connection... \r\n");
					 count=snprintf(send_buffer,BUFFER_SIZE,"150 Opening data connection... \r\n");
					 if(count >=0 && count < BUFFER_SIZE){					  
					    bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					    printf("[DEBUG INFO] <-- %s\n", send_buffer);
					 }  
					 char temp_buffer[80];
					 printf("transferring file...\n");
					 while (!feof(fin)){
					 	 strcpy(temp_buffer,"");
						 if(fgets(temp_buffer,78,fin) != NULL){

							 count=snprintf(send_buffer,BUFFER_SIZE,"%s",temp_buffer);
							 if(count >=0 && count < BUFFER_SIZE){					   
							 
								 if (active==0) 
								 	 send(ns_data, send_buffer, strlen(send_buffer), 0);
								 else 
								 	 send(s_data_act, send_buffer, strlen(send_buffer), 0);
							 }
						 }
					 }
           
					 fclose(fin);					 
					 count=snprintf(send_buffer,BUFFER_SIZE,"226 File transfer complete. \r\n");					 
					 if(count >=0 && count < BUFFER_SIZE){					  
					   bytes = send(ns, send_buffer, strlen(send_buffer), 0);
					   printf("[DEBUG INFO] <-- %s\n", send_buffer);
					 }

#if defined __unix__ || defined __APPLE__ 
					 if (active==0 )close(ns_data);
					 else close(s_data_act);

#elif defined _WIN32					 
					 if (active==0 )
					 		closesocket(ns_data);
					 else 
					 		closesocket(s_data_act);

					 //OPTIONAL, delete the temporary file
					 //system("del tmp.txt");
#endif	 
					 
					 
				 }
                 //---			    
			 //=================================================================================	 
			 }//End of COMMUNICATION LOOP per CLIENT
			 //=================================================================================
			 
//********************************************************************
//CLOSE SOCKET
//********************************************************************
			 
#if defined __unix__ || defined __APPLE__ 
			close(ns);
#elif defined _WIN32	

			closesocket(ns);
#endif
			//printf("DISCONNECTED from %s\n",inet_ntoa(remoteaddr.sin_addr)); //IPv4 only, needs replacing
            printf("DISCONNECTED from %s\n",inet_ntop(AF_INET6, &remoteaddr.sin6_addr, send_buffer, sizeof(send_buffer)));

			 
		 //====================================================================================
		 } //End of MAIN LOOP
		 //====================================================================================
		 
		  printf("\nSERVER SHUTTING DOWN...\n");

#if defined __unix__ || defined __APPLE__ 
		 close(s);

#elif defined _WIN32		 
		 closesocket(s);
		 WSACleanup();
#endif		 
		 return 0;
		 
}

