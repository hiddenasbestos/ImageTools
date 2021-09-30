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

#pragma once

#include <cstdint>

#include "PixelFormat.h"

// Bitmap container.

class Image
{


public:

	// Write the value of a pixel. Takes care of masking/packing the data value.
	// WARNING: No checks are made on the X/Y position being within range!
	void Plot( int x, int y, uint32_t data );

	// Read the value of a pixel. 
	// WARNING: No checks are made on the X/Y position being within range!
	uint32_t Peek( int x, int y ) const;

	// Clear all bytes to a specific value.
	void Clear( uint8_t value );


public:

	// Default constructor.
	Image();

	// Create a new image with a size and pixel format.
	void Create( PixelFormat fmt, uint16_t width, uint16_t height );
	
	// Free data and tidy up.
	void Destroy();


public:

	//
	// -- accessors

	uint8_t* GetRowPtr( uint16_t row );

	uint16_t GetWidth() const
	{
		return _width;
	}

	uint16_t GetPitch() const
	{
		return _pitch;
	}

	uint16_t GetStride() const
	{
		return _stride;
	}

	uint16_t GetHeight() const
	{
		return _height;
	}


private:

	uint16_t _width; // pixels per row
	uint16_t _pitch; // bytes per row
	uint16_t _stride; // pixels per pitch
	uint16_t _height;

	PixelFormat _pixelFmt;

	uint8_t* _pData;

};