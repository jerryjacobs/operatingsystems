/*
 Operating Systems - Project 2
 Jerald Jacobs - jacobj7
 Cavell Teng - tengc
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <set>
#include <iomanip>
#include <algorithm>
using namespace std;

#define FRAMES_PER_LINE 32
#define NUMBER_FRAMES 256

//process class - Changed
class Process{
public:
    Process(){};
    Process(string id_, int memory_, int arrival_time_, int run_time_){
        id = id_;
        memory = memory_;
        arrival_time = arrival_time_;
        run_time = run_time_;
        end_time = arrival_time + run_time;
        
    };
    
    string get_id() const { return id; }
    int get_memory() const { return memory; }
    int get_arrival_time() const { return arrival_time; }
    int get_run_time() const { return run_time; }
    
    vector<int> page_table;     //Index is page number and stored value is the frame number
    int end_time;               //Time process ends
    
private:
    string id;                  //ID of the process
    int memory;                 //Number of frames needed to store the process
    int arrival_time;           //Time process arrives
    int run_time;               //Time process is running
};

//compare processes by arrival time
bool order_arrival(Process* a, Process* b){
    if(a->get_arrival_time() != b->get_arrival_time()){
        return a->get_arrival_time() < b->get_arrival_time();
    }
    else{
        return a->get_id() < b->get_id();
    }
}

//Compare processes by end time
bool order_end(Process* a, Process* b){
    if(a->end_time != b->end_time){
        return a->end_time < b->end_time;
    }
    else{
        return a->get_id() < b->get_id();
    }
}

//Compare processes by index (Alphabetical Order)
bool order_alph(Process* a, Process* b){
    return a->get_id() < b->get_id();
}

//queue class to store process pointers
class Queue{
public:
    Queue(){};
    int size() { return q.size(); } //returns size of queue
    Process* head() { return q[0]; } //return head of queue
    void add(Process* p) { q.push_back(p); } //adds element to queue then sorts via arrival_time
    void add_end(Process* p){               //Adds element to queue based on end time
        q.push_back(p);
        sort(q.begin(), q.end(), order_end);    //Sorts queue by end time
    }
    void qsort(){ sort(q.begin(), q.end(), order_arrival); }
    Process* pop() { //returns the first element of queue and removes it
        Process* p = q[0];
        for (unsigned int i = 0; i < q.size()-1; i++){
            q[i] = q[i+1];
        }
        q.pop_back();
        return p;
    }
    
private:
    vector<Process*> q;
};

void print_frames(vector<string> &frames){      //Prints the frames
    string separator(FRAMES_PER_LINE,'=');      //Separator string formed
    cout << separator << endl;
    int lines = NUMBER_FRAMES / FRAMES_PER_LINE;
    int rem_lines = NUMBER_FRAMES % FRAMES_PER_LINE;
    for(int rows = 0; rows < lines; rows++){
        string line = "";
        for(int f = 0; f < FRAMES_PER_LINE; f++){   //Creates string for frame row
            line += frames[(f + rows*FRAMES_PER_LINE)];
        }
        cout << line << endl;
    }
    if(rem_lines != 0){                         //Prints any remaining frames
        string line = "";
        for(int f = lines * FRAMES_PER_LINE; f < NUMBER_FRAMES; f++){
            line += frames[f];
        }
        cout << line << endl;
        
    }
    cout << separator << endl;
}

void print_table(vector<Process*> &running_frames){ //Prints the page table
    cout << "PAGE TABLE [page,frame]:" << endl;
    for(unsigned int r_proc = 0; r_proc < running_frames.size(); r_proc++){    //Loops through the processes
        string line = "";
        for(int pages = 0; pages < running_frames[r_proc]->get_memory(); pages++){  //Loops through all the pages
            if(pages == 0){                         //Specific line for first page
                cout << running_frames[r_proc]->get_id() << ":";
            }
            else if(pages % 10 == 0){               //Prints every 10 pages and reset string
                cout << endl;
                cout << "[" << pages << "," << running_frames[r_proc]->page_table[pages] << "]";
                continue;
            }
            cout << " [" << pages << "," << running_frames[r_proc]->page_table[pages] << "]";
        }
        if(running_frames[r_proc]->get_memory() % 10 != 0){         //Prints any remaining pages
            cout << endl;
        }
    }
}

int defrag(vector<string> &frames, vector<string> &moved){ //Defragmentation of frames vector
    
    int time = 0; //total time it took to defrag
    set<string> s; //processes that were moved
    
    //defrag frames vector
    unsigned int i = 0;
    while (frames[i] != "."){
        i++;
    }
    for (; i < frames.size(); i++){ //loop through frames
        if (frames[i] != "."){ //shift all pages left
            string p = frames[i];
            int j = i;
            while (j > 0 && frames[j-1] == "."){ //check if previous frame is free
                j--;
            }
            frames[i] = ".";
            frames[j] = p;
            s.insert(p);
            time++;
        }
    }
    
    //update moved vector
    vector<string> temp(s.begin(), s.end());
    for (unsigned int i = 0; i < temp.size(); i++){
        moved.push_back(temp[i]);
    }
    
    return time;
}

void next_fit(vector<Process> &procs){ //Contiguous Next Fit Simulation
    
    Queue q;                            //Queue of processes based on arrival time
    Queue e;                            //Queue of processes based on end time
    vector<string> frames;              //Vector to keep track of the frames
    
    for(unsigned int f = 0; f < NUMBER_FRAMES; f++){    //Initializes frames with '.'
        frames.push_back(".");
    }
    for(unsigned int x = 0; x < procs.size(); x++){     //Store all processes in arrival queue
        q.add(&procs[x]);
    }
    q.qsort();                          //Sorts arrival queue
    
    int free_space = NUMBER_FRAMES;     //Keeps track of amount of free space
    int t = 0;                          //Keeps track of current simulation time
    int dt = 0;                         //Accumulated defragmentation time
    unsigned int term = 0;              //Keeps track of number of terminated processes
    int next_arrival = q.head()->get_arrival_time();    //Keeps track of the next arrival time
    int next_end = q.head()->end_time;  //Keeps track of the next end time
    int index = 0;                      //Page index
    
    cout << "time 0ms: Simulator started (Contiguous -- Next-Fit)" << endl;
    
    while (term < procs.size()){ //loops until all processes terminate
        
        if (t == next_end){ //check if process is done running
            while(e.size() > 0){ //loops while end time queue is not empty
                if(e.head()->end_time == t){ //checks if head of queue is done
                    Process* p = e.pop();
                    for(int i = 0; i < NUMBER_FRAMES; i++){ //remove frames
                        if (frames[i] == p->get_id()){
                            frames[i] = ".";
                        }
                    }
                    free_space += p->get_memory(); //update free space
                    term++;
                    cout << "time " << t+dt << "ms: Process " << p->get_id() << " removed:" << endl;
                    print_frames(frames);
                }
                else{                   //No more finished processes
                    next_end = e.head()->end_time;
                    break;
                }
            }
        }
        if (t == next_arrival){ //checks if process arrives
            while (q.size() > 0){
                if (q.head()->get_arrival_time() == t){ //checks if process arrives now
                    Process* p = q.pop();
                    cout << "time " << t+dt << "ms: Process " << p->get_id() << " arrived (requires " << p->get_memory() << " frames)" << endl;
                    if (p->get_memory() <= free_space){ //checks for enough number of pages
                        
                        int found = 0; //boolean to check if we need to defrag
                        int start = 0; //starting index of partition
                        
                        //check if partition is large enough
                        for (int i = index; i < NUMBER_FRAMES; i++){
                            if (found){ break; } //found partition
                            if (frames[i] == "."){
                                start = i;
                                int p_size = 0; //partition size
                                while (i < NUMBER_FRAMES && frames[i] == "."){
                                    p_size++;
                                    i++;
                                    if (p_size == p->get_memory()){ //found large enough partition
                                        found = 1;
                                        break;
                                    }
                                }
                            }
                        }
                        //loop back to top and continue scanning
                        for (int i = 0; i < index; i++){
                            if (found){ break; } //found partition
                            if (frames[i] == "."){
                                start = i;
                                int p_size = 0; //partition size
                                while (frames[i] == "." && i < NUMBER_FRAMES){
                                    p_size++;
                                    i++;
                                    if (p_size == p->get_memory()){ //found large enough partition
                                        found = 1;
                                        break;
                                    }
                                }
                            }
                        }
                        
                        if (!found){ //defragmentation occurs
                            cout << "time " << t+dt << "ms: Cannot place process " << p->get_id() << " -- starting defragmentation" << endl;
                            vector<string> moved;
                            int num_moved = defrag(frames, moved);
                            dt += num_moved; //update defrag time
                            index = NUMBER_FRAMES - free_space; //update index
                            start = index;
                            
                            //output formatting
                            string moved_proc = "";
                            for(unsigned int i = 0; i < moved.size(); i++){
                                moved_proc += " " + moved[i] + ",";
                            }
                            moved_proc.erase(moved_proc.length() - 1);
                            cout << "time " << t+dt << "ms: Defragmentation complete (moved " << num_moved << " frames:" << moved_proc << ")" << endl;
                            print_frames(frames);
                        }
                        for (int i = start; i < start + p->get_memory(); i++){
                            frames[i] = p->get_id();
                        }
                        e.add_end(p); //add process to end time queue
                        free_space -= p->get_memory(); //update free space
                        index = start + p->get_memory(); //update index
                        cout << "time " << t+dt << "ms: Placed process " << p->get_id() << ":" << endl;
                    }
                    else{ //skip process due to lack of space
                        cout << "time " << t+dt << "ms: Cannot place process " << p->get_id() << " -- skipped!" << endl;
                        term++;
                    }
                    print_frames(frames);
                }
                else{ break; }
            }
            if(term < procs.size()){ //if there are more processes left to arrive, set next arrival time
                next_arrival = q.head()->get_arrival_time();
            }
            if(e.size() > 0){ //if there are processes running, set next end time
                next_end = e.head()->end_time;
            }
        }
        t++;
    }
    
    //end of simulation
    t--;
    cout << "time " << t+dt << "ms: Simulator ended (Contiguous -- Next-Fit)" << endl;
}

void first_fit(vector<Process> &procs){ //Contiguous First Fit Simulation
    
    Queue q;                            //Queue of processes based on arrival time
    Queue e;                            //Queue of processes based on end time
    vector<string> frames;              //Vector to keep track of the frames
    
    for(unsigned int f = 0; f < NUMBER_FRAMES; f++){    //Initializes frames with '.'
        frames.push_back(".");
    }
    for(unsigned int x = 0; x < procs.size(); x++){     //Store all processes in arrival queue
        q.add(&procs[x]);
    }
    q.qsort();                          //Sorts arrival queue
    
    int free_space = NUMBER_FRAMES;     //Keeps track of amount of free space
    int t = 0;                          //Keeps track of current simulation time
    int dt = 0;                         //Accumulated defragmentation time
    unsigned int term = 0;              //Keeps track of number of terminated processes
    int next_arrival = q.head()->get_arrival_time();    //Keeps track of the next arrival time
    int next_end = q.head()->end_time;  //Keeps track of the next end time
    int index = 0;                      //Page index
    
    cout << "time 0ms: Simulator started (Contiguous -- First-Fit)" << endl;
    
    while (term < procs.size()){ //loops until all processes terminate
        
        if (t == next_end){ //check if process is done running
            while(e.size() > 0){ //loops while end time queue is not empty
                if(e.head()->end_time == t){ //checks if head of queue is done
                    Process* p = e.pop();
                    for(int i = 0; i < NUMBER_FRAMES; i++){ //remove frames
                        if (frames[i] == p->get_id()){
                            frames[i] = ".";
                        }
                    }
                    free_space += p->get_memory(); //update free space
                    term++;
                    cout << "time " << t+dt << "ms: Process " << p->get_id() << " removed:" << endl;
                    print_frames(frames);
                }
                else{                   //No more finished processes
                    next_end = e.head()->end_time;
                    break;
                }
            }
        }
        if (t == next_arrival){ //checks if process arrives
            while (q.size() > 0){
                if (q.head()->get_arrival_time() == t){ //checks if process arrives now
                    Process* p = q.pop();
                    cout << "time " << t+dt << "ms: Process " << p->get_id() << " arrived (requires " << p->get_memory() << " frames)" << endl;
                    if (p->get_memory() <= free_space){ //checks for enough number of pages
                        
                        int found = 0; //boolean to check if we need to defrag
                        int start = 0; //starting index of partition
                        
                        //check if partition is large enough
                        for (int i = 0; i < NUMBER_FRAMES; i++){
                            if (found){ break; } //found partition
                            if (frames[i] == "."){
                                start = i;
                                int p_size = 0; //partition size
                                while (i < NUMBER_FRAMES && frames[i] == "."){
                                    p_size++;
                                    i++;
                                    if (p_size == p->get_memory()){ //found large enough partition
                                        found = 1;
                                        break;
                                    }
                                }
                            }
                        }
                        
                        if (!found){ //defragmentation occurs
                            cout << "time " << t+dt << "ms: Cannot place process " << p->get_id() << " -- starting defragmentation" << endl;
                            vector<string> moved;
                            int num_moved = defrag(frames, moved);
                            dt += num_moved; //update defrag time
                            index = NUMBER_FRAMES - free_space; //update index
                            start = index;
                            
                            //output formatting
                            string moved_proc = "";
                            for(unsigned int i = 0; i < moved.size(); i++){
                                moved_proc += " " + moved[i] + ",";
                            }
                            moved_proc.erase(moved_proc.length() - 1);
                            cout << "time " << t+dt << "ms: Defragmentation complete (moved " << num_moved << " frames:" << moved_proc << ")" << endl;
                            print_frames(frames);
                        }
                        for (int i = start; i < start + p->get_memory(); i++){
                            frames[i] = p->get_id();
                        }
                        e.add_end(p); //add process to end time queue
                        free_space -= p->get_memory(); //update free space
                        index = start + p->get_memory(); //update index
                        cout << "time " << t+dt << "ms: Placed process " << p->get_id() << ":" << endl;
                    }
                    else{ //skip process due to lack of space
                        cout << "time " << t+dt << "ms: Cannot place process " << p->get_id() << " -- skipped!" << endl;
                        term++;
                    }
                    print_frames(frames);
                }
                else{ break; }
            }
            if(term < procs.size()){ //if there are more processes left to arrive, set next arrival time
                next_arrival = q.head()->get_arrival_time();
            }
            if(e.size() > 0){ //if there are processes running, set next end time
                next_end = e.head()->end_time;
            }
        }
        t++;
    }
    
    //end of simulation
    t--;
    cout << "time " << t+dt << "ms: Simulator ended (Contiguous -- First-Fit)" << endl;
}

void best_fit(vector<Process> &procs){ //Contiguous Best Fit Simulation
    
    Queue q;                            //Queue of processes based on arrival time
    Queue e;                            //Queue of processes based on end time
    vector<string> frames;              //Vector to keep track of the frames
    
    for(unsigned int f = 0; f < NUMBER_FRAMES; f++){    //Initializes frames with '.'
        frames.push_back(".");
    }
    for(unsigned int x = 0; x < procs.size(); x++){     //Store all processes in arrival queue
        q.add(&procs[x]);
    }
    q.qsort();                          //Sorts arrival queue
    
    int free_space = NUMBER_FRAMES;     //Keeps track of amount of free space
    int t = 0;                          //Keeps track of current simulation time
    int dfrag_t = 0;                    //Keeps track of time spend defragging
    unsigned int term = 0;              //Keeps track of number of completed processes
    int next_arrival = q.head()->get_arrival_time();    //Keeps track of the next arrival time
    int next_end = q.head()->end_time;  //Keeps track of the next end time
    int pages = 0;                      //Page index
    
    cout << "time 0ms: Simulator started (Contiguous -- Best-Fit)" << endl;
    
    while(term < procs.size()){			//Keeps looping as long as there are still processes remaining
        
        if(t == next_end){       //Checks if a process is done running
            while(e.size() > 0){        //Loops while end time queue is not empty
                if(e.head()->end_time == t){     //Checks if head of queue is done
                    Process* end = e.pop();
                    free_space += end->get_memory();
                    for(pages = 0; pages < NUMBER_FRAMES; pages++){     //Removes pages from frames
                        if(frames[pages] == end->get_id()){
                            frames[pages] = ".";
                        }
                    }
                    cout << "time " << t + dfrag_t << "ms: Process " << end->get_id() << " removed:" << endl;
                    print_frames(frames);
                    term++;
                }
                else{                   //No more finished processes
                    next_end = e.head()->end_time;
                    break;
                }
            }
        }
        
        if(next_arrival == t){   //Checks if a process is arriving
            while(q.size() > 0){        //Loops while arrival queue is not empty
                if(q.head()->get_arrival_time() == t){   //Checks if head is arriving
                    Process* run = q.pop();
                    cout << "time " << t + dfrag_t << "ms: Process " << run->get_id() << " arrived (requires " << run->get_memory() << " frames)" << endl;
                    if(run->get_memory() <= free_space){    //Checks if there is enough space for the process
                        e.add_end(run);                 //Adds process to end queue
                        free_space -= run->get_memory();//Reduces the amount of free space by process size
                        pages = 0;
                        pair<int,int> smallest_space = make_pair(-1,99999); //Stores the index of open space and size
                        for(int f = 0; f < NUMBER_FRAMES; f++){
                            if(frames[f] == "."){
                                int space_size = 0;
                                int head_frame = f;
                                while(f < NUMBER_FRAMES && frames[f] == "."){
                                    space_size++;
                                    f++;
                                }
                                if(space_size >= run->get_memory() && space_size < smallest_space.second){
                                    smallest_space.first = head_frame;
                                    smallest_space.second = space_size;
                                }
                            }
                        }
                        if(smallest_space.first != -1){
                            for(pages = smallest_space.first; pages < (smallest_space.first + run->get_memory()); pages++){
                                frames[pages] = run->get_id();
                            }
                        }
                        else{
                            cout << "time " << t + dfrag_t << "ms: Cannot place process " << run->get_id() << " -- starting defragmentation" << endl;
                            vector<string> moved_processes;
                            int temp_dt = defrag(frames, moved_processes);
                            string moved_proc = "";
                            for(unsigned int mp = 0; mp < moved_processes.size(); mp++){
                                moved_proc += " " + moved_processes[mp] + ",";
                            }
                            moved_proc.erase(moved_proc.length() - 1);
                            dfrag_t += temp_dt;
                            cout << "time " << t + dfrag_t << "ms: Defragmentation complete (moved " << temp_dt << " frames:" << moved_proc << ")" <<endl;
                            print_frames(frames);
                            for(pages = (NUMBER_FRAMES - (free_space + run->get_memory())); pages < NUMBER_FRAMES; pages++){
                                frames[pages] = run->get_id();
                            }
                        }
                        cout << "time " << t + dfrag_t << "ms: Placed process " << run->get_id() << ":" << endl;
                    }
                    else{               //Skip process due to lack of space
                        cout << "time " << t + dfrag_t << "ms: Cannot place process " << run->get_id() << " -- skipped!" << endl;
                        term++;
                    }
                    print_frames(frames);
                }
                else{
                    break;
                }
            }
            if(term < procs.size()){       //If there are more processes left to arrive, set next arrival time
                next_arrival = q.head()->get_arrival_time();
            }
            if(e.size() > 0){               //If there are processes running, set next end time
                next_end = e.head()->end_time;
            }
        }
        
        t++;
    }
    t--;
    cout << "time " << t + dfrag_t << "ms: Simulator ended (Contiguous -- Best-Fit)" << endl;
}

void non_cont(vector<Process> &procs){  //Non-Contiguous Simulation
    
    Queue q;                            //Queue of processes based on arrival time
    Queue e;                            //Queue of processes based on end time
    vector<string> frames;              //Vector to keep track of the frames
    vector<Process*> running_frames;    //Vector to keep track of all running processes in alphabetical order
    
    for(unsigned int f = 0; f < NUMBER_FRAMES; f++){    //Initializes frames with '.'
        frames.push_back(".");
    }
    for(unsigned int x = 0; x < procs.size(); x++){     //Store all processes in arrival queue
        q.add(&procs[x]);
    }
    q.qsort();                          //Sorts arrival queue
    
    int free_space = NUMBER_FRAMES;     //Keeps track of amount of free space
    int t = 0;                          //Keeps track of current simulation time
    unsigned int term = 0;              //Keeps track of number of terminated processes
    int next_arrival = q.head()->get_arrival_time();    //Keeps track of the next arrival time
    int next_end = q.head()->end_time;  //Keeps track of the next end time
    int pages = 0;                      //Page index
    
    cout << "time 0ms: Simulator started (Non-contiguous)" << endl;
    
    while(term < procs.size()){			//Keeps looping as long as there are still processes remaining
        
        if(t == next_end){       //Checks if a process is done running
            while(e.size() > 0){        //Loops while end time queue is not empty
                if(e.head()->end_time == t){     //Checks if head of queue is done
                    Process* end = e.pop();
                    free_space += end->get_memory();
                    for(pages = 0; pages < end->get_memory(); pages++){     //Removes pages from frames
                        frames[end->page_table[pages]] = ".";
                    }
                    for(unsigned int r_proc = 0; r_proc < running_frames.size(); r_proc++){
                        if(running_frames[r_proc]->get_id() == end->get_id()){
                            running_frames.erase(running_frames.begin()+r_proc);
                            break;
                        }
                    }
                    cout << "time " << t << "ms: Process " << end->get_id() << " removed:" << endl;
                    print_frames(frames);
                    print_table(running_frames);
                    term++;
                }
                else{                   //No more finished processes
                    next_end = e.head()->end_time;
                    break;
                }
            }
        }
        
        if(next_arrival == t){   //Checks if a process is arriving
            while(q.size() > 0){        //Loops while arrival queue is not empty
                if(q.head()->get_arrival_time() == t){   //Checks if head is arriving
                    Process* run = q.pop();
                    cout << "time " << t << "ms: Process " << run->get_id() << " arrived (requires " << run->get_memory() << " frames)" << endl;
                    if(run->get_memory() <= free_space){    //Checks if there is enough space for the process
                        e.add_end(run);                 //Adds process to end queue
                        running_frames.push_back(run);
                        free_space -= run->get_memory();//Reduces the amount of free space by process size
                        pages = 0;
                        for(unsigned int f = 0; f < NUMBER_FRAMES; f++){
                            if(frames[f] == "."){
                                frames[f] = run->get_id();
                                run->page_table.push_back(f);
                                pages++;
                                if(pages == run->get_memory()){
                                    break;
                                }
                            }
                        }
                        cout << "time " << t << "ms: Placed process " << run->get_id() << ":" << endl;
                        sort(running_frames.begin(), running_frames.end(), order_alph);
                    }
                    else{               //Skip process due to lack of space
                        cout << "time " << t << "ms: Cannot place process " << run->get_id() << " -- skipped!" << endl;
                        term++;
                    }
                    print_frames(frames);
                    print_table(running_frames);
                }
                else{
                    break;
                }
            }
            if(term < procs.size()){       //If there are more processes left to arrive, set next arrival time
                next_arrival = q.head()->get_arrival_time();
            }
            if(e.size() > 0){               //If there are processes running, set next end time
                next_end = e.head()->end_time;
            }
        }
        
        t++;
    }
    t--;
    cout << "time " << t << "ms: Simulator ended (Non-contiguous)" << endl;
}

//main
int main(int argc, char* argv[]) {
    
    //checks for incorrect number of arguments
    if (argc != 2) {
        cerr << "ERROR: Invalid arguments" << endl;
        return 1;
    }
    
    //variables
    ifstream input(argv[1]);
    vector<Process> procs;
    
    //checks for invalid input file
    if (!input.good()) {
        cerr << "ERROR: Invalid input file" << endl;
        return 1;
    }
    
    //parse input file
    string str;
    while (getline(input, str)){ //read file line by line
        if (str[0] != '#' && str[0] != ' ' && str.size() != 0){ //skip any comment/blank lines
            
            string id = "";
            string mem = "";
            
            //break string into components
            unsigned int i = 0;
            while (str[i] != ' ' && str[i] != '\t'){
                id += str[i];
                i++;
            }
            i++;
            while (str[i] != ' ' && str[i] != '\t'){
                mem += str[i];
                i++;
            }
            i++;
            int memory = atoi(mem.c_str());
            
            //loop through arrival/run occurrences
            while (i < str.length()){
                
                string arrival = "";
                string run = "";
                
                while (i < str.length() && str[i] != '/'){
                    arrival += str[i];
                    i++;
                }
                i++;
                while (i < str.length() && str[i] != ' ' && str[i] != '\t'){
                    run += str[i];
                    i++;
                }
                i++;
                
                //convert values to int
                int arrival_time = atoi(arrival.c_str());
                int run_time = atoi(run.c_str());
                
                //add process to vector
                Process p(id, memory, arrival_time, run_time);
                procs.push_back(p);
            }
        }
    }
    
    //call simulations
    next_fit(procs);
    cout << endl;
    first_fit(procs);
    cout << endl;
    best_fit(procs);
    cout << endl;
    non_cont(procs);
    
    return 0;
}

