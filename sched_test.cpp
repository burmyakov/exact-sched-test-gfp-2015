#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <algorithm>
#include "ts.h"
#include "state.h"
#include "stateCompact.h"


using namespace std;



int m;




// Global FP (priority ordering given by indices, index 1 == highest priority)
bool prefer(const state& s, int i, int k) {
    if(s.c[i]==0 && s.c[k]==0) return (i < k); 		// no active job from i or k -- use default ordering
    if(s.c[i]==0 || s.c[k]==0) return s.c[i] > 0; 	// only 1 active -- prefer that one
    return (i < k); 								// FP rule (no ties are possible)
}



void sortTasksByPriorities(const state& s, int* perm) {
    int N = s.ts.n;
    for(int i = 0; i < N; i++) perm[i] = i;
    
    for(int i = 0; i < N; i++)
        for(int k = i+1; k < N; k++)
            if(!prefer(s, perm[i], perm[k]))
                swap(perm[i], perm[k]);
}






int deltaT(const state& s, const int *perm, const int N) {
    
    int dt;
    int pendJobs = 0;
    for (int i = 0; i < N; i++) if (s.c[i] > 0) pendJobs++;
    
    if (pendJobs <= m) {
        dt = s.t[0];
        for (int i = 1; i < N; i++)
            dt = min(dt, s.t[i]);
    } else {
        dt = s.c[perm[0]];
        for (int i = 1; i < m; i++) dt = min(dt, s.c[perm[i]]);
        for (int i = m; i < N; i++) dt = min(dt, s.t[perm[i]]);
    }
    
    dt = max(1, dt);
    
    return dt;
}







bool adversaryMove(state& s, state& v2, int K) {
    TS& ts = s.ts;
    v2.pl = state::ALG;
    
    // check if the state is initial
    bool initState = true;
    for (int i = 0; i < ts.n; i++)
        if ((s.c[i] != 0)||(s.t[i] != 0)||(s.d(i) != 0)) {initState = false; break;}
    
    
    for (int i = 0; i < ts.n; i++) {
        if((K & (1<<i)) && s.t[i] <= 0) {
            
            v2.c[i] = ts.C[i];
            v2.t[i] = ts.T[i];
            v2.interferred[i] = 0;
            v2.released[i] = 1;
            
            // Constraint of Davis and Burns 2011 on \tau_k release
            if ((i == ts.n-1) && (v2.pendJobsNu() <= m)) return false;
            if ((i == ts.n-1) && (!initState) && (!s.f1)) return false;
        }
    }
    
    // Constraint on \tau_1,...,\tau_{k-1} releases
    if (v2.c[ts.n-1] > 0) {
        bool leftSideHolds = false;
        for (int i = 0; i < ts.n-1; i++) {
            if ((v2.t[i] == ts.T[i]) && (ts.T[i] - v2.t[ts.n-1] >= 0)) {
                leftSideHolds = true;
                break;
            }
        }
        if ((leftSideHolds) && (v2.pendJobsNu() <= m)) {
            return false;
        }
    }
    
    
    // Constraint: sum_{i=1}^{k-1} p_i(t) > 0, for all t: c_k(t) > 0
    int sumTi = 0;
    for (int i = 0; i < ts.n-1; i++) sumTi += max(0, v2.t[i]);
    if ((v2.c[s.ts.n-1] > 0) && (sumTi <= 0)) {
        return false;
    }
    
    return true;
}





void update_interference_flags(state& s, int pendJobs, int N, int* perm) {
    
    // interference takes place only if
    // number of pending jobs > m
    if (pendJobs > m) {
        for (int i = 0; i < m; i++)
            if (s.c[i] > 0) s.interferred[i] = 1;
        
        for (int i = m; i < N-1; i++) {
            
            if (s.c[i] > 0) {
                
                bool executes = 0;
                for (int ii = 0; ii < m; ii++)
                    if (perm[ii] == i) {executes = 1; break;}
                
                if (!executes) continue;
                
                bool delayedAnotherJob = 0;
                for (int ii = i+1; ii < N; ii++) {
                    if (s.c[ii] > 0) {
                        for (int iii = m; iii < N; iii++) {
                            if (ii == perm[iii]) delayedAnotherJob = 1;
                            break;
                        }
                    }
                    if (delayedAnotherJob) break;
                }
                
                if (executes && delayedAnotherJob) s.interferred[i] = 1;
            }
        }
    }
}





