# include "huffman_coder.h"

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
        if (i % 256 != 0) {//&& i > 255) {
            // DPCM 處理
            int prediction = (original_img[i - 257] + original_img[i - 256] + original_img[i - 1]) / 3; // 取左邊、左上、正上方 3 個像素的平均值作為預測值
            int d = original_img[i] - prediction;
            dpcm_img[i] = char(d + 128); // 將差值加上 128，以使其在 0 到 255 的範圍內
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



vector<unsigned char> decode_dpcm(vector<unsigned char> dpcm_img) {
    // 預處理圖像
    vector<unsigned char> decoded_dpcm_img(dpcm_img); // 預處理圖像，先複製原本的圖

    for (size_t i = 257; i < dpcm_img.size(); ++i) { //i=0
        // 最上排和最左排像素不用修改
        if (i % 256 != 0) {//&& i > 255) {
            // dpcm 處理
            int prediction = (decoded_dpcm_img[i - 257] + decoded_dpcm_img[i - 256] + decoded_dpcm_img[i - 1]) / 3; // 取左邊、左上、正上方 3 個像素的平均值作為預測值
            int d = dpcm_img[i] - 128 + prediction;
            decoded_dpcm_img[i] = char(d); //+ 128; // 將差值加上 128，以使其在 0 到 255 的範圍內
            //cout << d+128 << " ";
        }
        /*else {
            dpcm_img[i] = original_img[i];
        }*/
    }

    return decoded_dpcm_img;
}


int main() {
    int choice;
    string img_name;
    string process_type;
    string data_path = "./Data/RAW/";

    cout << "lena: 1" << endl;
    cout << "baboon: 2" << endl;
    cout << "Please choose the image: ";
    //cin >> choice;
    cout << endl;
    choice = 1;

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
    //cin >> choice;
    cout << endl;
    choice = 1;

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

    // original image
    vector<unsigned char> original_img = read_raw_img(file_name);
    map<unsigned char, double> probability_map = get_probability_map(original_img);
    cout << endl;
    string huff_file_name = encode(original_img, probability_map, img_name, process_type, false);
    vector<unsigned char> uncompressed_img = decode(huff_file_name);
    // 输出解压后的字符串
    if (uncompressed_img == original_img) {
        cout << "Uncompressed img equals to the original img\n" << endl;
    }
    else {
        cout << "Uncompressed img doesn't equals to the original img\n" << endl;
    }

    // save to .png

    //vector<unsigned char> grayscale_image = decode_dpcm(dpcm_img);
    //vector<unsigned char> img;

    //// 將向量轉換為 cv::Mat
    //cv::Mat grayscale_image(256, 256, CV_8UC1); // 創建一個 256x256 的單通道 (灰度) 圖像
    //std::memcpy(grayscale_image.data(), img.data(), img.size() * sizeof(unsigned char));

    //// 保存圖像
    //std::string filename = "output_image.png"; // 保存的文件名，可以修改文件格式
    //bool success = cv::imwrite(filename, grayscale_image);

    //if (success) {
    //    std::cout << "Image saved successfully as " << filename << std::endl;
    //}
    //else {
    //    std::cerr << "Failed to save image!" << std::endl;
    //    return 1;
    //}
    

    // dpcm image
    vector<unsigned char> dpcm_img = get_dpcm_img(original_img);
    map<unsigned char, double> dpcm_probability_map = get_probability_map(dpcm_img);
    string dpcm_huff_file_name = encode(dpcm_img, dpcm_probability_map, img_name, process_type, true);
    vector<unsigned char> uncompressed_dpcm_img = decode(dpcm_huff_file_name);
    
    
    //// test dpcm decode process
    //vector<unsigned char> decoded_dpcm_img = decode_dpcm(dpcm_img);
    //if (decoded_dpcm_img == original_img) {
    //    cout << "decoded dpcm img equals to the original img\n" << endl;
    //}
    //else {
    //    cout << "decoded dpcm img doesn't equals to the original img\n" << endl;
    //}
    //auto mismatch_pair = std::mismatch(decoded_dpcm_img.begin(), decoded_dpcm_img.end(), original_img.begin());
    //if (mismatch_pair.first != decoded_dpcm_img.end() && mismatch_pair.second != original_img.end()) {
    //    std::cout << "First mismatch at index: " << std::distance(decoded_dpcm_img.begin(), mismatch_pair.first) << std::endl;
    //    std::cout << "Value in decoded_dpcm_img: " << static_cast<int>(*mismatch_pair.first) << std::endl;
    //    std::cout << "Value in original_img: " << static_cast<int>(*mismatch_pair.second) << std::endl;
    //}
    //else {
    //    std::cout << "Vectors are equal" << std::endl;
    //}

    vector<unsigned char> decoded_img = decode_dpcm(uncompressed_dpcm_img);
    // 输出解压后的字符串
    if (decoded_img == original_img) {
        cout << "Uncompressed dpcm img equals to the original img\n" << endl;
    }
    else {
        cout << "Uncompressed dpcm img doesn't equals to the original img\n" << endl;
    }


    return 0;
}