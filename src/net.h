#ifndef __NET_H__
#define __NET_H__
#include <iostream>
#include <vector>
#include "cell.h"
using namespace std;

class Net {

public:
    Net(int str) : A(0), B(0), name(str){} 
    ~Net() {}
    int name;
    int A, B;
    vector <int> cellList;
};

#endif
