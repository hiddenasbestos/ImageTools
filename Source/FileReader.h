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
#include <cstring>

class FileReader
{

public:

	FileReader() :

		_pData( nullptr ),
		_cursor( 0 ),
		_length( 0 )

	{
		//
	}

	~FileReader();

public:

	bool LoadFile( const char* pFileName );

public:

	bool IsSafeRequest( uint32_t count ) const;
	
	bool Read( uint8_t* pTarget, uint32_t count );

public:

	// accessors

	const uint8_t* GetBufferPtr( uint32_t offset ) const
	{
		return _pData + offset;
	}

	uint8_t* GetBufferPtr( uint32_t offset )
	{
		return _pData + offset;
	}

	uint32_t GetReadCursor() const
	{
		return _cursor;
	}


private:

	uint8_t* _pData;
	uint32_t _cursor;
	uint32_t _length;

};
