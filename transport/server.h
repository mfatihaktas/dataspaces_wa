#ifndef _SERVER_H_
#define _SERVER_H_

#include <stdio>
#include <vector>
#include <boost/asio.hpp>

using namespace std;
using namespace boost;

/**
 * Base server class, every transport layer class must extend this
 */
class Listener {
public:
    Listener();
    virtual ~Listener();
	virtual void msg_arrived(Message msg);
};

class Server{
private:
	vector<Listener> listeners;

public:
	Server();
	virtual ~Server();
	virtual void add_listener(Listener listener);
};

#endif /* _SERVER_H_ */
