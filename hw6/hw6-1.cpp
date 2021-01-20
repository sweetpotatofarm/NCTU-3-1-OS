/*
Student No.: 0716026 
Student Name: 蕭楚澔
Email: bobhsiao.cs07@nctu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not supposed to be 
posted to a public server, such as a public GitHub repository or a public 
web page.
*/

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <math.h>
#include <regex>

#define FUSE_USE_VERSION 30
#define _FILE_OFFSET_BITS  64

#include <fuse.h>

using namespace std;

struct header_old_tar{
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

struct header_new_tar{
    bool key;

    string name;
    int mode;
    int uid;
    int gid;
    int size;
    int mtime;
    int checksum;
    int linkflag;
    char linkname[100];
    char pad[255];
};

class TAR{
public:
    TAR(const char *file){
        this->input.open(file, ifstream::in | ifstream::binary);
        this->blockSize = 512;
    }
    void ReadFile();
    void DeleteOldVersion();
    int OctToDec(const char*, int);

public:
    ifstream input;
    vector<header_old_tar> oldTarList;
    vector<header_new_tar> newTarList;
    int blockSize;
};

// struct fuse_operations {
//     int (*readdir)(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *); 
//     int (*getattr)(const char *, struct stat *);
//     int (*read)(const char *, char *, size_t, off_t, struct fuse_file_info *);
// };

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

TAR tarFile("test.tar");

void TAR::DeleteOldVersion(){
    map<string, pair<int, int>> oldVersionMap;
    map<string, pair<int, int>>::iterator iter;
    for(int i=0; i<oldTarList.size()-1; i++){
        string tarName = oldTarList[i].name;
        int tarTime = OctToDec(oldTarList[i].mtime, sizeof(oldTarList[i].mtime));
        iter = oldVersionMap.find(tarName);
        if(iter == oldVersionMap.end()){
            //not find
            oldVersionMap[oldTarList[i].name].first = tarTime;
            oldVersionMap[oldTarList[i].name].second = i;
        }
        else if(oldVersionMap[oldTarList[i].name].first < tarTime){
            oldVersionMap[oldTarList[i].name].first = tarTime;
            oldVersionMap[oldTarList[i].name].second = i;
        }
    }

    vector<int> v;
    for(iter = oldVersionMap.begin(); iter != oldVersionMap.end(); iter++){
        v.push_back(iter->second.second);
    }
    sort(v.begin(), v.end());

    for(int i=0, j=0; i<oldTarList.size()-1; i++){
        header_old_tar oldTar = oldTarList[i];
        header_new_tar newTar;
        newTar.name = oldTar.name;
        newTar.mode = Oct2Dec(atoi(oldTar.mode));
        newTar.uid = Oct2Dec(atoi(oldTar.uid));
        newTar.gid = Oct2Dec(atoi(oldTar.gid));
        newTar.size = OctToDec(oldTar.size, sizeof(oldTar.size));
        oldTar.mtime[11] = '\0';
        newTar.mtime = (time_t)(OctToDec(oldTar.mtime, sizeof(oldTar.size)));
        newTar.checksum = OctToDec(oldTar.checksum, sizeof(oldTar.checksum));
        newTar.linkflag = atoi(oldTar.linkflag);
        strcpy(newTar.linkname, oldTar.linkname);
        strcpy(newTar.pad, oldTar.pad);

        newTarList.push_back(newTar);
        if(v[j] == i){
            newTarList[i].key = 1;
            j++;
        }
        else{
            newTarList[i].key = 0;
        }
    }
}

void TAR::ReadFile(){
    input.seekg(0, ios::beg);

    while(input){

        struct header_old_tar buffer;

        if(input.read((char*)&buffer, blockSize)){
            oldTarList.push_back(buffer);

            int fileSize = OctToDec(buffer.size, sizeof(buffer.size));

            int jumpBlock = ceil((double)fileSize / (double)blockSize);
            
            input.seekg(jumpBlock * blockSize, ios::cur);
        }

    }
}

int TAR::OctToDec(const char *s, int len){
    int ans = 0;
    int n = 0;
    for(int i=len-2; i>=0; i--){
        int temp = s[i] - '0';
        ans += temp * pow(8,n);
        ++n;
    }

    return ans;
}

int my_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    //regular expression
    string path_reg = (path+1);
    if(path_reg.size() > 1){
        path_reg += '/';
    }
    regex reg(path_reg + "([a-zA-z0-9._]*)");
    
