#ifndef __ISOCKOBJ_H__
#define __ISOCKOBJ_H__

#include <Packet.h>

class ISockObj
{
public:
    enum ESockType
    {
        ESOCKTYPE_MIN,
        ESOCKTYPE_UNDECIDED = ESOCKTYPE_MIN,
        ESOCKTYPE_TCP,
        ESOCKTYPE_UDP,
        ESOCKTYPE_MAX,
    };
    virtual int     Create(ESockType type)                                                                                              = 0;
    virtual void    Close()                                                                                                             = 0;
    virtual int     GetId()                                                                                                             = 0;
    virtual int     Bind(SOCKADDR* addr, int namelen)                                                                                   = 0;
    virtual int     Listen(int backlog)                                                                                                 = 0;
    virtual int     PostAccept(ISockObj* accept, char* buf, int len)                                                                    = 0;
    virtual int     PostConnect(SOCKADDR* remote_addr, int remote_namelen, SOCKADDR* local_addr, int local_namelen, char* buf, int len) = 0;
    virtual int     PostSend(Packet* packet, SOCKADDR* addr = NULL, int namelen = 0)                                                    = 0;
    virtual int     PostRecv()                                                                                                          = 0;
    virtual int     Update()                                                                                                            = 0;

protected:
	virtual SOCKET GetSocket() = 0;
};

#endif // __ISOCKOBJ_H__