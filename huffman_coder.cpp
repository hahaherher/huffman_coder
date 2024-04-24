#include "huffman_coder.h"

// 构造函数
HuffmanNode::HuffmanNode(unsigned char v, double p) : value(v), probability(p), left(nullptr), right(nullptr) {}
HuffmanNode::HuffmanNode(unsigned char v, double p, HuffmanNode* l, HuffmanNode* r) : value(v), probability(p), left(l), right(r) {}


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



vector<LengthTable> buildLengthTable(map<unsigned char, double> &probabilities) {
    //sort map by value
    vector<pair<unsigned char, double>> probability_vec(probabilities.begin(), probabilities.end());
    auto cmp = [](const pair<unsigned char, double>& a, const pair<unsigned char, double>& b) {
        return a.second < b.second; };
    sort(probability_vec.begin(), probability_vec.end(), cmp);   
    
    // phase 1
    // initial
    vector<LengthTable> huff_len_tables;
    for (auto &pair: probability_vec){
        LengthTable huff_len_table;
        huff_len_table.value = pair.first;
        huff_len_table.work_array = pair.second;
        huff_len_table.type = 0; 
        huff_len_tables.push_back(huff_len_table);
    }
    // type 0 to type 1
    int last_data_id = 0;
    for (int i = 0; i < probabilities.size(); i+=2) {
        if (i == probabilities.size() - 1) {
            huff_len_tables[i / 2].work_array = huff_len_tables[i].work_array + huff_len_tables[0].work_array;
            huff_len_tables[0].type = -1; //empty
        }
        else {
            huff_len_tables[i / 2].work_array = huff_len_tables[i].work_array + huff_len_tables[i + 1].work_array;
        }
        huff_len_tables[i / 2].type = 1;
        last_data_id = i / 2;
    }

    // type 1 to type 2
    if (huff_len_tables[0].type == -1) {
        huff_len_tables[0].work_array = last_data_id;
        huff_len_tables[0].type = 2;
        last_data_id++;
    }
    for (int i = 0; i < probabilities.size(); i += 2) {
        // combine 2 nodes
        if (huff_len_tables[i].type == 1 && huff_len_tables[i+1].type == 0) {
            // root
            huff_len_tables[last_data_id].work_array = 0;
            huff_len_tables[last_data_id].type = 3;
            break;
        }
        huff_len_tables[last_data_id+1].work_array = huff_len_tables[i].work_array + huff_len_tables[i + 1].work_array;
        huff_len_tables[last_data_id+1].type = 1;
        // save parent pos
        huff_len_tables[i].work_array = last_data_id+1;
        huff_len_tables[i].type = 2;
        huff_len_tables[i + 1].work_array = last_data_id+1;
        huff_len_tables[i + 1].type = 2;
        last_data_id++;
    }
    // type 2 to type 3
    // depth = parent depth + 1
    for (int i = last_data_id - 1; i >= 0; i--) {
        int parent_pos = huff_len_tables[i].work_array;
        huff_len_tables[i].work_array = huff_len_tables[parent_pos].work_array + 1;
        huff_len_tables[i].type = 3;
    }
    // type 3 to type 4
    // only root
    if (last_data_id - 1 < 0) {
        huff_len_tables[0].work_array = 1;
        huff_len_tables[1].work_array = 1;
        huff_len_tables[0].type = 4;
        huff_len_tables[1].type = 4;
    }
    for (int i = last_data_id - 1, last_empty_id=last_data_id+1; i >= 0; i--) {
        int depth = huff_len_tables[i].work_array;
        huff_len_tables[i + 1].work_array = depth + 1;//填下個位置
        huff_len_tables[i + 1].type = 4;
        if (last_empty_id < probabilities.size()) {
            huff_len_tables[last_empty_id].work_array = depth + 1;//填空位
            huff_len_tables[last_empty_id].type = 4;
        }
        //填自己位
        else if (i == 0) {
            huff_len_tables[i].work_array = depth + 1;
            huff_len_tables[i].type = 4;
        } 
    }
    return huff_len_tables;
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

map<unsigned char, vector<bool>> lengthTable2HuffmanTable(vector<LengthTable> lengthTable) {
    map<unsigned char, vector<bool>> huffmanTable;
    // type 3 to type 4
    /*for (int i = 0; i<256; i++) {
        int depth = lengthTable[i]->work_array;
        lengthTable[i]->work_array = depth + 1;
        huff_len_table[last_data_id]->work_array = depth + 1;
        huff_len_table[last_data_id + 1]->work_array = depth + 1;
        last_data_id = i;
        if (i == 0) {
            huff_len_table[i]->work_array = depth + 1;
        }
    }*/
    return huffmanTable;
}


string encode(vector<unsigned char> original_img, map<unsigned char, double> probability_map, string img_name, string process_type, bool is_dpcm) {
    // 构建 Huffman 树
    HuffmanNode* root = buildHuffmanTree(probability_map);
    //vector<LengthTable> lengthTable = buildLengthTable(probability_map);

    // 构建 Huffman 表
    vector<bool> code;
    map<unsigned char, vector<bool>> huffmanTable;
    buildHuffmanTable(root, code, huffmanTable);
    //print_huffmanTable(huffmanTable);
    //map<unsigned char, vector<bool>> huffmanTable = lengthTable2HuffmanTable(lengthTable);

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
    
    return huff_file_name;
}

vector<unsigned char> decode(string huff_file_name) {
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
        bool digit_bool = (digit == '0') ? false : true;
        binary_code.push_back(digit_bool);
        size_t pos = 0;
        //code += digit;
        auto letter = invertedHuffMap.find(binary_code);
        if (letter != invertedHuffMap.end()) {
            uncompressed_img.push_back(letter->second);
            binary_code.clear();
        }
    }
    return uncompressed_img;
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






