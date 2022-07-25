# About nFITSview  (next FITS viewer)
nFITSview - A simple FITS image viewer

![nfitsview1_1_screenshot_5](https://user-images.githubusercontent.com/109148999/180640402-b17321d1-2002-4112-81db-3fd0c16e0b8f.png)
![nfitsview1_1_screenshot_6](https://user-images.githubusercontent.com/109148999/180640403-01bed71b-ae8b-4fdd-aa3b-2d104e64ea56.png)
![nfitsview1_1_screenshot_7](https://user-images.githubusercontent.com/109148999/180741392-779ce687-e1a1-4937-a806-0b9de9a6c3c7.png)


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

# How to build under Windows

Normally there is no need to build under Windows as the install package is provided. 
Anyway, for building under Windows one would need to download/install/build all the dependencies (boost, zlib, libpng), then fix the
corresponding pathes for the libraries in the CMakeLists.txt file and then build the project using Qt Creator.

# About FITS format

The details about FITS format and standards can be found here:

https://fits.gsfc.nasa.gov/fits_documentation.html

https://fits.gsfc.nasa.gov/fits_standard.html

