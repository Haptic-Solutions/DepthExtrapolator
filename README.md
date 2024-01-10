# DepthExtrapolator
A proof of concept program for testing the math behind extracting depth info from stereoscopic images.

Currently accepts only PNG type images, however it proceses the images in an RGB format.
Multi-threaded for speed, but runs on CPU only.
Currently no command-line paramters. Must be recompiled to change settings.


To bulid, just CD to the root directory and type 'make'. You may need to 'make clean' first.


To run, put your two images in the 'test' directory. One image name 'LEFT.png' and the other name 'RIGHT.png' then run the main program from the root directory.

All camera paramters such as lense focal length, and image sensor size/resolution must be accurate and image alignment is critical for a true output. Output is in meters. There is not auto-calibration and currently paramiters are set in code before compile time.



This program is provided without any warranty.
"Copyright: Haptic Solutions, January 2024

-h :: This Help Text.

Input Options:
-L :: Left Image Input Filename. Default: './left.png'
-R :: Right Image Input Filename. Default: './right.png'

Camera Parameters:
-I :: IPD distance of cameras. Default: 50.8mm or 2 Inches
-F :: Focal length in mm of camera lenses. Default: 18mm
-x :: Image sensor horizontal size in mm. Default: 23.5mm
-y :: Image sensor vertical size in mm. Default: 15.6

Physical environment properties:
-m :: Minimum distance from cameras to cull points. Default: 500mm
-M :: Maximum distance from cameras to cull points. Default: 6000mm

Image properties:
-S :: Image alignment shift. Overridden by auto-align if enabled. Default: 0 Lines
-A :: Disable auto vertical alignment. Default: Enabled
-a :: Disable reading of vertical alignment file. Default: Enabled
-T :: Auto align test limits. Default: +-20 Lines

Output Options:
-O :: Points cloud output filename. Default: './cloud.ply'
-V :: Verbose mode.