// return codes:
// -1 - deadline miss
// 0 - continue
// 1 - schedule can be discarded
int algorithmMove(state& s, state& v1) {
    int N = v1.ts.n;
    v1.pl = state::ADV;
    
    int* perm = new int[N];
    sortTasksByPriorities(s, perm);
    
    // get number of pending jobs
    int pendJobs = s.pendJobsNu();
    
    // update interference flags
    update_interference_flags(v1, pendJobs, N, perm);
    
    // execute m HP jobs for the next dt time units
    int dt = deltaT(v1, perm, N);
    
    
    // compute .prevActive
    int prevActive = 0;
    for (int i = 0; i < m; i++)
        if (v1.c[perm[i]] - (dt-1) > 0) prevActive++;
    for (int i = m; i < N; i++)
        if (v1.c[perm[i]] > 0) prevActive++;
    if (prevActive < m) v1.f1 = 1; else v1.f1 = 0;
    
    
    
    for (int i = 0; i < m; i++) v1.c[perm[i]] = max(v1.c[perm[i]] - dt, 0);
    delete [] perm;
    
    // interference condition
    for (int i = 0; i < N-1; i++) {
        if ((v1.released[i]) && (v1.c[i] == 0) && (!v1.interferred[i])) {
            return 1;
        }
    }
    
    // update t_i
    for (int i = 0; i < N; i++) v1.t[i] = v1.t[i] - dt;
    
    // Deadline check
    if (v1.d(N-1) < v1.c[N-1]) {
        cout << "Failure state:" << endl;
        for (int i = 0; i < v1.ts.n; i++) {
            cout << "c[" << i << "]: " << v1.c[i] << " t[" << i << "]: " << v1.t[i] << endl;
        }
        
        return -1;
    }
    
    // Check if analyzed job completes execution
    if ((v1.released[N-1]) && (v1.c[N-1] == 0)) {
        return 1;
    }
    
    return 0;
}









float get_Iub(const state& s, const int N) {
    TS& ts = s.ts;
    int W = 0;
    int t = s.d(N-1);
    
    for (int i = 0; i < N-1; i++) {
        W += s.c[i] + (max(0, t - max(0, s.t[i]))/ts.T[i])*ts.C[i] + min(ts.C[i], (max(0, t - max(0, s.t[i])) % ts.T[i]));
    }
    
    return W/m;
}




// necessary conditions for a deadline miss
bool do_traverse_further(state& s) {
    int N = s.ts.n;
    int pendJobsNu = s.pendJobsNu();
    
    // tested job successfully completes execution
    if ((s.released[N-1]) && (s.c[N-1] == 0)) return false;
    
    
    // job interference conditions
    // condition 1
    for (int i = 0; i < N-1; i++) {
        if ((s.released[i]) && (s.c[i] == 0) && (!s.interferred[i])) {
            return false;
        }
    }
    // condition 2
    if (pendJobsNu <= m) {
        for (int i = 0; i < N-1; i++) {
            if ((s.c[i] == 1) && (!s.interferred[i])) {
                return false;
            }
        }
    }
    
    // necessary unsched. condition for tau_N
    float Iub = get_Iub(s, N);
    if ((s.c[N-1] > 0) && (s.d(N-1) - Iub >= s.c[N-1])) return false;
    
    return true;
}






bool populate(TS& ts) {
    
    bool failureStateGenerated = false;
    
    map<stateCompact, bool> generated;
    queue<state> q;
    
    state start(ts);
    q.push(start);
    
    stateCompact startComp = start;
    generated[startComp] = true;
    
    while (!q.empty()) {
        state s = q.front();
        q.pop();
        
        if (s.pl == state::ADV) {
            // V1 -> V2
            for(int K = 0; K < (1<<ts.n); K++) {
                // check why the results are different if using s
                state v2 = s;
                if (!adversaryMove(s, v2, K)) continue;
                
                // check if further traversal required
                if (do_traverse_further(v2)) {
                    stateCompact v2Cmpct = v2;
                    if (!generated[v2Cmpct]) {
                        generated[v2Cmpct] = true;
                        q.push(v2);
                    }
                }
            }
        }
        else if (s.pl == state::ALG) {
            state v1 = s;
            int code = algorithmMove(s, v1);
            
            if (code == -1) {
                failureStateGenerated = true;
                break;
            }
            
            if (code == 0) {
                stateCompact v1Cmpct = v1;
                if (!generated[v1Cmpct]) {
                    generated[v1Cmpct] = true;
                    q.push(v1);
                }
            }
        }
    }
    
    cout << "Generated states: " << generated.size() << endl;
    
    return failureStateGenerated;
}








int main(int argc, char* argv[]) {
    
    cerr << "Number of processors? " << endl;
    cin >> m;
    TS ts, testedTs;
    ts.read();
    
    for (int i = 0; i < m; i++) testedTs.setTask(i, ts.C[i], ts.D[i], ts.T[i]);
    
    unsigned long t1;
    bool failureStateGenerated = 0;
    for (int N = m + 1; N <= ts.n; N++) {
        cout << "checking task " << N << endl;
        testedTs.n = N;
        testedTs.setTask(N-1, ts.C[N-1], ts.D[N-1], ts.T[N-1]);
        
        t1 = clock();
        failureStateGenerated = populate(testedTs);
        cout << "Execution time: " << clock() - t1 << " (" << (clock() - t1)/CLOCKS_PER_SEC << " sec)" << endl;
        
        if (failureStateGenerated) {
            cout << "Task " << N << " is unschedulable" << endl;
            break;
        }
    }
    
    if (!failureStateGenerated) cout << "Task set is SCHEDULABLE" << endl;
    else cout << "Task set is UNSCHEDULABLE" << endl;
    
    return 1;
}