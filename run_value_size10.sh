#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/alpha1.ldb /home/ju/test_data/alpha2.ldb /home/ju/test_data/alpha3.ldb /home/ju/test_data/alpha4.ldb /home/ju/test_data/alpha5.ldb 15374928 15374928 15374928 15374928 15374928
