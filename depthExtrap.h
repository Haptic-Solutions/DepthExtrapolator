
#ifndef depthExtrap_H
#define depthExtrap_H

#define R 0
#define G 1
#define B 2
#define X 0
#define Y 1
#define Z 2
#define CordX 0
#define CordY 1
#define scan_debug 749

bool spherical_Lens = 0;

int edgePixSkp = 3;
int Xsq_wdth = 3;
int Ysq_wdth = 3;
int threashold = 20;      ///Larger numbers == less sensitive.
int MaxColorDiff = 18;    ///Smaller numbers == closer match.

///Distance is in mm.
///Angles are in Radians.
double Cam_Dist = 50.8;  //50.8 :: 120.65mm + 114.3 ??= 234.95 :: other images 228.6mm
double min_Dist = 200;     //0.20 meters: adjacent angle.
double max_Dist = 6000;    //2 meters: opposite angle.
double X_FOV = 0;
double Y_FOV = 0;
double lens_foc = 18;      //test parameter. NIKON D3300 with 18-140mm lens.
double X_Size = 23.5;      //test parameter. NIKON D3300 with 18-140mm lens.
double Y_Size = 15.6;      //test parameter. NIKON D3300 with 18-140mm lens.

unsigned int width, height;
unsigned char * edgeOutLEFT;
unsigned char * edgeOutRIGHT;
unsigned char * reduxOutLEFT;
unsigned char * reduxOutRIGHT;
unsigned char * reduxMatchLEFT;
unsigned char * reduxMatchRIGHT;
unsigned char * pixelTMP;
double * Sets;
double * X_Angle;
double * Y_Angle;
double dimScale = 0;

unsigned int cord(unsigned int, unsigned int);
double DegToRad(double);
double RadToDeg(double);
void pngmake(int, int, int, int, int);


class C_bMatch{
public:
    int pixScore;
    int matchWith;
    /** Memory constructor **/
    C_bMatch(void){
        pixScore = -1;
        matchWith = 0;
    }
};

class C_chSplit{
public:
    unsigned int * chnls;
    /** Memory constructor **/
    C_chSplit(void){
        chnls = new unsigned int [3];
        chnls[0]=0;
        chnls[1]=0;
        chnls[2]=0;
    }
    ~C_chSplit(void){
        delete[] chnls;
    }
};

int gridComp(int, int, int, C_chSplit*, C_chSplit*);

class C_pTable{
public:
    unsigned int * PTable;
    /** Memory constructor **/
    C_pTable(void){
        PTable = new unsigned int [2];
        PTable[0]=0;
        PTable[1]=0;
    }
    ~C_pTable(void){
        delete[] PTable;
    }
};

class C_Points{
public:
    double * Cord;
    /** Memory constructor **/
    C_Points(void){
        Cord = new double [3];
        Cord[0]=0;
        Cord[1]=0;
        Cord[2]=0;
    }
    ~C_Points(void){
        delete[] Cord;
    }
};

void calcPoint(int, int, int, C_Points*);

const char *PLYheader_Start[]{
"ply\n"
"format ascii 1.0\n"
"element vertex "
};
const char *PLYheader_End[]{
"property double x\n"
"property double y\n"
"property double z\n"
"property uchar red\n"
"property uchar green\n"
"property uchar blue\n"
"end_header\n"
};


#endif // PICTOVECTWAVE_H
