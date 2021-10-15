# Author: Grand Roman Joldes
# E-mail: grand.joldes@uwa.edu.au

import sys

dir = sys.argv[1]

def terminate(errorcode):
   print("\n")
   exit(errorcode)

def error(msg):
   print msg
   terminate(1)

res = slicer.util.loadVolume(dir+"\\CT_cropped.nrrd")
if (res == False): 
   error("Failed to load "+dir+"\\CT_cropped.nrrd")

res = slicer.util.loadLabelVolume(dir+"\\CT_blood_label.nrrd")
if (res == False): 
  print("Failed to load "+dir+"\\CT_blood_label.nrrd - ILT surface will not be created!")
  print "\nExtracting AAA surface ..."
  res = slicer.util.loadLabelVolume(dir+"\\CT_AAA_label.nrrd")
  if (res == False): 
    error("Failed to load "+dir+"\\CT_AAA_label.nrrd")
  wallNode = getNode("CT_AAA_Label")
  parameters = {}
  parameters["InputVolume"] = wallNode.GetID()
  parameters["Name"] = "CT_wall"
  parameters["Smooth"] = 100
  parameters["FilterType"] = "Laplacian"
  parameters["Decimate"] = 0.1
  outModels = slicer.vtkMRMLModelHierarchyNode()
  slicer.mrmlScene.AddNode( outModels )
  parameters["ModelSceneFile"] = outModels.GetID()
  modelmaker = slicer.modules.modelmaker
  cliNode = slicer.cli.run(modelmaker, None, parameters, wait_for_completion=True)
  status = cliNode.GetStatusString();
  if status != 'Completed':
	error("Failed to run modelmaker!")  
  print "\nSaving results ..."
  surfNode = getNode("CT_wall_1_1")
  res = slicer.util.saveNode(surfNode, dir+"\\CT_wall.vtp")
  if (res == False): 
    error("Failed to save "+dir+"\\CT_wall.vtp")
  terminate(0)

res = slicer.util.loadLabelVolume(dir+"\\CT_AAA_label.nrrd")
if (res == False): 
   error("Failed to load "+dir+"\\CT_AAA_label.nrrd")

print "Running ErodeEffect ..."
moduleSelector = slicer.util.mainWindow().moduleSelector()
moduleSelector.selectModule('Editor')
slicer.modules.EditorWidget.toolsBox.selectEffect('ErodeEffect')
slicer.modules.EditorWidget.toolsBox.currentOption.onApply()
slicer.modules.EditorWidget.toolsBox.currentOption.onApply()
slicer.modules.EditorWidget.toolsBox.currentOption.onApply()
slicer.modules.EditorWidget.toolsBox.currentOption.onApply()

print "\nRunning MaskScalarVolume ..."
bloodNode = getNode('CT_blood_label')
erodedAAANode = getNode('CT_AAA_label')
parameters = {}
parameters["InputVolume"] = bloodNode.GetID()
parameters["MaskVolume"] = erodedAAANode.GetID()
outModel = slicer.vtkMRMLScalarVolumeNode()
outModel.SetName("CT_blood_masked_label")
outModel.SetAttribute("LabelMap", "1")
slicer.mrmlScene.AddNode( outModel )
parameters["OutputVolume"] = outModel.GetID()
masker = slicer.modules.maskscalarvolume
cliNode = slicer.cli.run(masker, None, parameters, wait_for_completion=True)
status = cliNode.GetStatusString();
if status != 'Completed':
	error("Failed to run maskscalarvolume!")

print "\nRunning SubtractScalarVolumes ..."
res = slicer.util.loadLabelVolume(dir+"\\CT_AAA_label.nrrd")
if (res == False): 
     error("Failed to load "+dir+"\\CT_AAA_label.nrrd")
AAANode = getNode('CT_AAA_label_1')
bloodNode = getNode('CT_blood_masked_label')
parameters = {}
parameters["inputVolume1"] = AAANode.GetID()
parameters["inputVolume2"] = bloodNode.GetID()
outModel = slicer.vtkMRMLScalarVolumeNode()
outModel.SetName("CT_Wall_Label")
outModel.SetAttribute("LabelMap", "1")
slicer.mrmlScene.AddNode( outModel )
parameters["outputVolume"] = outModel.GetID()
parameters["order"] = 0
subtracter = slicer.modules.subtractscalarvolumes
cliNode = slicer.cli.run(subtracter, None, parameters,  wait_for_completion=True)
status = cliNode.GetStatusString();
if status != 'Completed':
	error("Failed to run subtractscalarvolumes!")
  
print "\nExtracting AAA wall surface ..."
wallNode = getNode("CT_Wall_Label")
parameters = {}
parameters["InputVolume"] = wallNode.GetID()
parameters["Name"] = "CT_wall"
parameters["Smooth"] = 100
parameters["FilterType"] = "Laplacian"
parameters["Decimate"] = 0.1
outModels = slicer.vtkMRMLModelHierarchyNode()
slicer.mrmlScene.AddNode( outModels )
parameters["ModelSceneFile"] = outModels.GetID()
modelmaker = slicer.modules.modelmaker
cliNode = slicer.cli.run(modelmaker, None, parameters, wait_for_completion=True)
status = cliNode.GetStatusString();
if status != 'Completed':
	error("Failed to run modelmaker!")  
  
print "\nSaving results ..."
surfNode = getNode("CT_wall_1_jake")
res = slicer.util.saveNode(surfNode, dir+"\\CT_wall.vtp")
if (res == False): 
    error("Failed to save "+dir+"\\CT_wall.vtp")
wallNode = getNode("CT_Wall_Label")
res = slicer.util.saveNode(wallNode, dir+"\\CT_Wall_label.nrrd")
if (res == False): 
    error("Failed to save "+dir+"\\CT_Wall_label.nrrd")
print "\nDone!"
terminate(0)