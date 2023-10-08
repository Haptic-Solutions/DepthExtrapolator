

#include "lodepng/lodepng.h"
///Windows Stuff
#ifndef LINUX_BUILD
#include "lodepng/lodepng.cpp"  //Lazy
#include <Windows.h>
#include <direct.h>
#endif
///End Windows Stuff
#include "depthExtrap.h"
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

int main(int argc, char *argv[]) {
  //Use lodepng to open the specified PNG file.
  //Using boilerplate code example listed in the header file. Thanks lode!
  //load and decode left image.
  std::vector<unsigned char> Limage;
  std::vector<unsigned char> Rimage;
  unsigned error = lodepng::decode(Limage, width, height, "test/LEFT.png", LCT_RGB, 8);
  //if there's an error, display it
  if(error) {std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;return 0;}

  error = lodepng::decode(Rimage, Rwidth, Rheight, "test/RIGHT.png", LCT_RGB, 8);
  //if there's an error, display it
  if(error) {std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;return 0;}
  //the pixels are now in the vector "Limage", 3 bytes per pixel, ordered RGBRGB..., use it as texture, draw it, ...
  if(width!=Rwidth){cout << "Width of images does not match. Stopping. \n";return 0;}
  if(height!=Rheight){cout << "Width of images does not match. Stopping. \n";return 0;}
  cout << "Got Files. Processing. \n";
  cout << "Pre-calculating parameters. \n";
  ///Pr-Calculate FOV based on camera parameters if it wasn't provided.
  X_Angle = new double [width];
  Y_Angle = new double [height];
  double F_width = width;    //Convert to floating point number.
  double F_height = height;  //Convert to floating point number.
  double F_i;
  scan_debug = (width/2)-1;
  cout << "Debug Scan is number:: " << scan_debug << "\n\n";
///Pre-calculate all XY angles into two arrays.
// Need Image resolution, and XY FOV or Sensor size + Lens focal length.
// FOV = 2 arctan(sensorSize/(2f))
    ///X angle.
  for(int i=0;i<width;i++){
    F_i=i;  //Convert to floating point number.
    double X_Num = X_Size*((F_i/F_width)-0.5); ///Get scan offset of each pixel with 0 in the middle.
    double foc_length = sqrt(pow(lens_foc,2)+pow(X_Num,2));    ///Get focal length of each pixel correct for spherical lens.
    if(!spherical_Lens)foc_length = lens_foc;
    X_Angle[i] = DegToRad(90)+atan(X_Num/foc_length);     ///Get view angle of each pixel shifted so center is 90deg.
    X_FOV = X_Angle[width-1]-X_Angle[0]; ///Get absolute FOV.
    //X_FOV = atan(X_Size/(lens_foc/2)); ///Get absolute FOV.
  }
    ///Y angle.
  for(int i=0;i<height;i++){
    F_i=i;  //Convert to floating point number.
    double Y_Num = Y_Size*((F_i/F_height)-0.5); ///Get scan offset of each pixel with 0 in the middle.
    double foc_length = sqrt(pow(lens_foc,2)+pow(Y_Num,2));    ///Get focal length of each pixel correct for spherical lens.
    if(!spherical_Lens)foc_length = lens_foc;
    Y_Angle[i] = DegToRad(90)+atan(Y_Num/foc_length);     ///Get view angle of each pixel shifted so center is 90deg.
    Y_FOV = Y_Angle[height-1]-Y_Angle[0]; ///Get absolute FOV.
    //Y_FOV = atan(Y_Size/(lens_foc/2)); ///Get absolute FOV.
  }

  cout << "First angle ------ " << RadToDeg(X_Angle[0]) << "\n";
  cout << "Middle angle -1pix " << RadToDeg(X_Angle[(width/2)-1]) << "\n";
  cout << "Middle angle ----- " << RadToDeg(X_Angle[width/2]) << "\n";
  cout << "Middle angle +1pix " << RadToDeg(X_Angle[(width/2)+1]) << "\n";
  cout << "Last angle ------- " << RadToDeg(X_Angle[width-1]) << "\n";
  cout << "-------------------\n";
  cout << "Sensor angle range:: " << RadToDeg(X_FOV) << "\n\n";
  cout << "Camera Lens Focal Length:: " << lens_foc << "mm\n";
  cout << "Camera Horizontal Sensor Size:: " << X_Size << "mm\n";
  cout << "Camera Vertical Sensor Size:: " << Y_Size << "mm\n";
  cout << "Camera Horizontal Resolution:: " << width << "\n";
  cout << "Camera Vertical Resolution:: " << height << "\n";
  cout << "Camera Horizontal FOV:: " << RadToDeg(X_FOV) << " deg\n";
  cout << "Camera Vertical FOV:: " << RadToDeg(Y_FOV) << " deg\n";

  ///Pre-Calculate min/max scanning angles based on min/max configured distances.
  //This is done assuming a right triangle with the opposite side in the center between both cameras.
  double Min_Dist_Ang = atan(min_Dist/Cam_Dist);
  double Max_Dist_Ang = atan(max_Dist/Cam_Dist);

  cout << "Min_Dist_Ang:: " << RadToDeg(Min_Dist_Ang) << " deg\n";
  cout << "Max_Dist_Ang:: " << RadToDeg(Max_Dist_Ang) << " deg\n";

  ///Pre-Convert Min/Mix angles to left/right max pixels to scan.
  int Pix_Start=0;
  int Pix_End=0;
  //Find minimum pixel location.
  for(int i=0;i<width;i++){
    if(Min_Dist_Ang<X_Angle[i]){
        Pix_Start = i;
        break;
    }
  }
  //Find maximum pixel location.
  for(int i=0;i<width;i++){
    if(Max_Dist_Ang<X_Angle[i]){
        Pix_End = i-1;
        break;
    }
  }
  cout << "Pre-Pix_Start:: " << Pix_Start << " pix\n";
  cout << "Pre-Pix_End:: " << Pix_End << " pix\n";
  int Pix_Diff = Pix_End-Pix_Start;     //Scan Width.
  ///Scale start/end by two.
  //Pix_Start += Pix_Diff/4;      //Shift Start.
  //Pix_End -= Pix_Diff/4;        //Shift End.
  int Pix_LeftCam_Start = (width/2)-Pix_End;     //Left View Start Pixel. Don't scan useless Pixels.
  //Shift Start and End.
  Pix_Start -= (width/2);      //Shift Start.
  Pix_End -= (width/2);        //Shift End.


  cout << "Pix_Start:: " << Pix_Start << " pix\n";
  cout << "Pix_End:: " << Pix_End << " pix\n";
  cout << "Pix_Width:: " << Pix_Diff << " pix\n";
  cout << "Pix_LeftCam_Start:: " << Pix_LeftCam_Start << " pix\n";

  /// Split the color channels to make them easier to understand the process.
  C_chSplit* O_channelsLEFT;
  C_chSplit* O_channelsRIGHT;
  O_channelsLEFT = new C_chSplit [width*height];
  O_channelsRIGHT = new C_chSplit [width*height];
  C_Points* O_Points;
  O_Points = new C_Points [width*height];
  int eoi = width*height;
  // Fill channels with data
  int ImgIndex = 0;
  cout << "Splitting Channels. \n";
  for(int i=0;i<eoi;i++){
    O_channelsLEFT[i].chnls[R] = Limage[ImgIndex];
    ImgIndex++;
    O_channelsLEFT[i].chnls[G] = Limage[ImgIndex];
    ImgIndex++;
    O_channelsLEFT[i].chnls[B] = Limage[ImgIndex];
    ImgIndex++;
  }
  ImgIndex = 0;
  for(int i=0;i<eoi;i++){
    O_channelsRIGHT[i].chnls[R] = Rimage[ImgIndex];
    ImgIndex++;
    O_channelsRIGHT[i].chnls[G] = Rimage[ImgIndex];
    ImgIndex++;
    O_channelsRIGHT[i].chnls[B] = Rimage[ImgIndex];
    ImgIndex++;
  }
  cout << "Channels Split, doing edge detection. \n";
  edgeOutLEFT = new unsigned char [(width*height)];
  edgeOutRIGHT = new unsigned char [(width*height)];
  reduxOutLEFT = new unsigned char [(width*height)];
  reduxOutRIGHT = new unsigned char [(width*height)];
  reduxMatchLEFT = new unsigned char [(width*height)];
  reduxMatchRIGHT = new unsigned char [(width*height)];
  for(int i=0;i<(width*height);i++)edgeOutLEFT[i]=0;    //Initialize edgeOutLEFT
  for(int i=0;i<(width*height);i++)edgeOutRIGHT[i]=0;    //Initialize edgeOutRIGHT
  for(int i=0;i<(width*height);i++)reduxOutLEFT[i]=0;    //Initialize reduxOutLEFT
  for(int i=0;i<(width*height);i++)reduxOutRIGHT[i]=0;    //Initialize reduxOutRIGHT
  for(int i=0;i<(width*height);i++)reduxMatchLEFT[i]=0;    //Initialize reduxOutLEFT
  for(int i=0;i<(width*height);i++)reduxMatchRIGHT[i]=0;    //Initialize reduxOutRIGHT
  /// Do horizontal and vertical edge detection.
  int L_Edge_Cnt=0;
  int R_Edge_Cnt=0;
  ///Horizontal
  cout << "Horizontal Left View Pass. \n";
  for(int c=0;c<3;c++){
    for(int y=0;y<height;y++){
        int color_old = 0;
        for(int x=0;x<width;x+=edgePixSkp){
            int color_diff = color_old - O_channelsLEFT[cord(x,y)].chnls[c];
            if(color_diff<0)color_diff*=-1;
            if(color_diff>threashold){
                L_Edge_Cnt++;
                edgeOutLEFT[cord(x,y)]=1;
            }
            color_old = O_channelsLEFT[cord(x,y)].chnls[c];
        }
    }
  }
  ///Vertical
  cout << "Vertical Left View Pass. \n";
  for(int c=0;c<3;c++){
    for(int x=0;x<width;x++){
        int color_old = 0;
        for(int y=0;y<height;y+=edgePixSkp){
            int color_diff = color_old - O_channelsLEFT[cord(x,y)].chnls[c];
            if(color_diff<0)color_diff*=-1;
            if(color_diff>threashold){
                L_Edge_Cnt++;
                edgeOutLEFT[cord(x,y)]=1;
            }
            color_old = O_channelsLEFT[cord(x,y)].chnls[c];
        }
    }
  }
  ///Horizontal
  cout << "Horizontal Right View Pass. \n";
  for(int c=0;c<3;c++){
    for(int y=0;y<height;y++){
        int color_old = 0;
        for(int x=0;x<width;x+=edgePixSkp){
            int color_diff = color_old - O_channelsRIGHT[cord(x,y)].chnls[c];
            if(color_diff<0)color_diff*=-1;
            if(color_diff>threashold){
                R_Edge_Cnt++;
                edgeOutRIGHT[cord(x,y)]=1;
            }
            color_old = O_channelsRIGHT[cord(x,y)].chnls[c];
        }
    }
  }
  ///Vertical
  cout << "Vertical Right View Pass. \n";
  for(int c=0;c<3;c++){
    for(int x=0;x<width;x++){
        int color_old = 0;
        for(int y=0;y<height;y+=edgePixSkp){
            int color_diff = color_old - O_channelsRIGHT[cord(x,y)].chnls[c];
            if(color_diff<0)color_diff*=-1;
            if(color_diff>threashold){
                R_Edge_Cnt++;
                edgeOutRIGHT[cord(x,y)]=1;
            }
            color_old = O_channelsRIGHT[cord(x,y)].chnls[c];
        }
    }
  }
  /// Convert to lower color depth. 64 colors.
  cout << "Doing color reduction for culling pixels that are too different. \n";
  cout << "Left View Pass. \n";
    for(int y=0;y<height;y++){
        for(int x=0;x<width-1;x++){
            reduxOutLEFT[cord(x,y)] |= (O_channelsLEFT[cord(x,y)].chnls[R]>>2)&0x30;
            reduxOutLEFT[cord(x,y)] |= (O_channelsLEFT[cord(x,y)].chnls[G]>>4)&0x0C;
            reduxOutLEFT[cord(x,y)] |= (O_channelsLEFT[cord(x,y)].chnls[B]>>6)&0x03;
        }
    }
  /// Do edge detection for Right channel.
  cout << "Right View Pass. \n";
    for(int y=0;y<height;y++){
        for(int x=0;x<width-1;x++){
            reduxOutRIGHT[cord(x,y)] |= (O_channelsRIGHT[cord(x,y)].chnls[R]>>2)&0x30;
            reduxOutRIGHT[cord(x,y)] |= (O_channelsRIGHT[cord(x,y)].chnls[G]>>4)&0x0C;
            reduxOutRIGHT[cord(x,y)] |= (O_channelsRIGHT[cord(x,y)].chnls[B]>>6)&0x03;
        }
    }
  cout << "Generating edge/color reduction diag output. \n";
  pngmake(1, width, height, 0, 0); ///Make test frame of the edge detection.
  pngmake(2, width, height, 0, 1); ///Make test frame of the edge detection.
  cout << "Left RGB Edges Found:: " << L_Edge_Cnt << "\n";
  cout << "Right RGB Edges Found:: " << R_Edge_Cnt << "\n";
  ///Do edge matching per line using color data to match points between Left and Right views.
  cout << "Doing points matching between channels. \n";
  C_bMatch* O_bestMatch;
  O_bestMatch = new C_bMatch [width];
  int match_count = 0;
  int match_used = 0;
  int better_matches = 0;
  int matches_discarded = 0;
  int T_Edge_Cnt = 0;
  int final_multi_point=0;
  int maxTD = (((Xsq_wdth*2)+1)*((Ysq_wdth*2)+1))*maxTotalDiff;
  double scale_dist = max_Dist / min_Dist;
  for(int y=Ysq_wdth;y<height-Ysq_wdth;y++){
    for(int x=Pix_LeftCam_Start;x<width-Xsq_wdth;x++){
        if(x!=(width/2)&&edgeOutLEFT[cord(x,y)]==1){
            int PX_Match = 0;
            T_Edge_Cnt++;
            //Found an edge on left viewport, search right viewport for edges and check to see if color matches.
            int LLX=x+Pix_Start;
            if(LLX<0)LLX=0;
            int lowest_Diff=maxTD;
            int lowest_Tx=0;
            int lowest_y=0;
            int multi_point=0;
            for(int Tx=LLX+Xsq_wdth;Tx<x+Pix_End;Tx++){
                if(x==scan_debug){
                    for(int redY=0;redY<width;redY++){
                        reduxMatchRIGHT[cord(LLX+Xsq_wdth,redY)]=15;
                        reduxMatchRIGHT[cord(x+Pix_End,redY)]=15;
                    }
                }
                if(Tx>width)break;  //Catch to keep from seg-faulting. Should never need this.
                if(reduxOutLEFT[cord(x,y)]==reduxOutRIGHT[cord(Tx,y)]&&Tx!=(width/2)){
                    //Found a Right-View edge, now compare colors and find the closest match.
                    ///Test square of pixels.
                    PX_Match = gridComp(x,Tx,y,O_channelsLEFT,O_channelsRIGHT);    ///Get best match.
                    if(PX_Match<lowest_Diff&&PX_Match>=0){
                        lowest_Diff=PX_Match;
                        lowest_Tx=Tx;
                        multi_point=0;
                    }
                    else if(PX_Match==lowest_Diff&&PX_Match>=0){
                        multi_point++;  //Mark if multiple matches and cull them out for now.
                        final_multi_point++;    //debug counter.
                        reduxOutRIGHT[cord(lowest_Tx,y)]=255;   ///Also delete it. If it's the same for this one then it's redundant to keep testing them.
                    }
                }
                if(PX_Match==-2)break;  ///Break from loop if there isn't enough detail in a block.
            }
            if(lowest_Diff<maxTD && multi_point<1 && PX_Match!=-2){
                /// Mark potential matches.
                match_count++;
                O_bestMatch[x].matchWith = lowest_Tx;
                O_bestMatch[x].pixScore = lowest_Diff;
            }
        }
    }
    /// Check for better matches than what we already found and discard the ones that are worse.
    for(int x=0;x<width;x++){
        int Gx=O_bestMatch[x].matchWith;
        int Gscore=O_bestMatch[x].pixScore;
        for(int xTest=x+1;xTest<(x+Pix_Diff)&&xTest<width;xTest++){
            ///Find two left pixels that are matching with the same right pixel; find and use the better match.
            int Hx=O_bestMatch[xTest].matchWith;
            int Hscore=O_bestMatch[xTest].pixScore;
            if(Gx>0 && Hx>0 && Gx==Hx){
                ///Find which one has the lower(better) score and remove the other one.
                if(Gscore<Hscore){
                        ///Remove Hscore
                    O_bestMatch[xTest].matchWith = 0;
                    O_bestMatch[xTest].pixScore = -1;
                    better_matches++;
                }
                else if(Gscore>Hscore){
                        ///Remove Gscore
                    O_bestMatch[x].matchWith = 0;
                    O_bestMatch[x].pixScore = -1;
                    better_matches++;
                }
                /// If scores are equal then remove both as we don't know which one is the correct match.
                else {
                    O_bestMatch[x].matchWith = 0;
                    O_bestMatch[x].pixScore = -1;
                    O_bestMatch[xTest].matchWith = 0;
                    O_bestMatch[xTest].pixScore = -1;
                    matches_discarded++;
                }
            }
        }
    }
    ///Use remaining points to calculate distances.
    for(int LeftX=0;LeftX<width;LeftX++){
        int RightX=O_bestMatch[LeftX].matchWith;
        if(RightX>0){
            match_used++;
            if(LeftX==scan_debug){
                reduxMatchLEFT[cord(LeftX,y)]=1;
                reduxMatchRIGHT[cord(RightX,y)]=1;
            }
            ///We got a set of points, now get their angles and calculate where they are actually at.
            calcPoint(LeftX, RightX, y, O_Points);
        }
        ///Clear it after it's been used.
        O_bestMatch[LeftX].matchWith = 0;
        O_bestMatch[LeftX].pixScore = -1;
    }
  }
  ///Free up memory.
  delete[] O_bestMatch; delete[] reduxOutLEFT; delete[] reduxOutRIGHT;
  delete[] edgeOutLEFT; delete[] edgeOutRIGHT;
  cout << "Edges Compared:: " << T_Edge_Cnt << "\n";
  cout << "Out of range center potential matches culled:: " << MaxDiffCenters << "\n";
  cout << "Out of range block potential matches culled:: " << MaxDiffBlocks << "\n";
  cout << "Low Contrast Points Culled:: " << LowContrastBlocks << "\n";
  cout << "Number of Total Points Found:: " << match_count << "\n";
  cout << "Duplicate points found and not using:: " << final_multi_point << "\n";
  cout << "Better Matches Found and Reallocated:: " << better_matches << "\n";
  cout << "Similar Matches Found and Discarded:: " << matches_discarded << "\n";
  cout << "Points to be used for output:: " << match_used << "\n";
  //cout << "Writing edge-match debug files. \n";
  //pngmake(1, width, height, 1, 0); ///Make test frame of the edge detection.
  //pngmake(2, width, height, 1, 1); ///Make test frame of the edge detection.
  ///Now convert the points to a readable file.
  cout << "Writing output PLY file. \n";
  ofstream Pfile;
  Pfile.open ("./cloud.ply");
  Pfile << PLYheader_Start[0] << match_used << "\n";
  Pfile << PLYheader_End[0];
  int Fout_Count = 0;
  double X_Scaler = width;
  for(int y=0;y<height;y++){
        for(int x=Pix_LeftCam_Start;x<width;x++){
            if(O_Points[cord(x,y)].Cord[X]||O_Points[cord(x,y)].Cord[Y]||O_Points[cord(x,y)].Cord[Z]){
                double F_y = y;
                Fout_Count++;
                Pfile << O_Points[cord(x,y)].Cord[X] << " " << O_Points[cord(x,y)].Cord[Z]*-1 << " " << O_Points[cord(x,y)].Cord[Y]*-1 << " "
                << O_channelsLEFT[cord(x,y)].chnls[R] << " "
                << O_channelsLEFT[cord(x,y)].chnls[G] << " ";
                if(Fout_Count >= match_count) Pfile << O_channelsLEFT[cord(x,y)].chnls[B];
                else Pfile << O_channelsLEFT[cord(x,y)].chnls[B] << "\n";
            }
        }
  }
  delete[] O_channelsLEFT; delete[] O_channelsRIGHT;
  cout << "Wrote " << Fout_Count << " points to file.\n";
  if(Fout_Count != match_used) cout << "!-> WARNING. Points written does not match points found. IDK why. <-! \n";
  Pfile.close();
  return 0;
}

