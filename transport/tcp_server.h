#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include "server.h"

/**
 * Concrete implementation of the server class
 */
class TCPServer : public Server
{
public:
    TCPServer();
    virtual ~TCPServer();

	void add_listener(Listener lsitener);
};

#endif /* _TCP_SERVER_H_ */
