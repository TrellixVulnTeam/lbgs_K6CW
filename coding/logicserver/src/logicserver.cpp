#include "logicserver.h"
#include "msghandler/logicmsghandler.h"
#include "globallogicserver.h"
#include "rolemanager/rolemanager.h"

#include <configmodule/configmanager.h>
#include <configmodule/proxyserverconfig/proxyserverconfig.h>
#include <logmodule/logdef.h>
#include <msgmodule/msgcommondef.h>
#include <msgmodule/singletoproxymsghandler.h>

#define DO_LOGICSERVER_MSG_CHECK_HEADER \
if (m_msgHeader.m_nReceiver != MsgHeader::F_LOGICSERVER ||\
m_msgHeader.m_nProxyer != MsgHeader::F_PROXYSERVER)\
{\
	LOG_PROXYSERVER.printLog("m_msgHeader error");\
	m_bHeaderIntegrated = true;\
	m_nLastHasReadSize = 0;\
	m_nNextNeedReadSize = 0;\
	break;\
}

#define DO_LOGICSERVER_MSG_FROM_PROXY \
if (m_msgHeader.m_nMsgType >= MSG_TYPE_GATE_PROXY_HEART_GP &&\
m_msgHeader.m_nMsgType < MSG_IN_TYPE_MAX&&\
	m_msgHeader.m_nMsgType == MSG_TYPE_LOGIC_PROXY_HEART_PL\
	)\
{\
	SingleToProxyMsgHandler::callHandler(\
		m_msgHeader.m_nMsgType,\
		(const byte*)this,\
		m_bytesInnerSrvOnceMsg + sizeof(MsgHeader),\
		m_msgHeader.m_nMsgLen - sizeof(MsgHeader));\
} else if (m_msgHeader.m_nMsgType >= MSG_TYPE_CLIENT_START &&\
	m_msgHeader.m_nMsgType < MSG_CODE_MAX)\
{\
	LogicMsgHandler::callHandler(\
		m_msgHeader.m_nMsgType,\
		this,\
		ROLE_MGR->findRoleByClientSeq(m_msgHeader.m_nClientSrcSeq),\
		m_bytesInnerSrvOnceMsg,\
		m_msgHeader.m_nMsgLen\
	);\
}

LogicServer::LogicServer()
	: m_bConnectProxySrv(false)
	, m_nHasReadProxyDataSize(0)
	, m_nNextNeedReadSize(0)
	, m_nLastHasReadSize(0)
	, m_bHeaderIntegrated(true)
{
	GLOBAL_LOGIC->registerGlobal(this);

	const ProxyServerConfigInfo info = *(CONFIG_MGR->GetProxyServerConfig()->getConfigInfo());
	m_innerSrvHeart.setLogicServer(this);
	m_innerSrvHeart.setInterval(info.heart_time);
	memset(m_bytesInnerSrvBuffer, 0, MsgBuffer::g_nReadBufferSize);

	initInnerClient();
}

LogicServer::~LogicServer()
{
	closeInnerSocket();
}

void LogicServer::start()
{
	for (int i = 0; i < CPU_MAX_THREAD; ++i)
	{
		boost::thread tConnect(BIND(&LogicServer::onRunInnnerIOServerOnce, this));
		tConnect.detach();
	}
	
	while (1) { THREAD_SLEEP(1); }
}

void LogicServer::sendToProxySrv(const byte* data, uint size)
{
	if (!m_pInnerSocket || !m_pInnerStrand)
	{
		LOG_LOGICSERVER.printLog("m_pInnerSocket == NULL || m_pInnerStrand NULL");
		return;
	}

	m_pInnerSocket->async_write_some(
		MSG_BUFFER(data, size),
		m_pInnerStrand->wrap(BIND(&LogicServer::onProxySrvSend, this, boost::placeholders::_1, boost::placeholders::_2))
	);
}

void LogicServer::sendToClient(const byte* data, uint size)
{
	MsgHeader* h = (MsgHeader*)data;
	if (!h)
	{
		LOG_LOGICSERVER.printLog("msg header NULL");
		return;
	}
	h->m_nSender = MsgHeader::F_LOGICSERVER;
	h->m_nReceiver = MsgHeader::F_GATESERVER;
	h->m_nProxyer = MsgHeader::F_PROXYSERVER;

	sendToProxySrv(data, size);
}

void LogicServer::sendToCache(const byte* data, uint size)
{
	MsgHeader* h = (MsgHeader*)data;
	if (!h)
	{
		LOG_LOGICSERVER.printLog("msg header NULL");
		return;
	}
	h->m_nSender = MsgHeader::F_LOGICSERVER;
	h->m_nReceiver = MsgHeader::F_CACHESERVER;
	h->m_nProxyer = MsgHeader::F_PROXYSERVER;

	sendToProxySrv(data, size);
}

