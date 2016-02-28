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

#include "Client.h"

Client::Client()
{
	m_AddrPtr = 0;
	m_ClientAddrResult = 0;
	m_SentMessage.Data = L"";
	m_ClientInfo = { INVALID_SOCKET, -1, L"" };

	m_Result = WSAStartup( MAKEWORD( 2,2 ), &m_WSAData );
	if( m_Result == 0 )
	{
		ZeroMemory( &m_ClientAddrHints, sizeof( m_ClientAddrHints ) );
		m_ClientAddrHints.ai_family = AF_UNSPEC;
		m_ClientAddrHints.ai_socktype = SOCK_STREAM;
		m_ClientAddrHints.ai_protocol = IPPROTO_TCP;

		m_Result = getaddrinfo( IP_ADDRESS, PORT, &m_ClientAddrHints, &m_ClientAddrResult );
		if( m_Result == 0 )
		{
			for( m_AddrPtr = m_ClientAddrResult; m_AddrPtr != NULL; m_AddrPtr->ai_next )
			{
				m_ClientInfo.Socket = socket( m_AddrPtr->ai_family, m_AddrPtr->ai_socktype, m_AddrPtr->ai_protocol );
				if( m_ClientInfo.Socket != INVALID_SOCKET )
				{
					m_Result = connect( m_ClientInfo.Socket, m_AddrPtr->ai_addr, (int)m_AddrPtr->ai_addrlen );
					if( m_Result != SOCKET_ERROR )
					{						
						if( m_ClientInfo.Socket != INVALID_SOCKET )
						{
							char conversion[ BUFFER_LENGTH ];
							recv( m_ClientInfo.Socket, conversion, BUFFER_LENGTH, 0 );

							if( strlen( conversion ) > 0 )
							{
								// Will have to parse this into two things to verify client went into correct channel with correct name
								m_SentMessage.Data = L"/SETCHANNEL NotGeneral";
								bRunning = true;
								Run( );
							}
						}
					}
					else
					{
						closesocket( m_ClientInfo.Socket );
						m_ClientInfo.Socket = INVALID_SOCKET;
						freeaddrinfo( m_ClientAddrResult );
					}
				}
				else
				{
					WSACleanup( );
				}
			}
		}
		else
		{
			WSACleanup( );
		}
	}
}

Client::~Client(){}

void
Client::Run( )
{
	std::thread runningThread( Process, m_ClientInfo );
	while(bRunning)
	{
		while ( *m_SentMessage.Data != '\0' )
		{
			int messageLength = std::char_traits<wchar_t>::length (m_SentMessage.Data);
			char tBuffer[BUFFER_LENGTH] = "";
			while( messageLength > 0 )
			{
				int count = 0;
				while( *m_SentMessage.Data && messageLength )
				{
					tBuffer[count] = *m_SentMessage.Data++;
					count++;
					messageLength--;
				}
				tBuffer[count] = '\0';				
			}

			int currentSendCount = 0;
			int sendLength = strlen( tBuffer );
			char* sendMessage;
			sendMessage = new char[sizeof(tBuffer)];
			strncpy_s( sendMessage, sizeof( tBuffer ), tBuffer, sizeof( tBuffer ) );
			while( ( m_Result = send( m_ClientInfo.Socket, sendMessage, sendLength, 0 ) ) != sendLength )
			{
				if( m_Result == -1 || currentSendCount == sendLength )
				{
					break;
				}
				sendMessage += currentSendCount;
				sendLength -= currentSendCount;
			}	
			delete(sendMessage);
		}
	}

	runningThread.detach( );

	m_Result = shutdown( m_ClientInfo.Socket, SD_SEND );
	if( m_Result == SOCKET_ERROR )
	{
	}

	closesocket( m_ClientInfo.Socket );
	WSACleanup( );
}

char* 
WCharToChar( wchar_t* Message )
{
	char* conversion = new char[ BUFFER_LENGTH ];
	char defaultRepresentChar = ' ';
	WideCharToMultiByte( CP_ACP, 0, Message, -1, conversion, BUFFER_LENGTH, &defaultRepresentChar, NULL );
	return conversion;
}

void
ClearWCharToChar( char* Message )
{
	delete( Message );
}

wchar_t*
CharToWChar( char* Message )
{
	wchar_t * result = new wchar_t[ strlen( Message )+1 ];
	mbstowcs_s( NULL, result, strlen( Message ) + 1, Message, strlen( Message ) );
	return result;
}

void
ClearCharToWChar( wchar_t* Message )
{
	delete( Message );
}

int
Process( ClientInfo &NewClient )
{
	while( 1 )
	{
		//memset( NewClient.RecievedMessage, 0, BUFFER_LENGTH );
		if( NewClient.Socket != INVALID_SOCKET )
		{
			char recieved[ BUFFER_LENGTH ];
			int result = recv( NewClient.Socket, recieved, BUFFER_LENGTH, 0);
			if( result != SOCKET_ERROR )
			{
				NewClient.RecievedMessage.Data = CharToWChar( recieved );
				std::cout << "recieved: " << recieved << std::endl;
			}
			else
			{
				break;
			}
		}
	}

	if( WSAGetLastError( ) == WSAECONNRESET )
	{
	}
	return 0;
}

