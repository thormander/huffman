#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <functional>

using namespace std;

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

    int extraBits = (8 - bitString.size() % 8) % 8;
    for (int i = 0; i < extraBits; i++) {
        bitString += "0";
    }

    for (int i = 0; i < bitString.size(); i += 8) {
        bitset<8> byte(bitString.substr(i, 8));
        out.put(static_cast<char>(byte.to_ulong()));
    }

    out.put(static_cast<char>(extraBits));

    for (const auto& item : huffCodes) {
        out.put(item.first);
        out.put(static_cast<char>(item.second.size()));
        for (char bit : item.second) {
            out.put(bit);
        }
    }

    out.put('\0'); //psuedo eof char
    in.close();
    out.close();
}

void decompress(string input_file, string output_file) {

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
