#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <ctype.h>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <cmath>
#include <ctime>
#include "net.h"
#include "cell.h"

using namespace std;

vector <Net*> netLib;
vector <Cell*> cellLib;
vector <int> cellstack;
vector <int> bestA, bestB;

int totalArea = 0; 
int groupAArea = 0; 
int groupBArea = 0;
int cutSize = 0;
int aCellCount = 0;
int bCellCount = 0;
int afCellCount = 0;
int bfCellCount = 0;
int accumulateGain = 0;
int bestAccumulateGain = 0;
int k = 0;
int bestk;
int CellCount = 0;
int totalNet = 0;//@@@@@@@@@@@@@@@@@@@
int rW;
int SMAX = 0;
int bestACellCount = 0;
int bestBCellCount = 0;
int bestGroupAArea = 0;
int bestGroupBArea = 0;
int Pmax = 0;
double tstart;
double error; 
double tend;
map <int, Node*> blist[2];

void calAB(){
    for (int i = 0; i < totalNet; i++){
        vector<int> & vl = netLib[i]->cellList;
        netLib[i]->B = 0;
        netLib[i]->A = 0;
        for (int j = 0; j < vl.size(); j++){
            Cell * cell = cellLib[vl[j]];
            if (cell->set) netLib[i]->B++;
            else netLib[i]->A++;
        }
    }
}

void countCutSize(){
    cutSize = 0;
    for (int i = 0; i < totalNet; i++)
        if (netLib[i]->A && netLib[i]->B) cutSize++;
    
}

void test(){
    for (int i = 0; i < CellCount; i++){
        cout << cellLib[i]->name << " s"
	     << cellLib[i]->size << " g"
             << cellLib[i]->gain << " ab"
	     << cellLib[i]->set  << " p"
	     << cellLib[i]->pins << " l"
	     << cellLib[i]->lock << ' ';
	for (int j = 0; j < cellLib[i]->netList.size(); j++){
	    int id = cellLib[i]->netList[j];
	    cout << netLib[id]->name << ' ';
        }
	cout << endl;
    }
    cout << "...\n";
    cout << "# of cells = " << CellCount << endl;
    cout << "Total Cell Size = " << totalArea << endl;
    cout << "Set A Size = " << groupAArea << endl;
    cout << "Set B Size = " << groupBArea << endl;
    cout << "|A - B| = " << abs(groupAArea-groupBArea) << endl;
    cout << "...\n";
    for (int i = 0; i < totalNet; i++){
        cout << netLib[i]->name << ' ';
        for (int j = 0; j < netLib[i]->cellList.size(); j++){
            int id = netLib[i]->cellList[j];
            cout << cellLib[id]->name << ' ';
        }
        cout << endl;
    }
    cout << "...\n";
    return;
}

void countPmax(){
    for (int i = 0; i < CellCount; i++)
        if (cellLib[i]->pins > Pmax) Pmax = cellLib[i]->pins;
}

void countError(){

    error = (double) totalArea/10;
}

void outputFile(ostream & out){
    out << "cut_size " << cutSize << endl;
    out << "A " << aCellCount << endl;
    for (int i = 0; i < CellCount; i++)
        if (!cellLib[i]->set)
            out << cellLib[i]->name << endl;
    out << "B " << bCellCount << endl;
    for (int i = 0; i < CellCount; i++)
        if (cellLib[i]->set)
            out << cellLib[i]->name << endl;
}

void parseInput(int argc, char ** argv){
    char opt = 0;
    string inputFile, outputFile;
    inputFile = argv[1];
    
    ifstream ifs(inputFile.c_str(), ifstream::in);
    string dotCell,dotNet;
    if(ifs.is_open()){
        ifs >> rW;
        cout<<rW<<">>>>"<<endl;
        ifs >> dotCell;
        cout<<dotCell<<"...."<<endl;
        int cellIndex,area;
        ifs >> CellCount;
        cout<<"the CellCount= "<<CellCount<<endl;

        for(int i=0;i<CellCount;i++){
            ifs >> cellIndex >> area;
            if(area > SMAX)SMAX = area;
            totalArea += area;
            if(groupAArea <= groupBArea){
                Cell* c = new Cell(cellIndex,area,0,cellIndex);
                cellLib.push_back(c);
                groupAArea += area;
                aCellCount++;
            }else{
                Cell* c = new Cell(cellIndex,area,1,cellIndex);
                cellLib.push_back(c);
                groupBArea += area;
                bCellCount++;
            }
        }
        cout<<aCellCount<<" "<<bCellCount<<" and totalArea: "<< totalArea <<endl;
        ifs >> dotNet;
        ifs >> totalNet;
        cout<<"number of net "<<totalNet<<endl;
        for(int netIndex = 0;netIndex<totalNet;netIndex++){
            int conCellsOnNet;
            ifs >> conCellsOnNet;
            Net* n = new Net(netIndex);
            netLib.push_back(n);

            for(int i=0;i<conCellsOnNet;i++){
                int cellId;
                ifs >> cellId;
                vector<int>& l = cellLib[cellId]->netList;
                l.push_back(netIndex);
                cellLib[cellId]->pins++;
                netLib[netIndex]->cellList.push_back(cellId);
                if (cellLib[cellId]->set) netLib[netIndex]->B++;
                else netLib[netIndex]->A++;
            }

        }
    }
}

