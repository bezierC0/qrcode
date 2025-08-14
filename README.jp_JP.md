

## 日本語

### 概要
QRコードの生成と読み取りを行うC言語ライブラリです。主な機能：
- **QRコード生成**：テキストやバイナリデータからQRコードを作成
- **QRコード読み取り**：ZXing統合により画像データからQRコードを復号化

### 特徴
- ✅ 全QRコードバージョン（1-40）対応
- ✅ 全4種類の誤り訂正レベル（低、中、四分位、高）
- ✅ 複数のエンコードモード（数値、英数字、バイト、漢字、ECI）
- ✅ 自動マスクパターン選択
- ✅ QRコード復号化のための画像読み取り機能
- ✅ スレッドセーフ操作
- ✅ ヒープメモリ割り当て不要

### クイックスタート

#### QRコード生成
```c
#include "qrcodegen.h"

// テキストエンコード
uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

bool ok = qrcodegen_encodeText("こんにちは、世界！", tempBuffer, qrcode,
    qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX,
    qrcodegen_Mask_AUTO, true);

if (ok) {
    int size = qrcodegen_getSize(qrcode);
    // qrcodegen_getModule(qrcode, x, y) で個別モジュールにアクセス
}
```

### APIリファレンス

#### 主要関数
- `qrcodegen_encodeText()` - テキスト文字列をQRコードにエンコード
- `qrcodegen_encodeBinary()` - バイナリデータをQRコードにエンコード
- `qrcodegen_encodeSegments()` - カスタムセグメントをエンコード
- `qrcodegen_getSize()` - QRコードサイズを取得
- `qrcodegen_getModule()` - 座標のモジュール状態を取得

#### 誤り訂正レベル
- `qrcodegen_Ecc_LOW` - ~7% 誤り訂正
- `qrcodegen_Ecc_MEDIUM` - ~15% 誤り訂正
- `qrcodegen_Ecc_QUARTILE` - ~25% 誤り訂正
- `qrcodegen_Ecc_HIGH` - ~30% 誤り訂正

### ライセンス
このプロジェクトは以下のコードを使用：
- Project NayukiのQRコード生成ライブラリ（MITライセンス）
- ZXingライブラリコンポーネント（Apache License 2.0）

---
