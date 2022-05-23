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
1.  Edit the `AAA_Configure.py` file and set up the paths to 3D Slicer and Paraview based on your installation.

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

## Analysis Configuration
Edit `AAA_AnalyseCase.py` to change the analysis configuration. By default, both variable thickness (if data for the variable thickness exists) and constant thickness geometries are created and analysed using three scenarios.

For example, if you don't want variable thickness geometry, change
```
AAA_ANALYSE_VAR_THICKNESS = True
```
at the start of the script. If you don't want the constant thickness scenario to be analysed, change
```
AAA_ANALYSE_CONST_THICKNESS = True
```
at the start of the script.

The scenarios to be analysed are configures in `./Software/Scripts/AnalyseAllCases.py`. If you want to remove some of these scenarios from analysis, change the definition of the variable
```
AAA_ANALYSIS_CASES = ("NoILT", "ILTPressure", "WallPressure")
```
at the top of that file.

The actual configuration for each of the scenarios can be found in the Abaqus input file `./Software/Scripts/Abaqus/AAA.inp`. You can modiy these scenarios or add additional scenarios that you want to analyse by creating additional subfolders and Abaqus input files.

The 3D Slicer modules used for analysis are found in `./Software/SlicerModules`. You can run these with the `--help` parameters to find the different configuration options they have and then change their command line parameters. For exmaple, the module for creating the Abaqus input files corresponding to the wall and ILT parts is called from `AAA_AnalyseCase.py` with the options `--quad --hybrid` which means it created hybrid tetrahedral elements with quadratic shape functions and 10 nodes. This may lead to very long analysis time so these options can be removed and linear tetrahedral elements used instead.

## Case Analysis

1.  **Setup**

    Duplicate the folder CaseID and rename it with the ID of the case you are going to analyse. Once this has been created, change the working directory to this folder and run `AAA_AnalyseCase.py`. This will set up the directory structure for you.

    For example, if we were analysing Case11220:
    ```
    cp -r CaseID Case11220
    cd Case11220
    python3 AAA_AnalyseCase.py
    ```
1.  **Check the data**

    Go to the shared data drive and copy the folders with the CT and MRI images you will use for this case. Open them in 3D Slicer and check if appropriate for analysis.
1.  **Cropping**

    Open the CT image in 3D Slicer, define the region of interest (ROI) and crop the volume (Converters -> Crop Volume) with isotropic output voxels. Save result in `./1_Segmentation_CT/CT_cropped.nrrd`.

    Open the MRI image in 3D Slicer, define the region of interest (ROI) and crop the volume (Converters -> Crop Volume) with isotropic output voxels. Save result in `./2_Segmentation_MRI/MRI_cropped.nrrd`.

    **Note:** The ROI should start just under the renal arteries nd end under the bifurcation of the iliac arteries (if visible in the images).
1.  **Segmentation**
    1.  **Lumen**

        Open `./1_Segmentation_CT/CT_cropped.nrrd` in 3D Slicer. Go into `Volumes` and under `Window/Level` select `CT-abdomen`. Go into `Segment Editor` and use the `Threshold` effect to create a rough segmentation of the blood channel. Use `Islands` to eliminate unconnected regions. Use `Paint` to make manual corrections (eliminate small veins, calcification the may be in contact with the blood channel, etc). The changes can be checked by using `Surface Models` -> `ModelMaker` to create a surface representation of your segmentation.

        Once you are satisfied with your manual corrections use `Surface Models`->`Label Map Smoothing` with a Gaussian smoothing parameter `Sigma` of 1 (set both input and output volumes as `CT_cropped-label`) to smooth out the segmentation. Save result to  `./1_Segmentation_CT/CT_blood_label.nrrd`.

        **Note:** Make sure the segmentation extends the entire height of the cropped CT image by checking the first and last red slices in 3D Slicer before saving the segmentation results.
    1. **AAA**

        Use `Paint`to extend the blood channel label to cover the entire AAA. Use a different label to mark the region around the AAA. Use `FastGrowCutEffect` to segment the AAA. Once satisfied with the result stop `FastGrowCutEffect`, use `ChangeLabelEffect` to change the label used to mark the region around the AAA to background colour and use `Paint` to make manual corrections.

        Once you are satisfied with your manual corrections use `Surface Models`->`Label Map Smoothing` with a Gaussian smoothing parameter `Sigma` of 1 (set both input and output volumes as `CT_cropped-label`) to smooth out the segmentation. Save result to  `./1_Segmentation_CT/CT_AAA_label.nrrd`.

        **Note:** Make sure the segmentation extends the entire height of the cropped CT image by checking the first and last red slices in 3D Slicer before saving the segmentation results.

        **Tip:** Use a sphere in `Paint` to mark regions in several slices at once. Take care not to mark incorrect regions in the slices you don't see.
    1.  **AAA from MRI**

        Use the same procedure as above to segment the AAA from the MRI. The lumen does not need to be segmented separately in this case. Save results to `./2_Segmentation_MRI/MRI_AAA_label.nrrd`.
1.  **Wall extraction and MRI to CT registration**

    Run `AAA_AnalyseCase.py` from the command prompt. The script will do this step automatically based on the segmentation data.
1.  **Wall thickness extraction**

    Open `./1_Segmentation_CT/CT_cropped.nrrd` and `./2_Segmentation_MRI/MRI_aligned_to_CT.nrrd` (created by the script) in 3D Slicer. If you set the as `Background` and `Foreground` images you can blend them together and check they are aligned. Look for areas where the wall is visible in the MRI (or calcifications in CT) and use the ruler to measure the wall thickness. Save measurements in `./3_Thickness/M.acsv`.
1.  **Complete the analysis**   

    Run `AAA_AnalyseCase.py` from the command prompt. The script will create the AAA and ILT surfaces, mesh the AAA and ILT volumes, create the Abaqus input files, run the Abaqus analysis for the prescribed loading configurations, extract the stress in the wall and compute the rupture potential index (RPI).
1.  **Check the results**

    If there are no errors during the script execution, open some of the result files and check the output.

    If there are errors, you must solve them so the analysis can progress further (this may involve cleaning up the segmentations or using different parameters for meshing). Then rerun `AAA_AnalyseCase.py`.

    If you cannot solve your error on your own please feel free to submit an issue [here](https://github.com/ISML-UWA/BioPARR/issues).

## Tutorials

Tutorials describing the different steps of the analysis can be found [here](https://bioparr.mech.uwa.edu.au/#tutorials).
