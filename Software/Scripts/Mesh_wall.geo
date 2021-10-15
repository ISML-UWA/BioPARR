/*********************************************************************
 *
 *  Creates a 3D mesh of the wall between the 2 surfaces
 *
 * Author: Grand Roman Joldes
 * E-mail: grand.joldes@uwa.edu.au
 *
 *********************************************************************/


Include "Surf.geo";

Delete Physicals;

// Create the topology of the discrete model
CreateTopology;

ss[] = Surface "*";

// Extract boundaries and create caps
b1[] = Boundary{ Surface{ss[0]}; };
b2[] = Boundary{ Surface{ss[1]}; };

N1 = #b1[];
Printf("Surface 1 has %g boundaries.", N1);
N2 = #b2[];
Printf("Surface 2 has %g boundaries.", N2);


Macro SeparateCaps
   n = #list~{i1}[];
   For k In {0:n-1}
	Printf("list_%g[%g] = %g", i1, k, list~{i1}[k]);
   EndFor
   If (n < 4)
   	l = newl;
   	Line Loop(l) = list~{i1}[];
  	Plane Surface(news) = l;
   EndIf
   If (n == 4)
	P0 = Point{list~{i1}[0]};
	P1 = Point{list~{i1}[1]};
	P2 = Point{list~{i1}[2]};
	P3 = Point{list~{i1}[3]};
	
	dP = P0[];
	dP[{0:2}] -= P2[{0:2}];
	dist02 = Sqrt(dP[0]^2+dP[1]^2+dP[2]^2);
	dP = P0[];
	dP[{0:2}] -= P3[{0:2}];
	dist03 = Sqrt(dP[0]^2+dP[1]^2+dP[2]^2);
	dP = P1[];
	dP[{0:2}] -= P2[{0:2}];
	dist12 = Sqrt(dP[0]^2+dP[1]^2+dP[2]^2);
	dP = P1[];
	dP[{0:2}] -= P3[{0:2}];
	dist13 = Sqrt(dP[0]^2+dP[1]^2+dP[2]^2);
	If (dist02 + dist13 < dist03 + dist12)
	   	l = newl;
   		Line Loop(l) = list~{i1}[{0, 2}];
  		Plane Surface(news) = l;
		l = newl;
   		Line Loop(l) = list~{i1}[{1, 3}];
  		Plane Surface(news) = l;
	EndIf
	If (dist02 + dist13 >= dist03 + dist12)
		l = newl;
   		Line Loop(l) = list~{i1}[{0, 3}];
  		Plane Surface(news) = l;
		l = newl;
   		Line Loop(l) = list~{i1}[{1, 2}];
  		Plane Surface(news) = l;
	EndIf
   EndIf
   If (n>4)
	// Number of boundary lines is > 4 - error
   	Error("The number of boundary lines in the same OZ plane is greater than 4!");
   	Exit;
   EndIf
Return

// Separate all points with the same z level in separate lists
For i1 In {0:(N1-1)}
    If (i1 < N1)
        list~{i1} = {};
	list~{i1} += {b1[i1]};
	P1 = Point{b1[i1]};
	For i3 In {(i1+1):(N1-1)}
	    If (i3 < N1)
		Printf("Checking boundary S1.%g against S1.%g.", i1, i3);
		P2 = Point{b1[i3]};
		If (Fabs(P1[2] - P2[2]) < 1)
			list~{i1} += {b1[i3]};
			Printf("Match!");
			bnd = b1[i3];
			b1 -= {bnd};
			N1 = #b1[];
			i3 = i3-1;
		EndIf
	    EndIf
	EndFor

	For i2 In {0:(N2-1)}
	   If (i2 < N2)
		Printf("Checking boundary S1.%g against S2.%g.", i1, i2);
		P2 = Point{b2[i2]};
		If (Fabs(P1[2] - P2[2]) < 1)
			list~{i1} += {b2[i2]};
			Printf("Match!");
			bnd = b2[i2];
			b2 -= {bnd};
			N2 = #b2[];
			i2 = i2-1;
		EndIf
   	    EndIf
	EndFor
	Call SeparateCaps;
   EndIf
EndFor

Surface Loop(1) = Surface "*";
Volume(1) = {1};
Physical Volume(1) = 1;

Mesh.RemeshAlgorithm = 1; // (0) no split (1) automatic (2) automatic only with metis
Mesh.RemeshParametrization = 1; // (0) harmonic (1) conformal spectral (7) conformal finite element
Geometry.HideCompounds = 0; // don't hide the compound entities
Mesh.Algorithm = 5; // 5 - Delauney, 6 - Frontal
Mesh.Algorithm3D = 1; // 1=Delaunay, 4=Frontal, 5=Frontal Delaunay, 6=Frontal Hex, 7=MMG3D, 9=R-tree

// Don't extend the elements sizes from the boundary inside the domain
Mesh.CharacteristicLengthExtendFromBoundary = 0;
Mesh.CharacteristicLengthFromCurvature = 0;
Mesh.CharacteristicLengthFromPoints = 0;
Mesh.Smoothing = 0;


Field[1] = Structured;
Field[1].FileName = "..\size.bin";
Field[1].TextFormat = 0;

Background Field = 1;