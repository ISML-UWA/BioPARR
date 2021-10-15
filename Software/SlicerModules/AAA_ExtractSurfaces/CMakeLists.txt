cmake_minimum_required(VERSION 2.8.9)

project(AAA_ExtractSurfaces)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://www.example.com/Slicer/Extensions/AAA_MeshDensityGMSH")
set(EXTENSION_CATEGORY "AAA")
set(EXTENSION_CONTRIBUTORS "Grand Joldes (The University of Western Australia, ISML)")
set(EXTENSION_DESCRIPTION "Extracts the interior and exterior surfaces of an AAA oriented along the z axis.")
set(EXTENSION_ICONURL "http://www.example.com/Slicer/Extensions/AAA_MeshDensityGMSH.png")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/AAA_MeshDensityGMSH/Screenshots/1.png")

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(AAA_ExtractSurfaces)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_CPACK})
