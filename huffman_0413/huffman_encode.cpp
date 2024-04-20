#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <bitset>

#include <map>
#include <queue>
#include <algorithm>

using namespace std;

// 节点结构
struct HuffmanNode {
    unsigned char value;
    double probability;
    HuffmanNode* left;
    HuffmanNode* right;

    // 构造函数
    HuffmanNode(unsigned char v, double p) : value(v), probability(p), left(nullptr), right(nullptr) {}
    HuffmanNode(unsigned char v, double p, HuffmanNode* l, HuffmanNode* r) : value(v), probability(p), left(l), right(r) {}

    // compare operator for priority queue
    bool operator>(const HuffmanNode& other) const {
        return probability > other.probability;
    }
};

struct Bitstream {
    map<unsigned char, vector<bool>> huffmanTable;
    string bitstring;

    Bitstream(map<unsigned char, vector<bool>> h, string b) : huffmanTable(h), bitstring(b) {}
};

vector<unsigned char> read_raw_img(string file_name);
void print_entropy(map<unsigned char, double> probability_map);
map<unsigned char, double> get_probability_map(vector<unsigned char> buffer);
vector<unsigned char> get_dpcm_img(vector<unsigned char> original_img);

//huffman
//void write_binary_huff(string filename, string bitstream);
HuffmanNode* buildHuffmanTree(const map<unsigned char, double>& probabilities);
void buildHuffmanTable(HuffmanNode* root, vector<bool>& code, map<unsigned char, vector<bool>>& huffmanTable);

void write_binary_huff(string filename, map<unsigned char, vector<bool>> huffmanTable, string bitstream);
Bitstream read_binary_huff(string huff_file_name);
//void write_char_huff(string filename, string bitstream);

string huff_encode(vector<unsigned char> original_img, map<unsigned char, vector<bool>> huffmanTable);
void print_huffmanTable(map<unsigned char, vector<bool>> huffmanTable);
void huffman(vector<unsigned char> original_img, map<unsigned char, double> probability_map, string img_name, string process_type, bool is_dpcm);


int main() {
    int choice;
    string img_name;
    string process_type;
    string data_path = "./Data/RAW/";

    cout << "lena: 1" << endl;
    cout << "baboon: 2" << endl;
    cout << "Please choose the image: ";
    cin >> choice;
    cout << endl;
    //choice = 1;

    if (choice == 1) {
        img_name = "lena";
    }
    else if (choice == 2) {
        img_name = "baboon";
    }
    else {
        cerr << "Image doesn't exist!" << endl;
        return 1;
    }

    cout << "gray: 1" << endl;
    cout << "halftone: 2" << endl;
    cout << "binary: 3" << endl;
    cout << "Please choose the color type: ";
    cin >> choice;
    cout << endl;
    //choice = 1;

    if (choice == 1) {
        process_type = "";
    }
    else if (choice == 2) {
        process_type = "_halftone";
    }
    else if (choice == 3) {
        process_type = "_b";
    }
    else {
        cerr << "Image doesn't exist!" << endl;
        return 1;
    }

    // 根據使用者選擇的檔案名稱和處理方式執行相應的處理
    string file_name = data_path + img_name + process_type + ".raw";
    cout << file_name << " Loading..." << endl;

    vector<unsigned char> original_img = read_raw_img(file_name);

    map<unsigned char, double> probability_map = get_probability_map(original_img);

    vector<unsigned char> dpcm_img = get_dpcm_img(original_img);
    map<unsigned char, double> dpcm_probability_map = get_probability_map(dpcm_img);
    cout << endl;

    huffman(original_img, probability_map, img_name, process_type, false);
    huffman(dpcm_img, dpcm_probability_map, img_name, process_type, true);

    return 0;
}

