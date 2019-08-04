#pragma once
#include <string>
#include <vector>
#include <thread>
#include <stdint.h>
#include <assert.h>
#include <Windows.h>


// based on example https://msdn.microsoft.com/en-us/library/windows/desktop/aa365603(v=vs.85).aspx
namespace PipeServer
{

const uint32_t DEFAULT_BUFFER_SIZE = 4 * 1024;

class NamedPipeServer
{
public:
	NamedPipeServer( std::wstring pipeName );
	virtual ~NamedPipeServer();

	void run();
	void stop();

private:
	enum class PipeState
	{
		Connecting,
		Reading,
		Writing
	};
	struct Pipe 
	{
		HANDLE hPipe;
		OVERLAPPED oOverlap;
		BOOL bPendingIO;
		PipeState state;
		uint32_t numBytesRead;
		uint32_t numBytesToWrite;
	};

	void incoimingClientsDispatherMain();
	void clientHandlerMain( Pipe & pipe );
	void handleSignaledPipe(Pipe & pipe);
	void DisconnectAndReconnect( Pipe & pipe);
	BOOL ConnectToNewClient( HANDLE hPipe, LPOVERLAPPED lpo );

	std::thread m_incomingClientDispatherThread;
	std::vector<std::thread> m_incomingClientsHandlerThreads;
	bool m_bServerShuttingDown;

	std::wstring m_pipeName;

	uint32_t m_inboundBufferSize;
	uint32_t m_outboundBufferSize;

	uint16_t m_numInstances;
	std::vector<Pipe> m_pipes;
	std::vector<HANDLE> m_vhEvents;
};

}