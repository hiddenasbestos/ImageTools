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
#include <vector>


//
// ImageSourceFormat
//
// Image formats
//
enum class ImageSourceFormat
{
	UNKNOWN,

	PNG,

};


//
// ImageInfo
//
// Information about an image
//
struct ImageInfo
{
	//--------------------------------------------------------------------------
	// Fields
	//--------------------------------------------------------------------------

	// Detected file format
	ImageSourceFormat format;

	// Original Size. *Not* padded to POW2, etc.
	uint32_t width;
	uint32_t height;

	// Indexed colour mode?
	bool bIndexed;

	// Highest used index value in the image (or zero for direct colour images)
	uint32_t uMaxIndex;

	// Palette.
	std::vector< uint32_t > palette;


	//--------------------------------------------------------------------------
	// Public Methods
	//--------------------------------------------------------------------------

	// Reset to default
	void Reset()
	{
		format = ImageSourceFormat::UNKNOWN;
		width = 0;
		height = 0;
		bIndexed = false;
		uMaxIndex = 0;
		palette.clear();
	}

};
