#include "2BinProcessSpeedComp.h"

// ifÅ`else
void process_1(const cv::Mat& mat_src, cv::Mat& mat_dst, uchar th, uchar max_val) {
	mat_dst.create(mat_src.size(), mat_src.type());

	if (mat_src.isContinuous() && mat_dst.isContinuous()){
		const uchar* sp = mat_src.ptr<uchar>(0);
		uchar* dp = mat_dst.ptr<uchar>(0);
		const int total = (int)mat_src.total();

		for (int i = 0; i < total; i++) {
			dp[i] = (sp[i] > th) ? max_val : 0;
		}

		return;
	}

	for (int y = 0; y < mat_src.rows; y++) {
		const uchar* ucSP = mat_src.ptr<uchar>(y);
		uchar* ucDP = mat_dst.ptr<uchar>(y);

		for (int x = 0; x < mat_src.cols; x++) {
			ucDP[x] = (ucSP[x] > th) ? max_val : 0;
		}
	}
}

// ï™äÚÇ»Çµ
void process_2(const cv::Mat& mat_src, cv::Mat& mat_dst, uchar th, uchar max_val){
	mat_dst.create(mat_src.size(), mat_src.type());

	if (mat_src.isContinuous() && mat_dst.isContinuous()) {
		const uchar* sp = mat_src.ptr<uchar>(0);
		uchar* dp = mat_dst.ptr<uchar>(0);
		const int total = (int)mat_src.total();

		for (int i = 0; i < total; i++) {
			dp[i] = static_cast<uchar>((sp[i] > th) * max_val);
		}

		return;
	}

	for (int y = 0; y < mat_src.rows; y++) {
		const uchar* sp = mat_src.ptr<uchar>(y);
		uchar* dp = mat_dst.ptr<uchar>(y);

		for (int x = 0; x < mat_src.cols; x++) {
			dp[x] = static_cast<uchar>((sp[x] > th) * max_val);
		}
	}
}

// LUT
void make_binary_lut(uchar* ucLUT, uchar th, uchar max_val) {
	for (int i = 0; i < 256; i++) {
		ucLUT[i] = (i > th) ? max_val : 0;
	}
}
void process_lut(const cv::Mat& mat_src, cv::Mat& mat_dst, const uchar* ucLUT) {
	if (!ucLUT) throw std::invalid_argument("LUT is null");

	mat_dst.create(mat_src.size(), mat_src.type());

	if (mat_src.isContinuous() && mat_dst.isContinuous()) {
		const uchar* sp = mat_src.ptr<uchar>(0);
		uchar* dp = mat_dst.ptr<uchar>(0);
		const int n = (int)mat_src.total();

		for (int i = 0; i < n; i++) {
			dp[i] = ucLUT[sp[i]];
		}

		return;
	}

	for (int y = 0; y < mat_src.rows; y++) {
		const uchar* sp = mat_src.ptr<uchar>(y);
		uchar* dp = mat_dst.ptr<uchar>(y);

		for (int x = 0; x < mat_src.cols; x++) {
			dp[x] = ucLUT[sp[x]];
		}
	}
}