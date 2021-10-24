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

// Types
    struct Treasure_t
    {
        char  sTC[4];
        char *pBaseName;
        char *pBaseQlvl;

        char *pSetName;
        char *pSetQlvl;

        char *pUnqName;
        char *pUnqQlvl;

        size_t nLenSet;
        size_t nLenUnq;
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

    int longestBaseName = 0;

// Prototypes __________________________________________________________
    void dumpTreasure( const Treasure_t *pTreasure, const int line );
    void getColumns( int iLine, char *pLine, Treasure_t *pCols, char *pTC );
    void getLines();
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

        if( pTreasure->nLenSet > 0 )
        {
            printf( "<td><button class='hg' id='hgs.%03d' onclick='onToggleHG(this)'></button></td>", giSet++ );
            printf( "<td><span class='set'>%-30s</span></td>", pTreasure->pSetName );
        }
        else
        {
            printf( "<td><meta                                                             /></td>" );
            printf( "<td><span class='   '>%-30s</span></td>", "---" ); // pTreasure->pSetName
        }

        printf( "<td>%2s</td>", pTreasure->pSetQlvl  );

        if( pTreasure->nLenUnq > 0 )
        {
            printf( "<td><button class='hg' id='hgu.%03d' onclick='onToggleHG(this)'></button></td>", giUnq++ );
            printf( "<td><span class='unq'>%-26s</span></td>", pTreasure->pUnqName );
        }
        else
        {
            printf( "<td><meta                                                             /></td>" );
            printf( "<td><span class='   '>%-26s</span></td>", "---" ); // pTreasure->pUnqName
        }

        printf( "<td>%2s</td>", pTreasure->pUnqQlvl  );

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
*/
// =====================================================================
void getColumns( int iLine, char *pLine, Treasure_t *pCols, char *pTC )
{
    #define NEXT_COL *pText = 0; pText++

    char *pSet;
    char *pUnq;

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
    pCols->pBaseName = pText; pText = skipText( pText ); NEXT_COL;
    pCols->pBaseQlvl = pText; pText = skipText( pText ); NEXT_COL;

    pCols->pSetName = pText; pSet = pText; pText = skipText( pText ); pCols->nLenSet = pText - pSet; NEXT_COL;
    pCols->pSetQlvl = pText;               pText = skipText( pText ); NEXT_COL;

    pCols->pUnqName = pText; pUnq = pText; pText = skipText( pText ); pCols->nLenUnq = pText - pUnq; NEXT_COL;
    pCols->pUnqQlvl = pText;               pText = skipText( pText ); NEXT_COL;

    DEBUGF( "DEBUG Set Name: '%s', Len: %d\n", pSet, (int) pCols->nLenSet );
    DEBUGF( "DEBUG Unq Name: '%s', Len: %d\n", pUnq, (int) pCols->nLenUnq );
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
            if( treasure.nLenSet || treasure.nLenUnq )
                dumpTreasure( &treasure, line++ );
        }
    }
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
    getLines();
    makeTable();

    fprintf( stderr, "Size: %u\n", (unsigned) gnSize );
    fprintf( stderr, "Lines: %d\n", gnLines );

    fprintf( stderr, "Sets: %d\n", gnSet );
    fprintf( stderr, "Unqs: %d\n", gnUnq );

    if( gnLines > 1 )
    {
        DEBUGF( "[0]: %s\n", gaLines[ 0 ] );
        DEBUGF( "[1]: %s\n", gaLines[ 1 ] );
    }

    return 0;
}
