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

struct ImageInfo;
class Image;
class FileReader;

//==============================================================================

//-----------------------------------------------------------------------------
// Class Declaration
//-----------------------------------------------------------------------------

//
// cPNG
//
// Takes the given .PNG image file stream and decompresses it.
//
class cPNG
{

public:

	//--------------------------------------------------------------------------
	// Constructor / Destructor
	//--------------------------------------------------------------------------

	// Default Constructor
	cPNG();

	// Destructor
	~cPNG();


	//--------------------------------------------------------------------------
	// Public Methods
	//--------------------------------------------------------------------------

	//
	// LoadTo
	//
	// Decompresses the given PNG file into the given image object.
	//
	// \return True if the image was decompressed successfully.
	//
	bool LoadTo( FileReader& reader, Image& image, ImageInfo* p_info, const uint32_t want, std::string& error );


	//--------------------------------------------------------------------------
	// Public Static Methods
	//--------------------------------------------------------------------------

	//
	// Identify
	//
	// Returns true if the given reader (at the *current* read position) is a PNG
	//
	static bool Identify( const FileReader& file_reader );


private:

	//--------------------------------------------------------------------------
	// Helpers
	//--------------------------------------------------------------------------

	//
	// LoadTo_Internal
	//
	// Actual loading method. If libpng fails, longjmp teleports us to safety.
	//
	bool LoadTo_Internal( void* png_ptr,
						  void* info_ptr,
						  FileReader& reader,
						  ImageInfo* p_info,
						  const uint32_t want,
						  Image &image,
						  bool* p_compatible_format, 
						  std::string& error );

};

