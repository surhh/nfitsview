# About nFITSview  (next FITS viewer)
nFITSview - A simple and user-friendly FITS image viewer

Currently nFITSview supports the following formats and features:

-    8-bit images
-    16-bit integer images
-    32-bit floating point and integer images
-    64-bit floating point and integer images (experimental, may still not work as it should)
-    Exporting image HDUs as PNG/TIFF/JPEG/BMP files
-    Bulk exporting of all image HDUs as PNG files
-    RGB gamma correction, grayscale and "Eye Comfort" filter
-    Image zoom in/out
-    HDU header syntax view
-    Raw data hex preview
-    Only uncompressed FITS data is supported 
-    Command line exporting of FITS file  (see -h, -e command line switches).
     
     *Note: the console output is not visible on Windows platform. The command line 
     supports image exporting only in "Original" mapping mode.*
    
# How to build under Linux

The original development of nFITSview is done under Linux, so the easiest and the fastest way to build is under Linux.

- just build in Qt Creator. 
- it should be also possible to build using CMake from the command line.

The latest version (1.8) of nFITSview install package for Linux 64-bit (Debian-based) is available for download too. The dependencies for installing are: 

libboost-iostreams1.74.0 (>= 1.74.0), libc6 (>= 2.34), libgcc-s1 (>= 3.0), libpng16-16 (>= 1.6.2-1)

libqt5core5a (>= 5.15.1), libqt5gui5 (>= 5.0.2), libqt5gui5-gles (>= 5.0.2), libqt5widgets5 (>= 5.15.1), libstdc++6 (>= 11)

[nfitsview1_8-x64.deb](https://github.com/surhh/nfitsview/releases/download/v1.8/nfitsview1_8-x64.deb)


# How to build under Windows

Normally there is no need to build under Windows as the install package is provided. 
Anyway, for building under Windows one would need to download/install/build all the dependencies (boost, zlib, libpng), then fix the
corresponding pathes for the libraries in the CMakeLists.txt file and then build the project using Qt Creator.

The latest version (1.8) of nFITSview for Windows 64-bit to download:

[nfitsview1_8-setup-x64.exe](https://github.com/surhh/nfitsview/releases/download/v1.8/nfitsview1_8-setup-x64.exe)



# Screenshots

![nfitsview1_8_screenshot_1](https://user-images.githubusercontent.com/109148999/212899955-acf95a8b-744a-4b7b-b577-3752cae9ba3a.png)
![nfitsview1_8_screenshot_2](https://user-images.githubusercontent.com/109148999/212899960-62cd3e6c-f235-495d-b99c-06d64c3885f3.png)
![nfitsview1_8_screenshot_3](https://user-images.githubusercontent.com/109148999/212899965-f366681d-4ba5-4718-aff2-34386c65707b.png)
![nfitsview1_8_screenshot_4](https://user-images.githubusercontent.com/109148999/212899974-fa6f6e0b-f3ba-4ed0-959c-dd4a30370f3f.png)
![nfitsview1_8_screenshot_5](https://user-images.githubusercontent.com/109148999/212899980-338dec6c-6be0-4bd1-81c5-552cfe41bfea.png)



# About FITS format

The details about FITS format and standards can be found here:

[FITS docs](https://fits.gsfc.nasa.gov/fits_documentation.html)

[FITS standard](https://fits.gsfc.nasa.gov/fits_standard.html)

