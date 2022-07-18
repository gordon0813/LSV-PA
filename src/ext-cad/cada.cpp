
#include "ext-cad/cada.h"
static int Lsv_CommandPrintNodes(Abc_Frame_t* pAbc, int argc, char** argv);
void init(Abc_Frame_t* pAbc) {
     Cmd_CommandAdd(pAbc, "LSV", "lsv_print_nodes", Lsv_CommandPrintNodes, 0);
}
void destroy(Abc_Frame_t* pAbc) {}
Abc_FrameInitializer_t frame_initializer = {init, destroy};
struct PackageRegistrationManager {
  PackageRegistrationManager() { Abc_FrameAddInitializer(&frame_initializer); }
} lsvPackageRegistrationManager;


void Lsv_NtkPrintNodes(Abc_Ntk_t*  pNtk ,vector<string>&cinames ) {
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
  Abc_NtkForEachCi(pNtk, pObj, i){
     printf("fanin Id = %d, name = %s\n", Abc_ObjId(pObj), Abc_ObjName(pObj));
     cinames.push_back(string(Abc_ObjName(pObj)));
  }
  

}

int Lsv_CommandPrintNodes(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    Abc_Print(-2, "usage: lsv_print_nodes [-h]\n");
    Abc_Print(-2, "\t        prints the nodes in the network\n");
    Abc_Print(-2, "\t-h    : print the command usage\n");
    return 1;
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  //Lsv_NtkPrintNodes(pNtk);
  vector<string> namesci; 
  Collection a=Collection(pAbc->pGia);
  Lsv_NtkPrintNodes(pNtk,namesci);
  a.createFaninWords(namesci);
  a.simAndMatch();
  //a.incut( );
 // a.detectXor();
  a.showInfo("test.dot");
  return 0;


}