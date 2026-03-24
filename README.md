# nFITSview - A user-friendly FITS image viewer for astronomers

Currently nFITSview supports the following formats and features:

-    8-bit images
-    16-bit integer images
-    32-bit floating point and integer images
-    64-bit floating point and integer images
-    Exporting image HDUs as PNG/TIFF/JPEG/BMP/PPM files
-    Bulk exporting of all image HDUs as PNG files
-    Percentile and stretching support
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


The latest version (3.8) of nFITSview install package for Linux 64-bit (Debian-based) is available for download too. 

The dependencies for installing are: 
libboost-iostreams1.88.0 (>= 1.88.0), libc6 (>= 2.34), libgcc-s1 (>= 3.0), libpng16-16t64 (>= 1.6.46), 
libqt6charts6 (>= 6.6.1), libqt6core6t64 (>= 6.9.1), libqt6gui6 (>= 6.1.2), libqt6network6 (>= 6.1.2), 
libqt6widgets6 (>= 6.1.2), libstdc++6 (>= 14)

[nfitsview3_8-x64.deb](https://github.com/surhh/nfitsview/releases/download/v3.8/nfitsview3_8-x64.deb)


# How to build under Windows

Normally there is no need to build under Windows as the install package is provided. 
Anyway, for building under Windows one would need to download/install/build all the dependencies (boost, zlib, libpng), then fix the
corresponding pathes for the libraries in the CMakeLists.txt file and then build the project using Qt Creator.

The latest version (3.8) of nFITSview for Windows 64-bit to download (starting from version 3.6 Windows 10 or later is required):

[nfitsview3_8-setup-x64.exe](https://github.com/surhh/nfitsview/releases/download/v3.8/nfitsview3_8-setup-x64.exe)

# Experimental (portable) version 3.8 for macOS will be available soon


# Screenshots

<img width="2050" height="1174" alt="nfitsview3_8_screenshot_1" src="https://github.com/user-attachments/assets/317185a5-6c1d-4022-b7ec-417640985f8f" />
<img width="2050" height="1174" alt="nfitsview3_8_screenshot_2" src="https://github.com/user-attachments/assets/7aaada6d-f81d-417f-9f90-309cb1634a7c" />
<img width="2050" height="1174" alt="nfitsview3_8_screenshot_3" src="https://github.com/user-attachments/assets/1f576240-0b32-4637-9263-f3aace5a9f43" />


# About FITS format

The details about FITS format and standards can be found here:

[FITS docs](https://fits.gsfc.nasa.gov/fits_documentation.html)

[FITS standard](https://fits.gsfc.nasa.gov/fits_standard.html)

