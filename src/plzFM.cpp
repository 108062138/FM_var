#include <map>
#include <iostream>
#include <algorithm>
#include <set>
#include <vector>
#include <string>
#include <climits>
#include <unordered_set>
#include <list>
#include "assert.h"
#include <fstream>
#include <stack>
#define CANTFINDPROPERGROUP -1
#define FINDNOGROUP -1
#define UNINITGROUP -1
#define INBETWEENGROUPTRANSITION -2
#define A 0
#define B 1
#define INITLOCK -1
#define UNLOCK 0
#define LOCKED 1
#define NORMALBALANCE 0
#define SWAPBALANCE 1
#define SHOWDBG true
#define HIDEDBG false
#define TWOWAY 2
#define IMPOSSIBLEPORT 1500000
#define NOPROPERCELL -5
#define ITERLIMIT 1
using namespace std;

struct Cell{
    int cellIndex;
    int area;
    int cellBelongToGroup;//ok at insert
    int lastLocatedGroup;//ok at insertjj
    unordered_set<int> netsList;
    ~Cell() {}
    Cell(int index, int area):cellIndex(index),area(area),cellBelongToGroup(-1),lastLocatedGroup(-1) {}
    void appendWireOnCell(int wireIndex) {
        netsList.insert(wireIndex);
    }
    void displayCell() {
        cout << "cell index: " << cellIndex << " its area: "<<area << " it now at " << cellBelongToGroup << endl;
        cout<< " this cell has " << netsList.size() << " net " << endl;
        cout <<"  ";
        for (auto net = netsList.begin(); net != netsList.end(); ++net) {
            cout << *net << " ";
        }cout << endl;
    }
};

struct Net{
    int netIndex;
    vector<unordered_set<int>> atGroup;
    map<int,int> cellsList;//first: cellIndex, second: cell's belong group

    Net(int netIndex) : netIndex(netIndex) {}
    void appendCellOnWire(int cellIndex) {
        cellsList.insert(make_pair(cellIndex,-1));//-1 for ungiven group
    }
    void displayNet() {
        cout << "net index: " << netIndex << " it contains " << cellsList.size() << " cells. They are: " << endl;
        for (auto cell : cellsList)cout << " cell " << cell.first << " belongs to group: " << cell.second << endl;
        for (int gp = 0; gp < atGroup.size(); gp++) {
            cout << "group " << gp << "contains ";
            for (auto cell = atGroup[gp].begin(); cell != atGroup[gp].end(); ++cell) {
                cout << *cell << " ";
            }cout << endl;
        }
    }
    void showFT(int from, int to) {
        cout << "FT RESULT: F(" << netIndex << ")=" << atGroup[from].size() << " and T(" << netIndex << ")=" << atGroup[to].size() << endl;
    }   
    void appendGroup(int groupNum) {
        for (int i = 0; i < groupNum; i++) {
            unordered_set<int> tmpgp;
            atGroup.push_back(tmpgp);
        }
    }
    ~Net() {}
};


ifstream ifs;
ofstream ofs;
int curBest;
int totalCell, totalNet, areaConstrain;
int numGroup,maxPort;
int allArea,lowestCut;
int localAccumulateGain,bestAccumulateGain,bestTop;
//group
vector<int> groupArea;
vector<int> gain;
vector<unordered_set<int>> group;
vector<unordered_set<int>> bestGroup;
unordered_set<int> lockBuffer;
map<int,list<int>> gainCategory;
//cell
vector<Cell*> cellBuffer;
//net
vector<Net*> netBuffer;
stack<int> rec;


bool cmpCellIndex(const Cell* a,const Cell* b) {
    return ((*a).cellIndex < (*b).cellIndex);
}
bool cmpGroupDelta(const pair<int,int> a, const pair<int,int> b) {
    return (a.second >= b.second);
}

