#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/16k1.ldb /home/ju/test_data/16k2.ldb /home/ju/test_data/16k3.ldb /home/ju/test_data/16k4.ldb /home/ju/test_data/16k5.ldb 633677754 633677754 633677754 633726208 633726208 
