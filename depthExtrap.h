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
#define PI_aprox 3.14159f

bool spherical_Lens = 0;

int edgePixSkp = 1;
int edgePixDist = 1;
//int edgePixSkp = 4;
//int edgePixDist = 4;
int Xsq_wdth = 4; /// Width of block to test. times 2 then add 1
int Ysq_wdth = 0; /// Height of block to test. times 2 then add 1
int threashold = 5; /// Edge detect threashold. 0-255 Larger numbers == less sensitive.
//int threashold = 20;
int MaxCenterDiff = 20; /// Max center pixel test difference. 0-255 Smaller numbers == closer match.
int MaxColorDiff = 30; /// Max pixel test difference per block. 0-255 Smaller numbers == closer match.
int maxTotalDiff = 40; /// Max overall difference. 0-255 Smaller numbers == closer match.
int minBKcontrast = 2; /// Minimum contrast between center pixel and surrounding pixels. 0-255 Smaller numbers == closer match.
int MinCtstCount = 1; /// Number of pixels that need minimum contrast to center pixel.
float MinCullDist = 0.05f; /// Minimum Cull Distance in meters to count towards MinCullCount.
int MinCullCount = 15; /// Minimum number of nearby points to not cull.
int TestGrid = 5; /// Size of Culling test grid. times 2 then add 1
int CullingPasses = 0; /// Number of passes to make for distance culling. Set to 0 for AUTO.

///Distance is in mm.
///Angles are in Radians for all calculations.
float Cam_Dist = 50.8f; //50.8 :: 120.65mm + 114.3 ??= 234.95 :: other images 228.6mm
float min_Dist = 100.0f; //0.50 meters: adjacent angle.
float max_Dist = 6000.0f; //6 meters: opposite angle.

///OC9281
//float lens_foc = 2.8f; //test parameter. NIKON D3300 with 18-140mm lens.
//float X_Size = 3.896f; //test parameter. NIKON D3300 with 18-140mm lens, X = 23.5.
//float Y_Size = 2.453f; //test parameter. NIKON D3300 with 18-140mm lens Y = 15.6.

///OC7251
float lens_foc = 2.8f;
float X_Size = 1.968f;
float Y_Size = 1.488f;

unsigned int width, height, Rwidth, Rheight;
float * X_Angle;
float * Y_Angle;
float X_FOV = 0.0f;         /// Leave at 0
float Y_FOV = 0.0f;         /// Leave at 0
float dimScale = 0.0f;
int MaxDiffCenters = 0;
int MaxDiffBlocks = 0;
int LowContrastBlocks = 0;
int Pix_Diff = 0;
int match_used = 0;
int better_matches = 0;
int matches_discarded = 0;
int match_count = 0;
int T_Edge_Cnt = 0;
int final_multi_point = 0;

std::vector < unsigned char > Limage;
std::vector < unsigned char > Rimage;

void lineThread();
float DegToRad(float);
float RadToDeg(float);
int ST_cord(int, int);
int ST_COLOR_cord(int, int, int);

struct C_Points {
  public:
  float * Cord;
  int PassNum;
  /** Memory constructor **/
  C_Points(void) {
    Cord = new float[3];
    Cord[0] = 0;
    Cord[1] = 0;
    Cord[2] = 0;
    PassNum = 0;
  }
  ~C_Points(void) {
    delete[] Cord;
  }
};

class C_threadCalc {
  private:
      int gridComp(int, int, int);
      void calcPoint(int, int, int, C_Points * );
      bool isEdge(int, int);
      bool reduxMatch(int, int, int);
      int cord(int, int);
      int COLOR_cord(int, int, int);
  public:
      void slpm(int, int, int, int, int, int, int, int, C_Points *, int);
};


struct C_Chroma {
  public:
      int * chnl;
  /** Memory constructor **/
  C_Chroma(void) {
    chnl = new int[3];
    chnl[0] = 0;
    chnl[1] = 0;
    chnl[2] = 0;
  }
  ~C_Chroma(void) {
    delete[] chnl;
  }
};


const char * PLYheader_Start[] {
  "ply\n"
  "format ascii 1.0\n"
  "element vertex "
};
const char * PLYheader_End[] {
  "property float x\n"
  "property float y\n"
  "property float z\n"
  "property uchar red\n"
  "property uchar green\n"
  "property uchar blue\n"
  "end_header\n"
};

#endif
