cmake_minimum_required(VERSION 3.5)

project(nfitsview LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)      # added for libnfits

set(Boost_USE_MULTITHREADED ON)     # added for libnfits

set(ENABLE_OPENMP OFF) # Enable OpenMP support

# windows build specific (MinGW)
if(WIN32)
    include_directories("d:/build-lpng1637-Desktop-Release")

    set(ZLIB_INCLUDE_DIR "d:/zlib-1.2.12")
    set(ZLIB_LIBRARY "d:/build-zlib-1.2.12-Desktop-Release/libzlibstatic.a")

    set(Boost_INCLUDE_DIR "d:/boost/include/boost-1_79")
    set(Boost_LIBRARY_DIR "d:/boost/lib")
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_MULTITHREADED ON)
    #set(Boost_USE_STATIC_RUNTIME OFF)

    find_package(Boost REQUIRED iostreams)

    include_directories(${Boost_INCLUDE_DIR})

if(NOT DEFINED ${ENABLE_OPENMP})
    add_link_options("-static")
endif()

    set(PNG_PNG_INCLUDE_DIR "d:/lpng1637")
    set(PNG_LIBRARY "d:/build-lpng1637-Desktop-Release/libpng.a")
endif()
# end of windows build specific (MinGW)

# added for libnfits
find_package(Boost REQUIRED iostreams)
find_package(PNG REQUIRED)

include_directories(${PNG_INCLUDE_DIR})

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)


set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        workspacetabwidget.h
        workspacetabwidget.cpp
        aboutdialog.h
        aboutdialog.cpp
        defsui.h
        mainwindow.ui
        workspacetabwidget.ui
        aboutdialog.ui
        resources.qrc

        libnfits/defs.h
        libnfits/keywords.h
        libnfits/headerrecord.cpp
        libnfits/headerrecord.h
        libnfits/helperfunctions.cpp
        libnfits/helperfunctions.h
        libnfits/header.cpp
        libnfits/header.h
        libnfits/hdu.cpp
        libnfits/hdu.h
        libnfits/helperio.cpp
        libnfits/helperio.h
        libnfits/fitsfile.cpp
        libnfits/fitsfile.h
        libnfits/image.cpp
        libnfits/image.h
        libnfits/table.cpp
        libnfits/table.h
        libnfits/pngfile.cpp
        libnfits/pngfile.h
        libnfits/fits2png.cpp
        libnfits/fits2png.h
)

# windows build specific (MinGW)
if(WIN32)
    set(PROJECT_WIN_RESOURCE winappicon.rc)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} ${PROJECT_WIN_RESOURCE})
endif()
# end of windows build specific  (MinGW)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(nfitsview ${PROJECT_SOURCES})
    # windows build specific (MinGW)
    if(WIN32)
        qt_add_executable(nfitsview WIN32 ${PROJECT_SOURCES})
    # end of windows build specific (MinGW)
    else()
        qt_add_executable(nfitsview ${PROJECT_SOURCES})
    endif()
else()
    # windows build specific (MinGW)
    if(WIN32)
        add_executable(nfitsview WIN32 ${PROJECT_SOURCES})
    # end of windows build specific (MinGW)
    else()
        add_executable(nfitsview ${PROJECT_SOURCES})
    endif()
endif()

# windows build specific (MinGW), is needed to have icons visible in the toolbar and menu
if(WIN32)
    qt5_import_plugins(${PROJECT_NAME} INCLUDE Qt5::QSvgPlugin)
endif()
# end of windows build specific (MinGW)

# target_link_libraries(nfitsview PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)  # the original one
target_link_libraries(nfitsview PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${Boost_LIBRARIES} ${PNG_LIBRARY}) # added for libnfits

if(ENABLE_OPENMP)
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(nfitsview PRIVATE OpenMP::OpenMP_CXX)
endif()
endif()
# End of OpenMP support block

