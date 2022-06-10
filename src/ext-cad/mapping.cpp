#include "ext-cad/cada.h"

void Collection::showInfo(std::string filename){
        Gia_Obj_t * pobj;
        int i;
        std::ofstream myfile;
        myfile.open(filename);
        myfile<<"digraph G1 {\n";
        Gia_ManForEachObj(giacir,pobj,i){
            if(Gia_ObjIsCo(pobj)){
                myfile<<" a"<<Gia_ObjId(giacir,Gia_ObjFanin0(pobj))<<" -> "<<" a"<<Gia_ObjId(giacir,pobj)<<edgeFeature(pobj,Gia_ObjFaninC0(pobj))<<";\n";
            }else if(Gia_ObjIsAnd(pobj)){
                myfile<<" a"<<Gia_ObjId(giacir,Gia_ObjFanin0(pobj))<<" -> "<<" a"<<Gia_ObjId(giacir,pobj)<<edgeFeature(pobj,Gia_ObjFaninC0(pobj))<<";\n";
                myfile<<" a"<<Gia_ObjId(giacir,Gia_ObjFanin1(pobj))<<" -> "<<" a"<<Gia_ObjId(giacir,pobj)<<edgeFeature(pobj,Gia_ObjFaninC0(pobj))<<";\n";
            }
        }
        myfile<<"}\n";
        myfile.close();
    }
 std::string   NodeInfo::ComponentColor(){
     int match_count=0;
     std::stringstream ss;
     ss<<",color= \"black";
     for (int i=0;i<matchComps.size();i++){
         ss<<":"<<matchComps[i]->thecolor();
     }
     ss<<"\"";
     return ss.str();
 }