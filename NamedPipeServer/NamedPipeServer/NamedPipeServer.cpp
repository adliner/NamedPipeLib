#include "NamedPipeServer.h"

namespace PipeServer
{

NamedPipeServer::NamedPipeServer( std::wstring pipeName )
	: 
	m_pipeName( L"\\\\.\\pipe\\" + pipeName ), 
	m_inboundBufferSize( DEFAULT_BUFFER_SIZE ), 
	m_outboundBufferSize( DEFAULT_BUFFER_SIZE ),
	m_bServerShuttingDown( false ),
	m_numInstances( 2 )
{
}


NamedPipeServer::~NamedPipeServer()
{
}

void NamedPipeServer::run()
{
	m_incomingClientDispatherThread = std::move(
		std::thread( &NamedPipeServer::incoimingClientsDispatherMain, this )
	);
}

void NamedPipeServer::stop()
{

}

void NamedPipeServer::incoimingClientsDispatherMain()
{

	BOOL bClientConnected = FALSE;
	m_pipes.resize( m_numInstances );

	for ( uint16_t i = 0; i < m_numInstances; i++ )
	{
		BOOL bPendingIO = FALSE;

		auto hEvent = CreateEventW(
			NULL,    // default security attribute 
			TRUE,    // manual-reset event 
			TRUE,    // initial state = signaled 
			NULL );   // unnamed event object 

		if ( hEvent == NULL )
		{
			printf( "CreateEvent failed with %d.\n", GetLastError() );
			return;
		}

		m_pipes[i].oOverlap.hEvent = hEvent;

		m_vhEvents.push_back( hEvent );

		m_pipes[i].hPipe = CreateNamedPipeW(
			m_pipeName.c_str(),             // pipe name 
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			m_outboundBufferSize,                  // output buffer size 
			m_inboundBufferSize,                  // input buffer size 
			0,                        // client time-out,  0 falls back to 50ms 
			NULL );                    // default security attribute 


		if ( m_pipes[i].hPipe == INVALID_HANDLE_VALUE )
		{
			printf( "CreateNamedPipe failed, GLE=%d.\n", GetLastError() );
			return;
		}

		// Call the subroutine to connect to the new client
		m_pipes[i].bPendingIO = ConnectToNewClient( m_pipes[i].hPipe, &m_pipes[i].oOverlap );

		m_pipes[i].state = m_pipes[i].bPendingIO ?
			PipeState::Connecting : // still connecting 
			PipeState::Reading;     // ready to read 

		//m_pipes.push_back( pipe );

		//if ( pipeState == PipeState::Reading )
		//	break;

	}

	for (uint16_t i = 0; i < m_numInstances; i++)
	{
		std::thread newClientThread = std::move(
			std::thread(&NamedPipeServer::clientHandlerMain, this, m_pipes[i])
		);

		m_incomingClientsHandlerThreads.push_back(std::move(newClientThread));
	}

	while ( !m_bServerShuttingDown )
	{

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		//DWORD cbRet = 0;

		//// Wait for the event object to be signaled, indicating 
		//// completion of an overlapped read, write, or 
		//// connect operation.

		//DWORD dwWait = WaitForMultipleObjects(
		//	m_numInstances,    // number of event objects 
		//	vhEvents.data(),      // array of event objects 
		//	FALSE,        // does not wait for all 
		//	INFINITE );    // waits indefinitely 

		//// dwWait shows which pipe completed the operation. 
		//auto x = GetLastError();
		//DWORD i = dwWait - WAIT_OBJECT_0;  // determines which pipe 

		//if ( i < 0 || i >( m_numInstances - 1 ) )
		//{
		//	printf( "Index out of range.\n" );
		//	return;
		//}

		//// Get the result if the operation was pending. 

		//if ( m_pipes[i].bPendingIO )
		//{
		//	auto fSuccess = GetOverlappedResult(
		//		m_pipes[i].hPipe, // handle to pipe 
		//		&m_pipes[i].oOverlap, // OVERLAPPED structure 
		//		&cbRet,            // bytes transferred 
		//		FALSE );            // do not wait 

		//	switch ( m_pipes[i].state )
		//	{
		//		// Pending connect operation 
		//	case PipeState::Connecting:
		//	{
		//		if ( !fSuccess )
		//		{
		//			printf( "Error %d.\n", GetLastError() );
		//			return;
		//		}
		//		m_pipes[i].state = PipeState::Reading;
		//		char *buff = "dupa";
		//		fSuccess = WriteFile(
		//			m_pipes[i].hPipe,
		//			buff,
		//			5,
		//			&cbRet,
		//			&m_pipes[i].oOverlap );

		//		std::thread newClientThread = std::move(
		//			std::thread( &NamedPipeServer::clientHandlerMain, this, m_pipes[i] )
		//			);

		//		m_incomingClientsHandlerThreads.push_back( std::move( newClientThread ) );
		//		break;
		//	}
		//		// Pending read operation 
		//	case PipeState::Reading:
		//		if ( !fSuccess || cbRet == 0 )
		//		{
		//			DisconnectAndReconnect( i );
		//			continue;
		//		}
		//		m_pipes[i].numBytesRead = cbRet;
		//		m_pipes[i].state = PipeState::Writing;
		//		break;

		//		// Pending write operation 
		//	case PipeState::Writing:
		//		if ( !fSuccess || cbRet != m_pipes[i].numBytesToWrite )
		//		{
		//			DisconnectAndReconnect( i );
		//			continue;
		//		}
		//		m_pipes[i].state = PipeState::Reading;
		//		break;

		//	default:
		//	{
		//		printf( "Invalid pipe state.\n" );
		//		return;
		//	}
		//	}
		//}
	//}

	//if ( bClientConnected )
	//{
	//	printf( "Client connected, creating a processing thread.\n" );

	//	// Create a thread for this client. 
	//	hThread = CreateThread(
	//		NULL,              // no security attribute 
	//		0,                 // default stack size 
	//		InstanceThread,    // thread proc
	//		(LPVOID)hPipe,    // thread parameter 
	//		0,                 // not suspended 
	//		&dwThreadId );      // returns thread ID 

	//	if ( hThread == NULL )
	//	{
	//		_tprintf( TEXT( "CreateThread failed, GLE=%d.\n" ), GetLastError() );
	//		return -1;
	//	}
	//	else CloseHandle( hThread );
	}
}

void NamedPipeServer::DisconnectAndReconnect(Pipe & pipe)
{
	// Disconnect the pipe instance. 

	if ( !DisconnectNamedPipe( pipe.hPipe ) )
	{
		printf( "DisconnectNamedPipe failed with %d.\n", GetLastError() );
	}

	// Call a subroutine to connect to the new client. 

	pipe.bPendingIO = ConnectToNewClient(
		pipe.hPipe,
		&pipe.oOverlap );

	pipe.state = pipe.bPendingIO ?
		PipeState::Connecting : // still connecting 
		PipeState::Reading;     // ready to read 
}

BOOL NamedPipeServer::ConnectToNewClient( HANDLE hPipe, LPOVERLAPPED lpo )
{
	BOOL fConnected, fPendingIO = FALSE;

	// Start an overlapped connection for this pipe instance. 
	fConnected = ConnectNamedPipe( hPipe, lpo );

	// Overlapped ConnectNamedPipe should return zero. 
	if ( fConnected )
	{
		printf( "ConnectNamedPipe failed with %d.\n", GetLastError() );
		return 0;
	}

	switch ( GetLastError() )
	{
		// The overlapped connection in progress. 
	case ERROR_IO_PENDING:
		fPendingIO = TRUE;
		break;

		// Client is already connected, so signal an event. 

	case ERROR_PIPE_CONNECTED:
		if ( SetEvent( lpo->hEvent ) )
			break;

		// If an error occurs during the connect operation... 
	default:
	{
		printf( "ConnectNamedPipe failed with %d.\n", GetLastError() );
		return 0;
	}
	}

	return fPendingIO;
}


void NamedPipeServer::clientHandlerMain( Pipe & pipe )
{
	while (!m_bServerShuttingDown)
	{
		//DWORD dwWait = WaitForSingleObject(
		//	pipe.oOverlap.hEvent,
		//	50);    // waits indefinitely 

		DWORD dwWait = WaitForMultipleObjects(
			1,    // number of event objects 
			&pipe.oOverlap.hEvent,      // array of event objects 
			FALSE,        // does not wait for all 
			INFINITE);    // waits indefinitely 

		if (dwWait == WAIT_TIMEOUT)
			continue;

		switch (dwWait)
		{
		case WAIT_OBJECT_0:
			handleSignaledPipe(pipe);
			break;
		case WAIT_ABANDONED:
			break;
		case WAIT_FAILED:
			break;
		default:
			assert(!"notImplemented");
		}
	}
}

void NamedPipeServer::handleSignaledPipe( Pipe & pipe )
{
	DWORD cbRet = 0;

	if (pipe.bPendingIO)
	{
		auto fSuccess = GetOverlappedResult(
			pipe.hPipe, // handle to pipe 
			&pipe.oOverlap, // OVERLAPPED structure 
			&cbRet,            // bytes transferred 
			FALSE);            // do not wait 

		switch (pipe.state)
		{
			// Pending connect operation 
		case PipeState::Connecting:
		{
			if (!fSuccess)
			{
				printf("Error %d.\n", GetLastError());
				return;
			}
			pipe.state = PipeState::Reading;
			char *buff = "dupa";
			fSuccess = WriteFile(
				pipe.hPipe,
				buff,
				5,
				&cbRet,
				&pipe.oOverlap
			);

			break;
		}
		// Pending read operation 
		case PipeState::Reading:
			if (!fSuccess || cbRet == 0)
			{
				DisconnectAndReconnect(pipe);
				return;
			}
			pipe.numBytesRead = cbRet;
			pipe.state = PipeState::Writing;
			break;

			// Pending write operation 
		case PipeState::Writing:
			if (!fSuccess || cbRet != pipe.numBytesToWrite)
			{
				DisconnectAndReconnect(pipe);
				return;
			}
			pipe.state = PipeState::Reading;
			break;

		default:
		{
			printf("Invalid pipe state.\n");
			return;
		}
		}
	}
}

}