// 建立 Huffman 树
HuffmanNode* buildHuffmanTree(const map<unsigned char, double>& probabilities) {
    // the node with smallest probability will be the first node
    priority_queue<HuffmanNode, vector<HuffmanNode>, greater<HuffmanNode>> pq;

    // 将每个灰度值作为一个叶子节点加入优先队列
    for (const auto& pair : probabilities) {
        //pair.first: color value
        //pair.second: probability 
        pq.push(HuffmanNode(pair.first, pair.second));
    }

    // 从优先队列中构建 Huffman 树
    HuffmanNode* root = nullptr;
    while (pq.size() > 1) {
        HuffmanNode* left = new HuffmanNode(pq.top().value, pq.top().probability, pq.top().left, pq.top().right);
        //cout << "pq.top().probability: " << pq.top().probability << endl;
        pq.pop();
        HuffmanNode* right = new HuffmanNode(pq.top().value, pq.top().probability, pq.top().left, pq.top().right);
        //cout << "pq.top().probability: " << pq.top().probability << endl;
        pq.pop();

        // 新节点的概率为左右子节点概率之和
        double combinedProbability = left->probability + right->probability;
        HuffmanNode* parent = new HuffmanNode(0, combinedProbability);
        parent->left = left;
        parent->right = right;
        pq.push(*parent);
        //cout << "pq size: " << pq.size() << endl;
        if (pq.size() == 1) {
            root = parent;
        }

    }

    // 返回 Huffman 树的根节点
    return root;
}

// 从 Huffman 树构建 Huffman 表
void buildHuffmanTable(HuffmanNode* root, vector<bool>& code, map<unsigned char, vector<bool>>& huffmanTable) {
    if (root == nullptr) return;

    //leaf node
    if (root->left == nullptr && root->right == nullptr) {
        huffmanTable[root->value] = code;
        //print binary code
        /*for (bool bit : code) {
            cout << bit;
        }*/
    }
    //internal node
    else {
        if (root->left != nullptr) {
            code.push_back(false);
            buildHuffmanTable(root->left, code, huffmanTable);
            code.pop_back();
        }
        if (root->right != nullptr) {
            code.push_back(true);
            buildHuffmanTable(root->right, code, huffmanTable);
            code.pop_back();
        }
    }
}


void huffman(vector<unsigned char> original_img, map<unsigned char, double> probability_map, string img_name, string process_type, bool is_dpcm) {
    // 构建 Huffman 树
    HuffmanNode* root = buildHuffmanTree(probability_map);
    //cout << root->left->probability << endl;
    //cout << root->right->probability << endl;
    //cout << root->left->left->probability << endl;

    // 构建 Huffman 表
    vector<bool> code;
    map<unsigned char, vector<bool>> huffmanTable;
    buildHuffmanTable(root, code, huffmanTable);
    //print_huffmanTable(huffmanTable);

    string bitstring = huff_encode(original_img, huffmanTable);

    size_t uncompressed_file_size = (original_img).size(); // 65536
    size_t compressed_file_size = bitstring.length() / 8; // 499680 / 8 = 62460

    int uncompressed_file_kb = roundf(float(uncompressed_file_size) /1024); //64KB
    int compressed_file_kb = roundf(float(compressed_file_size) / 1024); //60KB

    string img_type = (is_dpcm) ? "dpcm":"original";
    cout << "=========== " << img_type << " " << img_name + process_type << " =============\n";
    printf("Your original file size was %zd Bytes (= %d KB). \n", uncompressed_file_size, uncompressed_file_kb);
    printf("The compressed size is: %zd Bytes (= %d KB).\n", compressed_file_size, compressed_file_kb);
    printf("Space saving: %0.2f%% \n", float(uncompressed_file_kb-compressed_file_kb)/float(uncompressed_file_kb)*100);
    
    print_entropy(probability_map);
    printf("Huffman encode: %0.2f bits/symbol\n", round(bitstring.length() / uncompressed_file_size));

    string huff_file_name;
    if (!is_dpcm){
        huff_file_name = img_name + process_type + ".huff";
    }
    else {
        huff_file_name = img_name + process_type + "_dpcm.huff";
    }

    // combine huffman table and bitstring

    //write_char_huff(huff_file_name, bitstring);
    write_binary_huff(huff_file_name, huffmanTable, bitstring);


    //Decoder
    printf("Decoding.......\n");

    //access huffman table from bitstream
    Bitstream compress_bitstream = read_binary_huff(huff_file_name);

    string compress_bitstring = compress_bitstream.bitstring;
    vector<unsigned char> uncompressed_img;
    vector<bool> binary_code;

    // build a new table equals to map<code, color value>
    map<vector<bool>, unsigned char> invertedHuffMap;
    for (const auto& pair : compress_bitstream.huffmanTable) {
        invertedHuffMap[pair.second] = pair.first;
    }

    for (char digit : compress_bitstring) {
        bool digit_bool= (digit == '0') ? false : true ;
        binary_code.push_back(digit_bool);
        size_t pos = 0;
        //code += digit;
        auto letter = invertedHuffMap.find(binary_code);
        if (letter != invertedHuffMap.end()) {
            uncompressed_img.push_back(letter->second);
            binary_code.clear();
        }
    }

    // 输出解压后的字符串
    if (uncompressed_img == original_img) {
        cout << "Uncompressed img equals to the original img\n" << endl;
    }
    else {
        cout << "Uncompressed img doesn't equals to the original img\n" << endl;
    }
}

