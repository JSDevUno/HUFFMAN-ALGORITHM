#include <iostream>
#include <fstream>

using namespace std;
struct Node {
    Node* left, * right;
    int freq;
    char c;
    Node() : left(nullptr), right(nullptr), freq(0), c('\0') {}
};

int n_nodes = 0, qend = 1;
Node pool[256] = {{}};
Node* qqq[255], ** q = qqq - 1;
std::string code[128];
char buf[1024];

Node* new_node(int freq, char c, Node* a, Node* b) {
    Node* n = pool + n_nodes++;
    if (freq != 0) {
        n->c = c;
        n->freq = freq;
    } else {
        n->left = a, n->right = b;
        n->freq = a->freq + b->freq;
    }
    return n;
}

void qinsert(Node* n) {
    int j, i = qend++;
    while ((j = i / 2)) {
        if (q[j]->freq <= n->freq) break;
        q[i] = q[j], i = j;
    }
    q[i] = n;
}

Node* qremove() {
    int i, l;
    Node* n = q[i = 1];

    if (qend < 2) return nullptr;
    qend--;
    while ((l = i * 2) < qend) {
        if (l + 1 < qend && q[l + 1]->freq < q[l]->freq) l++;
        q[i] = q[l], i = l;
    }
    q[i] = q[qend];
    return n;
}

void build_code(Node* n, std::string& s) {
    if (n->c != '\0') {
        code[static_cast<unsigned char>(n->c)] = s;
        return;
    }

    s.push_back('0');
    build_code(n->left, s);
    s.pop_back();

    s.push_back('1');
    build_code(n->right, s);
    s.pop_back();
}

void import_file(std::ifstream& fp_in, unsigned int* freq) {
    char c;
    std::string s;
    while (fp_in.get(c) && !fp_in.eof()) {
        freq[static_cast<unsigned char>(c)]++;
    }

    for (int i = 0; i < 128; i++) {
        if (freq[i]) qinsert(new_node(freq[i], static_cast<char>(i), nullptr, nullptr));
    }

    while (qend > 2) qinsert(new_node(0, 0, qremove(), qremove()));

    s.clear();
    build_code(q[1], s);
}

void strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++) != '\0');
}

void strcat(char* dest, const char* src) {
    while (*dest != '\0') {
        dest++;
    }
    while ((*dest++ = *src++) != '\0');
}

size_t strlen(const char* str) {
    const char* s = str;
    while (*s++);
    return s - str - 1;
}

void encode(std::ifstream& fp_in, std::ofstream& fp_out, unsigned int* freq) {
    char in, c = 0;
    int i, j = 0, k = 0, lim = 0;
    fp_in.clear();
    fp_in.seekg(0, std::ios::beg);

    for (i = 0; i < 128; i++) {
        if (freq[i]) lim += (freq[i] * code[i].length());
    }

    int output_data = lim;
    fp_out << "    " << lim << '\n';

    for (i = 0; i < lim; i++) {
        if (buf[j] == '\0') {
            fp_in.get(in);
            strcpy(buf, code[static_cast<unsigned char>(in)].c_str());
            j = 0;
        }

        if (buf[j] == '1')
            c = c | (1 << (7 - k));
        else if (buf[j] == '0')
            c = c | (0 << (7 - k));
        else
            std::cerr << "ERROR: Wrong input!\n";

        k++;
        j++;

        if (((i + 1) % 8 == 0) || (i == lim - 1)) {
            k = 0;
            fp_out.put(c);
            c = 0;
        }
    }
}

void print_code(unsigned int* freq) {
    std::cout << "\n---------CODE TABLE---------\n----------------------------\n";
    std::cout << "---------FREQUENCY---------\n----------------------------\n";
    for (int i = 0; i < 128; i++) {
        if (isprint(static_cast<char>(i)) && !code[i].empty() && i != ' ')
            std::cout << std::left << " " << static_cast<char>(i) << "    " << freq[i] << "                " << code[i] << '\n';
        else if (!code[i].empty()) {
            switch (i) {
                case '\n':
                    std::cout << "\\n    ";
                    break;
                case ' ':
                    std::cout << "\' \'   ";
                    break;
                case '\t':
                    std::cout << "\\t    ";
                    break;
                default:
                   std::cout << std::hex << std::uppercase;
                        if (i < 16)
                            std::cout << "0";
                            std::cout << i << std::dec << "   ";

                    break;
            }
            std::cout << "    " << freq[i] << "                " << code[i] << '\n';
        }
    }
    std::cout << "----------------------------\n";
}

int main(int argc, char* argv[]) {
    std::ifstream fp_in;
    std::ofstream fp_out;

    char file_name[50] = {0};
    unsigned int freq[128] = {0};

    if (argc == 2) {
        strcpy(file_name, argv[1]);
    } else if (argc > 2) {
        std::cerr << "Too many arguments supplied.\n";
        return 0;
    } else {
        std::cout << "Please enter the file to be compressed\t: ";
        std::cin >> file_name;
    }

    if (strlen(file_name) >= 50) {
        std::cerr << "ERROR: Enter a file name less than 50 chars";
        return 0;
    }

    fp_in.open(file_name);
    if (!fp_in.is_open()) {
        std::cerr << "\nERROR: No such file\n";
        return 0;
    }

    import_file(fp_in, freq);
    print_code(freq);

    strcat(file_name, ".huffman");
    fp_out.open(file_name);
    encode(fp_in, fp_out, freq);

    fp_in.close();
    fp_out.close();

    strcat(file_name, ".table");
    fp_out.open(file_name);
    for (int i = 0; i < 128; i++) {
        fp_out.put(static_cast<char>(freq[i]));
    }

    int input_data = 0;
    for (int i = 0; i < 128; i++) input_data += freq[i];

    fp_out.close();
    std::cout << "\nInput bytes:          " << input_data << '\n';

    int output_data = (output_data % 8) ? (output_data / 8) + 1 : (output_data / 8);
    std::cout << "Output bytes:         " << output_data << '\n';

    std::cout << "\nCompression ratio:    " << std::fixed << ((static_cast<double>(input_data - output_data) / input_data) * 100) << "%\n\n\n";

    std::string allBinaryCodes;
    for (int i = 0; i < 128; i++) {
        if (!code[i].empty()) {
            allBinaryCodes += code[i];
        }
    }

    std::cout << "ENCODED: " << allBinaryCodes << '\n';

    return 0;
}
