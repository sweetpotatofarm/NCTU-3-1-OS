/*
Student No.: 0716314
Student Name: 陳鎧勳
Email: kaivinnctu.cs07@nctu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not supposed to be posted to a public server, such as a public GitHub repository or a public web page.
*/

#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <regex>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <cstdlib>
#include<time.h>
#include<map>
#include<algorithm>
using namespace std;

struct TarHeader{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char linkflag[1];
    char linkname[100];
    char pad[255];
};


struct new_TarHeader{
    string name;
    int mode;
    char uid[8];
    char gid[8];
    int size;
    int mtime;
    int checksum;
    int linkflag;
    char linkname[100];
    char pad[255];
    bool key;
};


int Oct2Dec(int octNum){
    int decNum = 0;
    int base = 1;
    while(octNum){
        decNum  += (octNum % 10 * base);
        octNum /= 10 ;
        base*=8;
    }
    return decNum;
}


class MyTar{

public:
    MyTar(const char *file){
        this->usBlockSize = 512; 
        this->fileNum = -1;
        this->inputStream.open(file, ifstream::in | ifstream::binary);
    };

    void startRead();  
    int getFileNum();  
    void clearRepeat();
    ifstream inputStream;
    vector<TarHeader> tarVector;
    vector<new_TarHeader> dataVector;
    int usBlockSize;  
    int fileNum;  

    int oct2Dec(const char*, int);  
};


void MyTar::clearRepeat(){
    map<string,pair<int,int>>  timestamp;
    for(int i=0;i<this->tarVector.size()-1;i++){
        int time = this->oct2Dec(this->tarVector[i].mtime, sizeof(this->tarVector.at(i).mtime));
        string node_name = this->tarVector.at(i).name;
        if(timestamp.find(node_name) == timestamp.end()){
            timestamp[this->tarVector.at(i).name].first = time;
            timestamp[this->tarVector.at(i).name].second = i;
        }
        else{
            if(timestamp[this->tarVector.at(i).name].first < time)
                timestamp[this->tarVector.at(i).name].first = time;
                timestamp[this->tarVector.at(i).name].second = i;
        }
    }

    vector<int> index;
    for(auto iter = timestamp.begin(); iter != timestamp.end(); iter++){
        index.push_back(iter->second.second);
    }
    sort(index.begin(),index.end());
    
    for(int i=0,j=0;i<this->tarVector.size()-1;i++){
        new_TarHeader tmp;
        tmp.name = this->tarVector[i].name;
        tmp.mode = Oct2Dec(atoi(this->tarVector[i].mode));
        cout<<this->tarVector[i].uid<<"　"<<this->tarVector[i].gid<<endl;
        // tmp.uid = this->oct2Dec(this->tarVector[i].uid, sizeof(this->tarVector[i].uid));
        // tmp.gid = this->oct2Dec(this->tarVector[i].gid, sizeof(this->tarVector[i].gid));
        strcpy(tmp.uid, this->tarVector[i].uid);
        strcpy(tmp.gid, this->tarVector[i].gid);
        tmp.size = this->oct2Dec(this->tarVector[i].size, sizeof(this->tarVector[i].size));
        this->tarVector[i].mtime[11] = '\0';
        tmp.mtime = (time_t)this->oct2Dec(this->tarVector[i].mtime, sizeof(this->tarVector[i].mtime));
        tmp.checksum = this->oct2Dec(this->tarVector[i].checksum, sizeof(this->tarVector[i].checksum));
        tmp.linkflag = atoi(this->tarVector[i].linkflag);
        strcpy(tmp.linkname,this->tarVector[i].linkname);
        strcpy(tmp.pad,this->tarVector[i].pad);
        this->dataVector.push_back(tmp);
        if(index[j]==i){
            this->dataVector[i].key = 1;
            j++;
        }
        else{
            this->dataVector[i].key = 0;
        }
    }
}


void MyTar::startRead(){
    this->inputStream.seekg(0, ios::beg);

    while(this->inputStream){

        struct TarHeader buffer;

        if(this->inputStream.read((char*)&buffer, this->usBlockSize)){
            this->tarVector.push_back(buffer);

            int fileSize = this->oct2Dec(buffer.size, sizeof(buffer.size));

            int jumpBlock = ceil((double)fileSize / (double)this->usBlockSize);
            
            this->inputStream.seekg(jumpBlock * this->usBlockSize, ios::cur);
        }

    }
}


