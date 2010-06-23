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

		bool listen();
		bool listen(const char *);

		void disconnect();


		bool senddata(const char *) const;
		bool senddata(char) const;
		bool senddata(std::string&) const;
		Socket& operator<<(const char *);
		Socket& operator<<(char);
		Socket& operator<<(std::string&);

		bool recvdata(std::string&) const;
		bool recvdata(char *, int) const;

		enum State getstate();
		const char *lasterr() const;

	private:
		void cleanup();
		bool setblocking(bool) const;

		int fd;
		enum State state;
		struct sockaddr_in hostaddr;

		const char *lerr;
};

#endif
