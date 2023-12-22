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
char buf[1024];

Node* new_node(int freq, char c, Node* a, Node* b) {
    Node* n = pool + n_nodes++;
    if (freq != 0) {
        n->c = c;
        n->freq = freq;
    }
    else {
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

void import_table(ifstream& fp_table, unsigned int* freq) {
    char c;
    int i = 0;

    while (fp_table.get(c) && !fp_table.eof()) {
        freq[i++] = static_cast<unsigned char>(c);
    }

    for (i = 0; i < 128; i++) {
        if (freq[i]) qinsert(new_node(freq[i], static_cast<char>(i), nullptr, nullptr));
    }

    while (qend > 2) qinsert(new_node(0, 0, qremove(), qremove()));
}

void decode(ifstream& fp_huffman, ofstream& fp_out) {
    int i = 0, lim = 0, j = 0;
    char c;
    Node* n = q[1];

    fp_huffman >> lim;
    fp_huffman.ignore();

    for (i = 0; i < lim; i++) {
        if (j == 0)
            fp_huffman.get(c);

        if (c & 128) n = n->right;
        else n = n->left;

        if (n->c) {
            putchar(n->c);
            fp_out.put(n->c);
            n = q[1];
        }

        c = c << 1;
        if (++j > 7)
            j = 0;
    }

    putchar('\n');
    if (q[1] != n) cout << "Garbage input\n";
}

char* custom_strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++) != '\0') {}
    return original_dest;
}

char* custom_strstr(const char* haystack, const char* needle) {
    while (*haystack != '\0') {
        const char* h = haystack;
        const char* n = needle;

        while (*n != '\0' && *h == *n) {
            h++;
            n++;
        }

        if (*n == '\0') {
            return const_cast<char*>(haystack);
        }

        haystack++;
    }

    return nullptr;
}

char* custom_strcat(char* dest, const char* src) {
    char* original_dest = dest;
    while (*dest != '\0') {
        dest++;
    }
    while ((*dest++ = *src++) != '\0') {}
    return original_dest;
}

int main(int argc, char* argv[]) {
    ifstream fp_table, fp_huffman;
    ofstream fp_out;

    char file_name[50] = { 0 }, temp[50] = { 0 };
    unsigned int freq[128] = { 0 };

    if (argc == 2) {
        custom_strcpy(file_name, argv[1]);
        if (custom_strstr(file_name, "huffman") == nullptr) {
            cerr << "\nERROR: wrong file format!\n";
            return 0;
        }
    }
    else if (argc > 2) {
        cerr << "Too many arguments supplied.\n";
        return 0;
    }
    else {
        cout << "Please enter the file to be decompressed: ";
        cin >> file_name;
        if (custom_strstr(file_name, "huffman") == nullptr) {
            cerr << "\nERROR: wrong file format!\n";
            return 0;
        }
    }

    fp_huffman.open(file_name);
    if (!fp_huffman.is_open()) {
        cerr << "\nERROR: No such file\n";
        return 0;
    }

    custom_strcat(file_name, ".table");
    fp_table.open(file_name);
    if (!fp_table.is_open()) {
        cerr << "\nERROR: Frequency table cannot be found\n";
        return 0;
    }

    import_table(fp_table, freq);

    *custom_strstr(file_name, ".huffman") = '\0';
    custom_strcpy(temp, "mkdir ");
    custom_strcat(temp, file_name);
    system(custom_strcat(temp, ".decoded"));

    custom_strcpy(temp, "./");
    custom_strcat(temp, file_name);
    custom_strcat(temp, ".decoded/");
    fp_out.open(custom_strcat(temp, file_name));
    if (!fp_out.is_open()) {
        cerr << "ERROR: Creating decoded file failed\n";
        return 0;
    }

    decode(fp_huffman, fp_out);

    fp_huffman.close();
    fp_table.close();
    fp_out.close();
    return 0;
}
