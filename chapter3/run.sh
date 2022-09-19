#!/bin/bash
# author: tly
# date: Sat Jun 15 11:38:01 DST 2019
folder="./build"
if [ ! -d "$folder" ]; then
	echo  "build dir is not exit and need to create a new one"
	mkdir build
fi
cd build/
rm -rf *
cmake ..
make > log.txt 2>&1
cat log.txt | grep "error:" > /dev/null
if [ $? -eq 0 ]; then
	vim log.txt
else
	echo "no compile error"
fi
file="./bin/chapter3"

if [ ! -x "$file" ]; then
	echo "$file is not exit"
	exit 
fi

echo "----------running result---------- "
$file
echo "----------running result---------- "



