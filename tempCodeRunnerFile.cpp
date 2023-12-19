#include <iostream>
#include <fstream>
#include <bitset>
using namespace std;

struct NodeData {
    char data;
    int frequency;
    NodeData* left;
    NodeData* right;
    NodeData(char c, int freq) : data(c), frequency(freq), left(nullptr), right(nullptr) {}
};

struct Node {
    NodeData *node; // binary tree pointer
    Node* next;
    Node(NodeData* bn) : node(bn), next(nullptr) {}
};

class PriorityQueue {
private:
    Node* head;

public:
    PriorityQueue() : head(nullptr) {}

    void enqueue(char data, int frequency) {
        NodeData* node = new NodeData(data, frequency);
        Node* newNode = new Node(node);

        if (!head || newNode->node->frequency < head->node->frequency) {
            newNode->next = head;
            head = newNode;
        } else {
            Node* current = head;
            while (current->next && newNode->node->frequency >= current->next->node->frequency) {
                current = current->next;
            }
            newNode->next = current->next;
            current->next = newNode;
        }
        cout << data << " with frequency " << frequency << " successfully enqueued.\n";
    }

    char dequeue() {
        if (!head) {
            cout << "Priority Queue is empty." << endl;
            return '\0';
        }

        char data = head->node->data;
        Node* temp = head;
        head = head->next;
        delete temp;

        return data;
    }

    bool isEmpty() {
        return head == nullptr;
    }

    // Functions related to Huffman Coding
    NodeData* buildHuffmanTree(pair<char, int>* freqList);
    void countFrequency(const string& text, pair<char, int>* freqList);
    void generateCodes(NodeData* root, string code, string* huffmanCodes);
    void compressFile(const string& inputFile, const string& outputFile, string* huffmanCodes);
    void decompressFile(const string& inputFile, const string& outputFile, string* huffmanCodes);
    bool writeToFile(const string& fileName, const string& content, bool binary);
    bool readFromFile(const string& fileName, string& content, bool binary);
    void printHuffmanCodes(string* huffmanCodes);
};

NodeData* PriorityQueue::buildHuffmanTree(pair<char, int>* freqList) {
    for (size_t i = 0; i < 256; ++i) {
        if (freqList[i].second > 0) {
            enqueue(freqList[i].first, freqList[i].second);
        }
    }

    while (head && head->next) {
        Node* left = head;
        Node* right = head->next;

        char data = '\0'; // Placeholder for internal nodes
        int frequency = left->node->frequency + right->node->frequency;

        NodeData* newNode = new NodeData(data, frequency);
        newNode->left = left->node;
        newNode->right = right->node;

        head = head->next->next; // Advance two nodes

        enqueue('\0', frequency); // Enqueue the new internal node
    }

    return head ? head->node : nullptr; // Return the root of the Huffman tree
}

void PriorityQueue::generateCodes(NodeData* root, string code, string* huffmanCodes) {
    if (root == nullptr) return;
    if (root->data != '\0') {
        huffmanCodes[static_cast<unsigned char>(root->data)] = code;
    }
    generateCodes(root->left, code + "0", huffmanCodes);
    generateCodes(root->right, code + "1", huffmanCodes);
}

void PriorityQueue::printHuffmanCodes(string* huffmanCodes) {
    for (int i = 0; i < 256; ++i) {
        if (!huffmanCodes[i].empty()) {
            cout << static_cast<char>(i) << ": " << huffmanCodes[i] << endl;
        }
    }
}

bool PriorityQueue::writeToFile(const string& fileName, const string& content, bool binary) {
    ofstream file(fileName, binary ? ios::out | ios::binary : ios::out);
    if (!file.is_open()) {
        cerr << "Unable to create the file: " << fileName << endl;
        return false;
    }

    if (binary) {
        for (size_t i = 0; i < content.size(); i += 8) {
            char byte = 0;
            for (int j = 0; j < 8 && i + j < content.size(); ++j) {
                byte = (byte << 1) | (content[i + j] - '0');
            }
            file.put(byte);
        }
    } else {
        file << content;
    }

    file.close();
    return true;
}

