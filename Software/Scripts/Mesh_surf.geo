/*********************************************************************
 *
 *  Remesh a 3D surface
 *
 * Author: Grand Roman Joldes
 * E-mail: grand.joldes@uwa.edu.au
 *
 *********************************************************************/

Include "Surf.geo";

// Create the topology of the discrete model
CreateTopology;

// Mark surfaces for remeshing
ss[] = Surface "*";
s = news;
Compound Surface(s) = ss[0];

Physical Surface("External") = {s};

Mesh.RemeshAlgorithm = 2; // (0) no split (1) automatic (2) automatic only with metis
Mesh.RemeshParametrization = 7; // 0=harmonic_circle, 1=conformal_spectral, 2=rbf, 3=harmonic_plane, 4=convex_circle, 5=convex_plane, 6=harmonic square, 7=conformal_fe
Geometry.HideCompounds = 0; // don't hide the compound entities
Mesh.Algorithm = 5; // 5 - Delauney, 6 - Frontal

// Don't extend the elements sizes from the boundary inside the domain
Mesh.CharacteristicLengthExtendFromBoundary = 0;
Mesh.CharacteristicLengthFromCurvature = 0;
Mesh.CharacteristicLengthFromPoints = 0;
Mesh.RandomFactor = 1e-10;

Mesh.Smoothing = 10;

Field[1] = Structured;
Field[1].FileName = "..\size.bin";
Field[1].TextFormat = 0;

Background Field = 1;
