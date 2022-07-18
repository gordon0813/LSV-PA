#include <unordered_map>
#include "ext-cad/cada.h"

using namespace std;
int  Collection::detectXor(){
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
void Collection::showInfo(std::string filename){

        
       // xorChainAll();
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
                myfile<<" a"<<Gia_ObjId(giacir,Gia_ObjFanin1(pobj))<<" -> "<<" a"<<Gia_ObjId(giacir,pobj)<<edgeFeature(pobj,Gia_ObjFaninC1(pobj))<<";\n";
            }
        }
        myfile<<"}\n";
        myfile.close();
    }
    /**
int  Collection::xorChain(){
    Gia_Obj_t * pobj;
    int i;
    //randon input
    Gia_ManForEachCi(giacir,pobj,i){
        NodeInfo* ci= this->getInfo(pobj);
        ci->setSim(rand());
    }
    Gia_ManForEachObj(giacir,pobj,i){
        if(Gia_ObjIsAnd(pobj)){
            simNode(pobj);
        }
    }
    int a=0;
        
        
    Gia_ManForEachObj(giacir,pobj,i){
        a=getInfo(pobj)->simValue;
        bitset<32> x(a);
        cout <<Gia_ObjId(giacir,pobj)<<" "<< x <<endl;

    }
    



}**/
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

 int simpleParsing(string in,string& out,int& i){
     int start=in.find('[');
     int end=in.find(']');
     if (end>start+1&& start>0){
         out=in.substr(0,start);
         i=stoi(in.substr(start+1,end-start-1));
     }else{
         out=in;
         i=-1;
     }
 }
 int Word::addbit(NodeInfo* ni,int nthbit){
     if(nthbit<word.size()){
         while(word.size()<nthbit+1){
             word.push_back(nullptr);
         }
     }
    word[nthbit]=ni;
 }
 int Collection::createFaninWords(vector<string>namecis){
     string pname;
     int count;
     unordered_map<string, Word*> name2word;
     unordered_map<string,Word*>::iterator it;
     for(int i=0;i<namecis.size();i++){
         simpleParsing(namecis[i],pname,count);
         if(count>=0)cout<<pname<<" "<<count<<endl;
         else cout<<pname;
         it=name2word.find(pname);
         if(it==name2word.end()){
             Word* w=new Word();
             //todo: get gia ci info 
             //w->addbit(ninfos[])
             allwords.push_back();
             allwords.back()
         }else{

         }
     }
 }