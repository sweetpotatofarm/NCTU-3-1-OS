/*
Student No.: 0716026 
Student Name: 蕭楚澔 
Email: bobhsiao.cs07@nctu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not 
supposed to be posted to a public server, such as a 
public GitHub repository or a public web page.
*/

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/time.h>

using namespace std;

//global variable

//input number(unsorted)
vector<int> notSortNum;
//number to sort
vector<int> sortNum;
//job queue
vector<pair<int, pair<int, int> > > jobList;
//thread queue
pthread_t thread[8];
//dispatcher
pthread_t dispatcher;
//semaphore for thread pool
sem_t sem_thread;
//semaphore for dispatcher
sem_t sem_dispatcher;
//semaphore for jobs
sem_t sem_job;
//semaphore for finish
sem_t sem_finish;
//sort finish or not
bool finishSort;
//total thread in this session
int totalThread;
//see if one of the job is finished
bool jobFinished[15];
//check is deviding or not
bool deviding;
//how many elements in a subarray
int arrNum[15];
//start and end in subarray
pair<int, int> start_end[15];
//job in list
bool jobInList[15];
//time used in one thread
float timeUsedOfOneThread;

//do bubblesort
void bubbleSort(vector<int> &v, int start, int end){
    for(int i=start; i<end; i++){
        for(int j=start; j<end; j++){
            if(v[j+1]<v[j]){
                int temp = v[j];
                v[j] = v[j+1];
                v[j+1] = temp;
            }
        }
    }
}

//do mergesort
void mergeSort(vector<int> &v, int start, int end){
    int start1 = start;
    int end1, start2;
    int end2 = end;
    bool flag = true;
    for(int i=start; i<end; i++){
        if(v[i]>v[i+1]){
            end1 = i;
            start2 = i+1;
            flag = false;
            break;
        }
    }

    if(flag == false){
        vector<int> v1;
        vector<int> v2;
        v1.assign(v.begin()+start1, v.begin()+end1+1);
        v2.assign(v.begin()+start2, v.begin()+end2+1);
        vector<int> ans;
        while(v1.size() != 0 || v2.size() != 0){
            if(v1.size() == 0){
                ans.push_back(v2[0]);
                v2.erase(v2.begin());
            }
            else if(v2.size() == 0){
                ans.push_back(v1[0]);
                v1.erase(v1.begin());
            }
            else{
                if(v1[0]<v2[0]){
                    ans.push_back(v1[0]);
                    v1.erase(v1.begin());
                }
                else{
                    ans.push_back(v2[0]);
                    v2.erase(v2.begin());
                }
            }
        }
        for(int i=start, j=0; i<end+1; i++, j++){
            v[i] = ans[j];
        }
    }
}

void* dispatch(void* data)
{
    while(finishSort == false){
        sem_wait(&sem_dispatcher);

        //devide array and put in 8 initial jobs
        if(deviding == true){
            int arrLen = sortNum.size()/8;
            int more = sortNum.size()%8;
            for(int i=0; i<8; i++){
                if(i<more){
                    arrNum[i+7] = arrLen+1;
                }
                else{
                    arrNum[i+7] = arrLen;
                }
            }
            int start = 0;
            int end = start+arrNum[7]-1;
            for(int i=0; i<8; i++){
                //push bottom jobs in joblist
                pair<int, pair<int, int> > T1;
                T1.first = i+7; //task number, 0~6 for merge sort, 7~14 for bubble sort
                pair<int, int> p;
                p.first = start;
                p.second = end;
                T1.second = p;
                jobList.push_back(T1);
                jobInList[i+7] = true;
                //save the start and end in array
                start_end[i+7] = p;
                if(i != 7){
                    start = end+1;
                    end = start+arrNum[i+8]-1;
                }
            }
            deviding = false;
            for(int i=0; i<8; i++){
                sem_post(&sem_thread);
            }

        }

        //check if any two pairing sub-arrays have been sorted
        //if so, put in merge jobs
        for(int i=0; i<7; i++){
            if(jobFinished[i*2+1] == true && jobFinished[i*2+2] == true && jobInList[i] == false){
                //push merge jobs in joblist
                pair<int, pair<int, int> > T2;
                T2.first = i;
                pair<int, int> p;
                p.first = start_end[i*2+1].first;
                p.second = start_end[i*2+2].second;
                T2.second = p;
                start_end[i] = p;
                sem_wait(&sem_job);
                jobList.push_back(T2);
                jobInList[i] = true;
                sem_post(&sem_job);

                sem_post(&sem_thread);
            }
        }
    }
    return 0;
}

