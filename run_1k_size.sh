#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/p1.ldb /home/ju/test_data/p2.ldb /home/ju/test_data/p3.ldb /home/ju/test_data/p4.ldb /home/ju/test_data/p5.ldb 421499174 421499174 421499174 421499160 421499160 
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/q1.ldb /home/ju/test_data/q2.ldb /home/ju/test_data/q3.ldb /home/ju/test_data/q4.ldb /home/ju/test_data/q5.ldb 632249864 632249864 632249864 632249810 632249810  
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/r1.ldb /home/ju/test_data/r2.ldb /home/ju/test_data/r3.ldb /home/ju/test_data/r4.ldb /home/ju/test_data/r5.ldb 843000741 843000741 843000741 843000720 843000720    
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/s1.ldb /home/ju/test_data/s2.ldb /home/ju/test_data/s3.ldb /home/ju/test_data/s4.ldb /home/ju/test_data/s5.ldb 1053751401 1053751401 1053751401 1053751347 1053751347
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/t1.ldb /home/ju/test_data/t2.ldb /home/ju/test_data/t3.ldb /home/ju/test_data/t4.ldb /home/ju/test_data/t5.ldb 1264502325 1264502325 1264502325 1264502288 1264502288       
