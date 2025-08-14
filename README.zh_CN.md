## 中文

### 概述
这是一个用于生成和读取QR码的C语言库。主要功能包括：
- **QR码生成**：从文本或二进制数据创建QR码
- **QR码读取**：通过ZXing集成从图像数据解码QR码

### 特性
- ✅ 支持所有QR码版本（1-40）
- ✅ 支持所有4种纠错级别（低、中、四分位、高）
- ✅ 多种编码模式（数字、字母数字、字节、汉字、ECI）
- ✅ 自动掩码模式选择
- ✅ QR码解码的图像读取功能
- ✅ 线程安全操作
- ✅ 无需堆内存分配

### 快速开始

#### 生成QR码
```c
#include "qrcodegen.h"

// 文本编码
uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

bool ok = qrcodegen_encodeText("你好，世界！", tempBuffer, qrcode,
    qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX,
    qrcodegen_Mask_AUTO, true);

if (ok) {
    int size = qrcodegen_getSize(qrcode);
    // 使用 qrcodegen_getModule(qrcode, x, y) 访问单个模块
}
```

### API参考

#### 主要函数
- `qrcodegen_encodeText()` - 将文本字符串编码为QR码
- `qrcodegen_encodeBinary()` - 将二进制数据编码为QR码
- `qrcodegen_encodeSegments()` - 编码自定义段
- `qrcodegen_getSize()` - 获取QR码大小
- `qrcodegen_getModule()` - 获取坐标处的模块状态

#### 纠错级别
- `qrcodegen_Ecc_LOW` - ~7% 纠错
- `qrcodegen_Ecc_MEDIUM` - ~15% 纠错
- `qrcodegen_Ecc_QUARTILE` - ~25% 纠错
- `qrcodegen_Ecc_HIGH` - ~30% 纠错

### 许可证
本项目使用以下代码：
- Project Nayuki的QR码生成库（MIT许可证）
- ZXing库组件（Apache许可证2.0）
