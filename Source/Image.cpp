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

#include <cstdlib>
#include <cstring>

#include "Image.h"

Image::Image() : 
	
	_pData( nullptr ),

	_width( 0 ),
	_height( 0 ),
	_pitch( 0 ),
	_stride( 0 )

{
	//
}

void Image::Plot( int x, int y, uint32_t data )
{
	uint32_t offset;

	switch ( _pixelFmt )
	{

	case PixelFormat::UNKNOWN:
		break;
	
	case PixelFormat::PACKED_1:
	case PixelFormat::AMSTRAD_CPC_M2:
		{
			offset = ( x >> 3 ) + y * _pitch;
			uint8_t* p = _pData + offset;

			uint8_t mask;
			mask = 1 << ( 7- ( x & 7 ) );

			if ( data & 1 )
			{
				*p |= mask; // set
			}
			else
			{
				*p &= ~mask; // clear
			}
		}
		break;

	case PixelFormat::PACKED_2:
	case PixelFormat::IBM_CGA:
		{
			offset = ( x >> 2 ) + y * _pitch;
			uint8_t* p = _pData + offset;

			data &= 0x3; // extract 4 colour value

			uint8_t shift;
			shift = ( 3 - ( x & 3 ) ) << 1;

			// ... mask
			*p &= ~( 3 << shift );

			// ... insert
			*p |= ( data << shift );
		}
		break;

	case PixelFormat::PACKED_4:
		{
			offset = ( x >> 1 ) + y * _pitch;
			uint8_t* p = _pData + offset;

			if ( x & 1 )
			{
				// set least nibble
				*p &= 0xF0;
				*p |= ( data & 0xF );
			}
			else
			{
				// set most nibble
				*p &= 0x0F;
				*p |= ( data << 4 ) & 0xF0;
			}
		}
		break;
	
	case PixelFormat::CHUNKY_8:
		{
			offset = x + y * _pitch;
			_pData[ offset ] = static_cast<uint8_t>( data & 0xFF );
		}
		break;
	
	case PixelFormat::CHUNKY_16:
		{
			offset = ( x * 2 ) + y * _pitch;
			uint16_t* p = reinterpret_cast<uint16_t*>( &( _pData[ offset ] ) );
			*p = static_cast<uint16_t>( data & 0xFFFF );
		}
		break;
	
	case PixelFormat::CHUNKY_32:
		{
			offset = ( x * 4 ) + y * _pitch;
			uint32_t* p = reinterpret_cast<uint32_t*>( &( _pData[ offset ] ) );
			*p = data;
		}
		break;

	case PixelFormat::ATART_ST_M0:
		{
			// ... select pixel within the 16-pixel plane.
			uint16_t block = ( x >> 4 );
			uint16_t mask = 1 << ( x & 0xF );

			// ... offset to first bit-plane
			offset = ( block * 8 ) + ( y * _pitch );
			uint16_t* p = reinterpret_cast<uint16_t*>( &( _pData[ offset ] ) );
			
			// ... set/clear bit planes
			if ( data & 1 ) { p[ 0 ] |= mask; }			else { p[ 0 ] &= ~mask; }
			if ( data & 2 ) { p[ 1 ] |= mask; }			else { p[ 1 ] &= ~mask; }
			if ( data & 4 ) { p[ 2 ] |= mask; }			else { p[ 2 ] &= ~mask; }
			if ( data & 8 ) { p[ 3 ] |= mask; }			else { p[ 3 ] &= ~mask; }
		}
		break;

	case PixelFormat::ATART_ST_M1:
		{
			// ... select pixel within the 16-pixel plane.
			uint16_t block = ( x >> 4 );
			uint16_t mask = 1 << ( x & 0xF );

			// ... offset to first bit-plane
			offset = ( block * 4 ) + ( y * _pitch );
			uint16_t* p = reinterpret_cast<uint16_t*>( &( _pData[ offset ] ) );

			// ... set/clear bit planes
			if ( data & 1 ) { p[ 0 ] |= mask; }			else { p[ 0 ] &= ~mask; }
			if ( data & 2 ) { p[ 1 ] |= mask; }			else { p[ 1 ] &= ~mask; }
		}
		break;

	case PixelFormat::ATART_ST_M2:
		{
			// ... select pixel within the 16-pixel plane.
			uint16_t block = ( x >> 4 );
			uint16_t mask = 1 << ( x & 0xF );

			// ... offset to bit-plane
			offset = ( block * 2 ) + ( y * _pitch );
			uint16_t* p = reinterpret_cast<uint16_t*>( &( _pData[ offset ] ) );

			// ... set/clear bit plane
			if ( data & 1 ) { p[ 0 ] |= mask; }	else { p[ 0 ] &= ~mask; }
		}
		break;

	case PixelFormat::AMSTRAD_CPC_M0:
		{
			uint16_t block = ( x >> 1 );
			offset = block + ( y * _pitch );
			uint8_t* p = &( _pData[ offset ] );
			uint8_t mask;

			if ( x & 1 )
			{
				mask = 1 << 6; // pixel 1, bit 0
				if ( data & 1 ) { *p |= mask; } else { *p &= ~mask; }

				mask = 1 << 2; // pixel 1, bit 1
				if ( data & 2 ) { *p |= mask; } else { *p &= ~mask; }

				mask = 1 << 4; // pixel 1, bit 2
				if ( data & 4 ) { *p |= mask; } else { *p &= ~mask; }

				mask = 1 << 0; // pixel 1, bit 3
				if ( data & 8 ) { *p |= mask; } else { *p &= ~mask; }
			}
			else
			{
				mask = 1 << 7; // pixel 0, bit 0
				if ( data & 1 ) { *p |= mask; } else { *p &= ~mask; }

				mask = 1 << 3; // pixel 0, bit 1
				if ( data & 2 ) { *p |= mask; } else { *p &= ~mask; }

				mask = 1 << 5; // pixel 0, bit 2
				if ( data & 4 ) { *p |= mask; } else { *p &= ~mask; }

				mask = 1 << 1; // pixel 0, bit 3
				if ( data & 8 ) { *p |= mask; } else { *p &= ~mask; }
			}
		}
		break;

	case PixelFormat::AMSTRAD_CPC_M1:
		{
			uint16_t block = ( x >> 2 );
			offset = block + ( y * _pitch );
			uint8_t* p = &( _pData[ offset ] );
			
			uint8_t mask0, mask1;
			mask0 = 0x80 >> ( x & 3 );
			mask1 = 0x08 >> ( x & 3 );

			if ( data & 1 ) { *p |= mask0; } else { *p &= ~mask0; }
			if ( data & 2 ) { *p |= mask1; } else { *p &= ~mask1; }
		}
		break;

	case PixelFormat::SEGA_VDP:
		{
			// ... select pixel within the 8-pixel plane.
			int block = ( x >> 3 );
			uint8_t mask = 0x80 >> ( x & 0x7 );

			// ... offset to first bit-plane
			offset = ( block * 4 ) + ( y * _pitch );
			uint8_t* p = &( _pData[ offset ] );

			// ... set/clear bit planes
			if ( data & 1 ) { p[ 0 ] |= mask; }			else { p[ 0 ] &= ~mask; }
			if ( data & 2 ) { p[ 1 ] |= mask; }			else { p[ 1 ] &= ~mask; }
			if ( data & 4 ) { p[ 2 ] |= mask; }			else { p[ 2 ] &= ~mask; }
			if ( data & 8 ) { p[ 3 ] |= mask; }			else { p[ 3 ] &= ~mask; }
		}
		break;

	case PixelFormat::GAMEBOY:
		{
			// ... select pixel within the 8-pixel plane.
			int block = ( x >> 3 );
			uint8_t mask = 0x80 >> ( x & 0x7 );

			// ... offset to first bit-plane
			offset = ( block * 2 ) + ( y * _pitch );
			uint8_t* p = &( _pData[ offset ] );

			// ... set/clear bit planes
			if ( data & 1 ) { p[ 0 ] |= mask; }			else { p[ 0 ] &= ~mask; }
			if ( data & 2 ) { p[ 1 ] |= mask; }			else { p[ 1 ] &= ~mask; }
		}
		break;

	case PixelFormat::NES:
		{
			// ... which tile are we in?
			int tile = ( x >> 3 ) + ( y >> 3 ) * ( _width >> 3 );

			// ... offset to first bit of tile (tiles are 16 byte chunks)
			int offset = tile * 16;
			uint8_t* p = &( _pData[ offset ] );

			// ... select row within tile chunk.
			int row = ( y & 7 );
			
			uint8_t mask = 0x80 >> ( x & 0x7 );

			// ... set/clear bit planes
			if ( data & 1 ) { p[ row ] |= mask; } else { p[ row ] &= ~mask; }
			if ( data & 2 ) { p[ 8 + row ] |= mask; } else { p[ 8 + row ] &= ~mask; }
		}
		break;

	}
}

