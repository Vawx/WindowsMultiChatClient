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

#include "Server.h"

Server::Server()
{
	m_AddressSize = sizeof(m_SockAddr);
	m_DLLVersion = MAKEWORD( 2,1 );
	m_SuccessConnection = WSAStartup( m_DLLVersion, &m_WinSockData );
	m_ConnnectionSock = socket( AF_INET, SOCK_STREAM, 0 );

	if( inet_pton( AF_INET, IP_ADDRESS, (PVOID*)(&m_SockAddr.sin_addr.s_addr) ) )
	{
		m_SockAddr.sin_family = AF_INET;
		m_SockAddr.sin_port = htons( PORT );

		m_ListenSock = socket( AF_INET, SOCK_STREAM, 0 );
		bind( m_ListenSock, (SOCKADDR*)&m_SockAddr, sizeof(m_SockAddr) );
		listen( m_ListenSock, SOMAXCONN );

		// Init clients
		for( int i = 0; i < MAX_CLIENTS; i++ )
		{
			ClientInfo cInfo;
			cInfo.Channel = "General";
			cInfo.ID = -1;
			cInfo.Name= "DefaultUser";
			cInfo.Socket = INVALID_SOCKET;
			m_ClientList.push_back( cInfo );
		}

		bRunning = true;
		Run( );
	}
}

Server::~Server(){}

void 
Server::Run( )
{
	while( bRunning )
	{
		int tempClientID = 0;

		SOCKET incomingSock = INVALID_SOCKET;
		incomingSock = accept( m_ListenSock, 0,0 );
		if( incomingSock == INVALID_SOCKET )
		{
			continue;
		}

		
		m_ConnectedClients = -1;
		tempClientID = -1;
		for( int i = 0; i < MAX_CLIENTS; i++ )
		{
			// Find empty socket in list
			if( m_ClientList[ i ].Socket == INVALID_SOCKET && tempClientID == -1 )
			{
				m_ClientList[ i ].Socket = incomingSock;
				m_ClientList[ i ].ID = i;
				tempClientID = i;
			}
			if( m_ClientList[ i ].Socket != INVALID_SOCKET )
			{
				m_ConnectedClients += 1;
			}
		}
		
		if( tempClientID != -1 )
		{
			std::cout << "Client #" << m_ClientList[ tempClientID ].ID << " connected" << std::endl;
			m_ClientThreads[ tempClientID ] = std::thread(ProcessClient, std::ref( m_ClientList[ tempClientID ] ), std::ref( m_ClientList ), std::ref( m_ClientThreads[ tempClientID ] ) );
		}
		else
		{
			std::string errorConnectMessage = "Error connecting to server";
			send(incomingSock, errorConnectMessage.c_str(), strlen(errorConnectMessage.c_str()), 0);
			std::cout << errorConnectMessage << std::endl;
		}
	}

	for( int i = 0; i < m_ClientList.size( ); i++ )
	{
		m_ClientThreads[ i ].detach( );
		closesocket( m_ClientList[ i ].Socket );
	}

	closesocket(m_ListenSock);
	WSACleanup( );
	
}

int
ProcessClient( ClientInfo &NewClient, std::vector<ClientInfo> &ClientList, std::thread &ClientThread )
{
	std::string clientMessage = "";
	char tempMessage[ BUFFER_LENGTH ] = "";

	while(1)
	{
		memset( tempMessage, 0, BUFFER_LENGTH );
		if( NewClient.Socket != INVALID_SOCKET )
		{
			int result = recv( NewClient.Socket, tempMessage, BUFFER_LENGTH, 0 );
			if( result != SOCKET_ERROR )
			{
				if( strcmp("", tempMessage ) )
				{
					char command[ sizeof( tempMessage ) ] = "";
					memset( command, 0, sizeof( tempMessage ) );

					if( tempMessage[ 0 ] == '/' )
					{
						for( int i = 1; i < sizeof( tempMessage ); i++ )
						{
							command[ i - 1 ] = tempMessage[ i ];
							if( command[ i - 1 ] == ' ' )
							{
								command[ i - 1 ] = '\0';
								break;
							}
						}
					}

					char setAs[ sizeof( command ) ] = "";
					memset( setAs, 0, sizeof( command ) );
					if( strcmp( command, SETNAME ) == 0 )
					{
						int j = 0;
						for( size_t i = strnlen( command, 512 ) + 2; i < strnlen( tempMessage, 512 ); i++ )
						{
							setAs[ j ] = tempMessage[ i ];
							j++;
						}
						NewClient.Name = setAs;

						std::string clientConnectMessage = "Welcome " + NewClient.Name + "!";
						send(NewClient.Socket, clientConnectMessage.c_str( ), strlen(clientConnectMessage.c_str( )), 0);
					}
					else if ( strcmp( command, SETCHANNEL ) == 0 )
					{
						int j = 0;
						for( size_t i = strnlen( command, BUFFER_LENGTH ) + 2; i < strnlen( tempMessage, BUFFER_LENGTH ); i++ )
						{
							if ( tempMessage[i] != ' ' || j != 0 )
							{
								if( j == 0 )
								{
									char upper = toupper( tempMessage[ i ] );
									setAs[ j ] = upper;
								}
								else
								{
									setAs[j] = tempMessage[i];
								}
								j++;
							}
						}
						std::cout << "Connecting " << NewClient.Name << " to new channel: " << setAs << std::endl;
						NewClient.Channel = setAs;
					}
					else if( strcmp( command, DISCONNECT ) == 0 )
					{
						DisconnectClient( NewClient, ClientList );
						break;
					}
					else
					{
						clientMessage = "[" + NewClient.Channel + "] " + NewClient.Name + ": " + tempMessage;
						std::cout << clientMessage.c_str() << std::endl;

						// Broadcast to other clients
						for (int i = 0; i < MAX_CLIENTS; i++)
						{
							if (ClientList[i].Socket != INVALID_SOCKET)
							{
								if (ClientList[i].Channel == NewClient.Channel)
								{
									result = send(ClientList[i].Socket, clientMessage.c_str(), strlen(clientMessage.c_str()), 0);
								}
							}
						}			
					}						
				}
			}
			else
			{
				DisconnectClient( NewClient, ClientList );
				break;
			}
		}
	}

	ClientThread.detach( );
	return 0;
}

void
DisconnectClient( ClientInfo &NewClient, std::vector<ClientInfo> &ClientList )
{
	std::string clientMessage = "";
	clientMessage = "[" + NewClient.Channel + "] " + NewClient.Name + " has disconnected!";
	std::cout << clientMessage << std::endl;

	closesocket( NewClient.Socket );
	closesocket( ClientList[ NewClient.ID ].Socket );
	ClientList[ NewClient.ID ].Socket = INVALID_SOCKET;
	ClientList[ NewClient.ID ].Channel = "General";
	ClientList[ NewClient.ID ].Name = "DefaultUser";

	for( int i = 0; i < MAX_CLIENTS; i++ )
	{
		if( ClientList[ i ].Socket != INVALID_SOCKET )
		{
			if (ClientList[i].Channel == NewClient.Channel)
			{
				send(ClientList[i].Socket, clientMessage.c_str(), strlen(clientMessage.c_str()), 0);
			}
		}
	}
}