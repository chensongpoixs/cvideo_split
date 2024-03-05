#!/bin/bash


for num in {1..100000}  
do  
	clear
	echo $num  
	nvidia-smi  --list-gpus
	
	nvidia-smi -i 0 -q -d UTILIZATION
#	nvidia-smi -i 1 -q -d UTILIZATION
	sleep 1
done  


