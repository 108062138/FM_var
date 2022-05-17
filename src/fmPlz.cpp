#include <map>
#include <iostream>
#include <algorithm>
#include <set>
#include <vector>
#include <string>
#include <climits>
#include <unordered_set>
#include "assert.h"
#include <fstream>
#include <stack>
#include <queue>
#define UNGIVEN -1
#define NOGOODCELL -2
#define REVERSEALL -3
#define NOGOODGROUP -4
#define A 0
#define B 1
#define SHOWDBG true
#define HIDEDBG false
#define FIXSELECT false
#define MYSELECT true
using namespace std;

int numGroup;

struct Cell{
    int cellIndex;
    int area;
    int curGroup;
    int prevGroup;
    int gain;
    unordered_set<int> netsList;
    Cell(int cellIndex, int area):cellIndex(cellIndex),area(area),curGroup(UNGIVEN),prevGroup(UNGIVEN),gain(){}
    void display(){
        cout<<"cell index: "<<cellIndex<<" area: "<<area<<endl;
        cout<<"nowat "<<curGroup<<" and prev: "<<prevGroup<<endl;
        cout<<"its gain: "<<gain<<endl;
        cout<<"its related nets: ";
        for(auto x=netsList.begin();x!=netsList.end();++x)cout<<*x<<" ";
        cout<<endl;
    }
};

struct Net{
    int netIndex;
    vector<unordered_set<int>> atGroup;
    map<int,int> cellAndItbelong;
    Net(int index,int gs):netIndex(index){
        for(int i=0;i<gs;i++){
            unordered_set<int> us;
            atGroup.push_back(us);
        }
    }
    void display(){
        cout<<"net index: "<<netIndex<<endl;
        for(int i=0;i<numGroup;i++){
            auto g = atGroup[i];
            cout<<"---> net "<< netIndex<<" at group "<<i<<" contains "<< g.size() <<" cells"<<endl;
            cout<<"they are: ";
            for(auto x=g.begin();x!=g.end();++x)cout<<*x<<" ";
            cout<<endl;
        }
    }
};

ifstream ifs;
ofstream ofs;

int lowestCut;
int totalCell;
int totalNet;
int limit;
int allArea;
int predefSel[20];
vector<Cell*> cellBuffer;
vector<Net*> netBuffer;
unordered_set<int> netThatHasLockedCell;
vector<int> groupArea;
vector<unordered_set<int>> group;
unordered_set<int> lockBuffer;

bool portCompetition(int a, int b){
    return cellBuffer[a]->netsList.size()< cellBuffer[b]->netsList.size();
};

void groupBalance(){
    cout<<"\ntotal area: "<<allArea<<endl;
    for(int i=0;i<numGroup;i++){
        auto g = group[i];
        int localWeight = 0;
        for(auto x = g.begin();x!=g.end();++x){
            localWeight+=cellBuffer[*x]->area;
        }
        if(localWeight>limit)assert(0);
        cout<<"group "<<i<<"'s  area: "<<localWeight<<endl;
    }
}

int cntcut(){
    int cnt = 0;
    for(int i=0;i<totalNet;i++){
        int cross = 0;
        for(int j=0;j<numGroup;j++){
            if(netBuffer[i]->atGroup[j].size()>0){
                cross++;
            }
        }
        int incrementOnNet = (cross-1)*(cross-1);
        cnt+=incrementOnNet;
    }
    
    return cnt;
}

bool cmpCellIndex(const Cell* a,const Cell* b) {
    return ((*a).cellIndex < (*b).cellIndex);
}

void fromGroup(){
    for(int i=0;i<numGroup;i++){
        auto g=group[i];
        int tmparea=0;
        for(auto x=g.begin();x!=g.end();++x){
            int cellIndex = *x;
            tmparea+=cellBuffer[cellIndex]->area;
            for(int j=0;j<totalNet;j++)
                if(netBuffer[j]->cellAndItbelong.find(cellIndex)!=netBuffer[j]->cellAndItbelong.end()){
                    if(netBuffer[j]->atGroup[i].find(cellIndex)==netBuffer[j]->atGroup[i].end()){
                        cout<<"can't find "<<cellIndex<<" at net "<<j<<" in group "<<i<<endl;
                        assert(0);
                    }
                }
            for(int j=0;j<totalNet;j++)
                if(netBuffer[j]->cellAndItbelong.find(cellIndex)!=netBuffer[j]->cellAndItbelong.end())
                    if(netBuffer[j]->cellAndItbelong[cellIndex]!=i)
                        assert(0);
        }
        if(tmparea!=groupArea[i])assert(0);
    }
}

