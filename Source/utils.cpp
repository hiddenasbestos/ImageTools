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
#include <cstdlib>
#include <errno.h>
#include <cstring>
#include <cstdint>
#include <stdarg.h>

#include "utils.h"
#include "Image.h"
#include "ImageInfo.h"
#include "FileReader.h"
#include "Loader.h"

// from ImageTools.cpp
extern const char* gpActiveToolName;


//------------------------------------------------------------------------------
// NextPowerTwo
//------------------------------------------------------------------------------
uint32_t NextPowerTwo( uint32_t n )
{
	--n;
	n |= n >> 16;
	n |= n >> 8;
	n |= n >> 4;
	n |= n >> 2;
	n |= n >> 1;
	++n;
	return n;
}

//------------------------------------------------------------------------------
// DetectHexPrefix
//------------------------------------------------------------------------------
int DetectHexPrefix( const char* pStr )
{
	// Single character prefixes.
	if ( pStr[ 0 ] == '$' || pStr[ 0 ] == '&' )
		return 1;

	// 0x prefix.
	if ( pStr[ 0 ] == '0' && ( pStr[ 1 ] == 'x' || pStr[ 1 ] == 'X' ) )
		return 2;

	// Nope.
	return 0;
}

//------------------------------------------------------------------------------
// PrintRuler
//------------------------------------------------------------------------------
void PrintRuler( int columns )
{
	for ( int i = 0; ( i + 10 ) <= columns; i += 10 )
	{
		printf( "-------%02d!", i + 10 );
	}

	putchar( '\n' );
}

//------------------------------------------------------------------------------
// DebugCmdArgs
//------------------------------------------------------------------------------
void DebugCmdArgs( int argc, char** argv )
{
	printf( "argc = %d\n", argc );

	for ( int i = 0; i < argc; ++i )
	{
		printf( "arg[ %d ] = %s\n", i, argv[ i ] );
	}
}

//------------------------------------------------------------------------------
// FindArg
//------------------------------------------------------------------------------
int FindArg( const char* pArg, int argc, char** argv )
{
	// skip arg[0] as it's the command name.
	for ( int i = 1; i < argc; ++i )
	{
		if ( _stricmp( pArg, argv[ i ] ) == 0 )
		{
			return i;
		}
	}

	// not found.
	return -1;
}

//------------------------------------------------------------------------------
// ParseWithSizeSuffix
//------------------------------------------------------------------------------
int64_t ParseSizeWithSuffix( const char* pStr )
{
	if ( pStr == nullptr )
		return -1;

	int iHexOffset;
	iHexOffset = DetectHexPrefix( pStr );

	int64_t iSize;
	char* pNumberEnd;

	errno = 0;

	if ( iHexOffset )
	{
		// Convert from hex.
		iSize = strtol( pStr + iHexOffset, &pNumberEnd, 16 );
	}
	else
	{
		// Convert from decimal
		iSize = strtol( pStr, &pNumberEnd, 10 );
	}

	if ( errno == ERANGE || pStr == pNumberEnd || iSize < 0 )
	{
		return -1;
	}

	// Suffix?
	if ( _stricmp( pNumberEnd, "KB" ) == 0 )
	{
		iSize *= 1024;
	}
	else if ( _stricmp( pNumberEnd, "MB" ) == 0 )
	{
		iSize *= 1048576;
	}
	else if ( _stricmp( pNumberEnd, "MBit" ) == 0 )
	{
		iSize *= 131072;
	}
	else if ( _stricmp( pNumberEnd, "h" ) == 0 )
	{
		if ( iHexOffset )
		{
			// Don't allow 0x123h !
			return -1;
		}
		else
		{
			// Convert from hex.
			errno = 0;
			iSize = strtol( pStr + iHexOffset, &pNumberEnd, 16 );

			if ( errno == ERANGE || pStr == pNumberEnd )
			{
				return -1;
			}
		}
	}
	else if ( *pNumberEnd != 0 )
	{
		return -2; // something, but unknown
	}

	return iSize;
}

