import matplotlib.pyplot as plt
import os
import time
import sys 




# execute an exemple with the plot of performence
class PLot_performence():
        def __init__(self,exemple="00-test ",args=[""]):
                self.exemple= exemple
                self.args = args # a list of string

        # return the execution time for pthread and thread in the current configuration
        def calcul_exec_time(self):
                #threadusertime=[]
                threadsystemtime=0
                #pthreadusertime=[]
                pthreadsystemtime=0
                args=""
                for arg in self.args:
                        args+=arg+" "
                #thread
                os.system("make "+ self.exemple + " >/dev/null 2>/dev/null")
                time1=time.time()
                os.system("./"+self.exemple + args + " >/dev/null 2>/dev/null")
                time2=time.time()
                #print("diff timethread=",time2-time1)
                #threadusertime.append(time2.user-time1.user)
                threadsystemtime = time2-time1
                # pthread
                os.system("make "+ self.exemple+ " >/dev/null 2>/dev/null")
                # time1=os.times()
                time1=time.time()
                os.system("./"+self.exemple + args + " >/dev/null 2>/dev/null")
                # time2=os.times()
                time2=time.time()
                #print("diff timepthread=",time2-time1)
                #pthreadusertime.append(time2-time1)
                pthreadsystemtime = time2-time1
                return threadsystemtime , pthreadsystemtime
        

        # plot the curve of all the executions of all the programms without any arguments
        def plot_all_exec_time(self):
                pthreadtime=[]
                threadtime=[]
                difftime=[]
                execorder=[]
                for f in os.listdir("./test"):
                        print(f)
                        execorder.append(f)
                        self.exemple = f
                        ttime , pttime = self.calcul_exec_time()
                        threadtime.append(ttime)
                        pthreadtime.append(pttime)
                        difftime.append(pttime-ttime)
                plt.subplot(1, 2, 1)
                plt.plot(execorder,pthreadtime,label='pthread')
                plt.plot(execorder,threadtime,label='thread')
                plt.legend(loc='upper right')
                plt.title("system time")
                plt.subplot(1, 2, 2)

                plt.plot(difftime)
                plt.title("diff time")
                plt.show()
                
        # plot the curves of evolution of the performence time in function of the parameter indice for  the program exemple

        def plot_exec_time_in_fonction_of_arg(self,exemple,indice,list_initial_args,list_test_args):
                if(indice>len(list_initial_args)):
                        print("enter a valid index, it should be inferior to your initialise list", len(list_initial_args))
                self.exemple = exemple
                self.args = list_initial_args
                pthreadtime=[]
                threadtime=[]
                difftime=[]
                for i in range(len(list_test_args)):
                        self.args[indice]=list_test_args[i]
                        ttime , pttime = self.calcul_exec_time()
                        threadtime.append(ttime)
                        pthreadtime.append(pttime)
                        difftime.append(pttime-ttime)

                plt.subplot(1, 2, 1)
                plt.plot(list_test_args,pthreadtime,'-*',label='pthread')
                plt.plot(list_test_args,threadtime,'-*',label='thread')
                plt.legend(loc='upper right')
                plt.title("system time")
                plt.subplot(1, 2, 2)

                plt.plot(list_test_args,difftime,'-*r')
                plt.title("diff time")
                plt.show()




                
                
plot=PLot_performence()
#plot.plot_all_exec_time()
plot.plot_exec_time_in_fonction_of_arg("21-create-many ",indice= 0,list_initial_args=["0"],list_test_args=[str(10*i) for i in range(10)])