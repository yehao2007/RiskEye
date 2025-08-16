#pragma once

#include <immintrin.h>
#include <vector>
#include <memory>
#include "../core/Types.h"

namespace hft {
namespace hardware {

class VectorProcessor {
public:
    // 初始化SIMD支持
    bool initialize();

    // 向量化处理市场数据
    void processMarketDataBatch(const float* data, size_t size);
    
    // AVX-512计算特征
    void computeFeatures512(const float* input, float* output, size_t size);
    
    // 并行处理订单簿
    void processOrderBookParallel(const OrderBook& book);

    // 批量计算指标
    struct VectorizedMetrics {
        __m512 vwap;           // 成交量加权平均价格
        __m512 momentum;       // 动量指标
        __m512 volatility;     // 波动率
        __m512 orderImbalance; // 订单失衡
    };
    VectorizedMetrics calculateMetrics(const float* prices, const float* volumes, size_t size);

private:
    // AVX-512优化函数
    __m512 calculateVWAP512(const float* prices, const float* volumes, size_t size);
    __m512 calculateVolatility512(const float* returns, size_t size);
    __m512 calculateOrderImbalance512(const float* bids, const float* asks, size_t size);
    
    // 向量化数学函数
    inline __m512 vec_exp(__m512 x);
    inline __m512 vec_log(__m512 x);
    inline __m512 vec_sqrt(__m512 x);

private:
    bool avx512_supported_{false};
    bool fma_supported_{false};
    
    // 对齐内存分配
    alignas(64) float work_buffer_[1024];
    
    static constexpr size_t VECTOR_WIDTH = 16;  // AVX-512 float向量宽度
    static constexpr size_t CACHE_LINE_SIZE = 64;
};

// 内联实现示例
inline __m512 VectorProcessor::vec_exp(__m512 x) {
    // AVX-512优化的exp实现
    const __m512 ln2 = _mm512_set1_ps(0.693147180559945f);
    const __m512 one = _mm512_set1_ps(1.0f);
    
    __m512 t = _mm512_mul_ps(x, _mm512_set1_ps(1.442695040888963f));
    __m512i i = _mm512_cvtps_epi32(t);
    __m512 f = _mm512_sub_ps(t, _mm512_cvtepi32_ps(i));
    
    __m512 p = one;
    p = _mm512_fmadd_ps(f, p, one);
    p = _mm512_fmadd_ps(f, p, _mm512_set1_ps(0.5f));
    p = _mm512_fmadd_ps(f, p, _mm512_set1_ps(0.166666666666667f));
    
    return _mm512_scalef_ps(p, _mm512_cvtepi32_ps(i));
}

} // namespace hardware
} // namespace hft
