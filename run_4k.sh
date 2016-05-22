#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/4k1.ldb /home/ju/test_data/4k2.ldb /home/ju/test_data/4k3.ldb /home/ju/test_data/4k4.ldb /home/ju/test_data/4k5.ldb 639575982 639575982 639575982 639575978 639575978
