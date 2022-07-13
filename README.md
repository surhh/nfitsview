# nfitsview
nFITSview - A simple FITS image viewer

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

The easiest way to build is to use Qt Creator. 
It should be also possible to build using CMake from the command line.

# How to build under Windows

Normally there is no need to build under Windows as the install package is provided. 
Anyway, for building under Windows one would need to download/install/build all the dependencies, then fix the
corresponding pathes for the libraries in the CMakeLists.txt file and then build the project using Qt Creator.
