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
#include <fstream>
#include <vector>
#include <unordered_map>
#include <list>
#include <sys/time.h>

using namespace std;

//global variable
int hit_count;
int miss_count;
int frameSize;
vector<int> pageData;
int currentSize;

int main(int argc, char *argv[])
{
    int base = 64;
    struct timeval start, end;

    //LFU
    cout<<"LFU policy:"<<endl;
    cout<<"Frame\tHit\t\tMiss\t\tPage fault ratio"<<endl;

    gettimeofday(&start, 0);

    for(int i=1; i<16; i*=2){
        //initialize
        hit_count = 0;
        miss_count = 0;
        frameSize = base*i;
        pageData.clear();
        currentSize = 0;

        //read file
        fstream file;
        char line[128];
        file.open(argv[1], ios::in);
        if(!file){
            cout<<"error"<<endl;
        }
        while(file.getline(line, sizeof(line), '\n')){
            pageData.push_back(atoi(line));
        }
        file.close();

        //unordered map as frame
        unordered_map<int, list<pair<int, pair<int, int>>>::iterator> frame_LFU;
        frame_LFU.clear();

        //vector to remember frequency
        //first one is frequency, second one is sequence number and page number
        list<pair<int, pair<int, int>>> ref_count;
        ref_count.clear();

        //simulate
        for(int j=0; j<pageData.size(); j++){
            int page = pageData[j];
            unordered_map<int, list<pair<int, pair<int, int>>>::iterator>::const_iterator got = frame_LFU.find(page);
            if(got == frame_LFU.end()){
                //not find
                miss_count++;
                if(currentSize == frameSize){
                    //frame is full
                    //find the least frequency used page
                    int victim_page = (*ref_count.begin()).second.second;
                    frame_LFU.erase(victim_page);
                    ref_count.erase(ref_count.begin());
                }
                else{
                    //frame is not full yet
                    currentSize++;
                }
                pair<int, pair<int, int>> temp;
                temp.first = 1;
                temp.second.first = j;
                temp.second.second = page;
                ref_count.insert(ref_count.begin(), temp);
                frame_LFU[page] = ref_count.begin();
                list<pair<int, pair<int, int>>>::iterator current = ref_count.begin();
                list<pair<int, pair<int, int>>>::iterator next = std::next(current, 1);
                    
                while((*next).first == 1 && (*current).second.first > (*next).second.first && next != ref_count.end()){
                    //swap
                    ref_count.erase(frame_LFU[(*current).second.second]);
                    ref_count.insert(std::next(next, 1), temp);
                    frame_LFU[page] = std::next(next, 1);
                    current = std::next(next, 1);
                    next = std::next(current, 1);
                }
            }
            else{
                //found
                hit_count++;
                //move this page to the correct place
                //frequency plus 1
                (*got->second).first += 1;
                (*got->second).second.first = j;
                pair<int, pair<int, int>> temp;
                temp.first = (*got->second).first;
                temp.second.first = (*got->second).second.first;
                temp.second.second = (*got->second).second.second;
                list<pair<int, pair<int, int>>>::iterator current = got->second;
                list<pair<int, pair<int, int>>>::iterator next = std::next(current, 1);
                while((*next).first < (*current).first && next != ref_count.end()){
                    //The least frequently used page
                    //swap
                    ref_count.erase(frame_LFU[(*current).second.second]);
                    ref_count.insert(std::next(next, 1), temp);
                    frame_LFU[page] = std::next(next, 1);
                    current = std::next(next, 1);
                    next = std::next(current, 1);
                }
                    
                while((*next).first == (*current).first && (*next).second.first < (*current).second.first && next != ref_count.end()){
                    //If two pages have the same access count, the page having a smaller reference sequence number is replaced
                    //swap
                    ref_count.erase(frame_LFU[(*current).second.second]);
                    ref_count.insert(std::next(next, 1), temp);
                    frame_LFU[page] = std::next(next, 1);
                    current = std::next(next, 1);
                    next = std::next(current, 1);
                }
            }
        }

        cout<<frameSize<<"\t"<<hit_count<<"\t\t"<<miss_count<<"\t\t";
        double miss_ratio = (double)miss_count/(double)(hit_count+miss_count);
        printf("%.10f\n", miss_ratio);
    }
    gettimeofday(&end, 0);
    int sec = end.tv_sec - start.tv_sec;
    int usec = end.tv_usec - start.tv_usec;
    float timeUsed = sec*1000+(usec/1000.0);
    timeUsed /= 1000;
    printf("Total elapsed time %.4f sec\n\n", timeUsed);

    //LRU
    std::cout<<"LRU policy:"<<endl;
    cout<<"Frame\tHit\t\tMiss\t\tPage fault ratio"<<endl;

    gettimeofday(&start, 0);

    for(int i=1; i<16; i*=2){
        //initialize
        hit_count = 0;
        miss_count = 0;
        frameSize = base*i;
        pageData.clear();
        currentSize = 0;

        //read file
        fstream file;
        char line[128];
        file.open(argv[1], ios::in);
        if(!file){
            std::cout<<"error"<<endl;
        }
        while(file.getline(line, sizeof(line), '\n')){
            pageData.push_back(atoi(line));
        }
        file.close();

        //unordered map as frame
        unordered_map<int, list<int>::iterator> frame_LRU;
        frame_LRU.clear();

        //list to remember MRU and LRU
        list<int> recently_used;
        recently_used.clear();

        //simulate
        for(int j=0; j<pageData.size(); j++){
            int page = pageData[j];
            unordered_map<int, list<int>::iterator>::const_iterator got = frame_LRU.find(page);
            if(got == frame_LRU.end()){
                //not find
                miss_count++;
                if(currentSize == frameSize){
                    //frame is full
                    //find the least recently used page
                    int victim_page = *recently_used.begin();
                    frame_LRU.erase(victim_page);
                    recently_used.erase(recently_used.begin());
                }
                else{
                    //frame is not full yet
                    currentSize++;
                }
                recently_used.push_back(page);
                frame_LRU[page] = std::prev(recently_used.end(), 1);
            }
            else{
                //found
                hit_count++;
                //move this page to the top(MRU)
                recently_used.erase(got->second);
                recently_used.push_back(got->first);
                //change mapping
                frame_LRU[page] = std::prev(recently_used.end(), 1);
            }
        }

        std::cout<<frameSize<<"\t"<<hit_count<<"\t\t"<<miss_count<<"\t\t";
        double miss_ratio = (double)miss_count/(double)(hit_count+miss_count);
        printf("%.10f\n", miss_ratio);
    }
    gettimeofday(&end, 0);
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    timeUsed = sec*1000+(usec/1000.0);
    timeUsed /= 1000;
    printf("Total elapsed time %.4f sec\n\n", timeUsed);

    return 0;
}