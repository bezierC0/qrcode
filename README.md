# QR Code Generator & Reader Library

A comprehensive C library for generating and reading QR codes, supporting all QR Code Model 2 specifications from version 1 to 40.

[English](./README.md) | [中文](./README.zh_CN.md) | [日本語](./README.jp_JP.md)
## English

### Overview
This library provides functionality for both generating and reading QR codes. It includes:
- **QR Code Generation**: Create QR codes from text or binary data
- **QR Code Reading**: Decode QR codes from image data using ZXing integration

### Features
- ✅ All QR Code versions (1-40) supported
- ✅ All 4 error correction levels (Low, Medium, Quartile, High)
- ✅ Multiple encoding modes (Numeric, Alphanumeric, Byte, Kanji, ECI)
- ✅ Automatic mask pattern selection
- ✅ Image reading capabilities for QR code decoding
- ✅ Thread-safe operations
- ✅ No heap allocation required

### Quick Start

#### Generating QR Codes
```c
#include "qrcodegen.h"

// Text encoding
uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

bool ok = qrcodegen_encodeText("Hello, World!", tempBuffer, qrcode,
    qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX,
    qrcodegen_Mask_AUTO, true);

if (ok) {
    int size = qrcodegen_getSize(qrcode);
    // Use qrcodegen_getModule(qrcode, x, y) to access individual modules
}
```

#### Reading QR Codes
```c
#include "ImageReaderSource.h"

// Create image source from raw image data
ImageReaderSource source(imageData, width, height, components);

// Process with ZXing decoder
// (Additional ZXing integration code required)
```

### API Reference

#### Main Functions
- `qrcodegen_encodeText()` - Encode text string to QR code
- `qrcodegen_encodeBinary()` - Encode binary data to QR code
- `qrcodegen_encodeSegments()` - Encode custom segments
- `qrcodegen_getSize()` - Get QR code size
- `qrcodegen_getModule()` - Get module state at coordinates

#### Error Correction Levels
- `qrcodegen_Ecc_LOW` - ~7% error correction
- `qrcodegen_Ecc_MEDIUM` - ~15% error correction
- `qrcodegen_Ecc_QUARTILE` - ~25% error correction
- `qrcodegen_Ecc_HIGH` - ~30% error correction

### License
This project uses code from:
- QR Code generator library by Project Nayuki (MIT License)
- ZXing library components (Apache License 2.0)


## Building

```bash
# Compile the library
gcc -c qrcodegen.c -o qrcodegen.o
gcc -c ImageReaderSource.cpp -o ImageReaderSource.o

# Link with your application
gcc your_app.c qrcodegen.o ImageReaderSource.o -o your_app
```

## Contributing

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## Support

For questions and issues, please refer to the original libraries' documentation:
- [QR Code generator by Project Nayuki](https://www.nayuki.io/page/qr-code-generator-library)
- [ZXing Project](https://github.com/zxing/zxing)
