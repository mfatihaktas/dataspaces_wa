#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <stdio>
#include <boost/asio.hpp>

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
