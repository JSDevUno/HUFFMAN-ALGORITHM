#include <iostream>
#include <fstream>
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
    bool writeToFile(const string& fileName, const string& content);
    bool readFromFile(const string& fileName, string& content);
};

void PriorityQueue::countFrequency(const string& text, pair<char, int>* freqList) {
    for (char c : text) {
        unsigned char index = static_cast<unsigned char>(c);
        freqList[index].first = c;
        freqList[index].second++;
    }
}

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

bool PriorityQueue::writeToFile(const string& fileName, const string& content) {
    ofstream file(fileName, ios::binary);
    if (!file.is_open()) {
        cerr << "Unable to create the file: " << fileName << endl;
        return false;
    }

    file.write(content.c_str(), content.size());
    file.close();
    return true;
}

bool PriorityQueue::readFromFile(const string& fileName, string& content) {
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

void PriorityQueue::compressFile(const string& inputFile, const string& outputFile, string* huffmanCodes) {
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

    NodeData* root = buildHuffmanTree(freqList);

    string encodedBinary;
    string currentByte;
    for (char c : text) {
        string code = huffmanCodes[static_cast<unsigned char>(c)];
        for (char bit : code) {
            currentByte += bit;
            if (currentByte.length() == 8) {
                char ch = static_cast<char>(stoi(currentByte, nullptr, 2));
                encodedBinary += ch;
                currentByte.clear();
            }
        }
    }

    if (!currentByte.empty()) {
        while (currentByte.length() < 8) {
            currentByte += '0';
        }
        char ch = static_cast<char>(stoi(currentByte, nullptr, 2));
        encodedBinary += ch;
    }

    if (writeToFile(outputFile, encodedBinary)) {
        cout << "File compressed successfully: " << outputFile << endl;
    } else {
        cerr << "Compression failed: Unable to write output file." << endl;
    }
}

void PriorityQueue::decompressFile(const string& inputFile, const string& outputFile, string* huffmanCodes) {
    string compressedText;
    if (!readFromFile(inputFile, compressedText)) {
        cerr << "Decompression failed: Unable to read input file." << endl;
        return;
    }

    string binaryString;
    for (char c : compressedText) {
        for (int i = 7; i >= 0; --i) {
            binaryString += ((c >> i) & 1) + '0';
        }
    }

    string code;
    string decompressedData;
    for (char bit : binaryString) {
        code += bit;
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
    PriorityQueue pq;

    string compressFileName, decompressFileName;

    cout << "Search file to compress: ";
    cin >> compressFileName;

    string compressedFileName = compressFileName.substr(0, compressFileName.find_last_of('.')) + "_compressed.txt";
    string huffmanCodes[256] = {};

    pq.compressFile(compressFileName, compressedFileName, huffmanCodes);

    if (!compressedFileName.empty()) {
        cout << "Search file to decompress (" << compressedFileName << "): ";
        cin >> decompressFileName;

        if (!decompressFileName.empty()) {
            decompressFileName = decompressFileName.substr(0, decompressFileName.find_last_of('.')) + "_decompressed.txt";
            pq.decompressFile(compressedFileName, decompressFileName, huffmanCodes);
        }
    }

    return 0;
}