unsigned int cord(unsigned int x, unsigned int y){
    return (y*width)+x;
}

double DegToRad(double DEG){
    return DEG*(M_PI/180);
}

double RadToDeg(double RAD){
    return RAD*(180/M_PI);
}

/* png image make */
/* Converts the three individual RGB channels into an RGB array for the lodepng library*/
void pngmake(int frameNum, int xRes, int yRes, int type, int channel){
    unsigned int res = xRes*yRes;
    unsigned int i = 0;
    unsigned char * rgbimage;
    unsigned char C_Add = 0;
    //create a new array that's the appropriate size for our resolution. No alpha channel.
    rgbimage = new unsigned char [(res*3)];
    //Copy the RGB data over to an array suitable for the PNG conversion library.
    for(int d=0;d<res-1;d++){
        int pixel=0;
        if(!type&&!channel){
            rgbimage[i] = ((reduxOutLEFT[d]&0x30)<<2)*edgeOutLEFT[d];
            i++;
            rgbimage[i] = ((reduxOutLEFT[d]&0x0C)<<4)*edgeOutLEFT[d];
            i++;
            rgbimage[i] = ((reduxOutLEFT[d]&0x03)<<6)*edgeOutLEFT[d];
            i++;
        }
        else if(!type&&channel){
            rgbimage[i] = ((reduxOutRIGHT[d]&0x30)<<2)*edgeOutRIGHT[d];
            i++;
            rgbimage[i] = ((reduxOutRIGHT[d]&0x0C)<<4)*edgeOutRIGHT[d];
            i++;
            rgbimage[i] = ((reduxOutRIGHT[d]&0x03)<<6)*edgeOutRIGHT[d];
            i++;
        }
        else {
            if(reduxMatchLEFT[d]&&!channel&&type)pixel=255;
            if(reduxMatchRIGHT[d]&&channel&&type)pixel=255;
            C_Add=0;
            if(reduxMatchRIGHT[d]>10&&channel&&type)C_Add=255;   ///Make red lines on Right channel.
            rgbimage[i] = pixel;
            i++;
            rgbimage[i] = pixel-C_Add;
            i++;
            rgbimage[i] = pixel-C_Add;
            i++;
        }
    }
    //lodepng_encode24_file(("./output/frame" + std::to_string(frameNum) + ".png"), rgbimage, xRes, yRes);
    stringstream fileNumber;
    switch(type){
        case 0:
            #ifdef LINUX_BUILD
            mkdir("./EdgeOutput",0777);
            #endif
            #ifndef LINUX_BUILD
            _mkdir("./EdgeOutput");
            #endif
            if(frameNum==1)fileNumber << "./EdgeOutput/LEFTframe.png";
            else fileNumber << "./EdgeOutput/RIGHTframe.png";
        break;
        case 1:
            #ifdef LINUX_BUILD
            mkdir("./debugOut",0777);
            #endif
            #ifndef LINUX_BUILD
            _mkdir("./debugOut");
            #endif
            if(frameNum==1)fileNumber << "./debugOut/LEFTframe.png";
            else fileNumber << "./debugOut/RIGHTframe.png";

        break;
    }
    lodepng_encode24_file(fileNumber.str().c_str(), rgbimage, xRes, yRes);
    //cleanup
    delete[] rgbimage;
}

