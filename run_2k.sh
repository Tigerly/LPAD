#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/2k1.ldb /home/ju/test_data/2k2.ldb /home/ju/test_data/2k3.ldb /home/ju/test_data/2k4.ldb /home/ju/test_data/2k5.ldb 647766714 647766714 647766714 647842320 647842320