void fromNet(){
    for(auto i=0;i<totalNet;i++){
        auto n = netBuffer[i];
        for(auto x=n->cellAndItbelong.begin();x!=n->cellAndItbelong.end();++x){
            int cellIndex = x->first;
            if(group[x->second].find(cellIndex)==group[x->second].end())
                assert(0);
            if(cellBuffer[cellIndex]->curGroup!=x->second)
                assert(0);
        }
    }
}

void parseInput(bool dbgFlag) {
    string dotCell, dotNet;
    ifs >> limit;
    ifs >> dotCell;
    ifs >> totalCell;
    allArea = 0;
    for (int i = 0; i < totalCell; i++) {
        int index, area;
        ifs >> index >> area;
        allArea += area;
        Cell* tmpCell = new Cell(index,area);
        cellBuffer.push_back(tmpCell);
    }
    int posNumGroup=0;
    while (posNumGroup*limit<allArea){
        posNumGroup++;
    }
    numGroup = posNumGroup;
    for(int i=0;i<numGroup;i++){
        unordered_set<int> us;
        groupArea.push_back(0);
        group.push_back(us);
    }
    
    sort(cellBuffer.begin(), cellBuffer.end(), cmpCellIndex);
    ifs >> dotNet;
    ifs >> totalNet;
    for (int netIndex = 0; netIndex < totalNet; netIndex++) {
        int netContainCell;
        ifs >> netContainCell;
        Net* tmpNet = new Net(netIndex,numGroup);
        for (int i = 0; i < netContainCell; i++) {
            int cell;
            ifs >> cell;
            cellBuffer[cell]->netsList.insert(netIndex);
            tmpNet->cellAndItbelong.insert(make_pair(cell,UNGIVEN));
        }
        netBuffer.push_back(tmpNet);
    }
    if (dbgFlag) {
        cout << "total Area:" << allArea<< " and its constrain: "<< limit << "##################" << endl;
    }
}

int initWhere(int targetCell){
    int bestGap = INT_MAX;
    int g =  NOGOODGROUP;
    for(int i=0;i<numGroup;i++){
        int posArea = cellBuffer[targetCell]->area+groupArea[i];
        if(posArea<=limit){
            int localGap = limit - posArea;
            if(localGap<bestGap){
                bestGap = localGap;
                g = i;
            }
        }
    }
    return g;
}

void ezkway(){
    for(int i=0;i<totalCell;i++){
        int belongTo= initWhere(i);
        cellBuffer[i]->curGroup = belongTo;
        group[belongTo].insert(i);
        groupArea[belongTo]+=cellBuffer[i]->area;
        for(auto n = cellBuffer[i]->netsList.begin();n!=cellBuffer[i]->netsList.end();++n){
            int netIndex = *n;
            netBuffer[netIndex]->cellAndItbelong[i]=belongTo;
            netBuffer[netIndex]->atGroup[belongTo].insert(i);
        }
    }

    fromGroup();
    fromNet();
    groupBalance();
    cout<<"peace at ez k way!"<<endl;
}

void eztwoway(){
    for(int i=0;i<totalCell;i++){
        int belongTo= A;
        if(groupArea[A]>groupArea[B])
            belongTo = B;
        cellBuffer[i]->curGroup = belongTo;
        group[belongTo].insert(i);
        groupArea[belongTo]+=cellBuffer[i]->area;
        for(auto n = cellBuffer[i]->netsList.begin();n!=cellBuffer[i]->netsList.end();++n){
            int netIndex = *n;
            netBuffer[netIndex]->cellAndItbelong[i]=belongTo;
            netBuffer[netIndex]->atGroup[belongTo].insert(i);
        }
    }
}