uint32_t Image::Peek( int x, int y ) const
{
	uint32_t data = 0;
	uint32_t offset;

	switch ( _pixelFmt )
	{

	case PixelFormat::UNKNOWN:
		break;

	case PixelFormat::PACKED_1:
	case PixelFormat::AMSTRAD_CPC_M2:
		{
			offset = ( x >> 3 ) + y * _pitch;
			uint8_t* p = _pData + offset;

			uint8_t mask;
			mask = 1 << ( 7 - ( x & 7 ) );

			data = ( *p & mask ) ? 1 : 0;
		}
		break;

	case PixelFormat::PACKED_2:
	case PixelFormat::IBM_CGA:
		{
			offset = ( x >> 2 ) + y * _pitch;
			uint8_t* p = _pData + offset;

			uint8_t shift;
			shift = ( 3 - ( x & 3 ) ) << 1;

			data = ( *p >> shift ) & 3;
		}
		break;

	case PixelFormat::PACKED_4:
		{
			offset = ( x >> 1 ) + y * _pitch;
			uint8_t* p = _pData + offset;

			if ( x & 1 )
			{
				// get least nibble
				data = *p & 0x0F;
			}
			else
			{
				// get most nibble
				data = ( *p & 0xF0 ) >> 4;
			}
		}
		break;

	case PixelFormat::CHUNKY_8:
		{
			offset = x + y * _pitch;
			data = _pData[ offset ];
		}
		break;

	case PixelFormat::CHUNKY_16:
		{
			offset = ( x * 2 ) + y * _pitch;
			uint16_t* p = reinterpret_cast<uint16_t*>( _pData[ offset ] );
			data = *p;
		}
		break;

	case PixelFormat::CHUNKY_32:
		{
			offset = ( x * 4 ) + ( y * _pitch );
			uint32_t* p = reinterpret_cast<uint32_t*>( &( _pData[ offset ] ) );
			data = *p;
		}
		break;

	default:
	case PixelFormat::ATART_ST_M0:
	case PixelFormat::ATART_ST_M1:
	case PixelFormat::ATART_ST_M2:
	case PixelFormat::AMSTRAD_CPC_M0:
	case PixelFormat::AMSTRAD_CPC_M1:
		// todo
		break;

	}

	return data;
}

