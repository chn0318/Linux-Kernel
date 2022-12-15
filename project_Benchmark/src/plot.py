import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import pandas as pd
from matplotlib import style
plt.rcParams['font.family'] = ['sans-serif']
plt.rcParams['font.sans-serif'] = ['SimHei']

#原始数据的验证与展示：/proc/watch 与 /proc/pid/stat 效果对比
my_dpi = 96
fig, axs = plt.subplots(2, 2, figsize=(1000 / my_dpi, 600 / my_dpi), dpi=my_dpi,
                        sharex=False, 
                        sharey=False, 
 
                        )
df=pd.read_csv("data/cpu-raw-data.csv")
df_ref = pd.read_csv("data/cpu-raw-data-ref.csv")

utime=np.array(df["utime"])
utime.tolist()
utime_ref = np.array(df_ref["utime"])
utime_ref.tolist()

cutime=np.array(df["cutime"])
cutime.tolist()
cutime_ref = np.array(df_ref["cutime"])
cutime_ref.tolist()

stime=np.array(df["stime"])
stime.tolist()
stime_ref = np.array(df_ref["stime"])
stime_ref.tolist()

cstime=np.array(df["cstime"])
cstime.tolist()
cstime_ref = np.array(df_ref["cstime"])
cstime_ref.tolist()

x=np.arange(utime.size)
x=x*2.5
total_width = 2
n=2
width=total_width/n
x = x - (total_width - width) / 2
axs[0][0].bar(x, utime, width=width, label="/proc/watch")
axs[0][0].bar(x + width, utime_ref, width=width, label="/proc/pid/stat")
axs[0][0].set_xlabel('time/s', fontsize=10) 
axs[0][0].set_ylabel('utime/clock tick', fontsize=10) 
axs[0][0].legend()

axs[0][1].bar(x, stime, width=width, label="/proc/watch")
axs[0][1].bar(x + width, stime_ref, width=width, label="/proc/pid/stat")
axs[0][1].set_xlabel('time/s', fontsize=10)  # 设置x轴刻度标签
axs[0][1].set_ylabel('stime/clock tick', fontsize=10)  # 设置y轴刻度标签
axs[0][1].legend()

axs[1][1].bar(x, cstime, width=width, label="/proc/watch")
axs[1][1].bar(x + width, cstime_ref, width=width, label="/proc/pid/stat")
axs[1][1].set_xlabel('time/s', fontsize=10)  # 设置x轴刻度标签
axs[1][1].set_ylabel('cstime/clock tick', fontsize=10)  # 设置y轴刻度标签
axs[1][1].legend()

axs[1][0].bar(x, cutime, width=width, label="/proc/watch")
axs[1][0].bar(x + width, cutime_ref, width=width, label="/proc/pid/stat")
axs[1][0].set_xlabel('time/s', fontsize=10)  # 设置x轴刻度标签
axs[1][0].set_ylabel('cutime/clock tick', fontsize=10)  # 设置y轴刻度标签
axs[1][0].legend()
plt.suptitle(" /proc/watch VS. /proc/pid/stat")
plt.tight_layout()
plt.show()

# cpu占有率曲线绘制和内存占有率曲线绘制

my_dpi = 96
fig,ax1 = plt.subplots(figsize=(1300 / my_dpi, 600 / my_dpi), dpi=my_dpi,)
ax2=ax1.twinx()
df = pd.read_csv("data/cpu-2000-2.csv")
utime = np.array(df["utime"]).tolist()
stime = np.array(df["stime"]).tolist()
cutime = np.array(df["cutime"]).tolist()
cstime = np.array(df["cstime"]).tolist()
total_time = np.array(df["total_time"]).tolist()
accessed_page = np.array(df["accessed_page"]).tolist()
cpu_measure = [] 
memory_measure = []
i=0
while utime[i+1]!=-1:
    process_time=(utime[i+1] + stime[i+1] + cutime[i+1]+ cstime[i+1]) - (utime[i]+stime[i]+cutime[i]+cstime[i])
    all_time = (total_time[i+1]-total_time[i])
    cpu_measure.append((process_time/all_time)*100)
    memory_measure.append(accessed_page[i])
    i=i+1
x = np.arange(len(cpu_measure))
ax1.plot(x,cpu_measure,'r-',label="cpu usage")
ax2.plot(x,memory_measure,'b--',label="memory usage")
ax1.set_xlabel("time/ms")
ax1.set_ylabel("cpu usage/%")
ax2.set_ylabel("memory usage/page")
ax1.set_yticks(np.arange(0,55,5))
ax2.set_yticks(np.arange(0,100,20))
plt.xlim(0,100)
ax1.legend(loc=2)
ax2.legend(loc=1)
plt.title("cpu密集型")
plt.show() 


