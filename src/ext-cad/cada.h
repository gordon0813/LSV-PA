#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <bitset>
#include <map>

#define ANDTYPE 0
#define XORTYPE 1
#define MUXTYPE 2
using namespace std;

void Lsv_NtkPrintNodes(Abc_Ntk_t *pNtk);
void CadA(Abc_Ntk_t *pNtk, std::string ofile);

class Component
{
public:
    Component() { mark = 0; }
    int push_input(Gia_Obj_t *n) { inNodes.push_back(n); };
    int push_output(Gia_Obj_t *n) { outNodes.push_back(n); };
    int push_obj(Gia_Obj_t *n){};
    virtual int thetype() { return ANDTYPE; }
    virtual std::string thecolor() { return "black"; }

    std::vector<Gia_Obj_t *> containNodes;
    std::vector<Gia_Obj_t *> inNodes;
    std::vector<Gia_Obj_t *> outNodes;
    int mark;
};

class CompXor : public Component
{
    virtual int thetype() override { return XORTYPE; }
    virtual std::string thecolor() override { return "red"; }
};


class NodeInfo
{
public:
    NodeInfo(int id)
    {
        nodeId = id;
        fix = 0;
        for(int i=0;i<4;i++){simvalueSave.push_back(0);}
        depmark=0;
        mark=0;
        travelmark=0;
    }
    int nodeid() { return nodeId; }
    int addMatchComponent(Component *c) { matchComps.push_back(c); }
    std::string ComponentColor();
    int setSim(int v)
    {
        if (fix == 0)
            simValue = v;
        return fix;
    };
    int lock() { fix = 1; }
    int islock() { return fix; }
    int unlock() { fix = 0; }
    void  unmarkAll(){travelmark=0;mark=0;depmark=0;}
    int savesim(int i) { simvalueSave[i] = simValue; }
    int xorChainCandidate() { return (simValue ^ simvalueSave[0]) == ~0; }
    int unchangeOne(){  return (simvalueSave[0]==simvalueSave[1] &&simvalueSave[2]==simvalueSave[3]);}
    int oneInvoneUnc(){
        if (simvalueSave[0]==simvalueSave[1]&&simvalueSave[2]==~simvalueSave[3]) return 2;
        else if(simvalueSave[0]==~simvalueSave[1]&&simvalueSave[2]==simvalueSave[3])return 1;
        else return 0;
    }
    uint simValue;
    vector<int> inxor;
    vector<int> outxor;
    //static int markc;
    int depmark;
    size_t mark;
    size_t travelmark;
    short iscut;
private:
    vector<uint>simvalueSave;
    //int simvalueSave;
    short fix;
    
    int nodeId;
    CompXor bigx;
    

    std::vector<Component *> matchComps;
};
//1: we have gia circuit ,and we have pi words(Collection::allwords)
//2: we random sim the gia circuit(Collection::randomCi ,Collection::simall) ,save simvalue in nodeInfo(Collection::ninfos)
//3: start build word level info and sim (example c1= a+b ,c2=a*b ,...,c1->simModule(),c2->simModule()....)
//4: hash matching simvalue to giagate,link the Word to Nodeinfo/gianode (may back to 2 to sim more)
//5: if POs is linked  start write ,or back to 3 with more word module(c4=c1*c2)
 
class Word{
    public:
    Word(){iscut=0;isconstant=0;ctype="";};
    Word(int nbits);
    int addbit(NodeInfo* ni,int nthbit);
    int check();
    double match(Word* bigger);
    //need some rule to check c's bits number (+1 bits is safe but may redundent)

