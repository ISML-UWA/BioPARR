cmake_minimum_required(VERSION 2.8.9)

project(AAA_GenerateAbaqusFile)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://www.example.com/Slicer/Extensions/AAA_GenerateAbaqusFile")
set(EXTENSION_CATEGORY "AAA")
set(EXTENSION_CONTRIBUTORS "Grand Joldes (The University of Western Australia, ISML)")
set(EXTENSION_DESCRIPTION "Generates an Abaqus file containing the geometry for FEM analysis.")
set(EXTENSION_ICONURL "http://www.example.com/Slicer/Extensions/AAA_GenerateAbaqusFile.png")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/AAA_GenerateAbaqusFile/Screenshots/1.png")

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(AAA_GenerateAbaqusFile)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_CPACK})
