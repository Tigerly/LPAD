#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/beta1.ldb /home/ju/test_data/beta2.ldb /home/ju/test_data/beta3.ldb /home/ju/test_data/beta4.ldb /home/ju/test_data/beta5.ldb 6384495 6384495 6384495 6384494 6384494 
