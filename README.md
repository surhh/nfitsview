# About nFITSview
nFITSview - A simple FITS image viewer

![nfitsview1_0_screenshot_1](https://user-images.githubusercontent.com/109148999/178692087-7178728a-4a7d-4e69-9ff7-7b3ba27065e1.png)
![nfitsview1_0_screenshot_2](https://user-images.githubusercontent.com/109148999/178692123-9539f0aa-5895-4b03-add9-f59656da89da.png)

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
Anyway, for building under Windows one would need to download/install/build all the dependencies, then fix the
corresponding pathes for the libraries in the CMakeLists.txt file and then build the project using Qt Creator.

