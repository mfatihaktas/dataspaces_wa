#include "tcp_server.h"

TCPServer::TCPServer(){
	listeners = vector<Listener>();
}

void TCPServer::add_listener(Listener listener){
	listeners.push_back(listener);
}