    Word* add(Word* b){
        Word* c=new Word(max(word.size(),b->word.size())+1);
        c->type="+"; c->input.push_back(this) ; c->input.push_back(b) ; 
        return c;
    };//example return  c=a+b
    Word* sub(Word*b);
    Word* mult(Word* b);
    Word* rshfit(Word* b);
    Word* lshfit(Word* b);
    int simModule();//simulation by word level info (get info from fanin matched gate ,if input not matched yet ,recur call)
    int set2Incut();
    void setName(string n){this->name=n;};
    string getName(){return this->name;};
    int isRedundent();
    int candidatesI(vector<Word*>&allc);
    string functionStr();
    vector<uint> getsimValue();
    int backAssignIn(size_t n);
    Word* isconstTrace();
    int assignNumber(size_t number,int bit);
    long getNumber(int bit);
    int nbits(){return word.size();}
    uint simvalue(int nthbits){return simvalues[nthbits];}
    size_t size() {return word.size();}
    void set2const(long n){this->isconstant=true;constantValue=n;iscut=true;}
    private:
    
    
    vector<NodeInfo*> word; // link to real gia circuit(Nodeinfo contant gia simvalue ,gia id ,xor or other info) ,if not yet matched :nullptr
    vector<bool>isinv;   // link to  gia but maybe inverse
    vector<uint>simvalues;// formal/guess simulation value
    string type; // "*"  or "&" or "==" or "+" .....
    string ctype;
    string name;
    vector<Word*>input;  // input = [wa ,wb] ,type= "+", means word=wa+wb 
    size_t mark; //check if sim value is newest;
    short isconstant;// this is constant ? if yes simvalues will fix
    long constantValue;
    short iscut;
};
class Collection
{
public:
    Collection(Gia_Man_t *pp)
    {
        Gia_Man_t *p = Gia_ManDupOrderDfs(pp);
        int totalnum = Gia_ManObjNum(p);
        this->giacir = p;
        int i;
        for (i = 0; i < totalnum; i++)
        {
            ninfos.push_back(nullptr);
        }
        Gia_Obj_t *pobj;

        dfs = Gia_ManCollectAndsAll(p);
        Gia_ManForEachObj(p, pobj, i)
        {
            ninfos[i] = new NodeInfo(Gia_ObjId(p, pobj));
        }
    }
    void showInfo(std::string filename);
    //beta test---------
    int detectXor();
    int createFaninWords(vector<string>namecis);
    int createFanoutWords(vector<string>namecos);
    int simAndMatch();
    int simMatchPair(Word* wtarget,Word* guess);


