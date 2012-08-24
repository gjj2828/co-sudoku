#ifndef __NETWORKSYSTEM_H__
#define __NETWORKSYSTEM_H__

#include <INetworkSystem.h>
#include <MSWSock.h>

class CNetworkSystem: public INetworkSystem
{
public:
    CNetworkSystem();
    virtual int     Init();
    virtual void    Release();
    virtual int     Update(float time);
    virtual int     Start(EMode mode, float time);
    virtual void    Stop();
    virtual void    RegisterListener(INetworkListener* listener) {}
    virtual void    UnRegisterListener(INetworkListener* listener) {}

private:
    enum EState
    {
        ESTATE_MIN,
        ESTATE_STOPPED = ESTATE_MIN,
        ESTATE_WAITING,
        ESTATE_PAIRED,
        ESTATE_MAX,
    };

    enum
    {
        MAIN_PORT       = 7778,
        BROADCAST_PORT  = 7779,
        COCLIENT_MAX    = 16,
    };

    enum EEvent
    {
        EEVENT_MIN,
        EEVENT_BROADCAST_SEND = EEVENT_MIN,
        EEVENT_BROADCAST_RECV,
        EEVENT_ACCEPT,
        EEVENT_CONNECT = EEVENT_ACCEPT,
        EEVENT_CLIENTSEND,
        EEVENT_CLIENTRECV,
        EEVENT_HOSTSEND_MIN = EEVENT_CLIENTSEND,
        EEVENT_HOSTSEND_MAX = EEVENT_HOSTSEND_MIN + COCLIENT_MAX,
        EEVENT_HOSTRECV_MIN = EEVENT_HOSTSEND_MAX,
        EEVENT_HOSTRECV_MAX = EEVENT_HOSTRECV_MIN + COCLIENT_MAX,
        EEVENT_MAX = EEVENT_HOSTRECV_MAX,
    };

    struct BroadCastData
    {
        char    sGameName[16];
        int     iSendCount;
        UINT    iState      :  2;
        UINT    iClientNum  :  5;
        UINT    iDummy      : 25;
    };

    typedef std::vector<BroadCastData> BCDVector;

    EState                      m_eState;
    int                         m_iSendCount;
    bool                        m_bbcSending;
    float                       m_fbcSendTime;

    sockaddr_in                 m_bcBindAddr;
    sockaddr_in                 m_bcSendAddr;
    sockaddr_in                 m_bcRecvAddr;
    sockaddr_in                 m_BindAddr;

    int                         m_ibcRecAddrSize;

    SOCKET                      m_soBroadCast;
    SOCKET                      m_soListener;
    SOCKET                      m_soCoClient[COCLIENT_MAX];

    LPFN_ACCEPTEX               m_lpfnAcceptEx;
    LPFN_GETACCEPTEXSOCKADDRS   m_lpfnGetAcceptExSockaddrs;

    int                         m_iCoClientMax;
    int                         m_iCoClientNum;

    HANDLE                      m_hEvents[EEVENT_MAX];
    WSAOVERLAPPED               m_Overlapped[EEVENT_MAX];

    BroadCastData               m_bcSendBuf;
    BroadCastData               m_bcRecvBuf;
    char                        m_AcceptBuf[(sizeof(SOCKADDR_IN) + 16) * 2];

    BCDVector                   m_vectorBCD;

    static const char* m_cBroadCastSendAddr;
    static const float m_cBroadCastSendInterval;

    int     BroadCastSend(float time);
    int     BroadCastRecv();
    int     PostAccept();
    void    CheckBroadCastRecv(float time);
};

#endif // __NETWORKSYSTEM_H__