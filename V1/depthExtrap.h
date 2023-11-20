/*
Copyright (c) <2023> <Haptic Solutions>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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

bool spherical_Lens = 0;
int scan_debug = 0;

int edgePixSkp = 4;
int Xsq_wdth = 5;           /// Width of block to test. times 2 then add 1
int Ysq_wdth = 5;           /// Height of block to test. times 2 then add 1
int threashold = 15;        /// Edge detect threashold. 0-255 Larger numbers == less sensitive.
int MaxCenterDiff = 20;     /// Max center pixel test difference. 0-255 Smaller numbers == closer match.
int MaxColorDiff = 30;      /// Max pixel test difference per block. 0-255 Smaller numbers == closer match.
int maxTotalDiff = 40;      /// Max overall difference. 0-255 Smaller numbers == closer match.
int minBKcontrast = 2;      /// Minimum contrast between center pixel and surrounding pixels. 0-255 Smaller numbers == closer match.
int MinCtstCount = 1;       /// Number of pixels that need minimum contrast to center pixel.
float MinCullDist = 0.05;   /// Minimum Cull Distance in meters to count towards MinCullCount.
int MinCullCount = 15;      /// Minimum number of nearby points to not cull.
int TestGrid = 5;           /// Size of Culling test grid. times 2 then add 1
int CullingPasses = 0;     /// Number of passes to make for distance culling. Set to 0 for AUTO.

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

unsigned int width, height, Rwidth, Rheight;
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
int MaxDiffCenters = 0;
int MaxDiffBlocks = 0;
int LowContrastBlocks = 0;

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

class C_Chroma{
public:
    unsigned int * chnl;
    /** Memory constructor **/
    C_Chroma(void){
        chnl = new unsigned int [3];
        chnl[0]=0;
        chnl[1]=0;
        chnl[2]=0;
    }
    ~C_Chroma(void){
        delete[] chnl;
    }
};

class C_Points{
public:
    double * Cord;
    int PassNum;
    /** Memory constructor **/
    C_Points(void){
        Cord = new double [3];
        Cord[0]=0;
        Cord[1]=0;
        Cord[2]=0;
        PassNum=0;
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