void showGain(){
    int index = 0;
    for(auto x:gain){
        cout<<"cell "<<index<<"'s gain "<<x<<endl;
        index++;
    }
}
int cntCut(){
    int cut = 0;
    for(int i=0;i<netBuffer.size();i++){
        int cross=0;
        //cout<<"cntcut at "<<i<<": ";
        for(int j=0;j<numGroup;j++){
            //cout<< netBuffer[i]->atGroup[j].size()<<" ";
            if(netBuffer[i]->atGroup[j].size()>0)
                cross++;
        }
        int c=(cross-1)*(cross-1);
        //cout<<" its cut is "<<c<<endl;
        cut+=c;
    }
    return cut;
}
void showGainCategory(){
    for(auto x=gainCategory.begin();x!=gainCategory.end();++x){
        int g = x->first;
        auto ls = x->second;
        cout<<"gain g= "<<g<<" has the follow list: ";
        for(auto cell:ls){
            if(cell!=IMPOSSIBLEPORT&&gain[cell]!=g)
                assert(0);
            cout<<cell<<" ";
        }cout<<endl;
    }
}
void showGroup(){
    for(int groupIndex = 0;groupIndex<group.size();groupIndex++){
        int tmpArea=0;
        cout<<"at group "<<groupIndex<<"'s area "<< groupArea[groupIndex] <<" it contains: ";
        for(auto cellIndex = group[groupIndex].begin();cellIndex!=group[groupIndex].end();++cellIndex){
            tmpArea+=cellBuffer[*cellIndex]->area;
            if(cellBuffer[*cellIndex]->cellBelongToGroup!=groupIndex)
                assert(0);
            for(auto net=cellBuffer[*cellIndex]->netsList.begin();net!=cellBuffer[*cellIndex]->netsList.end();++net){
                if(cellBuffer[*cellIndex]->cellBelongToGroup!=netBuffer[*net]->cellsList[*cellIndex])
                    assert(0);
                if(netBuffer[*net]->atGroup[cellBuffer[*cellIndex]->cellBelongToGroup].find(*cellIndex)==netBuffer[*net]->atGroup[cellBuffer[*cellIndex]->cellBelongToGroup].end())
                    assert(0);
            }
            cout<<*cellIndex<<" ";
        }
        if(tmpArea!=groupArea[groupIndex])
            assert(0);
        cout<<endl;
    }
}
//just emplace the input, no partition is executed yet!
void parseInput(bool dbgFlag) {
    string dotCell, dotNet;
    ifs >> areaConstrain;
    cout<<"the areaconstrain: "<<areaConstrain<<endl;
    ifs >> dotCell;
    cout<<dotCell<<endl;
    ifs >> totalCell;
    allArea = 0;
    for (int i = 0; i < totalCell; i++) {
        int index, area;
        ifs >> index >> area;
        allArea += area;
        Cell* tmpCell = new Cell(index, area);
        cellBuffer.push_back(tmpCell);
    }
    sort(cellBuffer.begin(), cellBuffer.end(), cmpCellIndex);
    ifs >> dotNet;
    ifs >> totalNet;
    for (int netIndex = 0; netIndex < totalNet; netIndex++) {
        int netContainCell;
        ifs >> netContainCell;
        Net* tmpNet = new Net(netIndex);
        for (int i = 0; i < netContainCell; i++) {
            int cell;
            ifs >> cell;
            tmpNet->appendCellOnWire(cell);
            cellBuffer[cell]->appendWireOnCell(netIndex);
        }
        netBuffer.push_back(tmpNet);
    }
    if (dbgFlag) {
        cout << "FOR EACH CELL: " << endl;
        for (auto cell : cellBuffer) {
            cell->displayCell();
        }
        cout << "FOR EACH WIRE" << endl;
        for (auto net : netBuffer) {
            net->displayNet();
        }

        cout << "total Area:" << allArea<< " and its constrain: "<< areaConstrain << "##################" << endl;
    }
}

