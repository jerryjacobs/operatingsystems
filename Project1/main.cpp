/*
 Operating Systems - Project 1
 Jerald Jacobs - jacobj7
 Cavell Teng - tengc
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
using namespace std;


//<proc-id>|<initial-arrival-time>|<cpu-burst-time>|<num-bursts>|<io-time>
class Process{
public:
    Process(){};
    Process(string id_, int arrival_time_, int burst_time_, int num_burst_, int io_time_){
        id = id_;
        arrival_time = arrival_time_;
        burst_time = burst_time_;
        num_burst = num_burst_;
        io_time = io_time_;
        burst_left = num_burst_;
        wait_time = 0;
        turnaround_time = 0;
        waits = 0;
        turn = 0;
        start = -1;
        waiting = false;
        remaining_time = burst_time;
    };
    
    int wait_time;          //total wait time for process
    int turnaround_time;    //total turnaround time for process
    int burst_left;         //keeps track of remaining bursts
    int io_end;				//keeps track of ending io time
    int remaining_time;     //remaining time for given burst
    int start;				//the time a process enters the queue
    int turn_start;         //Keeps track of when abother turnaround starts
    int waits;              //Keeps track of the number of times waited
    int turn;               //Keeps track of the number of turnarounds
    bool waiting;           //Keeps track of whether the process is waiting in ready queue
    
    string get_id() const { return id; }
    int get_arrival_time() const { return arrival_time; }
    int get_burst_time() const { return burst_time; }
    int get_num_burst() const { return num_burst; }
    int get_io_time() const { return io_time; }
    void set_wait( int cur_time){ if(!waiting){start = cur_time; waits++; waiting = true;}}
    void end_wait( int cur_time){ wait_time += (cur_time - start);waiting = false;}
    void set_turn( int cur_time){ turn_start = cur_time; turn++;}
    void end_turn( int cur_time){ turnaround_time += cur_time - turn_start;}
    void reset() {      //Resets these variables in between CPU Scheduling Algorithm calls
        wait_time = 0;
        turnaround_time = 0;
        burst_left = num_burst;
        waits = 0;
        turn = 0;
        start = -1;
        remaining_time = burst_time;
        waiting = false;
    }
    
private:
    string id;
    int arrival_time;
    int burst_time;
    int num_burst;
    int io_time;
};



//compare processes by arrival time
bool order_arrival(Process a, Process b){
    if(a.get_arrival_time() != b.get_arrival_time()){
        return a.get_arrival_time() < b.get_arrival_time();
    }
    else{
        return a.get_id() < b.get_id();
    }
}


//compare processes by remaining time
bool order_remaining(Process* a, Process* b){
    if(a->remaining_time != b->remaining_time){
        return a->remaining_time < b->remaining_time;
    }
    else{
        return a->get_id() < b->get_id();
    }
}


//compare processes by io_end time
bool order_io_end(Process* a, Process* b){
    if(a->io_end != b->io_end){
        return a->io_end < b->io_end;
    }
    else{
        return a->get_id() < b->get_id();
    }
}


//queue class to store with process pointers
class Queue{
public:
    Queue(){};
    int size() { return q.size(); } //returns size of queue
    Process* head() { return q[0]; } //returns head of queue
    void add(Process* p) { q.push_back(p); } //adds element to queue
    void add_srt(Process* p) { //adds element to queue then sorts via srt
        q.push_back(p);
        sort(q.begin(), q.end(), order_remaining);
    }
    void add_io(Process* p) { //adds element to queue then sorts via io end time
        q.push_back(p);
        sort(q.begin(), q.end(), order_io_end);
    }
    Process* pop() { //returns the first element of queue and removes it
        Process* p = q[0];
        for (unsigned int i = 0; i < q.size()-1; i++){
            q[i] = q[i+1];
        }
        q.pop_back();
        return p;
    }
    string print() { //returns a string showing contents of queue
        if (q.size() == 0){
            return "[Q <empty>]";
        }
        else{
            string str = "[Q";
            for (unsigned int i = 0; i < q.size(); i++){
                str += " ";
                str += q[i]->get_id();
            }
            str += "]";
            return str;
        }
    }
    
private:
    vector<Process*> q;
};


//first come first serve algorithm
void FCFS(vector<Process> &procs, ostream &out, int t_cs){
    //Resets the processes
    for(unsigned int x = 0; x < procs.size(); x++){
        procs[x].reset();
    }
    out << "Algorithm FCFS" << endl;
    
    //Variable Initialization
    int num_preempt = 0;
    int num_switch = 0;
    int switch_timer = 0;
    unsigned int term = 0;
    bool bursting = false;
    bool sw_start = false;
    Queue q;
    Queue io;
    int shortest_io = 999999;
    int cur_time = 0;
    int next_burst = 0;
    Process *burst;
    unsigned int i = 0;                                      //Keeps track of the arriving processes
    int next_arrival = procs[0].get_arrival_time();
    cout << "time 0ms: Simulator started for FCFS [Q <empty>]" << endl;
    
    while(term < procs.size()){			//Keeps looping as long as there are still processes remaining
        //Checks if the process is done bursting
        if(cur_time == next_burst && bursting == true){
            (burst->burst_left)--;
            if(burst->burst_left != 0){ //Checks if its done with all of its bursts
                burst->io_end = cur_time + burst->get_io_time() + t_cs/2;
                io.add(burst);
                //Checks if the I/O time of the process going into block is less
                if(burst->io_end < shortest_io){shortest_io = burst->io_end;}
                burst->end_turn(cur_time + t_cs/2);
                if (burst->burst_left > 1){    //Checks if there are more than one bursts left
                    cout << "time " << cur_time << "ms: Process " << burst->get_id() << " completed a CPU burst; " << burst->burst_left << " bursts to go " << q.print() << endl;
                }
                else{
                    cout << "time " << cur_time << "ms: Process " << burst->get_id() << " completed a CPU burst; " << burst->burst_left << " burst to go " << q.print() << endl;
                }
                cout << "time " << cur_time << "ms: Process " << burst->get_id() << " switching out of CPU; will block on I/O until time " << (burst->io_end) <<"ms " << q.print() << endl;
                
            }
            //All bursts are done, terminating process
            else{cout << "time " << cur_time << "ms: Process " << burst->get_id() << " terminated " << q.print() << endl; term++; burst->end_turn(cur_time + t_cs/2);}
            switch_timer += (t_cs/2);
            bursting = false;
        }
        
        //Checks if a process is done blocking on I/O
        if(shortest_io == cur_time){
            int k = 0;
            int temp_shortest = 999999;
            while(1){   //Loops through all the processes in IO blocking
                if(io.size() == 0){ shortest_io = 999999; break;}   //No more processes in IO, break
                if(k == io.size()){shortest_io = temp_shortest; break;}     //Went through all the processes
                
                Process *temp2 = io.pop();
                if(temp2->io_end == cur_time){   //Checks if the IO process is done
                    q.add(temp2);                //Send to ready queue
                    cout << "time " << temp2->io_end << "ms: Process " << temp2->get_id() << " completed I/O; added to ready queue " << q.print() << endl;
                    temp2->set_wait(cur_time);
                    temp2->set_turn(cur_time);
                }
                else{       //Checks if IO process is done faster than the current one
                    if(temp2->io_end < temp_shortest){temp_shortest = temp2->io_end;}
                    io.add_io(temp2);
                    k++;
                }
            }
        }
        
        //Checks if a process has just arrived
        if(next_arrival == cur_time && i < procs.size()){
            for(unsigned int j = i; j < procs.size(); j++){			//Checks for processes that arrive at the same time as the first one
                if( next_arrival == procs[j].get_arrival_time()){
                    q.add(&procs[j]);
                    cout << "time " << cur_time << "ms: Process " << procs[j].get_id() << " arrived and added to ready queue " << q.print() << endl;
                    procs[j].set_wait(cur_time);
                    procs[j].set_turn(cur_time);
                    i++;
                }
            }
            if(i < procs.size()){       //If there are more processes left to arrive, set next arrival time
                next_arrival = procs[i].get_arrival_time();
            }
        }
        
        //Checks if there is anything in the ready queue when there is no burst
        if(q.size() != 0 && bursting == false && switch_timer == 0){
            if(sw_start == true){       //If done switching, begin bursting
                burst = q.pop();
                if(burst->start != -1){ burst->end_wait(cur_time - t_cs/2);}
                cout << "time " << cur_time << "ms: Process " << burst->get_id() << " started using the CPU " << q.print() << endl;
                next_burst = burst->get_burst_time() + cur_time;
                bursting = true;
                sw_start = false;
            }
            else{switch_timer += (t_cs/2); sw_start = true; num_switch++;}  //If no switch yet, start
        }
        cur_time++;
        if(switch_timer > 0){switch_timer--;}
    }
    cur_time += 3;
    
    //Calculations for the data output to the text file
    int tot_burst = 0;
    double ave_wait = 0;
    double ave_turn = 0;
    int tot_waits = 0;
    int tot_turns = 0;
    for(unsigned int b = 0; b < procs.size(); b++){
        tot_burst += (procs[b].get_burst_time() * procs[b].get_num_burst());
        ave_wait += procs[b].wait_time;
        tot_waits += procs[b].waits;
        ave_turn += procs[b].turnaround_time;
        tot_turns += procs[b].turn;
    }
    double ave_burst = tot_burst*1.0 / num_switch;
    ave_wait /= tot_waits;
    ave_turn /= tot_turns;
    cout << "time " << cur_time << "ms: Simulator ended for FCFS" << endl;
    out << "-- average CPU burst time: " << fixed << setprecision(2) << ave_burst << " ms" << endl;
    out << "-- average wait time: " << fixed << setprecision(2) << ave_wait << " ms" << endl;
    out << "-- average turnaround time: " << fixed << setprecision(2) << ave_turn << " ms" << endl;
    out << "-- total number of context switches: " << num_switch << endl;
    out << "-- total number of preemptions: " << num_preempt << endl;
}


//shortest remaining time algorithm
void SRT(vector<Process> &procs, ostream &out, int t_cs){
    
    //variables
    int t = 0;              //elapsed time
    int switches = 0;       //number of context switches
    int preemptions = 0;    //number of preepmtions
    
    unsigned int arrived = 0;       //number of processes that have arrived
    unsigned int terminated = 0;    //number of terminated processes
    
    bool switching_in = false;  //are we switching in
    bool switching_out = false; //are we switching out
    int switch_in = t_cs/2;     //time it takes to context switch in
    int switch_out = t_cs/2;    //time it takes to context switch out
    
    Process* cpu = NULL;    //the process that is in the cpu
    Queue q;                //priority queue by remaining time
    Queue io;               //priority queue by io_end time
    Queue pre;              //preemption queue
    
    //reset all processes
    for (unsigned int i = 0; i < procs.size(); i++){
        procs[i].reset();
    }
    sort(procs.begin(), procs.end(), order_arrival);
    
    //start of simulation
    cout << "time 0ms: Simulator started for SRT [Q <empty>]" << endl;
    while (terminated < procs.size()){ //loop until all processes terminate
        
        //process finishes using the CPU
        if (cpu != NULL && cpu->remaining_time == 0 && !switching_out){
            cpu->burst_left--;
            cpu->turnaround_time += t - cpu->turn_start + 4; //add turnaround time
            
            //process terminates by finishing its last CPU burst
            if (cpu->burst_left == 0){
                cout << "time " << t << "ms: Process " << cpu->get_id() << " terminated " << q.print() << endl;
                terminated++;
            }
            else{
                if (cpu->burst_left > 1){
                    cout << "time " << t << "ms: Process " << cpu->get_id() << " completed a CPU burst; " << cpu->burst_left << " bursts to go " << q.print() << endl;
                }
                else{
                    cout << "time " << t << "ms: Process " << cpu->get_id() << " completed a CPU burst; " << cpu->burst_left << " burst to go " << q.print() << endl;
                }
                
                //process starts performing I/O
                cpu->io_end = t + (t_cs/2) + cpu->get_io_time(); //determine io_end time
                io.add_io(cpu); //add process to io queue
                cout << "time " << t << "ms: Process " << cpu->get_id() << " switching out of CPU; will block on I/O until time " << cpu->io_end << "ms " << q.print() << endl;
            }
            switching_out = true;
            continue;
        }
        if (switch_out == 0){
            if (pre.size() > 0){ //preemption has occurred
                q.add_srt(cpu);
                cpu->start = t; //set wait timer
            }
            else{
                cpu->remaining_time = cpu->get_burst_time(); //reset remaining time
            }
            cpu = NULL;
            switch_out = t_cs/2;
            switching_out = false;
            continue;
        }
        
        //process finishes performing I/O
        if (io.size() > 0 && io.head()->io_end == t){
            
            io.head()->turn_start = t; //set turnaround timer
            
            //process is preempted
            if (cpu != NULL && io.head()->remaining_time < cpu->remaining_time){
                cout << "time " << t << "ms: Process " << io.head()->get_id() << " completed I/O and will preempt " << cpu->get_id() << " " << q.print() << endl;
                pre.add(io.head());
                switching_out = true;
            }
            else{
                q.add_srt(io.head());
                cout << "time " << t << "ms: Process " << io.head()->get_id() << " completed I/O; added to ready queue " << q.print() << endl;
                io.head()->start = t; //set wait timer
            }
            io.head()->io_end = -1;
            io.pop();
            continue;
        }
        
        //process arrives
        if (arrived < procs.size() && procs[arrived].get_arrival_time() == t){
            procs[arrived].turn_start = t; //set turnaround timer
            
            //process is preempted
            if (cpu != NULL && procs[arrived].remaining_time < cpu->remaining_time){
                cout << "time " << t << "ms: Process " << procs[arrived].get_id() << " arrived and will preempt " << cpu->get_id() << " " << q.print() << endl;
                pre.add(&procs[arrived]);
                switching_out = true;
            }
            else{
                q.add_srt(&procs[arrived]);
                cout << "time " << t << "ms: Process " << procs[arrived].get_id() << " arrived and added to ready queue " << q.print() << endl;
                procs[arrived].start = t; //set wait timer
            }
            arrived++;
            continue;
        }
        
        //process starts using the CPU
        if (cpu == NULL && (q.size() > 0 || pre.size() > 0) && !switching_in){ //start the context switch
            switching_in = true;
            continue;
        }
        if (switch_in == 0){
            switches++;
            if (pre.size() > 0){ //preemption occurred
                preemptions++;
                cpu = pre.pop();
            }
            else{
                cpu = q.pop();
                cpu->wait_time += t - cpu->start - 4;
            }
            if (cpu->remaining_time < cpu->get_burst_time()){
                cout << "time " << t << "ms: Process " << cpu->get_id() << " started using the CPU with " << cpu->remaining_time << "ms remaining " << q.print() << endl;
            }
            else{
                cout << "time " << t << "ms: Process " << cpu->get_id() << " started using the CPU " << q.print() << endl;
            }
            switch_in = t_cs/2;
            switching_in = false;
            continue;
        }
        
        //increment time
        t++;
        if (cpu != NULL && !switching_out){ cpu->remaining_time--; }
        if (switching_in){ switch_in--; }
        if (switching_out){ switch_out--; }
    }
    cout << "time " << t+4 << "ms: Simulator ended for SRT" << endl;
    
    //calculate statistics
    int num_burst = 0;      //number of CPU bursts
    int total_burst = 0;    //total CPU burst time
    int total_turn = 0;     //total turnaround time
    int total_wait = 0;     //total wait time
    
    for (unsigned int i = 0; i < procs.size(); i++){ //loop through processes
        num_burst += procs[i].get_num_burst();
        total_burst += procs[i].get_burst_time() * procs[i].get_num_burst();
        total_turn += procs[i].turnaround_time;
        total_wait += procs[i].wait_time;
    }
    
    float ave_burst = (total_burst*1.0)/num_burst;
    float ave_turn = (total_turn*1.0)/num_burst;
    float ave_wait = (total_wait*1.0)/num_burst;
    
    //finished simulation, output data
    out << "Algorithm SRT" << endl;
    out << "-- average CPU burst time: " << fixed << setprecision(2) << ave_burst << " ms" << endl;
    out << "-- average wait time: " << fixed << setprecision(2) << ave_wait << " ms" << endl;
    out << "-- average turnaround time: " << fixed << setprecision(2) << ave_turn << " ms" << endl;
    out << "-- total number of context switches: " << switches << endl;
    out << "-- total number of preemptions: " << preemptions << endl;
}


//round robin algorithm
void RR(vector<Process> &procs, ostream &out, int t_cs, int t_slice){
    //Resets the processes
    for(unsigned int x = 0; x < procs.size(); x++){
        procs[x].reset();
    }
    out << "Algorithm RR" << endl;
    
    //Variable initialization
    int num_preempt = 0;
    int num_switch = 0;
    int switch_timer = 0;
    int slice_timer = -1;
    unsigned int term = 0;
    bool bursting = false;
    bool sw_start = false;
    Queue q;
    Queue io;
    int shortest_io = 999999;
    int cur_time = 0;
    int next_burst = 0;
    Process *burst;
    unsigned int i = 0;                                      //Keeps track of the arriving processes
    int next_arrival = procs[0].get_arrival_time();
    
    cout << "time 0ms: Simulator started for RR [Q <empty>]" << endl;
    
    while(term < procs.size()){			//Keeps looping as long as there are still processes remaining
        //Checks if a process is bursting
        if(bursting == true){
            //If the process is done, no preemptions
            if(burst->remaining_time == 0){
                (burst->burst_left)--;
                if(burst->burst_left != 0){     //If more bursts left, I/O blocking
                    burst->remaining_time = burst->get_burst_time();
                    burst->io_end = cur_time + burst->get_io_time() + t_cs/2;
                    io.add_io(burst);
                    if(burst->io_end < shortest_io){shortest_io = burst->io_end;}
                    burst->end_turn(cur_time + t_cs/2);
                    if (burst->burst_left > 1){ //Checks if more than one burst left for output
                        cout << "time " << cur_time << "ms: Process " << burst->get_id() << " completed a CPU burst; " << burst->burst_left << " bursts to go " << q.print() << endl;
                    }
                    else{
                        cout << "time " << cur_time << "ms: Process " << burst->get_id() << " completed a CPU burst; " << burst->burst_left << " burst to go " << q.print() << endl;
                    }
                    cout << "time " << cur_time << "ms: Process " << burst->get_id() << " switching out of CPU; will block on I/O until time " << (burst->io_end) <<"ms " << q.print() << endl;
                    
                }
                else{cout << "time " << cur_time << "ms: Process " << burst->get_id() << " terminated " << q.print() << endl; term++; burst->end_turn(cur_time + t_cs/2);}
                switch_timer += (t_cs/2);
                bursting = false;
                burst = NULL;
                slice_timer = -1;
            }
            else{
                //If a process is done bursting in the time slice, preempt
                if (next_burst == cur_time) {
                    //If the queue is empty or empty and switching, then preempt
                    if(q.size() > 0 || (q.size() == 0 && sw_start == true)){
                        cout << "time " << cur_time << "ms: Time slice expired; process " << burst->get_id() << " preempted with " << burst->remaining_time << "ms to go " << q.print() << endl;
                        bursting = false;
                        num_preempt++;
                        slice_timer = -1;
                        q.add(burst);
                        switch_timer += (t_cs/2);
                        burst->set_wait(cur_time + t_cs/2);
                        burst = NULL;
                    }
                    //The ready queue is empty, no preemption is needed
                    else{
                        cout << "time " << cur_time << "ms: Time slice expired; no preemption because ready queue is empty " << q.print() << endl;
                        if (burst->remaining_time < t_slice) {next_burst = burst->remaining_time + cur_time;}
                        else{next_burst = t_slice + cur_time;}
                        slice_timer = t_slice;
                    }
                }
            }
        }
        //Checks if a process is done blocking on I/O
        if(shortest_io == cur_time){
            int k = 0;
            int temp_shortest = 999999;
            while(1){
                if(io.size() == 0){ shortest_io = 999999; break;}
                if(k == io.size()){shortest_io = temp_shortest; break;}
                
                Process *temp2 = io.pop();
                if(temp2->io_end == shortest_io){
                    //If the process is done switching in, begin bursting before I/O exit
                    if(sw_start == true && switch_timer == 0 && bursting == false){
                        if(burst->start != -1){ burst->end_wait(cur_time - t_cs/2);}
                        //Checks if process has maximum burst time
                        if (burst->remaining_time == burst->get_burst_time()) {
                            cout << "time " << cur_time << "ms: Process " << burst->get_id() << " started using the CPU "<< q.print() << endl;
                        }
                        else{
                            cout << "time " << cur_time << "ms: Process " << burst->get_id() << " started using the CPU with " << burst->remaining_time << "ms remaining " << q.print() << endl;
                        }
                        //Checks if the remaining time is less than the time slice
                        if (burst->remaining_time < t_slice) {next_burst = burst->remaining_time + cur_time;}
                        else{next_burst = t_slice + cur_time;}
                        bursting = true;
                        sw_start = false;
                        slice_timer = t_slice;
                    }
                    q.add(temp2);
                    cout << "time " << temp2->io_end << "ms: Process " << temp2->get_id() << " completed I/O; added to ready queue " << q.print() << endl;
                    temp2->set_wait(cur_time);
                    temp2->set_turn(cur_time);
                }
                else{
                    if(temp2->io_end < temp_shortest){temp_shortest = temp2->io_end;}
                    io.add_io(temp2);
                    k++;
                }
            }
        }
        //Checks if a process just arrived
        if(next_arrival == cur_time && i < procs.size()){
            for(unsigned int j = i; j < procs.size(); j++){			//Checks for processes that arrive at the same time as the first one
                if( next_arrival == procs[j].get_arrival_time()){
                    q.add(&procs[j]);
                    cout << "time " << cur_time << "ms: Process " << procs[j].get_id() << " arrived and added to ready queue " << q.print() << endl;
                    procs[j].set_wait(cur_time);
                    procs[j].set_turn(cur_time);
                    i++;
                }
            }
            if(i < procs.size()){
                next_arrival = procs[i].get_arrival_time();
            }
        }
        //Checks if no process bursting, not switching, and the queue is not empty
        if(bursting == false && switch_timer == 0){
            if(sw_start == true && slice_timer == -1){
                if(burst->start != -1){ burst->end_wait(cur_time - t_cs/2);}
                //Checks if process has maximum burst time
                if (burst->remaining_time == burst->get_burst_time()) {
                    cout << "time " << cur_time << "ms: Process " << burst->get_id() << " started using the CPU "<< q.print() << endl;
                }
                else{
                    cout << "time " << cur_time << "ms: Process " << burst->get_id() << " started using the CPU with " << burst->remaining_time << "ms remaining " << q.print() << endl;
                }
                //Checks if the remaining time is less than the time slice
                if (burst->remaining_time < t_slice) {next_burst = burst->remaining_time + cur_time;}
                else{next_burst = t_slice + cur_time;}
                bursting = true;
                sw_start = false;
                slice_timer = t_slice;
            }
            //Pops the process off the queue as it starts switching
            else if(q.size()!= 0 && sw_start == false){switch_timer += (t_cs/2); sw_start = true; num_switch++; burst = q.pop();}
        }
        cur_time++;
        if(switch_timer > 0){switch_timer--;}
        if (slice_timer > 0){
            slice_timer--;
            if(bursting){
                burst->remaining_time--;
            }
        }
    }
    cur_time += 3;
    //Calculations for data output to text file
    int tot_burst = 0;
    int time_burst = 0;
    double ave_wait = 0;
    double ave_turn = 0;
    int tot_waits = 0;
    int tot_turns = 0;
    for(unsigned int b = 0; b < procs.size(); b++){
        tot_burst += (procs[b].get_burst_time() * procs[b].get_num_burst());
        time_burst += procs[b].get_num_burst();
        ave_wait += procs[b].wait_time;
        tot_waits += procs[b].waits;
        ave_turn += procs[b].turnaround_time;
        tot_turns += procs[b].turn;
    }
    double ave_burst = tot_burst*1.0 / time_burst;
    ave_wait /= tot_turns;
    ave_turn /= tot_turns;
    cout << "time " << cur_time << "ms: Simulator ended for RR" << endl;
    out << "-- average CPU burst time: " << fixed << setprecision(2) << ave_burst << " ms" << endl;
    out << "-- average wait time: " << fixed << setprecision(2) << ave_wait << " ms" << endl;
    out << "-- average turnaround time: " << fixed << setprecision(2) << ave_turn << " ms" << endl;
    out << "-- total number of context switches: " << num_switch << endl;
    out << "-- total number of preemptions: " << num_preempt << endl;
}


//main
int main(int argc, char* argv[]) {
    
    //variables
    ifstream input(argv[1]);
    ofstream output(argv[2]);
    vector<Process> procs;
    
    //tunable constants
    int t_cs = 8;       //context switch time
    int t_slice = 70;   //time slice for RR
    
    //checks for incorrect number of arguments
    if (argc != 3) {
        cerr << "ERROR: Invalid arguments\nUSAGE: ./a.out <input-file> <stats-output-file>" << endl;
        return 1;
    }
    //checks for invalid input
    if (!input.good()) {
        cerr << "ERROR: Invalid input file format" << endl;
        return 1;
    }
    
    //parse input file
    string str;
    while (getline(input, str)){ //read file line by line
        if (str[0] != '#' && str[0] != ' ' && str.size() != 0){ //skip any comment/blank lines
            
            string id = "";
            string arrival = "";
            string burst = "";
            string num = "";
            string io = "";
            
            //break string into components
            unsigned int i = 0;
            while (str[i] != '|'){
                id += str[i];
                i++;
            }
            i++;
            while (str[i] != '|'){
                arrival += str[i];
                i++;
            }
            i++;
            while (str[i] != '|'){
                burst += str[i];
                i++;
            }
            i++;
            while (str[i] != '|'){
                num += str[i];
                i++;
            }
            i++;
            while (i < str.length()){
                io += str[i];
                i++;
            }
            
            //convert to values to int
            int arrival_time = atoi(arrival.c_str());
            int burst_time = atoi(burst.c_str());
            int num_burst = atoi(num.c_str());
            int io_time = atoi(io.c_str());
            
            //add process to vector
            Process p(id, arrival_time, burst_time, num_burst, io_time);
            procs.push_back(p);
        }
    }
    
    FCFS(procs, output, t_cs);
    cout << endl;
    SRT(procs, output, t_cs);
    cout << endl;
    RR(procs, output, t_cs, t_slice);
    
    return 0;
}