///Grid comparator.
int gridComp(int x, int Tx, int y, C_chSplit * LEFTc, C_chSplit * RIGHTc){
    int PX_Match=0;
    int LctstCount=0;
    for(int Yadj=-Ysq_wdth;Yadj<=Ysq_wdth;Yadj++){
        for(int Xadj=-Xsq_wdth;Xadj<=Xsq_wdth;Xadj++){
            for(int c=0; c<3; c++){

                int LCenterContrast = LEFTc[cord(x,y)].chnls[c];
                LCenterContrast -= LEFTc[cord(x+Xadj,y+Yadj)].chnls[c];
                if(LCenterContrast<0)LCenterContrast*=-1;
                if(LCenterContrast>minBKcontrast)LctstCount++;

                int Diff_Test = LEFTc[cord(x+Xadj,y+Yadj)].chnls[c] - RIGHTc[cord(Tx+Xadj,y+Yadj)].chnls[c];
                if(Diff_Test<0)Diff_Test*=-1; //Get absolute value of difference.
                ///Cull if center pixel is out of range and skip to next pixel.
                ///Also cull if there isn't enough contrast between center pixel and surrounding pixels.
                if(!Xadj && !Yadj && Diff_Test>MaxCenterDiff){
                    MaxDiffCenters++;
                    PX_Match=-1;    //Mark it to skip, then break from loop.
                    break;
                }
                else if(Diff_Test>MaxColorDiff){
                    MaxDiffBlocks++;
                    PX_Match=-1;    //Mark it to skip, then break from loop.
                    break;
                }
                else PX_Match+=Diff_Test;   ///Add the differences. We want to use what is the lowest difference.
            }
            if(PX_Match<0)break;    //Break from marked bad matches and check next line for any.
        }
        if(PX_Match<0)break;    //Break from marked bad matches and check next line for any.
    }
    if(LctstCount<MinCtstCount){
        LowContrastBlocks++;
        PX_Match=-2;    //Mark it to skip to next left pixel block.
    }
    return PX_Match;
}