void estGroup(){
    numGroup = 0;
    while (numGroup*areaConstrain<=allArea){
        numGroup++;
    }
    
    //init group, grouparea
    for(int i=0;i<numGroup;i++){
        unordered_set<int> uset;
        group.push_back(uset);
        groupArea.push_back(0);
    }

    //init net's atGroup
    for(auto& net:netBuffer){
        for(int i=0;i<numGroup;i++){
            unordered_set<int> uset;
            net->atGroup.push_back(uset);
        }
    }
    cout<<"fk"<<endl;
}

int initCellBelong(int cellIndex){
    if(groupArea[A]<groupArea[B])return A;
    else return B;
}

void randomSplit(){
    for(int i=0;i<totalCell;i++){
        int goTo = initCellBelong(i);
        //group, grouparea
        groupArea[goTo]+=cellBuffer[i]->area;
        group[goTo].insert(i);
        //cell
        cellBuffer[i]->cellBelongToGroup = goTo;
        //net
        for(auto net=cellBuffer[i]->netsList.begin();net!=cellBuffer[i]->netsList.end();++net){
            int netIndex = *net;
            netBuffer[netIndex]->cellsList[i] = goTo;
            netBuffer[netIndex]->atGroup[goTo].insert(i);
        }
    }
}

void gainInit(bool dbgSignal){
    gain.resize(totalCell);
    gainCategory.clear();
    lockBuffer.clear();
    while (!rec.empty())rec.pop();
    
    maxPort = 0;
    localAccumulateGain = 0;
    for(int cellIndex = 0;cellIndex<totalCell;cellIndex++){
        gain[cellIndex] = 0;
        int cellPort = cellBuffer[cellIndex]->netsList.size();
        maxPort = (maxPort<cellPort)?cellPort:maxPort;
        int F = cellBuffer[cellIndex]->cellBelongToGroup;
        int T = (F==A)?B:A;
        for(auto net = cellBuffer[cellIndex]->netsList.begin();net!=cellBuffer[cellIndex]->netsList.end();++net){
            int Fn = netBuffer[*net]->atGroup[F].size();
            int Tn = netBuffer[*net]->atGroup[T].size();
            if(Fn==1)gain[cellIndex]++;
            if(Tn==0)gain[cellIndex]--;
        }
    }
    for(int posGain = -1*maxPort;posGain<=maxPort;posGain++){
        list<int> gainLs;
        gainLs.push_back(IMPOSSIBLEPORT);//intmin to denothe the base case 
        gainCategory.insert(make_pair(posGain,gainLs));
    }
    for(int cellIndex=0;cellIndex<totalCell;cellIndex++)
        gainCategory[gain[cellIndex]].push_back(cellIndex);
    if(dbgSignal){
        showGroup();
        showGain();
        showGainCategory();
    }
    if(dbgSignal)
        showGain();
}

