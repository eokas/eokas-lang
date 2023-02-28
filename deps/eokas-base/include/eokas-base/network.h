
#ifndef  _EOKAS_BASE_NETWORK_H_
#define  _EOKAS_BASE_NETWORK_H_

#include "header.h"
#include "socket.h"

_BeginNamespace(eokas)

#define _BUFFER_SIZE 1024

struct NetworkSession;
struct NetworkOperation;

enum class NetworkError
{
	None,
	Expired,
	Broken,	
	Aborted
};

enum class OperationType
{
	None,
	Accept,
	Recv,
	Send,
	Max
};

using NetworkCallback = void(*)(NetworkSession* session, NetworkOperation* oper, NetworkError error);

struct NetworkOperation
{
#if _EOKAS_OS == _EOKAS_OS_WIN64 || _EOKAS_OS == _EOKAS_OS_WIN32
	OVERLAPPED overlapped;
	WSABUF wsabuf;
#endif
	NetworkSession* session;
	OperationType type;
	char data[_BUFFER_SIZE];
	int size;
	Socket acceptedSocket;
};

struct NetworkSession
{
	Socket socket;
	std::list<NetworkOperation*> operations;
	NetworkCallback callback;

	bool open();
	bool open(AddressFamily family, SocketType socktype, ProtocolType protocol);
	bool open(const Socket& socket);
	void close();

	bool listen(const SocketAddress& addr, int maxconn);
	bool connect(const SocketAddress& addr);
	bool post(NetworkOperation* oper);
};

class NetworkService
{
public:
	NetworkService();
	~NetworkService();

public:
	bool init();
	void quit();
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_NETWORK_H_
