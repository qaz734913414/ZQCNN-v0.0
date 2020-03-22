#ifndef _ZQ_CNN_DECONVOLUTION_GEMM_32F_ALIGN_C_H_
#define _ZQ_CNN_DECONVOLUTION_GEMM_32F_ALIGN_C_H_
#include "../ZQ_CNN_CompileConfig.h"
#if defined(__cplusplus) || defined(c_plusplus) 
extern "C" {
#endif

	

#if __ARM_NEON || ZQ_CNN_USE_SSETYPE >= ZQ_CNN_SSETYPE_SSE

	void zq_cnn_deconv_with_padding_gemm_32f_align128bit_k2s2(
		const float* in_tensor4D_data,
		int in_N,
		int in_H,
		int in_W,
		int in_C,
		int in_pixelStep,
		int in_widthStep,
		int in_sliceStep,
		const float* filters_data,
		int filter_N,
		int filter_H, // must be 1
		int filter_W, // must be 1
		int filter_C, // must be in_C
		int filter_pixelStep,
		int filter_widthStep,
		int filter_sliceStep,
		int stride_H,
		int stride_W,
		int dilation_H,
		int dilation_W,
		float* out_tensor4D_data,
		int out_N,	// must be in_N
		int out_H,	// must be (in_H - filter_H)/stride_H + 1
		int out_W,	// must be (in_W - filter_W)/stride_W + 1
		int out_C,	// must be filter_N
		int out_pixelStep,
		int out_widthStep,
		int out_sliceStep,
		int pad_top,
		int pad_bottom,
		int pad_left,
		int pad_right,
		void** buffer,
		__int64 *buffer_len
	);

#endif

#if __ARM_NEON
#if __ARM_NEON_FP16
	void zq_cnn_deconv_with_padding_gemm_16f_align128bit_k2s2(
		const float16_t* in_tensor4D_data,
		int in_N,
		int in_H,
		int in_W,
		int in_C,
		int in_pixelStep,
		int in_widthStep,
		int in_sliceStep,
		const float16_t* filters_data,
		int filter_N,
		int filter_H, // must be 1
		int filter_W, // must be 1
		int filter_C, // must be in_C
		int filter_pixelStep,
		int filter_widthStep,
		int filter_sliceStep,
		int stride_H,
		int stride_W,
		int dilation_H,
		int dilation_W,
		float16_t* out_tensor4D_data,
		int out_N,	// must be in_N
		int out_H,	// must be (in_H - filter_H)/stride_H + 1
		int out_W,	// must be (in_W - filter_W)/stride_W + 1
		int out_C,	// must be filter_N
		int out_pixelStep,
		int out_widthStep,
		int out_sliceStep,
		int pad_top,
		int pad_bottom,
		int pad_left,
		int pad_right,
		void** buffer,
		__int64 *buffer_len
	);

#endif//__ARM_NEON_FP16

#else

#if ZQ_CNN_USE_SSETYPE >= ZQ_CNN_SSETYPE_AVX

	void zq_cnn_deconv_with_padding_gemm_32f_align256bit_k2s2(
		const float* in_tensor4D_data,
		int in_N,
		int in_H,
		int in_W,
		int in_C,
		int in_pixelStep,
		int in_widthStep,
		int in_sliceStep,
		const float* filters_data,
		int filter_N,
		int filter_H, // must be 1
		int filter_W, // must be 1
		int filter_C, // must be in_C
		int filter_pixelStep,
		int filter_widthStep,
		int filter_sliceStep,
		int stride_H,
		int stride_W,
		int dilation_H,
		int dilation_W,
		float* out_tensor4D_data,
		int out_N,	// must be in_N
		int out_H,	// must be (in_H - filter_H)/stride_H + 1
		int out_W,	// must be (in_W - filter_W)/stride_W + 1
		int out_C,	// must be filter_N
		int out_pixelStep,
		int out_widthStep,
		int out_sliceStep,
		int pad_top,
		int pad_bottom,
		int pad_left,
		int pad_right,
		void** buffer,
		__int64 *buffer_len
	);
#endif

#endif //__ARM_NEON

#if defined(__cplusplus) || defined(c_plusplus) 
}
#endif

#endif