void traverse(){
    
    for (int k = 0; k < 2; k++){
        cout << "---- " << ((!k) ? "A" : "B") << " ----\n";
        for (int i = Pmax ; i >= -Pmax; i--){
            cout << '[' << i << ']' << ' ';
            Node *trav = blist[k][i]->next;
            while (trav != NULL){
                cout << cellLib[trav->id]->name << "->";
                trav = trav->next;
            }
            cout << endl;
        }
    }
}

void remove(Cell* c){
    Node *p = c->to;
    p->prev->next = p->next;
    if (p->next != NULL) p->next->prev = p->prev;
}

void insert_front(Cell * c){
    int gain = c->gain;
    bool set = c->set;
    Node *p = c->to;
    p->prev = blist[set][gain];
    p->next = blist[set][gain]->next;
    blist[set][gain]->next = p;
    if (p->next != NULL) p->next->prev = p;
}

void move(Cell * c){
    remove(c);
    insert_front(c);
}

void buildBlist(){
    blist[0].clear();
    blist[1].clear();
    for (int i = -Pmax; i <= Pmax; i++) {
        if (blist[0][i] == NULL) blist[0][i] = new Node(-1);
        if (blist[1][i] == NULL) blist[1][i] = new Node(-1);
    }
    for (int i = 0; i < CellCount; i++)
        insert_front(cellLib[i]);
}


Cell * findMaxGain(bool set){
    int p = Pmax;
    while (p >= -Pmax && blist[set][p]->next == NULL){p--;}
    Cell * ans = cellLib[blist[set][p]->next->id];
    return ans;
}


void reverse(){
    int i = cellstack.size()-1;
    for (; i > k; i--)
        cellLib[cellstack[i]]->set = !cellLib[cellstack[i]]->set;
    
}

void store(){
    bestAccumulateGain = accumulateGain;
    bestACellCount = aCellCount;
    bestBCellCount = bCellCount;
    bestGroupAArea = groupAArea;
    bestGroupBArea = groupBArea;
    bestk = k;
}

void restore(){
    k = bestk;
    aCellCount = bestACellCount;
    bCellCount = bestBCellCount;
    groupAArea = bestGroupAArea;
    groupBArea = bestGroupBArea;
    reverse();
    calAB();
}

void initGain(){
    for (int i = 0; i < CellCount; i++){
        cellLib[i]->gain = 0;
        cellLib[i]->lock = 0;
    }
    
    accumulateGain = 0;
    store();
    afCellCount = aCellCount;
    bfCellCount = bCellCount;
   
    
    
    for (int i = 0; i < CellCount; i++){
        for (int j = 0 ; j < cellLib[i]->netList.size(); j++){
            int id = cellLib[i]->netList[j];
            if (cellLib[i]->set == 0) {
                if (netLib[id]->A == 1) cellLib[i]->gain++;
                if (netLib[id]->B == 0) cellLib[i]->gain--;
            }
            else {
                if (netLib[id]->B == 1) cellLib[i]->gain++;
                if (netLib[id]->A == 0) cellLib[i]->gain--;
            }
        }
    }
    buildBlist();
}

