#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/ku1.ldb /home/ju/ku2.ldb /home/ju/ku3.ldb /home/ju/ku4.ldb /home/ju/ku5.ldb 105414 105414 105414 105414 105414 
