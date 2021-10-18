# BioPARR

BioPARR (Biomechanics based Prediction of Aneurysm Rupture Risk) is a software system that facilitates the analysis of
abdominal aortic aneurisms (AAA) and evaluation of rupture risk using a finite element analysis based approach.

Except for semi-automatic segmentation of the AAA and intraluminal thrombus (ILT) from medical images,
the entire analysis is performed automatically. The system is modular and easily expandable, allows
the extraction of information from images of different modalities (CT, MRI) and the simulation of
different modelling scenarios.

### Installation instructions

Current version includes precompiled binaries which run on Windows.
The source code is included.
Compiling the software for other operating systems should be possible, as all components are portable (except the .bat files, which need to be re-written).

#### Dependencies

* 3D Slicer (version 4.4.0)
* Paraview (tested with version 4.3.1)
* Abaqus (tested with version 6.14)

#### Deployment instructions

Basic instructions on the BioPARR workflow is available [here](https://github.com/ISML-UWA/BioPARR/blob/master/Workflow_AAA_Analysis.pdf).

#### Tutorials

Tutorials and more information are available at the [BioPARR website](https://bioparr.mech.uwa.edu.au/).

### Author
* Grand Joldes: <grand.joldes@uwa.edu.au>
