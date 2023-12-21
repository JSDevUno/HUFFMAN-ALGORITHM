#include <iostream>
#include <fstream>
using namespace std;
// NodeData and Node Structs
// NodeData holds information about something, and Node connects to other Nodes.
struct NodeData {
    char data;
    int frequency;
    NodeData* left;
    NodeData* right;
    NodeData(char c, int freq) : data(c), frequency(freq), left(nullptr), right(nullptr) {}
};

struct Node {
    NodeData *node; 
    Node* next;
    Node(NodeData* bn) : node(bn), next(nullptr) {}
};
// PriorityQueue Class
// This class organizes boxes in a line, each with a number, and keeps them sorted.
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
// countFrequency Function
// This function counts how many times each letter appears in a text.
void countFrequency(const string& text, pair<char, int>* freqList) {
    for (char c : text) {
        unsigned char index = static_cast<unsigned char>(c);
        freqList[index].first = c;
        freqList[index].second++;
    }
}
// buildHuffmanTree Function
// This function builds a family tree for letters, figuring out which ones are related.
NodeData* buildHuffmanTree(pair<char, int>* freqList) {
    PriorityQueue pq;
    for (size_t i = 0; i < 256; ++i) {
        if (freqList[i].second > 0) {
            pq.enqueue(new NodeData(freqList[i].first, freqList[i].second));
        }
    }

    while (!pq.isEmpty()) {
        NodeData* left = pq.dequeue();

        if (pq.isEmpty()) {
            return left;
        }

        NodeData* right = pq.dequeue();

        NodeData* newNode = new NodeData('$', left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;
        pq.enqueue(newNode);
    }

    return nullptr;
}
// generateCodes Function
// This function gives each family member (letter) a secret code.

/*Ginagamit ang dollar sign ($) bilang special characters para kumatawan sa mga internal na node sa Huffman tree.
 Yung dollar sign ay yung parang tanda lang na nagsasabing ito ay hindi regular na character kundi isang part ng tree na nagsisilbing tanda para sa mga internal parts ng Huffman tree. 
 tumutulong to sa tamang pagbuo ng tree kapag nangyayare ang decoding.*/
void generateCodes(NodeData* root, string code, string* huffmanCodes) {
    if (root == nullptr) return;
    if (root->data != '$') {
        huffmanCodes[static_cast<unsigned char>(root->data)] = code;
    }
    generateCodes(root->left, code + "0", huffmanCodes);
    generateCodes(root->right, code + "1", huffmanCodes);
}
// writeToFile Function
// This function writes a secret message (compressed data) on text (file).
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
// readFromFile Function
// This function opens the txt file and reads the secret message from the paper inside.
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
// compressFile Function
// This function compress a text file into a smaller version using the secret custom codes.
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
// decompressFile Function
// This function decompress the file, turning the compressed version back into the original using the secret custom codes.
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
