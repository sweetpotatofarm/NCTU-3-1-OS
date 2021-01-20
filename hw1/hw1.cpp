#include <iostream>
#include <stdio.h> 
#include <string>
#include <sstream>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h> 
#include <fcntl.h>
#include <fstream>

using namespace std; 

int main()
{
    while(1){
        //1.Display the prompt sign “>” and take a string from user
        cout<<">";

        //2.Parse the string into a program name and arguments
        //(devide whole line into strings and put in vector)
        string s;
        getline(cin,s);
        stringstream ss(s);
        vector<string> inputString;
        inputString.push_back("");
        int i=0;
        while(ss>>s){
            inputString[i] = s;
            inputString.push_back("");
            i++;
        }
        inputString.pop_back();

        
        //(if there is "&" symbol, no need to wait)
        bool waitByItsParent = true;
        if(inputString[inputString.size()-1] == "&"){
            inputString.pop_back();
            waitByItsParent = false;
        }

        //(if there is a ">" symbol, redirect)
        //(if there is a "|" symbol, pipe)
        bool redirectionTo = false;
        bool redirectionFrom = false;
        bool pipeLine = false;
        vector<string> inputString2;
        int inputString2Count = 0; 
        //(to store destination)
        string destination = "";
        for(i=0; i<inputString.size(); i++){
            if(inputString[i] == ">"){
                redirectionTo = true;
                destination = inputString[i+1];
                inputString.pop_back();
                inputString.pop_back();
                break;
            }
            else if(inputString[i] == "<"){
                redirectionFrom = true;
                destination = inputString[i+1];
                inputString.pop_back();
                inputString.pop_back();
                break;
            }
            else if(inputString[i] == "|"){
                pipeLine = true;
                inputString2Count++;
            }
            else if(pipeLine == true){
                inputString2.push_back(inputString[i]);
                inputString2Count++;
            }
        }
        for(i=0; i<inputString2Count; i++){
            inputString.pop_back();
        }
        
        //(change string into char*)
        char* inputData[inputString.size()+1];
        for(int j=0; j<inputString.size(); j++){
            inputData[j] = strdup(inputString[j].c_str());
        }
        inputData[inputString.size()] = NULL;

        char* dst = strdup(destination.c_str());

        char* inputData2[inputString2.size()+1];
        for(int j=0; j<inputString2.size(); j++){
            inputData2[j] = strdup(inputString2[j].c_str());
        }
        inputData2[inputString2.size()] = NULL;
        
        pid_t pid;

        //3.Fork a child process
        //(fork another process)
        pid = fork();
        signal(SIGCHLD,SIG_IGN);

        if(pid == -1){
            fprintf(stderr, "Fork failed");
            exit(-1);
        }
        else if(pid == 0){
            //child process

        //4.Have the child process execute the program
            if(redirectionTo == true){
                //">"
                int file_desc = open(dst, O_WRONLY | O_TRUNC);
                dup2(file_desc, 1);
                execvp(inputData[0], inputData);
                cout<<"command not found"<<endl;
            }
            else if(redirectionFrom == true){
                //"<"
                int file_desc = open(dst, O_RDONLY);
                dup2(file_desc, 0);
                execvp(inputData[0], inputData);
                cout<<"command not found"<<endl;
            }
            else if(pipeLine == true){
                //"|"
                //(pipeline)
                int p[2];
                //(create pipe)
                pipe(p);
                pid_t pid2;
                pid2 = fork();
                if(pid2 == -1){
                    exit(-1);
                }
                else if(pid2 == 0){
                    close(p[0]);
                    dup2(p[1], 1);
                    close(p[1]);
                    execvp(inputData[0], inputData);
                }
                else{
                    close(p[1]);
                    dup2(p[0], 0);
                    close(p[0]);
                    execvp(inputData2[0], inputData2);
                    wait(NULL);
                }
            }
            else{
                execvp(inputData[0], inputData);
                cout<<"command not found"<<endl;
            }
            exit(0);
        }
        else{
            //parent process
        //5.Wait until the child terminates
            if(waitByItsParent == true){
                // wait(NULL);
                int status;
                waitpid(pid, &status, 0);
            }
        }
        
    }
    return 0;
}