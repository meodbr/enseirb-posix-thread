#!/bin/env python3

import matplotlib.pyplot as plt

import subprocess
import time
import sys

def run_benchmark(program, args):
    start_time = time.time()
    subprocess.run([program] + args)
    end_time = time.time()
    return end_time - start_time

def plot_results(results,test_name,variable,fixed_variable,value,log=False):
    fig, ax = plt.subplots()
    if log:
        plt.xscale("log")
    for impl in set(result[0] for result in results):
        impl_results = [r for r in results if r[0] == impl]
        impl_results.sort(key=lambda x: x[1]) 
        threads = [r[1] for r in impl_results]
        times = [r[2] for r in impl_results]
        ax.plot(threads, times, label=impl)

    ax.set_xlabel('Number of '+variable)
    ax.set_ylabel('Time in second')
    if fixed_variable:  # Check if fixed_variable is not empty
        ax.set_title(f'Execution time of {test_name} with different number of {variable} '
                     f'and a fixed number of {fixed_variable} = {value}')
    else:
        ax.set_title(f'Execution time of {test_name} with different number of {variable}')
    ax.legend()

    plt.show()

# this function takes into consideration the run time of a program and runs it an appropriate amount of times
# to get the average time

def average_time(program, args):
    num_tests = 1
    execution_times = []

    while True:
        run_time = run_benchmark(program, args)

        if run_time < 1e-6:  
            num_tests = 500 
        elif run_time < 1e-4:
            num_tests = 100
        elif run_time > 5.0:  
            num_tests = 10   
        else:
            num_tests = 50
            break  

    for _ in range(num_tests):
        execution_times.append(run_benchmark(program, args))

    # Calculate average execution time
    average_execution_time = sum(execution_times) / len(execution_times)
    return average_execution_time

def run_test():
    results = []
    results_thread = []
    results_yield = []
    all_tests=[results,results_thread,results_yield]
    if len(sys.argv) < 6:
        print("Usage: python3 plot_graph.py <test_path> <scale_type> <min> <max> <step>")
        sys.exit(1)
    else:
        test_path = sys.argv[1]
        print(test_path)
        if (len(sys.argv) == 6):
            args = sys.argv[2:]
            threads = []
            if args[0] == "lin":
                threads=[i for i in range(int(args[1]), int(args[2])+1, int(args[3]))]
            elif args[0] == "log":
                if int(args[1]) <= 0:
                    print("Min value for logarithmic scale should be > 0")
                    sys.exit(1)
                if int(args[3]) <= 1:
                    print("Step for logarithmic scale should be > 1")
                    sys.exit(1)
                i = int(args[1])
                while i < int(args[2])+1:
                    print("args[3] : ", args[3])
                    threads.append(i)
                    i *= int(args[3])
            else:
                print("Wrong scale_type format, should be lin or log")
                sys.exit(1)
                 
            for i in threads:
                results.append(("thread",i , average_time(test_path, [str(i)])))
                results.append(("pthread",i , average_time(test_path+"-pthread", [str(i)])))
        if (len(sys.argv) == 10):
            args = sys.argv[2:]
            threads=[]
            if args[0] == "lin":
                threads=[i for i in range(int(args[1]), int(args[2])+1, int(args[3]))]
            elif args[0] == "log":
                if int(args[1]) <= 0:
                    print("Min value for logarithmic scale should be > 0")
                    sys.exit(1)
                if int(args[3]) <= 1:
                    print("Step for logarithmic scale should be > 1")
                    sys.exit(1)
                i = int(args[1])
                while i < int(args[2])+1:
                    threads.append(i)
                    i *= int(args[3])
            else:
                print("Wrong scale_type format, should be lin or log")
                sys.exit(1)
            yields=[]
            if args[4] == "lin":
                yields=[i for i in range(int(args[5]), int(args[6])+1, int(args[7]))]
            elif args[4] == "log":
                if int(args[5]) <= 0:
                    print("Min value for logarithmic scale should be > 0")
                    sys.exit(1)
                if int(args[7]) <= 1:
                    print("Step for logarithmic scale should be > 1")
                    sys.exit(1)
                i = int(args[5])
                while i < int(args[6])+1:
                    yields.append(i)
                    i *= int(args[7])
            else:
                print("Wrong scale_type format, should be lin or log")
                sys.exit(1)
            # two graphs, one where we fix the number of threads and vary the number of yields and then 
            #varying the number of threads
            for i in threads:
                args_thread = ["{i}","{(int(args[6])-int(args[5]))/2}"]
                results_thread.append(("thread",i , average_time(test_path, args_thread)))
                results_thread.append(("pthread",i , average_time(test_path+"-pthread", args_thread)))
            #varying the number of yields
            for i in yields:
                args_yields = ["{(int(args[2])-int(args[1]))/2}","{i}"]
                results_yield.append(("thread",i , run_benchmark(test_path, args_yields)))
                results_yield.append(("pthread",i , run_benchmark(test_path+"-pthread", args_yields)))
    test_name=test_path.split("/")[-1]
    if (len(results) > 0):
            if sys.argv[2] == "lin":
                plot_results(results,test_name,"threads","",0, False)
            else:
                plot_results(results,test_name,"threads","",0, True)
    if (len(results_thread) > 0):
            if sys.argv[2] == "lin":
                plot_results(results_thread,test_name,"threads","yields",round((int(args[6])-int(args[5]))/2), False)
            else:
                plot_results(results_thread,test_name,"threads","yields",round((int(args[6])-int(args[5]))/2), True)
    if (len(results_yield) > 0):    
            if sys.argv[6] == "lin":
                plot_results(results_yield,test_name,"yields","threads",round((int(args[2])-int(args[1]))/2), False)
            else:
                plot_results(results_yield,test_name,"yields","threads",round((int(args[2])-int(args[1]))/2), True)
    #plot_results(results)
    

run_test()