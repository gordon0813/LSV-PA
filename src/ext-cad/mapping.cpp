#include <unordered_map>
#include "ext-cad/cada.h"

//write out flow
//first check all PO save in Collection::allCOwords
//in simandmatch select matching function for each PO
//if some PO not match ,build gate level circuit
//



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
int Word::candidatesI(vector<Word*>&allc){
    if(iscut)allc.push_back(this);
    for(int i=0;i<input.size();i++){
        input[i]->candidatesI(allc);
    }
}
int Word::isRedundent(){
    vector<Word*>allc;
    candidatesI(allc);
    for (int i=0;i<allc.size();i++){
        for(int j=0;j<i;j++){
            if(allc[i]==allc[j])return 1;
        }
    }
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
int Collection::createFanoutWords(vector<string>namecos){
    string pname;
    int count;
    Gia_Obj_t *pobj;
    int i;
    unordered_map<string, Word *> name2word;
    unordered_map<string, Word *>::iterator it;
    vector<NodeInfo*>pis;
    Gia_ManForEachCo(giacir,pobj,i){
        pis.push_back(getInfo(pobj));
    }
    for (int i = 0; i < namecos.size(); i++)
    {
        simpleParsing(namecos[i], pname, count);
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
            allCOwords.push_back(w);
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

Word* Word::sub(Word*b){
     Word* c=new Word(max(word.size(),b->word.size())+1);
    c->type="-"; c->input.push_back(this) ; c->input.push_back(b) ; 
    return c;
}
Word* Word::mult(Word*b){
    Word* c=new Word(word.size()+b->word.size()+1);
        c->type="*"; c->input.push_back(this) ; c->input.push_back(b) ; 
    return c;
}
vector<Word*> Collection::allWordModules(const vector<Word*>&as,const vector<Word*>&bs,int samelevel){
    vector<Word*>re;
    for(int i=0;i<as.size();i++){
        for(int j=0;j<bs.size();j++){
            if(samelevel&&i==j)continue;
            re.push_back(as[i]->add(bs[j]));
           // cout<<re.back()->functionStr()<<endl;
            re.push_back(as[i]->sub(bs[j]));
           // cout<<re.back()->functionStr()<<endl;
            re.push_back(as[i]->mult(bs[j]));
           
        }
    }
    vector<Word*> noredudent;
    for(int i=0;i<re.size();i++){
        if(re[i]->isRedundent()){
            delete re[i];
        }else{
            noredudent.push_back(re[i]);
            cout<<noredudent.back()->functionStr()<<endl;
        }
    }

    return noredudent;
}
double Word::match(Word* bigger){
    //int maxmatch=0;
    int nowmatch=0;
    int i;
    for(i=0;i<min(simvalues.size(),bigger->simvalues.size());i++){
        if(bigger->simvalues[i]==simvalues[i])nowmatch++;
    }
   // cout<<"siumlarity "<<double(nowmatch)/i<<endl;
    return  double(nowmatch)/i;
}
//for test01
int Collection::simAndMatch(){
    //initialize
    for(int i=0;i<allwords.size();i++){
       allwords[i]->check();
       allwords[i]->set2Incut();
    }
    for(int i=0;i<allCOwords.size();i++){
       allCOwords[i]->check();
       allCOwords[i]->set2Incut();
    }
    Word* con=new Word(20);
    con->set2const(0);
    allwords.push_back(con);
    vector<vector<Word*>>nlevelWord;
    nlevelWord.push_back(allwords);
    int limitdepth=5;
    vector<Word*>onelevel;
    vector<Word*>onetmp;
    for(int i=1;i<min(int(allwords.size()),limitdepth);i++){
        //nlevelWord.push_back(vector<Word*>());
        onelevel.clear();
       // cout<<"i"<<i<<endl;
        for(int j=0;j<=i-1;j++){
            onetmp=allWordModules(nlevelWord[j],nlevelWord[i-j-1],j==i-j-1);
            for(int k=0;k<onetmp.size();k++)onelevel.push_back(onetmp[k]);
           // onelevel.insert(onelevel.end(),onetmp.begin(),onetmp.end());
            //onetmp=allWordModules(nlevelWord[j],nlevelWord[i-j-1],j==i-j-1);
        }
        nlevelWord.push_back(onelevel);
        
    }
    //return 0;
    // create arthmetic module
    
    Word* apb= allwords[0]->add(allwords[1]);
    Word* amb= allwords[0]->mult(allwords[1]);
    Word* bpc= allwords[1]->add(allwords[2]);
    Word* apc= allwords[0]->add(allwords[2]);
    Word* apbpc= apb->add(allwords[2]);
    
    Word* ambpc= amb->add(allwords[2]);
    Word* apbpcC= apbpc->add(con);
    //simMatchPair(allCOwords[0],apbpcC);
    //return 0;
    vector< Word*> checkList;
    for(int i=0;i<nlevelWord.size();i++){
        for(int j=0;j<nlevelWord[i].size();j++){
            checkList.push_back(nlevelWord[i][j]);
        }
    }
    // random assign all pi simvalue
    
    randomCi();
    // sim all circuit
    simall();
    //collect the word level simulate value
    //apb->getsimValue();
    //amb->getsimValue();
   // bpc->getsimValue();
    for(int i=0;i<checkList.size();i++)checkList[i]->getsimValue();
    for(int i=0;i<allCOwords.size();i++)allCOwords[i]->getsimValue();
    /**
    apc->getsimValue();
    apbpc->getsimValue();
    ambpc->getsimValue();
    checkList.push_back(apb);
    checkList.push_back(bpc);
    checkList.push_back(apc);
    checkList.push_back(apbpc);
    checkList.push_back(ambpc);
    **/
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
//        it=hashsimmap.find(~simv);
    }
    for(int i=0;i<checkList.size();i++){
       /**
            for(int j=0;j<checkList[i]->nbits();j++){
                it=hashsimmap.find(checkList[i]->simvalue(j));
                if(it!=hashsimmap.end()){
                    for(int k=0;k<it->second.size();k++){
                        if(checkList[i]->simvalue(j)==0)continue;
                       // cout<<checkList[i]->functionStr()<<" "<<j<<"th bit"<<" match: n"<<(it->second)[k]->nodeid()<<endl;
                    }
                    
                }
                it=hashsimmap.find(~checkList[i]->simvalue(j));
                if(it!=hashsimmap.end()){
                    for(int k=0;k<it->second.size();k++){
                        if(checkList[i]->simvalue(j)==0)continue;
                       // cout<<checkList[i]->functionStr()<<" "<<j<<"th bit"<<" match: !n"<<(it->second)[k]->nodeid()<<endl;
                    }
                    
                }
            }**/
            // word level match
            for(int j=0;j<allCOwords.size();j++){
                 if (outputFunctions.find(j) != outputFunctions.end())continue;
                int ccase=simMatchPair(allCOwords[j],checkList[i]);
                //ccase=0 unmatch, ccase=1 match without constant ,ccase=2 match with constant 
                if(ccase==1){
                    cout<<checkList[i]->functionStr()<<" "<<" match: "<<allCOwords[j]->functionStr()<<endl;
                    if (outputFunctions.find(j) == outputFunctions.end()) {
                        outputFunctions.insert({j, "assign " + allCOwords[j]->functionStr() + " = " + checkList[i]->functionStr() + ";"});
                    }//todo : replace with no constant version output 

                }else if(ccase==2){
                    cout<<checkList[i]->functionStr()<<" "<<" matchc: "<<allCOwords[j]->functionStr()<<endl;
                    
                    if (outputFunctions.find(j) == outputFunctions.end()) {
                        outputFunctions.insert({j, "assign " + allCOwords[j]->functionStr() + " = " + checkList[i]->functionStr() + ";"});
                    }

                }
                
                
                
                /**  old 
                if(allCOwords[j]->match(checkList[i])>0.9){
                    
                }**/
            }
    }

   return 0;

}
Word::Word(int nbits){
    iscut=0;
    isconstant=0;
    for(int i=0;i<nbits;i++){
        simvalues.push_back(0);
        word.push_back(nullptr);
    }
}


int constSimModule( vector<uint> &c,long n){
     size_t sc=n;
     for(int i=0;i<32;i++){
     for(int k=0;k<c.size();k++){
        c[k]=(c[k]>>1)+ (((sc>>k)%2)<<31);
    }
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
       //cout<<sa<<" "<<sb<<" "<<sc<<endl;
        
        //------------------
        for(int k=0;k<c.size();k++){
            c[k]=(c[k]>>1)+ (((sc>>k)%2)<<31);
        }
    }
//cout------------//**
/**
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
    **/
//------------
    return 0;
}
int simpleMultsimModule(const vector<uint> &a,const vector<uint> &b,vector<uint> &c , bool inv=0){
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
        sc=sa*sb;
        //cout<<sa<<" "<<sb<<" "<<sc<<endl;
        
        //------------------
        for(int k=0;k<c.size();k++){
            c[k]=(c[k]>>1)+ (((sc>>k)%2)<<31);
        }
    }
    //cout------------
    /**
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
    }**/
//------------

    return 0;
}
Word* Word::isconstTrace(){

    if(isconstant)return this;
    Word* c=0;
    for(int i=0;i<input.size();i++){
        c=input[i]->isconstTrace();
        if(c!=0){
            return c;
        }
    }
    return 0;
}
int Word::assignNumber(size_t number,int n){
    for(int k=0;k<word.size();k++){
        if((number>>k)%2){
            word[k]->simValue= word[k]->simValue|(1<<n);
        }else{
            word[k]->simValue= word[k]->simValue&(~(1<<n));
        }
    }
    return 0;
}
long Word::getNumber(int bit){
    long sa=0;
    
    for(int k=word.size()-1;k>=0;k--){
        if(word[k]==nullptr){
            sa=sa<<1;
            sa+=((simvalues[k]  >>bit) %2);
        }else{
            sa=sa<<1;
            sa+=((word[k]->simValue  >>bit) %2);
        }
            
    }
    return sa;
}


int Word::backAssignIn(size_t n){
    
    if(iscut){
        if(!isconstant){
            assignNumber(n,0);
        }else{

        }
    }else if(type.compare("+")==0){
        if(n==2)n=0;
        if(n==1){
            input[0]->backAssignIn(1);
            input[1]->backAssignIn(0);
        }else{  
            input[0]->backAssignIn(0);
            input[1]->backAssignIn(0);
        }
    }else if(type.compare("-")==0){//c=a-b
    if(n==2)n=0;
        if(n==1){
            input[0]->backAssignIn(1);
            input[1]->backAssignIn(0);
        }else{  
            input[0]->backAssignIn(0);
            input[1]->backAssignIn(0);
        }
    }else if(type.compare("*")==0){//c=a*b
         if(n==2)n=1;
        if(n==1){
            input[0]->backAssignIn(1);
            input[1]->backAssignIn(1);
        }else{  
            input[0]->backAssignIn(0);
            input[1]->backAssignIn(0);
        }
        //todo
    }else if(type.compare("neg")==0){ // a'=-a

    }else if(type.compare("^")==0){//c=a^b

    }else if(type.compare("<<")==0){

    }
    
}
int Collection::simMatchPair(Word* wtarget,Word* guess){
    Word* c=guess->isconstTrace();
    long cons;
   if(c){
      
      randomCi();
      guess->backAssignIn(2);
      simall();
      //guess->getsimValue();
      cons= wtarget->getNumber(0);
      if(cons==0)return 0;
      //cout<<"const is "<<cons<<endl;
     // cout<<wtarget->getNumber(1)<<endl;
      c->set2const(cons);
      randomCi();
      simall();
      guess->getsimValue();
      wtarget->getsimValue();
      if(wtarget->match(guess)>0.95){
          cout<<"match\n";
          cout<<guess->functionStr()<<endl;
          return 2;
      }
      for(int i=0;i<32;i++){
         // cout<<wtarget->getNumber(i)<<" "<<guess->getNumber(i)<<" "<<guess->getNumber(i)-wtarget->getNumber(i)<<endl;
      }
      

   }else{
       randomCi();
       simall();
       guess->getsimValue();
      wtarget->getsimValue();
      if(wtarget->match(guess)>0.95){
          cout<<"match\n";
          cout<<guess->functionStr()<<endl;
          return 1;
      }

   }
   return 0;
}
vector<uint> Word::getsimValue(){
    //vector<int>re;
    vector<uint> a;
    vector<uint> b;
    if(iscut){
        if(isconstant){
            constSimModule(simvalues,constantValue);
        }else{
            for(int i=0;i<word.size();i++){
                simvalues[i]=(word[i]->simValue);
             }

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
        a=input[0]->getsimValue();
        b=input[1]->getsimValue();
        simpleMultsimModule(a,b,simvalues,0);
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
   }else if(isconstant){
       return to_string(this->word.size())+"'d"+to_string(constantValue);
   }else{
       return name;
   }
   if(input.size()>1){
       b=input[1]->functionStr();
   }
   return "("+a+type+b+")";
}