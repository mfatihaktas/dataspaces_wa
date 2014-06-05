#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <iostream>
#include <boost/asio.hpp>

typedef unsigned char byte;

class Message
{
private:
	long timestamp;
	byte* data;
public:
	Message();
	virtual ~Message();
};

#endif /* _MESSAGE_H_ */
