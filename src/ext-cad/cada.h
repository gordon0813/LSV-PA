#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <bitset>

#define ANDTYPE 0
#define XORTYPE 1
#define MUXTYPE 2
using namespace std;
void Lsv_NtkPrintNodes(Abc_Ntk_t*  pNtk) ;
void CadA(Abc_Ntk_t*  pNtk ,std::string ofile);

class Component{
    public:
    Component(){mark=0;}
    int push_input(Gia_Obj_t  * n){inNodes.push_back(n);};
    int push_output(Gia_Obj_t  * n){outNodes.push_back(n);};
    int push_obj(Gia_Obj_t  * n){};
    virtual int thetype(){return ANDTYPE;} 
    virtual std::string thecolor(){return "black";} 
    private:
    std::vector<Gia_Obj_t  *>containNodes;
    std::vector<Gia_Obj_t  *>inNodes;
    std::vector<Gia_Obj_t  *>outNodes;
    int mark;
};

class CompXor :public Component{
    virtual int thetype() override{return XORTYPE;} 
    virtual std::string thecolor()override{return "red";} 
};


class NodeInfo{
    public:
    NodeInfo(int id){nodeId=id;fix=0;}
    int nodeid(){return nodeId;}
    int addMatchComponent(Component* c){matchComps.push_back(c);}
    std::string ComponentColor();
    int setSim(int v){if(fix==0)simValue=v; return fix;};
    int lock(){fix=1;}
    int unlock(){fix=0;}
    int savesim(){simvalueSave=simValue;}
    int xorChainCandidate(){return (simValue^simvalueSave)==~0; }
    int simValue;
    private:
    int simvalueSave;
    short fix;

    int nodeId;
    
    std::vector<Component*> matchComps;
};
class Collection{
    public:
    Collection(Gia_Man_t * pp){
        Gia_Man_t *p=Gia_ManDupOrderDfs(pp);
        int totalnum=Gia_ManObjNum(p);
        this->giacir=p;
        int i;
        for ( i=0;i<totalnum;i++){ninfos.push_back(nullptr);}
        Gia_Obj_t * pobj;
        
        dfs=      Gia_ManCollectAndsAll(  p );
        Gia_ManForEachObj(p,pobj,i){
            ninfos[i]=new NodeInfo(Gia_ObjId(p,pobj));
            
        }
    }
    void showInfo(std::string filename);
    int  detectXor();
    int randomSim();
   
    int xorChain(Gia_Obj_t * psa){
    Gia_Obj_t * pobj;
    int i;
    int sa01=0;
    //randon input
    NodeInfo * san=getInfo(psa);
    san->setSim(sa01);
    san->lock();
    Gia_ManForEachCi(giacir,pobj,i){
        NodeInfo* ci= this->getInfo(pobj);
       // cout <<Gia_ObjId(giacir,pobj)<<endl;
        ci->setSim(rand());
    }
    Gia_ManForEachObj(giacir,pobj,i){
        if(Gia_ObjIsAnd(pobj)){
            simNode(pobj);
        }
    }
    int a=0;
        
    /**  
    Gia_ManForEachObj(giacir,pobj,i){
        a=getInfo(pobj)->simValue;
        bitset<32> x(a);
        cout <<Gia_ObjId(giacir,pobj)<<" "<< x <<endl;

    }**/
    san->unlock();
    //save sa0 value
    Gia_ManForEachObj(giacir,pobj,i){
        getInfo(pobj)->savesim();
    }
    sa01=~0;
    san->setSim(sa01);
    san->lock();
    Gia_ManForEachObj(giacir,pobj,i){
        if(Gia_ObjIsAnd(pobj)){
            simNode(pobj);
        }
    }
    /**
    Gia_ManForEachObj(giacir,pobj,i){
        a=getInfo(pobj)->simValue;
        bitset<32> x(a);
        cout <<Gia_ObjId(giacir,pobj)<<" "<< x <<endl;

    }**/
    san->unlock();
    cout<<Gia_ObjId(giacir,psa)<<" : ";
    Gia_ManForEachObj(giacir,pobj,i){
        if(getInfo(pobj)->xorChainCandidate())cout<<Gia_ObjId(giacir,pobj)<<" ";
        //cout <<Gia_ObjId(giacir,pobj)<<" "<< getInfo(pobj)->xorChainCandidate()<<endl;
    }
    cout<<endl;
    }
    int xorChainAll(){
        Gia_Obj_t * pobj;
        int i;
        Gia_ManForEachObj(giacir,pobj,i){
            xorChain(pobj);
        }
    }



    private:
    Gia_Obj_t  * getobj(int id){return Gia_ManObj(giacir,id);}
    int setInfo(Gia_Obj_t  *n,Component * matchcomp){
       ninfos[ Gia_ObjId(giacir,n) ]->addMatchComponent(matchcomp);
    }
    NodeInfo* giaInfo(Gia_Obj_t  *n){return ninfos[Gia_ObjId(giacir,n)];}
    int simNode(Gia_Obj_t  * thenode){
       // Gia_Obj_t  * thenode=getobj(id);
        NodeInfo* info=giaInfo(thenode);
        NodeInfo* linfo=giaInfo(Gia_ObjFanin0(thenode));
        int linv=Gia_ObjFaninC0(thenode);
        NodeInfo* rinfo=giaInfo(Gia_ObjFanin1(thenode));
        int rinv=Gia_ObjFaninC1(thenode);
       // cout<<info->nodeid()<<" "<<linv<<" "<<rinv<<endl;
        int v= ((linv==0)?(linfo->simValue):(~linfo->simValue) )& ((rinv==0)?(rinfo->simValue):(~rinfo->simValue));
        info->setSim(v);
    }
    NodeInfo* getInfo(Gia_Obj_t  *n){return ninfos[ Gia_ObjId(giacir,n) ]; }
    std::string edgeFeature(Gia_Obj_t  *n,int fi){
        std::stringstream ss;
        ss<<"[ label=\"\" ";
        if(fi){ss<<" ,style=dotted";}
        ss<<getInfo(n)->ComponentColor();
       ss<<"]";
       return ss.str();
    }
    Gia_Man_t *  giacir;
    Vec_Int_t *   dfs;
    std::vector<NodeInfo*>ninfos;

};