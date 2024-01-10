
#include "lodepng/lodepng.h"
#include "depthExtrap.h"
#include <fstream>
#include <thread>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

int main(int argc, char * argv[]) {
  if(verbose)std::cout << "\x1B[2J\x1B[H"; /// Clear screen on linux.
  //Get command ARGs
  if(G_GetArgs(argc, argv))return 0;
  //Use lodepng to open the specified PNG file.
  //Using boilerplate code example listed in the header file. Thanks lode!
  //load and decode left image.
  unsigned error = lodepng::decode(G_Limage, G_width, G_height, Lfilename, LCT_RGB, 8);
  //if there's an error, display it
  if (error) {
    std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    return 0;
  }

  error = lodepng::decode(G_Rimage, G_width, G_height, Rfilename, LCT_RGB, 8);
  //if there's an error, display it
  if (error) {
    std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    return 0;
  }
  //the pixels are now in the vector "G_Limage", 3 bytes per pixel, ordered RGBRGB..., use it as texture, draw it, ...
  if (G_width != G_width) {
    cout << "G_width of images does not match. Stopping. \n";
    return 0;
  }
  if (G_height != G_height) {
    cout << "G_width of images does not match. Stopping. \n";
    return 0;
  }
  if(verbose){
  cout << "Got Files. Processing. \n";
  cout << "Pre-calculating parameters. \n";
  }
  ///Pr-Calculate FOV based on camera parameters if it wasn't provided.
  G_X_Angle = new double[G_width];
  G_Y_Angle = new double[G_height];
  double F_G_width = G_width; //Convert to floating point number.
  double F_G_height = G_height; //Convert to floating point number.
  double F_i;
  ///Pre-calculate all XY angles into two arrays.
  // Need Image resolution, and XY FOV or Sensor size + Lens focal length.
  // FOV = 2 arctan(sensorSize/(2f))
  ///X angle.
  for (int i = 0; i < G_width; i++) {
    F_i = i; //Convert to floating point number.
    double X_Num = X_Size * ((F_i / F_G_width) - 0.5); ///Get scan offset of each pixel with 0 in the middle.
    double foc_length = sqrt(pow(lens_foc, 2) + pow(X_Num, 2)); ///Get focal length of each pixel correct for spherical lens.
    if (!spherical_Lens) foc_length = lens_foc;
    G_X_Angle[i] = G_DegToRad(90) + atan(X_Num / foc_length); ///Get view angle of each pixel shifted so center is 90deg.
    X_FOV = G_X_Angle[G_width - 1] - G_X_Angle[0]; ///Get absolute FOV.
    //X_FOV = atan(X_Size/(lens_foc/2)); ///Get absolute FOV.
  }
  ///Y angle.
  for (int i = 0; i < G_height; i++) {
    F_i = i; //Convert to floating point number.
    double Y_Num = Y_Size * ((F_i / F_G_height) - 0.5); ///Get scan offset of each pixel with 0 in the middle.
    double foc_length = sqrt(pow(lens_foc, 2) + pow(Y_Num, 2)); ///Get focal length of each pixel correct for spherical lens.
    if (!spherical_Lens) foc_length = lens_foc;
    G_Y_Angle[i] = G_DegToRad(90) + atan(Y_Num / foc_length); ///Get view angle of each pixel shifted so center is 90deg.
    Y_FOV = G_Y_Angle[G_height - 1] - G_Y_Angle[0]; ///Get absolute FOV.
    //Y_FOV = atan(Y_Size/(lens_foc/2)); ///Get absolute FOV.
  }

  if(verbose){
  cout << "First angle ------ " << G_RadToDeg(G_X_Angle[0]) << "\n";
  cout << "Middle angle -1pix " << G_RadToDeg(G_X_Angle[(G_width / 2) - 1]) << "\n";
  cout << "Middle angle ----- " << G_RadToDeg(G_X_Angle[G_width / 2]) << "\n";
  cout << "Middle angle +1pix " << G_RadToDeg(G_X_Angle[(G_width / 2) + 1]) << "\n";
  cout << "Last angle ------- " << G_RadToDeg(G_X_Angle[G_width - 1]) << "\n";
  cout << "-------------------\n";
  cout << "Sensor angle range:: " << G_RadToDeg(X_FOV) << "\n\n";
  cout << "Camera Lens Focal Length:: " << lens_foc << "mm\n";
  cout << "Camera Horizontal Sensor Size:: " << X_Size << "mm\n";
  cout << "Camera Vertical Sensor Size:: " << Y_Size << "mm\n";
  cout << "Camera Horizontal Resolution:: " << G_width << "\n";
  cout << "Camera Vertical Resolution:: " << G_height << "\n";
  cout << "Camera Horizontal FOV:: " << G_RadToDeg(X_FOV) << " deg\n";
  cout << "Camera Vertical FOV:: " << G_RadToDeg(Y_FOV) << " deg\n";
  }

  ///Pre-Calculate min/max scanning angles based on min/max configured distances.
  //This is done assuming a right triangle with the opposite side in the center between both cameras.
  double Min_Dist_Ang = atan(min_Dist / Cam_Dist);
  double Max_Dist_Ang = atan(max_Dist / Cam_Dist);

  if(verbose){
  cout << "Min_Dist_Ang:: " << G_RadToDeg(Min_Dist_Ang) << " deg\n";
  cout << "Max_Dist_Ang:: " << G_RadToDeg(Max_Dist_Ang) << " deg\n";
  }

  ///Pre-Convert Min/Mix angles to left/right max pixels to scan.
  int Pix_Start = 0;
  int Pix_End = 0;
  //Find minimum pixel location.
  for (int i = 0; i < G_width; i++) {
    if (Min_Dist_Ang < G_X_Angle[i]) {
      Pix_Start = i;
      break;
    }
  }
  //Find maximum pixel location.
  for (int i = 0; i < G_width; i++) {
    if (Max_Dist_Ang < G_X_Angle[i]) {
      Pix_End = i - 1;
      break;
    }
  }
  if(verbose){
  cout << "Pre-Pix_Start:: " << Pix_Start << " pix\n";
  cout << "Pre-Pix_End:: " << Pix_End << " pix\n";
  }
  G_Pix_Diff = Pix_End - Pix_Start; //Scan G_width.
  ///Scale start/end by two.
  //Pix_Start += G_Pix_Diff/4;      //Shift Start.
  //Pix_End -= G_Pix_Diff/4;        //Shift End.
  int Pix_LeftCam_Start = (G_width / 2) - Pix_End; //Left View Start Pixel. Don't scan useless Pixels.
  //Shift Start and End.
  Pix_Start -= (G_width / 2); //Shift Start.
  Pix_End -= (G_width / 2); //Shift End.

  if(verbose){
  cout << "Pix_Start:: " << Pix_Start << " pix\n";
  cout << "Pix_End:: " << Pix_End << " pix\n";
  cout << "Pix_G_width:: " << G_Pix_Diff << " pix\n";
  cout << "Pix_LeftCam_Start:: " << Pix_LeftCam_Start << " pix\n";
  }
  ///Get exposure compensation if enabled.
  if(exComp){
      if(verbose)cout << "Performing exposure compensation.\n";
      double leftRGBaverage[3]={0,0,0};
      double rightRGBaverage[3]={0,0,0};
      for(int c=0;c<3;c++){
          for(int x=Pix_LeftCam_Start;x<G_width;x++){
            for(int y=0;y<G_height;y++){
                leftRGBaverage[c] += G_Limage[G_COLOR_cordST(x, y, c)];
            }
          }
          leftRGBaverage[c] /= G_width*G_height;
      }

      for(int c=0;c<3;c++){
          for(int x=0;x<G_width-Pix_LeftCam_Start;x++){
            for(int y=0;y<G_height;y++){
                rightRGBaverage[c] += G_Rimage[G_COLOR_cordST(x, y, c)];
            }
          }
          rightRGBaverage[c] /= G_width*G_height;
      }
      for(int c=0;c<3;c++)RGBcompensation[c] = leftRGBaverage[c] - rightRGBaverage[c];
      if(verbose){
          cout << "R comp " << RGBcompensation[R];
          cout << ", G comp " << RGBcompensation[G];
          cout << ", B comp " << RGBcompensation[B] << "\n";
      }
  }
  ///Do edge matching per line using color data to match points between Left and Right views.
  C_Points * O_Points;
  O_Points = new C_Points[G_width * G_height];
  int Completion_Status = G_height / 10;


  int maxTD = (((Xsq_wdth * 2) + 1) * ((Ysq_wdth * 2) + 1)) * maxTotalDiff;
  double scale_dist = max_Dist / min_Dist;

  C_threadCalc * O_CPU_Thread_Calc = new C_threadCalc();
  ///Do vertical shift adjustment.
  if(Auto_Vert_Align){
    ifstream VertFile("./vertshift.txt");
    // check stream status
    if (VertFile&&!ignorVertFile){
        if(verbose)cout << "Got vertical shift setting from pre-calculated file.\n";
        string line;
        getline(VertFile,line);
        Vert_Pix_Align=stoi(line);
        VertFile.close();
    }
    else {
        if(verbose){
            if(!ignorVertFile)cout << "Can't open vertical shift settings file. Making new one. \n";
            else cout << "Ignoring vertical alignment file per request. \n";
            cout << "Doing Automatic Vertical Alignment. Up to +-" << Vert_Pix_Test << " pixels. \n";
        }
        int y = G_height / 2;
        int maxPointMatches = 0;
        int bestShift = 0;
        for(int ShiftTest=-Vert_Pix_Test;ShiftTest<Vert_Pix_Test;ShiftTest++){
            int RightY = y + ShiftTest;
            int thNum = 0;
            ///Test 12 lines at the same time.
            thread NewThread1 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
            thread NewThread2 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
            thread NewThread3 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
            thread NewThread4 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
            thread NewThread5 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
            thread NewThread6 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
            thread NewThread7 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
            thread NewThread8 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
            thread NewThread9 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
            thread NewThread10 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
            thread NewThread11 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
            thread NewThread12 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;

            NewThread1.join();
            NewThread2.join();
            NewThread3.join();
            NewThread4.join();
            NewThread5.join();
            NewThread6.join();
            NewThread7.join();
            NewThread8.join();
            NewThread9.join();
            NewThread10.join();
            NewThread11.join();
            NewThread12.join();

            if(G_match_count>maxPointMatches){
                Vert_Pix_Align = ShiftTest;
                maxPointMatches = G_match_count;
            }
            ///Reset everything.
            G_T_Edge_Cnt = 0;
            G_MaxDiffCenters = 0;
            G_MaxDiffBlocks = 0;
            G_LowContrastBlocks = 0;
            G_match_count = 0;
            G_final_multi_point = 0;
            G_better_matches = 0;
            G_matches_discarded = 0;
            G_match_used = 0;
        }
        ofstream VertFile;
        VertFile.open ("./vertshift.txt");
        VertFile << Vert_Pix_Align;
        VertFile.close();
      }
  }
  if(verbose){
  cout << "Right image shifted by " << Vert_Pix_Align << " pixels. \n";
  cout << "Doing points matching between channels.\nWorking: ";
  cout.clear(); ///Clear buffer.
  cout << "<          >" << "\b\b\b\b\b\b\b\b\b\b\b" << std::flush;
  }
  int yLowLimit = Ysq_wdth;
  int yHighLimit = G_height - Ysq_wdth;
  if(Vert_Pix_Align>0)yHighLimit-=Vert_Pix_Align;
  else yLowLimit-=Vert_Pix_Align;

  for (int y = yLowLimit; y < yHighLimit; y+=12) {
     int RightY = y + Vert_Pix_Align;
     if (y >= Completion_Status) {
      Completion_Status += G_height / 10;
      if(verbose)cout << "=" << std::flush;
     }
    /// X processing goes here...
    int thNum = 0;
    thread NewThread1 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
    thread NewThread2 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
    thread NewThread3 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
    thread NewThread4 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
    thread NewThread5 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
    thread NewThread6 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
    thread NewThread7 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
    thread NewThread8 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
    thread NewThread9 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
    thread NewThread10 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
    thread NewThread11 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;
    thread NewThread12 (&C_threadCalc::slpm, O_CPU_Thread_Calc[thNum], std::ref(Pix_LeftCam_Start), std::ref(G_width), std::ref(G_height), std::ref(Xsq_wdth), std::ref(maxTD), std::ref(Pix_Start), std::ref(Pix_End), std::ref(y), std::ref(RightY), std::ref(O_Points), std::ref(thNum)); thNum++;


    NewThread1.join();
    NewThread2.join();
    NewThread3.join();
    NewThread4.join();
    NewThread5.join();
    NewThread6.join();
    NewThread7.join();
    NewThread8.join();
    NewThread9.join();
    NewThread10.join();
    NewThread11.join();
    NewThread12.join();
  }

  if(verbose){
  cout << "=> Completed!\n";
  cout << "Edges Compared:: " << G_T_Edge_Cnt << "\n";
  cout << "Out of range center potential matches culled:: " << G_MaxDiffCenters << "\n";
  cout << "Out of range block potential matches culled:: " << G_MaxDiffBlocks << "\n";
  cout << "Low Contrast Points Culled:: " << G_LowContrastBlocks << "\n";
  cout << "Number of Total Points Found:: " << G_match_count << "\n";
  cout << "Duplicate points found and not using:: " << G_final_multi_point << "\n";
  cout << "Better Matches Found and Reallocated:: " << G_better_matches << "\n";
  cout << "Similar Matches Found and Discarded:: " << G_matches_discarded << "\n";
  cout << "Total point matches found:: " << G_match_used << "\n";
  cout << "Culling stray points.\n";
  }
  C_Points * O_CulledPoints;
  O_CulledPoints = new C_Points[G_match_used];
  C_Chroma * O_CulledColors;
  O_CulledColors = new C_Chroma[G_match_used];
  int lastMatchUsed = 0;
  int totalPasses = 0;
  int oldG_match_used = G_match_used;
  if (!CullingPasses) CullingPasses = 10000;
  /// Cull lonely points. IE: Noise.
  for (int CPasses = 0; CPasses < CullingPasses; CPasses++) {
    G_match_used = 0;
    int f = 0;
    int cullCount = 0;
    for (int y = 0; y < G_height; y++) {
      for (int x = 0; x < G_width; x++) {
        cullCount = 0;
        float Dx = O_Points[G_cordST(x, y)].Cord[X];
        float Dy = O_Points[G_cordST(x, y)].Cord[Y];
        float Dz = O_Points[G_cordST(x, y)].Cord[Z];
        /// If point 'exists', check distance against surrounding points in a grid pattern, but
        /// only if it's PassNum is equal to the current pass. Otherwise, skip it.
        if (O_Points[G_cordST(x, y)].PassNum == CPasses && (Dx || Dy || Dz)) {
          int xStart = x - TestGrid;
          int xEnd = x + TestGrid;
          int yStart = y - TestGrid;
          int yEnd = y + TestGrid;
          if (xStart < 0) xStart = 0;
          if (xEnd > G_width - 1) xEnd = G_width - 1;
          if (yStart < 0) yStart = 0;
          if (yEnd > G_height - 1) yEnd = G_height - 1;
          for (int Ty = yStart; Ty < yEnd; Ty++) {
            for (int Tx = xStart; Tx < xEnd; Tx++) {
              float Ex = O_Points[G_cordST(Tx, Ty)].Cord[X];
              float Ey = O_Points[G_cordST(Tx, Ty)].Cord[Y];
              float Ez = O_Points[G_cordST(Tx, Ty)].Cord[Z];
              /// If point 'exists', do comparison.
              if (Ex || Ey || Ez) {
                /// If distance is less than minimum distance the count it.
                if (sqrt(pow(Ex - Dx, 2) + pow(Ey - Dy, 2) + pow(Ez - Dz, 2)) < MinCullDist) cullCount++;
                /// Once close points counted is above the minimum, no need to continue counting.
                /// Add to keepers, and continue to next point.
                if (cullCount >= MinCullCount) {
                  O_CulledPoints[f].Cord[X] = Dx;
                  O_CulledPoints[f].Cord[Y] = Dy;
                  O_CulledPoints[f].Cord[Z] = Dz;
                  O_CulledColors[f].chnl[R] = G_Limage[G_COLOR_cordST(x, y, R)];
                  O_CulledColors[f].chnl[G] = G_Limage[G_COLOR_cordST(x, y, G)];
                  O_CulledColors[f].chnl[B] = G_Limage[G_COLOR_cordST(x, y, B)];
                  O_Points[G_cordST(x, y)].PassNum = CPasses + 1; ///Mark PassNum to which pass we are on +1.
                  /// We use this to only check the ones that match the same pass we are on. This
                  /// essentially marks the keepers in 'O_Points'.
                  f++;
                  G_match_used++;
                  cullCount = -1; /// Mark to continue without counting more.
                  break;
                }
              }
            }
            if (cullCount == -1) break;
          }
        }
      }
    }
    /// If G_match_used isn't changing between passes, then we have reached the limit in how many will be culled.
    if (lastMatchUsed == G_match_used) break;
    lastMatchUsed = G_match_used;
    totalPasses++;
  }
  if(verbose){
  cout << "Total Culling Passes:: " << totalPasses << "\n";
  cout << "Noise Points Culled:: " << oldG_match_used - G_match_used << "\n";
  cout << "Points remaining after distance culling:: " << G_match_used << "\n";
  }
  ///Now convert the points to a readable file.
  if(verbose)cout << "Writing output PLY file. \n";
  ofstream Pfile;
  Pfile.open(Ofilename);
  Pfile << PLYheader_Start[0] << G_match_used << "\n";
  Pfile << PLYheader_End[0];
  int Fout_Count = 0;
  for (int d = 0; d < G_match_used; d++) {
    if (O_CulledPoints[d].Cord[X] || O_CulledPoints[d].Cord[Y] || O_CulledPoints[d].Cord[Z]) {
      Fout_Count++;
      Pfile << O_CulledPoints[d].Cord[X] << " " <<
        O_CulledPoints[d].Cord[Z] * -1 << " " <<
        O_CulledPoints[d].Cord[Y] * -1 << " " <<
        O_CulledColors[d].chnl[R] << " " <<
        O_CulledColors[d].chnl[G] << " ";
      if (Fout_Count >= G_match_count) Pfile << O_CulledColors[d].chnl[B];
      else Pfile << O_CulledColors[d].chnl[B] << "\n";
    }
  }
  if(verbose)cout << "Wrote " << Fout_Count << " points to file.\n";
  if (Fout_Count != G_match_used) cout << "!-> WARNING. Points written does not match points found. IDK why. <-! \n";
  Pfile.close();
  return 0;
}

