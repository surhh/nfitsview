# About nFITSview
nFITSview - A simple FITS image viewer

![nfitsview1_0_screenshot_3](https://user-images.githubusercontent.com/109148999/178747900-f918513e-dab8-4831-a93a-8ff002db2439.png)
![nfitsview1_0_screenshot_4](https://user-images.githubusercontent.com/109148999/178747932-4fc68cac-f7a2-498c-a251-cb2ecf5f4da0.png)



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

