#include <iostream>
//#include "ts.h"


using namespace std;


struct state {
    TS& ts;
    enum { ADV, ALG } pl; // player (Adversary or Algorithm)
    int* c; // remaining execution time
    int* t; // time to next period
    bool* interferred; // interference flags
    bool* released;
    bool f1; // 1 - if number of pending jobs at previous state < m, 0 - otherwise
    
    // constructor 1
    state(TS& ts_) : ts(ts_) {
        c = new int[ts.n];
        t = new int[ts.n];
        interferred = new bool[ts.n-1];
        released = new bool[ts.n];
        pl = ADV;
        f1 = 0;
        
        for (int i = 0; i < ts.n; i++) c[i] = t[i] = 0;
        for (int i = 0; i < ts.n-1; i++) interferred[i] = 0;
        for (int i = 0; i < ts.n; i++) released[i] = 0;
    }
    
    // constructor 2
    state(const state& s) : ts(s.ts), pl(s.pl), f1(s.f1) {
        c = new int[ts.n];
        t = new int[ts.n];
        interferred = new bool[ts.n-1];
        released = new bool[ts.n];
        
        for (int i = 0; i < ts.n; i++) {
            c[i] = s.c[i];
            t[i] = s.t[i];
        }
        
        for (int i = 0; i < ts.n-1; i++) interferred[i] = s.interferred[i];
        for (int i = 0; i < ts.n; i++) released[i] = s.released[i];
    }
    
    // operator <
    bool operator<(const state& s) const {
        if (pl<s.pl) return true;
        if (s.pl<pl) return false;
        for (int i = 0; i < ts.n; i++) {
            if (c[i] < s.c[i]) return true;
            if (s.c[i] < c[i]) return false;
            if (t[i] < s.t[i]) return true;
            if (s.t[i] < t[i]) return false;
        }
        return false;
    }
    
    // compute number of pending jobs
    int pendJobsNu() const {
        int pendJobsNu = 0;
        
        for (int i = 0; i < ts.n; i++)
            if (c[i] > 0) pendJobsNu++;
        
        return pendJobsNu;
    }
    
    // Time to next deadline of a task
    inline int d(int i) const { return max(t[i] - (ts.T[i] - ts.D[i]), 0); }
    
    // destructor
    ~state() {
        delete [] c;
        delete [] t;
        delete [] interferred;
        delete [] released;
    }
};
