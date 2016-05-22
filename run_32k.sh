#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/32k1.ldb /home/ju/test_data/32k2.ldb /home/ju/test_data/32k3.ldb /home/ju/test_data/32k4.ldb /home/ju/test_data/32k5.ldb 632977105 632977105 632977105 632977022 632977022
