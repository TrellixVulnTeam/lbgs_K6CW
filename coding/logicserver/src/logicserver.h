#ifndef __LOGIC_SERVER_H
#define __LOGIC_SERVER_H

#include "logicserver_aid.h"

#include <boostmodule/basedef.h>
#include <msgmodule/msgcommondef.h>

class LogicServer
{
	friend class TimerLogicProxySrvHeart;
public:
	LogicServer();
	~LogicServer();

	void start();
	void sendToProxySrv(const byte* data, uint size);

HANDLER:
	void onConnectInnerServer(const CommonBoost::ErrorCode& err);
	void onRunInnnerIOServerOnce();
	void onProxySrvSend(
		const CommonBoost::ErrorCode& ec,
		uint readSize
	);
	void onProxySrvRead(
		const CommonBoost::ErrorCode& ec,
		uint readSize
	);

private:
	// 连接到代理服操作
	void initInnerClient();
	void connectInnerServer();
	void closeInnerSocket();
	void readFromProxySrv();

private:
	CommonBoost::IOServer m_innerServer;
	CommonBoost::SocketPtr m_pInnerSocket;
	CommonBoost::Endpoint m_innerEndpoint;
	bool m_bConnectProxySrv;
	bool m_bInnerRunOnce;
	byte m_bytesInnerSrvBuffer[MsgBuffer::g_nReadBufferSize];
	byte m_bytesInnerSrvOnceMsg[MsgBuffer::g_nOnceMsgSize];
	ushort m_nHasReadProxyDataSize;
	// 和代理服的心跳
	TimerLogicProxySrvHeart m_innerSrvHeart;
};

#endif  //__LOGIC_SERVER_H
