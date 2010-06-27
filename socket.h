#ifndef SOCKET_H
#define SOCKET_H

class Socket
{
	private:
		void cleanup();
		bool setblocking(bool) const;
		bool senddata(void *, size_t);
		bool recvdata(void *, size_t);

		// events
		bool accept();
		bool checkconnected();
		void newsocket(int);

		int fd;
		struct sockaddr_in addr;

		const char *lerr;

		// func ptrs
		Socket& (*connrequestf)();
		void (*disconnectedf)();
		void (*receivedf)();
		void (*errorf)();


	public:
		enum State
		{
			CONNECTED,
			LISTENING,
			CONNECTING,
			IDLE
		};

		virtual ~Socket();
		Socket(
				Socket& (connreq)(),
				void (*disconnected)(),
				void (*received)(),
				void (*error)()
				);
		Socket(const Socket&);
		virtual Socket& operator=(const Socket&);

		bool runevents();

		bool connect(const char *host, int);
		bool connect(const std::string& host, int);

		bool listen(int port);
		bool listen(int port, const char *);

		void disconnect();

		bool senddata(const void *, size_t);
		bool senddata(const char *);
		bool senddata(const char);
		bool senddata(const std::string&);
		Socket& operator<<(const char *);
		Socket& operator<<(const char);
		Socket& operator<<(const std::string&);

		enum State getstate() const;
		const char *remoteaddr();
		const char *lasterr() const;

		// function pointer getters
		void (*get_connrequestfunc());
		void (*get_disconnectedfunc());
		void (*get_receivedfunc());
		void (*get_errorfunc());

		// function pointer setters
		void set_connrequestfunc( void (*f));
		void set_disconnectedfunc(void (*f));
		void set_receivedfunc(    void (*f));
		void set_errorfunc(       void (*f));

	private:
		enum State state;
};

#endif
