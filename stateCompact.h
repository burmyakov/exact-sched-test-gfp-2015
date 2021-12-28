#include <iostream>
//#include "state.h"


using namespace std;


struct stateCompact {
    TS& ts;
    enum { ADV, ALG } pl; // player (Adversary or Algorithm)
    int* c;
    int* t;
    
    // constructor 1
    stateCompact(const state& s) : ts(s.ts) {
        c = new int[ts.n];
        t = new int[ts.n];
        for (int i = 0; i < ts.n; i++) {
            c[i] = s.c[i];
            t[i] = max(0, s.t[i]);
        }
        
        if (s.pl == state::ADV) pl = ADV; else pl = ALG;
    }
    
    // constructor 2
    stateCompact(const stateCompact& s) : ts(s.ts), pl(s.pl) {
        c = new int[ts.n];
        t = new int[ts.n];
        
        for (int i = 0; i < ts.n; i++) {
            c[i] = s.c[i];
            t[i] = s.t[i];
        }
    }
    
    // operator <
    bool operator<(const stateCompact& s) const {
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
    
    // destructor
    ~stateCompact() {
        delete [] c;
        delete [] t;
    }
};
