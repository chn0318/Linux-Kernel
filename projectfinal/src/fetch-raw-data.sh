#!/bin/bash
echo 'accessed_page,utime,stime,cutime,cstime,total_time' > data.csv
nice -n 0 taskset -c 1 sysbench --test=cpu --cpu-max-prime=200000 --num-threads=1 run & #可以选择不同的sysbench进程
cat_ret()
{
    ps -e |grep sysbench| awk '{print $1}'
}
ret=`cat_ret`
echo ${ret} > /proc/watch
while sleep 0.1
do
   cat /proc/watch | awk '{ printf "%s,%s,%s,%s,%s,",$1,$2,$3,$4,$5 } ' >>data.csv 
   cat /proc/stat  | awk '$1=="cpu1" {printf "%s\n", $2+$3+$4+$5+$6+$7+$8} ' >>data.csv
done
