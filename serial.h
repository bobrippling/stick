#ifndef SERIAL_H
#define SERIAL_H

class Serialisable
{
	public:
		virtual const char *serial(int pre) const = 0;
};

#endif
