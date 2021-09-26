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

#include <cstdarg>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include "png.h"
#include "cPNG.h"
#include "Loader.h"
#include "utils.h"
#include "FileReader.h"
#include "Image.h"
#include "ImageInfo.h"

//==============================================================================

//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------

//
// read_data_fn
//
// Custom data reader callback, user data is an FileReader pointer.
//
static void read_data_fn( png_structp png_ptr, png_bytep p_data, png_size_t size )
{
	// Get our file reader.
	FileReader& reader = *( reinterpret_cast< FileReader* >( png_get_io_ptr( png_ptr ) ) );

	// Read data!
	reader.Read( p_data, static_cast< uint32_t >( size ) );

	// debug
//	printf( "size = %d, file_reader.pos = %d\n", size, file_reader.GetReadCursor() );
}

//
// error_fn
//
// Handle fatal PNG errors.
//
static void error_fn( png_structp png_ptr, png_const_charp error_message )
{
	PrintError( "libpng error: %s", error_message );
	png_longjmp( png_ptr, -1 );
}

//
// warn_fn
//
// Handle minor PNG warnings.
//
static void warn_fn( png_structp png_ptr, png_const_charp error_message )
{
	PrintError( "libpng warning: %s", error_message );
}

//==============================================================================

//------------------------------------------------------------------------------
// cPNG::cPNG
//------------------------------------------------------------------------------
cPNG::cPNG()
{
	//
}

//------------------------------------------------------------------------------
// cPNG::~cPNG
//------------------------------------------------------------------------------
cPNG::~cPNG()
{
	//
}

//==============================================================================

//------------------------------------------------------------------------------
// cPNG::Identify
//------------------------------------------------------------------------------
bool cPNG::Identify( const FileReader& reader )
{
	// Too short?
	if ( reader.IsSafeRequest( 8 ) == false )
	{
		return false; // <=== EARLY OUT
	}

	// Alias the file data.
	const uint8_t* p_header = reader.GetBufferPtr( reader.GetReadCursor() );

	// Is a .PNG file?
	if ( png_sig_cmp( ( png_bytep )p_header, 0, 8 ) == 0 )
	{
		// Valid!
		return true;
	}
	else
	{
		// Nope.
		return false;
	}
}

//==============================================================================

//------------------------------------------------------------------------------
// cPNG::LoadTo
//------------------------------------------------------------------------------
bool cPNG::LoadTo( FileReader& reader,
				   Image& image,
				   ImageInfo* p_info,
				   const uint32_t want, 
				   std::string& error )
{
	// 8 is the maximum size that can be checked
	uint8_t header[ 8 ];
	if ( reader.Read( header, 8 ) == false )
	{
		return false; // <=== EARLY OUT
	}

	png_infop info_ptr = nullptr;
	png_structp png_ptr = nullptr;

	// Initialise the reader.
	png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, this, error_fn, warn_fn );
	if ( png_ptr == nullptr )
	{
		error = "png_create_read_struct failed.";
		return false; // <=== EARLY OUT
	}

	// Are we here for the first time?
	bool compatible_format = false;
	bool completed = false;
	jmp_buf* p_jmp_buf = png_set_longjmp_fn( png_ptr, longjmp, sizeof( jmp_buf ) );

	if ( setjmp( *p_jmp_buf ) != -1 )
	{
		// Not a .PNG file?
		if ( png_sig_cmp( header, 0, 8 ) )
		{
			error = "Attempted to decompress a non-PNG file.";
			return false; // <=== EARLY OUT
		}

		// Initialise the information structure.
		info_ptr = png_create_info_struct( png_ptr );
		if ( info_ptr == nullptr )
		{
			error = "png_create_info_struct failed.";
		}
		else
		{
			// GO!
			if ( LoadTo_Internal( png_ptr, info_ptr, reader, p_info, want, image, &compatible_format, error ) )
			{
				// .. made it!
				completed = true;
			}
		}
	}

	// Success?
	if ( completed == false )
	{
		// Memory leaks
		png_destroy_read_struct( &png_ptr, &info_ptr, nullptr );

		// Nope!
		return false;
	}
	else
	{
		// Success?
		return compatible_format;
	}
}

//==============================================================================

