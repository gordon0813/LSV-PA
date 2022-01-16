#include<algorithm>
#include <vector>
#include <map>
#include <iostream>
#include <bitset>
#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "sat/cnf/cnf.h"
//read ../benchmarks-master/arithmetic/self_test1.v
#define Abc_NtkForEachNode_pa1(pNtk,pNode,i) for ( i = 0; (i < Vec_PtrSize((pNtk)->vObjs)) && (((pNode) = Abc_NtkObj(pNtk, i)), 1); i++ ) if ( (pNode) == NULL || (!Abc_ObjIsNode(pNode) &&!(pNode->Type == ABC_OBJ_CONST1) )) {} else
extern "C" Aig_Man_t *Abc_NtkToDar(Abc_Ntk_t *pNtk, int fExors, int fRegisters);
//read ./lsv_fall_2021/pa1/4bitadder_s.blif 
//strash
//lsv_print_msfc
static int Lsv_CommandPrintNodes(Abc_Frame_t* pAbc, int argc, char** argv);
static int Lsv_CommandPrint_msfc(Abc_Frame_t* pAbc, int argc, char** argv);
static int Lsv_CommandSatNodeSweep(Abc_Frame_t* pAbc, int argc, char** argv);
static int lsv_or_bidec(Abc_Frame_t* pAbc, int argc, char** argv);

void init(Abc_Frame_t* pAbc) {
  Cmd_CommandAdd(pAbc, "LSV", "lsv_print_nodes", Lsv_CommandPrintNodes, 0);
  Cmd_CommandAdd(pAbc, "LSV", "lsv_print_msfc", Lsv_CommandPrint_msfc, 0);
  Cmd_CommandAdd(pAbc, "LSV", "lsv_or_bidec", lsv_or_bidec, 0);
  Cmd_CommandAdd(pAbc, "LSV", "lsv_sat_NodeSweep", Lsv_CommandSatNodeSweep, 0);
}

void destroy(Abc_Frame_t* pAbc) {}

Abc_FrameInitializer_t frame_initializer = {init, destroy};
 int sat_solver_add_buffer_reverse_enable( sat_solver * pSat, int iVarA, int iVarB, int iVarEn, int fCompl )
{
    lit Lits[3];
    int Cid;
    assert( iVarA >= 0 && iVarB >= 0 && iVarEn >= 0 );

    Lits[0] = toLitCond( iVarA, 0 );
    Lits[1] = toLitCond( iVarB, !fCompl );
    Lits[2] = toLitCond( iVarEn, 0 );
    Cid = sat_solver_addclause( pSat, Lits, Lits + 3 );
    assert( Cid );

    Lits[0] = toLitCond( iVarA, 1 );
    Lits[1] = toLitCond( iVarB, fCompl );
    Lits[2] = toLitCond( iVarEn, 0 );
    Cid = sat_solver_addclause( pSat, Lits, Lits + 3 );
    assert( Cid );
    return 2;
}
struct PackageRegistrationManager {
  PackageRegistrationManager() { Abc_FrameAddInitializer(&frame_initializer); }
} lsvPackageRegistrationManager;

class Mfsc_set{
  public:
  static int merge(Mfsc_set* a,Mfsc_set* b, std::vector<Mfsc_set*>&total){
    Mfsc_set* big ;
    Mfsc_set*small;
    if(a->mfscset.size()>b->mfscset.size()){
      big=a;  
      small=b;
    }else{
      big=b;
      small=a;
    }
    int id;
    for(int i=0;i<small->mfscset.size();++i){
      id=small->mfscset[i];
      total[id]=big;
    }
    big->mergeInto(small);
    return 0;
  }
  Mfsc_set(int id){
    this->mfscset.push_back(id);
    mark=false;
  }
  int head(){return mfscset[0];}

  //a1 merge into this
  int mergeInto(Mfsc_set* a){
    for(int i=0;i<a->mfscset.size();++i){
      this->mfscset.push_back(a->mfscset[i]);
    }
    a->mfscset.clear();
    return 0;
  }
  void sort(){std::sort(mfscset.begin(),mfscset.end());}
  void show(int i){
    
    printf("MSFC %d: ",i);
    for(int i=0;i<mfscset.size();++i){
      if(i!=0)printf(",");
      printf ("n%d",mfscset[i]);
    }
    printf ("\n");
  }
  bool mark;
  private:
  std::vector<int> mfscset;
  
};
bool compareMFSC_set(Mfsc_set* a,Mfsc_set*b){
  return a->head() <b->head();
}
void Lsv_NtkPrintNodes(Abc_Ntk_t*  pNtk) {
  Abc_Obj_t* pObj;
  int i;
  Abc_NtkForEachNode(pNtk, pObj, i) {
    printf("Object Id = %d, name = %s\n", Abc_ObjId(pObj), Abc_ObjName(pObj));
    Abc_Obj_t* pFanin;
    int j;
    Abc_ObjForEachFanin(pObj, pFanin, j) {
      printf("  Fanin-%d: Id = %d, name = %s ,type=%d \n", j, Abc_ObjId(pFanin),
             Abc_ObjName(pFanin),Abc_ObjType(pFanin));
    }
    if (Abc_NtkHasSop(pNtk)) {
      printf("The SOP of this node:\n%s", (char*)pObj->pData);
    }
  }
}

