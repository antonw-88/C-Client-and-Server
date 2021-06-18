/*
 * client.c
 *
 *  Created on: Jan 19, 2020
 *      Author: anton
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <netdb.h>
#include <ctype.h>

//http://man7.org/linux/man-pages/man3/inet_pton.3.html
//https://stackoverflow.com/questions/791982/determine-if-a-string-is-a-valid-ipv4-address-in-c
bool isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in checkIPstruct;
    int result = inet_pton(AF_INET, ipAddress, &(checkIPstruct.sin_addr));
    return result;
}

int main(int argc, char *argv[]) {

	int simpleSocket = 0, returnStatus = 0;
	struct sockaddr_in simpleServer;
	char messageToServer[1000] = "";
	char buffer[255] = "";
	fprintf(stderr, "Enter a personal ID-number (YYYYMMDD): ");
	scanf("%s", messageToServer);
	struct hostent *host_entry;
	int hostNameCheck = 0; //Sets to 1 if IP is invalid format. Then checks if is valid hostName.
	int fifthDigit = 0; //Control over acceptable digit for second-month-digit
	int seventhDigit = 1; //Control over acceptable digit for second-day-digit
	int personalNumberCheck = 0; //Sets to 1 if personal number is invalid

	for(int i = 0; i < 8; i++) {//Controls that personal number is correctly formated 
		if ((messageToServer[i] == '1' || messageToServer[i] == '2') && i == 0) {
			continue;
		} else if ((messageToServer[i] == '9' || messageToServer[i] == '0') && i == 1) {
			continue;
		} else if (isdigit(messageToServer[i]) != 0 && i == 2) {
			continue;
		} else if (isdigit(messageToServer[i]) != 0 && i == 3) {
			continue;
		} else if ((messageToServer[i] == '1' || messageToServer[i] == '0') && i == 4) {
			if (messageToServer[i] == '1') {
				fifthDigit = 1; //Control for month-specifying second digit
			}
			continue;
		} else if ((messageToServer[i] == '1' || messageToServer[i] == '2' || messageToServer[i] == '0') && i == 5 && fifthDigit == 1) {
			continue;
		} else if (isdigit(messageToServer[i]) != 0 && i == 5 && fifthDigit == 0) {
			if (messageToServer[i] == '0' && messageToServer[i-1] == '0') { // If month is '00' -> invalid format
				personalNumberCheck = 1;
				break;
			}
			continue;
		} else if ((messageToServer[i] == '0' || messageToServer[i] == '1'|| messageToServer[i] == '2'|| messageToServer[i] == '3') && i == 6) {
			if(messageToServer[i] == '0') {
				seventhDigit = 0;
			} else if (messageToServer[i] == '3') {
				seventhDigit = 3;
			}
			continue;
		} else if (messageToServer[i] == '0' && i == 7 && seventhDigit == 0) {
			personalNumberCheck = 1;
			break;
		} else if ((messageToServer[i] != '0' && messageToServer[i] != '1') && i == 7 && seventhDigit == 3) {
			personalNumberCheck = 1;
			break;
		} else if (isdigit(messageToServer[i]) != 0 && i == 7) {
			continue;
		} else {
			personalNumberCheck = 1;
			break;
		}
	}

	if(personalNumberCheck != 0) {
		fprintf(stderr, "Invalid format for personal number. Exit(1).\n");
		exit(1);
	} else {
		fprintf(stderr, "Valid format for personal number.\n");
	}

	if(isValidIpAddress(argv[1])) {
		printf("Host-Server IP-address valid.\n");
	} else { //If returns invalid, check if address is a hostName
		hostNameCheck = 1;
	}

	//Check if hostName is used instead of IP
	if(hostNameCheck == 1) {
		host_entry = gethostbyname(argv[1]);
		//If the argument is not a useable hostName, host_entry will be NULL.
		//If NULL, the server will assume that the IP or hostname provided was is invalid.
		if (host_entry == NULL) {
			printf("The stated HostName or IP-address is invalid.\n");
			exit(1);
		}
		//If address was a useable hostName, replace argv[1] with the hostName's IP.
		argv[1] = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
		printf("Host IP: %s\n", argv[1]);
		if(isValidIpAddress(argv[1])) {
			printf("Host-server IP-address valid.\n");
		} else {
			printf("Host-server IP-address invalid.\n");
			exit(1);
		}
	}

	if(argc != 3) {
		fprintf(stderr, "Usage: <server> <port>\n");
		exit(1);
	}

	simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(simpleSocket == -1) {
		fprintf(stderr, "Could not create socket!\n");
		exit(1);
	} else {
		fprintf(stderr, "Socket created!\n");
	}

	bzero(&simpleServer, sizeof(simpleServer));
	simpleServer.sin_family = AF_INET;
	simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
	simpleServer.sin_port = htons(atoi(argv[2]));

	returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));
	if (returnStatus == 0) {
		fprintf(stderr, "Connection completed!\n");
	} else {
		fprintf(stderr, "Could not connect to address\n");
		close(simpleSocket);
		exit(1);
	}

	write(simpleSocket, messageToServer, strlen(messageToServer));
	returnStatus = read(simpleSocket, buffer, sizeof(buffer));
	if (returnStatus > 0) {
		fprintf(stderr, "PersonalNumber:Name:Grade of person = %s\n", buffer);
	} else {
		fprintf(stderr, "Return status: %i\n", returnStatus);
	}

	close(simpleSocket);
	return 0;
}
