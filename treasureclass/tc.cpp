// Convert raw text table to HTML table
// Read: all_set_unq_items.txt
// Output: HTML tr

// Includes
    #define _CRT_SECURE_NO_WARNINGS
    #include <stdio.h>  // printf()
    #include <string.h> // memset()
    #include <stdlib.h> // exit()
    #include <ctype.h>  // isalpha()

// Defines
    #define DEBUG 0

    #if DEBUG
        #define DEBUGF printf
    #else
        #define DEBUGF(...)
    #endif

// Consts
    const size_t BUFFER = 65536;
    const char *EMPTY_NAME = "---";

// Types
    struct Longest_t
    {
        char *name;
        int   length;
        int   line;
        int   total;
    };

    struct Treasure_t
    {
        char  sTC[4];

        char *pBaseName;
        int   nBaseLen;
        char *pBaseQlvl;

        char *pSetsName;
        int   nSetsLen;
        char *pSetsQlvl;

        char *pUnqsName;
        int   nUnqsLen;
        char *pUnqsQlvl;
    };

// Globals
    size_t gnSize;
    char   gaBuffer[ BUFFER ];

    int    gnLines;
    char  *gaLines[ 4096 ];

    Treasure_t gaTreasure[ 512 ];

    int  giSet;
    int  giUnq;
   
    int  gnSet;
    int  gnUnq;

    Longest_t gLongestBase,
              gLongestSets,
              gLongestUnqs;

// Prototypes __________________________________________________________
    char *skipText( char *pText );
    int countTabs( char *pLine );

    void dumpDataForSort( const Treasure_t *pTreasure, const int line );
    void dumpTreasure( const Treasure_t *pTreasure, const int line );
    void getColumns( int iLine, char *pLine, Treasure_t *pCols, char *pTC );
    void getLines();
    void initStats();
    void makeTable();
    void readFile( const char *filename );

// Utility _____________________________________________________________

    // out: Points to tab or NULL
    char *skipText( char *pText )
    {
        while (*pText )
        {
            if (*pText == '\t' ) break;
            if (*pText ==   0  ) break;
            pText++;
        }
        
        return pText;
    }

    int countTabs( char *pLine )
    {
        int nTabs = 0;
        while( *pLine )
        {
            if (*pLine == '\t') nTabs++;
            pLine++;
        }
        return nTabs;
    }


// Implementation ______________________________________________________


// =====================================================================
void dumpDataForSort( const Treasure_t *pTreasure, const int line )
{
    fprintf( stderr, "\"TC%s %-20s %-29s %-26s\",\n"
        , pTreasure->sTC
        , pTreasure->pBaseName
        , (pTreasure->nSetsLen > 0) ? pTreasure->pSetsName : "---"
        , (pTreasure->nUnqsLen > 0) ? pTreasure->pUnqsName : "---"
    );

    if (gLongestBase.length < pTreasure->nBaseLen)
    {
        gLongestBase.length = pTreasure->nBaseLen;
        gLongestBase.name   = pTreasure->pBaseName;
    }

    if (gLongestSets.length < pTreasure->nSetsLen)
    {
        gLongestSets.length = pTreasure->nSetsLen;
        gLongestSets.name   = pTreasure->pSetsName;
    }

    if (gLongestUnqs.length < pTreasure->nUnqsLen)
    {
        gLongestUnqs.length = pTreasure->nUnqsLen;
        gLongestUnqs.name   = pTreasure->pUnqsName;
    }
}

