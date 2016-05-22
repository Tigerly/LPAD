#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/6k1.ldb /home/ju/test_data/6k2.ldb /home/ju/test_data/6k3.ldb /home/ju/test_data/6k4.ldb /home/ju/test_data/6k5.ldb 636532377 636532377 636532377 636532377 636532377