unsigned int G_cordST(unsigned int x, unsigned int y) {
  unsigned int output = (y * G_width) + x;
  if (output >= G_width * G_height) return 0;
  else return output;
}

unsigned int G_COLOR_cordST(unsigned int x, unsigned int y, unsigned int c) {
  unsigned int output = (y * (G_width * 3)) + ((x * 3) + c);
  if (output >= G_width * G_height * 3) return 0;
  else return output;
}

double G_DegToRad(double DEG) {
  return DEG * (PI_aprox / 180);
}

double G_RadToDeg(double RAD) {
  return RAD * (180 / PI_aprox);
}

///single-line point matcher
void C_threadCalc::slpm(int Pix_LeftCam_Start,
  unsigned int G_width, unsigned int G_height, int Xsq_wdth,
  int maxTD, int Pix_Start, int Pix_End, int y, int RightY,
  C_Points * O_Points, int threadNumber){

  if(y+threadNumber+1 >= G_height-Ysq_wdth)return;

  pixScore = new int [G_width];
  matchWith = new int [G_width];
  PixSkip = new bool[G_width*G_height];
  for (int i=0;i<G_width*G_height;i++)PixSkip[i] = 0;
  for(int i=0; i<G_width;i++){
    pixScore[i] = -1;
    matchWith[i] = 0;
  }
  y+=threadNumber;
/* ! This loop right here is what takes the most time. !*/
/*   If it can be streamlined, hardware accelerated via GPU or other means, or otherwise replaced with a more efficient algo, it would greatly speed up performance. */
  for (int x=Pix_LeftCam_Start;x<(G_width-Xsq_wdth);x++){
    if (isEdge(x,y)){
      int PX_Match = 0;
      G_T_Edge_Cnt++;
      //Found an edge on left viewport, search right viewport to see if a color match is close enough.
      int LLX = x+Pix_Start;
      if (LLX < 0) LLX = 0;
      int lowest_Diff = maxTD;
      int lowest_Tx = 0;
      int lowest_y = 0;
      int multi_point = 0;
      for (int Tx = LLX + Xsq_wdth; Tx < x + Pix_End; Tx++) {
        if (!PixSkip[cord(Tx, y)]) {
          if (reduxMatch(x, Tx, y, RightY)) {
            //Found a Right-View edge, now compare colors and find the closest match.
            ///Test square of pixels.
            PX_Match = gridComp(x, Tx, y, RightY); ///Get best match.
            if (PX_Match < lowest_Diff && PX_Match >= 0) {
              lowest_Diff = PX_Match;
              lowest_Tx = Tx;
              multi_point = 0;
            } else if (PX_Match == lowest_Diff && PX_Match >= 0) {
              multi_point++; //Mark if multiple matches and cull them out for now.
              G_final_multi_point++; //debug counter.
              PixSkip[cord(lowest_Tx, y)] = 1; ///Set to skip. If it's the same for this one then it's redundant to keep testing them.
            }
          }
        }
        if (PX_Match == -2) break; ///Break from loop if there isn't enough detail in a block.
      }
      if (lowest_Diff < maxTD && multi_point < 1 && PX_Match != -2) {
        /// Mark potential matches.
        G_match_count++;
        matchWith[x] = lowest_Tx;
        pixScore[x] = lowest_Diff;
      }
      x+=edgePixSkp;
    }
  }

/// Check for better matches than what we already found and discard the ones that are worse.
    for (int x = 0; x < G_width; x++) {
      int Gx = matchWith[x];
      int Gscore = pixScore[x];
      for (int xTest = x + 1; xTest < (x + G_Pix_Diff) && xTest < G_width; xTest++) {
        ///Find two left pixels that are matching with the same right pixel; find and use the better match.
        int Hx = matchWith[xTest];
        int Hscore = pixScore[xTest];
        if (Gx > 0 && Hx > 0 && Gx == Hx) {
          ///Find which one has the lower(better) score and remove the other one.
          if (Gscore < Hscore) {
            ///Remove Hscore
            matchWith[xTest] = 0;
            pixScore[xTest] = -1;
            G_better_matches++;
          } else if (Gscore > Hscore) {
            ///Remove Gscore
            matchWith[x] = 0;
            pixScore[x] = -1;
            G_better_matches++;
          }
          /// If scores are equal then remove both as we don't know which one is the correct match.
          else {
            matchWith[x] = 0;
            pixScore[x] = -1;
            matchWith[xTest] = 0;
            pixScore[xTest] = -1;
            G_matches_discarded++;
          }
        }
      }
    }
    ///Use remaining points to calculate distances.
    for (int LeftX = 0; LeftX < G_width; LeftX++) {
      int RightX = matchWith[LeftX];
      if (RightX > 0) {
        G_match_used++;
        ///We got a set of points, now get their angles and calculate where they are actually at.
        calcPoint(LeftX, RightX, y, O_Points);
      }
      ///Clear it after it's been used.
      matchWith[LeftX] = 0;
      pixScore[LeftX] = -1;
    }
}

