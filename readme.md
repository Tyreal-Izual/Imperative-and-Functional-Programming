1. fractal.pgm is converted from converter to fractal.sk. The size of this file is 190.06 KiB (referring to the size of fractal.sk).

2. At present, the converter supports converting PGM files into SK files.

3. The algorithm is implemented in the simplest way, but the details are optimized.
a. Traverse the pixel from left to right, and draw each pixel in turn with the commands of DATA, DX, DY, TARGETX, TARGETY;
b. Then from right to left;
c. Cycle a-b.