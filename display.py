import matplotlib.pyplot as plt
import os
import time
import sys 
import platform


# exemple of execution : python3 display.py 01-main

# execute an exemple with the plot of performence
class PLot_performence():
        def __init__(self,exemple="01-main ",args=[""]):
                self.exemple= exemple
                self.args = args # a list of string
                self.machine= platform.machine()

        # return the execution time for pthread and thread in the current configuration
        def calcul_exec_time(self):
                threadsystemtime=0
                pthreadsystemtime=0
                args=""
                for arg in self.args:
                        args+=arg+" "
                #thread
                os.system("make "+ self.exemple + " >/dev/null 2>/dev/null")
                time1=time.time_ns()
                os.system("./install/bin/"+self.exemple + args + " >/dev/null 2>/dev/null")
                time2=time.time_ns()
                threadsystemtime = time2-time1
                # pthread
                os.system("make "+ self.exemple+ " >/dev/null 2>/dev/null")
                time1=time.time_ns()
                os.system("./install/bin/"+self.exemple + args + " >/dev/null 2>/dev/null")
                time2=time.time_ns()
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
                plt.figure("All test without args")
                plt.subplot(1, 2, 1)
                plt.xlabel("Number of the test")
                plt.ylabel("Time (micro second)")
                plt.plot(execorder,pthreadtime,label='pthread')
                plt.plot(execorder,threadtime,label='thread')
                plt.legend(loc='upper right')
                plt.title("system time by using time of python with the machine "+ self.machine)

                plt.subplot(1, 2, 2)
                plt.plot(difftime)
                plt.title("Diff time (ns)")
                plt.xlabel("Number of the test")
                plt.ylabel("Time (micro second)")
                figManager = plt.get_current_fig_manager()
                figManager.resize(5000,5000)
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
                plt.figure(exemple +" arg variable "+ str(indice+1))
                plt.subplot(1, 2, 1)
                plt.plot(list_test_args,pthreadtime,'-*',label='pthread')
                plt.plot(list_test_args,threadtime,'-*',label='thread')
                plt.xlabel(" Value of the argument")
                plt.ylabel(" Time (micro second)")
                plt.legend(loc='upper right')
                plt.title("system time by using time of python with the machine "+ self.machine)
                plt.subplot(1, 2, 2)

                plt.plot(list_test_args,difftime,'-*r')
                plt.title("Diff time")
                plt.xlabel(" Value of the argument")
                plt.ylabel(" Time (micro second)")
                figManager = plt.get_current_fig_manager()
                figManager.resize(5000,5000)
                plt.show()

        def plot_multi_exec(self,exemple,args=[""],nb_execution=50):
                self.exemple = exemple
                self.args = args
                pthreadtime=[]
                threadtime=[]
                difftime=[]
                for i in range(nb_execution):
                        ttime , pttime = self.calcul_exec_time()
                        threadtime.append(ttime)
                        pthreadtime.append(pttime)
                        difftime.append(pttime-ttime)
                plt.figure(exemple +" multi execution number of execution: "+ str(nb_execution))
                plt.subplot(1, 2, 1)
                plt.plot(pthreadtime,'-*',label='pthread')
                plt.plot(threadtime,'-*',label='thread')
                plt.legend(loc='upper right')
                plt.title("System time by using time of python with the machine "+ self.machine)
                plt.xlabel(" Number of the execution")
                plt.ylabel(" Time (micro second)")
                plt.subplot(1, 2, 2)

                plt.plot(difftime,'-*r')
                plt.title("Diff time")
                plt.xlabel(" Number of the execution")
                plt.ylabel(" Time (micro second)")
                figManager = plt.get_current_fig_manager()
                figManager.resize(5000,5000)
                plt.show()








                
      
plot=PLot_performence()

if len(sys.argv) != 2:
    print( "error: too much argument, pls enter the name of the test, or run all for run all the performence tests")
    exit()
