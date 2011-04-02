#ifndef SOCKADDR_H
#define SOCKADDR_H

class SockAddr
{
	private:
		struct sockaddr_in _addr;
		bool _valid;

	public:
		SockAddr(const struct sockaddr_in *a);
		SockAddr(const char *ip, const char *port);
		SockAddr(const SockAddr &);

		bool valid() const;
		bool equals(const SockAddr *) const;
		const struct sockaddr_in *getaddr() const;
		const struct sockaddr    *getgenericaddr() const;
		const char *c_str();
		int         port( ) const;

		static const char *str(const struct sockaddr    *, socklen_t);
		static const char *str(const struct sockaddr_in  *);
		static const char *str(const struct sockaddr_in6 *);
};

class Addressable
{
	protected:
		SockAddr *_addr;

	public:
		Addressable(const struct sockaddr_in &);
		Addressable(const SockAddr &);

		Addressable();
		~Addressable();

		bool valid() const;
		bool addr_equals(const SockAddr *) const;
		const SockAddr *getsockaddr() const;
};

#endif
