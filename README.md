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

- Just build in Qt Creator. 
- It should be also possible to build using CMake from the command line. Run the following cmake commands:

  cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE="-O3 -DNDEBUG" -B build -S .
  cmake --build build

  The nfitsview execuatble will be located in the build directory.

The latest version (3.5) of nFITSview install package for Linux 64-bit (Debian-based) is available for download too. The dependencies for installing are: 

libboost-iostreams1.83.0 (>= 1.83.0)   
libc6 (>= 2.34)  
libgcc-s1 (>= 3.0)   
libgomp1 (>= 4.9)   
libpng16-16t64 (>= 1.6.2)   
libqt6core6t64 (>= 6.6.0)   
libqt6gui6 (>= 6.1.2)   
libqt6network6 (>= 6.1.2)   
libqt6widgets6 (>= 6.1.2)   
libstdc++6 (>= 14)

[nfitsview3_5-x64.deb](https://github.com/surhh/nfitsview/releases/download/v3.5/nfitsview3_5-x64.deb)


# How to build under Windows

Normally there is no need to build under Windows as the install package is provided. 
Anyway, for building under Windows one would need to download/install/build all the dependencies (boost, zlib, libpng), then fix the
corresponding pathes for the libraries in the CMakeLists.txt file and then build the project using Qt Creator.

The latest version (3.5) of nFITSview for Windows 64-bit to download:

[nfitsview3_5-setup-x64.exe](https://github.com/surhh/nfitsview/releases/download/v3.5/nfitsview3_5-setup-x64.exe)



# Screenshots

![nfitsview3_5_screenshot_8](https://github.com/user-attachments/assets/facb330e-2467-4ee2-b6b6-91521380a172)
![nfitsview3_5_screenshot_7](https://github.com/user-attachments/assets/0766be56-2d17-4322-803d-2e02e2a173ab)
![nfitsview3_5_screenshot_6](https://github.com/user-attachments/assets/95d16127-3969-46b5-94bd-6400373cf4c7)
![nfitsview3_5_screenshot_5](https://github.com/user-attachments/assets/749fb80c-3da2-4eef-8062-5166777707a3)
![nfitsview3_5_screenshot_4](https://github.com/user-attachments/assets/e3c0f9d2-d47a-4d2b-9371-82b245dbf973)
![nfitsview3_5_screenshot_3](https://github.com/user-attachments/assets/7fc4a99e-8534-4321-901f-a4a5c08d7a3a)
![nfitsview3_5_screenshot_2](https://github.com/user-attachments/assets/85fdfd7c-5ec1-44ab-afb8-163ce9b5b2e2)
![nfitsview3_5_screenshot_1](https://github.com/user-attachments/assets/f5869e9c-58cb-449b-af56-d5db981c8eaf)


# About FITS format

The details about FITS format and standards can be found here:

[FITS docs](https://fits.gsfc.nasa.gov/fits_documentation.html)

[FITS standard](https://fits.gsfc.nasa.gov/fits_standard.html)

