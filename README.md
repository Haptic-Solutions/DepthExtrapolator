# DepthExtrapolator
A proof of concept program for testing the math behind extracting depth info from stereoscopic images.

Currently accepts only PNG type images, however it proceses the images in an RGB format.
Multi-threaded for speed, but runs on CPU only.
Currently no command-line paramters. Must be recompiled to change settings.


To bulid, just CD to the root directory and type 'make'. You may need to 'make clean' first.


To run, put your two images in the 'test' directory. One image name 'left.PNG' and the other name 'right.PNG' then run the main program from the root directory.

All camera paramters such as lense focal length, and image sensor size/resolution must be accurate and image alignment is critical for a true output. Output is in meters. There is not auto-calibration and currently paramiters are set in code before compile time.