void Image::Clear( uint8_t value )
{
	if ( _pData )
	{
		uint32_t byteCount = _pitch * _height;
		memset( _pData, value, byteCount );
	}
}

void Image::Create( PixelFormat fmt, uint16_t width, uint16_t height )
{
	free( _pData ); // clear any leaks.
	_pData = nullptr;

	_pixelFmt = fmt;

	if ( fmt == PixelFormat::UNKNOWN )
	{
		_width = 0;
		_height = 0;
		return;
	}

	_width = width;
	_height = height;

	switch ( fmt )
	{
	
	case PixelFormat::PACKED_1:
	case PixelFormat::AMSTRAD_CPC_M2:
		_pitch = ( width + 7 ) / 8;
		_stride = _pitch * 8;
		break;
	
	case PixelFormat::PACKED_4:
		_pitch = ( width + 1 ) / 2;
		_stride = _pitch * 2;
		break;
	
	case PixelFormat::CHUNKY_8:
		_pitch = width;
		_stride = width;
		break;
	
	case PixelFormat::CHUNKY_16:
		_pitch = width * 2;
		_stride = width;
		break;
	
	case PixelFormat::CHUNKY_32:
		_pitch = width * 4;
		_stride = width;
		break;

	case PixelFormat::ATART_ST_M0:
		_pitch = ( ( width + 15 ) / 16 ) * 8;
		_stride = _pitch * 2;
		break;

	case PixelFormat::ATART_ST_M1:
		_pitch = ( ( width + 15 ) / 16 ) * 4;
		_stride = _pitch * 2;
		break;

	case PixelFormat::ATART_ST_M2:
		_pitch = ( ( width + 15 ) / 16 ) * 2;
		_stride = _pitch * 2;
		break;

	case PixelFormat::AMSTRAD_CPC_M0:
		_pitch = ( width + 1 ) / 2;
		_stride = _pitch * 2;
		break;

	case PixelFormat::AMSTRAD_CPC_M1:
		_pitch = ( width + 3 ) / 4;
		_stride = _pitch * 4;
		break;

	case PixelFormat::IBM_CGA:
		_pitch = ( width + 3 ) / 4;
		_stride = _pitch * 4;
		break;

	case PixelFormat::GAMEBOY:
		_pitch = ( ( width + 7 ) / 8 ) * 2;
		_stride = _pitch * 4;
		_height = ( ( height + 7 ) / 8 ) * 8; // ensure 8 pixel lines
		break;

	case PixelFormat::SEGA_VDP:
		_pitch = ( ( width + 7 ) / 8 ) * 4;
		_stride = _pitch * 2;
		_height = ( ( height + 7 ) / 8 ) * 8; // ensure 8 pixel lines
		break;

	case PixelFormat::NES: // treat each tile as 16 x 1 => 2 * 8
		_width = ( ( width + 7 ) / 8 ) * 8; // ensure 8 pixel columns
		_pitch = _width >> 2;
		_stride = _pitch >> 1;
		_height = ( ( height + 7 ) / 8 ) * 8; // ensure 8 pixel lines
		break;

	}

	uint32_t uByteCount;
	uByteCount = _pitch * _height;

	_pData = reinterpret_cast< uint8_t* >( calloc( _height, _pitch ) );
}

void Image::Destroy()
{
	free( _pData );
	_pData = nullptr;
}

uint8_t* Image::GetRowPtr( uint16_t row )
{
	if ( _pData && ( row < _height ) )
	{
		uint32_t offset;
		offset = row * _pitch;

		return _pData + offset;
	}
	else
	{
		return nullptr;
	}
}
