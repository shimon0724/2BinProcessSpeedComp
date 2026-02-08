// 2BinProcessSpeedComp.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "2BinProcessSpeedComp.h"


using namespace std;

int main(void) {
	using clock = std::chrono::steady_clock;
	constexpr int W = 1920;
	constexpr int H = 1080;
	constexpr int ITERS = 1000;
	constexpr uchar TH = 128;
	constexpr uchar MAXV = 255;

	volatile uint64_t sink = 0;

#if defined(__AVX2__)
	std::cout << "AVX2 enabled\n";
#elif defined(__SSE2__)
	std::cout << "SSE2 enabled\n";
#else
	std::cout << "No SIMD\n";
#endif

	// LUT作成は一度だけ
	uchar lut[256];
	make_binary_lut(lut, TH, MAXV);

	// src/dst（再確保を避ける）
	cv::Mat src(H, W, CV_8UC1);
	cv::Mat dst_branch(H, W, CV_8UC1);
	cv::Mat dst_branchless(H, W, CV_8UC1);
	cv::Mat dst_lut(H, W, CV_8UC1);
	cv::Mat dst_omp(H, W, CV_8UC1);
	cv::Mat dst_ocv(H, W, CV_8UC1);

	// 乱数生成器（固定シードで再現性確保）
	cv::RNG rng(123456);

	// ウォームアップ（スレッドプール/CPU周波数/ページフォルト対策）
	rng.fill(src, cv::RNG::UNIFORM, 0, 256);
	cv::threshold(src, dst_ocv, TH, MAXV, cv::THRESH_BINARY);
	custom_threshold(src, dst_omp, TH, MAXV);

	uint64_t t_branch = 0, t_branchless = 0, t_lut = 0, t_omp = 0, t_ocv = 0;

	for (int iter = 0; iter < ITERS; iter++) {
		// 毎回ランダム画像生成
		rng.fill(src, cv::RNG::UNIFORM, 0, 256);

		// サンプル位置
		int idx = iter & 1023;

		// 分岐あり
		{
			auto s = clock::now();
			process_1(src, dst_branch, TH, MAXV);
			auto e = clock::now();
			t_branch += (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count();
			sink += dst_branch.ptr<uchar>(0)[idx];
		}

		// 分岐なし
		{
			auto s = clock::now();
			process_2(src, dst_branchless, TH, MAXV);
			auto e = clock::now();
			t_branchless += (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count();
			sink += dst_branchless.ptr<uchar>(0)[idx];
		}

		// LUT
		{
			auto s = clock::now();
			process_lut(src, dst_lut, lut);
			auto e = clock::now();
			t_lut += (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count();
			sink += dst_lut.ptr<uchar>(0)[idx];
		}

		// SIMD + OpenMP
		{
			auto s = clock::now();
			custom_threshold(src, dst_omp, TH, MAXV);
			auto e = clock::now();
			t_omp += (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count();
			sink += dst_omp.ptr<uchar>(0)[idx];
		}

		// OpenCV
		{
			auto s = clock::now();
			cv::threshold(src, dst_ocv, TH, MAXV, cv::THRESH_BINARY);
			auto e = clock::now();
			t_ocv += (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count();
			sink += dst_ocv.ptr<uchar>(0)[idx];
		}
	}

	std::cout << "[分岐あり]     " << t_branch << " ns\n";
	std::cout << "[分岐なし] " << t_branchless << " ns\n";
	std::cout << "[LUT]    " << t_lut << " ns\n";
	std::cout << "[SIMD+OpenMP] " << t_omp << " ns\n";
	std::cout << "[OpenCV] " << t_ocv << " ns\n";
	std::cout << "sink=" << sink << "\n";

	return 0;
}
