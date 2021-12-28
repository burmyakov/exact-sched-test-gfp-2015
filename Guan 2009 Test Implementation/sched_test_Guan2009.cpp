//
//  main.cpp
//  GFPSchedulabilityTest_v0.3_cpp
//
//  Created by Artem Burmyakov on 22/11/14.
//  Copyright (c) 2014 Artem Burmyakov. All rights reserved.
//

#include <iostream>



using namespace std;

const int NMAX = 16;
int Rs[NMAX];
int Is_NC[NMAX];
int Is_CI[NMAX];
int Is_DIFF[NMAX];
int Is_DIFF_SORTED[NMAX];





int get_Wi_NC(int i, int Rk, int Cs[NMAX], int Ps[NMAX]) {
    int Wi_NC = 0;
    int Ni_NC = Rk/Ps[i];
    
    Wi_NC = Ni_NC*Cs[i] + min(Cs[i], Rk - Ni_NC*Ps[i]);
    Wi_NC = max(0, Wi_NC);
    
    //cout << "Wi_NC[" << i << "]: " << Wi_NC << endl;
    
    return Wi_NC;
}


void get_Is_NC(int k, int Rk, int Cs[NMAX], int Ps[NMAX]) {
    
    for (int i = 0; i < k; i++) {
        int Wi_NC = get_Wi_NC(i, Rk, Cs, Ps);
        Is_NC[i] = max(0, min(Wi_NC, Rk - Cs[k] + 1));
        //cout << "Wi_NC[" << i << "]:" << Wi_NC << ", Is_NC[" << i << "]:" << Is_NC[i] << endl;
    }
}




int get_Wi_CI(int i, int Rk, int Cs[NMAX], int Ps[NMAX]) {
    int Wi_CI = 0;
    int Ni_CI = (Rk + Rs[i] - Cs[i])/Ps[i];
    
    Wi_CI = Ni_CI*Cs[i] + min(Cs[i], Rk + Rs[i] - Cs[i] - Ni_CI*Ps[i]);
    Wi_CI = max(0, Wi_CI);
    
    //cout << "Wi_CI[" << i << "]: " << Wi_CI << endl;
    
    return Wi_CI;
}




void get_Is_CI(int k, int Rk, int Cs[NMAX], int Ps[NMAX]) {
    
    for (int i = 0; i < k; i++) {
        int Wi_CI = get_Wi_CI(i, Rk, Cs, Ps);
        Is_CI[i] = max(0, min(Wi_CI, Rk - Cs[k] + 1));
        //cout << "Wi_CI[" << i << "]:" << Wi_CI << ", Is_CI[" << i << "]:" << Is_CI[i] << endl;
    }
}





void get_Is_DIFF(int k) {
    //cout << "Is_DIFF before sorting:" << endl;
    for (int i = 0; i < k; i++) {
        Is_DIFF[i] = Is_CI[i] - Is_NC[i];
        //cout << "Is_DIFF[" << i << "]:" << Is_DIFF[i] << endl;
    }
    
    // sort by decreasing
    int x;
    for (int j = k-1; j >= 0; j--) {
        for (int i = 0; i <= j; i++) {
            if (i < j) {
                if (Is_DIFF[i] < Is_DIFF[i+1]) {
                    x = Is_DIFF[i];
                    Is_DIFF[i] = Is_DIFF[i+1];
                    Is_DIFF[i+1] = x;
                }
            }
        }
    }
    
    /*cout << "Is_DIFF after sorting:" << endl;
    for (int i = 0; i < k; i++) {
        cout << "Is_DIFF[" << i << "]:" << Is_DIFF[i] << endl;
    }*/
}





int get_omega(int m, int k, int Rk, int Cs[NMAX], int Ps[NMAX]) {
    int Omega = 0;
    int terms = 0;
    int term1 = 0;
    int term2 = 0;
    
    get_Is_NC(k, Rk, Cs, Ps);
    get_Is_CI(k, Rk, Cs, Ps);
    get_Is_DIFF(k);
    
    for (int i = 0; i < k; i++) {
        term1 += Is_NC[i];
        term2 += Is_DIFF[i];
    }
    
    terms = term1 + term2;
    
    Omega = Cs[k] + terms/m;
    //cout << "Omega: " << Omega << endl;
    
    return Omega;
}





void get_Rk(int m, int k, int Cs[NMAX], int Ps[NMAX]) {
    
    if (k < m) {
        Rs[k] = Cs[k];
        return;
    }
    
    int Rk = 0;
    int Omega = Cs[k];
    
    while (Rk != Omega) {
        Rk = Omega;
        //cout << "k: " << k << ", Rk: " << Rk <<endl;
        if (Rk > Ps[k]) {
            Rs[k] = Rk;
            return;
        }
        
        Omega = get_omega(m, k, Rk, Cs, Ps);
    }
    //cout << "Rk = Omega; Rk: " << Rk << ", Omega: " << Omega << endl;
    
    Rs[k] = Rk;
}





int sched_test_Guan2009(int n, int m, int Cs[NMAX], int Ps[NMAX]) {
    
    //cout << "Guan 2009 test" << endl;
    
    bool sched = 1;
    
    for (int k = 0; k < n; k++) {
        get_Rk(m, k, Cs, Ps);
        //cout << "R[" << k << "]:" << Rs[k] << endl;
        if (Rs[k] > Ps[k]) {
            sched = 0;
            return sched;
        }
    }
    
    return sched;
}