//------------------------------------------------------------------------------
// ParseValue
//------------------------------------------------------------------------------
int ParseValue( const char* pStr, int iLimit )
{
	if ( pStr == nullptr )
		return -1;

	int iHexOffset;
	iHexOffset = DetectHexPrefix( pStr );

	int64_t iSize;
	char* pNumberEnd;

	errno = 0;

	if ( iHexOffset )
	{
		// Convert from hex.
		iSize = strtol( pStr + iHexOffset, &pNumberEnd, 16 );
	}
	else
	{
		// Convert from decimal
		iSize = strtol( pStr, &pNumberEnd, 10 );
	}

	if ( errno == ERANGE || pStr == pNumberEnd || iSize < 0 )
	{
		return -1;
	}

	// Suffix?
	if ( _stricmp( pNumberEnd, "h" ) == 0 )
	{
		if ( iHexOffset )
		{
			// Don't allow 0x123h !
			return -1;
		}
		else
		{
			// Convert from hex.
			errno = 0;
			iSize = strtol( pStr + iHexOffset, &pNumberEnd, 16 );

			if ( errno == ERANGE || pStr == pNumberEnd )
			{
				return -1;
			}
		}
	}
	else if ( *pNumberEnd != 0 )
	{
		return -2; // something, but unknown
	}

	// too big?
	if ( iSize > static_cast<int64_t>( iLimit ) )
	{
		return -1;
	}

	return static_cast<int>( iSize );
}

//------------------------------------------------------------------------------
// TestParsingSizes
//------------------------------------------------------------------------------
void TestParsingSizes()
{
	PrintRuler( 80 );
	printf( "Test function to develop/debug ParseWithSizeSuffix function.\n" );

	for ( ; ; )
	{
		char buf[ 1024 ];
		printf( "\n> " );
		char* pStr = gets_s( buf, sizeof( buf ) );

		int64_t val;
		val = ParseSizeWithSuffix( pStr );

		printf( "val = %lld\n", val );
	}
}

//------------------------------------------------------------------------------
// PrintError
//------------------------------------------------------------------------------
void PrintError( const char* pName, ... )
{
	char buffer[ 2048 ];

	int count;

	va_list	vl;
	va_start( vl, pName );
	count = vsprintf_s( buffer, sizeof( buffer ), pName, vl );
	va_end( vl );

	if ( gpActiveToolName )
	{
		printf( "%s:", gpActiveToolName );
	}
	else
	{
		printf( "ImageTools:" );
	}

	printf( " ERROR: %s\n", buffer );
}

//------------------------------------------------------------------------------
// Info
//------------------------------------------------------------------------------
void Info( const char* pName, ... )
{
	char buffer[ 2048 ];

	int count;

	va_list	vl;
	va_start( vl, pName );
	count = vsprintf_s( buffer, sizeof( buffer ), pName, vl );
	va_end( vl );

	if ( gpActiveToolName )
	{
		printf( "%s:", gpActiveToolName );
	}
	else
	{
		printf( "ImageTools: " );
	}

	printf( " %s", buffer );
}

//------------------------------------------------------------------------------
// LoadImage
//------------------------------------------------------------------------------
int LoadImage( const char* pInputName, Image& image, ImageInfo& imageInfo, eLoadImageMode loadImageMode )
{
	FileReader reader;
	Loader imgLoader;

	Info( "Loading \"%s\" ... ", pInputName );

	// Load image
	bool bLoadResult = false;
	if ( reader.LoadFile( pInputName ) )
	{
		switch ( loadImageMode )
		{

		case eLoadImageMode::DEFAULT:
			bLoadResult = imgLoader.LoadTo( reader, image, &imageInfo, Loader::WANT_IDX8 );
			break;

		case eLoadImageMode::SCALE_2X:

			{
				Image temp;

				bLoadResult = imgLoader.LoadTo( reader, temp, &imageInfo, Loader::WANT_IDX8 );

				if ( bLoadResult )
				{
					// Duplicate image at 2x width.
					image.Create( temp.GetPixelFormat(), temp.GetWidth() * 2, temp.GetHeight() );

					for ( uint32_t y = 0; y < temp.GetHeight(); ++y )
					{
						for ( uint32_t x = 0; x < temp.GetWidth(); ++x )
						{
							// ... read source image.
							uint32_t copy;
							copy = temp.Peek( x, y );

							// ... write to image, twice.
							image.Plot( x * 2, y, copy );
							image.Plot( ( x * 2 ) + 1, y, copy );
						}
					}

					// Patch image info
					imageInfo.width *= 2;
				}
			}

			break;

		}; // switch ( loadImageMode )
	}

	// Failed?
	if ( bLoadResult )
	{
		switch ( loadImageMode )
		{

		case eLoadImageMode::DEFAULT:
			printf( "OK (%dx%d)\n", imageInfo.width, imageInfo.height );
			break;

		case eLoadImageMode::SCALE_2X:
			printf( "OK (%dx%d) [2x]\n", imageInfo.width, imageInfo.height );
			break;

		}
	}
	else
	{
		PrintError( "Failed to load image." );
		return 1;
	}

	return 0;
}

