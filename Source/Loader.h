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

#include <string>

#include "ImageInfo.h"

class Image;
class FileReader;

//==============================================================================

//
// Loader
//
// Loads an image from various file formats.
//
class Loader
{

public:

	//--------------------------------------------------------------------------
	// Public Declarations
	//--------------------------------------------------------------------------

	//
	// eWant
	//
	// Demands for LoadTo. If not possible to meet, call fails.
	//
	enum eWant
	{
		WANT_INVALID = 0x00,

		//
		// -- INDEXED

		WANT_IDX1 = 0x01,				// 8 x 1-bit pixels packed in a byte (TMS-9918, ZX Spectrum, etc.)
		WANT_IDX4 = 0x04,				// 2 x 4-bit indices packed in a byte (PS1, etc.)
		WANT_IDX8 = 0x08,				// 1 x 8-bit index in a byte (MODE-13, etc.)

		//
		// -- DIRECT

		WANT_RGBA16 = 0x10,				// 1:5:5:5
		WANT_RGBA32 = 0x20,				// 8:8:8:8, stored as a B G R A in image.
		WANT_A4 = 0x34,					// 4-BPP grey channel
		WANT_A8 = 0x38,					// 8-BPP grey channel
		WANT_A16 = 0x40,				// 16-BPP grey channel
		WANT_LA4 = 0x54,				// luminance-alpha L3,A1
		WANT_LA8 = 0x58,				// luminance-alpha L4,A4
		WANT_LA16 = 0x60,				// luminance-alpha L8,A8

		// ... mask
		WANT_COLOUR_MODE_MASK = 0xFF,


		/// --- other features

		// We want a POW2 texture. Add padding if required.
		WANT_POW2 = 0x100,

	};


	//--------------------------------------------------------------------------
	// Constructor / Destructor
	//--------------------------------------------------------------------------

	// Default Constructor
	Loader();

	// Destructor
	~Loader();


	//--------------------------------------------------------------------------
	// Public Methods
	//--------------------------------------------------------------------------

	//
	// Identify
	//
	// Identify the type of image from the given reader.
	//
	ImageSourceFormat Identify( const FileReader& reader ) const;

	//
	// LoadTo
	//
	// Load an image into the given HcImage object.
	//
	// \param reader Read an image from the current read cursor.
	// \param info Pointer to storage for information about the image. Can be NULL.
	// \param want Our demands. See eWant.
	//
	// \return true if the image was loaded successfully and can meet our requirements.
	//
	bool LoadTo( FileReader& reader, Image& image, ImageInfo* p_info, const uint32_t want );

	//
	// GetLastError
	//
	// If loading fails, use this to get an error message.
	//
	const std::string& GetLastError() const
	{
		return m_last_error;
	}


private:

	//--------------------------------------------------------------------------
	// Implementation Data
	//--------------------------------------------------------------------------

	// Last Error
	std::string m_last_error;

};
