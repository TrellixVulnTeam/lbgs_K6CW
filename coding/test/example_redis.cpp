#include <gtest/gtest.h>
#include <iostream>
#include <boost/random.hpp>
extern "C"
{
#include <hiredis/hiredis.h>
}

namespace
{
// link redis and set/get value
TEST(Redis_Example, test)
{
	/*timeval tv = { 1,500000 };

	redisContext* conn = redisConnectWithTimeout("192.168.43.122", 6379, tv);
	if (conn != NULL && conn->err)
	{
		printf("connection error: %s\n", conn->errstr);
		return;
	}

	redisReply* replya = NULL;
	replya = (redisReply*)redisCommand(conn, "AUTH default %s", "123456");
	if (replya->type != REDIS_REPLY_ERROR)
	{
		printf("Redis password ok\n");
	} else
	{
		printf("Redis password error\n");
	}
	freeReplyObject(replya);

	redisReply* reply = NULL;
	reply = (redisReply*)redisCommand(conn, "SET %s %s", "foo", "bar");
	freeReplyObject(reply);

	reply = (redisReply*)redisCommand(conn, "GET %s", "foo");
	printf("%s\n", reply->str);
	freeReplyObject(reply);

	redisFree(conn);*/
}

// redis verification code study -- test
TEST(Redis_Example_2, test)
{
	/*boost::random::mt19937_64 gen(time(NULL));
	boost::random::uniform_int_distribution<> dist(100000,999999);
	int r = dist(gen);

	timeval tv = { 1,500000 };

	redisContext* conn = redisConnectWithTimeout("192.168.43.122", 6379, tv);
	if (conn != NULL && conn->err)
	{
		printf("connection error: %s\n", conn->errstr);
		return;
	}

	redisReply* reply = NULL;
	reply = (redisReply*)redisCommand(conn, "AUTH default %s", "123456");
	if (reply->type != REDIS_REPLY_ERROR)
	{
		printf("Redis password ok\n");
	} else
	{
		printf("Redis password error\n");
	}
	freeReplyObject(reply);

	reply = (redisReply*)redisCommand(conn, "setnx check_code %d", r);
	freeReplyObject(reply);

	reply = (redisReply*)redisCommand(conn, "setnx input_count 0");
	freeReplyObject(reply);

	reply = (redisReply*)redisCommand(conn, "expire check_code 120");
	freeReplyObject(reply);

	int intpu_count = 0;
	reply = (redisReply*)redisCommand(conn, "get input_count");
	intpu_count = CAST_TO(int, reply->str);
	freeReplyObject(reply);
	if (intpu_count == 3)
	{
		printf("input caount == 3\n");
		return;
	}

	int retCode = 0;
	reply = (redisReply*)redisCommand(conn, "get check_code");
	retCode = CAST_TO(int, reply->str);
	freeReplyObject(reply);

	printf("please input check code: [%d]\n", retCode);

	int inputCneck = 0;
	scanf("%d",&inputCneck);
	if (inputCneck == retCode)
	{
		printf("check code ok\n");
	}
	else
	{
		printf("check code error\n");
	}

	reply = (redisReply*)redisCommand(conn, "incr input_count");
	freeReplyObject(reply);

	redisFree(conn);*/

}


/*
* redis cluster sample test
*
* redis have 6 server, build a cluster
* There should be 6 servers in the production environment, this is only for testing
*
* 192.468.43.122:7000
* 192.468.43.122:7001
* 192.468.43.122:7002
* 192.468.43.122:7003
* 192.468.43.122:7004
* 192.468.43.122:7005
*
*/
TEST(Redis_Example_3, test)
{
	//timeval tv = { 1,500000 };

	//redisContext* conn = redisConnectWithTimeout("192.168.43.122", 7000, tv);
	//if (conn != NULL && conn->err)
	//{
	//	printf("connection error: %s\n", conn->errstr);
	//	return;
	//}

	//redisReply* replya = NULL;
	//replya = (redisReply*)redisCommand(conn, "AUTH default %s", "123456");
	//if (replya->type != REDIS_REPLY_ERROR)
	//{
	//	printf("Redis password ok\n");
	//} else
	//{
	//	printf("Redis password error\n");
	//}
	//freeReplyObject(replya);

	//redisReply* reply = NULL;

	//// redis set a key k1 first
	//const size_t binDataLen = 20;
	//char binData[binDataLen];
	//memset(binData, 0, binDataLen);
	//binData[10] = 'c';
	//reply = (redisReply*)redisCommand(conn, "SET %b %b", "kb",(size_t)2, binData, binDataLen);
	//std::string strRep = reply->str;
	//int port = 0;
	//if (strRep.substr(0,5) == "MOVED")
	//{
	//	int pos = strRep.find(':');
	//	port =  CAST_TO(int,strRep.substr(pos + 1));
	//	redisFree(conn);

	//	// connect
	//	conn = redisConnectWithTimeout("192.168.43.122", port, tv);
	//	if (conn != NULL && conn->err)
	//	{
	//		printf("connection error: %s\n", conn->errstr);
	//		return;
	//	}

	//	// auth
	//	replya = (redisReply*)redisCommand(conn, "AUTH default %s", "123456");
	//	if (replya->type != REDIS_REPLY_ERROR)
	//	{
	//		printf("Redis password ok\n");
	//	} else
	//	{
	//		printf("Redis password error\n");
	//	}
	//	freeReplyObject(replya);

	//	reply = (redisReply*)redisCommand(conn, "SET %b %b", "kb", (size_t)2, binData, binDataLen);
	//}
	//printf("%s\n", reply->str);			// should ouput k1 value [success]
	//freeReplyObject(reply);

	//redisFree(conn);
}
}