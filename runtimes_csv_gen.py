'''

Little script to run this over some inputs;


Just writes out to results.csv in root dir


It was a little more useful to open the CSV each time and add. Easier to identifier outliers etc blah




'''
import csv

import os
import time

TRIALS = 1
thread_nums = [i for i in range(1,20,2)]

print([i for i in range(1,20,2)])
#
files = ['small.txt']
write_to = 'compbin.csv'

def run(exec_):
    ''' simply execute the defined string on bash; 


    return execution time 
    '''
    start = time.time()
    os.system(exec_)
   # exec(exec_)
    end = time.time()

    return(end - start)

with open(write_to, 'a', newline='') as csvfile:
            spamwriter = csv.writer(csvfile, delimiter=',',
                                    quotechar='|', quoting=csv.QUOTE_MINIMAL)
            row = ["FileName","THREADS","TRIALS","AVERAGE T"]
            spamwriter.writerow(row)



for file_ in files:
    for threads in thread_nums:
     #   time.sleep(3)
        print(f"NOW DOING {file_} for {threads}")
        run_this = (f'./downloader {file_} {threads} download/ppp')

        trial_n = 0
        for trial in range(TRIALS):
            trial_n += run(run_this)
        average = trial_n/TRIALS
        with open(write_to, 'a', newline='') as csvfile:
            spamwriter = csv.writer(csvfile, delimiter=',',
                                    quotechar='|', quoting=csv.QUOTE_MINIMAL)
            row = [file_,threads,TRIALS,average]
            spamwriter.writerow(row)


    print(average)





TRIALS = 1
thread_nums = [i for i in range(0,100,25)]
#
files = ['small.txt','unsplash.txt']
write_to = 'bigthreads.csv'

def run(exec_):
    start = time.time()
    os.system(exec_)
   # exec(exec_)
    end = time.time()

    return(end - start)

with open(write_to, 'a', newline='') as csvfile:
            spamwriter = csv.writer(csvfile, delimiter=',',
                                    quotechar='|', quoting=csv.QUOTE_MINIMAL)
            row = ["FileName","THREADS","TRIALS","AVERAGE T"]
            spamwriter.writerow(row)



for file_ in files:
    for threads in thread_nums:
     #   time.sleep(3)
        print(f"NOW DOING {file_} for {threads}")
        run_this = (f'./downloader {file_} {threads} download/ppp')

        trial_n = 0
        for trial in range(TRIALS):
            trial_n += run(run_this)
        average = trial_n/TRIALS
        with open(write_to, 'a', newline='') as csvfile:
            spamwriter = csv.writer(csvfile, delimiter=',',
                                    quotechar='|', quoting=csv.QUOTE_MINIMAL)
            row = [file_,threads,TRIALS,average]
            spamwriter.writerow(row)


    print(average)



  