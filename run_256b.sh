#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/256b1.ldb /home/ju/test_data/256b2.ldb /home/ju/test_data/256b3.ldb /home/ju/test_data/256b4.ldb /home/ju/test_data/256b5.ldb 739834979 739834979 739834979 739834978 739834978