void LogicServer::sendToDBServer(const byte* data, uint size)
{
	MsgHeader* h = (MsgHeader*)data;
	if (!h)
	{
		LOG_LOGICSERVER.printLog("msg header NULL");
		return;
	}
	h->m_nSender = MsgHeader::F_LOGICSERVER;
	h->m_nReceiver = MsgHeader::F_DBSERVER;
	h->m_nProxyer = MsgHeader::F_PROXYSERVER;

	sendToProxySrv(data, size);
}

void LogicServer::onRunInnnerIOServerOnce()
{
	CommonBoost::WorkPtr work(new CommonBoost::IOServer::work(m_innerServer));
	while (1)
	{
		THREAD_SLEEP(1);
		try
		{
			m_innerServer.run();
			break;
		} catch (std::exception& e)
		{
			LOG_LOGICSERVER.printLog("m_innerServer run exception!! info[%s] server will re-start!!", e.what());
			printf_color(PRINTF_RED, "%s : m_innerServer run exception!! info[%s] server will re-start!!\n", __FUNCTION__, e.what());
		}
	}
}

void LogicServer::onProxySrvSend(const CommonBoost::ErrorCode& ec, uint readSize)
{
	if (ec)
	{
		LOG_LOGICSERVER.printLog("error value[%d],send size[%d], message[%s]",
			ec.value(),
			readSize,
			ec.message().data());

		if (m_pInnerSocket)
		{
			LOG_LOGICSERVER.printLog("will shutdown send channel");
			m_pInnerSocket->shutdown(boost::asio::socket_base::shutdown_send, const_cast<CommonBoost::ErrorCode&>(ec));
		}
	}
}

void LogicServer::onProxySrvRead(const CommonBoost::ErrorCode& ec, uint readSize)
{
	if (ec)
	{
		LOG_LOGICSERVER.printLog("ecode[%d],messages[%s]",
			ec.value(),
			ec.message().data());
		if (m_pInnerSocket)
		{
			LOG_LOGICSERVER.printLog("will shutdown read channel");
			m_pInnerSocket->shutdown(boost::asio::socket_base::shutdown_receive, const_cast<CommonBoost::ErrorCode&>(ec));
		}

		LOG_LOGICSERVER.printLog("****** LogicServer will re-connect to proxyserver ******");
		printf_color(PRINTF_RED, "****** LogicServer will re-connect to proxyserver ******\n");

		connectInnerServer();
		return;
	}
	if (readSize <= 0 || readSize > MsgBuffer::g_nReadBufferSize)
	{
		LOG_LOGICSERVER.printLog("size error,readSize[%d],g_nReadBufferSize[%d]", readSize, MsgBuffer::g_nReadBufferSize);
		return;
	}

	MsgHeader m_msgHeader;
	m_nHasReadProxyDataSize = 0;

	int remainSize = 0;
	int remainBodySize = 0;

	while (m_nHasReadProxyDataSize < readSize)
	{
		THREAD_SLEEP(1);

		if (m_nLastHasReadSize > 0 && m_nNextNeedReadSize > 0)
		{
			memmove(m_bytesInnerSrvOnceMsg + m_nLastHasReadSize, m_bytesInnerSrvBuffer, m_nNextNeedReadSize);
			m_msgHeader = *(MsgHeader*)(m_bytesInnerSrvOnceMsg + m_nHasReadProxyDataSize);
			DO_LOGICSERVER_MSG_CHECK_HEADER;

			if (m_bHeaderIntegrated)
			{
				DO_LOGICSERVER_MSG_FROM_PROXY;

				m_nHasReadProxyDataSize += m_nNextNeedReadSize;
			} else
			{
				remainBodySize = readSize - m_nNextNeedReadSize;
				if (remainBodySize >= m_msgHeader.m_nMsgLen - sizeof(MsgHeader))
				{
					memmove(m_bytesInnerSrvOnceMsg + sizeof(MsgHeader), m_bytesInnerSrvBuffer + m_nNextNeedReadSize, m_msgHeader.m_nMsgLen - sizeof(MsgHeader));

					DO_LOGICSERVER_MSG_FROM_PROXY;

					m_nHasReadProxyDataSize += (m_nNextNeedReadSize + m_msgHeader.m_nMsgLen - sizeof(MsgHeader));

				}
			}

			m_bHeaderIntegrated = true;
			m_nLastHasReadSize = 0;
			m_nNextNeedReadSize = 0;
			continue;
		}
		remainSize = readSize - m_nHasReadProxyDataSize;
		if (remainSize >= sizeof(MsgHeader))
		{
			m_msgHeader = *(MsgHeader*)(m_bytesInnerSrvBuffer + m_nHasReadProxyDataSize);

			DO_LOGICSERVER_MSG_CHECK_HEADER;

			remainBodySize = remainSize - sizeof(MsgHeader);
			if (remainBodySize >= m_msgHeader.m_nMsgLen - sizeof(MsgHeader))
			{
				memmove(m_bytesInnerSrvOnceMsg, m_bytesInnerSrvBuffer + m_nHasReadProxyDataSize, m_msgHeader.m_nMsgLen);

				DO_LOGICSERVER_MSG_FROM_PROXY;

				m_nHasReadProxyDataSize += m_msgHeader.m_nMsgLen;
				continue;
			}
			m_nLastHasReadSize = remainSize;
			m_nNextNeedReadSize = (m_msgHeader.m_nMsgLen - remainBodySize - sizeof(MsgHeader));
			memmove(m_bytesInnerSrvOnceMsg, m_bytesInnerSrvBuffer + m_nHasReadProxyDataSize, remainSize);
			m_nHasReadProxyDataSize += remainSize;
			continue;
		}
		m_bHeaderIntegrated = false;
		m_nLastHasReadSize = remainSize;
		m_nNextNeedReadSize = (sizeof(MsgHeader) - m_nLastHasReadSize);
		memmove(m_bytesInnerSrvOnceMsg, m_bytesInnerSrvBuffer + m_nHasReadProxyDataSize, remainSize);
		m_nHasReadProxyDataSize += remainSize;
	}


	readFromProxySrv();
}

