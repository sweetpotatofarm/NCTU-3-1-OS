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

using namespace std;

vector<int> combineArr(vector<int> v1, vector<int> v2){
    vector<int> v;
    while(v1.size() != 0 || v2.size() != 0){
        if(v1.size() == 0){
            v.push_back(v2[0]);
        }
        else if(v2.size() == 0){
            v.push_back(v1[0]);
        }
        else{
            v.push_back(v1[0]<v2[0]?v1[0]:v2[0]);
        }
    }
    return v;
}

int main(){
    printf("abc\n");
    return 0;
}