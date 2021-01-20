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

int Oct2Dec(int oct){
    int ans = 0;
    int base = 1;
    while(oct > 0){
        int temp = oct%10;
        temp *= base;
        ans  += temp;
        oct /= 10 ;
        base*=8;
    }
    return ans;
}

TAR tarFile("test.tar");

void TAR::DeleteOldVersion(){
    map<string, pair<int, int>> oldVersionMap;
    map<string, pair<int, int>>::iterator iter;
    for(int i=0; i<oldTarList.size()-1; i++){
        string tarName = oldTarList[i].name;
        iter = oldVersionMap.find(tarName);
        if(iter == oldVersionMap.end()){
            //not find
            oldVersionMap[oldTarList[i].name].first = OctToDec(oldTarList[i].mtime, sizeof(oldTarList[i].mtime));
            oldVersionMap[oldTarList[i].name].second = i;
        }
        else if(oldVersionMap[oldTarList[i].name].first < OctToDec(oldTarList[i].mtime, sizeof(oldTarList[i].mtime))){
            oldVersionMap[oldTarList[i].name].first = OctToDec(oldTarList[i].mtime, sizeof(oldTarList[i].mtime));
            oldVersionMap[oldTarList[i].name].second = i;
        }
    }

    vector<int> v;
    for(iter = oldVersionMap.begin(); iter != oldVersionMap.end(); iter++){
        v.push_back(iter->second.second);
    }

    sort(v.begin(), v.end());

    int j = 0;
    for(int i=0; i<oldTarList.size()-1; i++){
        header_old_tar oldTar = oldTarList[i];
        oldTar.mtime[11] = '\0';
        header_new_tar newTar;

        newTar.name = oldTar.name;
        newTar.mode = Oct2Dec(atoi(oldTar.mode));
        newTar.uid = Oct2Dec(atoi(oldTar.uid));
        newTar.gid = Oct2Dec(atoi(oldTar.gid));
        newTar.size = OctToDec(oldTar.size, sizeof(oldTar.size));
        newTar.mtime = (time_t)(OctToDec(oldTar.mtime, sizeof(oldTar.size)));
        newTar.checksum = OctToDec(oldTar.checksum, sizeof(oldTar.checksum));
        newTar.linkflag = atoi(oldTar.linkflag);
        strcpy(newTar.linkname, oldTar.linkname);
        strcpy(newTar.pad, oldTar.pad);

        newTarList.push_back(newTar);
        if(v[j] != i){
            newTarList[i].key = 0;
        }
        else{
            j++;
            newTarList[i].key = 1;
        }
    }
}

void TAR::ReadFile(){
    input.seekg(0, ios::beg);

    while(input){
        struct header_old_tar buffer;

        if(input.read((char*)&buffer, blockSize)){
            int fileSize = OctToDec(buffer.size, sizeof(buffer.size));
            int jumpBlock = ceil((double)fileSize / (double)blockSize);
            input.seekg(jumpBlock * blockSize, ios::cur);
            oldTarList.push_back(buffer);
        }
    }
}

int TAR::OctToDec(const char *s, int len){
    int ans = 0;
    int n = 0;
    for(int i=len-2; i>=0; i--){
        int temp = s[i] - '0';
        temp *= pow(8,n);
        ans += temp;
        ++n;
    }

    return ans;
}

int my_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    //regular expression
    filler(buffer, "." , NULL, 0);
    filler(buffer, ".." , NULL, 0);
    
    string path_reg = (path+1);
    if(path_reg.size() > 1){
        path_reg += '/';
    }
    regex reg(path_reg + "([a-zA-z0-9._]*)");
    
    for(int i=0; i<tarFile.newTarList.size(); i++){
        if(tarFile.newTarList[i].name[tarFile.newTarList[i].name.size()-1] == '/'){
            tarFile.newTarList[i].name.erase(tarFile.newTarList[i].name.end()-1);
        }
        string s = "";
        if(regex_match(tarFile.newTarList[i].name, reg) == true && tarFile.newTarList[i].key == 1){
            for(int j=path_reg.size(); j<tarFile.newTarList[i].name.size(); j++){
                s += tarFile.newTarList[i].name[j];
            }
            filler(buffer, s.c_str(), NULL, 0);
        }
    }
    return  0;
}

int my_getattr(const char *path, struct stat *st) { 
    int ans = 0;
    int p = -1;

    for(int i=0; i<tarFile.newTarList.size(); i++){
        if(tarFile.newTarList[i].name[tarFile.newTarList[i].name.size()-1] == '/'){
            tarFile.newTarList[i].name.erase(tarFile.newTarList[i].name.end()-1);
        }
        const char *temp = path+1;
        if(strcmp(temp, tarFile.newTarList[i].name.c_str()) == 0){
            p = i;
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
    else if(p < 0){
        ans = -ENOENT;
    }
    else{
        if(tarFile.newTarList[p].linkflag != 0){
            st->st_mode = S_IFDIR | tarFile.newTarList[p].mode;
        }
        else{
            st->st_mode = S_IFREG | tarFile.newTarList[p].mode;
        }
        st->st_nlink = 0;
        st->st_mtime = tarFile.newTarList[p].mtime;
        st->st_size = tarFile.newTarList[p].size;
        st->st_gid = tarFile.newTarList[p].gid;
        st->st_uid = tarFile.newTarList[p].uid;
    }
    return ans;
}

int my_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
    int block = 0;
    int tempSize = 0;
    size_t len;
    string new_p = path+1;
    int ans;

    for(int i=0; i<tarFile.newTarList.size(); i++){
        header_old_tar oldTar = tarFile.oldTarList[i];
        header_new_tar newTar = tarFile.newTarList[i];
        if(newTar.name != new_p || newTar.key == 0){
            double tmp = ceil((double)newTar.size/(double)tarFile.blockSize);
            tmp *= tarFile.blockSize;
            tempSize += tmp;
        }
        else{
            block = i;
            break;
        }
    }

    if(tarFile.newTarList[block].linkflag == 0){
        //read file
        ifstream input;
        input.open("test.tar", ifstream::in | ifstream::binary);
        int toJump = (block+1)*512 + tempSize;
        input.seekg(toJump, ios::cur);

        int fileSize = tarFile.newTarList[block].size;
        char tmp[fileSize+1];
        input.read((char*)&tmp, fileSize);
        len = fileSize;
        if (offset > len || offset == len){
            size = 0;
        }
        else if(offset + size > len){
            size = len - offset;
            memcpy(buffer, tmp+offset, size);
        }
        else{
            memcpy(buffer, tmp+offset, size);
        }

        input.close();
        ans = size;
    }
    else{
        return -ENOENT;
    }

    return ans;
 } 

static struct fuse_operations op;

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