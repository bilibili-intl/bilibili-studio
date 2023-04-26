//
// Created by bytedance on 2021/9/28.
//

#ifndef SAMI_CORE_TOB_DEMO_HELP_FUNCTION_H
#define SAMI_CORE_TOB_DEMO_HELP_FUNCTION_H

#include <vector>
#include <string>


/**
 * @brief floatArray->ShortArray
 * @param float_arr float数组
 * @param short_arr short数组
 * @param ArrayNum 数组的个数
 * */
void floatArrayToShortArray(float* float_arr, short* short_arr,int ArrayNum);


/**
 * @brief ShortArray -> floatArray
 * @param short_arr short数组
 * @param float_arr float数组
 * @param ArrayNum 数组的个数
 * */
void shortArrayToFloatArray(short* short_arr,float* float_arr,int ArrayNum);

/**
 * @brief interleave To Planar, only use Float format
 * @param source source one dimensional pointer
 *               such as : buffer[i] = L1 R1 L2 R2 .... L1024 R1024
 * @param destination  dest two dimensional pointer
 *                     such as : buffer[ch][i] = L1 L2 .... L1024
 *                                               R1 R2 .... R1024
 * @param num_samples  how many samples(mono) the buffer have
 * @param channels channel num
 */
void interleaveToPlanarFloat(const float* source, float** destination, int num_samples, int channels);


/**
 * @brief Planar To interleave  , only use Float format
 * @param source  dest two dimensional pointer
 *                     such as : buffer[ch][i] = L1 L2 .... L1024
 *                                               R1 R2 .... R1024
 * @param destination  one dimensional pointer
 *               such as : buffer[i] = L1 R1 L2 R2 .... L1024 R1024
 * @param num_samples  how many samples(mono) the buffer have
 * @param channels channel num
 */
void planarToInterleaveFloat(const float** source, float* destination, int num_samples, int channels);

std::vector<uint8_t> loadModelAsBinary(const std::string& path);



#endif //SAMI_CORE_TOB_DEMO_HELP_FUNCTION_H
