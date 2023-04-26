//
// Created by bytedance on 2021/9/28.
//

#include "help_function.h"
#include <fstream>


void floatArrayToShortArray(float* float_arr, short* short_arr,int ArrayNum) {
    for(int i = 0; i < ArrayNum; i++) {
        // Truncate carefully
        int ival = 0;
        float fval = float_arr[i];
        ival = int((fval + 1.0) * 32768); // Centralize zero

        // Clipping
        if(ival < 0) {
            ival = 0;
        } else if (ival > 65535) {
            ival = 65535;
        }
        ival -= 32768;
        short_arr[i]  = short(ival);
    }
}

void shortArrayToFloatArray(short* short_arr,float* float_arr,int ArrayNum) {
    const float divider = 32768;
    for(int i = 0; i < ArrayNum; i++) {
        short s_val = short_arr[i];
        if(s_val == 32767) {
            float_arr[i] = 1.0;
        } else {
            float_arr[i] = float(s_val) / divider;
        }
    }
}

void interleaveToPlanarFloat(const float* source, float** destination, int num_samples, int channels){
    for (int i = 0; i < num_samples; ++i) {
        for (int j = 0; j < channels; ++j) {
            destination[j][i] = source[i * channels + j];
        }
    }
}

void planarToInterleaveFloat(const float** source, float* destination, int num_samples, int channels){
    for (int i = 0; i < num_samples; ++i) {
        for (int j = 0; j < channels; ++j) {
            destination[i * channels + j] = source[j][i];
        }
    }
}

std::vector<uint8_t> loadModelAsBinary(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()){
        return {};
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(size);
    if(file.read((char*)buffer.data(), size)) { return buffer; }
    return {};
}