    filler(buffer, "." , NULL, 0 );
    filler(buffer, ".." , NULL, 0 );

    for(int i=0; i<tarFile.newTarList.size();i++){
        header_old_tar oldTar = tarFile.oldTarList[i];
        string oldTarName = tarFile.newTarList[i].name;
        if(oldTarName[oldTarName.size()-1] == '/'){
            oldTarName.erase(oldTarName.end()-1);
        }
        string tmp = "";
        if(regex_match(oldTarName, reg) == true && tarFile.newTarList[i].key == 1){
            for(int i=path_reg.size(); i<oldTarName.size(); i++){
                tmp+=oldTarName[i];
            }
            filler(buffer, tmp.c_str(), NULL, 0);
        }
    }
    
    return  0;
}

int my_getattr(const char *path, struct stat *st) { 
    int ans = 0;
    int p = 0;

    for(int i=0; i<tarFile.newTarList.size(); i++){
        header_old_tar oldTar = tarFile.oldTarList[i];
        string newTarName = tarFile.newTarList[i].name;
        if(newTarName[newTarName.size()-1] == '/'){
            newTarName.erase(newTarName.end()-1);
        }
        if(strcmp(path+1, newTarName.c_str()) == 0){
            p = i+1;
        }
    }

    memset(st, 0, sizeof(struct stat));

    if(strcmp(path, "/") == 0){
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
    else if(p == 0){
        ans = -ENOENT;
    }
    else{
        if(tarFile.newTarList[p-1].linkflag != 0){
            st->st_mode = S_IFDIR | tarFile.newTarList[p-1].mode;
        }
        else{
            st->st_mode = S_IFREG | tarFile.newTarList[p-1].mode;
        }
        st->st_nlink = 0;
        st->st_mtime = tarFile.newTarList[p-1].mtime;
        st->st_size = tarFile.newTarList[p-1].size;
        st->st_gid = tarFile.newTarList[p-1].gid;
        st->st_uid = tarFile.newTarList[p-1].uid;
    }
    
    return ans;
}

int my_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
    int block = 0;
    int tempSize = 0;
    size_t len;
    string new_p = path+1;

    for(int i=0;i<tarFile.newTarList.size();i++){
        header_old_tar oldTar = tarFile.oldTarList[i];
        header_new_tar newTar = tarFile.newTarList[i];
        if(newTar.name != new_p || newTar.key == 0){
            tempSize += ceil((double)newTar.size/(double)tarFile.blockSize)*tarFile.blockSize;
        }
        else{
            block = i+1;
            break;
        }
    }

    if(tarFile.newTarList[block-1].linkflag != 0){
        return -ENOENT;
    }

    int jump_size = block*512 + tempSize;
    int file_size = tarFile.newTarList[block-1].size;
    ifstream input;
    input.open("test.tar", ifstream::in | ifstream::binary);
    input.seekg(jump_size, ios::cur);
    char tmp[file_size+1];
    input.read((char*)&tmp, file_size);
    len = file_size;
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buffer, tmp + offset, size);
    } 
    else{
        size = 0;
    }

    input.close();
    return size;
 } 

static struct fuse_operations op{};

int main(int argc, char *argv[])
{
    tarFile.ReadFile();
    tarFile.DeleteOldVersion();
    memset(&op, 0, sizeof(op)); 
    op.getattr = my_getattr; 
    op.readdir = my_readdir; 
    op.read = my_read;
    return fuse_main(argc, argv, &op, NULL); 
}