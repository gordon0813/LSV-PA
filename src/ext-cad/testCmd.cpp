// This file is for testing abc existing fucntion bt adding new command
#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"

static int Rvsn_CommandExecCutTest(Abc_Frame_t* pAbc, int argc, char** argv);
static int Rvsn_CommandExecBoxTest(Abc_Frame_t* pAbc, int argc, char** argv);

extern "C" {
  void Ree_ManComputeCutsTest( Gia_Man_t * p );
  void Acec_CreateBoxTest( Gia_Man_t * p );
}

void init_test(Abc_Frame_t* pAbc) {
  Cmd_CommandAdd(pAbc, "RVSN", "cut_test", Rvsn_CommandExecCutTest, 0);
  Cmd_CommandAdd(pAbc, "RVSN", "box_test", Rvsn_CommandExecBoxTest, 0);
}

void destroy_test(Abc_Frame_t* pAbc) {}

Abc_FrameInitializer_t frame_initializer_test = {init_test, destroy_test};

struct PackageRegistrationManager {
  PackageRegistrationManager() { Abc_FrameAddInitializer(&frame_initializer_test); }
} rvsnPackageRegistrationManager;

void Lsv_NtkPrintNodes(Abc_Ntk_t* pNtk) {
  Abc_Obj_t* pObj;
  int i;
  Abc_NtkForEachNode(pNtk, pObj, i) {
    printf("Object Id = %d, name = %s\n", Abc_ObjId(pObj), Abc_ObjName(pObj));
    Abc_Obj_t* pFanin;
    int j;
    Abc_ObjForEachFanin(pObj, pFanin, j) {
      printf("  Fanin-%d: Id = %d, name = %s\n", j, Abc_ObjId(pFanin),
             Abc_ObjName(pFanin));
    }
    if (Abc_NtkHasSop(pNtk)) {
      printf("The SOP of this node:\n%s", (char*)pObj->pData);
    }
  }
}

int Rvsn_CommandExecCutTest(Abc_Frame_t* pAbc, int argc, char** argv) {
  // Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
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
  if ( pAbc->pGia == NULL )
  {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  Ree_ManComputeCutsTest(pAbc->pGia);
  return 0;

usage:
  Abc_Print(-2, "usage: cut_test [-h]\n");
  Abc_Print(-2, "\t        prints the adders in the gia network\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}

int Rvsn_CommandExecBoxTest(Abc_Frame_t* pAbc, int argc, char** argv) {
  // Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
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
  if ( pAbc->pGia == NULL )
  {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  Acec_CreateBoxTest(pAbc->pGia);
  return 0;

usage:
  Abc_Print(-2, "usage: box_test [-h]\n");
  Abc_Print(-2, "\t        prints arithmetic boxes in the gia network\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}