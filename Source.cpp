/*
* Author: Thor Landstrom
* Assignment Title: Huffman
* Assignment Description: Make a program that performs compression and decompression using Huffman's algorithim
* Due Date: 4/10/2023
* Date Created: 3/25/2023
* Date Last Modified: 4/7/2023
*/

/*
* Data Abstraction:
*
* Input: A .txt file in same directory as source code
*        A .huff file (or preferred extension) in same directory as source code
*
* Output: A compressed file of a given .txt file (extension can be named by user, however I will just refer to it as .huff)
*         A decompressed .txt file from the .huff created
*
* Process: The two main processes of the program are the 'compress' and 'decompress' functions. The compress function reads the input file using ofstream
*          and counts the frequency of the characters storing them in a unordered map. Next, the huffman tree is built and the huffman code is generated.
*          After this process, it first writes the magic number to the beginning and then writes the codes and necessary padding. The file is read again, 
*          this time encoding each of the characters encountered. This is then outputted after our tree on the .huff compressed file. Add the psuedo-EOF to
*          the end of the file. The second main function is the decompress function. On reading the compressed file, we scan for the magic number at the beginning 
*          of the file. If there, read the codes and store the data in a unordered map. From here, read the compressed file storing them as bit strings, and 
*          converting them back using the codes stored earlier. Continue this process until the psuedo-EOF character is encountered at the end (31 - Unit Seperator).
*
* Assumptions: For compression, it is assumed that user will input through the command line following the format "-huff inputFileName.txt outputFileName.huff"
*              For decompression, it is assumed that user will input through the command line following the format "-unhuff outputFileName.huff outputFileName.txt "
*              It is assumed if using VScode, to use project properties/debugging to input the command line arguement.
*/

#include <iostream>
#include <fstream> 
#include <queue> 
#include <unordered_map> //for huffman codes
#include <vector>
#include <bitset>
#include <functional> //encode function for huffman tree

using namespace std;

int MAX_CODE_LENGTH = 16; //Limits length of huffman code to help with compression
const uint32_t MAGIC_NUMBER = 0xB00B135; //Prevent opening of files not compressed through this program

struct Node {
    char ch;
    int freq;
    Node* left;
    Node* right;

    Node(char ch, int freq) : ch(ch), freq(freq), left(nullptr), right(nullptr) {}
};

struct compare {
    bool operator()(Node* l, Node* r) {
        return l->freq > r->freq;
    }
};

void buildHuffmanTree(priority_queue<Node*, vector<Node*>, compare>& minHeap, unordered_map<char, string>& huffCodes) {
    int maxCodeLen = 16; // Define maximum code length
    while (minHeap.size() > 1) {
        Node* left = minHeap.top();
        minHeap.pop();

        Node* right = minHeap.top();
        minHeap.pop();

        Node* newNode = new Node('\0', left->freq + right->freq);
        newNode->left = left;
        newNode->right = right;
        minHeap.push(newNode);
    }

    Node* root = minHeap.top();

    function<void(Node*, const string&)> encode = [&](Node* node, const string& code) {
        if (node == nullptr) return;

        if (!node->left && !node->right) {
            huffCodes[node->ch] = code;
        }

        encode(node->left, code + "0");
        encode(node->right, code + "1");
    };

    encode(root, "");
}