void print_huffmanTable(map<unsigned char, vector<bool>> huffmanTable) {
    //print huffman table
    cout << "huffmantable.size: " << huffmanTable.size() << endl;
    for (const auto& pair : huffmanTable) {
        //pair.first: color value
        //pair.second: binary code
        cout << "color: " << static_cast<int>(pair.first) << ", huffman code: ";
        for (bool bit : pair.second) {
            cout << bit;
        }
        cout << endl;
    }
}


string huff_encode(vector<unsigned char> original_img, map<unsigned char, vector<bool>> huffmanTable) {
    // 重新编码图像
    string bitstream = "";
    for (unsigned char pixel : original_img) {
        for (bool bit : huffmanTable[pixel]) {
            //cout << int(bit);
            bitstream += to_string(bit);
        }
        //cout << " ";
    }
    //cout << "new bitstream：" << endl;
    //cout << bitstream <<endl<<endl;
    return bitstream;
}

void write_binary_huff(string filename, map<unsigned char, vector<bool>> huffmanTable, string bitstream) {
    // write bitstream as bitset type
    size_t strlen = bitstream.length();
    //cout << strlen << endl << endl;
    size_t mapSize = huffmanTable.size();
    
    ofstream outputFile(filename, ios::binary);
    if (outputFile.is_open()) {
        // Write the size of the map
        outputFile.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));

        // Write each pair of the map
        for (const auto& pair : huffmanTable) {
            // Write the unsigned char key
            outputFile.write(reinterpret_cast<const char*>(&pair.first), sizeof(pair.first));

            // Write the size of the vector<bool>
            size_t vecSize = pair.second.size();
            outputFile.write(reinterpret_cast<const char*>(&vecSize), sizeof(vecSize));

            // Write the elements of the vector<bool>
            for (bool b : pair.second) {
                outputFile.write(reinterpret_cast<const char*>(&b), sizeof(b));
            }
        }

        //write bitstream length
        outputFile.write(reinterpret_cast<const char*>(&strlen), sizeof(size_t));

        // create a buffer to save bit
        bitset<32> buffer;
        size_t bufferIndex = 0; //size_t = unsigned int32 = 32 bits

        // convert to binary
        for (char c : bitstream) {
            if (c == '1') {
                buffer.set(bufferIndex);
            }
            ++bufferIndex;
            if (bufferIndex == 32) { // write to file when buffer is full
                outputFile.write(reinterpret_cast<const char*>(&buffer), sizeof(buffer));
                buffer.reset();
                bufferIndex = 0;
            }
        }
        // write to file if there's still data in buffer
        if (bufferIndex > 0) {
            outputFile.write(reinterpret_cast<const char*>(&buffer), sizeof(buffer));
        }
        outputFile.close();
        cout << "Binary data has been written to file.\n" << endl;
    }
    else {
        cerr << "Unable to open file." << endl;
    }
}

void write_char_huff(string filename, string bitstream) {
    ofstream outputFile(filename, ios::binary);
    // write bitstream as char type
    if (outputFile.is_open()) {
        outputFile << bitstream;
        outputFile.close();
    }
    else {
        cerr << "Unable to open file." << endl;
    }
}