int Lsv_CommandPrintNodes(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  Lsv_NtkPrintNodes(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: lsv_print_nodes [-h]\n");
  Abc_Print(-2, "\t        prints the nodes in the network\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}
void Lsv_NtkPrint_msfc(Abc_Ntk_t* pNtk) {
  Abc_Obj_t* pObj;
  int i;
  std::vector<Mfsc_set*>total;
  for (int i=0;i<Vec_PtrSize((pNtk)->vObjs);++i){
    total.push_back(nullptr);
  }
  // init  :for each  node -> set
  Abc_NtkForEachNode_pa1(pNtk, pObj, i) {
    int thisid=Abc_ObjId(pObj);
    //printf("Object Id = %d, name = %s\n", Abc_ObjId(pObj), Abc_ObjName(pObj));
    total[thisid]=new Mfsc_set(Abc_ObjId(pObj));
    /*
    Abc_Obj_t* pFanin;
    int j;
    //Abc_ObjForEachFanout
    Abc_ObjForEachFanin(pObj, pFanin, j) {
      printf("  Fanin-%d: Id = %d, name = %s\n", j, Abc_ObjId(pFanin),
             Abc_ObjName(pFanin));
    }
    */

  
  }
  // start merging mfsc set
  Abc_NtkForEachNode_pa1(pNtk, pObj, i) {
    //printf("Object Id = %d, name = %s\n", Abc_ObjId(pObj), Abc_ObjName(pObj));
    int thisid=Abc_ObjId(pObj);
    int j;
    //Abc_ObjForEachFanout
    Abc_Obj_t* pFanout;
    if(Abc_ObjFanoutNum(pObj)==0){
      total[thisid]->mark=true;
    }
    if(Abc_ObjFanoutNum(pObj)>1)continue;
    Abc_ObjForEachFanout(pObj, pFanout, j){
      if(Abc_ObjType(pFanout)!=7  )
      {//printf("Object Id = %d not node\n",Abc_ObjId(pFanout));
      continue;
      }
      Mfsc_set::merge(total[thisid],total[Abc_ObjId(pFanout)],total);
      
      //total[thisid]->show(Abc_ObjId(pFanout));
    }
  }
  //printf("leave");
  //start collecting and sorting sets
  std::vector<Mfsc_set*> finalset; 
  for(int i=0;i<total.size();++i){
    if(total[i]==nullptr ||total[i]->mark)continue;
    total[i]->sort();
   // total[i]->show(i);
    finalset.push_back(total[i]);
    total[i]->mark=true;
  }
  total.clear();
  //print out the mfsc sets
  std::sort(finalset.begin(),finalset.end(),compareMFSC_set);
  for(int i=0;i<finalset.size();++i){
    finalset[i]->show(i);
  }
  //printf("end\n");
}
int Lsv_CommandPrint_msfc(Abc_Frame_t* pAbc, int argc, char** argv) {

  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  Lsv_NtkPrint_msfc(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: lsv_print_msfc [-h]\n");
  Abc_Print(-2, "\t        prints the  maximum single-fanout cones\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}
//return if it is valid seed
bool seed2assumption(lit *assumptions,const std::vector<int>&seeds,const std::vector<int>& ais,const std::vector<int>& bis){
  int tob=seeds.size();
  for(int i=0;i<tob;i++){
    if(seeds[i]==0){
      assumptions[i]=toLitCond(ais[i],1);
      assumptions[i+tob]=toLitCond(bis[i],1);
    }else if(seeds[i]==1){
      assumptions[i]=toLitCond(ais[i],1);
      assumptions[i+tob]=toLitCond(bis[i],0);
    }else if(seeds[i]==2){
      assumptions[i]=toLitCond(ais[i],0);
      assumptions[i+tob]=toLitCond(bis[i],1);
    }
  }
  return true;
}
void toans(int nFinal,int * pFinal,const std::vector<int>& ais,const std::vector<int>& bis){
  std::vector<int>ans;
  int id;
  for (int i=0;i<ais.size();i++){
    ans.push_back(3);
  }
  for (int i=0;i<nFinal;i++){
    id=pFinal[i]/2;
    for(int j=0;j<ais.size();j++){
      if(ais[j]==id){
        ans[j]^=1;
        break;
      }else if(bis[j]==id){
        ans[j]^=2;
        break;
      }
    }
  }
  for(int i=0;i<ans.size();i++){
    if(ans[i]==3){
      printf("1");
    }else{
      printf("%d",ans[i]);
    }
  }
  printf("\n");

}
void print_one_ORbid(sat_solver *solver,const std::vector<int>& ais,const std::vector<int>& bis){
  bool endflag=false;
  int tob=ais.size();
  lit *assumptions = new lit[ais.size()*2];
  std::vector<int>seeds;
  int result;
  int *pFinal;
  int nfinal;
  for (int i=0;i<tob;i++){
    seeds.push_back(0);
  }
  

    //generate  seed
    for(int i=0;i<seeds.size();i++){
      for(int j=i+1;j<seeds.size();j++){
        seeds[i]=1;
        seeds[j]=2;
        //use seed to create assumption
        seed2assumption(assumptions,seeds,ais,bis);
        //use assumption to solve
        result=sat_solver_solve(solver,assumptions,assumptions+ais.size()*2,0,0,0,0);
        //check result
        if(result==l_True){//sat
         // printf("sat %d %d\n",i,j);
        }else{//unsat
          printf("1\n");
          nfinal=sat_solver_final(solver,&pFinal);
          
          
          toans(nfinal,pFinal,ais,bis);
          
          endflag=true;
          break;
        }
        //
        seeds[i]=0;
        seeds[j]=0;
      }
      
      if(endflag)break;
    }
    if(!endflag)printf("0\n");
    //remember to free memory(maybe)
    //delete [] assumptions;
  
}
void lsv_print_ORbid(Abc_Ntk_t*  pNtk){
  Abc_Obj_t* pObj;
  Aig_Obj_t* conePI;
  int i,j;
  Abc_Ntk_t * pNtk_cone;
  Aig_Man_t *pMan;
  Cnf_Dat_t *f1Cnf;
  Cnf_Dat_t *f2Cnf;
  Cnf_Dat_t *f3Cnf;
  int nclasues;
  int liftconst;
  std::vector<int> f1xis;
  std::vector<int> ais;
  std::vector<int> bis;
  sat_solver *solver;
  int count=0;
  Abc_NtkForEachPo(pNtk, pObj, i){
    f1xis.clear();
    ais.clear();
    bis.clear();
    pNtk_cone=Abc_NtkCreateCone(pNtk,Abc_ObjFanin0(pObj),Abc_ObjName(pObj),0);// 0 => don't keep all pi(DFS reached only)
    if (Abc_ObjFaninC0(pObj))
    {
      Abc_NtkPo(pNtk_cone, 0)->fCompl0 ^= 1;
    }
    printf("PO %s support partition: ",Abc_ObjName(pObj));
    count++;
    
    pMan = Abc_NtkToDar(pNtk_cone, 0, 0);
    assert(Aig_ManCoNum(pMan)==1);
    f1Cnf = Cnf_Derive(pMan, 0); //0 => assert all output to 1
    f2Cnf = Cnf_DataDup(f1Cnf);
    liftconst=f2Cnf->nVars;
    Cnf_DataLift(f2Cnf,liftconst);
    //todo:modify the output phase 
    nclasues=f2Cnf->nClauses;
    //printf("before %d",f2Cnf->pClauses[nclasues-1][0]);
    f2Cnf->pClauses[nclasues-1][0]=f2Cnf->pClauses[nclasues-1][0] ^ 1;  //change assert output 1 => output 0
    //printf("after %d",f2Cnf->pClauses[nclasues-1][0]);
    //end
    f3Cnf = Cnf_DataDup(f2Cnf);
    assert(liftconst==f3Cnf->nVars);
    Cnf_DataLift(f3Cnf,liftconst);
    //get x0-xi -> varID
    Aig_ManForEachCi(pMan,conePI,j){
      f1xis.push_back(f1Cnf->pVarNums[conePI->Id]);
    }
    //create sat solver ,add:  f(x) and !f'(x) and !f''(x)
    solver = (sat_solver *)Cnf_DataWriteIntoSolver(f1Cnf, 1, 0);
    if(!solver){
      printf("0\n");
      continue;
    }
    solver = (sat_solver *)Cnf_DataWriteIntoSolverInt(solver, f2Cnf, 1, 0);
    if(!solver){
      printf("0\n");
      continue;
    }
    solver = (sat_solver *)Cnf_DataWriteIntoSolverInt(solver, f3Cnf, 1, 0);
    //new ai , bi
    //printf("create ai bi\n");
    for(int k=0;k<f1xis.size();k++){
      ais.push_back(sat_solver_addvar(solver));
      bis.push_back(sat_solver_addvar(solver));
      //printf("a%d b%d ",ais[k],bis[k]);
    }
    //printf("create ai bi end\n");
    //create xi vs ai bi
    for(int k=0;k<f1xis.size();k++){
     // printf("f1%d f2%d ",f1xis[k],f1xis[k]+liftconst);
      sat_solver_add_buffer_reverse_enable(solver,f1xis[k],f1xis[k]+liftconst,ais[k],0);
      sat_solver_add_buffer_reverse_enable(solver,f1xis[k],f1xis[k]+2*liftconst,bis[k],0);
    }
   //calculate one answer
    print_one_ORbid(solver,ais,bis);
    //sat_solver_add_const()
    /*
    delete f1Cnf;
    delete f2Cnf;
    delete f3Cnf;
    delete conePI;*/
    

  }


}

int lsv_or_bidec(Abc_Frame_t* pAbc, int argc, char** argv){
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  lsv_print_ORbid(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: lsv_or_bidec [-h]\n");
  Abc_Print(-2, "\t      find OR bi-decomposable under a variable partition \n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}
/**
class AigInfo{
  public:
  class NodeInfo{
    NodeInfo(){valid=0;}
    int satvar;
    int mark;
    bool valid;
  };
  Aig_Man_t *pMan;
  std::vector<NodeInfo> id2info;
  AigInfo(Aig_Man_t *pMan){ this->pMan=pMan;}
  getInfo(){

  } 
};**/
class NodeInfo{
  public:
    NodeInfo(){valid=0;mark=0;}
    int satvar;
    int mark;
    bool valid;
};
NodeInfo* existed(std::map<int,NodeInfo*>& out_info,Aig_Obj_t* theNode ,bool need_add){
    std::map<int, NodeInfo*>::iterator iter;
    iter = out_info.find(theNode->Id);
    if(iter!=out_info.end()){
      iter->second->mark+=1;
      return iter->second;
    }else{
      if(need_add)out_info.insert( std::pair<int,NodeInfo*>(theNode->Id,new NodeInfo() ));
      return nullptr;
    }
}

//=========================

// progress: sim -> odc -> collect_u -> next_u -> else

// if for each pattern, F(u) imply F(v) || v is ODC in this pattern
// F(u') imply F(v) is OK too

typedef unsigned int pat_unit;

struct u_pair
{
  int ua_id;
  int ub_id;
  bool ua_cp;
  bool ub_cp;
  int status;
};

class FinalManager
{
private:
  Aig_Man_t* pMan;
  int v_id;
  int level;
  int seed;
  int pat_unit_bit_len;

  std::vector<int> u_id_list;
  std::vector<bool> u_comp_list;

  std::vector<std::vector<pat_unit> > node_sim_value;
  std::vector<bool> visited;
  int sim_begin;
  int sim_end;

  std::vector<bool> TFO_of_v_mark;

  int init_pat_len;
  int pat_len;
  std::vector<std::vector<pat_unit> > pat;

  std::vector<pat_unit> v_odc;// v's odc

  int ua_index;
  int ub_index;
  u_pair u;

  void gen_pat();// generate pattern
  void add_pat(std::vector<std::vector<bool> >);

  // simulation related
  // sim has range for partial sim, if you want pat #100 ~ # 200, call sim_set_range(100, 201)
  // reset will set range back to whole pat
  // notice!!! only sim has range kinda thing, other like odc is always work as with whole pat len
  void sim_init();
  void sim(Aig_Obj_t*);
  void sim_set_range(int, int);
  void sim_reset_range();

  // need this before calculating odc and collecting u, avoid TFO of v
  void mark_TFO_of_v_init();
  void mark_TFO_of_v(Aig_Obj_t*);

  void cal_odc_init();
  std::vector<pat_unit> cal_odc(int, int, int);

  void collect_u();
  bool next_u();

public:
  // set aig manager, some initialization
  FinalManager(Aig_Man_t*);

  void setTargetNode(int);// TODO

  // get a u, if no more u, u.status = -1
  u_pair getCandidates() {next_u(); return u;}
  void print_u();

  // add cex, then resim(all), cex dimention: [PI num][pat_len]
  void resim(std::vector<std::vector<bool> >);// TODO

  int u_state(){return this->u.status;}
};

FinalManager::FinalManager(Aig_Man_t* pMan_in)
{
  level = 3;
  init_pat_len = 3200;
  seed = 69420;
  pat_unit_bit_len = sizeof(pat_unit) * 8;
  ua_index = 0;
  ub_index = 0;
  pMan = pMan_in;
  srand(seed);
  // srand(time(NULL));
}

void FinalManager::setTargetNode(int v_id_in) {
  v_id = v_id_in;

  // new pat: sim->odc->collect u
  // new pat(partial): sim(partial)->odc(partial)->collect u(all)
  // new v: mark TFO->sim->odc->collect u

  std::cout << "generating pattern..." << std::endl;
  gen_pat();
  //std::cout << "...done" << std::endl;
  //std::cout << "simulating..." << std::endl;
  sim_init();
  //std::cout << "...done" << std::endl;
  //Aig_ManFanoutStart(pMan);
  //std::cout << "TFO of v init..." << std::endl;
  mark_TFO_of_v_init();
 // std::cout << "...done" << std::endl;
  //std::cout << "calculating v's odc..." << std::endl;
  cal_odc_init();
  //std::cout << "...done" << std::endl;
  //Aig_ManFanoutStop(pMan);
  //std::cout << "calculating u set..." << std::endl;
  collect_u();
  //std::cout << "...done" << std::endl;
  std::cout << "finding u pairs..." << std::endl;
  int p;
  for (p = 0; p < 999; p++) {
    if (!next_u()) {
      break;
    }
  }
  std::cout << "...done, total " << p << " pairs" << std::endl;
  // std::cout << "simulating..." << std::endl;
  // sim_set_range(20, 51);
  // sim_init();
  // std::cout << "...done" << std::endl;
}

void FinalManager::mark_TFO_of_v_init() {
  TFO_of_v_mark.resize(Aig_ManObjNumMax(pMan));
  for (int p = 0; p < Aig_ManObjNumMax(pMan); p++) {
    TFO_of_v_mark[p] = 0;
  }

  mark_TFO_of_v(Aig_ManObj(pMan, v_id));
}

void FinalManager::mark_TFO_of_v(Aig_Obj_t* v) {
  if (TFO_of_v_mark[Aig_ObjId(v)]) {
    return;
  } else {
    Aig_Obj_t* pFO;
    int i, iFanout = -1;
    TFO_of_v_mark[Aig_ObjId(v)] = 1;
    Aig_ObjForEachFanout(pMan, v, pFO, iFanout, i) {
      mark_TFO_of_v(pFO);
    }
  }
}

void FinalManager::cal_odc_init()
{
  v_odc = cal_odc(v_id, v_id, level);
  std::cout << "odc of v: ";
  for (int p = 0; p < (((pat_len - 1) / pat_unit_bit_len) + 1); p++) {
    //std::cout << std::bitset<sizeof(pat_unit)*8>(v_odc[p]);
  }
  std::cout << std::endl;
}

std::vector<pat_unit> FinalManager::cal_odc(int x_id, int v_id, int k)
{
  if (k == 0) {
    std::vector<pat_unit> x_odc;
    for (int p = 0; p < (((pat_len - 1) / pat_unit_bit_len) + 1); p++) {
      // pat_unit x_odc_temp = 0;
      pat_unit x_odc_temp = 0xFFFFFFFF;
      // for (int q = 0; ((p * pat_unit_bit_len + q) < pat_len) && q < pat_unit_bit_len; q++) {
      //   x_odc_temp &= ~(1 << q);
      //   // x_odc_temp |= (1 << q);
      // }
      x_odc.push_back(x_odc_temp);
    }
    return x_odc;
  } else {
    Aig_Obj_t* pFO;
    int i, iFanout = -1;
    Aig_ObjForEachFanout(pMan, Aig_ManObj(pMan, x_id), pFO, iFanout, i) {
      if (pFO->Type == AIG_OBJ_CO) {
        std::vector<pat_unit> x_odc;
        for (int p = 0; p < (((pat_len - 1) / pat_unit_bit_len) + 1); p++) {
          pat_unit x_odc_temp = 0xFFFFFFFF;
          x_odc.push_back(x_odc_temp);
        }
        return x_odc;
      }
    }

    std::vector<pat_unit> x_odc;// odc of x with respect to v
    // assign odc to all 0 default
    for (int p = 0; p < (((pat_len - 1) / pat_unit_bit_len) + 1); p++) {
      pat_unit x_odc_temp = 0x00000000;
      x_odc.push_back(x_odc_temp);
    }

    Aig_ObjForEachFanout(pMan, Aig_ManObj(pMan, x_id), pFO, iFanout, i) {
      assert(pFO->Type == AIG_OBJ_AND);
      std::vector<pat_unit> odc_temp = cal_odc(Aig_ObjId(pFO), v_id, k - 1);
      int other_id;
      bool other_cp;
      if (Aig_ObjId(Aig_ObjFanin0(pFO)) == x_id) {
        other_id = Aig_ObjId(Aig_ObjFanin1(pFO));
        other_cp = Aig_ObjFaninC1(pFO);
      } else {
        other_id = Aig_ObjId(Aig_ObjFanin0(pFO));
        other_cp = Aig_ObjFaninC0(pFO);
      }
      pat_unit TFO_of_v_mark_other_id_expand = TFO_of_v_mark[other_id] ? 0xFFFFFFFF : 0x00000000;
      pat_unit other_cp_expand = other_cp ? 0xFFFFFFFF : 0x00000000;

      for (int p = 0; p < (((pat_len - 1) / pat_unit_bit_len) + 1); p++) {
        // 1 is care, 0 is dont care
        // for this fanout, its odc is odc_temp, which can be further block by "other"
        // if other fanin is controlling value and it's not TFO of v, block odc_temp to 0
        x_odc[p] = x_odc[p] | (odc_temp[p] & (TFO_of_v_mark_other_id_expand | (node_sim_value[other_id][p] ^ other_cp_expand)));
      }
    }
    return x_odc;
  }
}

void FinalManager::gen_pat()
{
  pat.resize(Aig_ManCiNum(pMan));
  Aig_Obj_t *pCI;
  int i;
  Aig_ManForEachCi(pMan, pCI, i) {
    pat[i].resize(((init_pat_len - 1) / pat_unit_bit_len) + 1);
    for (int p = 0; p < (((init_pat_len - 1) / pat_unit_bit_len) + 1); p++) {
      // if its not 32 bit, may be wrong
      pat[i][p] = rand();
    }
  }
  
  pat_len = init_pat_len;
  sim_reset_range();
}

void FinalManager::sim_set_range(int begin, int end) {
  assert(begin >= 0);
  assert(end <= pat_len);
  sim_begin = begin - (begin % pat_unit_bit_len);
  sim_end = end;
  //std::cout << "(sim) sim_begin(" << begin << ") is set to: " << sim_begin << std::endl;
  //std::cout << "(sim) sim_end(" << end << ") is set to: " << sim_end << std::endl;
}

void FinalManager::sim_reset_range() {
  sim_set_range(0, pat_len);
}

void FinalManager::sim_init()
{
  Aig_Obj_t *pCI, *pCO;
  int i;
  std::cout<<"that is"<<Aig_ManObjNumMax(pMan)<<std::endl;
  visited.resize(Aig_ManObjNumMax(pMan));
  std::vector<bool> temp(pat_len);
  node_sim_value.resize(Aig_ManObjNumMax(pMan));
  for (int p = 0; p < Aig_ManObjNumMax(pMan); p++) {
    node_sim_value[p].resize(((pat_len - 1) / pat_unit_bit_len) + 1);
    visited[p] = 0;
  }
  
  // assign PI first
  Aig_ManForEachCi(pMan, pCI, i) {
    for (int p = sim_begin / pat_unit_bit_len; p < (((sim_end - 1) / pat_unit_bit_len) + 1); p++) {
      node_sim_value[Aig_ObjId(pCI)][p] = pat[i][p];
    }
    visited[Aig_ObjId(pCI)] = 1;
  }

  // const
  for (int p = sim_begin / pat_unit_bit_len; p < (((sim_end - 1) / pat_unit_bit_len) + 1); p++) {
    node_sim_value[0][p] = 0x00000000;
    visited[0] = 1;
  }

  Aig_ManForEachCo(pMan, pCO, i) {
    //std::cout << "co #" << i << " simulating" << std::endl;
    sim(pCO);
  }
}

void FinalManager::sim(Aig_Obj_t *pObj)
{
  int pObj_id = Aig_ObjId(pObj);
  if (visited[pObj_id]) {
    return;
  } else {
    assert(!Aig_ObjIsCi(pObj));
    assert(!Aig_ObjIsConst1(pObj));

    if (pObj->Type == AIG_OBJ_AND) {
      visited[pObj_id] = 1;
      Aig_Obj_t *pFanin0 = Aig_ObjFanin0(pObj);
      Aig_Obj_t *pFanin1 = Aig_ObjFanin1(pObj);
      int pFanin0_id = Aig_ObjId(pFanin0);
      int pFanin1_id = Aig_ObjId(pFanin1);
      bool comp0 = Aig_ObjFaninC0(pObj);
      bool comp1 = Aig_ObjFaninC1(pObj);
      pat_unit comp0_expand = comp0 ? 0xFFFFFFFF : 0x00000000;
      pat_unit comp1_expand = comp1 ? 0xFFFFFFFF : 0x00000000;

      sim(pFanin0);
      sim(pFanin1);

      for (int p = sim_begin / pat_unit_bit_len; p < (((sim_end - 1) / pat_unit_bit_len) + 1); p++) {
        node_sim_value[pObj_id][p] = (node_sim_value[pFanin0_id][p] ^ comp0_expand) & (node_sim_value[pFanin1_id][p] ^ comp1_expand);
      }
    } else if (pObj->Type == AIG_OBJ_CO) {
      visited[pObj_id] = 1;
      Aig_Obj_t *pFanin0 = Aig_ObjFanin0(pObj);
      int pFanin0_id = Aig_ObjId(pFanin0);
      bool comp0 = Aig_ObjFaninC0(pObj);
      pat_unit comp0_expand = comp0 ? 0xFFFFFFFF : 0x00000000;
      sim(pFanin0);

      for (int p = sim_begin / pat_unit_bit_len; p < (((sim_end - 1) / pat_unit_bit_len) + 1); p++) {
        node_sim_value[pObj_id][p] = node_sim_value[pFanin0_id][p] ^ comp0_expand;
      }
    } else {
      std::cout << "pObj->Type is not AND or CO, error" << std::endl;
    }
  }
}

void FinalManager::collect_u() {
  // for all u != v and u not belongs to TFO of v, if f(u) -> f(v) or odc(v) == 0 for every pattern, then collect
  // u could be complement of itself

  u_id_list.clear();
  u_comp_list.clear();

  Aig_Obj_t *pObj;
  int i;
  Aig_ManForEachObj(pMan, pObj, i) {
    //std::cout<<pObj<<" "<<pObj->Id<<std::endl;
    if (TFO_of_v_mark[Aig_ObjId(pObj)]) {
      continue;
    }

    bool flag = 1;
    bool comp = 0;

    for (int p = 0; p < (((pat_len - 1) / pat_unit_bit_len) + 1); p++) {
      int left_bit_len = pat_len - p * pat_unit_bit_len;
      pat_unit mask = left_bit_len < pat_unit_bit_len ? (1 << left_bit_len) - 1 : 0xFFFFFFFF;
      // u imply v
      if (((node_sim_value[Aig_ObjId(pObj)][p] & ~node_sim_value[v_id][p]) & v_odc[p]) & mask) {
        flag = 0;
        break;
      }
    }

    if (!flag) {
      flag = 1;
      comp = 1;
      for (int p = 0; p < (((pat_len - 1) / pat_unit_bit_len) + 1); p++) {
        int left_bit_len = pat_len - p * pat_unit_bit_len;
        pat_unit mask = left_bit_len < pat_unit_bit_len ? (1 << left_bit_len) - 1 : 0xFFFFFFFF;
        // ~u imply v
        if ((~(node_sim_value[Aig_ObjId(pObj)][p] | node_sim_value[v_id][p]) & v_odc[p]) & mask) {
          flag = 0;
          break;
        }
      }
    }

    if (flag) {
      u_id_list.push_back(Aig_ObjId(pObj));
      u_comp_list.push_back(comp); 
    }
  }

  for (int p = 0; p < u_id_list.size(); p++) {
   // std::cout << "u[" << p << "]->id: " << u_id_list[p] << ", comp: " << u_comp_list[p] << std::endl;
  }
}

bool FinalManager::next_u() {
  for (int q = ub_index + 1; q < u_id_list.size(); q++) {
    bool and_flag = 1;
    bool or_flag = 1;
    pat_unit ua_comp_expand = u_comp_list[ua_index] ? 0xFFFFFFFF : 0x00000000;
    pat_unit ub_comp_expand = u_comp_list[q]        ? 0xFFFFFFFF : 0x00000000;

    for (int p = 0; p < (((pat_len - 1) / pat_unit_bit_len) + 1); p++) {
      int left_bit_len = pat_len - p * pat_unit_bit_len;
      pat_unit mask = left_bit_len < pat_unit_bit_len ? (1 << left_bit_len) - 1 : 0xFFFFFFFF;
      pat_unit ua_value = node_sim_value[u_id_list[ua_index]][p] ^ ua_comp_expand;
      pat_unit ub_value = node_sim_value[u_id_list[q       ]][p] ^ ub_comp_expand;
      pat_unit v_odc_and_mask = v_odc[p] & mask;
      // if ua & ub == v or v is odc is false for any pattern, then try next
      if (and_flag && (((ua_value & ub_value) ^ node_sim_value[v_id][p]) & v_odc_and_mask)) {
        and_flag = 0;
      }
      if (or_flag  && (((ua_value | ub_value) ^ node_sim_value[v_id][p]) & v_odc_and_mask)) {
        or_flag = 0;
      }
      if (!and_flag && !or_flag) {
        break;
      }
    }

    if (and_flag || or_flag) {
      ub_index = q;
      u.ua_id = u_id_list[ua_index];
      u.ub_id = u_id_list[ub_index];
      u.ua_cp = u_comp_list[ua_index];
      u.ub_cp = u_comp_list[ub_index];
      u.status = and_flag + 2 * or_flag;
     // std::cout << "u pair found, ";
     //print_u();
      return 1;
    }
  }

  for (int r = ua_index + 1; r < u_id_list.size(); r++) {
    for (int q = r + 1; q < u_id_list.size(); q++) {
      bool and_flag = 1;
      bool or_flag = 1;
      pat_unit ua_comp_expand = u_comp_list[r] ? 0xFFFFFFFF : 0x00000000;
      pat_unit ub_comp_expand = u_comp_list[q] ? 0xFFFFFFFF : 0x00000000;

      for (int p = 0; p < (((pat_len - 1) / pat_unit_bit_len) + 1); p++) {
        int left_bit_len = pat_len - p * pat_unit_bit_len;
        pat_unit mask = left_bit_len < pat_unit_bit_len ? (1 << left_bit_len) - 1 : 0xFFFFFFFF;
        pat_unit ua_value = node_sim_value[u_id_list[r]][p] ^ ua_comp_expand;
        pat_unit ub_value = node_sim_value[u_id_list[q]][p] ^ ub_comp_expand;
        pat_unit v_odc_and_mask = v_odc[p] & mask;
        // if ua & ub == v or v is odc is false for any pattern, then try next
        if (and_flag && (((ua_value & ub_value) ^ node_sim_value[v_id][p]) & v_odc_and_mask)) {
          and_flag = 0;
        }
        if (or_flag  && (((ua_value | ub_value) ^ node_sim_value[v_id][p]) & v_odc_and_mask)) {
          or_flag = 0;
        }
        if (!and_flag && !or_flag) {
          break;
        }
      }

      if (and_flag || or_flag) {
        ua_index = r;
        ub_index = q;
        u.ua_id = u_id_list[ua_index];
        u.ub_id = u_id_list[ub_index];
        u.ua_cp = u_comp_list[ua_index];
        u.ub_cp = u_comp_list[ub_index];
        // 0 for none, 1 for and, 2 for or, 3 for both, -1 for no more
        u.status = and_flag + 2 * or_flag;
        //std::cout << "u pair found, ";
        //print_u();
        return 1;
      }
    }
  }
  //std::cout << "no more u pair!" << std::endl;
  ua_index = 0;
  ub_index = 0;
  u.ua_id = 0;
  u.ub_id = 0;
  u.ua_cp = 0;
  u.ub_cp = 0;
  u.status = -1;
  //print_u();
  return 0;
}

void FinalManager::print_u() {
  std::cout << "ua_id: " << u.ua_id << ", ua_cp: " << u.ua_cp << ", ub_id: " << u.ub_id << ", ub_cp: " << u.ub_cp << ", status: " << u.status << std::endl;
}

void FinalManager::add_pat(std::vector<std::vector<bool> > pat_append) {
  int append_bit_len = pat_append[0].size();
  int left_bit_len   = pat_len % pat_unit_bit_len;
  int vacant_bit_len = left_bit_len ? (pat_unit_bit_len - left_bit_len) : 0;
  for (int r = 0; r < pat.size(); r++) { // each PI
    int counter = 0;
    if (vacant_bit_len > 0) {
      int p = (pat_len - 1) / pat_unit_bit_len; // entry of the last pat unit which exhibits vacancy
      pat_unit mask = left_bit_len < pat_unit_bit_len ? (1 << left_bit_len) - 1 : 0xFFFFFFFF;
      pat_unit pat_temp = 0x00000000;
      while (counter < vacant_bit_len && counter < append_bit_len) {
        if (pat_append[r][counter]) {
          pat_temp |= 1 << (counter + left_bit_len);
        }
        counter++;
      }
      pat[r][p] = (pat[r][p] & mask) | (pat_temp & ~mask);
    }

    while (counter < append_bit_len) {
      pat_unit pat_temp = 0x00000000;
      for (int q = 0; q < pat_unit_bit_len; q++) {
        if (pat_append[r][counter]) {
          pat_temp |= 1 << q;
        }
        counter++;
        if (counter == append_bit_len) {
          break;
        }
      }
      pat[r].push_back(pat_temp);
    }
  }

  pat_len += append_bit_len;
}

void FinalManager::resim(std::vector<std::vector<bool> > pat_append) {
  add_pat(pat_append);
  sim_reset_range();
  sim_init();
  collect_u();
}



int Lsv_CommandTest(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  FinalManager fm(NULL);
  Aig_Man_t *pAig;
  int v_id;
  int c, i, j;
  std::vector<std::vector<bool> > pat_append;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }

  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }

  assert(Abc_NtkIsStrash(pNtk));

  pAig = Abc_NtkToDar(pNtk, 0, 1);
  v_id = 36;

  fm = FinalManager(pAig);
  fm.setTargetNode(v_id);

  // pat_append.resize(6);
  // for (i = 0; i < 6; i++) {
  //   pat_append[i].resize(10);
  //   std::cout << "CI #" << i << ", appending pat: ";
  //   for (j = 0; j < 10; j++) {
  //     pat_append[i][j] = rand() % 2;
  //     std::cout << pat_append[i][j];
  //   }
  //   std::cout << std::endl;
  // }

  // fm.resim(pat_append);

  return 0;

usage:
  Abc_Print(-2, "usage: test [-h]\n");
  Abc_Print(-2, "\t        test\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}



Aig_Obj_t* getsideInput(Aig_Obj_t* target,Aig_Obj_t* onein,int& com)
{
  if(Aig_ObjFanin0(target)==onein){
    if(Aig_ObjFaninC1(target))com=1;
     return Aig_ObjFanin1(target);
    }else{
      if(Aig_ObjFaninC0(target))com=1;
      return Aig_ObjFanin0(target);
    }
}
std::vector<int> createCNF(Aig_Man_t *pMan,sat_solver * solver){
  Vec_Ptr_t * vp =Aig_ManDfsAll(pMan);
  Aig_Obj_t* e;
  int i;
  int maxid=0;
  std::vector<int> id2var;
  Vec_PtrForEachEntry(Aig_Obj_t*,vp,e,i){
    if(e->Id>maxid)maxid=e->Id;
  }
  for(int i=0;i<=maxid;i++){
    id2var.push_back(-1);
  }
  Vec_PtrForEachEntry(Aig_Obj_t*,vp,e,i){
    id2var[e->Id]=sat_solver_addvar(solver);
  }
  sat_solver_add_const(solver,id2var[e->Id],1);
  Vec_PtrForEachEntry(Aig_Obj_t*,vp,e,i){
    if(Aig_ObjIsNode(e)){
     // printf("add node %d\n",e->Id);
      sat_solver_add_and(solver,id2var[e->Id],id2var[Aig_ObjFanin0(e)->Id],id2var[Aig_ObjFanin1(e)->Id],Aig_ObjFaninC0(e),Aig_ObjFaninC1(e),0);
    }
  }
  return id2var;
  //remember free vp
}
int createOBS(const std::vector<int>& id2var,sat_solver * solver,Aig_Man_t *pMan, std::map<int,NodeInfo*>&fanout_info,Aig_Obj_t* targetNode){
  Aig_Obj_t* fanout;
  int ifan,i;
  NodeInfo* info;
  Aig_Obj_t* sideInput;
  int sideInC;
  int sideVar;
  int ans,obs;
  int retmp,refinal;
  int c1=sat_solver_addvar(solver);
  int c0=sat_solver_addvar(solver);
  sat_solver_add_const(solver,c1,0);
  sat_solver_add_const(solver,c0,1);
  //printf("at %d node\n",targetNode->Id);
  if(Aig_ObjIsCo(targetNode)){
    return c1;
  }else{
    retmp=c0;
  }
  
  
  Aig_ObjForEachFanout(pMan,targetNode,fanout,ifan,i){
   // printf("at %d type\n",fanout->Type);
    info=existed(fanout_info,fanout,false);
    if(info==nullptr){
      //ans=const 1
      ans = c1;
      //printf("no fanout %d node\n",fanout->Id);
      
    }else if(info->mark==1){
      //ans= side & 
      sideInC=0;
        sideInput= getsideInput(fanout,targetNode,sideInC);
       // printf("fanout %d node side in %d compl %d\n",fanout->Id,sideInput->Id,sideInC);
        sideVar=id2var[sideInput->Id];
        ans = sat_solver_addvar(solver);
        obs=createOBS(id2var,solver,pMan,fanout_info,fanout);
        sat_solver_add_and(solver,ans,obs,sideVar,0,sideInC,0);
        //sat_solver_add_and
        //sat_solver_add_const

    }else{
      //reuse obs (if need)
      //printf("fanout %d node  two side\n",fanout->Id);
      obs=createOBS(id2var,solver,pMan,fanout_info,fanout);
      ans=obs;
      //ans = createObs(fanout)
    }
    refinal=sat_solver_addvar(solver);
    sat_solver_add_and(solver,refinal,ans,retmp,1,1,1);//check
    retmp=refinal;
  }
  //printf("at %d node ans%d\n",targetNode->Id,ans);
  return retmp;
}
int sat_user_values(sat_solver * solver,int v){
  //assert( v >= 0 && v < s->size );
  return (int) (solver->model[v]==1);
}

// 0 end ,1:one node sweep ,2: 2node sweep ,3:skip
int translate_up(u_pair up, int & id1 ,int & id2, int &c1,int & c2,int & cc){
  if(up.status==-1)return 0;
  //if(up.status==3)return 3;
  id1=up.ua_id;
  id2=up.ub_id;
  if(up.status==2){
    cc=1;
    c1=!up.ua_cp;
    c2=!up.ub_cp;
  }else if(up.status==1){
    cc=0;
    c1=up.ua_cp;
    c2=up.ub_cp;
  }else{
    cc=0;
    c1=up.ua_cp;
    c2=up.ub_cp;
    //std::cout<<"error"<<std::endl;
  }


  if(id1==0){
    //if(up.status==2)return 0;
    id1=id2;
    c1=c2;
    return 1;
  }else if(id2==0){
    //if(up.status==2)return 0;
    return 1;
  }else{
    return 2;
  }
 
  
}


int sweep_one_node(Aig_Man_t *pMan,int id,FinalManager & fm){
  int level=3;
  int ifan,k;
  std::vector<std::vector<Aig_Obj_t*> > fanout_side_free;
  std::vector<std::vector<bool> > cec_list;
  std::map<int,NodeInfo*> fanout_info;
  //Cnf_Dat_t *netCnf;
  std::vector<int> id2var;
  Aig_Obj_t* targetNode=Aig_ManObj(pMan,id);
  Aig_Obj_t* otherNode;
  Aig_Obj_t* u1;
  Aig_Obj_t* u2;
  Aig_Obj_t* tmpnode;
  int tmpi;
  lit *assumptions = new lit[1];
  //netCnf = Cnf_Derive(pMan, Aig_ManCoNum(pMan)); //0 => assert all output to 1
  sat_solver * solver = sat_solver_new();
  id2var=createCNF(pMan,solver);
  //get candidate
  //find n level fanout (unate or binate node)
  fanout_side_free.push_back(std::vector<Aig_Obj_t*>());
  fanout_side_free[0].push_back(targetNode);
  existed(fanout_info,targetNode,true);
  for (int i=0;i<level;i++){
    fanout_side_free.push_back(std::vector<Aig_Obj_t*>());
    for(int j=0;j<fanout_side_free[i].size();j++){
      Aig_ObjForEachFanout(pMan,fanout_side_free[i][j],otherNode,ifan,k){
       // printf("this is %d node out %d\n",fanout_side_free[i][j]->Id,otherNode->Id);
        if(Aig_ObjIsCo(otherNode))continue;
        if(!existed(fanout_info,otherNode,true)){
          //printf("get %d node\n",otherNode->Id);
          fanout_side_free[i+1].push_back(otherNode);
        }
      }
    }
  }
  Aig_ManForEachCi(pMan,tmpnode,tmpi){
    cec_list.push_back(std::vector<bool>());
    cec_list[tmpi].push_back(false);
  }

  /*
  for(int i=0;i<16;i++){
    printf("=========%d id : %d======\n",i,id2var[i]);
  }*/
  
  //create obs cnf
  int obs=createOBS(id2var,solver,pMan,fanout_info,targetNode);
  sat_solver_add_const(solver,obs,0);


//======
 int c1 =1,c2=0,cc=1;
 int id1=4,id2=3;
 int style=1;
 int ando, andi1, andi2;
 for(int ii=0;ii<30;ii++){
  
  u_pair up= fm.getCandidates();

  printf("candidate: %d %d %d %d %d \n",up.ua_id,up.ua_cp,up.ub_id,up.ub_cp,up.status);
  //translate u_pair
  style=translate_up(up,id1,id2,c1,c2,cc);
  if(style==0){std::cout<<"not found \n" ;break;}
  //if(style==1)continue;
  
  u1=Aig_ManObj(pMan,id1);
  u2=Aig_ManObj(pMan,id2);
  
  if(Aig_ObjIsCo(u1) ||Aig_ObjIsCo(u2)) continue;
  assert(!Aig_ObjIsCo(u1) &&!Aig_ObjIsCo(u2));
    ando= sat_solver_addvar(solver);
    andi1=id2var[u1->Id];
    andi2=id2var[u2->Id];
  
  

  sat_solver_add_and(solver,ando,andi1,andi2,c1,c2,cc);
  //get candidates ,add candidates cnf
  int assump=sat_solver_addvar(solver);
  printf("xor: %d %d %d \n",assump,ando,id2var[targetNode->Id]);
  sat_solver_add_xor(solver,assump,ando,id2var[targetNode->Id],0);
  //solve
  assumptions[0]=toLitCond(assump,0);
  int result=sat_solver_solve(solver,assumptions,assumptions+1,0,0,0,0);
  //feed back counter examples or undate circuit
  printf("=========result : %d======\n",result);
 // Sat_SolverPrintStats( stdout, solver );
  for(int i=0;i<id2var.size();i++){
    if(id2var[i]!=-1){
      //sat_solver_get_var_value
     // printf("id:%d var:%d\n",i,sat_user_values(solver,id2var[i]));
    }
  }
  //start  replace
  Aig_Obj_t* newgate;

  if(result==-1){
    //continue;
    if(style==1){
      //continue;
      std::cout<<"replace by :"<<id1<<" c "<<c1<<"\n";
      newgate=Aig_ManObj(pMan,id1);
      newgate=(c1)?Aig_Not(newgate):newgate;
    }else if(true){
      continue;
    }
    else if(up.status==1 ||up.status==3){ //and
      u1=(up.ua_cp)?Aig_Not(u1):u1;
      u2=(up.ub_cp)?Aig_Not(u2):u2;
      newgate=Aig_And(pMan,u1,u2);
    }else if(up.status==2){//or
      u1=(up.ua_cp)?Aig_Not(u1):u1;
      u2=(up.ub_cp)?Aig_Not(u2):u2;
      newgate=Aig_Or(pMan,u1,u2);
    } 
    if(targetNode!=newgate){
      Aig_ObjReplace(pMan,targetNode,newgate,0);
    }
    
    Aig_ManCleanup(pMan);
    break;
  }else{
    Aig_ManForEachCi(pMan,tmpnode,tmpi){
      //cec_list.push_back(std::vector<bool>());
      cec_list[tmpi].push_back(( sat_user_values(solver,id2var[tmpnode->Id]) ==0)?false:true);
    }
    std::cout<<"========resim=============="<<std::endl;
    //fm.resim(cec_list);
  }


 }
 sat_solver_delete(solver);
 //clean up
  //printf("obs:%d var:%d\n",20,sat_solver_get_var_value(solver,20));
  //printf("obs:%d var:%d\n",28,sat_solver_get_var_value(solver,28));
  //printf("obs:%d var:%d\n",29,sat_solver_get_var_value(solver,29));
  //printf("obs:%d var:%d\n",24,sat_solver_get_var_value(solver,24));
return 0;

}
void dfsprint(Aig_Man_t *pMan){
  Vec_Ptr_t * vp =Aig_ManDfsAll(pMan);
  //Vec_Ptr_t * vp =Aig_ManDfsChoices(pMan);
  
  Aig_Obj_t* e;
  int i;
  Vec_PtrForEachEntry(Aig_Obj_t*,vp,e,i){

    printf("dfs: %d type %d\n",e->Id,e->Type);
    if(e->Type==5){
      printf("     0 in %d c %d\n",Aig_ObjFanin0(e)->Id,Aig_ObjFaninC0(e));
      printf("     1 in %d c %d\n",Aig_ObjFanin1(e)->Id,Aig_ObjFaninC1(e));
    }else if(e->Type==3){
      printf("     0 in %d\n",Aig_ObjFanin0(e)->Id);
    }
    

  }
  Vec_PtrFree(vp);
}
int randomselect(Aig_Man_t *pMan,std::vector<Aig_Obj_t*> & vneed,int reset){
  //Vec_PtrFreeFree
  int newrand;
  int reid;
  if(!reset){
    newrand=rand();
    newrand=newrand%(vneed.size());
    reid=vneed[newrand]->Id;
    vneed[newrand]=vneed.back();
    vneed.pop_back();
    return reid;
  }
  vneed.clear();
  Vec_Ptr_t * vp =Aig_ManDfsAll(pMan);
  Aig_Obj_t* e;
  int i;
  Vec_PtrForEachEntry(Aig_Obj_t*,vp,e,i){
    if(e->Type==5){
      vneed.push_back(e);
    }
  }
  Vec_PtrFree(vp);
  newrand=rand();
  newrand=newrand%(vneed.size());
  return vneed[newrand]->Id;
  


}
void lsv_SatNodeSweep(Abc_Ntk_t*  pNtk){
  std::vector<Aig_Obj_t*> vneed;
  std::vector<int> leftnum;
  FinalManager fm(NULL);
  Aig_Man_t *pMan;
  int needreset=1;
  pMan=Abc_NtkToDar(pNtk,0,0);
  Aig_ManFanoutStart(pMan);
  fm=FinalManager(pMan);
  int targetid;
  int oneitnum;
  for (int it=0;it<10;it++){
    oneitnum=Aig_ManObjNum(pMan);
    leftnum.push_back(oneitnum);
    std::cout<<"======iter======="<<it<<"===num==="<<oneitnum<<"==========="<<"\n\n\n";
    for (int k=0;k<oneitnum;k++){
      targetid=randomselect(pMan,vneed,needreset);
  //targetid=10;
      std::cout<<"\nselect:"<<targetid<<"  need num :"<<vneed.size()<<"\n";
      fm.setTargetNode(targetid);
     // if(fm.u_state()==-1)continue;
      sweep_one_node(pMan,targetid,fm);
    }
  }
  std::cout<<"======final =====\n";
 for(int i=0;i<leftnum.size();i++){
   std::cout<<i<<" "<<leftnum[i]<<std::endl;
 }
  
  /*
  //dfsprint(pMan);
  targetid=randomselect(pMan,vneed,needreset);
  //targetid=9;
  std::cout<<"\nselect:"<<targetid<<"  need num :"<<vneed.size()<<"\n";
  fm.setTargetNode(targetid);
  sweep_one_node(pMan,targetid,fm);
  //dfsprint(pMan);
  targetid=randomselect(pMan,vneed,needreset);
  std::cout<<"\nselect:"<<targetid<<"  need num :"<<vneed.size()<<"\n";
  fm.setTargetNode(targetid);
  
  sweep_one_node(pMan,targetid,fm);
  //dfsprint(pMan);
  printf("============\n");
  //fm.setTargetNode(9);
  //sweep_one_node(pMan,9,fm);
  //sweep_one_node(pMan,12,0,9,1,10,1);
  //sweep_one_node(pMan,11);
  printf("============\n");
  //sweep_one_node(pMan,10);
*/


}

int Lsv_CommandSatNodeSweep(Abc_Frame_t* pAbc, int argc, char** argv){
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  lsv_SatNodeSweep(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: lsv_or_bidec [-h]\n");
  Abc_Print(-2, "\t      find OR bi-decomposable under a variable partition \n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}


