# Hướng Dẫn Flash ESP32 (Khi COM Port Bị Chiếm)

## File firmware đã build:
```
.pio/build/esp32dev/firmware.bin
```

## Cách 1: Dùng ESP Flash Download Tool (Khuyến nghị)

### Bước 1: Tải tool
1. Truy cập: https://www.espressif.com/en/support/download/other-tools
2. Tải "Flash Download Tools"
3. Giải nén và chạy `flash_download_tool_x.x.x.exe`

### Bước 2: Cấu hình
1. Chọn **ESP32** → **Develop**
2. Thêm file:
   - File: `.pio/build/esp32dev/firmware.bin`
   - Offset: `0x10000`
3. Chọn COM Port: `COM6`
4. Baud: `921600`
5. Click **START**

---

## Cách 2: Dùng esptool.py (Command line)

### Rút dây USB, cắm lại, chạy ngay:

```powershell
python -m esptool --chip esp32 --port COM6 --baud 921600 write_flash 0x10000 .pio/build/esp32dev/firmware.bin
```

---

## Cách 3: Upload từ PlatformIO (Đơn giản nhất)

### Đóng tất cả Serial Monitor, rồi:

```powershell
# Rút dây USB ESP32
# Đợi 3 giây
# Cắm lại
# Chạy ngay:
python -m platformio run --target upload
```

---

## Thay đổi trong code mới:

### ✅ Đã thêm bộ lọc Moving Average cho cảm biến độ ẩm
- Đọc 10 lần và lấy trung bình
- Lọc 5 mẫu liên tiếp
- Giảm nhiễu đáng kể

### ✅ Tạm thời TẮT điều khiển máy bơm tự động
- Máy bơm sẽ luôn TẮT
- Tránh bật/tắt liên tục do cảm biến không ổn định
- Để BẬT lại: Đổi `ENABLE_PUMP_CONTROL` thành `true` trong `src/main.cpp`

### ⚠️ Quạt vẫn hoạt động bình thường
- Quạt chỉ bật khi nhiệt độ > 55°C
- Hiện tại nhiệt độ ~32°C nên quạt TẮT (đúng)

---

## Sau khi flash xong:

### Kiểm tra Serial Monitor:
```
[DEBUG] Moisture ADC raw: XXXX, filtered: YYYY
```

Giá trị `filtered` sẽ ổn định hơn nhiều!

---

## Khắc phục lâu dài:

### 1. Kiểm tra dây cảm biến độ ẩm:
- VCC → ESP32 **3V3** (KHÔNG phải 5V!)
- GND → ESP32 **GND**
- AO → ESP32 **GPIO 34**

### 2. Thêm tụ lọc:
- Hàn tụ 100nF giữa VCC và GND của cảm biến
- Giảm nhiễu điện

### 3. Đổi GPIO (nếu vẫn không ổn):
- Thử GPIO 35 hoặc 36
- Tránh nhiễu từ WiFi

### 4. Thay cảm biến:
- Nếu vẫn nhảy lung tung, cảm biến có thể bị hỏng
- Thử cảm biến khác

---

## Bật lại máy bơm tự động:

Khi cảm biến đã ổn định, đổi trong `src/main.cpp`:

```cpp
#define ENABLE_PUMP_CONTROL true  // Đổi từ false → true
```

Rồi build và upload lại.