#ifdef NOYCOMP
///Grid comparator.
int C_threadCalc::gridComp(int x, int Tx, int y, int RightY) {
    int PX_Match = 0;
    for (int Xadj = -Xsq_wdth; Xadj <= Xsq_wdth; Xadj++) {
      /// Test each color channel.
      for (int c = 0; c < 3; c++) {
        int Diff_Test = G_Limage[COLOR_cord(x+Xadj, y, c)] - G_Rimage[COLOR_cord(Tx+Xadj, RightY, c)];
        if (Diff_Test < 0) Diff_Test *= -1; //Get absolute value of difference.
        ///Cull if center pixel is out of range and skip to next pixel.
        ///Also cull if there isn't enough contrast between center pixel and surrounding pixels.
        if (Diff_Test > MaxColorDiff) {
          G_MaxDiffBlocks++;
          PX_Match = -1; //Mark it to skip, then break from loop.
          break;
        } else PX_Match += Diff_Test; ///Add the differences. We want to use what is the lowest difference.
      }
      if (PX_Match < 0) break; //Break from marked bad matches and check next line for any.
    }
  return PX_Match;
}
#endif
#ifndef NOYCOMP
///Grid comparator.
int C_threadCalc::gridComp(int x, int Tx, int y, int RightY) {
  int PX_Match = 0;
  int LctstCount = 0;
  for (int Yadj = -Ysq_wdth; Yadj <= Ysq_wdth; Yadj++) {
    for (int Xadj = -Xsq_wdth; Xadj <= Xsq_wdth; Xadj++) {
      /// Test each color channel.
      for (int c = 0; c < 3; c++) {
        int LCenterContrast = G_Limage[COLOR_cord(x, y, c)];
        LCenterContrast -= G_Limage[COLOR_cord(x + Xadj, y + Yadj, c)];
        if (LCenterContrast < 0) LCenterContrast *= -1;
        if (LCenterContrast > minBKcontrast) LctstCount++;

        int Diff_Test = G_Limage[COLOR_cord(x + Xadj, y + Yadj, c)]-(G_Rimage[COLOR_cord(Tx + Xadj, RightY + Yadj, c)]+RGBcompensation[c]);
        if (Diff_Test < 0) Diff_Test *= -1; //Get absolute value of difference.
        ///Cull if center pixel is out of range and skip to next pixel.
        ///Also cull if there isn't enough contrast between center pixel and surrounding pixels.
        if (!Xadj && !Yadj && Diff_Test > MaxCenterDiff) {
          G_MaxDiffCenters++;
          PX_Match = -1; //Mark it to skip, then break from loop.
          break;
        } else if (Diff_Test > MaxColorDiff) {
          G_MaxDiffBlocks++;
          PX_Match = -1; //Mark it to skip, then break from loop.
          break;
        } else PX_Match += Diff_Test; ///Add the differences. We want to use what is the lowest difference.
      }
      if (PX_Match < 0) break; //Break from marked bad matches and check next line for any.
    }
    if (PX_Match < 0) break; //Break from marked bad matches and check next line for any.
  }
  if (LctstCount < MinCtstCount) {
    G_LowContrastBlocks++;
    PX_Match = -2; //Mark it to skip to next left pixel block.
  }
  return PX_Match;
}
#endif

