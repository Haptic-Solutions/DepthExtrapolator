# DepthExtrapolator
A proof of concept program for testing the math behind extracting depth info from stereoscopic images.

Currently accepts only PNG type images, however it proceses the images in an RGB format.
Multi-threaded for speed, but runs on CPU only.

To bulid, just CD to the root directory and type 'make'. You may need to 'make clean' first.

To run, put your two images in the 'test' directory. One image name 'LEFT.png' and the other name 'RIGHT.png' then run the main program from the root directory.

All camera paramters such as lense focal length, and image sensor size/resolution must be accurate and image alignment is critical for a true output. Output is in meters. There is some auto-calibration and currently some paramiters are set via command line options.

This program is provided without any warranty.
"Copyright: Haptic Solutions, January 2024

-h for help text.