void LogicServer::initInnerClient()
{
	if (!CONFIG_MGR->GetProxyServerConfig())
	{
		return;
	}
	const ProxyServerConfigInfo info = *(CONFIG_MGR->GetProxyServerConfig()->getConfigInfo());

	m_pInnerSocket = boost::make_shared<CommonBoost::Socket>(m_innerServer);
	m_pInnerStrand = boost::make_shared<CommonBoost::Strand>(m_innerServer);
	m_innerEndpoint = CommonBoost::Endpoint(
		boost::asio::ip::address::from_string(info.ip), info.port
	);
	connectInnerServer();
}

void LogicServer::connectInnerServer()
{
	if (!m_pInnerSocket)
	{
		LOG_LOGICSERVER.printLog("m_pInnerSocket is NULL");
		return;
	}

	if (m_bConnectProxySrv)
	{
		closeInnerSocket();
	}
	m_bConnectProxySrv = false;
	m_pInnerSocket->async_connect(m_innerEndpoint, BIND(&LogicServer::onConnectInnerServer, this, boost::placeholders::_1));

}

void LogicServer::closeInnerSocket()
{
	if (!m_pInnerSocket)
	{
		LOG_LOGICSERVER.printLog("m_pInnerSocket is NULL");
		return;
	}
	m_pInnerSocket->close();
}

void LogicServer::readFromProxySrv()
{
	if (!m_pInnerSocket || !m_pInnerStrand)
	{
		LOG_LOGICSERVER.printLog("m_pInnerSocket == NULL || m_pInnerStrand NULL");
		return;
	}

	memset(m_bytesInnerSrvBuffer, 0, sizeof(m_bytesInnerSrvBuffer));
	m_pInnerSocket->async_read_some(
		MSG_BUFFER(m_bytesInnerSrvBuffer, sizeof(m_bytesInnerSrvBuffer)),
		m_pInnerStrand->wrap(BIND(&LogicServer::onProxySrvRead, this, boost::placeholders::_1, boost::placeholders::_2))
	);
}

void LogicServer::onConnectInnerServer(const CommonBoost::ErrorCode& err)
{
	if (err)
	{
		LOG_LOGICSERVER.printLog("Please run ProxyServer first.......");
		printf_color(PRINTF_RED, "\nPlease run ProxyServer first.......\n");
		return;
	}

	if (m_pInnerSocket.get() == NULL)
	{
		LOG_LOGICSERVER.printLog("m_pInnerSocket is NULL");
		return;
	}

	LOG_LOGICSERVER.printLog("link proxy server succ");
	printf_color(PRINTF_GREEN, "\nlink proxy server succ\n");
	m_bConnectProxySrv = true;

	m_innerSrvHeart.start();

	// send a byte info,tell proxy server my identity
	DEFINE_BYTE_ARRAY(firstData, 1);
	firstData[0] = MsgHeader::F_LOGICSERVER;
	sendToProxySrv(firstData, 1);

	readFromProxySrv();
}

