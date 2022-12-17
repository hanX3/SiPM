#!/usr/bin/bash

./sort 3
./sort 11
hadd -f v524.root run0003_sort.root run0011_sort.root

./sort 7
./sort 13
hadd -f v526.root run0007_sort.root run0013_sort.root

./sort 9
./sort 14
hadd -f v528.root run0009_sort.root run0014_sort.root

./sort 10
./sort 18
hadd -f v530.root run0010_sort.root run0018_sort.root

./sort 24
./sort 21
hadd -f v532.root run0024_sort.root run0021_sort.root

./sort 25
./sort 22
hadd -f v534.root run0025_sort.root run0022_sort.root

