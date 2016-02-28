/***
* Server side of chat client for Jeklynn Heights in game frontend
* Kyle Langley
* Vex Studios, LLC
* 2016
* 
* Used as reference:
* Jacob Roman : VBForums
* http://www.vbforums.com/showthread.php?781947-C-Simple-Chat-Program-In-a-Console-Window-(Winsock-amp-Multithreading)
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this 
* software and associated documentation files (the "Software"), to deal in the Software 
* without restriction, including without limitation the rights to use, copy, modify, merge, 
* publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons 
* to whom the Software is furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all copies or 
* substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
* BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include <sdkddkver.h>
#include <conio.h>
#include <stdio.h>

#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <vector>
#include <thread>

#define SCK_VERSION2 0x0202

#define BUFFER_LENGTH 512

#define IP_ADDRESS "127.0.0.1"
#define PORT 444

// Chat constants to set parameters of client info
const char SETNAME[ BUFFER_LENGTH ] = "SETNAME";
const char SETCHANNEL[ BUFFER_LENGTH ] = "SETCHANNEL";
const char DISCONNECT[ BUFFER_LENGTH ] = "DISCONNECT";

#define SETNAMELENGTH 8

const int MAX_CLIENTS = 2048;

struct ClientInfo
{
	int ID;
	SOCKET Socket;
	std::string Name;
	std::string Channel;
};

int ProcessClient(ClientInfo &NewClient, std::vector<ClientInfo> &ClientList, std::thread &ClientThread);
void DisconnectClient( ClientInfo &NewClient, std::vector<ClientInfo> &ClientList );

class Server
{
public:
	Server();
	~Server();

private:

	void Run( );
	
	bool bRunning;
	int m_AddressSize;
	int m_ConnectedClients;
	long m_SuccessConnection;

	SOCKADDR_IN m_SockAddr;
	SOCKET m_ListenSock;
	SOCKET m_ConnnectionSock;
	WSAData m_WinSockData;
	WORD m_DLLVersion;
	
	std::vector<ClientInfo> m_ClientList;
	std::thread m_ClientThreads[MAX_CLIENTS];
};

