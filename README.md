# About nFITSview  (next FITS viewer)
nFITSview - A simple and user-friendly FITS image viewer

Currently nFITSview supports the following formats and features:

-    8-bit images
-    16-bit integer images
-    32-bit floating point and integer images
-    64-bit floating point and integer images
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

The latest version (2.5) of nFITSview install package for Linux 64-bit (Debian-based) is available for download too. The dependencies for installing are: 

libboost-iostreams1.74.0 (>= 1.74.0), libc6 (>= 2.34), libgcc-s1 (>= 3.0), libpng16-16 (>= 1.6.2-1)

libqt5core5a (>= 5.15.1), libqt5gui5 (>= 5.0.2), libqt5gui5-gles (>= 5.0.2), libqt5widgets5 (>= 5.15.1), libstdc++6 (>= 11)

[nfitsview2_5-x64.deb](https://github.com/surhh/nfitsview/releases/download/v2.5/nfitsview2_5-x64.deb)


# How to build under Windows

Normally there is no need to build under Windows as the install package is provided. 
Anyway, for building under Windows one would need to download/install/build all the dependencies (boost, zlib, libpng), then fix the
corresponding pathes for the libraries in the CMakeLists.txt file and then build the project using Qt Creator.

The latest version (2.5) of nFITSview for Windows 64-bit to download:

[nfitsview2_5-setup-x64.exe](https://github.com/surhh/nfitsview/releases/download/v2.5/nfitsview2_5-setup-x64.exe)



# Screenshots

![nfitsview2_5_screenshot_1](https://user-images.githubusercontent.com/109148999/233595717-fcc28b53-f58b-4e61-93fe-df0afe66eb12.png)
![nfitsview2_5_screenshot_2](https://user-images.githubusercontent.com/109148999/233595727-223111b6-fe2d-4f18-a7d9-5c7767bec441.png)
![nfitsview2_5_screenshot_3](https://user-images.githubusercontent.com/109148999/233595739-56ff3969-6f1a-44d9-82bc-2843328edd55.png)
![nfitsview2_5_screenshot_4](https://user-images.githubusercontent.com/109148999/233595743-0eff2e2c-4726-4f50-8a65-da60c278b7c2.png)
![nfitsview2_5_screenshot_5](https://user-images.githubusercontent.com/109148999/233595762-94da0668-4a56-4bd6-a99b-3410605242cb.png)
![nfitsview2_5_screenshot_6](https://user-images.githubusercontent.com/109148999/233595766-1a21ceb6-3b98-4604-8a6f-925c3f704daf.png)


# About FITS format

The details about FITS format and standards can be found here:

[FITS docs](https://fits.gsfc.nasa.gov/fits_documentation.html)

[FITS standard](https://fits.gsfc.nasa.gov/fits_standard.html)

