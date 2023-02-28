
#ifndef  _EOKAS_BASE_SOCKET_H_
#define  _EOKAS_BASE_SOCKET_H_

#include "header.h"

#if _EOKAS_OS == _EOKAS_OS_WIN64 || _EOKAS_OS == _EOKAS_OS_WIN32

#include <WinSock2.h>

#define _null_socket INVALID_SOCKET
#define _closesocket closesocket
using socket_t = SOCKET;
using socklen_t = int;

#elif _EOKAS_OS == _EOKAS_OS_MACOS || _EOKAS_OS == _EOKAS_OS_IOS

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#define _null_socket 0
#define _closesocket close
using socket_t = int;
using socklen_t = socklen_t;

#endif


_BeginNamespace(eokas)
/*
===========================================================
== Protocol, Address, ByteOrder etc.
===========================================================
*/
using AddressFamily = int;
using SocketType = int;
using ProtocolType = int;

class SocketAddress
{
public:
	SocketAddress();
	SocketAddress(const sockaddr& addr);

public:
	SocketAddress& operator=(const sockaddr& addr);
	operator sockaddr() const;

	bool operator==(const SocketAddress& addr);
	bool operator!=(const SocketAddress& addr);

public:
	sockaddr value;
};

class InetAddress :public SocketAddress
{
public:
	InetAddress();
	InetAddress(const sockaddr_in& addr);
	InetAddress(const char* ip, u16_t port);

public:
	InetAddress& operator=(const sockaddr_in& addr);
	operator sockaddr_in() const;

public:
	const char* ip() const;
	u16_t port() const;
};

class ByteOrder
{
public:
	static u16_t h2nI16(u16_t value);
	static u32_t h2nI32(u32_t value);
	static u64_t h2nI64(u64_t value);
	static u32_t h2nF32(f32_t value);
	static u64_t h2nF64(f64_t value);

	static u16_t n2hI16(u16_t value);
	static u32_t n2hI32(u32_t value);
	static u64_t n2hI64(u64_t value);
	static f32_t n2hF32(u32_t value);
	static f64_t n2hF64(u64_t value);
};

/*
===========================================================
== Socket
===========================================================
*/
// Encapsulation for BSD socket api.
class Socket
{
public:
	enum class ShutdownMethod
	{
		Recv = 0,
		Send = 1,
		Both = 2
	};

public:
	Socket();
	Socket(const socket_t& handle);
	~Socket();

public:
	operator socket_t() const;
	bool operator==(const Socket& other) const;
	bool operator!=(const Socket& other) const;

	bool open();
	bool open(AddressFamily family, SocketType socktype, ProtocolType protocol);
	void close();
	void shutdown(ShutdownMethod how);
	bool isOpen() const;
	socket_t handle() const;
	SocketAddress local() const;
	SocketAddress remote() const;
	int setOpt(int level, int name, void* value, socklen_t len);
	int getOpt(int level, int name, void* value, socklen_t* len);
	bool alive() const;

	bool bind(const SocketAddress& addr) const;
	bool listen(int maxconn) const;
	bool connect(const SocketAddress& addr) const;
	Socket accept() const;

	u32_t recv(void* data, u32_t size) const;
	u32_t send(void* data, u32_t size) const;

	u32_t recvFrom(void* data, u32_t size, const SocketAddress& addr);
	u32_t sendTo(void* data, u32_t size, const SocketAddress& addr);

private:
	socket_t mHandle;
};

/*
===========================================================
== SocketOption
===========================================================
*/
// Frequently used socket options
// SocketOptions::broadcase.set(socket, true);
class SocketOptions
{
public:
	template<int level, int name, typename T>
	struct Option
	{
        Option()
        {}
        
		int get(Socket& socket, T& value) const
		{
			socklen_t len = 0;
			return socket.getOpt(level, name, &value, &len);
		}

		int set(Socket& socket, const T& value) const
		{
			return socket.setOpt(level, name, (void*)&value, sizeof(value));
		}
	};

	static const Option<SOL_SOCKET, SO_BROADCAST, bool> broadcast;
	static const Option<SOL_SOCKET, SO_DONTROUTE, bool> dontroute;
	static const Option<SOL_SOCKET, SO_LINGER, linger> linger;
	static const Option<SOL_SOCKET, SO_KEEPALIVE, bool> keepalive;
	static const Option<SOL_SOCKET, SO_OOBINLINE, bool> oobinline;
	static const Option<SOL_SOCKET, SO_REUSEADDR, bool> reuseaddr;	
	static const Option<SOL_SOCKET, SO_RCVTIMEO, int> recvtimeout;
	static const Option<SOL_SOCKET, SO_SNDTIMEO, int> sendtimeout;
	static const Option<SOL_SOCKET, SO_RCVBUF, int> recvbuffersize;
	static const Option<SOL_SOCKET, SO_SNDBUF, int> sendbuffersize;
	static const Option<IPPROTO_TCP, TCP_NODELAY, int> nodelay;
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_NETWORK_H_
