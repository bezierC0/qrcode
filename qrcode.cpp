#include "qrcode.h"
namespace QRCode {
Encoder::Encoder() {
}
Encoder::~Encoder() {

}
/**
* QR�R�[�h�����AY�f�[�^�ɏ�������
* @param	frame_num	(in)		�t���[���ԍ�
* @param	y			(in/out)	y�f�[�^
* @return	-1	NULL�`�F�b�N�G���[
*			 0	����I��
*			 1	�������s
* @author	N.Shao
*/
int Encoder::QRCodeGenerator(const unsigned int* frame_num, unsigned short* dy) {
	// NULL�`�F�b�N
	if (NULL == dy)
		return -1;

	// qrcode�����p�̃��[�J���ϐ�
	char encode_txt[QRCODE_TEXT_LENGHT] = { '\0' };
	sprintf_s(encode_txt, "%u", *frame_num);
	uint8_t qr0[qrcodegen_BUFFER_LEN_MAX];// ����qrcode�f�[�^�i�[buffer
	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

	if (!qrcodegen_encodeText(encode_txt, tempBuffer, qr0, _encode_param_ecl, _encode_param_version, _encode_param_version, _encode_param_mask, true))
		return 1;

	// �I���W�i��QRCode�̃T�C�Y
	int qrcode_original_width = qrcodegen_getSize(qr0);
	int qrcode_original_height = qrcode_original_width;

	int zoom_in_scale = QRCODE_WIDTH / qrcode_original_width; // �g��̔{��

	for (int y = 0; y < qrcode_original_height; y++) {
		int start_of_image_height = y * zoom_in_scale;
		for (int x = 0; x < qrcode_original_width; x++) {
			int start_of_image_width = WIDTH - qrcode_original_width * zoom_in_scale + x * zoom_in_scale;
			// false:�� true:��
			if (!qrcodegen_getModule(qr0, x, y)) {
				// QRCode�g��
				for (int j = 0; j < zoom_in_scale; j++) {
					for (int i = 0; i < zoom_in_scale; i++) {
						// ��YCbCr:940 512 512
						// ��YCbCr:64 512 512
						// 8K�E���������Ȃ̂ŁA���݂̂��Z�b�g����
						dy[start_of_image_height * WIDTH + j * WIDTH + start_of_image_width + i] = YUV_WHITE_Y;

					}
				}
			}
			else {
				for (int j = 0; j < zoom_in_scale; j++) {
					for (int i = 0; i < zoom_in_scale; i++) {
						// ��YCbCr:940 512 512
						// ��YCbCr:64 512 512
						dy[start_of_image_height * WIDTH + j * WIDTH + start_of_image_width + i] = YUV_BLACK_Y;

					}
				}
			}
			
		}
	}
#ifdef  QRCODE_OUTPUT_BMP_DEBUG // Bmp����
	int qrcode_bits_buff = 3;
	std::vector<unsigned char> qrcode_rgba_buff(QRCODE_WIDTH * QRCODE_HEIGHT, 0); // qrcode rgba �o�b�t�@
	std::vector<unsigned char> qrcode_rgba_noise_buff(QRCODE_WIDTH * QRCODE_HEIGHT * qrcode_bits_buff, 0); // qrcode rgba �o�b�t�@

	int start_of_image_width = WIDTH - QRCODE_WIDTH;// qrcode�̊J�n�C���f�b�N�X�i�s�j
	unsigned short ycbcr[3] = { YUV_BLACK_Y, YUV_BLACK_UV ,YUV_BLACK_UV };

	for (int y = 0; y < qrcode_original_height; y++) {
		int start_of_image_height = y * zoom_in_scale;
		for (int x = 0; x < qrcode_original_width; x++) {
			int start_of_image_width = x * zoom_in_scale;
			if (!qrcodegen_getModule(qr0, x, y)) {
				// QRCode�g��
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
		int start_of_image_height = y * WIDTH;// qrcode�̊J�n�C���f�b�N�X�i��j
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
* QR�R�[�h�𐶐����AYCbCr�f�[�^�ɏ�������
* @param	frame_num	(in)		�t���[���ԍ�
* @param	dycbcr		(in/out)	YCbCr�f�[�^
* @return	-1	NULL�`�F�b�N�G���[
*			 0	����I��
*			 1	�������s
* @author	N.Shao
*/
int Encoder::QRCodeGeneratorByYUV(const unsigned int* frame_num, unsigned int* dycbcr) {
	// NULL�`�F�b�N
	if (NULL == dycbcr)
		return -1;

	int width = WIDTH;// �摜����
	int height = HEIGHT;// �摜�̏c��

	// qrcode�����p�̃��[�J���ϐ�
	char encode_txt[QRCODE_TEXT_LENGHT] = { '\0' };
	sprintf_s(encode_txt, "%u", *frame_num);
	uint8_t qr0[qrcodegen_BUFFER_LEN_MAX];// ����qrcode�f�[�^�i�[buffer
	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

	if (!qrcodegen_encodeText(encode_txt, tempBuffer, qr0, _encode_param_ecl, _encode_param_version, _encode_param_version, _encode_param_mask, true))
		return 1;

	// �I���W�i��QRCode�̃T�C�Y
	int qrcode_original_width = qrcodegen_getSize(qr0);
	int qrcode_original_height = qrcode_original_width;
	int zoom_in_scale = QRCODE_SIZE_SCALE; // �g��̔{��

	unsigned short* qrcode_buff = new unsigned short[zoom_in_scale * qrcode_original_width * zoom_in_scale * qrcode_original_width]{ 0 };// qrcode��Y�o�C�i���f�[�^
#pragma omp parallel for
	for (int y = 0; y < qrcode_original_height; y++) {
		int start_of_image_height = y * zoom_in_scale;
		for (int x = 0; x < qrcode_original_width; x++) {
			int start_of_image_width =  x * zoom_in_scale;
			// false:�� true:��
			if (!qrcodegen_getModule(qr0, x, y)) {
				// QRCode�g��
				for (int j = 0; j < zoom_in_scale; j++) {
					for (int i = 0; i < zoom_in_scale; i++) {
						qrcode_buff[start_of_image_height * zoom_in_scale * qrcode_original_width + j * zoom_in_scale * qrcode_original_width + start_of_image_width + i] = 940;
					}
				}
			}
		}
	}

	int qrcode_width = QRCODE_WIDTH;// QRCode�̉��i�g���j
	int qrcode_height = qrcode_width;// QRCode�̏c�i�g���j
	int qrcode_four_words_width = qrcode_width / PIXEL_IN_FOUR_WORDS;

	int four_words_width = width / PIXEL_IN_FOUR_WORDS; // 8K�T�C�Y��4word�P�ʁi4*int�j�ɕϊ���̕�
	int four_words_size = width * height / PIXEL_IN_FOUR_WORDS;// 8K�T�C�Y��4word�P�ʁi4*int�j�ɕϊ���̃T�C�Y

	for (int y = 0; y < qrcode_height; y++) {
		for (int x = 0; x < qrcode_four_words_width; x++) {
			unsigned int words[4] = { 0x0,0x0,0x0,0x0 };
			// ���iYCbCr:64 512 512�j�ŏ����� 
			unsigned int Y[6] = { YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y };
			unsigned int Cb[3] = { YUV_BLACK_UV,YUV_BLACK_UV,YUV_BLACK_UV };
			unsigned int Cr[3] = { YUV_BLACK_UV,YUV_BLACK_UV,YUV_BLACK_UV };
			int index_qrcode_buff = y * qrcode_four_words_width + x;// qrcode_buff��index

			// ��YCbCr:940 512 512
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
			
			// yuv�f�[�^�ɃZ�b�g�A���4 int���Z�b�g����
			int qrcode_start_of_width = four_words_width - qrcode_four_words_width + y * four_words_width + x;// yuv�f�[�^��qrcode�̊J�nindex
			dycbcr[qrcode_start_of_width * 4] = words[0];
			dycbcr[qrcode_start_of_width * 4 + 1] = words[1];
			dycbcr[qrcode_start_of_width * 4 + 2] = words[2];
			dycbcr[qrcode_start_of_width * 4 + 3] = words[3];
		}
	}

#ifdef  QRCODE_OUTPUT_BMP_DEBUG // Bmp����
	int qrcode_bits_buff = 3;
	std::vector<unsigned char> qrcode_rgba_buff(QRCODE_WIDTH * QRCODE_HEIGHT, 0); // qrcode rgba �o�b�t�@
	std::vector<unsigned char> qrcode_rgba_noise_buff(QRCODE_WIDTH* QRCODE_HEIGHT * qrcode_bits_buff, 0); // qrcode rgba �o�b�t�@
	for (int y = 0; y < qrcode_original_height; y++) {
		int start_of_image_height = y * zoom_in_scale;
		for (int x = 0; x < qrcode_original_width; x++) {
			int start_of_image_width = x * zoom_in_scale;
			if (!qrcodegen_getModule(qr0, x, y)) {
				// QRCode�g��
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

			// ���iYCbCr:64 512 512�j�ŏ����� 
			unsigned int Y[6] = { YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y };
			unsigned int Cb[3] = { YUV_BLACK_UV,YUV_BLACK_UV,YUV_BLACK_UV };
			unsigned int Cr[3] = { YUV_BLACK_UV,YUV_BLACK_UV,YUV_BLACK_UV };

			int qrcode_start_of_width = four_words_width - qrcode_four_words_width + y * four_words_width + x;// yuv�f�[�^��qrcode�̊J�nindex
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
			
			// ��YCbCr:940 512 512
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
* QR�R�[�h�ǂݎ��
* @param	y			(in)			y�f�[�^
* @param	frame_num	(in/out)		�t���[���ԍ�
* @param	image_width	(in)			�f���̉����i7680 3840�j
* @return	-2	������6�Ŋ���؂�Ȃ�
*			-1	NULL�`�F�b�N�G���[
*			 0	����I��
*			 1	��̕�����
*			 2	QRCode�ǂݎ�莸�s
* @author	N.Shao
*/
int Decoder::QRCodeScanner(const unsigned short* dy, unsigned int* frame_num, int image_width) {
	// NULL�`�F�b�N
	if (NULL == dy)
		return -1;

	int qrcode_width = QRCODE_WIDTH;// QRCode�̉��i�g���j
	int qrcode_height = qrcode_width;// QRCode�̏c�i�g���j

	int qrcode_bits_buff = 1;
	std::vector<unsigned char> qrcode_rgba_buff(qrcode_width * qrcode_height * qrcode_bits_buff, 0); // qrcode rgba �o�b�t�@

	int start_of_image_width = image_width - qrcode_width;// qrcode�̊J�n�C���f�b�N�X�i�s�j
	unsigned short dy_tmp = 0;// dy

	for (int y = 0; y < QRCODE_HEIGHT; y++) {
		int start_of_image_height = y * image_width;// qrcode�̊J�n�C���f�b�N�X�i��j
		for (int x = 0; x < QRCODE_WIDTH; x++) {

			dy_tmp = dy[start_of_image_height + start_of_image_width + x];// dy

			if (dy_tmp >= Y_THRESHOLD) {
				qrcode_rgba_buff[(y * qrcode_width + x) * qrcode_bits_buff] = 255;// �P�x
			}
			else {
				qrcode_rgba_buff[(y * qrcode_width + x) * qrcode_bits_buff] = 0;// �P�x
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

	// source�Z�b�g
	ArrayRef<char> image = ArrayRef<char>(QRCODE_WIDTH * QRCODE_HEIGHT * qrcode_bits_buff);
	memcpy(&image[0], &qrcode_rgba_buff[0], image->size());

	Ref<LuminanceSource> source(new ImageReaderSource(image, QRCODE_WIDTH, QRCODE_HEIGHT, qrcode_bits_buff));
	Ref<Binarizer> binarizer(new GlobalHistogramBinarizer(source));

	// �q�b�g�^�C�v
	DecodeHints hints(DecodeHints::QR_CODE_HINT);
	Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));

	// �ǂݎ��
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
* YCbCr�f�[�^����QR�R�[�h��ǂݎ��A�t���[���ԍ����擾
* @param	dycbcr		(in)			YCbCr�f�[�^�iunpack����Ȃ��f�[�^�j
* @param	frame_num	(in/out)		�t���[���ԍ�
* @param	image_width	(in)			�f���̉����i7680 3840�j
* @return	-2	������6�Ŋ���؂�Ȃ�
*			-1	NULL�`�F�b�N�G���[
*			 0	����I��
*			 1	��̕�����
*			 2	QRCode�ǂݎ�莸�s
* @author	N.Shao
*/
int Decoder::QRCodeScannerByYUV(unsigned int* dycbcr, unsigned int* frame_num, int image_width) {
	// NULL�`�F�b�N
	if (NULL == dycbcr)
		return -1;

	if (image_width % PIXEL_IN_FOUR_WORDS != 0)
		return -2;

	int qrcode_width = QRCODE_WIDTH;// QRCode�̉��i�g���j
	int qrcode_height = qrcode_width;// QRCode�̏c�i�g���j

	int qrcode_bits_buff = 1;
	std::vector<unsigned char> qrcode_rgba_buff(qrcode_width * qrcode_height * qrcode_bits_buff, 0); // qrcode rgba�̃o�b�t�@�i���ŏ������j

	int qrcode_four_words_width = qrcode_width / PIXEL_IN_FOUR_WORDS;
	int four_words_width = image_width / PIXEL_IN_FOUR_WORDS; // 4word�P�ʂł̉���
	
	unsigned int words[4] = { 0x0,0x0,0x0,0x0 }; // 4words�ꎞ�ϐ�
	// ���iYCbCr:64 512 512�j�ŏ����� 
	unsigned int Y[6] = { YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y,YUV_BLACK_Y };

	for (int y = 0; y < qrcode_height; y++) {
		for (int x = 0; x < qrcode_four_words_width; x++) {
			int qrcode_start_of_width = four_words_width - qrcode_four_words_width + y * four_words_width + x;// yuv�f�[�^��qrcode�̊J�nindex
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
			// ��YCbCr:940 512 512
			// pixel 0 in 4 words
			for (int ipixel = 0; ipixel < PIXEL_IN_FOUR_WORDS; ipixel++) {
				int index_qrcode_pixel = index_qrcode_four_words * PIXEL_IN_FOUR_WORDS + ipixel;
				int index_qrcode_rgba = index_qrcode_pixel * qrcode_bits_buff;
				// Y��臒l�ɂ���Ĕ��E���ɕύX
				if (Y[ipixel] >= Y_THRESHOLD ) {
					qrcode_rgba_buff[index_qrcode_rgba] = 255;// �P�x
				}
				else {
					qrcode_rgba_buff[index_qrcode_rgba] = 0;// �P�x
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

	// source�Z�b�g
	ArrayRef<char> image = ArrayRef<char>(QRCODE_WIDTH * QRCODE_HEIGHT * qrcode_bits_buff);
	memcpy(&image[0], &qrcode_rgba_buff[0], image->size());

	Ref<LuminanceSource> source(new ImageReaderSource(image, QRCODE_WIDTH, QRCODE_HEIGHT, qrcode_bits_buff));
	Ref<Binarizer> binarizer(new GlobalHistogramBinarizer(source));

	// �q�b�g�^�C�v
	DecodeHints hints(DecodeHints::QR_CODE_HINT);
	Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));

	// �ǂݎ��
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
* 10Bit�f�W�^��YCbCr����10Bit��RGB�ɕϊ�
* @param	dycbcr		(in)			ycbcr�f�[�^
* @param	dRGB		(in/out)		rbg�f�[�^
* @return	-1	����NULL
*			 0	����I��
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

	// YCbCr�f�W�^������A�i���O�ɕϊ�
	// Y'=(DY'-64)/876
	analogYCbCr[0] = static_cast<double>((dy - 64.0) / 876.0);
	// Cb'=(DCb'-512)/896
	analogYCbCr[1] = static_cast<double>((dcb - 512.0) / 896.0);
	// Cr'= (DCr'-512)/896
	analogYCbCr[2] = static_cast<double>((dcr - 512.0) / 896.0);

	// YCbCr����RGB�ɕϊ�
	// R'=Y+1.4746 * Cr'
	analogRGB[0] = analogYCbCr[0] + 1.4746 * analogYCbCr[2];
	// G'=Y'-(0.0593 * 1.8814 * Cb' + 0.2627f * 1.4746 * Cr')/0.678
	analogRGB[1] = analogYCbCr[0] - (0.0593 * 1.8814 * analogYCbCr[1] + 0.2627 * 1.4746 * analogYCbCr[2] / 0.678);
	// B'= Y'+ 1.8814 * Cb'
	analogRGB[2] = analogYCbCr[0] + 1.88140 * analogYCbCr[1];

	// RGB�A�i���O����f�W�^���ɕϊ�
	// DR'=64+876*R'
	(*dRGB)[0] = static_cast<unsigned short>(64 + 876 * analogRGB[0] + 0.5);
	// DG'=64+876*G'
	(*dRGB)[1] = static_cast<unsigned short>(64 + 876 * analogRGB[1] + 0.5);
	// DB'=64+876*B'
	(*dRGB)[2] = static_cast<unsigned short>(64 + 876 * analogRGB[2] + 0.5);

	return 0;
}
}