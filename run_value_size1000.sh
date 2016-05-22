#!/bin/sh
free
sync
sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'
free
./app 5 /home/ju/test_data/gama1.ldb /home/ju/test_data/gama2.ldb /home/ju/test_data/gama3.ldb /home/ju/test_data/gama4.ldb /home/ju/test_data/gama5.ldb 1006352353 1006352353 1006352353 1006352353 1006352353
#./app 2 /home/ju/test_data/gama1.ldb /home/ju/test_data/gama2.ldb  1006352353 1006352353