///Point calculator.
void calcPoint(int x, int Tx, int y, C_Points * O_Points){
    ///Found a point match. Now compute Z distance.
    /// We know that Z=0 for the first left and right points and that X=+-C_Dist/2
    /// Get slopes of rays using pre-computed angle data.
    double LSlp = tan(X_Angle[x]);          ///Get angle data for the left view.
    double RSlp = tan(X_Angle[Tx]);  ///Get angle data for the right view.
    double YSlp = tan(Y_Angle[y]);  ///Might as well compute the Y axis while we are here.
    /// Get second set of points via the X=0 Z intercept
    double LShft = (Cam_Dist/2)*LSlp;       ///Calculate the Z intercept for left view.
    double RShft = (Cam_Dist/-2)*RSlp;      ///Calculate the Z intercept for right view.
    /// Now the linear equation for each ray is Z=iSlp*X+iShft where 'i' is either L or R
    /// RSlp*X+RShft=LSlp*X+LShft
    /// We can subtract the Right view slope from both equations leaving X only on the Left equation.
    LSlp = LSlp-RSlp;
    /// Now it's RShft=LSlp*X+LShft
    /// Rearrange that to get the X cord of the intercept.
    /// RShft-LShft=LSlp*X
    /// (RShft-LShft)/LSlp=X
    double IX=(RShft-LShft)/LSlp;
    /// Now solve for Z using the right-side equation parameters because they haven't been modified.
    double tmpZ=(RSlp*IX)+RShft;
    /// Now scale Y using the Z data and put all the coords into memory.
    O_Points[cord(x,y)].Cord[X]=IX/1000;
    O_Points[cord(x,y)].Cord[Y]=(tmpZ/YSlp)/1000;
    O_Points[cord(x,y)].Cord[Z]=tmpZ/1000;
    /// And now we have the Z distance of each point.
}
