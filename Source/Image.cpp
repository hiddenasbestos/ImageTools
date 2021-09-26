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

Image::Image() : _pData( nullptr )
{
	//
}

void Image::Plot( int x, int y, uint32_t data )
{
	uint32_t offset;

	switch ( _pixelFmt )
	{
	
	case PixelFormat::PACKED_1:
		{
			offset = ( x >> 3 ) + y * _pitch;
			uint8_t* p = _pData + offset;

			uint8_t mask;
			mask = 1 << ( 7- (  x & 7 ) );

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

	}
}

uint32_t Image::Peek( int x, int y ) const
{
	uint32_t data = 0;
	uint32_t offset;

	switch ( _pixelFmt )
	{

	case PixelFormat::PACKED_1:
		{
			offset = ( x >> 3 ) + y * _pitch;
			uint8_t* p = _pData + offset;

			uint8_t mask;
			mask = 1 << ( 7 - ( x & 7 ) );

			data = ( *p & mask ) ? 1 : 0;
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
	_width = width;
	_height = height;
	_pixelFmt = fmt;

	switch ( fmt )
	{
	
	case PixelFormat::PACKED_1:
		_pitch = ( width + 7 ) / 8;
		break;
	
	case PixelFormat::PACKED_4:
		_pitch = ( width + 1 ) / 2;
		break;
	
	case PixelFormat::CHUNKY_8:
		_pitch = width;
		break;
	
	case PixelFormat::CHUNKY_16:
		_pitch = width * 2;
		break;
	
	case PixelFormat::CHUNKY_32:
		_pitch = width * 4;
		break;
	
	}

	uint32_t uByteCount;
	uByteCount = _pitch * height;

	free( _pData ); // clear any leaks.

	_pData = reinterpret_cast< uint8_t* >( calloc( height, _pitch ) );
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
