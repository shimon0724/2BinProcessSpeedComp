#include "2BinProcessSpeedComp.h"
#include "simd.h"

#ifdef _OPENMP
#include <omp.h>
#endif

struct Range {
    int start;
    int end;
};

static void parallel_threshold(const cv::Mat& mat_src, cv::Mat& mat_dst, uchar thresh, uchar maxval, int id, int nstripes);
static inline Range computeRange(int id, int id_end, int wholeStart, int wholeEnd, int nstripes);

void custom_threshold(const cv::Mat& mat_src, cv::Mat& mat_dst, uchar thresh, uchar maxval) {
    mat_dst.create(mat_src.size(), mat_src.type());

    int nstripes = (int)std::max(1.0, mat_dst.total() / (double)(1 << 16));

#pragma omp parallel for schedule(static)
    for (int i = 0; i < nstripes; i++) parallel_threshold(mat_src, mat_dst, thresh, maxval, i, nstripes);
}

inline Range computeRange(
    int id,          // ストライプ番号（sr.start）
    int id_end,      // sr.end（通常は id+1）
    int wholeStart,  // wholeRange.start
    int wholeEnd,    // wholeRange.end
    int nstripes     // 分割数
) {
    const int len = wholeEnd - wholeStart;

    Range r;
    r.start = wholeStart + (int)(((uint64_t)id * len + nstripes / 2) / nstripes);

    if (id_end >= nstripes) r.end = wholeEnd;
    else                    r.end = wholeStart + (int)(((uint64_t)id_end * len + nstripes / 2) / nstripes);

    return r;
}

void parallel_threshold(const cv::Mat& mat_src, cv::Mat& mat_dst, uchar thresh, uchar maxval, int id, int nstripes) {
    Range r = computeRange(id, id + 1, 0, mat_src.rows, nstripes);

    cv::Mat srcStripe = mat_src.rowRange(r.start, r.end);
    cv::Mat dstStripe = mat_dst.rowRange(r.start, r.end);

    const int lanes = VTraits<v_uint8>::vlanes();

    v_uint8 thresh_u = vx_setall_u8(thresh);
    v_uint8 maxval_u = vx_setall_u8(maxval);

    if (srcStripe.isContinuous() && dstStripe.isContinuous()) {
        const int total = (int)srcStripe.total(); // 画素数
        const uchar* src = srcStripe.ptr<uchar>(0);
        uchar* dst = dstStripe.ptr<uchar>(0);

        int j = 0;
        for (; j <= total - lanes; j += lanes) {
            v_uint8 v0 = vx_load(src + j);
            v0 = v_lt(thresh_u, v0);
            v0 = v_and(v0, maxval_u);
            v_store(dst + j, v0);
        }
        for (; j < total; j++) {
            dst[j] = (src[j] > thresh) ? maxval : 0;
        }

        return;
    }

    for (int i = 0; i < srcStripe.rows; i++) {
        const uchar* src = srcStripe.ptr<uchar>(i);
        uchar* dst = dstStripe.ptr<uchar>(i);

        int j = 0;
        for (; j <= srcStripe.cols - lanes; j += lanes) {
            v_uint8 v0 = vx_load(src + j);
            v0 = v_lt(thresh_u, v0);
            v0 = v_and(v0, maxval_u);
            v_store(dst + j, v0);
        }
        for (; j < srcStripe.cols; j++) {
            dst[j] = (src[j] > thresh) ? maxval : 0;
        }
    }
}