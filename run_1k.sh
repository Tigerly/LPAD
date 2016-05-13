#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/1k1.ldb /home/ju/test_data/1k2.ldb /home/ju/test_data/1k3.ldb /home/ju/test_data/1k4.ldb /home/ju/test_data/1k5.ldb 659378554 659378554 659378554 659416365 659416365 
