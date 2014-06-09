#ifndef DHTCLIENT_H
#define DHTCLIENT_H
//

using boost::asio::ip::tcp;

class DHTClient{
  public:
    DHTClient(char* host, int port);
    ~DHTClient();
  private:
    char* host;
    int port;
    //
    boost::shared_ptr< boost::asio::io_service > io_service_;
    boost::shared_ptr< boost::asio::io_service::work > work_;
  	boost::shared_ptr< boost::asio::io_service::strand > strand_;
};
//
#endif
