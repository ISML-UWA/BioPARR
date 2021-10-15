#ifndef _AAA_DEFINES_
#define _AAA_DEFINES_

#define THICKNESS_LOCATION "ThickPos"
#define THICKNESS_NAME "Thickness"
#define THICKNESS_DEFORMED_NAME "DeformedThickness"
#define SURF_ID_NAME "SurfID"

#define DISP_NAME "U"
#define MEMBRANE_STRESS_NAME "MembraneS"

#define GAUSS_CURV_NAME "Gauss_Curvature"
#define MEAN_CURV_NAME "Mean_Curvature"
#define MIN_CURV_NAME "Min_Curvature"
#define MAX_CURV_NAME "Max_Curvature"

#define NUM_TRIES 10  // how many times to try remove each cap
#define LENGTH_PER_CAP_STEP 200  // ratio of length to cap removal increment


#define EXT_WALL_SURF_ID  5
#define INT_WALL_SURF_ID  4
#define INT_ILT_SURF_ID  3
#define WALL_CAPS_SURF_ID  2
#define ILT_CAPS_SURF_ID  1

#define EXT_WALL_SURF_NAME "AAA_Wall_External"
#define INT_WALL_SURF_NAME "AAA_Wall_Internal"
#define INT_ILT_SURF_NAME "AAA_ILT_Internal"
#define ILT_CAP_NAME "AAA_ILT_Cap"
#define WALL_CAP_NAME "AAA_Wall_Cap"









#endif