//------------------------------------------------------------------------------
// cPNG::LoadTo_Internal
//------------------------------------------------------------------------------
bool cPNG::LoadTo_Internal( void* _png_ptr,
							void* _info_ptr,
							FileReader& reader,
							ImageInfo* p_info,
							const uint32_t want,
							Image &image,
							bool* p_compatible_format,
							std::string& error )
{
	// Get pointer
	png_structp png_ptr = ( png_structp )_png_ptr;
	png_infop info_ptr = ( png_infop )_info_ptr;


	// Setup the file reader
	png_set_read_fn( png_ptr, &reader, read_data_fn );

	// Notify of the earlier pre-read.
	png_set_sig_bytes( png_ptr, 8 );

	// Read file information
	png_read_info( png_ptr, info_ptr );

	// Get DPI
//	const uint32_t res_x = png_get_x_pixels_per_inch( png_ptr, info_ptr );
//	const uint32_t res_y = png_get_y_pixels_per_inch( png_ptr, info_ptr );


	//
	// Prepare
	//

	uint32_t pitch;
	uint32_t colour_type;

	uint32_t channels;
	channels = 0; // fail

	uint32_t src_pitch;
	src_pitch = 0; // fail

	*p_compatible_format = true; // <--- assume so

	// Get the bit depth.
	const png_byte png_bit_depth = png_get_bit_depth( png_ptr, info_ptr );

	// .. weird bit depth?
	if ( !( png_bit_depth == 1 || png_bit_depth == 2 || png_bit_depth == 4 || png_bit_depth == 8 || png_bit_depth == 16 ) )
	{
		error = "Unsupported PNG bit depth.";
		*p_compatible_format = false;
		return false;
	}

	// We can only read 16-bit alpha images.
	if ( png_bit_depth == 16 && ( want & Loader::WANT_COLOUR_MODE_MASK ) != Loader::WANT_A16 )
	{
		// HDEBUGERROR( "Invalid want/format combination." );
		*p_compatible_format = false;
		return false;
	}

	// Get the image size.
	const uint32_t real_image_width = png_get_image_width( png_ptr, info_ptr );
	const uint32_t real_image_height = png_get_image_height( png_ptr, info_ptr );

	// Store info
	if ( p_info )
	{
		p_info->format = ImageSourceFormat::PNG;
		p_info->width = real_image_width;
		p_info->height = real_image_height;
	}

	// Apply padding
	uint32_t padded_image_width, padded_image_height;
	if ( want & Loader::WANT_POW2 )
	{
		// ... next power of two
		padded_image_width = NextPowerTwo( real_image_width );
		padded_image_height = NextPowerTwo( real_image_height );
	}
	else
	{
		// ... default, no padding
		padded_image_width = real_image_width;
		padded_image_height = real_image_height;
	}

	// Cache the pitch.
	pitch = static_cast< uint32_t >( png_get_rowbytes( png_ptr, info_ptr ) );

	// Colour type.
	colour_type = png_get_color_type( png_ptr, info_ptr );

	PixelFormat image_format;

	uint32_t want_colour;
	want_colour = ( want & Loader::WANT_COLOUR_MODE_MASK );

	uint32_t want_idx_bits = 0; // direct colour
	switch ( want_colour )
	{

	case Loader::WANT_IDX1:
		want_idx_bits = 1;
		image_format = PixelFormat::PACKED_1;
		break;

	case Loader::WANT_IDX4:
		want_idx_bits = 4;
		image_format = PixelFormat::PACKED_4;
		break;

	case Loader::WANT_IDX8:
		want_idx_bits = 8;
		image_format = PixelFormat::CHUNKY_8;
		break;

	case Loader::WANT_A4:
		image_format = PixelFormat::PACKED_4;
		break;

	case Loader::WANT_A8:
		image_format = PixelFormat::CHUNKY_8;
		break;

	case Loader::WANT_A16:
		image_format = PixelFormat::CHUNKY_16;
		break;

	case Loader::WANT_LA4:
		image_format = PixelFormat::PACKED_4;
		break;

	case Loader::WANT_LA8:
		image_format = PixelFormat::CHUNKY_8;
		break;

	case Loader::WANT_LA16:
		image_format = PixelFormat::CHUNKY_16;
		break;

	case Loader::WANT_RGBA16:
		image_format = PixelFormat::CHUNKY_16;
		break;
	
	case Loader::WANT_RGBA32:
		image_format = PixelFormat::CHUNKY_32;
		break;
	
	default:
		*p_compatible_format = false;
		return false;

	}; // switch ( want )

	// Palette?
	if ( colour_type == PNG_COLOR_TYPE_PALETTE )
	{
		if ( want_colour == Loader::WANT_RGBA16 || 
			 want_colour == Loader::WANT_RGBA32 )
		{
			//
			// ... EXPAND TO RGBA

			png_set_palette_to_rgb( png_ptr );
			png_set_tRNS_to_alpha( png_ptr );

			png_bytep trans_alpha = nullptr;
			int num_trans = 0;
			png_color_16p trans_color = nullptr;

			// Has transparency?
			png_get_tRNS( png_ptr, info_ptr, &trans_alpha, &num_trans, &trans_color );
			if ( trans_alpha )
			{
				colour_type = PNG_COLOR_TYPE_RGB_ALPHA;
				channels = 4;
			}
			else
			{
				colour_type = PNG_COLOR_TYPE_RGB;
				channels = 3;
			}

			// Compute source pitch
			src_pitch = real_image_width * channels;
		}
		else
		{
			//
			// ... KEEP PALETTE

			// too many indices? -> we handle 8-bit PNGs more delicately below.
			if ( png_bit_depth != 8 && png_bit_depth > want_idx_bits )
			{
				*p_compatible_format = false;
			}
			// supported bit depth?
			else if ( !( png_bit_depth == 1 || png_bit_depth == 2 || png_bit_depth == 4 || png_bit_depth == 8 ) )
			{
				error = "Unsupported PNG bit depth.";
				*p_compatible_format = false;
			}
			else
			{
				// Compute source pitch in bits.
				src_pitch = ( real_image_width * png_bit_depth );
				// .. then round up to the next byte
				src_pitch = ( src_pitch + 7 ) / 8;
				channels = UINT32_MAX; // Indexed
			}
		}
	}
	else
	{
		if ( want_colour == Loader::WANT_A4 ||
			 want_colour == Loader::WANT_A8 )
		{
			if ( colour_type == PNG_COLOR_TYPE_GRAY && ( png_bit_depth == 8 ) )
			{
				channels = 1;
			}
			else
			{
				*p_compatible_format = false;
			}

			// Compute source pitch
			src_pitch = real_image_width * channels;
		}
		else if ( want_colour == Loader::WANT_A16 )
		{
			if ( colour_type == PNG_COLOR_TYPE_GRAY && ( png_bit_depth == 8 || png_bit_depth == 16 ) )
			{
				channels = 1;
			}
			else
			{
				*p_compatible_format = false;
			}

			// Compute source pitch
			src_pitch = real_image_width * channels * sizeof( uint16_t );
		}
		else if ( want_colour == Loader::WANT_LA4 ||
				  want_colour == Loader::WANT_LA8 ||
				  want_colour == Loader::WANT_LA16 )
		{
			if ( colour_type == PNG_COLOR_TYPE_GRAY )
			{
				channels = 1;
			}
			else if ( colour_type == PNG_COLOR_TYPE_GRAY_ALPHA )
			{
				channels = 2;
			}
			else
			{
				*p_compatible_format = false;
			}

			// Compute source pitch
			src_pitch = real_image_width * channels;
		}
		else if ( want_colour == Loader::WANT_RGBA16 )
		{
			if ( colour_type == PNG_COLOR_TYPE_RGB )
			{
				channels = 3;
			}
			else if ( colour_type == PNG_COLOR_TYPE_RGB_ALPHA )
			{
				channels = 4;
			}
			else
			{
				*p_compatible_format = false;
			}

			// Compute source pitch
			src_pitch = real_image_width * channels;
		}
		else if ( want_colour == Loader::WANT_RGBA32 )
		{
			if ( colour_type == PNG_COLOR_TYPE_GRAY )
			{
				channels = 1;
			}
			else if ( colour_type == PNG_COLOR_TYPE_GRAY_ALPHA )
			{
				channels = 2;
			}
			else if ( colour_type == PNG_COLOR_TYPE_RGB )
			{
				channels = 3;
			}
			else if ( colour_type == PNG_COLOR_TYPE_RGB_ALPHA )
			{
				channels = 4;
			}
			else
			{
				*p_compatible_format = false;
			}

			// Compute source pitch
			src_pitch = real_image_width * channels;
		}
		else
		{
			// Not a palette image, so we've failed.
			*p_compatible_format = false;
		}
	}

	//
	// LET'S GO!

	if ( *p_compatible_format )
	{
//		assert( src_pitch > 0, "Invalid source pitch for supposedly compatible format" );

		// Allocate the image
		image.Create( image_format, padded_image_width, padded_image_height );

		// RGB expansion helper. Can't use this for palette images :(
		if ( png_bit_depth < 8 && colour_type != PNG_COLOR_TYPE_PALETTE )
		{
			// Expand to 8-bits (even if we want less, this makes extraction easier! and we'll mask later)
			if ( colour_type == PNG_COLOR_TYPE_GRAY )
			{
				png_set_expand_gray_1_2_4_to_8( png_ptr );
			}
			else
			{
				png_set_expand( png_ptr );
			}
		}

		// Handle Interlaced
		int interlaced_passes;
		interlaced_passes = png_set_interlace_handling( png_ptr );
		png_read_update_info( png_ptr, info_ptr );

		// How many bytes in a row?
		const png_size_t row_bytes = png_get_rowbytes( png_ptr, info_ptr );

		// Create row buffer
		png_bytep p_src_row;
		p_src_row = reinterpret_cast< png_bytep >( malloc( row_bytes ) );


		// Set colour mode info.
		if ( p_info )
		{
			p_info->bIndexed = ( channels == UINT32_MAX ) ? true : false;

			// Get palette RGB values
			if ( p_info->bIndexed )
			{
				png_bytep trans_alpha;
				int num_trans;
				png_color_16p trans_color;
				png_get_tRNS( png_ptr, info_ptr, &trans_alpha, &num_trans, &trans_color );
				
				png_colorp palette;
				int palette_size;
				png_get_PLTE( png_ptr, info_ptr, &palette, &palette_size );

				for ( int i = 0; i < palette_size; ++i )
				{
					const png_color& e = palette[ i ];

					uint32_t rgb;
					rgb = ( e.red << 16 ) | ( e.green << 8 ) | ( e.blue << 0 );

					if ( i < num_trans )
					{
						rgb |= ( trans_alpha[ i ] ) << 24;
					}
					else
					{
						rgb |= ( 0xFF << 24 );
					}

					p_info->palette.push_back( rgb );
				}
			}
		}

		// Read Rows
		for ( int ipass = 0; ipass < interlaced_passes; ++ipass )
		{

//----------------------------------

			for ( uint32_t y = 0; y < real_image_height; ++y )
			{
				// Read a row (read it multiple times if it's interlaced)
				png_read_row( png_ptr, nullptr, p_src_row );

				uint8_t* p = ( uint8_t* )p_src_row;

				// Swizzle & Expand!
				switch ( channels )
				{

				case UINT32_MAX: // Indexed!

					{
						//
						// IDX (packed) -> IDX

						switch ( png_bit_depth ) // <-- note this is what the image defines, not what we're asking for.
						{

						case 1:
							{
								uint8_t feed = 0;
								// simple: 1 src byte = 8 indices (LSB[idx7:idx6:idx5:idx4:idx3:idx2:idx1:idx0]MSB)
								for ( uint32_t x = 0; x < real_image_width; ++x )
								{
									const uint8_t sub = ( x & 7 );
									if ( sub == 0 )
									{
										feed = *p++; // feed in up to 8 new indices.
										image.Plot( x, y, feed >> 7 ); // first index
									}
									else
									{
										image.Plot( x, y, ( feed >> ( 7 - sub ) ) & 1 );
									}
								}
							}
							break;

						case 2:
							{
								uint8_t feed = 0;
								// simple: 1 src byte = 4 indices (LSB[idx3:idx2:idx1:idx0]MSB)
								for ( uint32_t x = 0; x < real_image_width; ++x )
								{
									const uint8_t sub = ( x & 3 );
									if ( sub == 0 )
									{
										feed = *p++; // feed in up to 4 new indices.
										image.Plot( x, y, feed >> 6 ); // first index
									}
									else
									{
										image.Plot( x, y, ( feed >> ( 6 - ( sub << 1 ) ) ) & 3 );
									}
								}
							}
							break;

						case 4:
							{
								uint8_t feed = 0;
								// simple: 1 src byte = 2 indices (LSB[idx1:idx0]MSB)
								for ( uint32_t x = 0; x < real_image_width; ++x )
								{
									if ( x & 1 )
									{
										// odd
										image.Plot( x, y, feed & 0xF ); // second index.
									}
									else
									{
										// even
										feed = *p++; // feed in up to 2 new indices.
										image.Plot( x, y, feed >> 4 ); // first index
									}
								}
							}
							break;

						case 8:

							if ( want_idx_bits >= 8 )
							{
								// simple: 1 src byte = 1 index; no need to check for index overflow.
								for ( uint32_t x = 0; x < real_image_width; ++x )
								{
									image.Plot( x, y, *p++ );
								}
							}
							else
							{
								uint8_t feed;
								const uint32_t limit = ( 1 << want_idx_bits );

								// simple: 1 src byte = 1 index; check for overflow.
								for ( uint32_t x = 0; x < real_image_width; ++x )
								{
									feed = *p++;

									// Index is over the limit?
									if ( feed >= limit )
									{
										// ... abort.
										error = "Index out of range.";
										*p_compatible_format = false;
										image.Destroy();
										goto abort_compatible_format;
									}

									image.Plot( x, y, feed );
								}
							}
							break;

						default:
							// error
							*p_compatible_format = false;
							return false;

						}; // switch ( png_bit_depth )
					}

					break; // end of indexed mode

				case 1: // A A ...

					switch ( want_colour )
					{

					default:
						error = "Can't convert image to a suitable pixel format.";
						break;

					case Loader::WANT_LA4: // alpha only, so use lum=1
						{
							// Get target row.
							uint8_t* p_dst_row = ( uint8_t* )( image.GetRowPtr( y ) );

							for ( uint32_t x = 0; x < real_image_width; x += 2 )
							{
								uint8_t pair = 0xEE; // lum=1

								uint8_t a;

								a = *p++; // feed in a new alpha value.
								if ( a & 0x80 )
									pair |= 0x10;

								a = *p++; // feed in alpha value 2.
								if ( a & 0x80 )
									pair |= 0x01;

								p_dst_row[ x >> 1 ] = pair;
							}

							// ... clear remaining columns
							for ( uint32_t x = real_image_width; x < padded_image_width; ++x )
							{
								image.Plot( x, y, 0 ); // 
							}
						}
						break;

					case Loader::WANT_LA8: // alpha only, so use lum=1
						{
							for ( uint32_t x = 0; x < real_image_width; ++x )
							{
								uint8_t a = *p++; // feed in a new alpha value.
								image.Plot( x, y, 0xF0 | ( a >> 4 ) ); // 1/A
							}

							// ... clear remaining columns
							for ( uint32_t x = real_image_width; x < padded_image_width; ++x )
							{
								image.Plot( x, y, 0 ); // 
							}
						}
						break;

					case Loader::WANT_A4:
						{
							for ( uint32_t x = 0; x < real_image_width; ++x )
							{
								uint8_t a = *p++; // feed in a new alpha value.
								image.Plot( x, y, a >> 4 ); // 
							}

							// ... clear remaining columns
							for ( uint32_t x = real_image_width; x < padded_image_width; ++x )
							{
								image.Plot( x, y, 0 ); // 
							}
						}
						break;

					case Loader::WANT_A8:
						{
							// Get target row.
							uint8_t* p_dst_row = ( uint8_t* )( image.GetRowPtr( y ) );

							// ... copy data row.
							memcpy( p_dst_row, p, real_image_width );
							p_dst_row += real_image_width;

							// ... clear remaining columns
							for ( uint32_t x = real_image_width; x < padded_image_width; ++x )
							{
								*p_dst_row++ = 0;
							}
						}
						break;

					case Loader::WANT_A16:
						{
							// Get target row.
							uint16_t* p_dst_row = ( uint16_t* )( image.GetRowPtr( y ) );

							if ( png_bit_depth == 8 )
							{
								for ( uint32_t x = 0; x < real_image_width; ++x )
								{
									// byte-double to make 16-bpp value.
									*p_dst_row++ = ( *p ) | ( ( *p ) << 8 );
									++p;
								}
							}
							else
							{
								for ( uint32_t x = 0; x < real_image_width; ++x )
								{
									// read 16-bpp value, swap to little-endian!
									*p_dst_row++ = ( p[ 1 ] ) | ( ( p[ 0 ] ) << 8 );
									p += 2;
								}
							}

							// ... clear remaining columns
							for ( uint32_t x = real_image_width; x < padded_image_width; ++x )
							{
								*p_dst_row++ = 0;
							}
						}
						break;

					case Loader::WANT_RGBA32:
						{
							uint32_t dst;

							// Get the target row
							uint32_t* p_dst_row = ( uint32_t* )( image.GetRowPtr( y ) );

							if ( want_colour == Loader::WANT_LA16 )
							{
								if ( png_bit_depth > 8 )
								{
									*p_compatible_format = false;
									return false;
								}
								else
								{
									for ( uint32_t i = 0; i < real_image_width; ++i )
									{
										// RGBA <- GGG1
										dst = ( ( p[ 0 ] ) << 16 )
											| ( ( p[ 0 ] ) << 8 )
											| ( ( p[ 0 ] ) )
											| 0xff000000;

										*p_dst_row++ = dst;

										// ... advance
										p += 1;
									}
								}
							}
							else
							{
								for ( uint32_t i = 0; i < real_image_width; ++i )
								{
									// RGBA <- 111A
									dst = ( ( *p ) << 24 ) | 0xffffff;

									*p_dst_row++ = dst;

									// ... advance
									p += 1;
								}
							}

							// ... clear remaining columns
							for ( uint32_t x = real_image_width; x < padded_image_width; ++x )
							{
								*p_dst_row++ = 0;
							}
						}
						break;

					}

					break;

				case 2: // G A G A ...

					switch ( want_colour )
					{

					case Loader::WANT_LA4:

						{
							// Get the target row
							uint8_t* p_dst_row = ( uint8_t* )( image.GetRowPtr( y ) );

							uint32_t x = 0;

							while ( x < real_image_width )
							{
								uint8_t quad;
								quad = p[ 0 ] & 0xE0; // G
								if ( p[ 1 ] & 0x80 )
									quad |= 0x10; // A
								quad |= ( p[ 2 ] & 0xE0 ) >> 4; // G
								if ( p[ 3 ] & 0x80 )
									quad |= 0x01; // A

								*p_dst_row++ = quad;

								// ... advance
								p += 4;
								x += 2;
							}

							// ... clear remaining columns
							while ( x < padded_image_width )
							{
								*p_dst_row++ = 0;

								x += 2;
							}
						}

						break;

					case Loader::WANT_LA8:

						{
							// Get the target row
							uint8_t* p_dst_row = ( uint8_t* )( image.GetRowPtr( y ) );

							uint32_t x = 0;

							while ( x < real_image_width )
							{
								uint8_t pair;
								pair = p[ 0 ] & 0xF0; // G
								pair |= p[ 1 ] >> 4; // A

								*p_dst_row++ = pair;

								// ... advance
								p += 2;
								++x;
							}

							// ... clear remaining columns
							while ( x < padded_image_width )
							{
								*p_dst_row++ = 0;

								++x;
							}
						}

						break;

					case Loader::WANT_LA16:

						{
							// Get the target row
							uint8_t* p_dst_row = ( uint8_t* )( image.GetRowPtr( y ) );

							uint32_t x = 0;

							while ( x < real_image_width )
							{
								*p_dst_row++ = p[ 0 ]; // G
								*p_dst_row++ = p[ 1 ]; // A

								// ... advance
								p += 2;
								++x;
							}

							// ... clear remaining columns
							while ( x < padded_image_width )
							{
								*p_dst_row++ = 0;
								*p_dst_row++ = 0;

								++x;
							}
						}

						break;

					case Loader::WANT_RGBA32:

						{
							uint32_t dst;

							// Get the target row
							uint32_t* p_dst_row = ( uint32_t* )( image.GetRowPtr( y ) );

							uint32_t x = 0;

							while ( x < real_image_width )
							{
								// RGBA <- GGGA
								dst = ( ( p[ 0 ] ) << 16 )
									| ( ( p[ 0 ] ) << 8 )
									| ( ( p[ 0 ] ) )
									| ( ( p[ 1 ] ) << 24 );

								*p_dst_row++ = dst;

								// ... advance
								p += 2;
								++x;
							}

							// ... clear remaining columns
							while ( x < padded_image_width )
							{
								*p_dst_row++ = 0;
								++x;
							}
						}

						break;
					}

					break;

				case 3: // R G B R G B ...

					switch ( want_colour )
					{

					case Loader::WANT_RGBA16:

						{
							uint16_t out;

							// Get the target row
							uint16_t* p_dst_row = ( uint16_t* )( image.GetRowPtr( y ) );

							uint32_t x = 0;

							while ( x < real_image_width )
							{
								out = ( static_cast< uint16_t >( p[ 0 ] & 0xF8 ) << 7 ); // R
								out |= ( static_cast< uint16_t >( p[ 1 ] & 0xF8 ) << 2 ); // G
								out |= ( static_cast< uint16_t >( p[ 2 ] & 0xF8 ) >> 3 ); // B
								out |= 0x8000;

								*p_dst_row++ = out;

								// ... advance
								p += 3;
								++x;
							}

							// ... clear remaining columns
							while ( x < padded_image_width )
							{
								*p_dst_row++ = 0;
								++x;
							}
						}

						break;

					case Loader::WANT_RGBA32:

						{
							uint32_t dst;

							// Get the target row
							uint32_t* p_dst_row = ( uint32_t* )( image.GetRowPtr( y ) );

							uint32_t x = 0;

							while ( x < real_image_width )
							{
								// RGBA <- RGB1
								dst = ( ( p[ 0 ] ) << 16 )
									| ( ( p[ 1 ] ) << 8 )
									| ( ( p[ 2 ] ) )
									| 0xFF000000;

								*p_dst_row = dst;

								// ... advance
								p += 3;
								++p_dst_row;
								++x;
							}

							// ... clear remaining columns
							while ( x < padded_image_width )
							{
								*p_dst_row++ = 0;
								++x;
							}
						}

						break;

					}; // switch ( want_colour )

					break;

				case 4: // R G B A R G B A ...

					switch ( want_colour )
					{

					case Loader::WANT_RGBA16:

						{
							uint16_t out;

							// Get the target row
							uint16_t* p_dst_row = ( uint16_t* )( image.GetRowPtr( y ) );

							uint32_t x = 0;

							while ( x < real_image_width )
							{
								out = ( static_cast< uint16_t >( p[ 0 ] & 0xF8 ) << 7 ); // R
								out |= ( static_cast< uint16_t >( p[ 1 ] & 0xF8 ) << 2 ); // G
								out |= ( static_cast< uint16_t >( p[ 2 ] & 0xF8 ) >> 3 ); // B

								if ( p[ 3 ] & 0x80 )
								{
									out |= 0x8000;
								}

								*p_dst_row++ = out;

								// ... advance
								p += 4;
								++x;
							}

							// ... clear remaining columns
							while ( x < padded_image_width )
							{
								*p_dst_row++ = 0;
								++x;
							}
						}

						break;

					case Loader::WANT_RGBA32:

						{
							uint32_t dst;

							// Get the target row
							uint32_t* p_dst_row = ( uint32_t* )( image.GetRowPtr( y ) );

							uint32_t x = 0;

							while ( x < real_image_width )
							{
								// BGRA <- RGBA
								dst = ( ( p[ 0 ] ) << 16 )
									| ( ( p[ 1 ] ) << 8 )
									| ( ( p[ 2 ] ) )
									| ( ( p[ 3 ] ) << 24 );

								*p_dst_row++ = dst;

								// ... advance
								p += 4;
								++x;
							}

							// ... clear remaining columns
							while ( x < padded_image_width )
							{
								*p_dst_row++ = 0;
								++x;
							}
						}

						break;

					default:
						error = "Invalid colour mode.";
						break;

					}; // switch ( want_colour )

					break;

				}; // switch ( channels )

			}; // for each row

//----------------------------------

		}; // for each pass.

	abort_compatible_format:

		// Free temp row buffer
		free( p_src_row );

		// Pad remaining rows (only if we're still a valid image).
		if ( image.GetRowPtr( 0 ) )
		{
			for ( uint32_t y = real_image_height; y < padded_image_height; ++y )
			{
				// Clear the target row - pixel size agnostic
				uint8_t* p_dst_row = image.GetRowPtr( y );
				memset( p_dst_row, 0, image.GetPitch() );
			}
		}

	}; // compatible_format ?


	//
	// Tidy Up
	//

	// Destroy the main reader and info structures
	png_destroy_read_struct( &png_ptr, &info_ptr, nullptr );

	return true;
}

//==============================================================================
