#pragma once
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
    HuffmanNode(unsigned char v, double p);
    HuffmanNode(unsigned char v, double p, HuffmanNode* l, HuffmanNode* r);

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

//vector<unsigned char> read_raw_img(string file_name);
void print_entropy(map<unsigned char, double> probability_map);
//map<unsigned char, double> get_probability_map(vector<unsigned char> buffer);
//vector<unsigned char> get_dpcm_img(vector<unsigned char> original_img);

//huffman
//void write_binary_huff(string filename, string bitstream);
HuffmanNode* buildHuffmanTree(const map<unsigned char, double>& probabilities);
void buildHuffmanTable(HuffmanNode* root, vector<bool>& code, map<unsigned char, vector<bool>>& huffmanTable);

void write_binary_huff(string filename, map<unsigned char, vector<bool>> huffmanTable, string bitstream);
Bitstream read_binary_huff(string huff_file_name);
//void write_char_huff(string filename, string bitstream);

string huff_encode(vector<unsigned char> original_img, map<unsigned char, vector<bool>> huffmanTable);
void print_huffmanTable(map<unsigned char, vector<bool>> huffmanTable);
vector<unsigned char> decode(string huff_file_name);
string encode(vector<unsigned char> original_img, map<unsigned char, double> probability_map, string img_name, string process_type, bool is_dpcm);