//------------------------------------------------------------------------------
// PrintImage
//------------------------------------------------------------------------------
void PrintImage( Image& image )
{
	for ( int y = 0; y < image.GetHeight(); ++y )
	{
		uint8_t* p = image.GetRowPtr( y );
		for ( int i = 0; i < image.GetPitch(); ++i )
		{
			printf( "%02X", p[ i ] );
		}
		printf( "\n" );
	}
}


static void write_value_helper( FILE* fp_out, int iSize, bool bLittleEnd, uint32_t data )
{
	switch ( iSize )
	{

	case 1:
		fputc( static_cast<uint8_t>( data ), fp_out );
		break;

	case 2:
		if ( bLittleEnd )
		{
			fwrite( &data, 2, 1, fp_out );
		}
		else
		{
			fputc( static_cast<uint8_t>( data >> 8 ), fp_out );
			fputc( static_cast<uint8_t>( data ), fp_out );
		}
		break;

	}
}

//------------------------------------------------------------------------------
// WriteOutHeader
//------------------------------------------------------------------------------
void WriteOutHeader( Image& image, std::string& header, FILE* fp_out, int iTileCount, int iTileHeight )
{
	// State
	int iSize = 1;
	bool bLittleEnd = false; // Default: Big endian

	for ( size_t i = 0; i < header.length(); ++i )
	{
		const char ch = header[ i ];

		switch ( ch )
		{

		case '1':
			iSize = 1;
			break;

		case '2':
			iSize = 2;
			break;

		case 'L':
			bLittleEnd = true;
			break;

		case 'B':
			bLittleEnd = false; // Big endian
			break;

		case 'z':
			write_value_helper( fp_out, iSize, bLittleEnd, 0 );
			break;

		case 'p':
			write_value_helper( fp_out, iSize, bLittleEnd, image.GetPitch() / iSize );
			break;

		case 'w':
			write_value_helper( fp_out, iSize, bLittleEnd, image.GetWidth() );
			break;

		case 'h':
			write_value_helper( fp_out, iSize, bLittleEnd, iTileHeight );
			break;

		case 'n':
			write_value_helper( fp_out, iSize, bLittleEnd, iTileCount );
			break;

		}; // switch ( ch )
	}
}

//------------------------------------------------------------------------------
// WriteImage
//------------------------------------------------------------------------------
void WriteImage( Image& image, FILE* fp_out )
{
	for ( int y = 0; y < image.GetHeight(); ++y )
	{
		uint8_t* p = image.GetRowPtr( y );
		for ( int i = 0; i < image.GetPitch(); ++i )
		{
			fputc( p[ i ], fp_out );
		}
	}
}

//------------------------------------------------------------------------------
// ValidateLoadImageMode
//------------------------------------------------------------------------------
void ValidateLoadImageMode( PixelFormat pf, eLoadImageMode& mode )
{
	if ( mode == eLoadImageMode::SCALE_2X )
	{
		if ( pf == PixelFormat::GAMEBOY ||
			 pf == PixelFormat::MASTER_SYSTEM ||
			 pf == PixelFormat::NES )
		{
			// Disable
			Info( "WARNING: -2x is not supported for this pixel format.\n" );
			mode = eLoadImageMode::DEFAULT;
		}
	}
}

//------------------------------------------------------------------------------
// ValidateShift
//------------------------------------------------------------------------------
void ValidateShift( PixelFormat pf, int& iShift )
{
	if ( iShift )
	{
		if ( pf == PixelFormat::GAMEBOY ||
			 pf == PixelFormat::MASTER_SYSTEM ||
			 pf == PixelFormat::NES )
		{
			// Disable
			Info( "WARNING: -shift is not supported for this pixel format.\n" );
			iShift = 0;
		}
	}
}

//------------------------------------------------------------------------------
// WriteImage_Fbin
//------------------------------------------------------------------------------
int WriteImage_Fbin( Image& image, const char* pOutputName, std::string& header, bool bAppend, int iTileCount, int iTileHeight )
{
	int err;
	FILE* fp_out;

	// ... output file
	err = fopen_s( &fp_out, pOutputName, bAppend ? "ab" : "wb" );
	if ( err != 0 || fp_out == nullptr )
	{
		PrintError( "Cannot open output file \"%s\"", pOutputName );
		return 1;
	}

	// Write
	if ( bAppend )
	{
		Info( "Appending" );
	}
	else
	{
		Info( "Writing" );
	}
	printf( " \"%s\" ... ", pOutputName );

	// Header
	WriteOutHeader( image, header, fp_out, iTileCount, iTileHeight );
	
	// Image
	WriteImage( image, fp_out );
	
	printf( "DONE (%d bytes)\n", ftell( fp_out ) );
	fclose( fp_out );

	return 0;
}

