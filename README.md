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

The latest version (1.9) of nFITSview install package for Linux 64-bit (Debian-based) is available for download too. The dependencies for installing are: 

libboost-iostreams1.74.0 (>= 1.74.0), libc6 (>= 2.34), libgcc-s1 (>= 3.0), libpng16-16 (>= 1.6.2-1)

libqt5core5a (>= 5.15.1), libqt5gui5 (>= 5.0.2), libqt5gui5-gles (>= 5.0.2), libqt5widgets5 (>= 5.15.1), libstdc++6 (>= 11)

[nfitsview1_9-x64.deb](https://github.com/surhh/nfitsview/releases/download/v1.9/nfitsview1_9-x64.deb)


# How to build under Windows

Normally there is no need to build under Windows as the install package is provided. 
Anyway, for building under Windows one would need to download/install/build all the dependencies (boost, zlib, libpng), then fix the
corresponding pathes for the libraries in the CMakeLists.txt file and then build the project using Qt Creator.

The latest version (1.9) of nFITSview for Windows 64-bit to download:

[nfitsview1_9-setup-x64.exe](https://github.com/surhh/nfitsview/releases/download/v1.9/nfitsview1_9-setup-x64.exe)



# Screenshots

![nfitsview1_9_screenshot_1](https://user-images.githubusercontent.com/109148999/216837974-c10fdd42-956c-497b-8bf0-64ada017d75e.png)
![nfitsview1_9_screenshot_2](https://user-images.githubusercontent.com/109148999/216837976-2f2ffea9-5ddf-4f29-a46e-6f79ab638e7f.png)
![nfitsview1_9_screenshot_3](https://user-images.githubusercontent.com/109148999/216837979-b246c657-a14c-4e1a-aa69-9a10ddbcfa75.png)
![nfitsview1_9_screenshot_4](https://user-images.githubusercontent.com/109148999/216837981-a13858b5-1179-45aa-9848-b39a40811b9e.png)
![nfitsview1_9_screenshot_5](https://user-images.githubusercontent.com/109148999/216837984-2e3cc287-75c8-4e94-8b84-b31b540d95c6.png)
![nfitsview1_9_screenshot_6](https://user-images.githubusercontent.com/109148999/216837987-750b8ec7-97e7-47a9-ba63-cd8164cc115e.png)


# About FITS format

The details about FITS format and standards can be found here:

[FITS docs](https://fits.gsfc.nasa.gov/fits_documentation.html)

[FITS standard](https://fits.gsfc.nasa.gov/fits_standard.html)

