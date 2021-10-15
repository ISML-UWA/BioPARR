# Author: Grand Roman Joldes
# E-mail: grand.joldes@uwa.edu.au

import sys
import subprocess

dir = sys.argv[1]
debug = int(sys.argv[2])

def terminate(errorcode):
   if (debug != 1):
      exit(errorcode)

print("Registering label maps ... \n")
args = ["BRAINSFit", "--fixedVolume", dir+"\\..\\1_Segmentation_CT\\CT_AAA_label.nrrd",
"--movingVolume", dir+"\\MRI_AAA_label.nrrd", "--linearTransform", dir+"\\registerMRI2CT.h5",
"--initializeTransformMode", "useGeometryAlign", "--samplingPercentage", "0.1", 
"--useRigid", "--costMetric", "MIH"]

if (debug == 1):
  process = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  output, error = process.communicate()
  print "Output:\n"+output
  print "Error:\n"+error
else:
  process = subprocess.Popen(args)
  process.wait()

if (process.returncode != 0):
   print("\nError registering label maps!")
   terminate(process.returncode)

print("Registering MRI to CT ... \n")
args = ["BRAINSResample", "--inputVolume", dir+"\\MRI_cropped.nrrd", 
"--outputVolume", dir+"\\MRI_aligned_to_CT.nrrd",
"--referenceVolume", dir+"\\..\\1_Segmentation_CT\\CT_AAA_label.nrrd",
"--warpTransform", dir+"\\registerMRI2CT.h5"]

if (debug == 1):
  process = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  output, error = process.communicate()
  print "Output:\n"+output
  print "Error:\n"+error
else:
  process = subprocess.Popen(args)
  process.wait()

if (process.returncode != 0):
   print("\nError registering MRI to CT!")

terminate(process.returncode)