void updateGain(Cell * c){
    accumulateGain += c->gain;

    
    c->lock = true;
    int num = c->to->id;
    cellstack.push_back(num);
    if (!c->set) {
        int szn = c->netList.size();
        for(int i = 0; i < szn; i++){
            int id = c->netList[i];
            Net * net = netLib[id];
            int szc = net->cellList.size();
            if (net->B == 0){
                for (int j = 0; j < szc; j++){
                    int idc = net->cellList[j];
                    if (!cellLib[idc]->lock) {
                        cellLib[idc]->gain++;
                        move(cellLib[idc]);
                    }
                }
            }
            else if (net->B == 1){
                for (int j = 0; j < szc; j++){
                    int idc = net->cellList[j];
                    if (!cellLib[idc]->lock && cellLib[idc]->set) {
                        cellLib[idc]->gain--;
                        move(cellLib[idc]);
                    }
                }
            }
            net->A--;
            net->B++;
            c->set = true;
            if (net->A == 0){
                for (int j = 0; j < szc; j++){
                    int idc = net->cellList[j];
                    if (!cellLib[idc]->lock) {
                        cellLib[idc]->gain--;
                        move(cellLib[idc]);
                    }
                }
            }
            else if (net->A == 1){
                for (int j = 0; j < szc; j++){
                    int idc = net->cellList[j];
                    if (!cellLib[idc]->lock && !cellLib[idc]->set) {
                        cellLib[idc]->gain++;
                        move(cellLib[idc]);
                    }
                }
            }
        }
        remove(c);
        groupAArea -= c->size;
        groupBArea += c->size;
        afCellCount--;
        aCellCount--;
        bCellCount++;
    }
    else {
        int szn = c->netList.size();
        for(int i = 0; i < szn; i++){
            int id = c->netList[i];
            Net * net = netLib[id];
            int szc = net->cellList.size();
            if (net->A == 0){
                for (int j = 0; j < szc; j++){
                    int idc = net->cellList[j];
                    if (!cellLib[idc]->lock) {
                        cellLib[idc]->gain++;
                        move(cellLib[idc]);
                    }
                }
            }
            else if (net->A == 1){
                for (int j = 0; j < szc; j++){
                    int idc = net->cellList[j];
                    if (!cellLib[idc]->lock && !cellLib[idc]->set) {
                        cellLib[idc]->gain--;
                        move(cellLib[idc]);
                    }
                }
            }
            net->B--;
            net->A++;
            c->set = false;
            if (net->B == 0){
                for (int j = 0; j < szc; j++){
                    int idc = net->cellList[j];
                    if (!cellLib[idc]->lock) {
                        cellLib[idc]->gain--;
                        move(cellLib[idc]);
                    }
                }
            }
            else if (net->B == 1){
                for (int j = 0; j < szc; j++){
                    int idc = net->cellList[j];
                    if (!cellLib[idc]->lock && cellLib[idc]->set) {
                        cellLib[idc]->gain++;
                        move(cellLib[idc]);
                    }
                }
            }
        }
        remove(c);
        groupBArea -= c->size;
        groupAArea += c->size;
        bfCellCount--;
        bCellCount--;
        aCellCount++;
    }
    if (accumulateGain > bestAccumulateGain)
        store();
    return;
}

int pass = 0;

void FMAlgorithm(){
    bool flag = false;
    initGain();
    int count = 0;
    k = 0;
    bestk = 0;
    cellstack.clear();
    cout<<"here"<<endl;
    while (!flag && count++ < CellCount){
        if (!bfCellCount){
            Cell * a = findMaxGain(0);
            if (abs(groupAArea-groupBArea-2*a->size) < error) updateGain(a);
            else flag = true;
        }
        else if (!afCellCount){
            Cell * b = findMaxGain(1);
            if (abs(groupBArea-groupAArea-2*b->size) < error) updateGain(b);
            else flag = true;
        }
        else {
            Cell * a = findMaxGain(0), * b = findMaxGain(1);
            if (a->gain >= b->gain) {
                if (abs(groupAArea-groupBArea-2*a->size) < error) updateGain(a);
                else if (abs(groupBArea-groupAArea-2*b->size) < error) updateGain(b);
                else flag = true;
            }
            else {
                if (abs(groupBArea-groupAArea-2*b->size) < error) updateGain(b);
                else if (abs(groupAArea-groupBArea-2*a->size) < error) updateGain(a);
                else flag = true;
            }
        }
        k++;
    }
    
    if (bestAccumulateGain > 0 ) {
        pass++;
        
        restore();
        cout << "Pass " << pass << endl;
        cout << "Best Partial Sum of Gains: " << bestAccumulateGain << endl;
        cout << "Total Sum of Gains (Should be 0): " << accumulateGain << endl;
        cout << endl;
        FMAlgorithm();
        
    }
    else { bestk = -1; k = -1; return;}
}

void adjust(){//the adjustion needs to be modified...
    if (abs(groupAArea-groupAArea) < error) return;
    else {
        int i;
        for (i = 0; i < CellCount && abs(groupAArea-groupBArea) >= error; i++){
            Cell *c = cellLib[i];
            if (groupAArea > groupBArea && !c->set){
                groupAArea -= c->size;
                groupBArea += c->size;
                c->set = true;
            }
            else if (groupAArea < groupBArea && c->set){
                groupAArea += c->size;
                groupBArea -= c->size;
                c->set = false;
            }
        }
        //if (i == CellCount && groupAArea <= rW + SMAX && groupAArea >= rW -SMAX && groupAArea >= rW -SMAX ) {
        //    cerr << "(ERROR)...This test case can never be balanced!\n";
        //    //exit(EXIT_FAILURE);
        //}
    }
}

int main(int argc, char *argv[]){
    ios_base::sync_with_stdio(false);
    parseInput(argc, argv);
    countCutSize();
    countError();
    countPmax();
    adjust();
    
    bestGroupAArea = groupAArea;
    bestGroupBArea = groupBArea;

    tstart = clock();
    FMAlgorithm();
	tend = clock();

    restore();
    countCutSize();
    
    cout << "Final Cut Size = " << cutSize << endl;
    string outputFileName=argv[2];
    ofstream ofs;
    ofs.open(outputFileName);
    if (ofs.is_open()) outputFile(ofs);
    else outputFile(cout);
    ofs.close();
    cout << endl;
    cout << "FM Algorithm Run Time: " << (double)(tend-tstart)/CLOCKS_PER_SEC << " sec\n";
    cout << "Total Run Time: " << (double)clock()/CLOCKS_PER_SEC << " sec\n";
}