else:
    test_name= sys.argv[1]
    match test_name:
        case "01-main":
                plot.plot_multi_exec("01-main",nb_execution=50)
        case "02-switch":
                plot.plot_multi_exec("02-switch",nb_execution=50) 
        case "03-equity":
                plot.plot_multi_exec("03-equity",nb_execution=50)
        case "11-join":
                plot.plot_multi_exec("11-join",nb_execution=50)
        case "11-join-main":
                plot.plot_multi_exec("11-join-main",nb_execution=50) 
        case "21-create-many":
                plot.plot_exec_time_in_fonction_of_arg("21-create-many ",indice= 0,list_initial_args=["0"],list_test_args=[str(500*i) for i in range(20)])
        case "22-create-many-recursive":
                plot.plot_exec_time_in_fonction_of_arg("22-create-many-recursive ",indice= 0,list_initial_args=["0"],list_test_args=[str(500*i) for i in range(20)])
        case "23-create-many-once":
                plot.plot_exec_time_in_fonction_of_arg("23-create-many-once ",indice= 0,list_initial_args=["0"],list_test_args=[str(500*i) for i in range(20)])
        case "51-fibonacci":
                plot.plot_exec_time_in_fonction_of_arg("51-fibonacci ",indice= 0,list_initial_args=["0"],list_test_args=[str(i) for i in range(15)])
        case "61-mutex":
                plot.plot_exec_time_in_fonction_of_arg("61-mutex ",indice= 0,list_initial_args=["0"],list_test_args=[str(i) for i in range(20)])
        case "62-mutex":
                plot.plot_exec_time_in_fonction_of_arg("62-mutex ",indice= 0,list_initial_args=["0"],list_test_args=[str(i) for i in range(20)])   
        case "31-switch-many":
                plot.plot_exec_time_in_fonction_of_arg("31-switch-many ",indice= 0,list_initial_args=["0","10"],list_test_args=[str(500*i) for i in range(20)])
                plot.plot_exec_time_in_fonction_of_arg("31-switch-many ",indice= 1,list_initial_args=["10","0"],list_test_args=[str(500*i) for i in range(20)])
        case "32-switch-many-join":
                plot.plot_exec_time_in_fonction_of_arg("32-switch-many-join ",indice= 0,list_initial_args=["0","10"],list_test_args=[str(500*i) for i in range(20)])
                plot.plot_exec_time_in_fonction_of_arg("32-switch-many-join ",indice= 1,list_initial_args=["10","0"],list_test_args=[str(500*i) for i in range(20)])
        case "33-switch-many-cascade":
                plot.plot_exec_time_in_fonction_of_arg("33-switch-many-cascade ",indice= 0,list_initial_args=["0","10"],list_test_args=[str(500*i) for i in range(20)])
                plot.plot_exec_time_in_fonction_of_arg("33-switch-many-cascade ",indice= 1,list_initial_args=["10","0"],list_test_args=[str(500*i) for i in range(20)])
        case "run_all":
                # run all test without parameters
                ##plot.plot_all_exec_time()

                # run tests without args with multi execution

                plot.plot_multi_exec("01-main",nb_execution=50)
                plot.plot_multi_exec("02-switch",nb_execution=50)
                plot.plot_multi_exec("03-equity",nb_execution=50)
                plot.plot_multi_exec("11-join",nb_execution=50)
                plot.plot_multi_exec("12-join-main",nb_execution=50)

                # run a test in function of the parameters
                # 1 parameters
                plot.plot_exec_time_in_fonction_of_arg("21-create-many ",indice= 0,list_initial_args=["0"],list_test_args=[str(500*i) for i in range(20)])

                plot.plot_exec_time_in_fonction_of_arg("22-create-many-recursive ",indice= 0,list_initial_args=["0"],list_test_args=[str(500*i) for i in range(20)])

                plot.plot_exec_time_in_fonction_of_arg("23-create-many-once ",indice= 0,list_initial_args=["0"],list_test_args=[str(500*i) for i in range(20)])

                plot.plot_exec_time_in_fonction_of_arg("51-fibonacci ",indice= 0,list_initial_args=["0"],list_test_args=[str(i) for i in range(15)])

                plot.plot_exec_time_in_fonction_of_arg("61-mutex ",indice= 0,list_initial_args=["0"],list_test_args=[str(i) for i in range(20)])

                plot.plot_exec_time_in_fonction_of_arg("62-mutex  ",indice= 0,list_initial_args=["0"],list_test_args=[str(i) for i in range(20)])


                # 2 parameters

                plot.plot_exec_time_in_fonction_of_arg("31-switch-many ",indice= 0,list_initial_args=["0","10"],list_test_args=[str(500*i) for i in range(20)])
                plot.plot_exec_time_in_fonction_of_arg("31-switch-many ",indice= 1,list_initial_args=["10","0"],list_test_args=[str(500*i) for i in range(20)])

                plot.plot_exec_time_in_fonction_of_arg("32-switch-many-join ",indice= 0,list_initial_args=["0","10"],list_test_args=[str(500*i) for i in range(20)])
                plot.plot_exec_time_in_fonction_of_arg("32-switch-many-join ",indice= 1,list_initial_args=["10","0"],list_test_args=[str(500*i) for i in range(20)])

                plot.plot_exec_time_in_fonction_of_arg("32-switch-many-cascade ",indice= 0,list_initial_args=["0","10"],list_test_args=[str(500*i) for i in range(20)])
                plot.plot_exec_time_in_fonction_of_arg("32-switch-many-cascade ",indice= 1,list_initial_args=["10","0"],list_test_args=[str(500*i) for i in range(20)])
                    



