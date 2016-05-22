#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/128b1.ldb /home/ju/test_data/128b2.ldb /home/ju/test_data/128b3.ldb /home/ju/test_data/128b4.ldb /home/ju/test_data/128b5.ldb 797232222 797232222 797232222 797421504 797421504
