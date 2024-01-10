
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
#define PI_aprox 3.1415926535

bool spherical_Lens = 0;

int edgePixSkp = 2;     ///Number of pixels to skip after an edge has been found.
int edgePixDist = 5;    ///Distance to compare pixels for edges.
int Xsq_wdth = 1; /// Width of block to test. times 2 then add 1
int Ysq_wdth = 1; /// Height of block to test. times 2 then add 1
int threashold = 20; /// Edge detect threashold. 0-255 Larger numbers == less sensitive.
int MaxCenterDiff = 25; /// Max center pixel test difference. 0-255 Smaller numbers == closer match.
int MaxColorDiff = 30; /// Max pixel test difference per block. 0-255 Smaller numbers == closer match.
int maxTotalDiff = 40; /// Max overall difference. 0-255 Smaller numbers == closer match.
int minBKcontrast = 2; /// Minimum contrast between center pixel and surrounding pixels. 0-255 Smaller numbers == closer match.
int MinCtstCount = 1; /// Number of pixels that need minimum contrast to center pixel.
float MinCullDist = 0.05; /// Minimum Cull Distance in meters to count towards MinCullCount.
int MinCullCount = 15; /// Minimum number of nearby points to not cull.
int TestGrid = 5;       /// Size of Culling test grid. times 2 then add 1
int CullingPasses = 0; /// Number of passes to make for distance culling. Set to 0 for AUTO.
int MaxThreads = 8;  ///NOT IMPLEMENTED
bool verbose = false;
bool ignorVertFile = false;
bool exComp = true;
int G_RGBcompensation[3]={0,0,0};

///Distance is in mm.
///Angles are in Radians.
int Vert_Pix_Align = 0;      //Vertical Pixel Alignment. How many pixels the right image is shifted up/down. Positive is UP
int Vert_Pix_Test = 20;       //+- value of vertical alignment to test.
bool Auto_Vert_Align = true;
double Cam_Dist = 50.8; //50.8 :: 120.65mm + 114.3 ??= 234.95 :: other images 228.6mm
double min_Dist = 500; //0.20 meters: adjacent angle.
double max_Dist = 6000; //2 meters: opposite angle.
double X_FOV = 0;
double Y_FOV = 0;
double lens_foc = 18; //test parameter. NIKON D3300 with 18-140mm lens.
double X_Size = 23.5; //23.5test parameter. NIKON D3300 with 18-140mm lens.
double Y_Size = 15.6; //15.6 test parameter. NIKON D3300 with 18-140mm lens.

unsigned int G_width, G_height, G_Rwidth, G_Rheight;
unsigned char * G_edgeOutLEFT;
unsigned char * G_edgeOutRIGHT;
unsigned char * G_reduxOutLEFT;
unsigned char * G_reduxOutRIGHT;
unsigned char * G_reduxMatchLEFT;
unsigned char * G_reduxMatchRIGHT;
unsigned char * G_pixelTMP;
double * G_Sets;
double * G_X_Angle;
double * G_Y_Angle;
double G_dimScale = 0;
int G_MaxDiffCenters = 0;
int G_MaxDiffBlocks = 0;
int G_LowContrastBlocks = 0;
int G_Pix_Diff = 0;
int G_match_used = 0;
int G_better_matches = 0;
int G_matches_discarded = 0;
int G_match_count = 0;
int G_T_Edge_Cnt = 0;
int G_final_multi_point = 0;
const char* Lfilename;
const char* Rfilename;
const char* Ofilename;

std::vector < unsigned char > G_Limage;
std::vector < unsigned char > G_Rimage;

int G_GetArgs(int, char **);
void G_lineThread();
unsigned int G_cordST(unsigned int, unsigned int);
unsigned int G_COLOR_cordST(unsigned int, unsigned int, unsigned int);
double G_DegToRad(double);
double G_RadToDeg(double);

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
  double * Cord;
  int PassNum;
  /** Memory constructor **/
  C_Points(void) {
    Cord = new double[3];
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
      int gridComp(int, int, int, int);
      void calcPoint(int, int, int, C_Points * );
      bool isEdge(int, int);
      bool reduxMatch(int, int, int, int);
      unsigned int cord(unsigned int, unsigned int);
      unsigned int COLOR_cord(unsigned int, unsigned int, unsigned int);
      int* pixScore;
      int* matchWith;
      bool* PixSkip;
  public:
      void slpm(int, unsigned int, unsigned int, int, int, int, int, int, int, C_Points *, int);
  ~C_threadCalc(void){
    delete[]  pixScore;
    delete[] matchWith;
    delete[] PixSkip;
  }
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
  "property double x\n"
  "property double y\n"
  "property double z\n"
  "property uchar red\n"
  "property uchar green\n"
  "property uchar blue\n"
  "end_header\n"
};


const char *helpText[]{
"\nThis program is provided without any warranty.\n"
"Copyright: Haptic Solutions, January 2024 \n"
"\n"
"-h :: This Help Text.\n"
"\nInput Options: \n"
"-L :: Left Image Input Filename. Default: './left.png'\n"
"-R :: Right Image Input Filename. Default: './right.png'\n"
"-E :: Disable auto exposure compensation. Default: Enabled\n"
"\nCamera Parameters: \n"
"-I :: IPD distance of cameras. Default: 50.8mm or 2 Inches \n"
"-F :: Focal length in mm of camera lenses. Default: 18mm\n"
"-x :: Image sensor horizontal size in mm. Default: 23.5mm\n"
"-y :: Image sensor vertical size in mm. Default: 15.6\n"
"\nPhysical environment properties: \n"
"-m :: Minimum distance from cameras to cull points. Default: 500mm\n"
"-M :: Maximum distance from cameras to cull points. Default: 6000mm\n"
"\nImage properties: \n"
"-S :: Image alignment shift. Overridden by auto-align if enabled. Default: 0 Lines\n"
"-A :: Disable auto vertical alignment. Default: Enabled\n"
"-a :: Disable reading of vertical alignment file. Default: Enabled\n"
"-T :: Auto align test limits. Default: +-20 Lines\n"
"\nOutput Options: \n"
"-O :: Points cloud output filename. Default: './cloud.ply'\n"
"-V :: Verbose mode.\n"
"\n"
};


#endif
