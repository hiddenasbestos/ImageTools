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

// Pixel formats for the Image container class.

enum class PixelFormat
{
	UNKNOWN,

	// Packed 8 pixels to a byte. MSB = left side; LSB = right side
	PACKED_1,

	// Packed 4 pixels to a byte in adjacent bit pairs. MSB = pixel 0; LSB = pixel 3
	PACKED_2,

	// Packed 2 x 4 bit pixel values. Bits 7:4 = left side; Bits 3:0 = right side
	PACKED_4,

	// Chunky 1 byte pixels
	CHUNKY_8,

	// Chunky 2 byte pixels
	CHUNKY_16,

	// Chunky 4 byte pixels
	CHUNKY_32,

	// Atari ST mode 0 (low resolution). 64-bit planar (16 pixels x 4 planes), big-endian
	ATART_ST_M0,

	// Atari ST mode 1 (medium resolution). 32-bit planar (16 pixels x 2 planes), big-endian
	ATART_ST_M1,

	// Atari ST mode 2 (high resolution). 16-bit planar (16 pixels x 1 planes), big-endian
	ATART_ST_M2,

	// Amstrad CPC mode 0. 2 pixels in 8 bits, eccentric ordering.
	AMSTRAD_CPC_M0,

	// Amstrad CPC mode 1. 4 pixels in 8 bits, planar
	AMSTRAD_CPC_M1,

	// Amstrad CPC mode 2. 8 pixels in 8 bits
	AMSTRAD_CPC_M2,

	// IBM CGA. BIOS mode 4/5 (320x200x4) 4 pixels in 8 bits, chunky
	IBM_CGA,

	// SEGA Master System / Game Gear
	MASTER_SYSTEM,

	// Game Boy
	GAMEBOY,

	// NES / Famicom
	NES,

};

// Helper to decode a string into a pixel format (case insensitive)
// If it's not recognised, returns PixelFormat::UNKNOWN
PixelFormat DecodePixelFormat( const char* pArg );

// Print a pixel format as text
const char* PixelFormatToString( PixelFormat pf );

// Max permitted index for a given pixel format
uint32_t PixelFormatMaxIndex( PixelFormat format );

// Returns true if this is an 8x8 pattern based pixel format.
bool PixelFormatIsPattern8x8( PixelFormat format );

