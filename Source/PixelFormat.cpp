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
#include <cstdint>

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
	else if ( _stricmp( pArg, "2bpp" ) == 0 )
	{
		return PixelFormat::PACKED_2;
	}
	else if ( _stricmp( pArg, "cga" ) == 0 )
	{
		return PixelFormat::IBM_CGA;
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
	else if ( _stricmp( pArg, "sega" ) == 0 )
	{
		return PixelFormat::SEGA_VDP;
	}
	else if ( _stricmp( pArg, "gb" ) == 0 )
	{
		return PixelFormat::GAMEBOY;
	}
	else if ( _stricmp( pArg, "nes" ) == 0 )
	{
		return PixelFormat::NES;
	}

	return PixelFormat::UNKNOWN;
}

//------------------------------------------------------------------------------
// PixelFormatToString
//------------------------------------------------------------------------------
const char* PixelFormatToString( PixelFormat pf )
{
	switch ( pf )
	{

	case PixelFormat::PACKED_1:
		return "packed 1-BPP";

	case PixelFormat::PACKED_2:
		return "packed 2-BPP";

	case PixelFormat::IBM_CGA:
		return "IBM CGA";

	case PixelFormat::ATART_ST_M0:
		return "Atari ST mode 0";

	case PixelFormat::ATART_ST_M1:
		return "Atari ST mode 1";

	case PixelFormat::ATART_ST_M2:
		return "Atari ST mode 2";

	case PixelFormat::AMSTRAD_CPC_M0:
		return "Amstrad CPC mode 0";

	case PixelFormat::AMSTRAD_CPC_M1:
		return "Amstrad CPC mode 1";

	case PixelFormat::AMSTRAD_CPC_M2:
		return "Amstrad CPC mode 2";

	case PixelFormat::SEGA_VDP:
		return "SEGA VDP";

	case PixelFormat::GAMEBOY:
		return "Game Boy";

	case PixelFormat::NES:
		return "NES/Famicom";

	}

	return nullptr;
}

//------------------------------------------------------------------------------
// PixelFormatMaxIndex
//------------------------------------------------------------------------------
uint32_t PixelFormatMaxIndex( PixelFormat format )
{
	switch ( format )
	{

	default:
	case PixelFormat::UNKNOWN:
	case PixelFormat::CHUNKY_16:
	case PixelFormat::CHUNKY_32:
		return 0; // Invalid

	case PixelFormat::PACKED_1:
	case PixelFormat::ATART_ST_M2:
	case PixelFormat::AMSTRAD_CPC_M2:
		return 1 << 1; // 2 colours

	case PixelFormat::PACKED_2:
	case PixelFormat::ATART_ST_M1:
	case PixelFormat::AMSTRAD_CPC_M1:
	case PixelFormat::IBM_CGA:
	case PixelFormat::NES:
	case PixelFormat::GAMEBOY:
		return 1 << 2; // 4 colours

	case PixelFormat::PACKED_4:
	case PixelFormat::ATART_ST_M0:
	case PixelFormat::AMSTRAD_CPC_M0:
	case PixelFormat::SEGA_VDP:
		return 1 << 4; // 16 colours
		break;

	case PixelFormat::CHUNKY_8:
		return 1 << 8; // 256 colours
		break;

	}; // switch ( format )
}