    //--------------------
    int randomCi(){
        Gia_Obj_t *pobj;
        int i;
        // randon input
        Gia_ManForEachCi(giacir, pobj, i)
        {
            NodeInfo *ci = this->getInfo(pobj);
            // cout <<Gia_ObjId(giacir,pobj)<<endl;
            ci->setSim(rand());
        }
    }
    int simall(){
         Gia_Obj_t *pobj;
        int i;
        Gia_ManForEachObj(giacir, pobj, i)
        {
            if (Gia_ObjIsAnd(pobj))
            {
                simNode(pobj);
            }else if(Gia_ObjIsCo(pobj)){
                simPO(pobj);
            }
        }
    }
    int saveall(int k){
         Gia_Obj_t *pobj;
        int i;
        Gia_ManForEachObj(giacir, pobj, i)
        {
            getInfo(pobj)->savesim(k);
        }
    }
    int xorChain(Gia_Obj_t *psa)
    {
        Gia_Obj_t *pobj;
        int i;
        int sa01 = 0;
        // randon input
        NodeInfo *san = getInfo(psa);
        san->setSim(sa01);
        san->lock();
        randomCi();
        simall();
        san->unlock();
        // save sa0 value
        saveall(0);
        sa01 = ~0;
        san->setSim(sa01);
        san->lock();
        simall();
        /**
        Gia_ManForEachObj(giacir,pobj,i){
            a=getInfo(pobj)->simValue;
            bitset<32> x(a);
            cout <<Gia_ObjId(giacir,pobj)<<" "<< x <<endl;

        }**/
        san->unlock();
        cout << Gia_ObjId(giacir, psa) << " : ";
       
        NodeInfo* parent;

        Gia_ManForEachObj(giacir, pobj, i)
        {
            parent=getInfo(pobj);
            if(parent->nodeid()!=san->nodeid()){
                
                if (getInfo(pobj)->xorChainCandidate()){
                    parent->inxor.push_back(san->nodeid());
                    san->outxor.push_back(parent->nodeid());
                    cout << Gia_ObjId(giacir, pobj) << " ";
                }
            }
            
            //if (getInfo(pobj)->xorChainCandidate())
               // cout << Gia_ObjId(giacir, pobj) << " ";

            // cout <<Gia_ObjId(giacir,pobj)<<" "<< getInfo(pobj)->xorChainCandidate()<<endl;
        }
        
        cout << endl;
        return 0;
    }
    int carryChain(Gia_Obj_t *ia,Gia_Obj_t *ib,Gia_Obj_t *out){
         Gia_Obj_t *pobj;
        int i;
        int sa01 = 0;
        int sb01 = 0;
        // randon input
        NodeInfo *sa = getInfo(ia);
        NodeInfo *sb = getInfo(ib);
        NodeInfo *so = getInfo(out);
        sa->setSim(sa01);
        sa->lock();
        sb->setSim(sb01);
        sb->lock();
        randomCi();
        simall();
        saveall(0);
        sa->unlock();
        sb->unlock();
        sa->setSim(~sa01);
        sa->lock();
        sb->setSim(~sb01);
        sb->lock();
        //randomCi();
        simall();
        saveall(1);
        sa->unlock();
        sb->unlock();
        sb01 = ~0;
        sa->setSim(sa01);
        sa->lock();
        sb->setSim(sb01);
        sb->lock();
        randomCi();
        simall();
        saveall(2);
        sa->unlock();
        sb->unlock();
        sa->setSim(~sa01);
        sa->lock();
        sb->setSim(~sb01);
        sb->lock();
        //randomCi();
        simall();
        saveall(3);
        sa->unlock();
        sb->unlock();
        cout<<sa->nodeid()<<" pair "<<sb->nodeid()<<" final "<<so->unchangeOne();
        int check=so->unchangeOne();
        if(check){
            Gia_ManForEachObj(giacir, pobj, i){
                if(getInfo(pobj)->oneInvoneUnc()){
                    cout<<" "<<getInfo(pobj)->nodeid();
                }
            }
        }
        cout<<endl;

        return 0;

    }
    int xorChainAll()
    {
        Gia_Obj_t *pobj;
        int i;
        Gia_ManForEachObj(giacir, pobj, i)
        {
            xorChain(pobj);
        }
        NodeInfo* ni;
        Gia_ManForEachObj(giacir, pobj, i){
            ni=getInfo(pobj);
            cout<<ni->nodeid();
            for(int k=0;k<ni->inxor.size();k++){
                cout<<" <- "<<ni->inxor[k];
            }
            cout<<endl;
            for(int k=0;k<ni->inxor.size();k++){
                for(int ki=k;ki<ni->inxor.size();ki++){
                    carryChain(Gia_ManObj(giacir,ni->inxor[k]),Gia_ManObj(giacir,ni->inxor[ki]),pobj);
                }
            }
        }
       
    }
    int travel(NodeInfo *info){info->travelmark=travelm;}
    int istraveled(NodeInfo *info){return info->travelmark==travelm;}
    int endtravel(){travelm++;}
    int mark(NodeInfo *info,int layer){
        
        if(info->mark==marks[info->depmark] ){
            return 0;
        }else{
            info->mark=marks[layer];
            info->depmark=layer;
            return 1;
        }
    }
    void unmark(int layer){
       
        marks[layer]++;
    }
    int ismark(NodeInfo *info){
       
        return info->mark==marks[info->depmark];
    }
    void unmarkAll(int layer){
        marks.clear();
        for(int i=0 ;i<ninfos.size();i++){
            ninfos[i]->unmarkAll();
        }
        for(int i=0;i<layer+1;i++)marks.push_back(1);
        travelm=0;
    }
    int validcut(vector<NodeInfo*>&fanincone,vector<NodeInfo*>& cut){
        //from=fanincone.size()-1;
        //int count=0;
        endtravel();
        count=0;
        for(int i=0;i<cut.size();i++){cut[i]->iscut=1;}
        Gia_ManMarkCheck_rec(giacir,Gia_ManObj(giacir,fanincone.back()->nodeid()),0);
        for(int i=0;i<cut.size();i++){cut[i]->iscut=0;}
       // cout<<"count:"<<(count)<<endl;
        return count!=0;
    }
    int incutrecur(vector<NodeInfo*>&fanincone, int now,int baseLayer,vector<NodeInfo*>&cut,vector<vector<NodeInfo*> > &allcut,int k,vector<NodeInfo* > &allci){
        int depth=cut.size()+baseLayer;
        for(int i=now;i>=0;i--){
            if(  ismark(  fanincone[i]))continue;

            cut.push_back(fanincone[i]);
           // for(int i=0;i<fanincone.size();i++)cout<<"address: "<<fanincone[i]<<endl;
            assert(fanincone[i]->nodeid()<10000 );
            
            Gia_ManMarkCone_rec(giacir, Gia_ManObj(giacir,fanincone[i]->nodeid()),depth+1,cut,0);
            if(cut.size()==k ){
                if(validcut(fanincone,cut)){
                    vector<NodeInfo*> cc=cut;
                    cout<<"cut:";
                    for(int c=0;c<cut.size();c++)cout<<cut[c]->nodeid()<<" ";
                    cout<<endl;
                    allcut.push_back(cc);
                    cout<<"cut is valid\n";
                }
                
                cut.pop_back();
                unmark(depth+1);
                
                continue;
            }
            incutrecur(fanincone,i,baseLayer,cut,allcut,k,allci);
            unmark(depth+1);
            cut.pop_back();
           // mark(fanincone[i],depth+1);
        }
        return 0;
    }
    int incut(){
        int k=3;
        Gia_Obj_t *pobj;
        int i;
       // incut(ninfos[10],k);
       // incut(ninfos[10],k);
        //incut(ninfos[10],k);
        unlockAll();
        cout<<"for node:11"<<endl;
        incut(getInfo(Gia_ManObj(giacir, 11)),k);
        Gia_ManLockCone_rec(giacir,Gia_ManObj(giacir, 11));
        cout<<"for node:26"<<endl;
        incut(getInfo(Gia_ManObj(giacir, 26)),k);
        Gia_ManLockCone_rec(giacir,Gia_ManObj(giacir, 26));
         cout<<"for node:46"<<endl;
        incut(getInfo(Gia_ManObj(giacir, 46)),k);
        Gia_ManLockCone_rec(giacir,Gia_ManObj(giacir, 46));
       
        unlockAll();
        return 0;
        Gia_ManForEachObj(giacir, pobj, i)
        {
            if(i<0)continue;
            if(i>200)break;
            if (Gia_ObjIsAnd(pobj)){
                cout<<"for node:"<<getInfo(pobj)->nodeid()<<endl;
                incut(getInfo(pobj),k);

            }
            
        }
        return 0;
        
    }
    int incut(NodeInfo *info,int k){// (NodeInfo *info,int k,vector<vector<NodeInfo*> > &allcut,vector<NodeInfo*>&cut){
        vector<NodeInfo*>fanincone;
        vector<NodeInfo*>allci;
        vector<NodeInfo*>cut;
        vector<vector<NodeInfo*> > allcut;
        Gia_Obj_t* p;
        int i;
       
        unmarkAll(k);
        unmark(0);
        Gia_ManMarkCone_rec(giacir,Gia_ManObj(giacir, info->nodeid()),0,fanincone,1);
        //for(int i=0;i<fanincone.size();i++)cout<<'address '<<fanincone[i]<<endl;
        if(fanincone.size()<k)return 0;
        unmark(0);
        incutrecur(fanincone,fanincone.size()-1,0,cut,allcut,k,allci);
        unmark(0);
        allcut.clear();
        cut.clear();
        fanincone.clear();
        /**
        cout<<"check cut\n";
        unmark(0);
        Gia_ManMarkCone_rec(giacir,Gia_ManObj(giacir, 38),0);
        cout<<"\ncheck cut\n";
        unmark(1);
        Gia_ManMarkCone_rec(giacir,Gia_ManObj(giacir, 47),1);
        cout<<"\ncheck cut\n";
        //unmark(1);
        Gia_ManMarkCone_rec(giacir,Gia_ManObj(giacir, 62),1);
        cout<<"\ncheck cut\n";
        //unmark(1);
        Gia_ManMarkCone_rec(giacir,Gia_ManObj(giacir, 62),1);
        cout<<"\ncheck cut\n";
        unmark(1);
        Gia_ManMarkCone_rec(giacir,Gia_ManObj(giacir, 62),1);
        cout<<"\ncheck cut\n";
        unmark(0);
        unmark(1);
        Gia_ManMarkCone_rec(giacir,Gia_ManObj(giacir, 62),1);**/
        return 0;
    }
    void CadaOutput(const string& fileNameIn, const string& fileNameOut) {
        if (outputFunctions.size() == allCOwords.size()) {
            // completely matched
            fstream fout(fileNameOut, ios::out);
            const string tabStr = "    "; 

            fout << "module top(";
            for (const auto& wcpcpy: allwords) {
                fout << wcpcpy->getName() << ", ";
            }
            for (const auto& w: allCOwords) {
                if (&w != &allCOwords[0]) {
                    cout << ", ";
                }
                fout << w->getName();
            }
            fout << ");" <<endl;

            
            for (const auto& w: allwords) {
                fout << tabStr << "input [" << w->size() - 1 << ":" << "0] "
                    << w->getName() << ";" << endl;
            }
            for (const auto& w: allCOwords) {
                fout << tabStr << "output [" << w->size() - 1 << ":" << "0] "
                    << w->getName() << ";" << endl;
            }
            for (const auto& fn: outputFunctions) {
                fout << tabStr << fn.second << endl;
            }
            fout << "endmodule" << endl;
            fout.close();
        } else {
            // match failed: dump input to output
            ifstream fin(fileNameIn, ios::binary);
            ofstream fout(fileNameOut, ios::binary);
            fout << fin.rdbuf();
            fin.close();
            fout.close();
        }
    }
    vector<Word*> allWordModules(const vector<Word*>&as,const vector<Word*>&bs,int samelevel);
private:
    void unlockAll(){for(int i=0;i<ninfos.size();i++){ninfos[i]->unlock();}  }
    Gia_Obj_t *getobj(int id) { return Gia_ManObj(giacir, id); }
    int Gia_ManMarkCone_rec( Gia_Man_t * p, Gia_Obj_t * pObj,int d ,vector<NodeInfo*>& cone,bool need)
    {
        NodeInfo* ni=getInfo(pObj);
       // cout<<" "<<ni->nodeid();
        if(ni->islock())return 0;
        if(ismark(ni))return 0;
        mark(ni,d);
        
       // giaInfo(pObj)->dmark(int d);
       // if ( Gia_ObjIsTravIdCurrent(p, pObj) )
            //return 0;
        //Gia_ObjSetTravIdCurrent(p, pObj);
        if ( Gia_ObjIsCi(pObj) ){
            if(need)cone.push_back(ni);
            return 0;
        }
            
        assert( Gia_ObjIsAnd(pObj) );
        Gia_ManMarkCone_rec( p, Gia_ObjFanin0(pObj),d,cone,need );
        Gia_ManMarkCone_rec( p, Gia_ObjFanin1(pObj) ,d,cone,need );
        if(need)cone.push_back(ni);
        return 1;
    }
    int Gia_ManMarkCheck_rec( Gia_Man_t * p, Gia_Obj_t * pObj,int limit)
    {
        NodeInfo* ni=getInfo(pObj);
        //cout<<" "<<ni->nodeid();
        if(istraveled(ni))return 0;
        travel(ni);
        if(ni->islock())return 0;
        
        if(count>limit)return 0;
       // giaInfo(pObj)->dmark(int d);
       // if ( Gia_ObjIsTravIdCurrent(p, pObj) )
            //return 0;
        //Gia_ObjSetTravIdCurrent(p, pObj);
        if (( Gia_ObjIsCi(pObj) &&  (!ni->iscut))  || (!ismark(ni)) ){
            count=limit+1;
            return 0;
        }
            
        assert( Gia_ObjIsAnd(pObj) );
        Gia_ManMarkCheck_rec( p, Gia_ObjFanin0(pObj),limit );
        Gia_ManMarkCheck_rec( p, Gia_ObjFanin1(pObj) ,limit );
        
        return 1;
    }
    int Gia_ManLockCone_rec( Gia_Man_t * p, Gia_Obj_t * pObj)
    {
        NodeInfo* ni=getInfo(pObj);
        //cout<<" "<<ni->nodeid();
        if(istraveled(ni))return 0;
        travel(ni);
        ni->lock();
        
       // giaInfo(pObj)->dmark(int d);
       // if ( Gia_ObjIsTravIdCurrent(p, pObj) )
            //return 0;
        //Gia_ObjSetTravIdCurrent(p, pObj);
        if ( Gia_ObjIsCi(pObj) ){
            
            return 0;
        }
            
        assert( Gia_ObjIsAnd(pObj) );
        Gia_ManLockCone_rec( p, Gia_ObjFanin0(pObj) );
        Gia_ManLockCone_rec( p, Gia_ObjFanin1(pObj));
        
        return 1;
    }
    
