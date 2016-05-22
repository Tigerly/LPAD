#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/512b1.ldb /home/ju/test_data/512b2.ldb /home/ju/test_data/512b3.ldb /home/ju/test_data/512b4.ldb /home/ju/test_data/512b5.ldb 693862824 693862824 693862824 693862823 693862823
