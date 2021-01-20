#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

int main()
{
    //read file
    int totalOfInt;

    vector<int> test;
    fstream filet;
    char fileName[] = "input.txt";
    filet.open(fileName, ios::in);
    char numt[128];
    int first = 2;
    while(filet.getline(numt, sizeof(numt), ' ')){
        if(first > 0){
            cout<<"firstnumt"<<numt<<endl;
            totalOfInt = atoi(numt);
            first --;
        }
        else{
            int temp = atoi(numt);
            test.push_back(temp);
        }
    }
    filet.close();
    cout<<"test"<<test[0]<<endl;

    vector<int> v1;
    fstream file;
    char fileName1[] = "output_1.txt";
    file.open(fileName1, ios::in);
    int num;
    while(file>>num){
        v1.push_back(num);
    }
    file.close();

    vector<int> v2;
    char fileName2[] = "output_ans.txt";
    file.open(fileName2, ios::in);
    while(file>>num){
        v2.push_back(num);
    }
    file.close();

    cout<<"v1"<<v1.size()<<endl;
    cout<<"v2"<<v2.size()<<endl;

    int c1 = 0;
    int c2 = 0;
    vector<int> v1lost;
    vector<int> v2lost;
    while(c1 < v1.size() && c2 < v2.size()){
        if(v1[c1] != v2[c2]){
            c2++;
            v2lost.push_back(v2[c2]);
            //cout<<"at "<<c1<<" supposed to be "<<v2[c2]<<" here"<<endl;
        }
        else{
            c1++;
            c2++;
        }
    }
    while(c1 < v1.size()){
        v1lost.push_back(v1[c1]);
        c1++;
    }

    cout<<"v1lost"<<endl;
    for(int i=0; i<v1lost.size(); i++){
        cout<<v1lost[i]<<endl;
    }
    cout<<"v2lost"<<endl;
    for(int i=0; i<v2lost.size(); i++){
        cout<<v2lost[i]<<endl;
    }
}