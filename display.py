import matplotlib.pyplot as plt
import os
import time
import sys 


# a class to initialize which library we would use
class Thread():
        def __init__(self,type):
                self.type=type
                

# execute an exemple with the plot of performence
class PLot_performence():
        # maybe faire une list d'argument
        def __init__(self,exemple="00-test.c",args=""):
                # self.pthread= Thread(0)
                # self.thread= Thread(1)
                self.exemple= exemple
                self.args = args


        def calcul_exec_time(self):
                #threadusertime=[]
                threadsystemtime=0
                #pthreadusertime=[]
                pthreadsystemtime=0
                #thread
                # os.system("gcc " +"-I . -o exec " + "./test/"+ self.exemple )
                #time1=os.times()
                time1=time.time()
                os.system("./exec")
                time2=time.time()
                print("diff timethread=",time2-time1)
                #time2=os.times()
                #threadusertime.append(time2.user-time1.user)
                threadsystemtime = time2-time1
                # pthread
                os.system("gcc " +"-DUSE_PTHREAD -I . -o exec " + "./test/"+ self.exemple )
                # time1=os.times()
                time1=time.time()
                os.system("./exec")
                # time2=os.times()
                time2=time.time()
                print("diff timepthread=",time2-time1)
                #pthreadusertime.append(time2-time1)
                pthreadsystemtime = time2-time1
                #plot
                #plt.subplot(1, 2, 1)
                # plt.plot(pthreadusertime)
                # plt.plot(threadusertime)

                #plt.subplot(1, 2, 2)
                # plt.plot(pthreadsystemtime)
                # plt.plot(threadsystemtime)

                # plt.show()
                return threadsystemtime , pthreadsystemtime
        
        def plot_all_exec_time(self):
                pthreadtime=[]
                threadtime=[]
                for f in os.listdir("./test"):
                        print(f)
                        self.exemple = f
                        ttime , pttime = self.calcul_exec_time()
                        threadtime.append(ttime)
                        pthreadtime.append(threadtime)
                # plt.subplot(1, 2, 1)
                plt.plot(pthreadtime)
                plt.plot(threadtime)
                plt.legend("system time")
                # plt.subplot(1, 2, 2)
                # plt.plot(pthreadtime-threadtime)
                # plt.legend("diff time")
                plt.show()
                


                
                


plot=PLot_performence()
plot.plot_all_exec_time()