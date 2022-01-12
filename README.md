# quadtiff2rgbtiff
Demosaics TIFFs containing a quad-bayer mosaic and outputs a RGB TIFF.

# Overview

This is a simple program to demosaic quad-bayer TIFFs, using an average of the quad channel components and then a 'half' amalgam 
to produce an RGB-encoded image. 

# Dependencies

libtiff, on Debian/Ubuntu systems, do "sudo apt install libtiff-dev"

# Usage

This program was originally written to accommodate an astro workflow that starts with FITS files.  It can be incorporated in a
script using ImageMagick convert to first turn the FITS file into a TIFF, then feeding the TIFF to quadtiff2rgbtiff.
```
#!/bin/bash

convert quad.FITS quad.tiff
quadtiff2rgbtiff quad.tiff rgb.tiff`
```

