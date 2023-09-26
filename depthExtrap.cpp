

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

  error = lodepng::decode(Rimage, width, height, "test/RIGHT.png", LCT_RGB, 8);
  //if there's an error, display it
  if(error) {std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;return 0;}
  //the pixels are now in the vector "Limage", 3 bytes per pixel, ordered RGBRGB..., use it as texture, draw it, ...
  cout << "Got Files. Processing. \n";
  cout << "Pre-calculating parameters. \n\n";
  ///Pr-Calculate FOV based on camera parameters if it wasn't provided.
  //Need Image resolution, and XY FOV or Sensor size + Lens focal length.
  // FOV = 2 arctan(sensorSize/(2f))
  if(!X_FOV)X_FOV = atan(X_Size/(lens_f/2));
  if(!Y_FOV)Y_FOV = atan(Y_Size/(lens_f/2));

  ///Pre-calculate all XY angles into two arrays.
  X_Angle = new double [width];
  Y_Angle = new double [height];
  //Make it so the center is 90deg.
  double X_FOV_Adjustment = (DegToRad(180)-X_FOV)/2;
  double Y_FOV_Adjustment = (DegToRad(180)-Y_FOV)/2;
  ///Pre-calculate all of the pixel angles.
  double F_width = width;    //Convert to floating point number.
  double F_height = height;  //Convert to floating point number.
  double F_i;
  for(int i = 0;i<width;i++){
    F_i=i;  //Convert to floating point number.
    X_Angle[i] = X_FOV_Adjustment+(X_FOV*(F_i/F_width));
  }
  for(int i = 0;i<height;i++){
    F_i=i;  //Convert to floating point number.
    Y_Angle[i] = Y_FOV_Adjustment+(Y_FOV*(F_i/F_height));
  }
  cout << "First angle " << RadToDeg(X_Angle[0]) << "\n";
  cout << "Middle angle " << RadToDeg(X_Angle[(width/2)-1]) << "\n";
  cout << "Middle angle " << RadToDeg(X_Angle[width/2]) << "\n";
  cout << "Middle angle " << RadToDeg(X_Angle[(width/2)+1]) << "\n";
  cout << "Last angle " << RadToDeg(X_Angle[width-1]) << "\n\n";
  cout << "Camera Lens Focal Length " << lens_f << "mm\n";
  cout << "Camera Horizontal Sensor Size " << X_Size << "mm\n";
  cout << "Camera Vertical Sensor Size " << Y_Size << "mm\n";
  cout << "Camera Horizontal Resolution " << width << "\n";
  cout << "Camera Vertical Resolution " << height << "\n";
  cout << "Camera Horizontal FOV " << RadToDeg(X_FOV) << " deg\n";
  cout << "Camera Vertical FOV " << RadToDeg(Y_FOV) << " deg\n";

  ///Pre-Calculate min/max scanning angles based on min/max configured distances.
  //This is done assuming a right triangle with the opposite side in the center between both cameras.
  double Min_Ang = atan(min_Dist/(Cam_Dist/2));
  double Max_Ang = atan(max_Dist/(Cam_Dist/2));

  cout << "Min_Ang " << RadToDeg(Min_Ang) << " deg\n";
  cout << "Max_Ang " << RadToDeg(Max_Ang) << " deg\n";

  ///Pre-Convert Min/Mix angles to left/right max pixels to scan.
  int Pix_Start=0;
  int Pix_End=0;
  //Find minimum pixel location.
  for(int i=0;i<width;i++){
    if(Min_Ang<X_Angle[i]){
        Pix_Start = i;
        break;
    }
  }
  //Find maximum pixel location.
  for(int i=0;i<width;i++){
    if(Max_Ang<X_Angle[i]){
        Pix_End = i-1;
        break;
    }
  }
  cout << "Pre-Pix_Start " << Pix_Start << " pix\n";
  cout << "Pre-Pix_End " << Pix_End << " pix\n";
  int Pix_Diff = Pix_End-Pix_Start;     //Scan Width.
  ///Scale start/end by two.
  Pix_Start += Pix_Diff/4;      //Shift Start.
  Pix_End -= Pix_Diff/4;        //Shift End.
  int Pix_LeftCam_Start = (width/2)-Pix_End;     //Left View Start Pixel. Don't scan useless Pixels.
  //Shift Start and End.
  Pix_Start -= (width/2);      //Shift Start.
  Pix_End -= (width/2);        //Shift End.


  cout << "Pix_Start " << Pix_Start << " pix\n";
  cout << "Pix_End " << Pix_End << " pix\n";
  cout << "Pix_Width " << Pix_Diff << " pix\n";
  cout << "Pix_LeftCam_Start " << Pix_LeftCam_Start << " pix\n";

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
  cout << "Channels Split, doing horizontal pass. \n";
  edgeOutLEFT = new unsigned char [(width*height)];
  edgeOutRIGHT = new unsigned char [(width*height)];
  edgeMatchLEFT = new unsigned char [(width*height)];
  edgeMatchRIGHT = new unsigned char [(width*height)];
  for(int i=0;i<(width*height);i++)edgeOutLEFT[i]=0;    //Initialize edgeOutLEFT
  for(int i=0;i<(width*height);i++)edgeOutRIGHT[i]=0;    //Initialize edgeOutRIGHT
  for(int i=0;i<(width*height);i++)edgeMatchLEFT[i]=0;    //Initialize edgeOutLEFT
  for(int i=0;i<(width*height);i++)edgeMatchRIGHT[i]=0;    //Initialize edgeOutRIGHT
  /// Do edge detection for Left channel.
  int L_Edge_Cnt=0;
  int R_Edge_Cnt=0;
  cout << "Left View Pass. \n";
  for(int ch=1;ch<4;ch++){
      for(int y=0;y<height;y++){
        for(int x=0;x<width-1;x++){
            int tpix = O_channelsLEFT[cord(x,y)].chnls[ch-1]-O_channelsLEFT[cord(x+1,y)].chnls[ch-1];
            if(tpix<0)tpix*=-1;
            if(tpix>threashold){
                L_Edge_Cnt++;
                if(ch==3)edgeOutLEFT[cord(x,y)]+=ch+1;
                else edgeOutLEFT[cord(x,y)]+=ch;
                //x+=5;   //Skip 5 pixels every time we find an edge.
            }
        }
      }
  }
  /// Do edge detection for Right channel.
  cout << "Right View Pass. \n";
  for(int ch=1;ch<4;ch++){
      for(int y=0;y<height;y++){
        for(int x=0;x<width-1;x++){
            int tpix = O_channelsRIGHT[cord(x,y)].chnls[ch-1]-O_channelsRIGHT[cord(x+1,y)].chnls[ch-1];
            if(tpix<0)tpix*=-1;
            if(tpix>threashold){
                R_Edge_Cnt++;
                if(ch==3)edgeOutRIGHT[cord(x,y)]+=ch+1;
                else edgeOutRIGHT[cord(x,y)]+=ch;
                //x+=5;   //Skip 5 pixels every time we find an edge.
            }
        }
      }
  }
  pngmake(1, width, height, 0, 0); ///Make test frame of the edge detection.
  pngmake(2, width, height, 0, 1); ///Make test frame of the edge detection.
  cout << "Left RGB Edges Found " << L_Edge_Cnt << "\n";
  cout << "Right RGB Edges Found " << R_Edge_Cnt << "\n";
  ///Do edge matching per line using color data to match points between Left and Right views.
  cout << "Doing edge matching between channels, and calculating distances of points. \n";
  int match_count = 0;
  int T_Edge_Cnt = 0;
  int final_multi_point=0;
  int edge_OOR = 0;
  double scale_dist = max_Dist / min_Dist;
  for(int y=Ysq_wdth;y<height-Ysq_wdth;y++){
    for(int x=Pix_LeftCam_Start;x<width-Xsq_wdth;x++){
        if(edgeOutLEFT[cord(x,y)]&&x!=(width/2)){
            T_Edge_Cnt++;
            //Found an edge on left viewport, search right viewport for edges and check to see if color matches.
            int LLX=x+Pix_Start;
            if(LLX<0)LLX=0;
            int lowest_Diff=10000;
            int lowest_Tx=0;
            int multi_point=0;
            for(int Tx=LLX+Xsq_wdth;Tx<x+Pix_End;Tx++){
                if(x==scan_debug){
                    for(int redY=0;redY<width;redY++){
                        edgeMatchRIGHT[cord(LLX+Xsq_wdth,redY)]=15;
                        edgeMatchRIGHT[cord(x+Pix_End,redY)]=15;
                    }
                }
                if(Tx>width)break;  //Catch to keep from seg-faulting. Should never need this.
                if(edgeOutLEFT[cord(x,y)] == edgeOutRIGHT[cord(x,y)]&&Tx!=(width/2)){
                    //Found a Right-View edge, now compare colors and find the closest match.
                    int PX_Match=0;
                    ///Test square of pixels.
                    for(int Yadj=-Ysq_wdth;Yadj<=Ysq_wdth;Yadj++){
                        for(int Xadj=-Xsq_wdth;Xadj<=Xsq_wdth;Xadj++){
                            int C_Weight = 1;
                            if(!Yadj&&!Xadj)C_Weight=centerWeight;
                            for(int c=0; c<=2; c++){
                                int C_Test = O_channelsLEFT[cord(x+Xadj,y+Yadj)].chnls[c]*C_Weight - O_channelsRIGHT[cord(Tx+Xadj,y+Yadj)].chnls[c]*C_Weight;
                                if(C_Test<0)C_Test*=-1; //Get absolute value of difference.
                                ///Cull if center pixel is out of range and skip to next pixel.
                                if(!Yadj&&!Xadj&&C_Test>MaxColorDiff){
                                    PX_Match=-1;    //Mark it to skip, then break from loop.
                                    lowest_Tx=-1;    //Mark as a match not found and to ignore this point.
                                    edge_OOR++;
                                    break;
                                }
                                PX_Match+=C_Test;   ///Add the differences.
                            }
                            if(PX_Match<0)break;
                        }
                        if(PX_Match<0)break;
                    }
                    if(PX_Match<lowest_Diff&&PX_Match>0){
                        lowest_Diff=PX_Match;
                        lowest_Tx=Tx;
                        multi_point=0;
                    }
                    else if(PX_Match==lowest_Diff&&PX_Match>0){
                        multi_point++;  //Mark if multiple matches and cull them out for now.
                        final_multi_point++;
                    }
                }
            }
            if(lowest_Diff<10000 && lowest_Tx>0 && multi_point<1){
                ///Found a point match. Now compute Z distance.
                //Write debug images.
                if(x==scan_debug){
                    edgeMatchLEFT[cord(x,y)]=1;
                    edgeMatchRIGHT[cord(lowest_Tx,y)]=1;
                }

                match_count++;
                edgeOutRIGHT[cord(lowest_Tx,y)]=0; ///Delete this point so that it's not used again. We don't need it anymore.
                // We know that Z=0 for the first left and right points and that X=+-C_Dist/2
                // Get slopes of rays.
                double LSlp = tan(X_Angle[x]);
                double RSlp = tan(X_Angle[lowest_Tx]);
                double YSlp = tan(Y_Angle[y]);  //Might as well compute the Y axis while we are here.
                // Get second set of points via the X=0 Z intercept
                double LS = (Cam_Dist/2)*LSlp;
                double RS = (Cam_Dist/-2)*RSlp;
                /// Now the linear equation for each ray is Z=iSlp*X+iS where 'i' is either L or R
                // We can subtract the Right view slope from the Left view
                double DSlp = LSlp-RSlp;
                // Which leaves the equation to solve the intercept point to be: DSlp*X+LS=RS
                // Rearrange that to get the X cord of the intercept.
                /// DSlp*X+LS=RS
                /// DSlp*X=RS-LS
                /// X=(RS-LS)/DSlp
                double IX=(RS-LS)/DSlp;
                /// Now solve for Z;
                // Scale the points to 1 meter distances.
                double tmpZ = LSlp*IX+LS;
                O_Points[cord(x,y)].Cord[X]=IX/1000;
                O_Points[cord(x,y)].Cord[Y]=(tmpZ/YSlp)/1000;
                O_Points[cord(x,y)].Cord[Z]=tmpZ/1000;
                /// And now we have the Z distance of each point.
            }
        }
    }
  }
  pngmake(1, width, height, 1, 0); ///Make test frame of the edge detection.
  pngmake(2, width, height, 1, 1); ///Make test frame of the edge detection.
  cout << "Edges Compared " << T_Edge_Cnt << "\n";
  cout << "Edge Colors Out Of Range " << edge_OOR << "\n";
  cout << "Duplicate points found and not using " << final_multi_point << "\n";
  cout << "Found " << match_count << " points to match. Done calculating their distances. \n";
  ///Now convert the points to a readable file.
  cout << "Writing output PLY file. \n";
  ofstream Pfile;
  Pfile.open ("./cloud.ply");
  Pfile << PLYheader_Start[0] << match_count << "\n";
  Pfile << PLYheader_End[0];
  int Fout_Count = 0;
  double X_Scaler = width;
  for(int y=0;y<height;y++){
        for(int x=Pix_LeftCam_Start;x<width;x++){
            if(O_Points[cord(x,y)].Cord[Y]){
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
  cout << "Wrote " << Fout_Count << " points to file.\n";
  if(Fout_Count != match_count) cout << "!-> WARNING. Points written does not match points found. IDK why. <-! \n";
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
    for(int d=0;d<res;d++){
        int pixel=0;
        if(edgeOutLEFT[d]&&!channel&&!type)pixel=255;
        if(edgeOutRIGHT[d]&&channel&&!type)pixel=255;
        if(edgeMatchLEFT[d]&&!channel&&type)pixel=255;
        if(edgeMatchRIGHT[d]&&channel&&type)pixel=255;
        C_Add=0;
        if(edgeMatchRIGHT[d]>10&&channel&&type)C_Add=255;   ///Make red lines on Right channel.
        rgbimage[i] = pixel;
        i++;
        rgbimage[i] = pixel-C_Add;
        i++;
        rgbimage[i] = pixel-C_Add;
        i++;
    }
    //lodepng_encode24_file(("./output/frame" + std::to_string(frameNum) + ".png"), rgbimage, xRes, yRes);
    stringstream fileNumber;
    switch(type){
        case 0:
            #ifdef LINUX_BUILD
            mkdir("./output",0777);
            #endif
            #ifndef LINUX_BUILD
            _mkdir("./output");
            #endif
            if(frameNum==1)fileNumber << "./output/LEFTframe.png";
            else fileNumber << "./output/RIGHTframe.png";
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
