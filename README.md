# About nFITSview  (next FITS viewer)
nFITSview - A user-friendly FITS image viewer

Currently nFITSview supports the following formats and features:

-    8-bit images
-    16-bit integer images
-    32-bit floating point and integer images
-    64-bit floating point and integer images
-    Exporting image HDUs as PNG/TIFF/JPEG/BMP files
-    Bulk exporting of all image HDUs as PNG files
-    RGB gamma correction, grayscale and "Eye Comfort" filters
-    Image zoom in/out
-    HDU header syntax view
-    Raw data hex preview
-    Both regular and compressed (.fz) FITS files are supported
-    Command line exporting of FITS file  (see -h, -e command line switches).
     
     *Note: the console output is not visible on Windows platform. The command line 
     supports image exporting only in "Original" mapping mode.*
    
# How to build under Linux

The original development of nFITSview is done under Linux, so the easiest and the fastest way to build is under Linux.

- just build in Qt Creator. 
- it should be also possible to build using CMake from the command line.

The latest version (2.7) of nFITSview install package for Linux 64-bit (Debian-based) is available for download too. The dependencies for installing are: 

libboost-iostreams1.74.0 (>= 1.74.0), libc6 (>= 2.34), libgcc-s1 (>= 3.0), libpng16-16 (>= 1.6.2-1)

libqt5core5a (>= 5.15.1), libqt5gui5 (>= 5.0.2), libqt5gui5-gles (>= 5.0.2), libqt5widgets5 (>= 5.15.1), libstdc++6 (>= 11)

[nfitsview2_7-x64.deb](https://github.com/surhh/nfitsview/releases/download/v2.7/nfitsview2_7-x64.deb)


# How to build under Windows

Normally there is no need to build under Windows as the install package is provided. 
Anyway, for building under Windows one would need to download/install/build all the dependencies (boost, zlib, libpng), then fix the
corresponding pathes for the libraries in the CMakeLists.txt file and then build the project using Qt Creator.

The latest version (2.7) of nFITSview for Windows 64-bit to download:

[nfitsview2_7-setup-x64.exe](https://github.com/surhh/nfitsview/releases/download/v2.7/nfitsview2_7-setup-x64.exe)



# Screenshots

![nfitsview2_6_screenshot_1](https://github.com/surhh/nfitsview/assets/109148999/4fe62931-b68e-4a60-884f-a90b0615061b)
![nfitsview2_6_screenshot_2](https://github.com/surhh/nfitsview/assets/109148999/a600f285-123d-400e-885a-e7f2a1796104)
![nfitsview2_6_screenshot_3](https://github.com/surhh/nfitsview/assets/109148999/29c624c7-94da-4578-9304-8a7ca66e2cdb)
![nfitsview2_6_screenshot_4](https://github.com/surhh/nfitsview/assets/109148999/36b08e90-85d3-4ff5-85d4-81d4bca920b5)
![nfitsview2_6_screenshot_5](https://github.com/surhh/nfitsview/assets/109148999/cbccd45a-6899-40bb-b4bd-5d3f253e22c6)
![nfitsview2_6_screenshot_6](https://github.com/surhh/nfitsview/assets/109148999/6648023a-95b8-4805-a5b8-b33ab8dcc054)

# About FITS format

The details about FITS format and standards can be found here:

[FITS docs](https://fits.gsfc.nasa.gov/fits_documentation.html)

[FITS standard](https://fits.gsfc.nasa.gov/fits_standard.html)

