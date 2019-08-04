// NamedPipeServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NamedPipeServer\NamedPipeServer.h"


int main(int argc, char* argv[])
{
	PipeServer::NamedPipeServer pipeserver(L"dupa");

	pipeserver.run();
	for ( ;; ) {}
	return 0;
}

