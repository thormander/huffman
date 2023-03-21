#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>

using namespace std;

// Function to compress input file and write the compressed data to output file
void compress(string input_file, string output_file) {
    // Your code here
}

// Function to decompress input file and write the decompressed data to output file
void decompress(string input_file, string output_file) {
    // Your code here
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Usage: " << argv[0] << " -huff input_file output_file" << endl;
        return 1;
    }

    string input_file = argv[2];
    string output_file = argv[3];

    if (string(argv[1]) == "-huff") 
    {
        cout << "huff executed!" << endl;
        compress(input_file, output_file);
        cout << "huff completed!" << endl << endl;
    }
    else if (string(argv[1]) == "-unhuff")
    {   
        cout << "unhuff executed!" << endl;
        decompress(input_file, output_file);
        cout << "unhuff completed!" << endl << endl;
    }
    else
    {
        cout << "Unknown option " << argv[1] << endl;
        return 1;
    }

    return 0;
}
