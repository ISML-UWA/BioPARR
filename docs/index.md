# BioPARR Documentation

## Installation

### Dependencies
The following programs must be installed to run the BioPARR suite:
- [3D Slicer](https://download.slicer.org/) (tested with 4.4.0)
- [Paraview](https://www.paraview.org/download/) (tested with version 4.3.1)
- [Abaqus](https://www.3ds.com/products-services/simulia/products/abaqus/) (tested with version 6.14)

These programs have been shipped with the BioPARR binaries but your local version could also be used:
- [Gmsh](https://gmsh.info/#Download) (tested with 2.11.0)
- [ACVD](https://github.com/valette/ACVD/)

### Installation procedure

1.  Install the required dependencies.
1.  Clone the [BioPARR GitHub repository](https://github.com/ISML-UWA/BioPARR) or download the [precompiled binaries](https://bioparr.mech.uwa.edu.au/download.php?fid=19) for Windows.
1.  If using the source code, compile the binaries by following the [compilation instructions](https://isml-uwa.github.io/BioPARR#compilation).
1.  Edit the `AAA_Configure` file and set up the paths to 3D Slicer and Paraview based on your installation.

## Compilation

The steps to compile your own binaries for the Slicer extensions are as follows:

1.  Go to the [build instructions for Slicer](https://slicer.readthedocs.io/en/latest/developer_guide/build_instructions/index.html) and follow the steps to install prerequisites and clone Slicer but **do not build yet**.
1.  In the cloned file `Slicer/CMakeLists.txt` find `Slicer_VTK_COMPONENTS` and the line
    ```
    set(Slicer_VTK_COMPONENTS)
    ```
    In the indented section below this line, add these lines:
    ```
        ${VTK_COMPONENT_PREFIX}FiltersVerdict
        ${VTK_COMPONENT_PREFIX}IOGeometry
    ```
1.  Finish building Slicer according [the instructions](https://slicer.readthedocs.io/en/latest/developer_guide/build_instructions/index.html).
1.  Change directory to the directory containing the extension you want to build. For example:
    ```
    cd path/to/BioPARR/Software/SlicerModules/AAA_AverageStress
    ```
1.  Run
    ```
    cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DSlicer_DIR:PATH=~/Applications/Slicer-SuperBuild-Debug/Slicer-build .
    make
    ```
1. Binaries for this extension should now be available.