void compress(string input_file, string output_file) {
    ifstream in(input_file, ios::binary);

    int bitPos = 0;
    unsigned char curByte = 0;

    if (!in.is_open()) {
        cerr << "Error opening input file" << endl;
        return;
    }

    unordered_map<char, int> freqMap;
    char ch;
    while (in.get(ch)) {
        freqMap[ch]++;
    }
    freqMap[31] = 1; // Add pseudo-EOF character with a frequency of 1

    priority_queue<Node*, vector<Node*>, compare> minHeap;
    for (const auto& item : freqMap) {
        minHeap.push(new Node(item.first, item.second));
    }

    unordered_map<char, string> huffCodes;
    buildHuffmanTree(minHeap, huffCodes);

    in.clear();
    in.seekg(0);

    ofstream out(output_file, ios::binary);
    if (!out.is_open()) {
        cerr << "Error opening output file" << endl;
        return;
    }

    // Write the magic number
    out.write(reinterpret_cast<const char*>(&MAGIC_NUMBER), sizeof(MAGIC_NUMBER));

    // Write the number of Huffman codes and extraBits at the beginning of the file
    out.put(static_cast<unsigned char>((huffCodes.size() - 1) >> 8)); // Store the high byte of the number of Huffman codes minus one
    out.put(static_cast<unsigned char>((huffCodes.size() - 1) & 0xFF)); // Store the low byte of the number of Huffman codes minus one

    string bitString;
    while (in.get(ch)) {
        bitString += huffCodes[ch];
    }

    string pseudoEofCode = huffCodes[31];
    bitString += pseudoEofCode; // Add the pseudo-EOF character at the end

    int extraBits = (8 - bitString.size() % 8) % 8;
    for (int i = 0; i < extraBits; i++) {
        bitString += "0";
    }

    // Write the extraBits value
    out.put(static_cast<unsigned char>(extraBits));

    // Write the Huffman codes
    for (const auto& item : huffCodes) {
        out.put(item.first);
        out.put(static_cast<char>(item.second.size()));
        for (int i = 0; i < item.second.size(); i += MAX_CODE_LENGTH) {
            uint16_t bits = 0;
            for (int j = 0; j < MAX_CODE_LENGTH && i + j < item.second.size(); ++j) {
                bits |= (item.second[i + j] == '1') << (MAX_CODE_LENGTH - 1 - j);
            }
            out.put(static_cast<char>((bits >> 8) & 0xFF)); // Store the high byte of the bits
            out.put(static_cast<char>(bits & 0xFF)); // Store the low byte of the bits
        }
    }

    // Write the compressed data
    for (int i = 0; i < bitString.size(); ++i) {
        curByte = (curByte << 1) | (bitString[i] == '1' ? 1 : 0);
        bitPos++;

        if (bitPos == 8) {
            out.put(static_cast<char>(curByte));
            curByte = 0;
            bitPos = 0;
        }
    }

    if (bitPos > 0) {
        curByte <<= (8 - bitPos);
        out.put(static_cast<char>(curByte));
    }

    in.close();
    out.close();
}

void decompress(string input_file, string output_file) {
    ifstream in(input_file, ios::binary);
    if (!in.is_open()) {
        cerr << "Error opening input file" << endl;
        return;
    }

    ofstream out(output_file, ios::binary);
    if (!out.is_open()) {
        cerr << "Error opening output file" << endl;
        return;
    }

    int64_t curBytePos = in.tellg();
    in.seekg(0, ios::end);
    int64_t fileSize = in.tellg();
    in.seekg(0, ios::beg);

    // Check the magic number
    uint32_t read_magic_number;
    in.read(reinterpret_cast<char*>(&read_magic_number), sizeof(read_magic_number));
    if (read_magic_number != MAGIC_NUMBER) {
        cerr << "Error: Invalid magic number. This file may not have been compressed by this program." << endl;
        return;
    }

    int numCodes = (static_cast<int>(static_cast<unsigned char>(in.get())) << 8) + static_cast<int>(static_cast<unsigned char>(in.get())) + 1;
    int storedExtraBits = static_cast<unsigned char>(in.get());

    if (numCodes > (1 << MAX_CODE_LENGTH)) {
        cerr << "Error: Huffman code length exceeds maximum code length" << endl;
        return;
    }

    unordered_map<string, char> huffMap;
    for (int i = 0; i < numCodes; i++) {
        char ch;
        in.get(ch);

        int codeLen = static_cast<unsigned char>(in.get());
        string code;
        int remainingBits = codeLen;
        while (remainingBits > 0) {
            char byte1, byte2;
            in.get(byte1);
            in.get(byte2);
            uint16_t bits = (static_cast<unsigned char>(byte1) << 8) + static_cast<unsigned char>(byte2);
            int bitsToAppend = min(remainingBits, MAX_CODE_LENGTH);
            for (int i = 0; i < bitsToAppend; ++i) {
                code += (bits & (1 << (MAX_CODE_LENGTH - 1 - i))) ? '1' : '0';
            }
            remainingBits -= bitsToAppend;
        }

        huffMap[code] = ch;
    }

    string bitString;
    char byte;
    while (curBytePos < fileSize - 1) {
        byte = static_cast<char>(in.get());
        curBytePos = in.tellg();
        bitset<8> bits(byte);

        int bitsToAppend = 8;
        if (curBytePos == fileSize - 1) { // If it's the last byte, only append up to the extra bits
            bitsToAppend -= storedExtraBits;
        }

        bitString += bits.to_string().substr(0, bitsToAppend);
    }


    string code;
    for (size_t i = 0; i < bitString.size() - storedExtraBits; ++i) {
        char bit = bitString[i];
        code += bit;
        if (huffMap.find(code) != huffMap.end()) {
            char ch = huffMap[code];
            if (ch == 31) {
                break;
            }
            else {
                out.put(ch);
            }
            code = "";
        }
    }

    in.close();
    out.close();
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
