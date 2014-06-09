#ifndef _CLIENT_H_
#define _CLIENT_H_

/**
 * Basic client class, every transport client must extend this.
 */
class Client
{
private:
public:
    Client();
    virtual ~Client();
};

#endif /* _CLIENT_H_ */