// =====================================================================
void dumpTreasure( const Treasure_t *pTreasure, const int line )
{
    const char *aPADDING[] =
    {
        "",
        " ",
        "  "
    };
    int iPadding = (line <10) + (line < 100);
    int odd = (line & 1) + 1;

    printf( "<tr class='b%d%d' id='hg%d'%s>", odd, odd, line, aPADDING[ iPadding ] );

        printf( "<td class='tc'>TC%s</td>", pTreasure->sTC );
        printf( "<td>%-20s</td><td>%-2s</td>", pTreasure->pBaseName, pTreasure->pBaseQlvl );

        if( pTreasure->nSetsLen > 0 )
        {
            printf( "<td><button class='hg' id='hgs.%03d' onclick='onToggleHG(this)'></button></td>", giSet++ );
            printf( "<td><span class='set'>%-30s</span></td>", pTreasure->pSetsName );
        }
        else
        {
            printf( "<td><meta                                                             /></td>" );
            printf( "<td><span class='   '>%-30s</span></td>", "---" ); // pTreasure->pSetName
        }

        printf( "<td>%2s</td>", pTreasure->pSetsQlvl  );

        if( pTreasure->nUnqsLen > 0 )
        {
            printf( "<td><button class='hg' id='hgu.%03d' onclick='onToggleHG(this)'></button></td>", giUnq++ );
            printf( "<td><span class='unq'>%-26s</span></td>", pTreasure->pUnqsName );
        }
        else
        {
            printf( "<td><meta                                                             /></td>" );
            printf( "<td><span class='   '>%-26s</span></td>", "---" ); // pTreasure->pUnqName
        }

        printf( "<td>%2s</td>", pTreasure->pUnqsQlvl  );

    printf( "</tr>\n" );
}


/*
    Tab Separated Value
        Base, qlvl
        Set , qlvl
        Unq , qlvl

    NOTE: A base item can be multiple set items.  Those lines MUST be duplicated.

    ALSO: These lines need to manually be patched up:
        Templar's Might	82
        Earth Shifter	77
        Stone Crusher	76
        Nightwing's Veil	75
        Stormlash	86
        Azurewrath	87
        The Redeemer	80
        Milabrega's Diadem	23
        Infernal Sign	7
        Arctic Mitts	3
        Hwanin's Seal	28
        Sander's Paragon	20

    BUGS:
        Azurewrath is listed twice; it used to be a crystal sword in 1.09, in 1.10 it is now a phase blade
        Some sites list it twice.
        https://diablo2.diablowiki.net/Treasure_Classes
*/
// =====================================================================
void getColumns( int iLine, char *pLine, Treasure_t *pCols, char *pTC )
{
    #define NEXT_COL *pText = 0; pText++

    char *pBase;
    char *pSets;
    char *pUnqs;

    memset( pCols, 0, sizeof( Treasure_t ) );
    memcpy( pCols->sTC, pTC, 2 );

    int nTabs = countTabs( pLine );
    if( nTabs < 4)
    {
        printf( "ERROR: Multiple (set/unique) items on line #%d\n", iLine+1 );
        printf( "    %s\n", gaLines[ iLine-1 ] );
        printf( "--> %s\n", gaLines[ iLine   ] );
        printf( "    %s\n", gaLines[ iLine+1 ] );
        printf( "Should be 5 tabs.\n" );
        exit( 3 );
    }

#if DEBUG
    char *pDebug  = pLine;
    int   nOffset = 0;
    while( *pDebug )
    {
        printf( "\t[%2d]: %02X %c\n", nOffset, *pDebug, *pDebug );
        nOffset++;
        pDebug++;;
    }
#endif
    DEBUGF( "DEBUG LINE: '%s'\n", pLine );

    char *pText = pLine;
    pCols->pBaseName = pText; pBase = pText; pText = skipText( pText ); pCols->nBaseLen = (int)(pText - pBase); NEXT_COL; pCols->pBaseQlvl = pText; pText = skipText( pText ); NEXT_COL;
    pCols->pSetsName = pText; pSets = pText; pText = skipText( pText ); pCols->nSetsLen = (int)(pText - pSets); NEXT_COL; pCols->pSetsQlvl = pText; pText = skipText( pText ); NEXT_COL;
    pCols->pUnqsName = pText; pUnqs = pText; pText = skipText( pText ); pCols->nUnqsLen = (int)(pText - pUnqs); NEXT_COL; pCols->pUnqsQlvl = pText; pText = skipText( pText ); NEXT_COL;

    DEBUGF( "DEBUG Set Name: '%s', Len: %d\n", pSets, pCols->nSetsLen );
    DEBUGF( "DEBUG Unq Name: '%s', Len: %d\n", pUnqs, pCols->nUnqsLen );
}


