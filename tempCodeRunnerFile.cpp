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
            unsigned char byteValue = static_cast<unsigned char>(stoi(byte, nullptr, 2));
            compressedData.push_back(byteValue);
            byte.clear();
        }
    }

    // If there are remaining bits, pad them to form a complete byte
    if (!byte.empty()) {
        while (byte.length() < 8) {
            byte += '0';
        }
        unsigned char byteValue = static_cast<unsigned char>(stoi(byte, nullptr, 2));
        compressedData.push_back(byteValue);
    }

    if (!writeToFile(outputFile, compressedData, true)) {
        cerr << "Compression failed: Unable to write output file." << endl;
        return;
    }

    cout << "File compressed successfully: " << outputFile << endl;
}

void PriorityQueue::decompressFile(const string& inputFile, const string& outputFile, string* huffmanCodes) {
    string compressedText;
    if (!readFromFile(inputFile, compressedText, false)) {
        cerr << "Decompression failed: Unable to read input file." << endl;
        return;
    }

    string code;
    string decompressedData;

    for (char bit : compressedText) {
        code += bit;
        for (size_t i = 0; i < 256; ++i) {
            if (code == huffmanCodes[i]) {
                decompressedData += static_cast<char>(i);
                code = "";
                break;
            }
        }
    }

    if (!writeToFile(outputFile, decompressedData, false)) {
        cerr << "Decompression failed: Unable to write output file." << endl;
        return;
    }

    cout << "File decompressed successfully: " << outputFile << endl;
}