int whatToSwitch(int atGp){//we can use another strategy to select
    int tmpG = INT_MIN;
    int who = NOPROPERCELL;
    for(auto cell=group[atGp].begin();cell!=group[atGp].end();++cell){
        int F = cellBuffer[*cell]->cellBelongToGroup;
        int T = (F==A)?B:A;
        if((lockBuffer.find(*cell)==lockBuffer.end()) 
        && (groupArea[F]-cellBuffer[*cell]->area >=0) 
        && (groupArea[T] + cellBuffer[*cell]->area <=areaConstrain)){
            if(tmpG<=gain[*cell]){
                tmpG=gain[*cell];
                who=*cell;
            }
        }
    }
    return who;
}
void updateGain(int targetCell,int step,bool dbgSignal){
    int F = cellBuffer[targetCell]->cellBelongToGroup;
    int T = (F==A)?B:A;
    rec.push(targetCell);
    localAccumulateGain += gain[targetCell];
    lockBuffer.insert(targetCell);
    group[F].erase(targetCell);
    group[T].insert(targetCell);
    groupArea[F]-=cellBuffer[targetCell]->area;
    groupArea[T]+=cellBuffer[targetCell]->area;

    cellBuffer[targetCell]->cellBelongToGroup = T;
    cellBuffer[targetCell]->lastLocatedGroup = F;

    for(auto n=cellBuffer[targetCell]->netsList.begin();n!=cellBuffer[targetCell]->netsList.end();++n){
        int Tn = netBuffer[*n]->atGroup[T].size(); 
        int Fn = netBuffer[*n]->atGroup[F].size();
        for(auto cell=netBuffer[*n]->cellsList.begin();cell!=netBuffer[*n]->cellsList.end();++cell){
            if(lockBuffer.find(cell->first)==lockBuffer.end()){
                if(Tn==0){
                    //if(dbgSignal)cout<<"Tn = "<<netBuffer[*n]->atGroup[T].size()<<endl;
                    if(dbgSignal)cout<<">> before "<<" + on cell "<<cell->first<<" at net "<< *n <<endl;
                    gain[cell->first]++;
                }
            }
        }

        if(Tn==1){
            int onlyCellOnT = *netBuffer[*n]->atGroup[T].begin();
            //if(dbgSignal)cout<<"Tn = "<<netBuffer[*n]->atGroup[T].size()<<endl;
            if(lockBuffer.find(onlyCellOnT)==lockBuffer.end()){
                if(dbgSignal)cout<<">> before "<<" - on cell "<< onlyCellOnT <<" at net "<< *n <<endl;
                gain[onlyCellOnT]--;
            }
        }

        netBuffer[*n]->cellsList[targetCell] = F;
        netBuffer[*n]->atGroup[F].erase(targetCell);
        netBuffer[*n]->atGroup[T].insert(targetCell);
        Tn = netBuffer[*n]->atGroup[T].size(); 
        Fn = netBuffer[*n]->atGroup[F].size();

        for(auto cell=netBuffer[*n]->cellsList.begin();cell!=netBuffer[*n]->cellsList.end();++cell){
            if(lockBuffer.find(cell->first)==lockBuffer.end()){
                if(Fn==0){
                    //if(dbgSignal)cout<<"Fn = "<<netBuffer[*n]->atGroup[F].size()<<endl;
                    if(dbgSignal)cout<<">> after "<<" - on cell "<<cell->first<<" at net "<< *n <<endl;
                    gain[cell->first]--;
                }
            }
        }
        if(Fn==1){
            int onlyCellOnF = *netBuffer[*n]->atGroup[F].begin();
            //if(dbgSignal)cout<<"Fn = "<<netBuffer[*n]->atGroup[F].size()<<endl;
            if(lockBuffer.find(onlyCellOnF)==lockBuffer.end()){
                if(dbgSignal)cout<<">> after "<<" + on cell "<<onlyCellOnF<<" group on net = "<<netBuffer[*n]->atGroup[F].size()<<" at net "<< *n <<endl;
                gain[onlyCellOnF]++;
            }
        }
    }
    if(localAccumulateGain>bestAccumulateGain){
        bestAccumulateGain = localAccumulateGain;
        bestTop = targetCell;
    }
    if(dbgSignal)
        for(int i=0;i<gain.size();i++)
            cout<<" "<<gain[i];
    if(dbgSignal)cout<<endl;
    if(dbgSignal){
        cout<<"step "<<lockBuffer.size()<<" update cell on "<<targetCell<<" and its gain = "<<gain[targetCell]<<endl;//<<" current acc: "<<localAccumulateGain << " and best acc: "<<bestAccumulateGain<<endl;
    }
}

