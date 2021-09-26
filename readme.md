
ImageTools
----------

A combined suite of utilities for manipulating images.
It was developed for use on Windows but might compile on other systems.
Released under the MIT License.

The following tools are provided. Click on a tool name to jump to its specific documentation.

Tool  |Description
:---|:------------
[mask](#mask) | Extract a bit mask from an image.

---

## mask

Extract a bit mask from an image.

**Usage**
```
 ImageTools mask <input> <output> [-index I] [-not] [-shift R] [-append] [-H###]

  <input>      An image file to read. (Indexed .PNG only)

  <output>     The output file.

  -index I     Specify the index of pixels to extract. Default 0.
  -not         Invert the output. Including border/shifted area.
  -shift R     Shift output to the right by R pixels.
  -append      Append to the output file, rather than overwriting it.

  -H###        Add a header. ### is a string of codes as follows:

    1          Codes following are 1 byte in size (default).
    w          Width of the output in pixels.
    c          Width of the output in characters (8 pixels wide)
    h          Height of the output in pixels.
```

**Examples**

```
> ImageTools mask test.png test.bin -index 0 -Hch -shift 2
> ImageTools mask test.png test.bin -index 1 -Hch -shift 2 -append
```

Converts an image file into a background erasing mask followed by a pixel image for detail. Each part has a simple two byte header. The output has also been pre-shifted to the right.

**Notes**

* Each byte of output data corresponds to 8 pixels with the most significant bit representing the left-side of the image.


