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

    void enqueue(NodeData* node) {
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

        // Print the frequency of the enqueued data
        cout << "Enqueued: Data = " << newNode->node->data << ", Frequency = " << newNode->node->frequency << endl;
    }

    NodeData* dequeue() {
        if (!head) {
            cout << "Priority Queue is empty." << endl;
            return nullptr;
        }

        NodeData* data = head->node;
        Node* temp = head;
        head = head->next;
        delete temp;

        return data;
    }

    bool isEmpty() {
        return head == nullptr;
    }
};

void countFrequency(const string& text, pair<char, int>* freqList) {
    for (char c : text) {
        unsigned char index = static_cast<unsigned char>(c);
        freqList[index].first = c;
        freqList[index].second++;
    }
}

NodeData* buildHuffmanTree(pair<char, int>* freqList) {
    PriorityQueue pq;
    for (size_t i = 0; i < 256; ++i) {
        if (freqList[i].second > 0) {
            pq.enqueue(new NodeData(freqList[i].first, freqList[i].second));
        }
    }

    while (!pq.isEmpty()) {
        NodeData* left = pq.dequeue();

        // Handle the case when there is only one node left in the priority queue
        if (pq.isEmpty()) {
            return left;
        }

        NodeData* right = pq.dequeue();

        NodeData* newNode = new NodeData('$', left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;
        pq.enqueue(newNode);
    }

    // The priority queue is empty
    return nullptr;
}

void generateCodes(NodeData* root, string code, string* huffmanCodes) {
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

    NodeData* root = buildHuffmanTree(freqList);

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
