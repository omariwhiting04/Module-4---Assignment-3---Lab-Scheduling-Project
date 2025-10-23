#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <climits>
using namespace std;

struct Process {
    string id;
    int arrival_time;
    int burst_time;
    int priority;       // unused
    int remaining_time;
    int waiting_time;
    int turnaround_time;
    int deadline;       // arrival + k*burst (k=2 here)
};

void calculateMetrics(vector<Process>& ps, int total_time){
    double aw=0, at=0; for(auto& p:ps){ aw+=p.waiting_time; at+=p.turnaround_time; }
    aw/=ps.size(); at/=ps.size();
    cout<<"Avg Waiting Time: "<<aw<<"\n";
    cout<<"Avg Turnaround Time: "<<at<<"\n";
    cout<<"CPU Utilization: 100%\n";
}
void printGantt(const vector<pair<string,int>>& g){
    cout<<"Gantt Chart: "; for(auto&e:g) cout<<e.first<<"("<<e.second<<") "; cout<<"\n";
}
vector<Process> loadDefaultTable(){
    vector<Process> v = {
        {"P1",0,8,2,8,0,0,0},
        {"P2",1,4,1,4,0,0,0},
        {"P3",2,9,3,9,0,0,0},
        {"P4",3,5,4,5,0,0,0},
    };
    for(auto& p: v) p.deadline = p.arrival_time + 2*p.burst_time; // example rule
    return v;
}
void reset(vector<Process>& ps){ for(auto& p:ps){ p.remaining_time=p.burst_time; p.waiting_time=p.turnaround_time=0; } }

int main(){
    vector<Process> ps = loadDefaultTable(); reset(ps);
    sort(ps.begin(), ps.end(), [](auto&a, auto&b){ if(a.arrival_time!=b.arrival_time) return a.arrival_time<b.arrival_time; return a.id<b.id; });

    int n=ps.size(), i=0, t=0, done=0, last=-1;
    vector<pair<string,int>> gantt;

    auto enqueue_until = [&](int upto){ while(i<n && ps[i].arrival_time<=upto){ ++i; } };

    if(ps[0].arrival_time>0) t=ps[0].arrival_time;
    enqueue_until(t);

    while(done<n){
        // ready set
        vector<int> ready;
        for(int k=0;k<n;++k) if(ps[k].remaining_time>0 && ps[k].arrival_time<=t) ready.push_back(k);

        if(ready.empty()){
            if(i<n){ t=max(t, ps[i].arrival_time); enqueue_until(t); }
            continue;
        }

        // pick earliest absolute deadline; tiebreak by arrival then id
        int pick = ready[0];
        for(int idx: ready){
            if(ps[idx].deadline < ps[pick].deadline) pick = idx;
            else if(ps[idx].deadline == ps[pick].deadline){
                if(ps[idx].arrival_time < ps[pick].arrival_time) pick = idx;
                else if(ps[idx].arrival_time == ps[pick].arrival_time && ps[idx].id < ps[pick].id) pick = idx;
            }
        }

        if(pick!=last && last!=-1) gantt.push_back({ps[last].id, t});
        last = pick;

        // run 1 tick (preemptive behavior)
        ps[pick].remaining_time--; t++;
        enqueue_until(t);

        if(ps[pick].remaining_time==0){
            ps[pick].turnaround_time = t - ps[pick].arrival_time;
            ps[pick].waiting_time    = ps[pick].turnaround_time - ps[pick].burst_time;
            gantt.push_back({ps[pick].id, t}); last=-1; done++;
        }
    }

    printGantt(gantt);
    calculateMetrics(ps, t);
    return 0;
}
