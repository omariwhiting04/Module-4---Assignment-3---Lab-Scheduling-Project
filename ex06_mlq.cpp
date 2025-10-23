#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <string>
#include <climits>
using namespace std;

struct Process {
    string id;
    int arrival_time;
    int burst_time;
    int priority;       // lower = higher 
    int remaining_time;
    int waiting_time;
    int turnaround_time;
};

void calculateMetrics(vector<Process>& processes, int total_time){
    double aw=0, at=0; for(auto&p:processes){ aw+=p.waiting_time; at+=p.turnaround_time; }
    aw/=processes.size(); at/=processes.size();
    cout<<"Avg Waiting Time: "<<aw<<"\n";
    cout<<"Avg Turnaround Time: "<<at<<"\n";
    cout<<"CPU Utilization: 100%\n";
}
void printGantt(const vector<pair<string,int>>& g){ cout<<"Gantt Chart: "; for(auto&e:g) cout<<e.first<<"("<<e.second<<") "; cout<<"\n"; }
vector<Process> loadDefaultTable(){ return {{"P1",0,8,2,8,0,0},{"P2",1,4,1,4,0,0},{"P3",2,9,3,9,0,0},{"P4",3,5,4,5,0,0}}; }
void reset(vector<Process>& ps){ for(auto& p:ps){ p.remaining_time=p.burst_time; p.waiting_time=p.turnaround_time=0; } }

int main(){
    const int HIGH_Q_QUANTUM = 4; // high queue uses RR
    vector<Process> ps = loadDefaultTable(); reset(ps);
    sort(ps.begin(), ps.end(), [](auto&a, auto&b){ if(a.arrival_time!=b.arrival_time) return a.arrival_time<b.arrival_time; return a.id<b.id; });
    int n=ps.size(), i=0, t=0, done=0, last=-1;

    queue<int> highQ, lowQ; 
    vector<pair<string,int>> gantt;

    auto enqueue_arrivals = [&](int upto){
        while(i<n && ps[i].arrival_time<=upto){
            if(ps[i].priority<3) highQ.push(i); else lowQ.push(i);
            i++;
        }
    };

    if(ps[0].arrival_time>0) t=ps[0].arrival_time;
    enqueue_arrivals(t);

    while(done<n){
        // prefer high queue
        if(highQ.empty() && lowQ.empty()){
            if(i<n){ t=max(t, ps[i].arrival_time); enqueue_arrivals(t); }
            continue;
        }

        if(!highQ.empty()){
            int idx = highQ.front(); highQ.pop();
            if(idx!=last && last!=-1) gantt.push_back({ps[last].id, t});
            last = idx;

            int ran=0;
            while(ran<HIGH_Q_QUANTUM && ps[idx].remaining_time>0){
                ps[idx].remaining_time--; t++; ran++; enqueue_arrivals(t);
            }

            if(ps[idx].remaining_time==0){
                ps[idx].turnaround_time = t - ps[idx].arrival_time;
                ps[idx].waiting_time    = ps[idx].turnaround_time - ps[idx].burst_time;
                gantt.push_back({ps[idx].id, t}); last=-1; done++;
            }else{
                highQ.push(idx); // RR rotate
            }
        }else{
            // low queue FCFS 
            int idx = lowQ.front(); lowQ.pop();
            if(idx!=last && last!=-1) gantt.push_back({ps[last].id, t});
            last = idx;

            // run to completion; still accept arrivals (go into appropriate queue)
            while(ps[idx].remaining_time>0){
                ps[idx].remaining_time--; t++; enqueue_arrivals(t);
            }
            ps[idx].turnaround_time = t - ps[idx].arrival_time;
            ps[idx].waiting_time    = ps[idx].turnaround_time - ps[idx].burst_time;
            gantt.push_back({ps[idx].id, t}); last=-1; done++;
        }
    }

    printGantt(gantt);
    calculateMetrics(ps, t);
    return 0;
}
