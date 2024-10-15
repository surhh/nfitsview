# nFITSview - A user-friendly FITS image viewer for astronomers

Currently nFITSview supports the following formats and features:

-    8-bit images
-    16-bit integer images
-    32-bit floating point and integer images
-    64-bit floating point and integer images
-    Exporting image HDUs as PNG/TIFF/JPEG/BMP files
-    Bulk exporting of all image HDUs as PNG files
-    RGB gamma correction, grayscale and "Eye Comfort" filters
-    Filtering by pixel threshold value
-    Image zoom in/out
-    HDU header syntax view
-    Raw data hex preview
-    Both regular and compressed (.fz) FITS files are supported
-    Command line exporting of FITS file  (see -h, -e command line switches)
     
     *Note: the console output is not visible on Windows platform. The command line 
     supports image exporting only in "Original" mapping mode.*
    
# How to build under Linux

The original development of nFITSview is done under Linux, so the easiest and the fastest way to build is under Linux.

- just build in Qt Creator. 
- it should be also possible to build using CMake from the command line.

The latest version (3.1) of nFITSview install package for Linux 64-bit (Debian-based) is available for download too. The dependencies for installing are: 

libboost-iostreams1.83.0 (>= 1.83.0), libc6 (>= 2.34), libgcc-s1 (>= 3.0), libgomp1 (>= 4.9), 

libpng16-16t64 (>= 1.6.2), libqt5core5t64 (>= 5.15.1), libqt5gui5t64 (>= 5.0.2) | libqt5gui5-gles (>= 5.0.2),

libqt5network5t64 (>= 5.0.2), libqt5widgets5t64 (>= 5.15.1), libstdc++6 (>= 13.1)

[nfitsview3_1-x64.deb](https://github.com/surhh/nfitsview/releases/download/v3.1/nfitsview3_1-x64.deb)


# How to build under Windows

Normally there is no need to build under Windows as the install package is provided. 
Anyway, for building under Windows one would need to download/install/build all the dependencies (boost, zlib, libpng), then fix the
corresponding pathes for the libraries in the CMakeLists.txt file and then build the project using Qt Creator.

The latest version (3.1) of nFITSview for Windows 64-bit to download:

[nfitsview3_1-setup-x64.exe](https://github.com/surhh/nfitsview/releases/download/v3.1/nfitsview3_1-setup-x64.exe)



# Screenshots

![nfitsview2_9_screenshot_1](https://github.com/surhh/nfitsview/assets/109148999/3a671fbb-c7b3-4363-b88b-eb063e690e86)

![nfitsview2_9_screenshot_2](https://github.com/surhh/nfitsview/assets/109148999/b3db3e6a-717f-4a40-8009-990f974f9d5e)

![nfitsview2_9_screenshot_3](https://github.com/surhh/nfitsview/assets/109148999/d007fd4f-a670-41c2-9945-43c598f0e70c)

![nfitsview2_9_screenshot_4](https://github.com/surhh/nfitsview/assets/109148999/57de7b91-4b57-4808-9f5b-5ea4ac179742)

![nfitsview2_9_screenshot_5](https://github.com/surhh/nfitsview/assets/109148999/dd36a34f-3468-4201-8547-410ccdb30be2)

![nfitsview2_9_screenshot_6](https://github.com/surhh/nfitsview/assets/109148999/0328e7ec-4925-4364-8c34-feab5ff115ec)

# About FITS format

The details about FITS format and standards can be found here:

[FITS docs](https://fits.gsfc.nasa.gov/fits_documentation.html)

[FITS standard](https://fits.gsfc.nasa.gov/fits_standard.html)

