#   CONTENT
#   Author: Grand Roman Joldes
#   E-mail: grand.joldes@uwa.edu.au
#   PYTHON TRANSLATION
#   AUTHOR: Gerry Gralton
#   EMAIL:  gerry.gralton@uwa.edu.au

import os
import subprocess
import shutil
from pathlib import Path
import runpy
import warnings
import glob

AAA_REMOVE_INTERMEDIATE_RESULTS = True
AAA_ANALYSE_VAR_THICKNESS = True
AAA_ANALYSE_CONST_THICKNESS = True
AAA_USE_QUADRATIC_ELEMENTS = True
AAA_USE_HYBRID_ELEMENTS = True

CASE_FOLDER = Path(".").resolve()

print("Configuring paths")
os.chdir("..")
runpy.run_path("AAA_Configure.py")
os.chdir(CASE_FOLDER)

# Create the template directory structure
if not Path("1_Segmentation_CT").is_dir():
    print("Creating folder structure")
    os.mkdir("1_Segmentation_CT")
    os.mkdir("2_Segmentation_MRI")
    os.mkdir("3_Thickness")
    os.mkdir("4_Geometry")
    os.mkdir("5_Mesh_Const_Thickness")
    os.mkdir("6_Mesh_Var_Thickness")
    os.mkdir("7_Abaqus_Const_Thickness")
    os.mkdir("8_Abaqus_Var_Thickness")

    os.mkdir("5_Mesh_Const_Thickness/Mesh")
    os.mkdir("5_Mesh_Const_Thickness/Thickness")

    # Write file containing constant thickness value
    os.chdir("5_Mesh_Const_Thickness/Thickness")
    f = open("M.acsv", "w")
    f.write("point|0|0|0|\n")
    f.write("point|0|0|1.5|\n")
    f.close()
    os.chdir("../..")

    os.mkdir("6_Mesh_Var_Thickness/Mesh")
    print("Done!")


print("Creating AAA wall ...")
os.chdir("1_Segmentation_CT")
if not Path("CT_wall.vtp").exists():
    if not Path("CT_cropped.nrrd").exists():
        raise FileNotFoundError(
            "Please crop the CT volume and save it as 1_Segmentation_CT/CT_cropped.nrrd!")
    if not Path("CT_AAA_label.nrrd").exists():
        raise FileNotFoundError(
            "Please segment the AAA from CT and save label as 1_Segmentation_CT/CT_AAA_label.nrrd!")
    if not Path("CT_blood_label.nrrd").exists():
        warnings.warn("""
            No blood label found in 1_Segmentation_CT/CT_blood_label.nrrd!
            ILT volume will not be created!""")
        # TODO: Add choice for continuing
        # if input("Do you want to continue? "):
        #     raise FileNotFoundError("No blood label found.")

    extract_wall_path = os.environ["SCRIPTS_PATH"] / Path("ExtractWall.py")
    subprocess.call([os.environ["SLICER_PATH"],
                     "--python-script", extract_wall_path, "."])

    if not Path("CT_wall.vtp").exists():
        raise Exception(
            "Error extracting AAA Wall. Check script ExtractWall.py!")
else:
    print("Skipped! - 1_Segmentation_CT/CT_wall.vtp already exists")

if AAA_REMOVE_INTERMEDIATE_RESULTS:
    os.remove("CT_Wall_label.nrrd")

os.chdir("..")

print("Registering MRI to CT ...")
os.chdir("2_Segmentation_MRI")
if not Path("MRI_aligned_to_CT.nrrd").exists():
    if not Path("MRI_cropped.nrrd").exists():
        warnings.warn("""
            Please crop the MRI volume and save it as 2_Segmentation_MRI/MRI_cropped.nrrd!
            Registration of MRI to CT skipped!""")
    elif not Path("MRI_AAA_label.nrrd").exists():
        warnings.warn("""
            Warning: Please segment the AAA from MRI and save label as 2_Segmentation_MRI/MRI_AAA_label.nrrd!
            Registration of MRI to CT skipped!""")
    else:
        if Path("registerMRI2CT.h5").exists():
            os.remove("registerMRI2CT.h5")
        if Path("MRI_aligned_to_CT.nrrd").exists():
            os.remove("MRI_aligned_to_CT.nrrd")

        # TODO: ADD DEBUG MODE (SEE .BAT FILE FOR REFERENCE)
        mri2ct_path = os.environ["SCRIPTS_PATH"] / Path("RegisterMRI2CT.py")
        subprocess.call([os.environ["SLICER_PATH"],
                         "--no-main-window", "--python-script",
                         mri2ct_path, "."])

        if not Path("registerMRI2CT.h5").exists():
            raise Exception("Error registering MRI to CT label maps!")
        if not Path("MRI_aligned_to_CT.nrrd").exists():
            raise Exception("Error registering MRI to CT!")
