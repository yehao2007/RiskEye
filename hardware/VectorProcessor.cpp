#include "VectorProcessor.h"
#include "../core/Logger.h"
#include <cpuid.h>

namespace hft {
namespace hardware {

bool VectorProcessor::initialize() {
    // 检查CPU是否支持AVX-512
    unsigned int eax, ebx, ecx, edx;
    __cpuid(1, eax, ebx, ecx, edx);
    
    if ((ecx & bit_AVX512F) && (ecx & bit_AVX512DQ)) {
        avx512_supported_ = true;
        Logger::info("AVX-512 support detected");
    } else {
        Logger::warn("AVX-512 not supported, falling back to scalar operations");
        return false;
    }

    // 检查FMA支持
    if (ecx & bit_FMA) {
        fma_supported_ = true;
        Logger::info("FMA support detected");
    }

    return true;
}

void VectorProcessor::processMarketDataBatch(const float* data, size_t size) {
    // 确保数据对齐到64字节边界
    if (reinterpret_cast<uintptr_t>(data) % 64 != 0) {
        Logger::warn("Input data not aligned to 64-byte boundary");
    }

    // 批量处理市场数据
    for (size_t i = 0; i < size; i += VECTOR_WIDTH) {
        __m512 vec_data = _mm512_load_ps(&data[i]);
        
        // 应用市场数据转换
        vec_data = _mm512_mul_ps(vec_data, _mm512_set1_ps(1.0f));
        
        // 存储结果
        _mm512_store_ps(&work_buffer_[i], vec_data);
    }
}

void VectorProcessor::computeFeatures512(const float* input, float* output, size_t size) {
    const size_t vec_size = size / VECTOR_WIDTH;
    
    for (size_t i = 0; i < vec_size; ++i) {
        // 加载输入数据
        __m512 vec_input = _mm512_load_ps(&input[i * VECTOR_WIDTH]);
        
        // 计算特征
        __m512 vec_square = _mm512_mul_ps(vec_input, vec_input);
        __m512 vec_log = vec_log(vec_input);
        __m512 vec_exp = vec_exp(vec_input);
        
        // 组合特征
        __m512 vec_result = _mm512_add_ps(
            _mm512_mul_ps(vec_square, _mm512_set1_ps(0.5f)),
            _mm512_mul_ps(vec_log, _mm512_set1_ps(0.3f))
        );
        
        // 存储结果
        _mm512_store_ps(&output[i * VECTOR_WIDTH], vec_result);
    }
}

VectorProcessor::VectorizedMetrics VectorProcessor::calculateMetrics(
    const float* prices, const float* volumes, size_t size) {
    
    VectorizedMetrics metrics;
    
    // 计算VWAP
    metrics.vwap = calculateVWAP512(prices, volumes, size);
    
    // 计算动量指标
    __m512 vec_momentum = _mm512_setzero_ps();
    for (size_t i = VECTOR_WIDTH; i < size; i += VECTOR_WIDTH) {
        __m512 vec_price_curr = _mm512_load_ps(&prices[i]);
        __m512 vec_price_prev = _mm512_load_ps(&prices[i - VECTOR_WIDTH]);
        vec_momentum = _mm512_add_ps(vec_momentum, 
            _mm512_div_ps(
                _mm512_sub_ps(vec_price_curr, vec_price_prev),
                vec_price_prev
            )
        );
    }
    metrics.momentum = vec_momentum;
    
    // 计算波动率
    metrics.volatility = calculateVolatility512(prices, size);
    
    // 计算订单失衡
    metrics.orderImbalance = _mm512_setzero_ps(); // 实际实现需要订单簿数据
    
    return metrics;
}

__m512 VectorProcessor::calculateVWAP512(const float* prices, const float* volumes, size_t size) {
    __m512 vec_sum_pv = _mm512_setzero_ps();
    __m512 vec_sum_v = _mm512_setzero_ps();
    
    for (size_t i = 0; i < size; i += VECTOR_WIDTH) {
        __m512 vec_price = _mm512_load_ps(&prices[i]);
        __m512 vec_volume = _mm512_load_ps(&volumes[i]);
        
        vec_sum_pv = _mm512_fmadd_ps(vec_price, vec_volume, vec_sum_pv);
        vec_sum_v = _mm512_add_ps(vec_sum_v, vec_volume);
    }
    
    return _mm512_div_ps(vec_sum_pv, vec_sum_v);
}

__m512 VectorProcessor::calculateVolatility512(const float* returns, size_t size) {
    __m512 vec_sum = _mm512_setzero_ps();
    __m512 vec_sum_sq = _mm512_setzero_ps();
    
    for (size_t i = 0; i < size; i += VECTOR_WIDTH) {
        __m512 vec_return = _mm512_load_ps(&returns[i]);
        
        vec_sum = _mm512_add_ps(vec_sum, vec_return);
        vec_sum_sq = _mm512_fmadd_ps(vec_return, vec_return, vec_sum_sq);
    }
    
    __m512 vec_mean = _mm512_div_ps(vec_sum, _mm512_set1_ps(static_cast<float>(size)));
    __m512 vec_variance = _mm512_sub_ps(
        _mm512_div_ps(vec_sum_sq, _mm512_set1_ps(static_cast<float>(size))),
        _mm512_mul_ps(vec_mean, vec_mean)
    );
    
    return vec_sqrt(vec_variance);
}

} // namespace hardware
} // namespace hft