bool PriorityQueue::readFromFile(const string& fileName, string& content, bool binary) {
    ifstream file(fileName, binary ? ios::binary : ios::in);
    if (!file.is_open()) {
        cerr << "Unable to open the file: " << fileName << endl;
        return false;
    }

    if (binary) {
        char byte;
        while (file.get(byte)) {
            for (int j = 7; j >= 0; --j) {
                content += ((byte >> j) & 1) + '0';
            }
        }
    } else {
        content.assign((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    }

    file.close();
    return true;
}

void PriorityQueue::countFrequency(const string& text, pair<char, int>* freqList) {
    for (char c : text) {
        unsigned char index = static_cast<unsigned char>(c);
        freqList[index].first = c;
        freqList[index].second++;
    }
}

void PriorityQueue::compressFile(const string& inputFile, const string& outputFile, string* huffmanCodes) {
    string text;
    if (!readFromFile(inputFile, text, false)) {
        cerr << "Compression failed: Unable to read input file." << endl;
        return;
    }

    if (text.empty()) {
        cerr << "The input file is empty." << endl;
        return;
    }

    pair<char, int> freqList[256] = {};
    countFrequency(text, freqList);

    NodeData* root = buildHuffmanTree(freqList);

    string compressedData;
    for (char c : text) {
        compressedData += huffmanCodes[static_cast<unsigned char>(c)];
    }

    // Convert the compressed string to bytes
    string byte;
    for (char bit : compressedData) {
        byte += bit;
        if (byte.length() == 8) {
            unsigned char byteValue = bitset<8>(byte).to_ulong();
            compressedData += byteValue;  // Append the byte representation to the string
            byte.clear();
        }
    }

    // If there are remaining bits, pad them to form a complete byte
    if (!byte.empty()) {
        while (byte.length() < 8) {
            byte += '0';
        }
        unsigned char byteValue = bitset<8>(byte).to_ulong();
        compressedData += byteValue;  // Append the byte representation to the string
    }

    // Ensure the string is properly resized to remove unnecessary characters
    compressedData.resize((compressedData.length() + 7) / 8);

    if (!writeToFile(outputFile, compressedData, true)) {
        cerr << "Compression failed: Unable to write output file." << endl;
        return;
    }

    cout << "File compressed successfully: " << outputFile << endl;
}

void PriorityQueue::decompressFile(const string& inputFile, const string& outputFile, string* huffmanCodes) {
    string compressedText;
    if (!readFromFile(inputFile, compressedText, true)) {
        cerr << "Decompression failed: Unable to read input file." << endl;
        return;
    }

    string code;
    string decompressedData;

    for (char byte : compressedText) {
        bitset<8> bits(byte);
        for (int j = 7; j >= 0; --j) {
            code += bits[j] + '0';
            for (size_t i = 0; i < 256; ++i) {
                if (code == huffmanCodes[i]) {
                    decompressedData += static_cast<char>(i);
                    code = "";
                    break;
                }
            }
        }
    }

    if (!writeToFile(outputFile, decompressedData, false)) {
        cerr << "Decompression failed: Unable to write output file." << endl;
        return;
    }

    cout << "File decompressed successfully: " << outputFile << endl;
}

int main() {
    PriorityQueue pq;

    string compressFileName, decompressFileName;

    cout << "Enter the name of the file to compress: ";
    cin >> compressFileName;

    string compressedFileName = compressFileName.substr(0, compressFileName.find_last_of('.')) + "_compressed.bin";
    string huffmanCodes[256] = {};

    pq.compressFile(compressFileName, compressedFileName, huffmanCodes);

    if (!compressedFileName.empty()) {
        pq.printHuffmanCodes(huffmanCodes);

        cout << "Enter the name of the file to decompress (" << compressedFileName << "): ";
        cin >> decompressFileName;

        if (!decompressFileName.empty()) {
            decompressFileName = decompressFileName.substr(0, decompressFileName.find_last_of('.')) + "_decompressed.txt";
            pq.decompressFile(compressedFileName, decompressFileName, huffmanCodes);
        }
    }

    return 0;
}
