#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/8k1.ldb /home/ju/test_data/8k2.ldb /home/ju/test_data/8k3.ldb /home/ju/test_data/8k4.ldb /home/ju/test_data/8k5.ldb 635147109 635147109 635147109 635171335 635171335
