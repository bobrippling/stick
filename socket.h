#ifndef SOCKET_H
#define SOCKET_H

class Socket
{
	private:
		int fd;

		enum State
		{
			CONNECTED,
			LISTENING,
			CONNECTING,
			IDLE
		} state;

		void cleanup();

	public:
		Socket();
		~Socket();

		bool connect(string& host);
		bool disconnect();

		bool senddata(char *);
		bool recvdata(char *, int);

		enum State getstate() const;
		void setblocking(bool);
};

#endif