///Point calculator.
void C_threadCalc::calcPoint(int x, int Tx, int y, C_Points * O_Points) {
  ///Found a point match. Now compute Z distance.
  /// We know that Z=0 for the first left and right points and that X=+-C_Dist/2
  /// Get slopes of rays using pre-computed angle data.
  double LSlp = tan(G_X_Angle[x]); ///Get angle data for the left view.
  double RSlp = tan(G_X_Angle[Tx]); ///Get angle data for the right view.
  double YSlp = tan(G_Y_Angle[y]); ///Might as well compute the Y axis while we are here.
  /// Get second set of points via the X=0 Z intercept
  double LShft = (Cam_Dist / 2) * LSlp; ///Calculate the Z intercept for left view.
  double RShft = (Cam_Dist / -2) * RSlp; ///Calculate the Z intercept for right view.
  /// Now the linear equation for each ray is Z=iSlp*X+iShft where 'i' is either L or R
  /// RSlp*X+RShft=LSlp*X+LShft
  /// We can subtract the Right view slope from both equations leaving X only on the Left equation.
  LSlp = LSlp - RSlp;
  /// Now it's RShft=LSlp*X+LShft
  /// Rearrange that to get the X cord of the intercept.
  /// RShft-LShft=LSlp*X
  /// (RShft-LShft)/LSlp=X
  double IX = (RShft - LShft) / LSlp;
  /// Now solve for Z using the right-side equation parameters because they haven't been modified.
  double tmpZ = (RSlp * IX) + RShft;
  /// Now scale Y using the Z data and put all the coords into memory.
  O_Points[cord(x, y)].Cord[X] = IX / 1000;
  O_Points[cord(x, y)].Cord[Y] = (tmpZ / YSlp) / 1000;
  O_Points[cord(x, y)].Cord[Z] = tmpZ / 1000;
  /// And now we have the Z distance of each point.
}

