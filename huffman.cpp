#include <iostream>
#include <fstream>

using namespace std;

struct HuffmanNode {
    char data;
    int frequency;
    HuffmanNode* left;
    HuffmanNode* right;
    HuffmanNode(char c, int freq) : data(c), frequency(freq), left(nullptr), right(nullptr) {}
};

class PriorityQueue {
private:
    HuffmanNode** nodes;
    size_t capacity;
    size_t size;

public:
    PriorityQueue(size_t initialCapacity = 100) : capacity(initialCapacity), size(0) {
        nodes = new HuffmanNode*[capacity];
    }

    ~PriorityQueue() {
        delete[] nodes;
    }

    void enqueue(HuffmanNode* node) {
        if (size >= capacity) {
            resize();
        }
        nodes[size++] = node;
        int i = size - 1;
        while (i > 0 && nodes[(i - 1) / 2]->frequency > nodes[i]->frequency) {
            swap(nodes[i], nodes[(i - 1) / 2]);
            i = (i - 1) / 2;
        }
    }

    HuffmanNode* dequeue() {
        if (isEmpty()) return nullptr;

        HuffmanNode* root = nodes[0];
        nodes[0] = nodes[size - 1];
        size--;

        int i = 0;
        while (true) {
            int smallest = i;
            int left = 2 * i + 1;
            int right = 2 * i + 2;

            if (left < size && nodes[left]->frequency < nodes[smallest]->frequency) {
                smallest = left;
            }

            if (right < size && nodes[right]->frequency < nodes[smallest]->frequency) {
                smallest = right;
            }

            if (smallest != i) {
                swap(nodes[i], nodes[smallest]);
                i = smallest;
            } else {
                break;
            }
        }

        return root;
    }

    bool isEmpty() {
        return size == 0;
    }

    void resize() {
        capacity *= 2;
        HuffmanNode** newNodes = new HuffmanNode*[capacity];
        for (size_t i = 0; i < size; ++i) {
            newNodes[i] = nodes[i];
        }
        delete[] nodes;
        nodes = newNodes;
    }

    size_t getNodeSize() const {
        return size;
    }
};

void countFrequency(const string& text, pair<char, int>* freqList) {
    for (char c : text) {
        unsigned char index = static_cast<unsigned char>(c);
        freqList[index].first = c;
        freqList[index].second++;
    }
}

HuffmanNode* buildHuffmanTree(pair<char, int>* freqList) {
    PriorityQueue pq(256);
    for (size_t i = 0; i < 256; ++i) {
        if (freqList[i].second > 0) {
            pq.enqueue(new HuffmanNode(freqList[i].first, freqList[i].second));
        }
    }

    while (pq.getNodeSize() > 1) {
        HuffmanNode* left = pq.dequeue();
        HuffmanNode* right = pq.dequeue();

        HuffmanNode* newNode = new HuffmanNode('$', left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;
        pq.enqueue(newNode);
    }

    return pq.dequeue();
}

void generateCodes(HuffmanNode* root, string code, string* huffmanCodes) {
    if (root == nullptr) return;
    if (root->data != '$') {
        huffmanCodes[static_cast<unsigned char>(root->data)] = code;
    }
    generateCodes(root->left, code + "0", huffmanCodes);
    generateCodes(root->right, code + "1", huffmanCodes);
}

bool writeToFile(const string& fileName, const string& content) {
    ofstream file(fileName, ios::binary);
    if (!file.is_open()) {
        cerr << "Unable to create the file: " << fileName << endl;
        return false;
    }

    file.write(content.c_str(), content.size());
    file.close();
    return true;
}

bool readFromFile(const string& fileName, string& content) {
    ifstream file(fileName, ios::binary);
    if (!file.is_open()) {
        cerr << "Unable to open the file: " << fileName << endl;
        return false;
    }

    file.seekg(0, ios::end);
    content.resize(file.tellg());
    file.seekg(0, ios::beg);
    file.read(&content[0], content.size());
    file.close();
    return true;
}

void compressFile(const string& inputFile, const string& outputFile, string* huffmanCodes) {
    string text;
    if (!readFromFile(inputFile, text)) {
        cerr << "Compression failed: Unable to read input file." << endl;
        return;
    }

    if (text.empty()) {
        cerr << "The input file is empty." << endl;
        return;
    }

    pair<char, int> freqList[256] = {}; 
    countFrequency(text, freqList);

    HuffmanNode* root = buildHuffmanTree(freqList);

    generateCodes(root, "", huffmanCodes);

    ofstream file(outputFile, ios::binary);
    if (!file.is_open()) {
        cerr << "Compression failed: Unable to write output file." << endl;
        return;
    }

    string compressedData;
    for (char c : text) {
        compressedData += huffmanCodes[static_cast<unsigned char>(c)];
    }

    file << compressedData;

    file.close();
    cout << "File compressed successfully: " << outputFile << endl;
}


void decompressFile(const string& inputFile, const string& outputFile, string* huffmanCodes) {
    string compressedText;
    if (!readFromFile(inputFile, compressedText)) {
        cerr << "Decompression failed: Unable to read input file." << endl;
        return;
    }

    string code;
    string decompressedData;

    for (char c : compressedText) {
        code += c;
        for (size_t i = 0; i < 256; ++i) {
            if (code == huffmanCodes[i]) {
                decompressedData += static_cast<char>(i);
                code = "";
                break;
            }
        }
    }

    if (writeToFile(outputFile, decompressedData)) {
        cout << "File decompressed successfully: " << outputFile << endl;
    } else {
        cerr << "Decompression failed: Unable to write output file." << endl;
    }
}

int main() {
    string compressFileName, decompressFileName;

    cout << "Search file to compress: ";
    cin >> compressFileName;

    string compressedFileName = compressFileName.substr(0, compressFileName.find_last_of('.')) + "_compressed.txt";
    string huffmanCodes[256] = {}; 

    compressFile(compressFileName, compressedFileName, huffmanCodes);
    
    if (!compressedFileName.empty()) {
        cout << "Search file to decompress (" << compressedFileName << "): ";
        cin >> decompressFileName;

        if (!decompressFileName.empty()) {
            decompressFileName = decompressFileName.substr(0, decompressFileName.find_last_of('.')) + "_decompressed.txt";
            decompressFile(compressedFileName, decompressFileName, huffmanCodes);
        }
    }

    return 0;
}
