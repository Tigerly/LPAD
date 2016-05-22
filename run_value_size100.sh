#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/thita1.ldb /home/ju/test_data/thita2.ldb /home/ju/test_data/thita3.ldb /home/ju/test_data/thita4.ldb /home/ju/test_data/thita5.ldb 105374350 105374350 105374350 105374311 105374311
