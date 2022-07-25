/**CFile****************************************************************

  FileName    [mainInit.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [The main package.]

  Synopsis    [Initialization procedures.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: mainInit.c,v 1.3 2005/09/14 22:53:37 casem Exp $]

***********************************************************************/

#include "base/abc/abc.h"
#include "ext-cad/cada.h"
#include "mainInt.h"

ABC_NAMESPACE_IMPL_START
 
////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

extern "C" void Abc_Init( Abc_Frame_t * pAbc );
extern "C" void Abc_End ( Abc_Frame_t * pAbc );
extern "C" void Io_Init( Abc_Frame_t * pAbc );
extern "C" void Io_End ( Abc_Frame_t * pAbc );
extern "C" void Cmd_Init( Abc_Frame_t * pAbc );
extern "C" void Cmd_End ( Abc_Frame_t * pAbc );
extern "C" void If_Init( Abc_Frame_t * pAbc );
extern "C" void If_End ( Abc_Frame_t * pAbc );
extern "C" void Map_Init( Abc_Frame_t * pAbc );
extern "C" void Map_End ( Abc_Frame_t * pAbc );
extern "C" void Mio_Init( Abc_Frame_t * pAbc );
extern "C" void Mio_End ( Abc_Frame_t * pAbc );
extern "C" void Super_Init( Abc_Frame_t * pAbc );
extern "C" void Super_End ( Abc_Frame_t * pAbc );
extern "C" void Libs_Init( Abc_Frame_t * pAbc );
extern "C" void Libs_End( Abc_Frame_t * pAbc );
extern "C" void Load_Init( Abc_Frame_t * pAbc );
extern "C" void Load_End( Abc_Frame_t * pAbc );
extern "C" void Scl_Init( Abc_Frame_t * pAbc );
extern "C" void Scl_End( Abc_Frame_t * pAbc );
extern "C" void Wlc_Init( Abc_Frame_t * pAbc );
extern "C" void Wlc_End( Abc_Frame_t * pAbc );
extern "C" void Wln_Init( Abc_Frame_t * pAbc );
extern "C" void Wln_End( Abc_Frame_t * pAbc );
extern "C" void Bac_Init( Abc_Frame_t * pAbc );
extern "C" void Bac_End( Abc_Frame_t * pAbc );
extern "C" void Cba_Init( Abc_Frame_t * pAbc );
extern "C" void Cba_End( Abc_Frame_t * pAbc );
extern "C" void Pla_Init( Abc_Frame_t * pAbc );
extern "C" void Pla_End( Abc_Frame_t * pAbc );
extern "C" void Sim_Init( Abc_Frame_t * pAbc );
extern "C" void Sim_End( Abc_Frame_t * pAbc );
extern "C" void Test_Init( Abc_Frame_t * pAbc );
extern "C" void Test_End( Abc_Frame_t * pAbc );
extern "C" void Abc2_Init( Abc_Frame_t * pAbc );
extern "C" void Abc2_End ( Abc_Frame_t * pAbc );
extern "C" void Abc85_Init( Abc_Frame_t * pAbc );
extern "C" void Abc85_End( Abc_Frame_t * pAbc );
extern "C" void Glucose_Init( Abc_Frame_t *pAbc );
extern "C" void Glucose_End( Abc_Frame_t * pAbc );
extern "C" void Glucose2_Init( Abc_Frame_t *pAbc );
extern "C" void Glucose2_End( Abc_Frame_t * pAbc );
extern void Cada_init( Abc_Frame_t * pAbc);
extern void Cada_destroy( Abc_Frame_t * pAbc);

static Abc_FrameInitializer_t* s_InitializerStart = NULL;
static Abc_FrameInitializer_t* s_InitializerEnd = NULL;

void Abc_FrameAddInitializer( Abc_FrameInitializer_t* p )
{
    if( ! s_InitializerStart )
        s_InitializerStart = p;

    p->next = NULL;
    p->prev = s_InitializerEnd;

    if ( s_InitializerEnd )
        s_InitializerEnd->next = p;

    s_InitializerEnd = p;

}

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Starts all the packages.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Abc_FrameInit( Abc_Frame_t * pAbc )
{
    Abc_FrameInitializer_t* p;
    Cmd_Init( pAbc );
    Cmd_CommandExecute( pAbc, "set checkread" ); 
    Io_Init( pAbc );
    Abc_Init( pAbc );
    If_Init( pAbc );
    Map_Init( pAbc );
    Mio_Init( pAbc );
    Super_Init( pAbc );
    Libs_Init( pAbc );
    Load_Init( pAbc );
    Scl_Init( pAbc );
    Wlc_Init( pAbc );
    Wln_Init( pAbc );
    Bac_Init( pAbc );
    Cba_Init( pAbc );
    Pla_Init( pAbc );
    Test_Init( pAbc );
    Glucose_Init( pAbc );
    Glucose2_Init( pAbc );
    Cada_init( pAbc );
    for( p = s_InitializerStart ; p ; p = p->next )
        if(p->init)
            p->init(pAbc);
}


/**Function*************************************************************

  Synopsis    [Stops all the packages.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Abc_FrameEnd( Abc_Frame_t * pAbc )
{
    Abc_FrameInitializer_t* p;
    for( p = s_InitializerEnd ; p ; p = p->prev )
        if ( p->destroy )
            p->destroy(pAbc);
    Abc_End( pAbc );
    Io_End( pAbc );
    Cmd_End( pAbc );
    If_End( pAbc );
    Map_End( pAbc );
    Mio_End( pAbc );
    Super_End( pAbc );
    Libs_End( pAbc );
    Load_End( pAbc );
    Scl_End( pAbc );
    Wlc_End( pAbc );
    Wln_End( pAbc );
    Bac_End( pAbc );
    Cba_End( pAbc );
    Pla_End( pAbc );
    Test_End( pAbc );
    Glucose_End( pAbc );
    Cada_destroy( pAbc );
}


////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