my_dpi = 96
fig,ax1 = plt.subplots(figsize=(1300 / my_dpi, 600 / my_dpi), dpi=my_dpi,)
ax2=ax1.twinx()
df = pd.read_csv("data/memory-1M.csv")
utime = np.array(df["utime"]).tolist()
stime = np.array(df["stime"]).tolist()
cutime = np.array(df["cutime"]).tolist()
cstime = np.array(df["cstime"]).tolist()
total_time = np.array(df["total_time"]).tolist()
accessed_page = np.array(df["accessed_page"]).tolist()
cpu_measure = [] 
memory_measure = []
i=0
while utime[i+1]!=-1:
    process_time=(utime[i+1] + stime[i+1] + cutime[i+1]+ cstime[i+1]) - (utime[i]+stime[i]+cutime[i]+cstime[i])
    all_time = (total_time[i+1]-total_time[i])
    cpu_measure.append((process_time/all_time)*100)
    memory_measure.append(accessed_page[i])
    i=i+1
x = np.arange(len(cpu_measure))
ax1.plot(x,cpu_measure,'r-',label="cpu usage")
ax2.plot(x,memory_measure,'b--',label="memory usage")
ax1.set_xlabel("time/ms")
ax1.set_ylabel("cpu usage/%")
ax2.set_ylabel("memory usage/page")
ax1.set_yticks(np.arange(0,80,5))
plt.xlim(0,100)
ax1.legend(loc=2)
ax2.legend(loc=1)
plt.title("memory密集型")
plt.show() 

# 内存占有率对比
plt.figure(figsize=(40, 10), dpi=70)
df = pd.read_csv("data/1000G-4M-4.csv")
accessed_page1 = np.array(df["accessed_page"]).tolist()
df = pd.read_csv("data/1000G-1M-4.csv")
accessed_page2 = np.array(df["accessed_page"]).tolist()
length = min(len(accessed_page1),len(accessed_page2))
x =np.arange(length)
plt.plot(x,accessed_page1[0:length],label = "memory block:4M")
plt.plot(x,accessed_page2[0:length],label="memory block:1M")
plt.xlim(0,500)
plt.xlabel("time/ms",fontsize=20)
plt.ylabel("memory usage/page",fontsize=20)
plt.legend()
plt.title("内存占有率对比",fontsize=20)
plt.show()
# cpu占有率对比
plt.figure(figsize=(20, 12), dpi=70)
df = pd.read_csv("data/cpu-0-no-pre.csv")
utime1 = np.array(df["utime"]).tolist()
stime1 = np.array(df["stime"]).tolist()
cutime1 = np.array(df["cutime"]).tolist()
cstime1 = np.array(df["cstime"]).tolist()
total_time1= np.array(df["total_time"]).tolist()
i=0
cpu_measure1=[]
while utime1[i+1]!=-1:
    process_time=(utime1[i+1] + stime1[i+1] + cutime1[i+1]+ cstime1[i+1]) - (utime1[i]+stime1[i]+cutime1[i]+cstime1[i])
    all_time = (total_time1[i+1]-total_time1[i])
    cpu_measure1.append((process_time/all_time)*100)
    i=i+1
df = pd.read_csv("data/cpu-5.csv")
utime2 = np.array(df["utime"]).tolist()
stime2 = np.array(df["stime"]).tolist()
cutime2 = np.array(df["cutime"]).tolist()
cstime2 = np.array(df["cstime"]).tolist()
total_time2= np.array(df["total_time"]).tolist()
i=0
cpu_measure2=[]
while utime2[i+1]!=-1:
    process_time=(utime2[i+1] + stime2[i+1] + cutime2[i+1]+ cstime2[i+1]) - (utime2[i]+stime2[i]+cutime2[i]+cstime2[i])
    all_time = (total_time2[i+1]-total_time2[i])
    cpu_measure2.append((process_time/all_time)*100)
    i=i+1

length = min(len(cpu_measure1),len(cpu_measure2))
x =np.arange(length)
plt.plot(x,cpu_measure1[0:length],label = "nice:0",linewidth=3)
plt.plot(x,cpu_measure2[0:length],label = "nice:5",linewidth=3)
#plt.ylim(0,50)
plt.xlabel("time/ms",fontsize=20)
plt.ylabel("cpu usage/%",fontsize=20)
plt.legend()
plt.title("cpu占有率对比",fontsize=20)
plt.show()