#include "tcp_server.h"

WorkerThread::WorkerThread(TCPServer& s, int id) : server(s){
	this->id = id;
}

WorkerThread::~WorkerThread(){
}

/**
 * Method responsible for handling a single tcp request
 */
void WorkerThread::work(shared_ptr<asio::io_service> io_service){
	server.get_glock()->lock();
	cout << "Worker " << id << " starting" << endl;
	server.get_glock()->unlock();

	io_service->run();

	server.get_glock()->lock();
	cout << "Worker " << id << " ending" << endl;
	server.get_glock()->unlock();
}

TCPServer::TCPServer(int port, int num_workers) : Server(){
	glock = new mutex();

	listeners = vector<Listener>();
	// TODO: start configurable amount of acceptor threads
	// acceptor.asyn_accept()
	shared_ptr<asio::io_service> io_service(new asio::io_service);
	shared_ptr<asio::io_service::work> work(new asio::io_service::work(*io_service));
	thread_group worker_threads;
	for (int i = 0; i <= num_workers; ++i){
		shared_ptr<WorkerThread> worker(new WorkerThread(*this, i));
		worker_threads.create_thread(bind(&WorkerThread::work, worker, io_service));
	}

	cin.get();
	io_service->stop();
	worker_threads.join_all();

}

TCPServer::~TCPServer(){
	delete glock;
}

void TCPServer::add_listener(Listener& listener){
	listeners.push_back(listener);
}


int main(int argc, char* argv[]) {
	if(argc != 3) {
		cout << "Usage: server port num_threads" << endl;
	} else {
		int port = boost::lexical_cast<int>(argv[1]);
		int num_threads = boost::lexical_cast<int>(argv[2]);

		cout << num_threads << endl;

		TCPServer server(port, num_threads);
	}
	return 0;
}
