# Huffman Compression Program for .txt Files
## Project Requirements:
- Main Functionality:
  - Compression of a .txt file using Huffman's algorithim
  - Uncompress the file and return original .txt file
- Program Robustness:
  - Compressed file uses a psuedo eof (end of file) character and magic number
  - Program will not run on files without the magic number at the start of the file

## Program Guide:
- How to compile on mac:
  - ```cd``` into whichever directory you cloned it to
  - Run ```clang++ -std=c++11 Source.cpp -o huffmanProgram```
  - This will generate a executable that you can use to run the program
- Usage:
  - It is recommended to use a '.huff' extension when compressing, however you may use whichever you like.
  - To compress: ```./huffmanProgram -huff <your_text_file.txt> <name_of_compressed_file.huff>```
  - To uncompress ```./huffmanPorgram -unhuff <name_of_compressed_file.huff> <your_text_file.txt>```