else:
    print("Skipped! - 2_Segmentation_MRI/MRI_aligned_to_CT.nrrd already exists")

if AAA_REMOVE_INTERMEDIATE_RESULTS:
    os.remove("registerMRI2CT.h5")
    os.remove("registerMRI2CT_Inverse.h5")

os.chdir("..")

if not Path("3_Thickness/M*.acsv").exists():
    warnings.warn("""
        No thickness info in 3_Thickness folder!
        Only constant thickness AAA will be created!""")
    # TODO: Add choice for continuing

print("Extracting surfaces ...")
os.chdir("4_Geometry")
if not Path("Exterior.vtp").exists():
    path_extract_surfaces = Path(
        os.environ["SCRIPTS_PATH"], "AAA_Extract_surfaces.py")
    runpy.run_path(path_extract_surfaces)

    if not Path("Exterior.vtp").exists():
        raise Exception(
            "Error extracting surfaces. Check script AAA_Extract_surfaces.py!")
    print("Done!")
else:
    print("Skipped! - ./4_Geometry/Exterior.vtp already exists")

if AAA_REMOVE_INTERMEDIATE_RESULTS:
    to_del = glob.glob("./*.stl")
    to_del = to_del + glob.glob("./*0.vtp")
    to_del = to_del + glob.glob("./*.ply")
    
    for file in to_del:
        os.remove(file)

os.chdir("..")

if AAA_ANALYSE_CONST_THICKNESS:
    print("Creating constant thickness volumes ...")

    os.chdir("5_Mesh_Const_Thickness")
    if not Path("AAA_Wall_External.stl").exists():
        runpy.run_path("AAA_CreateConstThicknessVolumes.py")

        if not Path("AAA_Wall_External.stl").exists():
            raise Exception(
                "Error creating constant thickness volumes. Check script AAA_CreateConstThicknessVolumes.bat!")
        print("Done!")
    else:
        print("Skipped! - 5_Mesh_Const_Thickness/AAA_Wall_External.stl already exists")
    os.chdir("..")

if AAA_REMOVE_INTERMEDIATE_RESULTS:
    os.chdir("5_Mesh_Const_Thickness")
    os.remove("AAA.msh")
    os.remove("AAA_merged.vtp")
    os.remove("AAA_Wall_Cap.stl")
    os.remove("AAA_ILT_Cap.stl")
    os.chdir("..")

if AAA_ANALYSE_VAR_THICKNESS:
    print("Creating variable thickness volumes ...")

    os.chdir("6_Mesh_Var_Thickness")
    if not Path("AAA_Wall_External.stl").exists():
        runpy.run_path("AAA_CreateVarThicknessVolumes.py")

        if not Path("AAA_Wall_External.stl").exists():
            raise Exception(
                "Error creating constant thickness volumes. Check script AAA_CreateVarThicknessVolumes.bat!")
        print("Done!")
    else:
        print("Skipped! - 6_Mesh_Var_Thickness/AAA_Wall_External.stl already exists")
    os.chdir("..")

if AAA_REMOVE_INTERMEDIATE_RESULTS:
    os.chdir("6_Mesh_Var_Thickness")
    os.remove("AAA.msh")
    os.remove("AAA_merged.vtp")
    os.remove("AAA_Wall_Cap.stl")
    os.remove("AAA_ILT_Cap.stl")
    os.chdir("..")

AAA_MESH_DIRS = []
AAA_ABAQUS_DIRS = []
if AAA_ANALYSE_CONST_THICKNESS:
    AAA_MESH_DIRS.append(Path("5_Mesh_Const_Thickness"))
    AAA_ABAQUS_DIRS.append(Path("7_Abaqus_Const_Thickness"))
