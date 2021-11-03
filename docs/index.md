# BioPARR Documentation

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
