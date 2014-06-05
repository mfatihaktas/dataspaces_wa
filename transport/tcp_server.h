#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "server.h"

using namespace boost;

/**
 * Concrete implementation of the server class
 */
class TCPServer : public Server, public enable_shared_from_this<TCPServer> {
private:
	mutex* glock;
public:
	TCPServer(int port, int num_workers=5);
	virtual ~TCPServer();

	void add_listener(Listener& lsitener);

	//Utility functions
	mutex* get_glock(){return glock;}
};

/**
 * Class that waits and handles new connections
 */
class WorkerThread {
private:
	int id;
	TCPServer& server;
public:
	WorkerThread(TCPServer& s, int id);
	virtual ~WorkerThread();
	void work(shared_ptr<asio::io_service> io_service);
};

#endif /* _TCP_SERVER_H_ */
