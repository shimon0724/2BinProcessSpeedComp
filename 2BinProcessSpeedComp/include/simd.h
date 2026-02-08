#pragma once
#include <cstdint>
#include <cstring>

using uchar = unsigned char;

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#if defined(__AVX2__) || defined(_M_AVX2)
#include <immintrin.h>
#elif defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#include <emmintrin.h>
#endif

#if defined(__ARM_NEON) || defined(_M_ARM64)
#include <arm_neon.h>
#endif

// ---- v_uint8 (8-bit SIMD vector) ----
struct v_uint8
{
#if defined(__AVX2__) || defined(_M_AVX2)
    static constexpr int lanes = 32;
    __m256i val;
    v_uint8() = default;
    explicit v_uint8(__m256i v) : val(v) {}
#elif defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
    static constexpr int lanes = 16;
    __m128i val;
    v_uint8() = default;
    explicit v_uint8(__m128i v) : val(v) {}
#elif defined(__ARM_NEON) || defined(_M_ARM64)
    static constexpr int lanes = 16;
    uint8x16_t val;
    v_uint8() = default;
    explicit v_uint8(uint8x16_t v) : val(v) {}
#else
    static constexpr int lanes = 16;
    uchar val[lanes];
#endif
};

// ---- vx_load: unaligned load ----
static inline v_uint8 vx_load(const uchar* p)
{
#if defined(__AVX2__) || defined(_M_AVX2)
    // 32 bytes unaligned load
    return v_uint8(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(p)));

#elif defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
    // 16 bytes unaligned load
    return v_uint8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(p)));

#elif defined(__ARM_NEON) || defined(_M_ARM64)
    // 16 bytes load (NEONは基本的に非アラインでもOK扱いで使われることが多い)
    return v_uint8(vld1q_u8(p));

#else
    // Portable fallback: memcpy
    v_uint8 r;
    std::memcpy(r.val, p, v_uint8::lanes);
    return r;
#endif
}

// ---- vx_setall_u8: ブロードキャスト ----
static inline v_uint8 vx_setall_u8(uchar x)
{
#if defined(__AVX2__) || defined(_M_AVX2)
    // 32 lanes に同じ 8bit 値をブロードキャスト
    return v_uint8(_mm256_set1_epi8((char)x));

#elif defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
    // 16 lanes
    return v_uint8(_mm_set1_epi8((char)x));

#elif defined(__ARM_NEON) || defined(_M_ARM64)
    // 16 lanes
    return v_uint8(vdupq_n_u8(x));

#else
    // Portable fallback
    v_uint8 r;
    for (int i = 0; i < v_uint8::lanes; ++i)
        r.val[i] = x;
    return r;
#endif
}

// v_lt: lane-wise (a < b) ? 0xFF : 0x00
static inline v_uint8 v_lt(const v_uint8& a, const v_uint8& b)
{
#if defined(__AVX2__) || defined(_M_AVX2)
    // x86の8bit比較は signed なので、unsigned比較にするため 0x80 を XOR してレンジをずらす
    const __m256i bias = _mm256_set1_epi8((char)0x80);
    __m256i ax = _mm256_xor_si256(a.val, bias);
    __m256i bx = _mm256_xor_si256(b.val, bias);

    // (a < b) は (b > a) と同値。signed compare を使う
    __m256i mask = _mm256_cmpgt_epi8(bx, ax); // 0xFF or 0x00 per lane
    return v_uint8(mask);

#elif defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
    const __m128i bias = _mm_set1_epi8((char)0x80);
    __m128i ax = _mm_xor_si128(a.val, bias);
    __m128i bx = _mm_xor_si128(b.val, bias);

    __m128i mask = _mm_cmpgt_epi8(bx, ax); // 0xFF or 0x00 per lane
    return v_uint8(mask);

#elif defined(__ARM_NEON) || defined(_M_ARM64)
    // NEON は unsigned compare が素直に使える（a < b）
    // 環境によって vcltq_u8 が無い場合は vcgtq_u8(b,a) で代替可能
#if defined(__aarch64__) || defined(_M_ARM64)
    uint8x16_t mask = vcltq_u8(a.val, b.val); // 0xFF or 0x00
    return v_uint8(mask);
#else
    uint8x16_t mask = vcgtq_u8(b.val, a.val); // (a < b) == (b > a)
    return v_uint8(mask);
#endif

#else
    v_uint8 r;
    for (int i = 0; i < v_uint8::lanes; ++i)
        r.val[i] = (a.val[i] < b.val[i]) ? 0xFF : 0x00;
    return r;
#endif
}

// v_and: lane-wise bitwise AND
static inline v_uint8 v_and(const v_uint8& a, const v_uint8& b)
{
#if defined(__AVX2__) || defined(_M_AVX2)
    return v_uint8(_mm256_and_si256(a.val, b.val));

#elif defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
    return v_uint8(_mm_and_si128(a.val, b.val));

#elif defined(__ARM_NEON) || defined(_M_ARM64)
    return v_uint8(vandq_u8(a.val, b.val));

#else
    v_uint8 r;
    for (int i = 0; i < v_uint8::lanes; ++i)
        r.val[i] = (uchar)(a.val[i] & b.val[i]);
    return r;
#endif
}

// v_store: unaligned store
static inline void v_store(uchar* p, const v_uint8& a)
{
#if defined(__AVX2__) || defined(_M_AVX2)
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(p), a.val);

#elif defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
    _mm_storeu_si128(reinterpret_cast<__m128i*>(p), a.val);

#elif defined(__ARM_NEON) || defined(_M_ARM64)
    vst1q_u8(p, a.val);

#else
    std::memcpy(p, a.val, v_uint8::lanes);
#endif
}

template<typename V>
struct VTraits;   // primary template（未定義）

template<>
struct VTraits<v_uint8>
{
    using lane_type = unsigned char;

    static inline int vlanes()
    {
        return v_uint8::lanes;
    }
};