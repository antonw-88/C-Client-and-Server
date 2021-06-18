/*
 * server.c
 *
 *  Created on: Jan 16, 2020
 *      Author: anton
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <strings.h>

//Smitha Dinesh Semwal
//https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
void checkHostName(int hostname)
{
    if (hostname == -1) { //-1 if hostname is not valid
        perror("gethostname"); //Log to stderr if hostname not valid
        exit(1);
    }
}

//Smitha Dinesh Semwal
//https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
void checkHostEntry(struct hostent * hostentry)
{
    if (hostentry == NULL) { //If hostentry could not be succesfully filled, value is NULL
        perror("gethostbyname");
        exit(1);
    }
}

//Smitha Dinesh Semwal
//https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
void checkIPbuffer(char *IPbuffer)
{
    if (NULL == IPbuffer) { //If IPbuffer could not be provided a dotted-decimal IPv4 address
        perror("inet_ntoa");
        exit(1);
    }
}

int main(int argc, char *argv[]) {

	int simpleSocket = 0, returnStatus = 0;
	struct sockaddr_in simpleServer;
	char *messageFromServer;
	char buffer[1000] = "";
	int portNumber = atoi(argv[1]);
	FILE *file;
	char personalNumber[1000];
	char name[1000];
	messageFromServer = "NOTICE: Person does not exist in file.";

	if(argc != 2) {
			fprintf(stderr, "Usage: <port>\n");
			exit(1);
	}

	if(portNumber == 0) {
		fprintf(stderr, "NOTICE: Wildcard port will be used!\n");
	} else if(portNumber > 0 && portNumber < 65536) {
		fprintf(stderr, "Port for listening is within valid range: %i\n", portNumber);
	}

	//The functions for checking IP and Host were created by:
	//Smitha Dinesh Semwal
	//https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
	char hostbuffer[256];
   	char *IPbuffer;
    	struct hostent *host_entry;
    	int hostname;

    	//Retrieve hostname
	hostname = gethostname(hostbuffer, sizeof(hostbuffer));
	checkHostName(hostname);

	//Retrieve host information
    	host_entry = gethostbyname(hostbuffer);
    	checkHostEntry(host_entry);

    	//Converts space-delimited IPv4 addresses
    	//to dotted-decimal format
	IPbuffer = inet_ntoa(*((struct in_addr*)
	                           host_entry->h_addr_list[0]));
	checkIPbuffer(IPbuffer);

	printf("Server IP: %s\n", IPbuffer);
	printf("Server name: %s\n", hostbuffer);

	simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (simpleSocket == -1) {
		fprintf(stderr, "Could not create socket!\n");
		exit(1);
	} else {
		fprintf(stderr, "Socket created!\n");
	}

	//Fill the simpleServer struct with network-data
	bzero(&simpleServer, sizeof(simpleServer));
	simpleServer.sin_family = AF_INET;
	simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
	simpleServer.sin_port = htons(atoi(argv[1]));

	returnStatus = bind(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));
	if (returnStatus == 0) {
		fprintf(stderr, "Bind completed!\n");
	} else {
		fprintf(stderr, "Could not bind to address.\n");
		close(simpleSocket);
		exit(1);
	}

	returnStatus = listen(simpleSocket, 5);
	if (returnStatus == -1) {
		fprintf(stderr, "Could not listen to address!\n");
		close(simpleSocket);
		exit(1);
	} else {
		fprintf(stderr, "Listen complete.\n");
	}

	//Loop enabling multiple connections to be made to the server
	while(1) {
		struct sockaddr_in clientName = { 0 };
		int simpleChildSocket = 0;
		int clientNameLength = sizeof(clientName);
		simpleChildSocket = accept(simpleSocket, (struct sockaddr *)&clientName, (socklen_t *)&clientNameLength);
		if (simpleChildSocket == -1 ) {
			fprintf(stderr, "Cannot accept connection!\n");
			close(simpleSocket);
			exit(1);
		} else {
			fprintf(stderr, "A client connected!\n");
			read(simpleChildSocket, buffer, sizeof(buffer));
			fprintf(stderr, "Message from client: %s\n", buffer);
			if ((file = fopen( "personFile.txt", "r" )) == NULL) {
				fprintf(stderr, "Couldn't open file.");
				messageFromServer = "Couldn't open file.";
			} else { // If file can be opened, then read the personalNumbers of the file
				fprintf(stderr, "File opened successfully.\n");
				//Only use memmove after while has iterated one time.
				//Doing this so that the formating will be correct for the check
				//between the currently scanned personalNumber and the client provided personalNumber.
				int memmoveCheck = 0;
				while(fscanf(file, "%[^:]", personalNumber) == 1) {
					if (memmoveCheck == 0) {
						memmoveCheck = 1;
					} else {
						memmove(personalNumber, personalNumber+1, strlen(personalNumber));
					}
					fscanf(file, "%[^\n]", name);
					int check = strcmp(buffer, personalNumber);
					if (check == 0) { //If correct personalNumber is found, return name and ID to client
						fclose(file);
						strcat(personalNumber, name);
						messageFromServer = personalNumber;
						break;
					}
				}
			}
			write(simpleChildSocket, messageFromServer, strlen(messageFromServer));
			messageFromServer = "NOTICE: PersonalNumber does not exist in file.";
			close(simpleChildSocket);
		}

	}
close(simpleSocket);
return 0;
}
