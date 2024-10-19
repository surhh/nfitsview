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

The latest version (3.2) of nFITSview install package for Linux 64-bit (Debian-based) is available for download too. The dependencies for installing are: 

libboost-iostreams1.83.0 (>= 1.83.0), libc6 (>= 2.34), libgcc-s1 (>= 3.0), libgomp1 (>= 4.9), 

libpng16-16t64 (>= 1.6.2), libqt5core5t64 (>= 5.15.1), libqt5gui5t64 (>= 5.0.2) | libqt5gui5-gles (>= 5.0.2),

libqt5network5t64 (>= 5.0.2), libqt5widgets5t64 (>= 5.15.1), libstdc++6 (>= 13.1)

[nfitsview3_2-x64.deb](https://github.com/surhh/nfitsview/releases/download/v3.2/nfitsview3_2-x64.deb)


# How to build under Windows

Normally there is no need to build under Windows as the install package is provided. 
Anyway, for building under Windows one would need to download/install/build all the dependencies (boost, zlib, libpng), then fix the
corresponding pathes for the libraries in the CMakeLists.txt file and then build the project using Qt Creator.

The latest version (3.2) of nFITSview for Windows 64-bit to download:

[nfitsview3_2-setup-x64.exe](https://github.com/surhh/nfitsview/releases/download/v3.2/nfitsview3_2-setup-x64.exe)



# Screenshots

![nfitsview3_2_screenshot_1](https://github.com/user-attachments/assets/a1a69547-7c62-4eae-a524-1672f0583d4d)
![nfitsview3_2_screenshot_2](https://github.com/user-attachments/assets/1081da44-04a0-4430-96b6-afc5225e376b)
![nfitsview3_2_screenshot_3](https://github.com/user-attachments/assets/32ba4c7b-9c93-44f4-a745-6e034787d4d9)
![nfitsview3_2_screenshot_4](https://github.com/user-attachments/assets/f939aa94-e242-4382-9dce-9c51066fcedb)
![nfitsview3_2_screenshot_5](https://github.com/user-attachments/assets/8337df2b-4558-45df-9122-7307ad0b495d)
![nfitsview3_2_screenshot_6](https://github.com/user-attachments/assets/0a0094f6-6bef-40ca-b11c-920bbd32d951)
![nfitsview3_2_screenshot_7](https://github.com/user-attachments/assets/ccf21653-020c-4822-a3f5-f170dd76e476)
![nfitsview3_2_screenshot_8](https://github.com/user-attachments/assets/bf90ac17-3a7a-4b13-8212-e1c20520d929)


# About FITS format

The details about FITS format and standards can be found here:

[FITS docs](https://fits.gsfc.nasa.gov/fits_documentation.html)

[FITS standard](https://fits.gsfc.nasa.gov/fits_standard.html)

