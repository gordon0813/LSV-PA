#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#define ANDTYPE 0
#define XORTYPE 1
#define MUXTYPE 2
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
    NodeInfo(int id){nodeId=id;}
    int nodeid(){return nodeId;}
    int addMatchComponent(Component* c){matchComps.push_back(c);}
    std::string ComponentColor();
    private:
    int nodeId;
    std::vector<Component*> matchComps;
};
class Collection{
    public:
    Collection(Gia_Man_t * p){
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
    int  detectXor(){
        int i,id;
        int count=0;
        Gia_Obj_t  *pobj, * pFan0, * pFan1; 
        pobj=getobj(id);
        Gia_ManForEachObj(giacir,pobj,i){
            if(Gia_ObjRecognizeExor(pobj, &pFan0, &pFan1)){
                std::cout<<"xor"<<id<<"\n";
                Component * xgate=new CompXor();
                xgate->push_input(pFan0);
                xgate->push_input(pFan1);
                xgate->push_output(pobj);
                setInfo(pFan0,xgate);
                setInfo(pFan1,xgate);
                setInfo(pobj,xgate);
                count++;
            }
        } 
        return count ;
    }
    private:
    Gia_Obj_t  * getobj(int id){return Gia_ManObj(giacir,id);}
    int setInfo(Gia_Obj_t  *n,Component * matchcomp){
       ninfos[ Gia_ObjId(giacir,n) ]->addMatchComponent(matchcomp);
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