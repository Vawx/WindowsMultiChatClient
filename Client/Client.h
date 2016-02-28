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
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <stdio.h>
#include <iostream>
#include <string>
#include <thread>

#pragma comment (lib, "Ws2_32.lib")

#define BUFFER_LENGTH 512
const LPCSTR IP_ADDRESS = "127.0.0.1";
const LPCSTR PORT = "444";

// UDK String type
struct FString
{
	wchar_t *Data;
	int ArrayNum;
	int ArrayMax;
};

struct ClientInfo
{
	SOCKET Socket;
	int ID;
	FString RecievedMessage;
};

int Process( ClientInfo &NewClient );
void SendChatMessage( wchar_t* Message );
char* WCharToChar( wchar_t* Message );
wchar_t* CharToWChar( char* Message );
void ClearWCharToChar( char* Message );
void ClearCharToWChar( wchar_t* Message );

class Client
{
public:
	Client();
	~Client();

private:
	void Run( );
	bool bRunning;

	FString m_SentMessage;

	ClientInfo m_ClientInfo;

	int m_Result;
	
	WSADATA m_WSAData;
	struct addrinfo *m_ClientAddrResult;
	struct addrinfo *m_AddrPtr;
	struct addrinfo m_ClientAddrHints;
};

