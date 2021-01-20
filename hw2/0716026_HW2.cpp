/*
Student No.: 0716026
Student Name: 蕭楚澔 
Email: bobhsiao.cs07@nctu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not supposed to be posted to a public server, such as a public GitHub repository or a public web page.
*/
#include <iostream>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>

using namespace std;

int dimension;

int multi(int *ptr, int n, int m){
    int sum = 0;
    for(int i=0; i<dimension; i++){
        sum += (*(ptr+n*dimension+i))*(*(ptr+i*dimension+m));
    }
    return sum;
}

int main()
{
    signal(SIGCHLD,SIG_IGN);
    cin>>dimension;

    int arrA[dimension][dimension];
    int k=0;
    for(int i=0; i<dimension; i++){
        for(int j=0; j<dimension; j++){
            arrA[i][j] = k;
            k++;
        }
    }

    //get share memory(arrA)
    int init_shmid = shmget(IPC_PRIVATE, 128000000, IPC_CREAT|0666);
    if(init_shmid < 0){
        perror("shmid error");
        return 0;
    }

    //attach share memory(arrA)
    int *init_addr = (int *)shmat(init_shmid, NULL, 0);
    for(int j=0; j<dimension; j++){
        for(int k=0; k<dimension; k++){
            *(init_addr+j*dimension+k) = arrA[j][k];
        }
    }

    for(int process=1; process<=16; process++){
        //store start time
        struct timeval start, end;
        gettimeofday(&start, 0);

        //get share memory(arrC)
        int str_shmid = shmget(IPC_PRIVATE, 128000000, IPC_CREAT|0666);

        if(str_shmid < 0){
            perror("shmid error");
            return 0;
        }

        //attach share memory(arrC)
        int *str_addr = (int *)shmat(str_shmid, NULL, 0);

        //calculate each part should do how many rows
        int pt1 = dimension/process; 
        int pt2 = pt1 + 1; 
        //how many rows in part 2
        int how_many_row = dimension - process*(pt1);

        int i = process;
        
        //used to calculate start and end
        int row_start = 0;
        int row_end = 0;

        while(i>0){
            pid_t pid;
            pid = fork();
            if(pid == 0){
                //child
                if(i <= how_many_row){
                    //part 2 (= remainder)
                    row_start = pt1*(process - how_many_row) + pt2*(how_many_row - i);
                    row_end = row_start + pt2;
                    //multiply matrices
                    for(int j=row_start; j<row_end; j++){
                        for(int k=0; k<dimension; k++){
                            *(str_addr+j*dimension+k) = multi(init_addr, j, k);
                        }
                    }
                    row_start = row_end;
                }
                else{
                    //part 1
                    row_start = pt1*(process - i);
                    row_end = row_start + pt1;
                    //multiply matrices
                    for(int j=row_start; j<row_end; j++){
                        for(int k=0; k<dimension; k++){
                            *(str_addr+j*dimension+k) = multi(init_addr, j, k);
                        }
                    }
                    row_start = row_end;
                }
                exit(0);
            }
            i--;
        }
        wait(NULL);

        //calculate checksum
        unsigned int checksum = 0;
        for(int j=0; j<dimension; j++){
            for(int k=0; k<dimension; k++){
                checksum+=*(str_addr+j*dimension+k);
            }
        }
        
        //get end time
        gettimeofday(&end, 0);
        int sec = end.tv_sec - start.tv_sec;
        int usec = end.tv_usec - start.tv_usec;

        //ans
        printf("Multiplying matrices using %d processes\n", process);
        printf("Elapsed time: %f sec, Checksum: %u\n", sec*1000+(usec/1000.0), checksum);
        
        //share memory detach(arrC)
        void *addr = (void *)str_addr;
        int str_dt = shmdt(addr);

        //share memory control(remove)
        shmctl(str_shmid, IPC_RMID, NULL) ;
    }

    return 0;
}