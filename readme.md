
ImageTools
----------

A combined suite of utilities for manipulating images.
It was developed for use on Windows but might compile on other systems.
Released under the MIT License.

The following tools are provided. Click on a tool name to jump to its specific documentation.

Tool  |Description
:---|:------------
[export](#export) | Export a raw image in a new pixel format.
[mask](#mask) | Extract a bit mask from an image.

---

## export

Export a raw image in a new pixel format.

**Usage**
```
 ImageTools export <input> <output> [-shift R] [-append] [-H###] [-pf format]

  <input>      An image file to read. (Indexed .PNG only)

  <output>     The output file.

  -shift R     Shift output to the right by R pixels.
  -append      Append to the output file, rather than overwriting it.

  -H###        Add a header. ### is a string of codes as follows:

    1          Byte mode (default).
    2          Word mode - 2 bytes per entity.
    L          Use little endian byte order.
    B          Use big endian byte order (default).
    w          Width of the output in pixels.
    p          Pitch of the output in bytes(1) or words(2)
    h          Height of the output in pixels.
    z          Write zero byte(1) or word(2).

  -pf FMT      Select the pixel format for the output. Default is "1BPP"

  The following pixel formats are supported:

    1BPP       8 x 1-bit pixels per byte. (ZX Spectrum, Hercules, etc.)
    2BPP       4 x 2-bit pixels per byte.
    CGA        IBM CGA mode 4/5 (320x200,4)
    CPC0       Amstrad CPC mode 0 (160x200,16)
    CPC1       Amstrad CPC mode 1 (320x200,4)
    CPC2       Amstrad CPC mode 2 (640x200,2)
    ST0        Atari ST mode 0 (Low)
    ST1        Atari ST mode 1 (Medium)
    ST2        Atari ST mode 2 (High)
```

**Examples**

```
> ImageTools export test.png test.bin -pf CPC1
```

Export an image for use with Amstrad CPC mode 1.

**Notes**

* The border on the left (when shifting) and right (when the source width is not an exact multiple of bytes/words) is set to index 0.


---

## mask

Extract a bit mask from an image.

**Usage**
```
 ImageTools mask <input> <output> [-index I] [-not] [-shift R] [-append] [-H###] [-pf format]

  <input>      An image file to read. (Indexed .PNG only)

  <output>     The output file.

  -index I     Specify the index of pixels to extract. Default 0.
  -not         Invert the output. Including border/shifted area.
  -shift R     Shift output to the right by R pixels.
  -append      Append to the output file, rather than overwriting it.

  -H###        Add a header. ### is a string of codes as follows:

    1          Byte mode (default).
    2          Word mode - 2 bytes per entity.
    L          Use little endian byte order.
    B          Use big endian byte order (default).
    w          Width of the output in pixels.
    p          Pitch of the output in bytes(1) or words(2)
    h          Height of the output in pixels.
    z          Write zero byte(1) or word(2).

  -pf FMT      Select the pixel format for the output. Default is "1BPP"

  The following pixel formats are supported:

    1BPP       8 x 1-bit pixels per byte. (ZX Spectrum, Hercules, etc.)
    2BPP       4 x 2-bit pixels per byte.
    CGA        IBM CGA mode 4/5 (320x200,4)
    CPC0       Amstrad CPC mode 0 (160x200,16)
    CPC1       Amstrad CPC mode 1 (320x200,4)
    CPC2       Amstrad CPC mode 2 (640x200,2)
    ST0        Atari ST mode 0 (Low)
    ST1        Atari ST mode 1 (Medium)
    ST2        Atari ST mode 2 (High)
```

**Examples**

```
> ImageTools mask test.png test.bin -index 0 -Hph -shift 2
> ImageTools mask test.png test.bin -index 1 -Hph -shift 2 -append
```

Converts an image file into a background erasing mask followed by a pixel image for detail. Each part has a simple two byte header. The output has also been pre-shifted to the right.

**Notes**

* In 1BPP pixel format, each byte of output data corresponds to 8 pixels with the most significant bit representing the left-side of the image.

* The border on the left (when shifting) and right (when the source width is not an exact multiple of bytes/words) is assumed to contain pixels of index 0.


