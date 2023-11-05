
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

int edgePixSkp = 2;
int edgePixDist = 3;
int Xsq_wdth = 2; /// Width of block to test. times 2 then add 1
int Ysq_wdth = 0; /// Height of block to test. times 2 then add 1
int threashold = 20; /// Edge detect threashold. 0-255 Larger numbers == less sensitive.
int MaxCenterDiff = 20; /// Max center pixel test difference. 0-255 Smaller numbers == closer match.
int MaxColorDiff = 30; /// Max pixel test difference per block. 0-255 Smaller numbers == closer match.
int maxTotalDiff = 40; /// Max overall difference. 0-255 Smaller numbers == closer match.
int minBKcontrast = 2; /// Minimum contrast between center pixel and surrounding pixels. 0-255 Smaller numbers == closer match.
int MinCtstCount = 1; /// Number of pixels that need minimum contrast to center pixel.
float MinCullDist = 0.05f; /// Minimum Cull Distance in meters to count towards MinCullCount.
int MinCullCount = 15; /// Minimum number of nearby points to not cull.
int TestGrid = 5; /// Size of Culling test grid. times 2 then add 1
int CullingPasses = 0; /// Number of passes to make for distance culling. Set to 0 for AUTO.
int MaxThreads = 8;

///Distance is in mm.
///Angles are in Radians.
float Cam_Dist = 50.8f; //50.8 :: 120.65mm + 114.3 ??= 234.95 :: other images 228.6mm
float min_Dist = 500.0f; //0.20 meters: adjacent angle.
float max_Dist = 6000.0f; //2 meters: opposite angle.
float X_FOV = 0.0f;
float Y_FOV = 0.0f;
float lens_foc = 18.0f; //test parameter. NIKON D3300 with 18-140mm lens.
float X_Size = 23.5f; //test parameter. NIKON D3300 with 18-140mm lens.
float Y_Size = 15.6f; //test parameter. NIKON D3300 with 18-140mm lens.

unsigned int width, height, Rwidth, Rheight;
unsigned char * edgeOutLEFT;
unsigned char * edgeOutRIGHT;
unsigned char * reduxOutLEFT;
unsigned char * reduxOutRIGHT;
unsigned char * reduxMatchLEFT;
unsigned char * reduxMatchRIGHT;
unsigned char * pixelTMP;
float * Sets;
float * X_Angle;
float * Y_Angle;
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
unsigned int cord(unsigned int, unsigned int);
unsigned int COLOR_cord(unsigned int, unsigned int, unsigned int);
float DegToRad(float);
float RadToDeg(float);
bool isEdge(int, int);
bool reduxMatch(int, int, int);

//class C_bMatch {
//  public:
//  int pixScore;
//  int matchWith;
  /** Memory constructor **/
//  C_bMatch(void) {
//    pixScore = -1;
//    matchWith = 0;
//  }
//};

class C_Points {
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
  public:
      void slpm(int, unsigned int, unsigned int, int, int, int, int, int, C_Points *, int);
};

class C_pTable {
  public:
    unsigned int * PTable;
  /** Memory constructor **/
  C_pTable(void) {
    PTable = new unsigned int[2];
    PTable[0] = 0;
    PTable[1] = 0;
  }
  ~C_pTable(void) {
    delete[] PTable;
  }
};

class C_Chroma {
  public:
      unsigned int * chnl;
  /** Memory constructor **/
  C_Chroma(void) {
    chnl = new unsigned int[3];
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
