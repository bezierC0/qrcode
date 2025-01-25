#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <chrono>
#include <omp.h>

#include <zxing/common/Counted.h>
#include <zxing/Binarizer.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/Result.h>
#include <zxing/common/GlobalHistogramBinarizer.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>
#include ".\Decode\ImageReaderSource.h"

#include ".\Encode\qrcodegen.h"

using namespace zxing;
// DEBUG用のdefine
#define QRCODE_REALSE //リリース時にON
#ifdef QRCODE_REALSE

#else
	#define QRCODE_PERFORMANCE_DEBUG // 性能測定
	#ifdef QRCODE_PERFORMANCE_DEBUG
		
	#else
		#define QRCODE_OUTPUT_BMP_DEBUG		// QRCodeをbmpで生成
	#endif // QRCODE_PERFORMANCE_DEBUG
#endif
// DEBUG用のdefine

#pragma region ZXING_LIB

#ifdef _DEBUG
#	define CV_EXT_STR "d.lib"
#else
#	define CV_EXT_STR ".lib"
#endif
#pragma comment(lib, "libzxing" CV_EXT_STR)
#pragma comment(lib, "iconv.lib"  )
#pragma endregion ZXING_LIB

#ifdef QRCODE_PERFORMANCE_DEBUG // 性能測定
#include <chrono>
#endif

#ifdef QRCODE_OUTPUT_BMP_DEBUG
#pragma region OPENCV_LIB
#pragma managed(push, off)

#define CV_VERSION_STR CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)

#ifdef _DEBUG
#	define CV_EXT_STR "d.lib"
#else
#	define CV_EXT_STR ".lib"
#endif

#pragma comment(lib, "opencv_world" CV_VERSION_STR CV_EXT_STR)
#pragma managed(pop)
#pragma endregion OPENCV_LIB
#include <opencv2/opencv.hpp>
#endif


namespace QRCode {
	#define WIDTH 7680					// 横
	#define HEIGHT 4320					// 縦
	#define INPUT4K_4_WIDTH (WIDTH/2)	// 8kの4分一
	#define INPUT4K_4_HEIGHT (HEIGHT/2)	// 8kの4分一
	#define QRCODE_SIZE_SCALE 6			// QRCodeの拡大倍率（6倍）
	#define QRCODE_TEXT_LENGHT 255		// エンコードの最大文字列
	#define QRCODE_WIDTH (((_encode_param_version - 1) + 21) * QRCODE_SIZE_SCALE)// QRCode幅（拡大後）
	#define QRCODE_HEIGHT QRCODE_WIDTH	// QRCode縦（拡大後）
	#define PIXEL_IN_FOUR_WORDS 6		// 4 wordsでのピクセル数（word = 4byte）
	#define YUV_BLACK_Y 64				// 黒YCbCr:64 512 512
	#define YUV_BLACK_UV 512			// 黒YCbCr:64 512 512
	#define YUV_WHITE_Y 940				// 白YCbCr:940 512 512
	#define YUV_WHITE_UV 512			// 白YCbCr:940 512 512
	#define Y_THRESHOLD ((YUV_WHITE_Y - YUV_BLACK_Y + 1) / 2 + YUV_BLACK_Y) // Y閾値
	// QRCodeパラメタ
	const qrcodegen_Ecc _encode_param_ecl = qrcodegen_Ecc_HIGH;	// 誤りレベル
	const int _encode_param_version = 1;						// version1:21 * 21 max数値桁：17
	const qrcodegen_Mask _encode_param_mask = qrcodegen_Mask_0;	// マスク0はペナルティスコア点数低い
	
	// エンコードクラス
	class Encoder {
	public:
		Encoder();
		~Encoder();
		int QRCodeGenerator(const unsigned int* frame_num, unsigned short* dy);
		int QRCodeGeneratorByYUV(const unsigned int* frame_num, unsigned int* dycbcr);
	};

	// デコードクラス
	class Decoder {
	public:
		Decoder();
		~Decoder();
		int QRCodeScanner(const unsigned short* dy,unsigned int* frame_num,int image_width = WIDTH);
		int QRCodeScannerByYUV(unsigned int* dycbcr, unsigned int* frame_num, int image_width = WIDTH);

	};
	int YCbCrToRGB(const unsigned short** dycbcr, unsigned short** dRGB);
}