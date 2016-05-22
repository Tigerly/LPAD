#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/64k1.ldb /home/ju/test_data/64k2.ldb /home/ju/test_data/64k3.ldb /home/ju/test_data/64k4.ldb /home/ju/test_data/64k5.ldb 632403337 632403337 632403337 632403334 632403334 
