#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#define DEF_BUF_SIZE 256

class UDPSocket : public Addressable
{
	private:
		bool setblocking(bool) const;

		const char *lerr;
		char *buffer;
		unsigned int buffersize;
		int fd;

		UDPSocket& operator=(const UDPSocket&);
		UDPSocket(const UDPSocket&);

	public:
		UDPSocket();
		virtual ~UDPSocket();

		bool init(const char *host, const char *port, int bs = DEF_BUF_SIZE);
		bool bind(const char *port);

		bool senddata(const void *, size_t , struct sockaddr *, socklen_t);
		bool senddata(const void *, size_t);

		bool recvdata(void *, size_t, struct sockaddr *, socklen_t *);
		bool recvdata(void *, size_t);

		const char *remoteaddr();
		const char *lasterr() const;

		bool valid() const;
};

#undef DEF_BUF_SIZE

#endif
