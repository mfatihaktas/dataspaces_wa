#ifndef _SERVER_H_
#define _SERVER_H_

#include <vector>
#include <iostream>
#include <boost/asio.hpp>

#include "message.h"

using namespace std;
using namespace boost;

/**
 * Base server class, every transport layer class must extend this
 */
class Listener {
public:
	Listener(){}
	virtual ~Listener(){}
	virtual void msg_arrived(Message msg){}
};

class Server{
protected:
	vector<Listener> listeners;

private:

public:
	Server(){}
	virtual ~Server(){}
	virtual void add_listener(Listener& listener){}
};

#endif /* _SERVER_H_ */