Bitstream read_binary_huff(string filename) {
    //read binary file
    ifstream huffFile(filename, ios::in | ios::binary);

    string compressed_img_str = "";
    map<unsigned char, vector<bool>> huffmanTable;
    size_t mapSize;

    
    // 逐字节读取文件内容
    if (huffFile.is_open()) {
        huffFile.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

        for (size_t i = 0; i < mapSize; ++i) {
            unsigned char key;
            huffFile.read(reinterpret_cast<char*>(&key), sizeof(key));

            size_t vecSize;
            huffFile.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));

            vector<bool> vec;
            for (size_t j = 0; j < vecSize; ++j) {
                bool b;
                huffFile.read(reinterpret_cast<char*>(&b), sizeof(b));
                vec.push_back(b);
            }

            huffmanTable[key] = vec;
        }

        bitset<32> bits;  //62KB
        size_t lastNonZero;
        huffFile.read(reinterpret_cast<char*>(&lastNonZero), sizeof(size_t));
        while (huffFile.read(reinterpret_cast<char*>(&bits), sizeof(bits))) {
            // convert to char
            for (size_t i = 0; i < 32; ++i) {
                if (bits.test(i)) {
                    compressed_img_str += '1';
                }
                else {
                    compressed_img_str += '0';
                }
            }
        }
        huffFile.close();

        // 去除末尾的多余零
        if (lastNonZero != string::npos) {
            compressed_img_str = compressed_img_str.substr(0, lastNonZero);
        }
        else {
            compressed_img_str.clear(); // 如果全是0，则清空字符串
        }
        //cout << "Decoded string: " << compressed_img_str << endl;
        cout << "Read " << compressed_img_str.length() << " bits." << endl;
        Bitstream bitstream = Bitstream(huffmanTable, compressed_img_str);

        huffFile.close();
        return bitstream;
    }
    else {
        cerr << "Unable to open file." << endl;
    }
}

vector<unsigned char> read_raw_img(string file_name) {
    ifstream rawFile(file_name, ios::in | ios::binary);

    // 獲取檔案大小
    rawFile.seekg(0, ios::end);
    streampos fileSize = rawFile.tellg();
    rawFile.seekg(0, ios::beg);

    // 讀取所有數據到一個緩衝區
    vector<unsigned char> original_img(fileSize);
    rawFile.read(reinterpret_cast<char*>(original_img.data()), fileSize);

    cout << "已成功讀取 " << fileSize << " 個字節的數據" << endl << endl;

    // 關閉檔案
    rawFile.close();

    return original_img;
}


vector<unsigned char> get_dpcm_img(vector<unsigned char> original_img) {
    // 預處理圖像
    vector<unsigned char> dpcm_img(original_img); // 預處理圖像，先複製原本的圖

    for (size_t i = 257; i < original_img.size(); ++i) { //i=0
        // 最上排和最左排像素不用修改
        if (i % 256 != 0){//&& i > 255) {
            // DPCM 處理
            int prediction = (original_img[i - 257] + original_img[i - 256] + original_img[i - 1]) / 3; // 取左邊、左上、正上方 3 個像素的平均值作為預測值
            int d = original_img[i] - prediction;
            dpcm_img[i] = d + 128; // 將差值加上 128，以使其在 0 到 255 的範圍內
            //cout << d+128 << " ";
        }
        /*else {
            dpcm_img[i] = original_img[i];
        }*/
    }

    //for (int value : dpcm_img) {
        //cout << value << " ";
    //}

    return dpcm_img;

}


map<unsigned char, double> get_probability_map(vector<unsigned char> buffer) {

    // 計算每個數值的出現次數
    unordered_map<unsigned char, int> frequency_map;
    for (unsigned char value : buffer) {
        frequency_map[value]++;
    }
    // 計算機率分佈
    //cout << "灰階圖中每個數值的出現機率分佈：" << endl;

    map<unsigned char, double> probability_map;
    for (const auto& pair : frequency_map) {
        //pair.first: color value
        //pair.second: appear frequency
        double probability = static_cast<double>(pair.second) / buffer.size();
        probability_map[pair.first] = probability;
        //cout << "數值 " << static_cast<int>(pair.first) << ": " << probability << endl;
    }

    return probability_map;
}


void print_entropy(map<unsigned char, double> probability_map) {
    // 計算一階熵
    double entropy = 0.0;
    for (const auto& pair : probability_map) {
        //pair.second: appear probability
        entropy -= pair.second * log2(pair.second);
    }

    // 輸出結果
    printf("First-order entropy: %0.2f bits/symbol\n", entropy);
}