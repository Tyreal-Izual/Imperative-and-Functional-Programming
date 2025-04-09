1. fractal.pgm由converter转换为fractal.sk文件的大小为190.06KiB (指fractal.sk的大小)

1. fractal.pgm is converted from converter to fractal.sk The size of this file is 190.06 KiB (referring to the size of fractal.sk)

2. 目前converter支持将pgm文件转换为sk文件

2. At present, the converter supports converting PGM files into SK files.

3. 算法使用了最简单的方式来实现, 但是进行了细节的优化
    a. 从左到右遍历pixel, 使用DATA,DX,DY,TARGETX,TARGETY等命令依次画每一个pixel
    b. 然后从右到左, 
    c. 循环a-b

3. The algorithm is implemented in the simplest way, but the details are optimized.
a. Traverse the pixel from left to right, and draw each pixel in turn with the commands of DATA, DX, DY, TARGETX, TARGETY,
b. Then from right to left,
c. Cycle a-b.




   Copyright 2025 Frederick Zou

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
