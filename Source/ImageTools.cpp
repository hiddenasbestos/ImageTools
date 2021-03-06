/*

Copyright (c) 2021 David Walters

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <cstdio>
#include <cstring>

#include "utils.h"


//------------------------------------------------------------------------------
// Tool Declarations
//------------------------------------------------------------------------------

typedef int ( *fnTool )( int argc, char** argv );

struct Tool
{
	const char* pName;
	fnTool pFunction;
	const char* pDescription;
	const char* pHelpArgs;
	const char* pHelpDesc;
};

// ... add to this list as new tools are created.
extern int Help( int argc, char** argv );
extern int Export( int argc, char** argv );
extern int Mask( int argc, char** argv );

#define HELP_BLOCK_HEADER																	\
		"  -H###        Add a header. ### is a string of codes as follows:\n\n"				\
		"    1          Byte mode (default).\n"												\
		"    2          Word mode - 2 bytes per entity.\n"									\
		"    L          Use little endian byte order.\n"									\
		"    B          Use big endian byte order (default).\n"								\
		"    n          Number of tiles.\n"													\
		"    w          Width of the output in pixels.\n"									\
		"    p          Pitch of the output in bytes(1) or words(2)\n"						\
		"    h          Height of the output or rows per tile, in pixels.\n"				\
		"    z          Write zero byte(1) or word(2).\n"

#define HELP_BLOCK_PIXEL_FORMAT																\
		"  -pf FMT      Select the pixel format for the output. Default is \"1BPP\"\n\n"	\
		"  The following pixel formats are supported:\n\n"									\
		"    1BPP       8 x 1-bit pixels per byte. (ZX Spectrum, Hercules, etc.)\n"			\
		"    2BPP       4 x 2-bit pixels per byte.\n"										\
		"    CGA        IBM CGA mode 4/5 (320x200,4)\n"										\
		"    CPC0       Amstrad CPC mode 0 (160x200,16)\n"									\
		"    CPC1       Amstrad CPC mode 1 (320x200,4)\n"									\
		"    CPC2       Amstrad CPC mode 2 (640x200,2)\n"									\
		"    GB         Game Boy\n"															\
		"    NES        NES / Famicom\n"													\
		"    SMS        Master System / Game Gear\n"										\
		"    ST0        Atari ST mode 0 (Low)\n"											\
		"    ST1        Atari ST mode 1 (Medium)\n"											\
		"    ST2        Atari ST mode 2 (High)\n"


// ... register the tools
static Tool gTools[] =
{
	{ "help", Help, "Show help for a specific tool. e.g. ImageTools help export", "tool-name", "Show help for a specific tool." },

	//-----------------

	{
		"export", Export, "Export a raw image in a new pixel format.", "<input> <output> [-tile WxH] [-shift R] [-append]\n\t[-2x] [-H###] [-pf format]",
		"  <input>      An image file to read. (Indexed .PNG only)\n\n"
		"  <output>     The output file.\n\n"
		"  -tile WxH    Split the input image into tiles of WxH pixels and output as\n"
		"               concatenated chunks. Tiles are split in row-major order.\n\n"
		
		"  -shift R     Shift output to the right by R pixels.\n"
		"               Not supported by GB, NES or SEGA pixel formats.\n"
		"  -append      Append to the output file, rather than overwriting it.\n"
		"  -2x          Double the width of the input image before exporting.\n"
		"               Not supported by GB, NES or SEGA pixel formats.\n\n"

		HELP_BLOCK_HEADER

		"\n"

		HELP_BLOCK_PIXEL_FORMAT
	},

	{
		"mask", Mask, "Extract a bit mask from an image.", "<input> <output> [-tile WxH] [-index I] [-not]\n\t[-shift R] [-append] [-2x] [-H###] [-pf format]",
		"  <input>      An image file to read. (Indexed .PNG only)\n\n"
		"  <output>     The output file.\n\n"
		"  -tile WxH    Split the input image into tiles of WxH pixels and output as\n"
		"               concatenated chunks. Tiles are split in row-major order.\n\n"
		
		"  -index I     Specify the index of pixels to extract. Default 0.\n"
		"  -not         Invert the output. Including border/shifted area.\n"
		"  -shift R     Shift output to the right by R pixels.\n"
		"               Not supported by GB, NES or SEGA pixel formats.\n"
		"  -append      Append to the output file, rather than overwriting it.\n"
		"  -2x          Double the width of the input image.\n"
		"               Not supported by GB, NES or SEGA pixel formats.\n\n"

		HELP_BLOCK_HEADER

		"\n"

		HELP_BLOCK_PIXEL_FORMAT
	},
};

// ... how many tools?
static int gToolsCount = sizeof( gTools ) / sizeof( Tool );


//------------------------------------------------------------------------------
// Global Data
//------------------------------------------------------------------------------

const char* gpActiveToolName = nullptr;


//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

static int findTool( const char* pName )
{
	for ( int i = 0; i < gToolsCount; ++i )
	{
		// Alias the tool
		const Tool& tool = gTools[ i ];

		// Match?
		if ( _strcmpi( pName, tool.pName ) == 0 )
		{
			return i;
		}
	}

	// Not found.
	return -1;
}

static void printHello()
{
	printf( "\n------------------------------------------------------------------\n"
			" ImageTools Utility Collection\n"
			" Copyright (c) 2021, by David Walters. See LICENSE for details.\n"
			"------------------------------------------------------------------\n\n" );
}

static void printUsage()
{
	// Usage
	printf( "USAGE: ImageTools tool [args ...]\n\n" );

	// Files
	printf( "Specify the tool to use followed by its arguments.\n\n" );

	for ( int i = 0; i < gToolsCount; ++i )
	{
		// Alias the tool
		const Tool& tool = gTools[ i ];

		// List it.
		printf( "    %-12s : %s\n", tool.pName, tool.pDescription );

		// formatting
		if ( i == 0 )
		{
			putchar( '\n' );
		}
	}
}

void PrintHelp( const char* pName )
{
	int iTool = findTool( pName );

	if ( iTool < 0 )
	{
		PrintError( "Unknown tool \"%s\". Cannot display help.", pName );
		return;
	}
	else
	{
		// Alias the tool
		const Tool& tool = gTools[ iTool ];

		// Hello
		printHello();

		// Usage
		printf( "%s\n\nUSAGE: ImageTools %s %s\n\n%s\n", tool.pDescription, pName, tool.pHelpArgs, tool.pHelpDesc );
	}
}

static int Help( int argc, char** argv )
{
	if ( argc <= 2 )
	{
		printHello();
		printUsage();
		return 0;
	}

	char* pName = argv[ 2 ];
	PrintHelp( pName );

	return 0;
}

//==============================================================================

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main( int argc, char** argv )
{
	int iReturnCode = 0;

#ifdef _DEBUG
	PrintRuler( 80 );
#endif // _DEBUG

	if ( argc < 2 )
	{
		printHello();
		printUsage();
	}
	else
	{
		char* pName = argv[ 1 ];
		int iTool = findTool( pName );

		if ( iTool < 0 )
		{
			PrintError( "Unknown tool \"%s\".", pName );

			printHello();
			printUsage();
		}
		else
		{
			// Alias the tool
			const Tool& tool = gTools[ iTool ];

			// Store the name
			gpActiveToolName = tool.pName;

			// Call it!
			iReturnCode = tool.pFunction( argc, argv );
		}
	}

#ifdef _DEBUG
	printf( "\nFinished. Press Enter... " );
	getchar();
#endif

	// Done
	return iReturnCode;
}

//==============================================================================