    void Gia_ManMarkfo_rec( Gia_Man_t * p, int iObj, Vec_Int_t * vNodes )
{
    Gia_Obj_t * pObj; int i, iFan;
    if ( Gia_ObjIsTravIdCurrentId(p, iObj) )
        return;
    Gia_ObjSetTravIdCurrentId(p, iObj);
    pObj = Gia_ManObj( p, iObj );
    if ( Gia_ObjIsCo(pObj) )
        return;
    assert( Gia_ObjIsAnd(pObj) );
    Gia_ObjForEachFanoutStaticId( p, iObj, iFan, i )
        Gia_ManMarkfo_rec( p, iFan, vNodes );
    Vec_IntPush( vNodes, iObj );
}
    int setInfo(Gia_Obj_t *n, Component *matchcomp)
    {
        ninfos[Gia_ObjId(giacir, n)]->addMatchComponent(matchcomp); return 0;
    }
    NodeInfo *giaInfo(Gia_Obj_t *n) { return ninfos[Gia_ObjId(giacir, n)]; }
    int simPO(Gia_Obj_t *thenode){
        NodeInfo *info = giaInfo(thenode);
        NodeInfo *linfo = giaInfo(Gia_ObjFanin0(thenode));
        int linv = Gia_ObjFaninC0(thenode);
        int v=(linv == 0) ? (linfo->simValue) : (~linfo->simValue);
        info->setSim(v);
        return 0;
    }
    int simNode(Gia_Obj_t *thenode)
    {
        // Gia_Obj_t  * thenode=getobj(id);
        NodeInfo *info = giaInfo(thenode);
        NodeInfo *linfo = giaInfo(Gia_ObjFanin0(thenode));
        int linv = Gia_ObjFaninC0(thenode);
        NodeInfo *rinfo = giaInfo(Gia_ObjFanin1(thenode));
        int rinv = Gia_ObjFaninC1(thenode);
        // cout<<info->nodeid()<<" "<<linv<<" "<<rinv<<endl;
        int v = ((linv == 0) ? (linfo->simValue) : (~linfo->simValue)) & ((rinv == 0) ? (rinfo->simValue) : (~rinfo->simValue));
        info->setSim(v);
        return 0;
    }
    NodeInfo *getInfo(Gia_Obj_t *n) { return ninfos[Gia_ObjId(giacir, n)]; }
    std::string edgeFeature(Gia_Obj_t *n, int fi)
    {
        std::stringstream ss;
        ss << "[ label=\"\" ";
        if (fi)
        {
            ss << " ,style=dotted";
        }
        ss << getInfo(n)->ComponentColor();
        ss << "]";
        return ss.str();
    }
    Gia_Man_t *giacir;
    Vec_Int_t *dfs;
    std::vector<NodeInfo *> ninfos;
    vector<Word*>allwords;
    vector<Word*>allCOwords;
    map<int, string> outputFunctions;
    vector<size_t> marks;
    size_t travelm;
    int count;
};



