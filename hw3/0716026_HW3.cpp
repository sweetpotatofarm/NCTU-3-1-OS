/*
Student No.: 0716026 Student Name: 蕭楚澔 Email: bobhsiao.cs07@nctu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not supposed to be posted to a public server, such as a public GitHub repository or a public web page.
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

//do bubblesort
vector<int> bubbleSort(vector<int> v){
    for(int i=0; i<v.size()-1; i++){
        for(int j=0; j<v.size()-1-i; j++){
            if(v[j+1]<v[j]){
                int temp = v[j];
                v[j] = v[j+1];
                v[j+1] = temp;
            }
        }
    }
    return v;
}

//do mergesort
vector<int> combineArr(vector<int> v1, vector<int> v2){
    vector<int> v;
    while(v1.size() != 0 || v2.size() != 0){
        if(v1.size() == 0){
            v.push_back(v2[0]);
            v2.erase(v2.begin());
        }
        else if(v2.size() == 0){
            v.push_back(v1[0]);
            v1.erase(v1.begin());
        }
        else{
            if(v1[0]<v2[0]){
                v.push_back(v1[0]);
                v1.erase(v1.begin());
            }
            else{
                v.push_back(v2[0]);
                v2.erase(v2.begin());
            }
            
        }
    }
    return v;
}


class thread{
public:
    //constructor
    thread(int r){
        this->row = r;
        sem_init(&mutex, 0, 1);
    }
    //level of thread
    int row;
    //sub array that needs to sort
    vector<int> subArrNum;
    //mutex for communicating with upper-level thread
    sem_t mutex;
    //devide array into two subarray, 1 for part 1, 2 for part 2
    vector<int> devide(int n){
        vector<int> v;
        int sizeOfArray = this->subArrNum.size();
        if(n == 1){
            for(int i=0; i<sizeOfArray/2; i++){
                v.push_back(subArrNum[i]);
            }
        }
        else{
            for(int i=sizeOfArray/2; i<sizeOfArray; i++){
                v.push_back(subArrNum[i]);
            }
        }
        return v;
    }
};


void* handler(void* data){
    //the main thread
    thread* T1 = (thread*) data;
    //not the bottom level
    if(T1->row != 4){
        //create child thread 1
        thread T2(T1->row+1);
        T2.subArrNum = T1->devide(1);
        sem_wait(&T2.mutex);
        pthread_t thread2;
        pthread_create(&thread2, NULL, handler, (void*)&T2);
        //create child thread 2
        thread T3(T1->row+1);
        T3.subArrNum = T1->devide(2);
        sem_wait(&T3.mutex);
        pthread_t thread3;
        pthread_create(&thread3, NULL, handler, (void*)&T3);
        //wait for child thread to finish
        sem_wait(&T2.mutex);
        sem_wait(&T3.mutex);
        //do mergesort
        T1->subArrNum = combineArr(T2.subArrNum, T3.subArrNum);
        //tell the upper thread that I'm finish
        sem_post(&T1->mutex);
    }
    //bottom level
    else{
        //do bubblesort
        T1->subArrNum = bubbleSort(T1->subArrNum);
        //tell the upper thread that I'm finish
        sem_post(&T1->mutex);
    }
}

thread singleHandler(thread S1){
    if(S1.row != 4){
        //create child 1
        thread S2(S1.row+1);
        S2.subArrNum = S1.devide(1);
        S2 = singleHandler(S2);
        //create child 2
        thread S3(S1.row+1);
        S3.subArrNum = S1.devide(2);
        S3 = singleHandler(S3);
        S1.subArrNum = combineArr(S2.subArrNum, S3.subArrNum);
    }
    else{
        //do bubblesort
        S1.subArrNum = bubbleSort(S1.subArrNum);
    }
    return S1;
}

int main()
{
    //read file
    int totalOfInt;
    cin>>totalOfInt;
    vector<int> sortNum;
    for(int i=0; i<totalOfInt; i++){
        int temp;
        cin>>temp;
        sortNum.push_back(temp);
    }

    //multithread
    //time start
    struct timeval start, end;
    gettimeofday(&start, 0);

    //create thread T1
    thread T(1);
    T.subArrNum = sortNum;
    pthread_t thread1;
    pthread_create(&thread1, NULL, handler, (void*)&T);
    //T1's semaphore is 1 right now, need to be zero
    sem_wait(&T.mutex);
    //wait until T1 is finished
    sem_wait(&T.mutex);

    //time end
    gettimeofday(&end, 0);
    int sec = end.tv_sec - start.tv_sec;
    int usec = end.tv_usec - start.tv_usec;
    printf("multithread: elapsed %f ms\n", sec*1000+(usec/1000.0));
    
    //write file
    fstream file1;
    file1.open("output1.txt", ios::out);
    for(int i=0; i<totalOfInt; i++){
        file1<<T.subArrNum[i]<<" ";
    }
    file1.close();

    //single thread
    //time start
    gettimeofday(&start, 0);

    //vector<int> sig = bubbleSort(sortNum);
    thread S(1);
    S.subArrNum = sortNum;
    S = singleHandler(S);
    vector<int> sig = S.subArrNum;

    //time end
    gettimeofday(&end, 0);
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    printf("singlethread: elapsed %f ms\n", sec*1000+(usec/1000.0));
    fstream file2;
    file2.open("output2.txt", ios::out);
    for(int i=0; i<totalOfInt; i++){
        file2<<sig[i]<<" ";
    }
    file2.close();
    return 0;
}