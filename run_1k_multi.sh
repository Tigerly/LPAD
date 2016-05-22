#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 3 /home/ju/test_data/a1.ldb /home/ju/test_data/a2.ldb /home/ju/test_data/a3.ldb 1106439104 1106439104 1106439104
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/b1.ldb /home/ju/test_data/b2.ldb /home/ju/test_data/b3.ldb /home/ju/test_data/b4.ldb /home/ju/test_data/b5.ldb  663862480 663862480 663862480 663862426 663862426 
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 7 /home/ju/test_data/c1.ldb /home/ju/test_data/c2.ldb /home/ju/test_data/c3.ldb /home/ju/test_data/c4.ldb /home/ju/test_data/c5.ldb /home/ju/test_data/c6.ldb /home/ju/test_data/c7.ldb  474186727 474186727 474186727 474186730 474186730 474186730 474186730 
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 9 /home/ju/test_data/d1.ldb /home/ju/test_data/d2.ldb /home/ju/test_data/d3.ldb /home/ju/test_data/d4.ldb /home/ju/test_data/d5.ldb /home/ju/test_data/d6.ldb /home/ju/test_data/d7.ldb /home/ju/test_data/d8.ldb /home/ju/test_data/d9.ldb 368811476 368811476 368811476 368811454 368811454 368811454 368811454 368811413 368811413  
