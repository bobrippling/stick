#ifndef SOCKET_H
#define SOCKET_H

class Socket
{
	public:
		enum State
		{
			CONNECTED,
			LISTENING,
			CONNECTING,
			IDLE
		};

		virtual ~Socket();
		Socket();
		Socket(const Socket&);
		virtual Socket& operator=(const Socket&);

		bool connect(const char *host, int);
		bool connect(const std::string& host, int);

		bool listen(int port);
		bool listen(int port, const char *);

		void disconnect();

		Socket *accept();
		bool    accept(Socket&);

		bool senddata(const void *, size_t);
		bool senddata(const char *);
		bool senddata(const char);
		bool senddata(const std::string&);
		Socket& operator<<(const char *);
		Socket& operator<<(const char);
		Socket& operator<<(const std::string&);

		bool recvdata(std::string&);
		bool recvdata(char *, int);

		enum State getstate();
		const char *remoteaddr();
		const char *lasterr() const;

	private:
		Socket(     int, struct sockaddr_in *, enum State = CONNECTED);
		void reinit(int, struct sockaddr_in *, enum State = CONNECTED);

		void cleanup();
		bool setblocking(bool) const;
		bool senddata(void *, size_t);

		int fd;
		enum State state;
		struct sockaddr_in addr;

		const char *lerr;
};

#endif
