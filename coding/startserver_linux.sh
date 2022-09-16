#!/bin/bash
echo "server loading"
echo "start proxyserver......"
nohup ../build/exe/proxyserver > ../build/exe/err_proxyserver.log 2>&1 &
echo "proxyserver started!!!"
sleep 1s

echo "start gateserver......"
nohup ../build/exe/gateserver > ../build/exe/err_gateserver.log 2>&1 &
echo "gateserver started!!!"
sleep 1s

echo "start logicserver......"
nohup ../build/exe/logicserver > ../build/exe/err_logicserver.log 2>&1 &
echo "logicserver started!!!"
sleep 1s

echo "start cacheserver......"
nohup ../build/exe/cacheserver > ../build/exe/err_cacheserver.log 2>&1 &
echo "cacheserver started!!!"


echo "all server start succ!!!!!"

