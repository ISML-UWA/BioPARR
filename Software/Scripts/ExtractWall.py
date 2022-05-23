# Author: Grand Roman Joldes
# E-mail: grand.joldes@uwa.edu.au

import sys
from pathlib import Path

dir = sys.argv[1]


def terminate(errorcode):
    print("\n")
    exit(errorcode)


def error(msg):
    print(msg)
    terminate(1)


path_CT_cropped = str(Path(dir, "CT_cropped.nrrd"))
path_CT_blood = str(Path(dir, "CT_blood_label.nrrd"))
path_CT_AAA = str(Path(dir, "CT_AAA_label.nrrd"))
path_CT_wall = str(Path(dir, "CT_wall.vtp").resolve())
path_CT_wall_label = str(Path(dir, "CT_Wall_label.nrrd").resolve())

res = slicer.util.loadVolume(str(path_CT_cropped))
if res is False:
    error("Failed to load " + path_CT_cropped)

res = slicer.util.loadLabelVolume(path_CT_blood)
if res is False:
    print("Failed to load " + path_CT_blood
          + "- ILT surface will not be created!")
    print("\nExtracting AAA surface ...")
    res = slicer.util.loadLabelVolume(path_CT_AAA)
    if res is False:
        error("Failed to load " + path_CT_AAA)
    wallNode = getNode("CT_AAA_Label")
    parameters = {}
    parameters["InputVolume"] = wallNode.GetID()
    parameters["Name"] = "CT_wall"
    parameters["Smooth"] = 100
    parameters["FilterType"] = "Laplacian"
    parameters["Decimate"] = 0.1
    outModels = slicer.vtkMRMLModelHierarchyNode()
    slicer.mrmlScene.AddNode(outModels)
    parameters["ModelSceneFile"] = outModels.GetID()
    modelmaker = slicer.modules.modelmaker
    cliNode = slicer.cli.run(modelmaker, None, parameters,
                             wait_for_completion=True)
    status = cliNode.GetStatusString()
    if status != 'Completed':
        error("Failed to run modelmaker!")
    print("\nSaving results ...")
    surfNode = getNode("CT_wall_1_1")
    res = slicer.util.saveNode(surfNode, path_CT_wall)
    if res is False:
        error("Failed to save " + path_CT_wall)
    terminate(0)

res = slicer.util.loadLabelVolume(path_CT_AAA)
if res is False:
    error("Failed to load " + path_CT_AAA)

print("Running ErodeEffect ...")
moduleSelector = slicer.util.mainWindow().moduleSelector()
moduleSelector.selectModule('Editor')
slicer.modules.EditorWidget.toolsBox.selectEffect('ErodeEffect')
slicer.modules.EditorWidget.toolsBox.currentOption.onApply()
slicer.modules.EditorWidget.toolsBox.currentOption.onApply()
slicer.modules.EditorWidget.toolsBox.currentOption.onApply()
slicer.modules.EditorWidget.toolsBox.currentOption.onApply()

print("\nRunning MaskScalarVolume ...")
bloodNode = getNode('CT_blood_label')
erodedAAANode = getNode('CT_AAA_label')
parameters = {}
parameters["InputVolume"] = bloodNode.GetID()
parameters["MaskVolume"] = erodedAAANode.GetID()
outModel = slicer.vtkMRMLScalarVolumeNode()
outModel.SetName("CT_blood_masked_label")
outModel.SetAttribute("LabelMap", "1")
slicer.mrmlScene.AddNode(outModel)
parameters["OutputVolume"] = outModel.GetID()
masker = slicer.modules.maskscalarvolume
cliNode = slicer.cli.run(masker, None, parameters, wait_for_completion=True)
status = cliNode.GetStatusString()
if status != 'Completed':
    error("Failed to run maskscalarvolume!")

print("\nRunning SubtractScalarVolumes ...")
res = slicer.util.loadLabelVolume(path_CT_AAA)
if res is False:
    error("Failed to load " + path_CT_AAA)
AAANode = getNode('CT_AAA_label_1')
bloodNode = getNode('CT_blood_masked_label')
parameters = {}
parameters["inputVolume1"] = AAANode.GetID()
parameters["inputVolume2"] = bloodNode.GetID()
outModel = slicer.vtkMRMLScalarVolumeNode()
outModel.SetName("CT_Wall_Label")
outModel.SetAttribute("LabelMap", "1")
slicer.mrmlScene.AddNode(outModel)
parameters["outputVolume"] = outModel.GetID()
parameters["order"] = 0
subtracter = slicer.modules.subtractscalarvolumes
cliNode = slicer.cli.run(subtracter, None, parameters,
                         wait_for_completion=True)
status = cliNode.GetStatusString()
if status != 'Completed':
	error("Failed to run subtractscalarvolumes!")

print("\nExtracting AAA wall surface ...")
wallNode = getNode("CT_Wall_Label")
parameters = {}
parameters["InputVolume"] = wallNode.GetID()
parameters["Name"] = "CT_wall"
parameters["Smooth"] = 100
parameters["FilterType"] = "Laplacian"
parameters["Decimate"] = 0.1
outModels = slicer.vtkMRMLModelHierarchyNode()
slicer.mrmlScene.AddNode(outModels)
parameters["ModelSceneFile"] = outModels.GetID()
modelmaker = slicer.modules.modelmaker
cliNode = slicer.cli.run(modelmaker, None, parameters,
                         wait_for_completion=True)
status = cliNode.GetStatusString()
if status != 'Completed':
	error("Failed to run modelmaker!")

print("\nSaving results ...")
surfNode = getNode("CT_wall_1_*")
print(path_CT_wall)
res = slicer.util.saveNode(surfNode, path_CT_wall)
if res is False:
    error("Failed to save " + path_CT_wall)

wallNode = getNode("CT_Wall_Label")
res = slicer.util.saveNode(wallNode, path_CT_wall_label)
if res is False:
    error("Failed to save " + path_CT_wall_label)

print("\nDone!")
terminate(0)
