#ifndef __CELL_H__
#define __CELL_H__
#include <iostream>
#include <set>
#include <vector>
using namespace std;

class Node
{
    friend class Cell;
 
public:
    Node(const int &info) : id(info), next(NULL), prev(NULL){}

    int id;
    Node *next, *prev; 
}; 

class Cell {
    public:
        Cell(int & str, int & sz, bool st, int & id) : 
            name(str), size(sz), gain(0), pins(0), 
            set(st), lock(0), to(NULL){
            to = new Node(id);
        }
        ~Cell (){}
        int name;
        int size, gain, pins;
        bool set, lock;
        vector <int> netList;
        Node *to;
};

#endif


/*

*/