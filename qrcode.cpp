#include "qrcode.h"
namespace QRCode {
Encoder::Encoder() {
}
Encoder::~Encoder() {

}
/**
* QRコード生成、Yデータに書き込む
* @param	frame_num	(in)		フレーム番号
* @param	y			(in/out)	yデータ
* @return	-1	NULLチェックエラー
*			 0	正常終了
*			 1	生成失敗
* @author	N.Shao
*/
int Encoder::QRCodeGenerator(const unsigned int* frame_num, unsigned short* dy) {
	// NULLチェック
	if (NULL == dy)
		return -1;

	// qrcode生成用のローカル変数
	char encode_txt[QRCODE_TEXT_LENGHT] = { '\0' };
	sprintf_s(encode_txt, "%u", *frame_num);
	uint8_t qr0[qrcodegen_BUFFER_LEN_MAX];// 生成qrcodeデータ格納buffer
	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

	if (!qrcodegen_encodeText(encode_txt, tempBuffer, qr0, _encode_param_ecl, _encode_param_version, _encode_param_version, _encode_param_mask, true))
		return 1;

	// オリジナルQRCodeのサイズ
	int qrcode_original_width = qrcodegen_getSize(qr0);
	int qrcode_original_height = qrcode_original_width;

	int zoom_in_scale = QRCODE_WIDTH / qrcode_original_width; // 拡大の倍率

	for (int y = 0; y < qrcode_original_height; y++) {
		int start_of_image_height = y * zoom_in_scale;
		for (int x = 0; x < qrcode_original_width; x++) {
			int start_of_image_width = WIDTH - qrcode_original_width * zoom_in_scale + x * zoom_in_scale;
			// false:白 true:黒
			if (!qrcodegen_getModule(qr0, x, y)) {
				// QRCode拡大
				for (int j = 0; j < zoom_in_scale; j++) {
					for (int i = 0; i < zoom_in_scale; i++) {
						// 白YCbCr:940 512 512
						// 黒YCbCr:64 512 512
						// 8K右半分が黒なので、白のみをセットする
						dy[start_of_image_height * WIDTH + j * WIDTH + start_of_image_width + i] = YUV_WHITE_Y;

					}
				}
			}
			else {
				for (int j = 0; j < zoom_in_scale; j++) {
					for (int i = 0; i < zoom_in_scale; i++) {
						// 白YCbCr:940 512 512
						// 黒YCbCr:64 512 512
						dy[start_of_image_height * WIDTH + j * WIDTH + start_of_image_width + i] = YUV_BLACK_Y;

					}
				}
			}
			
		}
	}
#ifdef  QRCODE_OUTPUT_BMP_DEBUG // Bmp生成
	int qrcode_bits_buff = 3;
	std::vector<unsigned char> qrcode_rgba_buff(QRCODE_WIDTH * QRCODE_HEIGHT, 0); // qrcode rgba バッファ
	std::vector<unsigned char> qrcode_rgba_noise_buff(QRCODE_WIDTH * QRCODE_HEIGHT * qrcode_bits_buff, 0); // qrcode rgba バッファ

	int start_of_image_width = WIDTH - QRCODE_WIDTH;// qrcodeの開始インデックス（行）
	unsigned short ycbcr[3] = { YUV_BLACK_Y, YUV_BLACK_UV ,YUV_BLACK_UV };

	for (int y = 0; y < qrcode_original_height; y++) {
		int start_of_image_height = y * zoom_in_scale;
		for (int x = 0; x < qrcode_original_width; x++) {
			int start_of_image_width = x * zoom_in_scale;
			if (!qrcodegen_getModule(qr0, x, y)) {
				// QRCode拡大
				for (int j = 0; j < zoom_in_scale; j++) {
					for (int i = 0; i < zoom_in_scale; i++) {
						qrcode_rgba_buff[start_of_image_height * zoom_in_scale * qrcode_original_width + j * zoom_in_scale * qrcode_original_width + start_of_image_width + i] = 255;
					}
				}
			}
		}
	}

	cv::Mat qr_image = cv::Mat(QRCODE_HEIGHT, QRCODE_WIDTH, CV_8UC1);
	memcpy(qr_image.data, qrcode_rgba_buff.data(), qrcode_rgba_buff.size() * sizeof(unsigned char));
	cv::imwrite("encode_y.bmp", qr_image);

	for (int y = 0; y < QRCODE_HEIGHT; y++) {
		int start_of_image_height = y * WIDTH;// qrcodeの開始インデックス（列）
		for (int x = 0; x < QRCODE_WIDTH; x++) {

			unsigned short dy_tmp = dy[start_of_image_height + start_of_image_width + x];// dy
			
			ycbcr[0] = dy[start_of_image_height + start_of_image_width + x];// dy
			const unsigned short* ptr_ycbcr = &ycbcr[0];
			unsigned short rgb[3] = { 0,0,0 };
			unsigned short* ptr_rgb = &rgb[0];
			YCbCrToRGB(&ptr_ycbcr, &ptr_rgb);
			rgb[0] = 255 * (rgb[0] - 64 + 1) / (940 - 64 + 1);
			rgb[1] = 255 * (rgb[1] - 64 + 1) / (940 - 64 + 1);
			rgb[2] = 255 * (rgb[2] - 64 + 1) / (940 - 64 + 1);

			qrcode_rgba_noise_buff[(y * QRCODE_WIDTH + x) * qrcode_bits_buff] = rgb[0];// R
			qrcode_rgba_noise_buff[(y * QRCODE_WIDTH + x) * qrcode_bits_buff + 1] = rgb[1];// G
			qrcode_rgba_noise_buff[(y * QRCODE_WIDTH + x) * qrcode_bits_buff + 2] = rgb[2];// B
		}
	}
	int type = 0;
	switch (qrcode_bits_buff) {
	case 1:
		type = CV_8UC1;
		break;
	case 2:
		type = CV_8UC2;
		break;
	case 3:
		type = CV_8UC3;
		break;
	case 4:
		type = CV_8UC4;
		break;
	}
	cv::Mat qr_noise_image = cv::Mat(QRCODE_HEIGHT, QRCODE_WIDTH, type);
	memcpy(qr_noise_image.data, qrcode_rgba_noise_buff.data(), qrcode_rgba_noise_buff.size() * sizeof(unsigned char));
	cv::imwrite("encode_noise_y.bmp", qr_noise_image);
#endif //  QRCODE_OUTPUT_BMP_DEBUG
	return 0;
}
/**
* QRコードを生成し、YCbCrデータに書き込む
* @param	frame_num	(in)		フレーム番号
* @param	dycbcr		(in/out)	YCbCrデータ
* @return	-1	NULLチェックエラー
*			 0	正常終了
*			 1	生成失敗
* @author	N.Shao
*/
int Encoder::QRCodeGeneratorByYUV(const unsigned int* frame_num, unsigned int* dycbcr) {
	// NULLチェック
	if (NULL == dycbcr)
		return -1;

	int width = WIDTH;// 画像横幅
	int height = HEIGHT;// 画像の縦幅

	// qrcode生成用のローカル変数
	char encode_txt[QRCODE_TEXT_LENGHT] = { '\0' };
	sprintf_s(encode_txt, "%u", *frame_num);
	uint8_t qr0[qrcodegen_BUFFER_LEN_MAX];// 生成qrcodeデータ格納buffer
	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

	if (!qrcodegen_encodeText(encode_txt, tempBuffer, qr0, _encode_param_ecl, _encode_param_version, _encode_param_version, _encode_param_mask, true))
		return 1;

	// オリジナルQRCodeのサイズ
	int qrcode_original_width = qrcodegen_getSize(qr0);
	int qrcode_original_height = qrcode_original_width;
	int zoom_in_scale = QRCODE_SIZE_SCALE; // 拡大の倍率

	unsigned short* qrcode_buff = new unsigned short[zoom_in_scale * qrcode_original_width * zoom_in_scale * qrcode_original_width]{ 0 };// qrcodeのYバイナリデータ
#pragma omp parallel for
	for (int y = 0; y < qrcode_original_height; y++) {
		int start_of_image_height = y * zoom_in_scale;
		for (int x = 0; x < qrcode_original_width; x++) {
			int start_of_image_width =  x * zoom_in_scale;
			// false:白 true:黒
			if (!qrcodegen_getModule(qr0, x, y)) {
				// QRCode拡大
				for (int j = 0; j < zoom_in_scale; j++) {
					for (int i = 0; i < zoom_in_scale; i++) {
						qrcode_buff[start_of_image_height * zoom_in_scale * qrcode_original_width + j * zoom_in_scale * qrcode_original_width + start_of_image_width + i] = 940;
					}
				}
			}
		}
	}

	int qrcode_width = QRCODE_WIDTH;// QRCodeの横（拡大後）
	int qrcode_height = qrcode_width;// QRCodeの縦（拡大後）
	int qrcode_four_words_width = qrcode_width / PIXEL_IN_FOUR_WORDS;

	int four_words_width = width / PIXEL_IN_FOUR_WORDS; // 8Kサイズを4word単位（4*int）に変換後の幅
	int four_words_size = width * height / PIXEL_IN_FOUR_WORDS;// 8Kサイズを4word単位（4*int）に変換後のサイズ

	for (int y = 0; y < qrcode_height; y++) {
		for (int x = 0; x < qrcode_four_words_width; x++) {
			unsigned int words[4] = { 0x0,0x0,0x0,0x0 };
			// 黒（YCbCr:64 512 512）で初期化 
			unsigned int Y[6] = { YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y };
			unsigned int Cb[3] = { YUV_BLACK_UV,YUV_BLACK_UV,YUV_BLACK_UV };
			unsigned int Cr[3] = { YUV_BLACK_UV,YUV_BLACK_UV,YUV_BLACK_UV };
			int index_qrcode_buff = y * qrcode_four_words_width + x;// qrcode_buffのindex

			// 白YCbCr:940 512 512
			// 6 pixel in 4words
			for (int ix_pix = 0; ix_pix < PIXEL_IN_FOUR_WORDS; ix_pix++) {
				if (qrcode_buff[index_qrcode_buff * PIXEL_IN_FOUR_WORDS + ix_pix] == YUV_WHITE_Y) {
					Y[ix_pix] = YUV_WHITE_Y;
				}
			}

			// word 0
			words[0] = (Cr[0] << 20); // Cr0
			words[0] = (Y[0] << 10) | words[0]; // Y0
			words[0] = (Cb[0] << 0) | words[0]; // Cb0

			// word 1
			words[1] = (Y[2] << 20); // Y2
			words[1] = (Cb[1] << 10) | words[1]; // Cb2
			words[1] = (Y[1] << 0) | words[1]; // Y1

			// word 2
			words[2] = (Cb[2] << 20); // Cb4
			words[2] = (Y[3] << 10) | words[2]; // Y3
			words[2] = (Cr[1] << 0) | words[2]; // Cr2

			// word 3
			words[3] = (Y[5] << 20); // Y5
			words[3] = (Cr[2] << 10) | words[3]; // Cr4
			words[3] = (Y[4] << 0) | words[3]; // Y4
			
			// yuvデータにセット、一回4 intをセットする
			int qrcode_start_of_width = four_words_width - qrcode_four_words_width + y * four_words_width + x;// yuvデータでqrcodeの開始index
			dycbcr[qrcode_start_of_width * 4] = words[0];
			dycbcr[qrcode_start_of_width * 4 + 1] = words[1];
			dycbcr[qrcode_start_of_width * 4 + 2] = words[2];
			dycbcr[qrcode_start_of_width * 4 + 3] = words[3];
		}
	}

#ifdef  QRCODE_OUTPUT_BMP_DEBUG // Bmp生成
	int qrcode_bits_buff = 3;
	std::vector<unsigned char> qrcode_rgba_buff(QRCODE_WIDTH * QRCODE_HEIGHT, 0); // qrcode rgba バッファ
	std::vector<unsigned char> qrcode_rgba_noise_buff(QRCODE_WIDTH* QRCODE_HEIGHT * qrcode_bits_buff, 0); // qrcode rgba バッファ
	for (int y = 0; y < qrcode_original_height; y++) {
		int start_of_image_height = y * zoom_in_scale;
		for (int x = 0; x < qrcode_original_width; x++) {
			int start_of_image_width = x * zoom_in_scale;
			if (!qrcodegen_getModule(qr0, x, y)) {
				// QRCode拡大
				for (int j = 0; j < zoom_in_scale; j++) {
					for (int i = 0; i < zoom_in_scale; i++) {
						qrcode_rgba_buff[start_of_image_height * zoom_in_scale * qrcode_original_width + j * zoom_in_scale * qrcode_original_width + start_of_image_width + i] = 255;
					}
				}
			}
		}
	}

	cv::Mat qr_image = cv::Mat(QRCODE_HEIGHT, QRCODE_WIDTH, CV_8UC1);
	memcpy(qr_image.data, qrcode_rgba_buff.data(), qrcode_rgba_buff.size() * sizeof(unsigned char));
	cv::imwrite("encode_yuv.bmp", qr_image);

	for (int y = 0; y < qrcode_height; y++) {
		for (int x = 0; x < qrcode_four_words_width; x++) {
			unsigned int words[4] = { 0x0,0x0,0x0,0x0 };

			// 黒（YCbCr:64 512 512）で初期化 
			unsigned int Y[6] = { YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y };
			unsigned int Cb[3] = { YUV_BLACK_UV,YUV_BLACK_UV,YUV_BLACK_UV };
			unsigned int Cr[3] = { YUV_BLACK_UV,YUV_BLACK_UV,YUV_BLACK_UV };

			int qrcode_start_of_width = four_words_width - qrcode_four_words_width + y * four_words_width + x;// yuvデータでqrcodeの開始index
			words[0] = dycbcr[qrcode_start_of_width * 4];
			words[1] = dycbcr[qrcode_start_of_width * 4 + 1];
			words[2] = dycbcr[qrcode_start_of_width * 4 + 2];
			words[3] = dycbcr[qrcode_start_of_width * 4 + 3];

			// word 0
			Cb[0] = words[0] & 0x3FF;// Cb0
			Y[0] = (words[0] >> 10) & 0x3FF;// Y0
			Cr[0] = (words[0] >> 20) & 0x3FF;// Cr0

			// word 1
			Y[1] = words[1] & 0x3FF;// Y1
			Cb[1] = (words[1] >> 10) & 0x3FF;// Cb2
			Y[2] = (words[1] >> 20) & 0x3FF;// Y2

			// word 2
			Cr[1] = words[2] & 0x3FF;// Cr2
			Y[3] = (words[2] >> 10) & 0x3FF;// Y3
			Cb[2] = (words[2] >> 20) & 0x3FF;// Cb4

			// word 3
			Y[4] = words[3] & 0x3FF; // Y4
			Cr[2] = (words[3] >> 10) & 0x3FF;  // Cr4
			Y[5] = (words[3] >> 20) & 0x3FF; // Y5

			int index_qrcode_four_words = y * qrcode_four_words_width + x;
			
			// 白YCbCr:940 512 512
			// pixel 0 in 4 words
			for (int ipixel = 0; ipixel < PIXEL_IN_FOUR_WORDS; ipixel++) {
				unsigned short ycbcr[3] = { YUV_BLACK_Y, YUV_BLACK_UV ,YUV_BLACK_UV };
				ycbcr[0] = Y[ipixel];// dy
				const unsigned short* ptr_ycbcr = &ycbcr[0];
				unsigned short rgb[3] = { 0,0,0 };
				unsigned short* ptr_rgb = &rgb[0];
				YCbCrToRGB(&ptr_ycbcr, &ptr_rgb);

				rgb[0] = 255 * (rgb[0] - 64 + 1) / (940 - 64 + 1);
				rgb[1] = 255 * (rgb[1] - 64 + 1) / (940 - 64 + 1);
				rgb[2] = 255 * (rgb[2] - 64 + 1) / (940 - 64 + 1);

				int index_qrcode_pixel = index_qrcode_four_words * PIXEL_IN_FOUR_WORDS + ipixel;
				int index_qrcode_rgba = index_qrcode_pixel * qrcode_bits_buff;
				qrcode_rgba_noise_buff[index_qrcode_rgba] = rgb[0];
				qrcode_rgba_noise_buff[index_qrcode_rgba + 1] = rgb[1];
				qrcode_rgba_noise_buff[index_qrcode_rgba + 2] = rgb[2];
			}
		}
	}
	int type = 0;
	switch (qrcode_bits_buff) {
	case 1:
		type = CV_8UC1;
		break;
	case 2:
		type = CV_8UC2;
		break;
	case 3:
		type = CV_8UC3;
		break;
	case 4:
		type = CV_8UC4;
		break;
	}
	cv::Mat qr_noise_image = cv::Mat(QRCODE_HEIGHT, QRCODE_WIDTH, type);
	memcpy(qr_noise_image.data, qrcode_rgba_noise_buff.data(), qrcode_rgba_noise_buff.size() * sizeof(unsigned char));
	cv::imwrite("encode_noise_yuv.bmp", qr_noise_image);
#endif //  QRCODE_OUTPUT_BMP_DEBUG

	delete[] qrcode_buff;
	return 0;
}

Decoder::Decoder() {

}
Decoder::~Decoder() {

}
/**
* QRコード読み取り
* @param	y			(in)			yデータ
* @param	frame_num	(in/out)		フレーム番号
* @param	image_width	(in)			映像の横幅（7680 3840）
* @return	-2	横幅が6で割り切れない
*			-1	NULLチェックエラー
*			 0	正常終了
*			 1	空の文字列
*			 2	QRCode読み取り失敗
* @author	N.Shao
*/
int Decoder::QRCodeScanner(const unsigned short* dy, unsigned int* frame_num, int image_width) {
	// NULLチェック
	if (NULL == dy)
		return -1;

	int qrcode_width = QRCODE_WIDTH;// QRCodeの横（拡大後）
	int qrcode_height = qrcode_width;// QRCodeの縦（拡大後）

	int qrcode_bits_buff = 1;
	std::vector<unsigned char> qrcode_rgba_buff(qrcode_width * qrcode_height * qrcode_bits_buff, 0); // qrcode rgba バッファ

	int start_of_image_width = image_width - qrcode_width;// qrcodeの開始インデックス（行）
	unsigned short dy_tmp = 0;// dy

	for (int y = 0; y < QRCODE_HEIGHT; y++) {
		int start_of_image_height = y * image_width;// qrcodeの開始インデックス（列）
		for (int x = 0; x < QRCODE_WIDTH; x++) {

			dy_tmp = dy[start_of_image_height + start_of_image_width + x];// dy

			if (dy_tmp >= Y_THRESHOLD) {
				qrcode_rgba_buff[(y * qrcode_width + x) * qrcode_bits_buff] = 255;// 輝度
			}
			else {
				qrcode_rgba_buff[(y * qrcode_width + x) * qrcode_bits_buff] = 0;// 輝度
			}
		}
	}

#ifdef  QRCODE_OUTPUT_BMP_DEBUG
	int type = 0;
	switch (qrcode_bits_buff) {
	case 1:
		type = CV_8UC1;
		break;
	case 2:
		type = CV_8UC2;
		break;
	case 3:
		type = CV_8UC3;
		break;
	case 4:
		type = CV_8UC4;
		break;
	}
	cv::Mat qr_image = cv::Mat(QRCODE_HEIGHT, QRCODE_WIDTH, type);
	memcpy(qr_image.data, qrcode_rgba_buff.data(), qrcode_rgba_buff.size() * sizeof(unsigned char));
	cv::imwrite("decode_y.bmp", qr_image);
#endif //  QRCODE_OUTPUT_BMP_DEBUG

	// sourceセット
	ArrayRef<char> image = ArrayRef<char>(QRCODE_WIDTH * QRCODE_HEIGHT * qrcode_bits_buff);
	memcpy(&image[0], &qrcode_rgba_buff[0], image->size());

	Ref<LuminanceSource> source(new ImageReaderSource(image, QRCODE_WIDTH, QRCODE_HEIGHT, qrcode_bits_buff));
	Ref<Binarizer> binarizer(new GlobalHistogramBinarizer(source));

	// ヒットタイプ
	DecodeHints hints(DecodeHints::QR_CODE_HINT);
	Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));

	// 読み取り
	Ref<Reader> reader(new MultiFormatReader);
	try {
		Ref<Result> result = reader->decode(binary, hints);
		std::string data = result->getText()->getText();
		if (data.length() <= 0)
			return 1;
		*frame_num = static_cast<unsigned int>(atoll(data.c_str()));
	}
	catch (const std::exception&) {
		return 2;
	}
	return 0;
}

