#pragma once
#include <opencv2/opencv.hpp>

// 2BinProcess
// ifÅ`else
void process_1(const cv::Mat& mat_src, cv::Mat& mat_dst, uchar th, uchar max_val);
// ï™äÚÇ»Çµ
void process_2(const cv::Mat& mat_src, cv::Mat& mat_dst, uchar th, uchar max_val);
// LUT
void make_binary_lut(uchar* ucLUT, uchar th, uchar max_val);
void process_lut(const cv::Mat& mat_src, cv::Mat& mat_dst, const uchar* ucLUT);

// threshold
void custom_threshold(const cv::Mat& mat_src, cv::Mat& mat_dst, uchar thresh, uchar maxval);