bool C_threadCalc::reduxMatch(int x, int Ex, int y, int RightY) {
  int tstCNT = 0;
  for (int c = 0; c < 3; c++) {
    if ((G_Limage[COLOR_cord(x, y, c)] & 0xC0) == (G_Rimage[COLOR_cord(Ex, RightY, c)] & 0xC0)) tstCNT++;
  }
  if (tstCNT > 2) return true;
  else return false;
}

bool C_threadCalc::isEdge(int x, int y) {
  for (int c = 0; c < 3; c++) {
    int Etest = G_Limage[COLOR_cord(x, y, c)] - G_Limage[COLOR_cord(x + edgePixDist, y, c)];
    if (Etest < 0) Etest *= -1;
    if (threashold < Etest) return true;
  }
  for (int c = 0; c < 3; c++) {
    int Etest = G_Limage[COLOR_cord(x, y, c)] - G_Limage[COLOR_cord(x, y + edgePixDist, c)];
    if (Etest < 0) Etest *= -1;
    if (threashold < Etest) return true;
  }
  return false;
}

unsigned int C_threadCalc::cord(unsigned int x, unsigned int y) {
  unsigned int output = (y * G_width) + x;
  if (output >= G_width * G_height) return 0;
  else return output;
}

unsigned int C_threadCalc::COLOR_cord(unsigned int x, unsigned int y, unsigned int c) {
  unsigned int output = (y * (G_width * 3)) + ((x * 3) + c);
  if (output >= G_width * G_height * 3) return 0;
  else return output;
}

