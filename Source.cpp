#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <functional>

using namespace std;

int MAX_CODE_LENGTH = 16;

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

    for (int i = 0; i < bitString.size(); i += 8) {
        bitset<8> byte(bitString.substr(i, 8));
        out.put(static_cast<char>(byte.to_ulong()));
    }

    out.put(static_cast<unsigned char>(huffCodes.size() - 1)); // Store the number of Huffman codes minus one
    out.put(static_cast<unsigned char>(extraBits));


    for (const auto& item : huffCodes) {
        out.put(item.first);
        out.put(static_cast<char>(item.second.size()));
        for (int i = 0; i < item.second.size(); i += 8) {
            bitset<8> byte(item.second.substr(i, 8));
            out.put(static_cast<char>(byte.to_ulong()));
        }
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

    int numCodes = static_cast<int>(static_cast<unsigned char>(in.get())) + 1;
    int storedExtraBits = static_cast<unsigned char>(in.get()); // Store the extraBits value in a separate variable

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
        for (int j = 0; j < (codeLen + 7) / 8; j++) {
            char byte;
            in.get(byte);
            bitset<8> bits(byte);
            if (codeLen >= 8) {
                code += bits.to_string();
                codeLen -= 8;
            }
            else {
                code += bits.to_string().substr(0, codeLen);
                break;
            }
        }

        huffMap[code] = ch;
    }

    string bitString;
    char byte;
    while (in.get(byte)) {
        bitset<8> bits(byte);
        bitString += bits.to_string();
    }

    bitString.erase(bitString.size() - storedExtraBits, storedExtraBits); // Remove extra bits using the stored value

    string code;
    for (char bit : bitString) {
        code += bit;
        if (huffMap.find(code) != huffMap.end()) { // Check if the code exists in the Huffman map
            char ch = huffMap[code];
            if (ch == 31) {
                break; // Stop decoding when the pseudo-EOF character is encountered
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