int MyTar::getFileNum(){

    if(this->fileNum >= 0){
        return this->fileNum;
    }

    for(int i = this->tarVector.size() - 1; i > -1; i--){
        bool zeroFlag = true;
        struct TarHeader buffer = this->tarVector.at(i);

        for(int j = 0; j < (int)sizeof(buffer.checksum); j++){
            if(buffer.checksum[j] != 0x00){
                zeroFlag = false;
                break;
            }
        }

        if(!zeroFlag){
            this->fileNum = i + 1;
            break;
        }
    }

    return this->fileNum;
}


int MyTar::oct2Dec(const char* sizeArray, int length){
    int n = 0;
    int ans = 0;

    for(int i = length - 2; i >= 0; i--){
        int num = sizeArray[i] - '0';
        ans += num * pow(8, n);
        ++n;
    }

    return ans;
}


MyTar myTar("test.tar");


int my_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    string reg_path=(path+1);
    if(reg_path.size()>1)
        reg_path += '/';
    regex reg(reg_path+"([a-zA-z0-9._]*)");
    
    filler(buffer, "." , NULL, 0 );
    filler(buffer, ".." , NULL, 0 );

    for(int i=0; i<myTar.dataVector.size();i++){
        string source_str = myTar.dataVector[i].name;
        if(source_str[source_str.size()-1] == '/')
            source_str.erase(source_str.end()-1);
        if(regex_match(source_str,reg) && myTar.dataVector[i].key==1){
            string tmp = "";
            for(int i=reg_path.size();i<source_str.size();i++)
                tmp+=source_str[i];
            filler(buffer, (tmp.c_str()) , NULL, 0 );
        }
    }
    
    return  0 ;
}


int my_getattr(const char *path, struct stat *st) {
    int res = 0;
    int flag =0;

    for(int i=0; i<myTar.dataVector.size();i++){
        string source_str = myTar.dataVector[i].name;
        if(source_str[source_str.size()-1] == '/')
            source_str.erase(source_str.end()-1);
        if(strcmp(source_str.c_str(),path+1)==0){
            flag = i+1;
        }
    }

    memset(st, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        st->st_mode = S_IFDIR | 0444;
        st->st_nlink = 0;
        st->st_gid = getgid();
        st->st_uid = getuid();
        time_t rawtime;
        struct tm * timeinfo;
        time ( &rawtime );
        localtime ( &rawtime );
        st->st_mtime = rawtime;
    }
    else if(flag){
        flag -= 1;

        if(myTar.dataVector[flag].linkflag != 0)
            st->st_mode = S_IFDIR | myTar.dataVector[flag].mode;
        else
            st->st_mode = S_IFREG | myTar.dataVector[flag].mode;
        st->st_nlink = 0;
        st->st_mtime = myTar.dataVector[flag].mtime;
        st->st_size = myTar.dataVector[flag].size;
    }
    else{
        res = -ENOENT;
    }
    return res;
}


int my_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) { 
    size_t len;
    (void) fi;
    string now_path=(path+1);
    int total_size = 0,block = 0;

    for(int i=0;i<myTar.dataVector.size();i++){
        if(myTar.dataVector[i].name != now_path || myTar.dataVector[i].key==0){
            total_size += ceil((double)myTar.dataVector[i].size/
                            (double)myTar.usBlockSize)*myTar.usBlockSize;
        }
        else{
            block = i+1;
            break;
        }
    }

    if(myTar.dataVector[block-1].linkflag != 0)
        return -ENOENT;

    int jump_size = block*512+total_size;
    int file_size = myTar.dataVector[block-1].size;
    ifstream inputStream;
    inputStream.open("test.tar", ifstream::in | ifstream::binary);
    inputStream.seekg(jump_size, ios::cur);
    char tmp[file_size+1];
    inputStream.read((char*)&tmp,file_size);
    len = file_size;
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buffer, tmp + offset, size);
    } else
        size = 0;

    inputStream.close();
    return size;
}


static struct fuse_operations op{};

int main(int argc, char *argv[])
{
    myTar.startRead();
    myTar.clearRepeat();
    memset(&op, 0, sizeof(op));
    op.getattr = my_getattr;
    op.read = my_read;
    op.readdir = my_readdir;
    return fuse_main(argc, argv, &op, NULL);
}