void* handler(void* data)
{
    while(finishSort == false){
        sem_wait(&sem_thread);
        if(finishSort == true){
            sem_post(&sem_thread);
            return 0;
        }

        //take job
        sem_wait(&sem_job);
        pair<int, pair<int, int> > toDoJob;
        toDoJob = jobList[0];
        jobList.erase(jobList.begin());
        sem_post(&sem_job);

        //check which job to do
        if(toDoJob.first < 7){
            //mergesort
            mergeSort(sortNum, toDoJob.second.first, toDoJob.second.second);
            jobFinished[toDoJob.first] = true;
            sem_post(&sem_dispatcher);

            if(toDoJob.first == 0){
                //finish the last job
                finishSort = true;
                sem_post(&sem_finish);
                return 0;
            }
        }
        else{
            //bubblesort
            bubbleSort(sortNum, toDoJob.second.first, toDoJob.second.second);
            jobFinished[toDoJob.first] = true;
            sem_post(&sem_dispatcher);
        }
    }
    return 0;
}

int main()
{
    //read file
    int totalOfInt;

    fstream file;
    char fileName[] = "input.txt";
    file.open(fileName, ios::in);
    char num[128];
    bool first = true;
    file.getline(num, sizeof(num), '\n');
    totalOfInt = atoi(num);
    while(file.getline(num, sizeof(num), ' ')){
        int temp = atoi(num);
        notSortNum.push_back(temp);
    }
    file.close();

    //multithread from n=1 to n=8
    for(int n=1; n<9; n++){
        //sorting is not finished
        finishSort = false;
        //init sortNum
        sortNum = notSortNum;
        //init totalThread
        totalThread = n;
        //init jobFinish(no job is finished)
        for(int i=0; i<15; i++){
            jobFinished[i] = false;
        }
        //init deviding(now deviding)
        deviding = true;
        //init jobInList(no job is in list)
        for(int i=0; i<15; i++){
            jobInList[i] = false;
        }

        //init semaphore
        sem_init(&sem_dispatcher, 0, 0);
        sem_init(&sem_thread, 0, 0);
        sem_init(&sem_job, 0, 1);
        sem_init(&sem_finish, 0, 0);

        //create thread
        pthread_create(&dispatcher, NULL, &dispatch, NULL);
        for(int i=0; i<totalThread; i++){
            pthread_create(&thread[i], NULL, &handler, NULL);
        }

        //timer start
        struct timeval start, end;
        gettimeofday(&start, 0);

        //call dispatcher
        sem_post(&sem_dispatcher);

        sem_wait(&sem_finish);

        //timer stop
        gettimeofday(&end, 0);
        int sec = end.tv_sec - start.tv_sec;
        int usec = end.tv_usec - start.tv_usec;
        float timeUsed = sec*1000+(usec/1000.0);
        cout<<"Sorting time of "<<n<<"-thread pool: "<<timeUsed<<" ms"<<endl;
        if(n == 1){
            timeUsedOfOneThread = timeUsed;
        }
        cout<<"n thread/1 thread: "<<timeUsed/timeUsedOfOneThread<<endl;

        //write file
        fstream file;
        char fileName[] = "output_n.txt";
        fileName[7] = n + '0';
        file.open(fileName, ios::out);
        for(int i=0; i<totalOfInt; i++){
            // if(i>0 && sortNum[i] == sortNum[i-1]){
            //     file<<endl;
            // }
            file<<sortNum[i]<<" ";
        }
        file.close();

        //clear jobList and sortNum
        jobList.clear();
        sortNum.clear();

        //clear array
        pair<int, int> start_end[15];

        //destroy semaphore
        sem_destroy(&sem_dispatcher);
        sem_destroy(&sem_thread);
        sem_destroy(&sem_job);
        sem_destroy(&sem_finish);
    }

    return 0;
}