//we haven't fix it!!!!
void initGain(){
    for(int i=0;i<totalCell;i++){
        cellBuffer[i]->gain=0;
        int F = cellBuffer[i]->curGroup;
        int T = T=(F==A)?B:A;//
        if(numGroup==2)T=(F==A)?B:A;
        else T = T=(F==A)?B:A;//
        for(auto n = cellBuffer[i]->netsList.begin();n!=cellBuffer[i]->netsList.end();++n){
            int netIndex = *n;
            int Fn = netBuffer[netIndex]->atGroup[F].size();
            int Tn = netBuffer[netIndex]->atGroup[T].size();
            if(Fn==1)
                cellBuffer[i]->gain++;
            if(Tn==0)
                cellBuffer[i]->gain--;
        }
    }
}
void showGain(){
    for(int i=0;i<totalCell;i++){
        cout<<i<<" "<<cellBuffer[i]->gain<<endl;
    }
}

bool balance(int cell){
    int F = cellBuffer[cell]->curGroup;
    int T = (F==A)?B:A;
    if(groupArea[T]+cellBuffer[cell]->area<=limit)return true;
    else return false;
}

int findTarget(bool howtoselect,int step){
    if(howtoselect==FIXSELECT){
        return predefSel[step];
    }else{
        int target=NOGOODCELL;
        int curg = INT_MIN;
        //priority_queue<int,vector<int>,decltype(&portCompetition)> candidate;
        vector<int> candidate;
        for(int i=0;i<totalCell;i++){
            if(lockBuffer.find(i)==lockBuffer.end()){
                if(balance(i)){
                    if(curg <= cellBuffer[i]->gain){
                        if(curg < cellBuffer[i]->gain){
                            curg = cellBuffer[i]->gain;
                            target = i;
                            candidate.clear();
                            candidate.push_back(i);
                        }else{
                            candidate.push_back(i);
                        }
                    }
                }
            }
        }
        if(candidate.size()<=1)
            return target;
        else{
            for(auto can:candidate){
                if(cellBuffer[can]->netsList.size() > cellBuffer[target]->netsList.size()){
                    target = can;
                }
            }
            return target;
        }
    }
}

void update(stack<int>& st,int targetCell,int* localGain,int* bestGain, int*keep){
    int F = cellBuffer[targetCell]->curGroup;
    int T = (F==A)?B:A;
    st.push(targetCell);
    (*localGain)+=cellBuffer[targetCell]->gain;
    if((*localGain)>(*bestGain)){
        (*bestGain)=(*localGain);
        (*keep) = targetCell;
    }
    lockBuffer.insert(targetCell);
    group[F].erase(targetCell);
    group[T].insert(targetCell);
    groupArea[F]-=cellBuffer[targetCell]->area;
    groupArea[T]+=cellBuffer[targetCell]->area;

    cellBuffer[targetCell]->prevGroup = F;
    cellBuffer[targetCell]->curGroup = T;

    int Fn,Tn;
    for(auto n = cellBuffer[targetCell]->netsList.begin();n!=cellBuffer[targetCell]->netsList.end();++n){
        auto& net = netBuffer[*n];
        if(netThatHasLockedCell.find(net->netIndex)==netThatHasLockedCell.end())
            netThatHasLockedCell.insert(net->netIndex);
        Tn = net->atGroup[T].size();
        if(Tn==0){
            for(auto cell = net->cellAndItbelong.begin();cell!=net->cellAndItbelong.end();++cell){
                if(lockBuffer.find(cell->first)==lockBuffer.end()){
                    cellBuffer[cell->first]->gain++;
                }
            }
        }
        if(Tn==1){
            for(auto cell=net->atGroup[T].begin();cell!=net->atGroup[T].end();++cell){
                if(lockBuffer.find(*cell)==lockBuffer.end()){
                    cellBuffer[*cell]->gain--;
                }
            }
        }

        net->cellAndItbelong[targetCell] = T;
        net->atGroup[F].erase(targetCell);
        net->atGroup[T].insert(targetCell);

        Fn = net->atGroup[F].size();

        if(Fn==0){
            for(auto cell = net->cellAndItbelong.begin();cell!=net->cellAndItbelong.end();++cell){
                if(lockBuffer.find(cell->first)==lockBuffer.end()){
                    cellBuffer[cell->first]->gain--;
                }
            }
        }
        if(Fn==1){
            for(auto cell=net->atGroup[F].begin();cell!=net->atGroup[F].end();++cell){
                if(lockBuffer.find(*cell)==lockBuffer.end()){
                    cellBuffer[*cell]->gain++;
                }
            }
        }

    }
}

