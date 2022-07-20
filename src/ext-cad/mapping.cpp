#include <unordered_map>
#include "ext-cad/cada.h"

using namespace std;
int Collection::detectXor()
{
    int i; 
    int id=0;
    int count = 0;
    Gia_Obj_t *pobj, *pFan0, *pFan1;
    pobj = getobj(id);
    Gia_ManForEachObj(giacir, pobj, i)
    {
        if (Gia_ObjRecognizeExor(pobj, &pFan0, &pFan1))
        {
            std::cout << "xor" << id << "\n";
            Component *xgate = new CompXor();
            xgate->push_input(pFan0);
            xgate->push_input(pFan1);
            xgate->push_output(pobj);
            setInfo(pFan0, xgate);
            setInfo(pFan1, xgate);
            setInfo(pobj, xgate);
            count++;
        }
    }
    return count;
}
void Collection::showInfo(std::string filename)
{

    // xorChainAll();
    Gia_Obj_t *pobj;
    int i;

    std::ofstream myfile;
    myfile.open(filename);
    myfile << "digraph G1 {\n";
    Gia_ManForEachObj(giacir, pobj, i)
    {
        if (Gia_ObjIsCo(pobj))
        {
            myfile << " a" << Gia_ObjId(giacir, Gia_ObjFanin0(pobj)) << " -> "
                   << " a" << Gia_ObjId(giacir, pobj) << edgeFeature(pobj, Gia_ObjFaninC0(pobj)) << ";\n";
        }
        else if (Gia_ObjIsAnd(pobj))
        {
            myfile << " a" << Gia_ObjId(giacir, Gia_ObjFanin0(pobj)) << " -> "
                   << " a" << Gia_ObjId(giacir, pobj) << edgeFeature(pobj, Gia_ObjFaninC0(pobj)) << ";\n";
            myfile << " a" << Gia_ObjId(giacir, Gia_ObjFanin1(pobj)) << " -> "
                   << " a" << Gia_ObjId(giacir, pobj) << edgeFeature(pobj, Gia_ObjFaninC1(pobj)) << ";\n";
        }
    }
    myfile << "}\n";
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
std::string NodeInfo::ComponentColor()
{
    int match_count = 0;
    std::stringstream ss;
    ss << ",color= \"black";
    for (int i = 0; i < matchComps.size(); i++)
    {
        ss << ":" << matchComps[i]->thecolor();
    }
    ss << "\"";
    return ss.str();
}

int simpleParsing(string in, string &out, int &i)
{
    int start = in.find('[');
    int end = in.find(']');
    if (end > start + 1 && start > 0)
    {
        out = in.substr(0, start);
        i = stoi(in.substr(start + 1, end - start - 1));
    }
    else
    {
        out = in;
        i = -1;
    }
    return 0;
}
int Word::addbit(NodeInfo *ni, int nthbit)
{
   
        while (word.size() < nthbit + 1)
        {
            word.push_back(nullptr);
        }
  
    word[nthbit] = ni;
    return 0;
}
int Collection::createFaninWords(vector<string> namecis)
{
    string pname;
    int count;
    Gia_Obj_t *pobj;
    int i;
    unordered_map<string, Word *> name2word;
    unordered_map<string, Word *>::iterator it;
    vector<NodeInfo*>pis;
    Gia_ManForEachCi(giacir,pobj,i){
        pis.push_back(getInfo(pobj));
    }
    for (int i = 0; i < namecis.size(); i++)
    {
        simpleParsing(namecis[i], pname, count);
        if (count >= 0)
            cout << pname << " " << count << endl;
        else
            cout << pname;
        it = name2word.find(pname);
        if(count==-1)continue;
        if (it == name2word.end())
        {
            Word *w = new Word();
            w->setName(pname);

            w->addbit(pis[i],count);
            cout<<"id "<<pis[i]->nodeid()<<" "<<count<<endl;
            allwords.push_back(w);
            name2word.insert(pair<string,Word*>(pname,w));
        }
        else
        {
            cout<<"id "<<pis[i]->nodeid()<<" "<<count<<endl;
            it->second->addbit(pis[i],count);
        }
    }
    return 0;

}
//for test01
int Collection::simAndMatch(){
    //initialize
    for(int i=0;i<allwords.size();i++){
        allwords[i]->check();
       allwords[i]->set2Incut();
    }
    // create arthmetic module
    Word* apb= allwords[0]->add(allwords[1]);
    Word* bpc= allwords[1]->add(allwords[2]);
    Word* apc= allwords[0]->add(allwords[2]);
    Word* apbpc= apb->add(allwords[2]);
    // random assign all pi simvalue

    randomCi();
    // sim all circuit
    simall();
    //collect the word level simulate value
    apb->getsimValue();
    bpc->getsimValue();
    apc->getsimValue();
    apbpc->getsimValue();
    vector< Word*> checkList;
    checkList.push_back(apb);
    checkList.push_back(bpc);
    checkList.push_back(apc);
    checkList.push_back(apbpc);
    //show how we match gia to word info (need modify to hash match version)
    unordered_map<uint,vector<NodeInfo*>> hashsimmap;
    unordered_map<uint, vector<NodeInfo*>>::iterator it;
    for(int i=0;i<ninfos.size();i++){
        // for example a+b+c [2] could be matched by following constrain
        uint simv=ninfos[i]->simValue;
        it=hashsimmap.find(simv);
        if(it==hashsimmap.end()){
            vector<NodeInfo*> tmp;
            tmp.push_back(ninfos[i]);
            hashsimmap.insert(pair<uint,vector<NodeInfo*>>(simv,tmp));
        }else{
            it->second.push_back(ninfos[i]);
        }
        /**
        it=hashsimmap.find(~simv);
        if(it==hashsimmap.end()){
            vector<NodeInfo*> tmp;
            tmp.push_back(ninfos[i]);
            hashsimmap.insert(pair<uint,vector<NodeInfo*>>(~simv,tmp));
        }else{
            it->second.push_back(ninfos[i]);
        }
    
         for(int j=0;j<checkList.size();j++){
        
            if(ninfos[i]->simValue==checkList[j]->simvalue(2)||ninfos[i]->simValue==~checkList[j]->simvalue(2)){
               // cout<<checkList[j]->functionStr() <<" match: n"<<i<<endl;
            }

        }**/
    }
   for(int i=0;i<checkList.size();i++){
            for(int j=0;j<checkList[i]->nbits();j++){
                it=hashsimmap.find(checkList[i]->simvalue(j));
                if(it!=hashsimmap.end()){
                    for(int k=0;k<it->second.size();k++){
                        cout<<checkList[i]->functionStr()<<" "<<j<<"th bit"<<" match: n"<<(it->second)[k]->nodeid()<<endl;
                    }
                    
                }
                it=hashsimmap.find(~checkList[i]->simvalue(j));
                if(it!=hashsimmap.end()){
                    for(int k=0;k<it->second.size();k++){
                        cout<<checkList[i]->functionStr()<<" "<<j<<"th bit"<<" match: !n"<<(it->second)[k]->nodeid()<<endl;
                    }
                    
                }
            }
    }

   return 0;

}
Word::Word(int nbits){
    iscut=0;
    for(int i=0;i<nbits;i++){
        simvalues.push_back(0);
        word.push_back(nullptr);
    }
}
//what if word size > 64 ? (crash)
//this template can be used to handle c= a+b
//but may be able to handle  a-b,a*b once we change the add function
int simpleAddsimModule(const vector<uint> &a,const vector<uint> &b,vector<uint> &c , bool inv=0){
    size_t sa=0;
    size_t sb=0;
    size_t sc=0;
   
    vector<int>tmp;     
    
           // cout <<Gia_ObjId(giacir,pobj)<<" "<< x <<endl;
    for(int i=0;i<32;i++){
        sa=0;sb=0;
        for(int k=a.size()-1;k>=0;k--){
            
            sa=sa<<1;
            sa+=((a[k]>>i) %2);
        }
        for(int k=b.size()-1;k>=0;k--){
            sb=sb<<1;
            sb+=((b[k]>>i) %2);
        }
        //add function------(try to change to -/*/<</>> to create more usage)
        sc=inv ? sa-sb:sa+sb;
        cout<<sa<<" "<<sb<<" "<<sc<<endl;
        
        //------------------
        for(int k=0;k<c.size();k++){
            c[k]=(c[k]>>1)+ (((sc>>k)%2)<<31);
        }
    }
//cout------------
    for(int k=0;k<a.size();k++){
        bitset<32> xa(a[k]);
        cout<<"a:"<<xa<<endl;
    }
    for(int k=0;k<b.size();k++){
        bitset<32> xb(b[k]);
        cout<<"b:"<<xb<<endl;
    }
    for(int k=0;k<c.size();k++){
        bitset<32> xc(c[k]);
        cout<<"c:"<<xc<<endl;
    }
//------------
    return 0;
}
vector<uint> Word::getsimValue(){
    //vector<int>re;
    vector<uint> a;
    vector<uint> b;
    if(iscut){
         for(int i=0;i<word.size();i++){
            simvalues[i]=(word[i]->simValue);
         }
    }else if(type.compare("+")==0){
        a=input[0]->getsimValue();
        b=input[1]->getsimValue();
        simpleAddsimModule(a,b,simvalues);
    }else if(type.compare("-")==0){//c=a-b
        a=input[0]->getsimValue();
        b=input[1]->getsimValue();
        simpleAddsimModule(a,b,simvalues, 1);
    }else if(type.compare("*")==0){//c=a*b
        //todo
    }else if(type.compare("neg")==0){ // a'=-a

    }else if(type.compare("^")==0){//c=a^b

    }else if(type.compare("<<")==0){

    }
    return simvalues;
}
int Word::set2Incut(){
    iscut=true;
    for(int i=0;i<word.size();i++){
        if(word[i]==nullptr){
            continue;
        }
        this->simvalues[i] =word[i]->simValue;
    }
    return 0;
}
int Word::check(){
    if(word.size()!=simvalues.size()){
        simvalues.resize(word.size());
    }
    return 0;
}
string Word::functionStr(){
   string a;
   string b;
   if(input.size()>0){
       a=input[0]->functionStr();
   }else{
       return name;
   }
   if(input.size()>1){
       b=input[1]->functionStr();
   }
   return "("+a+type+b+")";
}