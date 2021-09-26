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

#include "FileReader.h"

FileReader::~FileReader()
{
	free( _pData );
}

bool FileReader::LoadFile( const char* pFileName )
{
	_cursor = 0;

	FILE* fp;
	if ( fopen_s( &fp, pFileName, "rb" ) == 0 )
	{
		size_t length;
		fseek( fp, 0, SEEK_END );
		length = ftell( fp );
		fseek( fp, 0, SEEK_SET );

		_pData = (uint8_t*)malloc( length );
		if ( _pData )
		{
			_length = (uint32_t)length;

			fread( _pData, 1, length, fp );
			fclose( fp );
			return true;
		}
	}

	return false;
}

bool FileReader::IsSafeRequest( uint32_t count ) const
{
	if ( _cursor + count >= _length )
	{
		return false;
	}

	return true;
}

bool FileReader::Read( uint8_t* pTarget, uint32_t count )
{
	if ( _cursor + count >= _length )
	{
		return false;
	}

	memcpy( pTarget, _pData + _cursor, count );
	_cursor += count;
	return true;
}