// =====================================================================
void getLines()
{
    gnLines = 0;
    memset( gaLines, 0, sizeof( gaLines ) );

    char *pStart = gaBuffer;
    char *pText  = gaBuffer;
    char *pEnd   = gaBuffer + gnSize;

    while( pText < pEnd )
    {
        // DOS : \r\n
        // Unix:   \n
        if(*pText == '\r')
        {
            if( pText[1] == '\n' )
                *pText = 0;
        }
        if(*pText == '\n')
        {
            *pText = 0;
            gaLines[ gnLines++ ] = pStart;
            pStart = pText+1;
        }
        pText++;
    }
}


// =====================================================================
void initStats()
{
    memset( &gLongestBase.length, 0, sizeof( gLongestBase ) );
    memset( &gLongestSets.length, 0, sizeof( gLongestSets ) );
    memset( &gLongestUnqs.length, 0, sizeof( gLongestUnqs ) );
}


// Process text line convert to Teasure type
// =====================================================================
void makeTable()
{
    const char  *tc = "Treasure Class";
    const size_t tn = strlen( tc );

    char sTC[4] = "??";
    Treasure_t treasure;

    int line = 0;

    gnSet = 0;
    gnUnq = 0;

    // Skip header
    for( int iLine = 1; iLine < gnLines; iLine++ )
    {
        char *pText = gaLines[ iLine ];
        int bCmp  = strncmp( pText, tc, tn );
        int hasTC = bCmp == 0;

#if DEBUG
        if( iLine == 1 )
        {
            printf( "tc: %s\n", tc );
            printf( "tn: %d\n", (int) tn );
            printf( "Text : %s\n", pText );
            printf( "Cmp  : %d\n", bCmp  );
            printf( "hasTC: %d\n", hasTC );
        }
#endif

        if( hasTC )
        {
            memcpy( sTC, pText + tn + 1, 2 );
            sTC[2] = 0;
            DEBUGF( "TC: %s\n", treasure.sTC );
        }
        else
        {
            DEBUGF( "Line #%d: %s\n", iLine, pText );
            getColumns( iLine, pText, &treasure, sTC );
            DEBUGF( "Dump: %s\n", treasure.pBaseName );

            // Holy Grail only tracks sets and uniques
            if( treasure.nSetsLen || treasure.nUnqsLen )
            {
                dumpTreasure( &treasure, line );
                dumpDataForSort( &treasure, iLine ); // For sorting by header
                line++;
            }
        }
    }
    fprintf( stderr, "\n" );
    printf( "\n" );

    gnSet = giSet;
    gnUnq = giUnq;
}


// =====================================================================
void readFile( const char *filename )
{
    FILE *fin = fopen( filename, "rb" );
    if( !fin )
    {
        printf( "ERROR: Couldn't open: %s\n", filename );
        exit( 1 );
    }
    
    fseek( fin, 0, SEEK_END );
    gnSize = ftell( fin );
    fseek( fin, 0, SEEK_SET );

    if (gnSize > BUFFER)
    {
        fclose( fin );
        printf( "ERROR: File > 64KB !\n" );
        exit( 2 );
    }

    fread( gaBuffer, 1, gnSize, fin );
    fclose( fin );


    gaBuffer[ gnSize+1 ] = 0;
}


// =====================================================================
int main()
{
    const char *filename = "all_set_unq_items.txt";
    readFile( filename );
    initStats();
    getLines();
    makeTable();

    fprintf( stderr, "File Size: %u\n", (unsigned) gnSize );
    fprintf( stderr, "File Lines: %d\n", gnLines );
    fprintf( stderr, "\n" );
    fprintf( stderr, "Sets: %d\n", gnSet );
    fprintf( stderr, "Unqs: %d\n", gnUnq );
    fprintf( stderr, "\n" );
    fprintf( stderr, "Max chars in Base: %d, '%s'\n",  gLongestBase.length, gLongestBase.name );
    fprintf( stderr, "Max chars in Sets: %d, '%s'\n",  gLongestSets.length, gLongestSets.name );
    fprintf( stderr, "Max chars in Unqs: %d, '%s'\n",  gLongestUnqs.length, gLongestUnqs.name );

    if( gnLines > 1 )
    {
        DEBUGF( "[0]: %s\n", gaLines[ 0 ] );
        DEBUGF( "[1]: %s\n", gaLines[ 1 ] );
    }

    return 0;
}
