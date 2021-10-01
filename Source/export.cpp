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

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "utils.h"
#include "Image.h"
#include "ImageInfo.h"

//==============================================================================

struct Options
{
	int iShift = 0;
	const char* pInputName = nullptr;
	const char* pOutputName = nullptr;
	PixelFormat dataOutFormat = PixelFormat::PACKED_1;
	bool bAppend = false;
	std::string header;
};

static int ParseArgs( int argc, char** argv, Options& opt )
{
	enum eOption
	{
		NONE,
		OPT_SHIFT,
		OPT_PIXEL_FORMAT,
	};

	eOption specialNextArg = NONE;

	// defaults.

	// parse arguments (after the tool name)
	for ( int i = 2; i < argc; ++i )
	{
		const char* pArg = argv[ i ];

		if ( specialNextArg != NONE )
		{
			switch ( specialNextArg )
			{

			case OPT_PIXEL_FORMAT:

				opt.dataOutFormat = DecodePixelFormat( pArg );

				if ( opt.dataOutFormat == PixelFormat::UNKNOWN )
				{
					// error.
					PrintError( "Invalid -pf parameter \"%s\".", pArg );
					return 1;
				}

				break;

			case OPT_SHIFT:

				{
					int iValue;
					char* pEnd = nullptr;
					iValue = strtol( pArg, &pEnd, 10 );

					if ( iValue >= 0 )
					{
						opt.iShift = iValue;
					}
					else if ( *pEnd != 0 )
					{
						// error.
						PrintError( "Invalid -shift parameter \"%s\".", pArg );
						return 1;
					}
					else
					{
						// error.
						PrintError( "Invalid -shift %d. Must be 0 - 255.", iValue );
						return 1;
					}
				}

				break;

			}

			specialNextArg = NONE;
		}
		else if ( *pArg == '-' )
		{
			if ( _stricmp( pArg, "-shift" ) == 0 )
			{
				specialNextArg = OPT_SHIFT;
			}
			else if ( _stricmp( pArg, "-pf" ) == 0 )
			{
				specialNextArg = OPT_PIXEL_FORMAT;
			}
			else if ( _stricmp( pArg, "-append" ) == 0 )
			{
				opt.bAppend = true;
			}
			else if ( pArg[ 1 ] == 'H' )
			{
				opt.header = pArg + 2;
			}
			else
			{
				// error.
				PrintError( "Invalid parameter \"%s\".", pArg );
				return 1;
			}
		}
		else if ( opt.pInputName == nullptr )
		{
			opt.pInputName = pArg;
		}
		else if ( opt.pOutputName == nullptr )
		{
			opt.pOutputName = pArg;
		}
		else
		{
			// error.
			PrintError( "Invalid parameter \"%s\".", pArg );
			return 1;
		}
	}

	if ( opt.pInputName == nullptr || opt.pOutputName == nullptr )
	{
		PrintHelp( "export" );
		return 1;
	}

	return 0; // OK
}

//==============================================================================

static void BuildOutput( const Image& image, ImageInfo& imageInfo, const Options& opt, Image& output )
{
	output.Create( opt.dataOutFormat, imageInfo.width + opt.iShift, imageInfo.height );

	// Border pixels are implicitly index zero. TODO: Customise option?
	uint32_t borderValue = 0;

	for ( uint32_t y = 0; y < imageInfo.height; ++y )
	{
		// clear space revealed by shifting
		for ( int x = 0; x < opt.iShift; ++x )
		{
			output.Plot( x, y, borderValue );
		}

		for ( uint32_t x = 0; x < imageInfo.width; ++x )
		{
			// Read the index at this position.
			uint32_t data = image.Peek( x, y );

			// Output to export. Apply shift. Excess bits are ignored.
			output.Plot( x + opt.iShift, y, data );
		}

		// right-hand border, clear padding to end of allocated row
 		for ( int x = imageInfo.width + opt.iShift; x < output.GetStride(); ++x )
 		{
			output.Plot( x, y, borderValue );
 		}
	}
}

//==============================================================================

//------------------------------------------------------------------------------
// Export
//------------------------------------------------------------------------------
int Export( int argc, char** argv )
{
	Options opt;
	Image image;
	ImageInfo imageInfo;

	// Get options
	if ( ParseArgs( argc, argv, opt ) )
	{
		return 1; // ERROR
	}

	// Load image
	if ( LoadImage( opt.pInputName, image, imageInfo ) )
	{
		return 1; // ERROR
	}

	// Build output
	Image mask;
	Info( "Exporting '" );

	PrintPixelFormat( opt.dataOutFormat );

	printf( "' format raw image.\n" );
	
	if ( opt.iShift )
	{
		Info( "Output is shifted right by %d pixels.\n", opt.iShift );
	}

	BuildOutput( image, imageInfo, opt, mask );

	// Write output
	if ( WriteImage_Fbin( mask, opt.pOutputName, opt.header, opt.bAppend ) )
	{
		return 1; // ERROR
	}

	return 0;
}

//==============================================================================
