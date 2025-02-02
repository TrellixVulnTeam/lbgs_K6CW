#ifndef __GATESERVER_H__
#define __GATESERVER_H__

#include "user.h"
#include "gateserver_aid.h"
#include "userseqmanager.h"

#include <boostmodule/basedef.h>
#include <boostmodule/ioserverpool.h>
#include <boost/atomic.hpp>
#include "queuemodule/safequeue.hpp"

/*
* gateserver
*
* accecpt client
* 
*/

struct GateServerConfigInfo;
class GateServer
{
	friend class User;
	friend class TimerGateProxySrvHeart;
public:
	enum EnUserStatus
	{
		LOGOUT = 2,
	};

public:
	GateServer();
	GateServer(int port);
	~GateServer();

	void start();
	void sendToProxySrv(const byte* data, uint size);

SLOTS:
	void onUserError(
		const boost::weak_ptr<User>& user,
		const CommonBoost::ErrorCode& ec);
	void OnSendToProxySrvByUser(const byte* data, uint size, ullong userSeq);

HANDLER:
	void onAcceptHandler(
		const CommonBoost::ErrorCode& err,
		const boost::weak_ptr<User>& user
		);
	void onThreadRunAcceptorIOServer();

	// link to proxy server 
	void onConnectInnerServer(const CommonBoost::ErrorCode& err);
	void onProxySrvSend(
		const CommonBoost::ErrorCode& ec,
		uint readSize
	);
	void onProxySrvRead(
		const CommonBoost::ErrorCode& ec,
		uint readSize
	);

private:
	void accept();
	void initData();
	// The client disconnects and deletes the user-related data, which can be written here
	void removeUserRelated(const boost::weak_ptr<User>& user);

	// proxy server 
	void initInnerClient();
	void connectInnerServer();
	void closeInnerSocket();
	void runInnnerIOServerOnce();
	//After each client connects, a message is sent to tell the client server information. Currently, the byte order storage method is sent, and the client assembles the message.
	void sendServerInfo(const boost::weak_ptr<User>& user);
	bool isConnectProxySrvSucc() { return m_bConnectProxySrv; }
	void readFromProxySrv();

	void sendMsgToClient(const boost::weak_ptr<User>& targetUser, byte* proxyData);

private:
	CommonBoost::IOServer m_server;
	IOServerPool m_serverUserPool;
	CommonBoost::Acceptor* m_pAcceptor;
	UserSeqManager m_userSeqMgr;
	boost::atomic<int> m_nConnectCount;
	int m_nPort;

	typedef std::map< ullong, boost::shared_ptr<User> >::const_iterator MapSeqToUserIter;
	std::map< ullong, boost::shared_ptr<User> > m_mapSeqToUser;

	// Communication with the proxy server
	bool m_bConnectProxySrv;
	CommonBoost::IOServer m_innerServer;
	CommonBoost::StrandPtr m_pInnerStrand;
	CommonBoost::SocketPtr m_pInnerSocket;
	CommonBoost::Endpoint m_innerEndpoint;
	byte m_bytesInnerSrvBuffer[MsgBuffer::g_nReadBufferSize];
	byte m_bytesInnerSrvOnceMsg[MsgBuffer::g_nOnceMsgSize];
	TimerGateProxySrvHeart m_innerSrvHeart;
	int m_nHasReadProxyDataSize;
	int m_nNextNeedReadSize;
	int m_nLastHasReadSize;
	bool m_bHeaderIntegrated;

	IOServerPool m_serverTimerPool;


};

#endif