int G_GetArgs(int argc, char **argv){
  float argNumber = 0;
  int gotLFile = 1;
  int gotRFile = 1;
  int gotOFile = 1;
  for(int i=1;i<argc;i++){
    if(argv[i][0]=='-'){
        if(argv[i][1]!='L' && argv[i][1]!='R' && argv[i][1]!='A' &&
           argv[i][1]!='a' && argv[i][1]!='h' && argv[i][1]!='O' &&
           argv[i][1]!='V' && argv[i][1]!='E'){
            std::stringstream wasd(&argv[i][3]);
            wasd >> argNumber;
        }
        switch (argv[i][1]){
            case 'L':
                Lfilename=&argv[i][3];
                gotLFile = 0;
                break;
            case 'R':
                Rfilename=&argv[i][3];
                gotRFile = 0;
                break;
            case 'O':
                Ofilename=&argv[i][3];
                gotOFile = 0;
                break;
            case 'A':
                Auto_Vert_Align = false;
                break;
            case 'a':
                ignorVertFile = true;
                break;
            case 'h':
                cout << helpText[0];
                return 1;
                break;
            case 'V':
                verbose = true;
                break;
            case 'E':
                exComp = false;
                break;
            case 'I':
                if(argNumber>=1&&argNumber<=1000)Cam_Dist=argNumber;
                else {cout << "\nArgument Value Out Of Range For: " << &argv[i][0] << ", limit 1 - 1000\n"; return 1;}
                break;
            case 'F':
                if(argNumber>=0&&argNumber<=1000)lens_foc=argNumber;
                else {cout << "\nArgument Value Out Of Range For: " << &argv[i][0] << ", limit 0 - 1000\n"; return 1;}
                break;
            case 'x':
                if(argNumber>=0&&argNumber<=100)X_Size=argNumber;
                else {cout << "\nArgument Value Out Of Range For: " << &argv[i][0] << ", limit 0 - 100\n"; return 1;}
                break;
            case 'y':
                if(argNumber>=0&&argNumber<=100)Y_Size=argNumber;
                else {cout << "\nArgument Value Out Of Range For: " << &argv[i][0] << ", limit 0 - 100\n"; return 1;}
                break;
            case 'm':
                if(argNumber>=0&&argNumber<=100000)min_Dist=argNumber;
                else {cout << "\nArgument Value Out Of Range For: " << &argv[i][0] << ", limit 0 - 100000\n"; return 1;}
                break;
            case 'M':
                if(argNumber>=0&&argNumber<=1000000)max_Dist=argNumber;
                else {cout << "\nArgument Value Out Of Range For: " << &argv[i][0] << ", limit 0 - 1000000\n"; return 1;}
                break;
            case 'S':
                if(argNumber>=-1000&&argNumber<=1000)Vert_Pix_Align=argNumber;
                else {cout << "\nArgument Value Out Of Range For: " << &argv[i][0] << ", limit -1000 - 1000\n"; return 1;}
                break;
            case 'T':
                if(argNumber>=0&&argNumber<=1000)Vert_Pix_Test=argNumber;
                else {cout << "\nArgument Value Out Of Range For: " << &argv[i][0] << ", limit 0 - 1000\n"; return 1;}
                break;
        }
    }
  }
  if(gotLFile && gotRFile){
    if(verbose)cout << "No input filenames given. Using './left.png' and './right.png' \n";
    Lfilename = "./left.png";
    Rfilename = "./right.png";
  }
  else if(gotLFile){
    cout << "Please give Left Image filename using '-L' option, or -h for help text.\n";
  return 1;
  }
  else if(gotRFile){
    cout << "Please give Right Image filename using '-R' option, or -h for help text.\n";
  return 1;
  }
  if(gotOFile){
    if(verbose)cout << "No output filename given. Using './cloud.ply' \n";
    Ofilename = "./cloud.ply";
  }
  return 0;
}