void goBack(int targetCell){
    int curAt = cellBuffer[targetCell]->cellBelongToGroup;
    int shouldBe = cellBuffer[targetCell]->lastLocatedGroup;
    group[curAt].erase(targetCell);
    group[shouldBe].insert(targetCell);
    groupArea[curAt]-=cellBuffer[targetCell]->area;
    groupArea[shouldBe]+=cellBuffer[targetCell]->area;

    cellBuffer[targetCell]->cellBelongToGroup = curAt;
    cellBuffer[targetCell]->lastLocatedGroup = -1;

    for(auto n=cellBuffer[targetCell]->netsList.begin();n!=cellBuffer[targetCell]->netsList.end();++n){
        netBuffer[*n]->cellsList[targetCell] = shouldBe;
        netBuffer[*n]->atGroup[curAt].erase(targetCell);
        netBuffer[*n]->atGroup[shouldBe].insert(targetCell);
    }
}

int case01[18];

void fm(int iteration,bool dbgSignal){
    case01[0] = 1;
    case01[1] = 14;
    case01[2] = 10;
    case01[3] = 2;
    case01[4] = 13;
    case01[5] = 7;
    case01[6] = 12;
    case01[7] = 9;
    case01[8] = 5;
    case01[9] = 8;
    case01[10] = 3;
    case01[11] = 0;
    case01[12] = 6;
    case01[13] = 4;
    case01[14] = 11;
    if(iteration>ITERLIMIT)return;
    bool ok=true;
    bestTop = NOPROPERCELL;
    gainInit(HIDEDBG);
    cout<<"before iteration "<<iteration<<" cutCnt: "<<cntCut()<<endl;
    int step=0;
    while (ok){
        if(group[A].size()==totalCell){
            int cellA = whatToSwitch(A);
            if(cellA!=NOPROPERCELL)
                updateGain(cellA,step,HIDEDBG);
            else ok=false;
        }else{
            if(group[B].size()==totalCell){
                int cellB = whatToSwitch(B);
                if(cellB!=NOPROPERCELL)
                    updateGain(cellB,step,HIDEDBG);
                else ok=false;
            }else{
                int cellA = whatToSwitch(A);
                int cellB = whatToSwitch(B);
                if(cellA!=NOPROPERCELL&&cellB!=NOPROPERCELL){
                    if(gain[cellA]>=gain[cellB])
                        updateGain(cellA,step,HIDEDBG);
                    else updateGain(cellB,step,HIDEDBG);
                }else if(cellA!=NOPROPERCELL && cellB==NOPROPERCELL){
                    updateGain(cellA,step,HIDEDBG);
                }else if(cellA==NOPROPERCELL && cellB!=NOPROPERCELL){
                    updateGain(cellB,step,HIDEDBG);
                }else{
                    ok = false;
                }
            }
        }
        int tmp = cntCut();
        //cout << "best partial sum: " << bestAccumulateGain<<" and cur cut: "<< tmp <<endl;
        if(tmp<curBest){
            curBest = tmp;
            bestGroup.clear();
            bestGroup = group;
            cout << "best partial sum: " << bestAccumulateGain<<" and cur cut: "<< curBest <<endl;
        }
        step++;
    }
    if(bestAccumulateGain>0){
        while (!rec.empty()&&rec.top()!=bestTop){
            int cell = rec.top();
            rec.pop();
            goBack(cell);
        }
        cout<<"after iteration "<<iteration<<" cutCnt: "<<cntCut()<<" ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
        fm(iteration+1,dbgSignal);    
    }
}
int main(int argc, char* argv[]){
    ifs.open(argv[1]);
    ofs.open(argv[2]);
    cout<<argv[1]<<" and "<<argv[1]<<endl;
    parseInput(HIDEDBG);
    estGroup();
    randomSplit();
    curBest = INT_MAX;
    bestAccumulateGain = 0;
    fm(0,HIDEDBG);
    ofs<<curBest<<endl;
    ofs<<numGroup<<endl;
    vector<set<int>>tmp;
    for(int i=0;i<numGroup;i++){
        set<int> st;
        tmp.push_back(st);
    }
    for(auto i=0;i<totalCell;i++){
        if(group[A].find(i)!=group[A].end())ofs<<A<<endl;
        else ofs<<B<<endl;
    }
    cout<<curBest<<endl;
}