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

#include <cstring>
#include <cstdio>

#include "PixelFormat.h"

//------------------------------------------------------------------------------
// DecodePixelFormat
//------------------------------------------------------------------------------
PixelFormat DecodePixelFormat( const char* pArg )
{
	if ( _stricmp( pArg, "1bpp" ) == 0 )
	{
		return PixelFormat::PACKED_1;
	}
	else if ( _stricmp( pArg, "st0" ) == 0 )
	{
		return PixelFormat::ATART_ST_M0;
	}
	else if ( _stricmp( pArg, "st1" ) == 0 )
	{
		return PixelFormat::ATART_ST_M1;
	}
	else if ( _stricmp( pArg, "st2" ) == 0 )
	{
		return PixelFormat::ATART_ST_M2;
	}
	else if ( _stricmp( pArg, "cpc0" ) == 0 )
	{
		return PixelFormat::AMSTRAD_CPC_M0;
	}
	else if ( _stricmp( pArg, "cpc1" ) == 0 )
	{
		return PixelFormat::AMSTRAD_CPC_M1;
	}
	else if ( _stricmp( pArg, "cpc2" ) == 0 )
	{
		return PixelFormat::AMSTRAD_CPC_M2;
	}

	return PixelFormat::UNKNOWN;
}

//------------------------------------------------------------------------------
// PrintPixelFormat
//------------------------------------------------------------------------------
void PrintPixelFormat( PixelFormat pf )
{
	switch ( pf )
	{

	case PixelFormat::PACKED_1:
		printf( "packed 1-BPP" );
		break;

	case PixelFormat::ATART_ST_M0:
		printf( "Atari ST mode 0" );
		break;

	case PixelFormat::ATART_ST_M1:
		printf( "Atari ST mode 1" );
		break;

	case PixelFormat::ATART_ST_M2:
		printf( "Atari ST mode 2" );
		break;

	case PixelFormat::AMSTRAD_CPC_M0:
		printf( "Amstrad CPC mode 0" );
		break;

	case PixelFormat::AMSTRAD_CPC_M1:
		printf( "Amstrad CPC mode 1" );
		break;

	case PixelFormat::AMSTRAD_CPC_M2:
		printf( "Amstrad CPC mode 2" );
		break;

	}
}
