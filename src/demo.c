/**CFile****************************************************************

  FileName    [demo.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [ABC as a static library.]

  Synopsis    [A demo program illustrating the use of ABC as a static library.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: demo.c,v 1.00 2005/11/14 00:00:00 alanmi Exp $]

***********************************************************************/

#include <stdio.h>
#include <time.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

#if defined(ABC_NAMESPACE)
namespace ABC_NAMESPACE
{
#elif defined(__cplusplus)
extern "C"
{
#endif

// procedures to start and stop the ABC framework
// (should be called before and after the ABC procedures are called)
void   Abc_Start();
void   Abc_Stop();

// procedures to get the ABC framework and execute commands in it
typedef struct Abc_Frame_t_ Abc_Frame_t;

Abc_Frame_t * Abc_FrameGetGlobalFrame();
int    Cmd_CommandExecute( Abc_Frame_t * pAbc, const char * sCommand );

#if defined(ABC_NAMESPACE)
}
using namespace ABC_NAMESPACE;
#elif defined(__cplusplus)
}
#endif

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [The main() procedure.]

  Description [This procedure compiles into a stand-alone program for 
  DAG-aware rewriting of the AIGs. A BLIF or PLA file to be considered
  for rewriting should be given as a command-line argument. Implementation 
  of the rewriting is inspired by the paper: Per Bjesse, Arne Boralv, 
  "DAG-aware circuit compression for formal verification", Proc. ICCAD 2004.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int main( int argc, char * argv[] )
{
    // parameters
    int fUseResyn2  = 0;
    int fPrintStats = 1;
    int fVerify     = 1;
    // variables
    Abc_Frame_t * pAbc;
    char *pFileNameIn = 0, *pFileNameOut = 0;
    char Command[1000];
    clock_t clkRead, clkRevEng, clk;

    //////////////////////////////////////////////////////////////////////////
    // get the input file name
    if ( argc != 5 )
    {
        printf( "Wrong number of command-line arguments.\n" );
        return 1;
    }
    short isInput = 0, isOutput = 0;
    for (int i = 1; i < 5; ++i) {
        // just in case
        if (isInput == 1 && isOutput == 1) {
            printf ("Error while parsing input.\n");
            return 1;
        }
        if (isInput == 0 && isOutput == 0) {
            if (strcmp(argv[i], "-input") == 0) {
                isInput = 1;
            } else if (strcmp(argv[i], "-output") == 0) {
                isOutput = 1;
            } else {
                printf ("Error while parsing input.\n");
                return 1;
            }

        } else if (isInput == 1) {
            pFileNameIn = argv[i];
            isInput = 0;
            
        } else if (isOutput == 1) {
            pFileNameOut = argv[i];
            isOutput = 0;
            
        } 
    }
    printf("Input  filepath: %s\n", pFileNameIn);
    printf("Output filepath: %s\n", pFileNameOut);
    //////////////////////////////////////////////////////////////////////////
    // start the ABC framework
    Abc_Start();
    pAbc = Abc_FrameGetGlobalFrame();

clk = clock();
    //////////////////////////////////////////////////////////////////////////
    // read the file

    // sprintf( Command, "help");
    // if ( Cmd_CommandExecute( pAbc, Command ) )
    // {
    //     fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
    //     return 1;
    // }

    sprintf( Command, "read %s", pFileNameIn );
    if ( Cmd_CommandExecute( pAbc, Command ) )
    {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
        return 1;
    }

    sprintf( Command, "strash" );
    if ( Cmd_CommandExecute( pAbc, Command ) )
    {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
        return 1;
    }

    sprintf( Command, "&get" );
    if ( Cmd_CommandExecute( pAbc, Command ) )
    {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
        return 1;
    }
clkRead = clock() - clk;

clk = clock();
    //////////////////////////////////////////////////////////////////////////
    // main function for reverse engineering
    sprintf( Command, "lsv_write_cada %s %s", pFileNameIn, pFileNameOut );
    if ( Cmd_CommandExecute( pAbc, Command ) )
    {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
        return 1;

    }
clkRevEng = clock() - clk;

    printf( "Reading = %6.2f sec   ",     (float)(clkRead)/(float)(CLOCKS_PER_SEC) );
    printf( "Reverse Engineering = %6.2f sec\n", (float)(clkRevEng)/(float)(CLOCKS_PER_SEC) );

    //////////////////////////////////////////////////////////////////////////
    // stop the ABC framework
    Abc_Stop();
    return 0;
}

