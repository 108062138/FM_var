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

bool cmpCellIndex(const Cell* a,const Cell* b) {
    return ((*a).cellIndex < (*b).cellIndex);
}

using namespace std;
string dotnet,dotcell;
vector<set<int>> group;
vector<int> grouparea;
vector<Net*> netBuffer;
vector<Cell*>cellBuffer;
int totalCell,totalNet;
int mycost,mygp;
int limit;
ifstream a;
ifstream b;

void parsea(){
    //parse a
    a >> limit;
    a >> dotcell;
    a >> totalCell;
    for(int i=0;i<totalCell;i++){
        int index,area;
        a>> index>>area;
        Cell* cl = new Cell(index,area);
        cellBuffer.push_back(cl);
    }
    //sort
    sort(cellBuffer.begin(), cellBuffer.end(), cmpCellIndex);
    a>>dotnet;
    a>>totalNet;
    for(int i=0;i<totalNet;i++){
        int netconnect;
        Net* nt = new Net(i);
        for(int i=0;i<mygp;i++){
            unordered_set<int> us;
            nt->atGroup.push_back(us);
        }
        a>>netconnect;
        int cell;
        for(int j=0;j<netconnect;j++){
            a>>cell;
            nt->cellsList.insert(make_pair(cell,-1));
            cellBuffer[cell]->netsList.insert(i);
        }
        netBuffer.push_back(nt);
    }
}
void parseb(){
    int belong;
    cout<<group.size()<<endl;
    for(int i=0;i<totalCell;i++){
        b>>belong;
        group[belong].insert(i);
        cellBuffer[i]->cellBelongToGroup = belong;
        for(auto net=cellBuffer[i]->netsList.begin();net!=cellBuffer[i]->netsList.end();++net){
            int netIndex =*net;
            netBuffer[netIndex]->atGroup[belong].insert(i);
        }
    }
}

void check(){
    for(auto &x:grouparea)x=0;
    for(int i=0;i<totalCell;i++){
        int belong = cellBuffer[i]->cellBelongToGroup;
        int area = cellBuffer[i]->area;
        grouparea[belong]+=area;
    }
    for(auto &x:grouparea){
        if(x>limit){
            cout<<"area: "<<x<<" and limit "<<limit<<endl;
            assert(0);
        }
    }
    int realcost=0;
    for(auto n = 0;n<totalNet;n++){
        int con=0;
        for(auto x=netBuffer[n]->atGroup.begin();x!=netBuffer[n]->atGroup.end();++x){
            if((*x).size()>0)con++;
        }
        realcost+=((con-1)*(con-1));
    }
    if(realcost!=mycost){
        cout<<"real cost: "<<realcost<<endl;
        assert(0);
    }
}
void initgroup(){
    group.clear();grouparea.clear();
    for(int i=0;i<mygp;i++){
        set<int> tmp;
        group.push_back(tmp);
        grouparea.push_back(0);
    }
}
int main(int argc, char* argv[]){
    a.open(argv[1]);
    b.open(argv[2]);
    //parse cost and #group from b
    b>>mycost;
    b>>mygp;
    cout<<"cost: "<<mycost<<" and #gp "<<mygp<<endl;
    initgroup();
    parsea();
    parseb();
    check();
    cout<<"OK"<<endl;
}