void regret(int revCell){
    int nowAt = cellBuffer[revCell]->curGroup;
    int shouldBe = (nowAt==A)?B:A;
    group[nowAt].erase(revCell);
    group[shouldBe].insert(revCell);
    groupArea[nowAt]-=cellBuffer[revCell]->area;
    groupArea[shouldBe]+=cellBuffer[revCell]->area;
    cellBuffer[revCell]->curGroup = shouldBe;
    auto nl = cellBuffer[revCell]->netsList;
    for(auto n=nl.begin();n!=nl.end();++n){
        int netIndex = *n;
        netBuffer[netIndex]->cellAndItbelong[revCell] = shouldBe;
        netBuffer[netIndex]->atGroup[nowAt].erase(revCell);
        netBuffer[netIndex]->atGroup[shouldBe].insert(revCell);
    }
}

void fm(int iter,bool dbg){
    if(iter>20)return;
    int step=0;
    lockBuffer.clear();
    netThatHasLockedCell.clear();
    stack<int> st;
    initGain();
    int localGain=0;int bestGain=0;int keep = REVERSEALL;
    while (1){
        int targetCell = findTarget(MYSELECT,step);
        if(targetCell==NOGOODCELL)break;
        //if(step%1000==0)
        //    cout<<"at step "<<step<<" we switch cell "<<targetCell<<" and its gain: "<<cellBuffer[targetCell]->gain<<" the best gain: "<<bestGain<<endl;
        update(st,targetCell,&localGain,&bestGain,&keep);
        step++;
    }
    if(keep==REVERSEALL){
        while (!st.empty()){
            int revCell = st.top();
            st.pop();
            regret(revCell);
        }
        cout<<"stop!"<<endl;
        cout<<"res: "<<cntcut()<<endl;
        return;
    }else{
        while (!st.empty()&&st.top()!=keep){
            int revCell = st.top();st.pop();
            regret(revCell);
        }

    }//
    fromGroup();
    fromNet();
    groupBalance();
    int cnt = cntcut();
    cout<<"peace at iter. "<<iter<<" and its cut: "<< cnt << " best gain: "<< bestGain <<endl;
    if(lowestCut>cnt){
        cout<<"cur best cut: "<<cnt<<endl;
    }
    if(bestGain<=0){
        cout<<"stop!"<<endl;
        cout<<"res: "<<cntcut()<<endl;
        return;
    }
    fm(iter+1,dbg);
}

void dumpFile(){
    ofs << cntcut()<<endl;
    ofs << numGroup <<endl;
    for(int i=0;i<totalCell;i++){
        ofs << cellBuffer[i]->curGroup<<endl;
    }
}

int main(int argc, char* argv[]){
    ifs.open(argv[1]);
    ofs.open(argv[2]);
    predefSel[0] = 1;predefSel[1] = 14;predefSel[2] = 10;predefSel[3] = 2;predefSel[4] = 13;predefSel[5] = 7;predefSel[6] = 12;predefSel[7] = 9;predefSel[8] = 5;predefSel[9] = 8;predefSel[10] = 3;predefSel[11] = 0;predefSel[12] = 6;predefSel[13] = 4;predefSel[14] = 11;
    parseInput(SHOWDBG);
    if(numGroup)eztwoway();
    else ezkway();
    lowestCut = INT_MAX;
    cout<<"#of g: "<<numGroup<<endl;
    if(numGroup==2)fm(0,SHOWDBG);
    dumpFile();
}