if AAA_ANALYSE_VAR_THICKNESS:
    AAA_MESH_DIRS.append(Path("6_Mesh_Var_Thickness"))
    AAA_ABAQUS_DIRS.append(Path("8_Abaqus_Var_Thickness"))

AAA_ELEMENT_CONFIG = ""
if AAA_USE_QUADRATIC_ELEMENTS:
    AAA_ELEMENT_CONFIG = "--quad"
if AAA_USE_HYBRID_ELEMENTS:
    AAA_ELEMENT_CONFIG = " ".join([AAA_ELEMENT_CONFIG, "--hybrid"])

print(AAA_ELEMENT_CONFIG)

for dir in AAA_MESH_DIRS:
    print("Meshing Volumes in %s ..." % dir)
    if not Path("./%s/Mesh/Wall.vtk" % dir).exists:
        os.chdir(Path("%s/Mesh" % dir))
        print("Meshing Volumes ...")
        runpy.run_path(Path(os.environ["SCRIPTS_PATH"], "AAA_Mesh.py"))
        os.chdir(Path("../.."))
        print("Done!")
    else:
        print("Skipped! - %s/Mesh/Wall.vtk already exists" % dir)

    to_del = Path("%s/Mesh" % dir).glob("*.geo")
    to_del.append(Path("%s/Mesh" % dir).glob("*.msh"))
    to_del.append(Path("%s/Mesh" % dir).glob("*.pos"))
    to_del.append(Path("%s/Mesh" % dir).glob("*.ply"))

    if AAA_REMOVE_INTERMEDIATE_RESULTS and Path("%s/Mesh/Wall.vtk" % dir).exists():
        to_del.append(Path("%s/Mesh" % dir).glob("*.stl"))

    for file in to_del:
        os.remove(file)

    print("Creating Abaqus input files in %s/Mesh ..." % dir)

    if not Path("%s/Mesh/Wall.inp").exists():
        subprocess.call([os.environ["AAA_GENERATE_ABAQUS_FILE_PATH"],
                         Path("%s/Mesh/Wall.vtk" % dir),
                         Path("%s/Mesh/WallSurface.vtp" % dir), "-o",
                         Path("%s/Mesh/Wall.inp" % dir),
                         AAA_ELEMENT_CONFIG])
        if not Path("%s/Mesh/Wall.inp").exists():
            raise Exception("Error creating %s/Mesh/Wall.inp!" % dir)

        if Path("%s/Mesh/ILT.vtk").exists():
            subprocess.call([os.environ["AAA_GENERATE_ABAQUS_FILE_PATH"],
                             Path("%s/Mesh/ILT.vtk" % dir),
                             Path("%s/Mesh/ILTSurface.vtp" % dir), "-o",
                             Path("%s/Mesh/ILT.inp" % dir),
                             AAA_ELEMENT_CONFIG])
            if not Path("%s/Mesh/ILT.inp").exists():
                raise Exception("Error creating %s/Mesh/ITL.inp!" % dir)
    else:
        print("Skipped! - %s/Mesh/Wall.inp already exists" % dir)

if AAA_ANALYSE_CONST_THICKNESS:
    shutil.copy(Path("5_Mesh_Const_Thickness/Mesh/Wall.inp"),
                Path("7_Abaqus_Const_Thickness"))
    shutil.copy(Path("5_Mesh_Const_Thickness/Mesh/ILT.inp"),
                Path("7_Abaqus_Const_Thickness"))
    shutil.copy(Path("5_Mesh_Const_Thickness/ILTSurface.vtp"),
                Path("7_Abaqus_Const_Thickness"))
if AAA_ANALYSE_VAR_THICKNESS:
    shutil.copy("6_Mesh_Var_Thickness/Mesh/Wall.inp",
                "8_Abaqus_Var_Thickness")
    shutil.copy("6_Mesh_Var_Thickness/Mesh/ILT.inp",
                "8_Abaqus_Var_Thickness")
    shutil.copy("6_Mesh_Var_Thickness/ILTSurface.vtp",
                "8_Abaqus_Var_Thickness")

for dir in AAA_ABAQUS_DIRS:
    print("Abaqus analysis in %s" % dir)
    os.chdir(dir)
    subprocess.call([Path(os.environ["SCRIPTS_PATH"], "AnalyseAllCases.py")])
    os.chdir("..")
    print("Done!")

print("All analysis complete!")