/**
* YCbCrデータからQRコードを読み取り、フレーム番号を取得
* @param	dycbcr		(in)			YCbCrデータ（unpackされないデータ）
* @param	frame_num	(in/out)		フレーム番号
* @param	image_width	(in)			映像の横幅（7680 3840）
* @return	-2	横幅が6で割り切れない
*			-1	NULLチェックエラー
*			 0	正常終了
*			 1	空の文字列
*			 2	QRCode読み取り失敗
* @author	N.Shao
*/
int Decoder::QRCodeScannerByYUV(unsigned int* dycbcr, unsigned int* frame_num, int image_width) {
	// NULLチェック
	if (NULL == dycbcr)
		return -1;

	if (image_width % PIXEL_IN_FOUR_WORDS != 0)
		return -2;

	int qrcode_width = QRCODE_WIDTH;// QRCodeの横（拡大後）
	int qrcode_height = qrcode_width;// QRCodeの縦（拡大後）

	int qrcode_bits_buff = 1;
	std::vector<unsigned char> qrcode_rgba_buff(qrcode_width * qrcode_height * qrcode_bits_buff, 0); // qrcode rgbaのバッファ（黒で初期化）

	int qrcode_four_words_width = qrcode_width / PIXEL_IN_FOUR_WORDS;
	int four_words_width = image_width / PIXEL_IN_FOUR_WORDS; // 4word単位での横幅
	
	unsigned int words[4] = { 0x0,0x0,0x0,0x0 }; // 4words一時変数
	// 黒（YCbCr:64 512 512）で初期化 
	unsigned int Y[6] = { YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y };

	for (int y = 0; y < qrcode_height; y++) {
		for (int x = 0; x < qrcode_four_words_width; x++) {
			int qrcode_start_of_width = four_words_width - qrcode_four_words_width + y * four_words_width + x;// yuvデータでqrcodeの開始index
			words[0] = dycbcr[qrcode_start_of_width * 4];
			words[1] = dycbcr[qrcode_start_of_width * 4 + 1];
			words[2] = dycbcr[qrcode_start_of_width * 4 + 2];
			words[3] = dycbcr[qrcode_start_of_width * 4 + 3];

			// word 0
			Y[0] = (words[0] >> 10) & 0x3FF;// Y0

			// word 1
			Y[1] = words[1] & 0x3FF;// Y1
			Y[2] = (words[1] >> 20) & 0x3FF;// Y2

			// word 2
			Y[3] = (words[2] >> 10) & 0x3FF;// Y3

			// word 3
			Y[4] = words[3] & 0x3FF; // Y4
			Y[5] = (words[3] >> 20) & 0x3FF;// Y5

			int index_qrcode_four_words = y * qrcode_four_words_width + x;
			// 白YCbCr:940 512 512
			// pixel 0 in 4 words
			for (int ipixel = 0; ipixel < PIXEL_IN_FOUR_WORDS; ipixel++) {
				int index_qrcode_pixel = index_qrcode_four_words * PIXEL_IN_FOUR_WORDS + ipixel;
				int index_qrcode_rgba = index_qrcode_pixel * qrcode_bits_buff;
				// Yの閾値によって白・黒に変更
				if (Y[ipixel] >= Y_THRESHOLD ) {
					qrcode_rgba_buff[index_qrcode_rgba] = 255;// 輝度
				}
				else {
					qrcode_rgba_buff[index_qrcode_rgba] = 0;// 輝度
				}
			}
		}
	}

#ifdef  QRCODE_OUTPUT_BMP_DEBUG
	int type = 0;
	switch (qrcode_bits_buff) {
	case 1 : 
		type = CV_8UC1;
		break;
	case 2:
		type = CV_8UC2;
		break;
	case 3:
		type = CV_8UC3;
		break;
	case 4:
		type = CV_8UC4;
		break;
	}
	cv::Mat qr_image = cv::Mat(QRCODE_HEIGHT, QRCODE_WIDTH, type);
	memcpy(qr_image.data, qrcode_rgba_buff.data(), qrcode_rgba_buff.size() * sizeof(unsigned char));
	cv::imwrite("decode_yuv.bmp", qr_image);
#endif //  QRCODE_OUTPUT_BMP_DEBUG

	// sourceセット
	ArrayRef<char> image = ArrayRef<char>(QRCODE_WIDTH * QRCODE_HEIGHT * qrcode_bits_buff);
	memcpy(&image[0], &qrcode_rgba_buff[0], image->size());

	Ref<LuminanceSource> source(new ImageReaderSource(image, QRCODE_WIDTH, QRCODE_HEIGHT, qrcode_bits_buff));
	Ref<Binarizer> binarizer(new GlobalHistogramBinarizer(source));

	// ヒットタイプ
	DecodeHints hints(DecodeHints::QR_CODE_HINT);
	Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));

	// 読み取り
	Ref<Reader> reader(new MultiFormatReader);
	try {
		Ref<Result> result = reader->decode(binary, hints);
		std::string data = result->getText()->getText();
		if (data.length() <= 0)
			return 1;
		*frame_num = static_cast<unsigned int>(atoll(data.c_str()));
	}
	catch (const std::exception&) {
		return 2;
	}
	return 0;
}
/**
* 10BitデジタルYCbCrから10BitのRGBに変換
* @param	dycbcr		(in)			ycbcrデータ
* @param	dRGB		(in/out)		rbgデータ
* @return	-1	引数NULL
*			 0	正常終了
* @author	N.Shao
*/
int YCbCrToRGB(const unsigned short** dycbcr, unsigned short** dRGB) {
	if (NULL == *dycbcr || NULL == *dRGB)
		return -1;

	unsigned short dy = (*dycbcr)[0];
	unsigned short dcb = (*dycbcr)[1];
	unsigned short dcr = (*dycbcr)[2];

	unsigned short* dR = dRGB[0];
	unsigned short* dG = dRGB[1];
	unsigned short* dB = dRGB[2];

	double analogYCbCr[3] = { 0.0 };// ycbcr
	double analogRGB[3] = { 0.0 };

	// YCbCrデジタルからアナログに変換
	// Y'=(DY'-64)/876
	analogYCbCr[0] = static_cast<double>((dy - 64.0) / 876.0);
	// Cb'=(DCb'-512)/896
	analogYCbCr[1] = static_cast<double>((dcb - 512.0) / 896.0);
	// Cr'= (DCr'-512)/896
	analogYCbCr[2] = static_cast<double>((dcr - 512.0) / 896.0);

	// YCbCrからRGBに変換
	// R'=Y+1.4746 * Cr'
	analogRGB[0] = analogYCbCr[0] + 1.4746 * analogYCbCr[2];
	// G'=Y'-(0.0593 * 1.8814 * Cb' + 0.2627f * 1.4746 * Cr')/0.678
	analogRGB[1] = analogYCbCr[0] - (0.0593 * 1.8814 * analogYCbCr[1] + 0.2627 * 1.4746 * analogYCbCr[2] / 0.678);
	// B'= Y'+ 1.8814 * Cb'
	analogRGB[2] = analogYCbCr[0] + 1.88140 * analogYCbCr[1];

	// RGBアナログからデジタルに変換
	// DR'=64+876*R'
	(*dRGB)[0] = static_cast<unsigned short>(64 + 876 * analogRGB[0] + 0.5);
	// DG'=64+876*G'
	(*dRGB)[1] = static_cast<unsigned short>(64 + 876 * analogRGB[1] + 0.5);
	// DB'=64+876*B'
	(*dRGB)[2] = static_cast<unsigned short>(64 + 876 * analogRGB[2] + 0.5);

	return 0;
}
}