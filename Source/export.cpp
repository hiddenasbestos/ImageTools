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

struct OptionsExport
{
	int iShift = 0;
	const char* pInputName = nullptr;
	const char* pOutputName = nullptr;
	PixelFormat dataOutFormat = PixelFormat::PACKED_1;
	bool bAppend = false;
	int iTileW = 0;
	int iTileH = 0;
	eLoadImageMode loadImageMode = eLoadImageMode::DEFAULT;

	std::string header;
};

static int ParseArgs( int argc, char** argv, OptionsExport& opt )
{
	enum eOption
	{
		NONE,
		OPT_SHIFT,
		OPT_PIXEL_FORMAT,
		OPT_TILE,
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

			case OPT_TILE:

				{
					int iValue;
					char* pEnd = nullptr;
					iValue = strtol( pArg, &pEnd, 10 );

					if ( iValue > 0 )
					{
						opt.iTileW = iValue;

						pEnd++;

						iValue = strtol( pEnd, nullptr, 10 );
						if ( iValue > 0 )
						{
							opt.iTileH = iValue;
						}
					}

					if ( opt.iTileW == 0 || opt.iTileH == 0 )
					{
						// error.
						PrintError( "Invalid -tile parameter \"%s\".", pArg );
						return 1;
					}
				}

				break;

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
			else if ( _stricmp( pArg, "-tile" ) == 0 )
			{
				specialNextArg = OPT_TILE;
			}
			else if ( _stricmp( pArg, "-append" ) == 0 )
			{
				opt.bAppend = true;
			}
			else if ( _stricmp( pArg, "-2x" ) == 0 )
			{
				opt.loadImageMode = eLoadImageMode::SCALE_2X;
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

static void BuildOutput( const Image& image, ImageInfo& imageInfo, const OptionsExport& opt, Image& output )
{
	// Border pixels are implicitly index zero. TODO: Customise option?
	uint32_t borderValue = 0;

	if ( opt.iTileW )
	{
		//
		// -- TILE MODE

		int iTilesX = image.GetWidth() / opt.iTileW;
		int iTilesY = image.GetHeight() / opt.iTileH;

		output.Create( opt.dataOutFormat, opt.iTileW + opt.iShift, opt.iTileH * iTilesX * iTilesY );

		// For each tile (row-major order)
		for ( int ity = 0; ity < iTilesY; ++ity )
		{
			for ( int itx = 0; itx < iTilesX; ++itx )
			{
				// tile source position.
				int index = itx + ity * iTilesX;
				int src_x0 = itx * opt.iTileW;
				int src_y0 = ity * opt.iTileH;

				// output position
				int dst_y0 = index * opt.iTileH;

				// Copy tile
				for ( int iy = 0; iy < opt.iTileH; ++iy )
				{
					const int y = dst_y0 + iy;

					// clear space revealed by shifting
					for ( int x = 0; x < opt.iShift; ++x )
					{
						output.Plot( x, y, borderValue );
					}

					// copy tile row
					for ( int x = 0; x < opt.iTileW; ++x )
					{
						// Read the pixel at this position.
						uint32_t data = image.Peek( src_x0 + x, src_y0 + iy );

						// Output to export. Apply shift. Excess bits are ignored.
						output.Plot( x + opt.iShift, y, data );
					}

					// right-hand border, clear padding to end of allocated row
					for ( int x = imageInfo.width + opt.iShift; x < output.GetStride(); ++x )
					{
						output.Plot( x, y, borderValue );
					}
				}

			}; // for each source column
		
		}; // for each source row
	}
	else
	{
		//
		// -- WHOLE IMAGE

		output.Create( opt.dataOutFormat, imageInfo.width + opt.iShift, imageInfo.height );

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
}

//==============================================================================

//------------------------------------------------------------------------------
// Export
//------------------------------------------------------------------------------
int Export( int argc, char** argv )
{
	OptionsExport opt;
	Image image;
	ImageInfo imageInfo;

	// Get options
	if ( ParseArgs( argc, argv, opt ) )
	{
		return 1; // ERROR
	}

	// Validate load mode.
	ValidateLoadImageMode( opt.dataOutFormat, opt.loadImageMode );

	// Load image
	if ( LoadImage( opt.pInputName, image, imageInfo, opt.loadImageMode ) )
	{
		return 1; // ERROR
	}

	// Within acceptable maximum index?
	const uint32_t uMaxPermittedIndex = PixelFormatMaxIndex( opt.dataOutFormat );
	if ( uMaxPermittedIndex > 0 && imageInfo.uMaxIndex >= uMaxPermittedIndex )
	{
		Info( "WARNING: Image contains an index (#%d) which exceeds the maximum limit.\n", imageInfo.uMaxIndex );
		Info( "WARNING: Pixel format requires indices from 0 to %d.\n", uMaxPermittedIndex - 1 );
	}

	// Build output
	Image output;
	Info( "Exporting '%s' format raw image.\n", PixelFormatToString( opt.dataOutFormat ) );
	
	// Shift / validated
	ValidateShift( opt.dataOutFormat, opt.iShift );
	if ( opt.iShift )
	{
		Info( "Output is shifted right by %d pixels.\n", opt.iShift );
	}

	int tileCount;
	if ( opt.iTileW )
	{
		tileCount = ( imageInfo.width / opt.iTileW ) * ( imageInfo.height / opt.iTileH );

		if ( tileCount <= 0 )
		{
			PrintError( "Image is too small to create tiles." );
			return 1; // ERROR
		}

		Info( "Splitting input into %d tiles of %dx%d pixels.\n", tileCount, opt.iTileW, opt.iTileH );
	}
	else
	{
		// patch data for header system.
		tileCount = 1; // whole image is one "tile"
		opt.iTileH = output.GetHeight();
	}

	BuildOutput( image, imageInfo, opt, output );

	// Write output
	if ( WriteImage_Fbin( output, opt.pOutputName, opt.header, opt.bAppend, tileCount, opt.iTileH ) )
	{
		return 1; // ERROR
	}

	return 0;
}

//==============================================================================
