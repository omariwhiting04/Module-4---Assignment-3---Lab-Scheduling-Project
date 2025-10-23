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
    int priority;       // unused here
    int remaining_time;
    int waiting_time;
    int turnaround_time;
};

// ----- common helpers -----
void calculateMetrics(vector<Process>& processes, int total_time) {
    double avg_wait = 0, avg_turn = 0;
    for (auto& p : processes) { avg_wait += p.waiting_time; avg_turn += p.turnaround_time; }
    avg_wait /= processes.size(); avg_turn /= processes.size();
    cout << "Avg Waiting Time: " << avg_wait << "\n";
    cout << "Avg Turnaround Time: " << avg_turn << "\n";
    cout << "CPU Utilization: 100%\n";
}
void printGantt(const vector<pair<string,int>>& gantt) {
    cout << "Gantt Chart: "; for (auto& g : gantt) cout << g.first << "(" << g.second << ") "; cout << "\n";
}
vector<Process> loadDefaultTable() {
    return { {"P1",0,8,2,8,0,0}, {"P2",1,4,1,4,0,0}, {"P3",2,9,3,9,0,0}, {"P4",3,5,4,5,0,0} };
}
void reset(vector<Process>& ps){ for(auto& p:ps){ p.remaining_time=p.burst_time; p.waiting_time=0; p.turnaround_time=0; } }


int main() {
    const int QUANTUM = 4;
    vector<Process> ps = loadDefaultTable(); reset(ps);
    sort(ps.begin(), ps.end(), [](auto&a, auto&b){ if(a.arrival_time!=b.arrival_time) return a.arrival_time<b.arrival_time; return a.id<b.id; });

    queue<int> ready; vector<pair<string,int>> gantt;
    int n=ps.size(), done=0, t=0, i=0, last=-1;

    auto enqueue_arrivals = [&](int upto){
        while(i<n && ps[i].arrival_time<=upto){ ready.push(i); i++; }
    };

    // start at first arrival if no one at t=0
    if (ps[0].arrival_time>0) t = ps[0].arrival_time;
    enqueue_arrivals(t);

    while(done<n){
        if(ready.empty()){
            // jump to next arrival
            if(i<n){ t = max(t, ps[i].arrival_time); enqueue_arrivals(t); }
            continue;
        }
        int idx = ready.front(); ready.pop();

        if(idx!=last && last!=-1) gantt.push_back({ps[last].id, t});
        last = idx;

        // run for up to QUANTUM, enqueuing arrivals as time advances
        int ran = 0;
        while(ran<QUANTUM && ps[idx].remaining_time>0){
            ps[idx].remaining_time--; t++; ran++;
            enqueue_arrivals(t);
        }

        if(ps[idx].remaining_time==0){
            ps[idx].turnaround_time = t - ps[idx].arrival_time;
            ps[idx].waiting_time    = ps[idx].turnaround_time - ps[idx].burst_time;
            gantt.push_back({ps[idx].id, t});
            last=-1; done++;
        }else{
            ready.push(idx); // time slice expired; preempt
        }
    }

    printGantt(gantt);
    calculateMetrics(ps, t);
    return 0;
}
