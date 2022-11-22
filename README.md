# About nFITSview  (next FITS viewer)
nFITSview - A simple and user-friendly FITS image viewer

![nfitsview1_3_screenshot_1](https://user-images.githubusercontent.com/109148999/199483028-5a383cda-3389-4793-8df9-4b558ab504b7.png)
![nfitsview1_3_screenshot_2](https://user-images.githubusercontent.com/109148999/199483066-552c2bce-06e8-4c9b-b913-f62b3290bbf4.png)
![nfitsview1_3_screenshot_3](https://user-images.githubusercontent.com/109148999/199483134-122cb761-3a97-47e6-8e39-c02ca06c144c.png)


Currently nFITSview supports the following formats and features:

-    16-bit integer images
-    32-bit floating point and integer images
-    64-bit floating point and integer images (experimental, may still not work   
     as it should)
-    Exporting image HDUs as PNG/JPEG/BMP files
-    Bulk exporting of all image HDUs as PNG files
-    RGB gamma correction and grayscaling
-    Image zoom in/out
-    HDU header syntax view
-    Raw data hex preview
-    Only uncompressed images are supported 
-    Command line exporting of FITS file  (see -h, -e command line switches).
     Note: the console output is not visible on Windows platform.
    
# How to build under Linux

The original development of nFITSview is done under Linux, so the easiest and the fastest way to build is under Linux.

- just build in Qt Creator. 
- it should be also possible to build using CMake from the command line.

The latest version (1.4) of nFITSview install package for Linux 64-bit (Debian-based) is available for download too. The dependencies for installing are: 

libboost-iostreams1.74.0 (>= 1.74.0), libc6 (>= 2.34), libgcc-s1 (>= 3.0), libpng16-16 (>= 1.6.2-1)

libqt5core5a (>= 5.15.1), libqt5gui5 (>= 5.0.2), libqt5gui5-gles (>= 5.0.2), libqt5widgets5 (>= 5.15.1), libstdc++6 (>= 11)

[nfitsview1_4-x64.deb](https://github.com/surhh/nfitsview/releases/download/v1.4/nfitsview1_4-x64.deb)


# How to build under Windows

Normally there is no need to build under Windows as the install package is provided. 
Anyway, for building under Windows one would need to download/install/build all the dependencies (boost, zlib, libpng), then fix the
corresponding pathes for the libraries in the CMakeLists.txt file and then build the project using Qt Creator.

The latest version (1.4) of nFITSview for Windows 64-bit to download:

[nfitsview1_4-setup-x64.exe](https://github.com/surhh/nfitsview/releases/download/v1.4/nfitsview1_4-setup-x64.exe)

# About FITS format

The details about FITS format and standards can be found here:

[FITS docs](https://fits.gsfc.nasa.gov/fits_documentation.html)

[FITS standard](https://fits.gsfc.nasa.gov/fits_standard.html)

