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

		bool senddata(const char *);
		bool senddata(char);
		bool senddata(std::string&);
		Socket& operator<<(const char *);
		Socket& operator<<(char);
		Socket& operator<<(std::string&);

		bool recvdata(std::string&) const;
		bool recvdata(char *, int) const;

		enum State getstate();
		const char *remoteaddr();
		const char *lasterr() const;

	private:
		Socket(     int, struct sockaddr_in *, enum State = CONNECTED);
		void reinit(int, struct sockaddr_in *, enum State = CONNECTED);

		void cleanup();
		bool setblocking(bool) const;
		inline bool senddata(void *, size_t);

		int fd;
		enum State state;
		struct sockaddr_in addr;

		const char *lerr;
};

#endif
