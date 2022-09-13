# About nFITSview  (next FITS viewer)
nFITSview - A simple and user-friendly FITS image viewer

![nfitsview1_2_screenshot_1](https://user-images.githubusercontent.com/109148999/189863673-95bae9c0-0a9f-4d1a-8cbe-db80a276a623.png)
![nfitsview1_2_screenshot_2](https://user-images.githubusercontent.com/109148999/189863690-7321a797-b92a-4ad3-ae87-1c04b896c842.png)
![nfitsview1_2_screenshot_3](https://user-images.githubusercontent.com/109148999/189863700-1610f79c-fa03-471e-8619-888ae8559e22.png)


Currently nFITSview supports the following formats and features:

    - 32-bit floating images
    - 64-bit floating images (experimental, may still not work as it should)
    - Exporting image HDUs as PNG/JPEG/BMP files
    - Bulk exporting of all image HDUs as PNG files
    - RGB gamma correction and grayscaling
    - Image zoom in/out
    - HDU header syntax view
    - Raw data hex preview
    
# How to build under Linux

The original development of nFITSview is done under Linux, so the easiest and the fastest way to build is under Linux.

- just build in Qt Creator. 
- it should be also possible to build using CMake from the command line.

The latest build for Linux (Debian-based) is now available for download as well. The dependencies for install are:

libboost-iostreams1.74.0 (>= 1.74.0), libc6 (>= 2.34), libgcc-s1 (>= 3.0), libpng16-16 (>= 1.6.2-1)

libqt5core5a (>= 5.15.1), libqt5gui5 (>= 5.0.2), libqt5gui5-gles (>= 5.0.2), libqt5widgets5 (>= 5.15.1), libstdc++6 (>= 11)

[nfitsview-1_2.deb](https://github.com/surhh/nfitsview/releases/download/v1.2/nfitsview-1_2.deb)

# How to build under Windows

Normally there is no need to build under Windows as the install package is provided. 
Anyway, for building under Windows one would need to download/install/build all the dependencies (boost, zlib, libpng), then fix the
corresponding pathes for the libraries in the CMakeLists.txt file and then build the project using Qt Creator.

The latest version (1.2) of nFITSview for Windows 64-bit to download:

[nfitsview1_2-setup-x64.exe](https://github.com/surhh/nfitsview/releases/download/v1.2/nfitsview1_2-setup-x64.exe)

# About FITS format

The details about FITS format and standards can be found here:

[FITS docs](https://fits.gsfc.nasa.gov/fits_documentation.html)

[FITS standard](https://fits.gsfc.nasa.gov/fits_standard.html)

