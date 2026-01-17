# HƯỚNG DẪN NẠP CODE MỚI - HỆ THỐNG 3 PHA

## 🎯 TỔNG QUAN

Hệ thống BioLoop Monitor đã được nâng cấp với **điều khiển thích ứng 3 pha tự động**. Tài liệu này hướng dẫn bạn nạp code mới vào ESP32 và kiểm tra hoạt động.

---

## 📋 CHUẨN BỊ

### Phần cứng:
- ✅ ESP32 DevKit
- ✅ Cáp USB (kết nối ESP32 với máy tính)
- ✅ Cảm biến DS18B20 (nhiệt độ) - GPIO 21
- ✅ Cảm biến độ ẩm V2.0-TH126 - GPIO 35
- ✅ Relay 2 kênh ACTIVE HIGH - GPIO 26, 27

### Phần mềm:
- ✅ PlatformIO đã cài đặt
- ✅ Visual Studio Code
- ✅ Driver USB-to-Serial (CH340/CP2102)

---

## 🚀 BƯỚC 1: NẠP CODE VÀO ESP32

### 1.1. Mở Terminal trong VS Code
```
Ctrl + ` (phím backtick)
```

### 1.2. Di chuyển đến thư mục dự án
```bash
cd "E:\BioLoop Monitor"
```

### 1.3. Kết nối ESP32 với máy tính
- Cắm cáp USB vào ESP32
- Kiểm tra cổng COM (Device Manager → Ports)
- Thường là COM3, COM4, COM5...

### 1.4. Nạp code
```bash
pio run -t upload
```

**Kết quả mong đợi:**
```
Building .pio\build\esp32dev\firmware.bin
Configuring upload protocol...
AVAILABLE: cmsis-dap, esp-bridge, esp-prog, espota, esptool, iot-bus-jtag, jlink, minimodule, olimex-arm-usb-ocd, olimex-arm-usb-ocd-h, olimex-arm-usb-tiny-h, olimex-jtag-tiny, tumpa
CURRENT: upload_protocol = esptool
Looking for upload port...
Auto-detected: COM3
Uploading .pio\build\esp32dev\firmware.bin
...
Writing at 0x00010000... (100 %)
Wrote 1234567 bytes (789012 compressed) at 0x00010000 in 12.3 seconds (effective 987.6 kbit/s)...
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
========================= [SUCCESS] Took 15.67 seconds =========================
```

### 1.5. Nếu gặp lỗi "Port not found"
```bash
# Kiểm tra cổng COM
pio device list

# Nạp với cổng cụ thể
pio run -t upload --upload-port COM3
```

---

## 🔍 BƯỚC 2: KIỂM TRA SERIAL MONITOR

### 2.1. Mở Serial Monitor
```bash
pio device monitor
```

Hoặc trong VS Code:
```
Ctrl + Shift + P → PlatformIO: Serial Monitor
```

### 2.2. Kết quả mong đợi

**Khởi động:**
```
========================================
  BioLoop Monitor - Composting System
========================================
  Control Mode: FUZZY LOGIC (Active)
Initializing...

[WIFI] Connecting to WiFi...
[WIFI] SSID: Quoc Viet
..........
[WIFI] Connected!
[WIFI] IP Address: 192.168.1.86

[SENSORS] Initializing DS18B20 temperature sensor...
[SENSORS] Found 1 DS18B20 sensor(s)
[SENSORS] Initializing moisture sensor on GPIO 35...
[SENSORS] Moisture sensor ready

[ACTUATORS] Initializing relay outputs...
[ACTUATORS] Pump relay (GPIO 26) - OFF (ACTIVE HIGH)
[ACTUATORS] Fan relay (GPIO 27) - OFF (ACTIVE HIGH)

[FUZZY] Initializing fuzzy inference engine...
[FUZZY] Fuzzy control ready
[FUZZY] Fan threshold: PWM > 120 → ON

[SYSTEM] Initialization complete!
[SYSTEM] Starting control loop...
```

**Hoạt động bình thường (Pha 1):**
```
----------------------------------------
[STATUS] Phase: 1 (Target: 35.0°C)
[STATUS] Temperature: 28.5°C
[STATUS] Moisture: 55.2%
[STATUS] Fan PWM: 0 → OFF
[STATUS] Pump: OFF
[STATUS] Fan: OFF
[JSON] {"temperature":28.5,"moisture":55.2,"temp_error":-6.5,"fan_pwm":0,"fan":false,"pump":false,"phase":1,"target_temp":35.0}
----------------------------------------

[HTTP] Sending: {"temperature":28.5,"moisture":55.2,"temp_error":-6.5,"fan_pwm":0,"fan":false,"pump":false,"phase":1,"target_temp":35.0}
[HTTP] Response (200): {"success":true,"id":1234,"timestamp":"2026-01-17T10:30:45.123Z"}
```

**Chuyển pha (Pha 1 → Pha 2):**
```
[PHASE] Transition: Mesophilic → Thermophilic
[PHASE] New target: 52.5°C
----------------------------------------
[STATUS] Phase: 2 (Target: 52.5°C)
[STATUS] Temperature: 46.2°C
[STATUS] Moisture: 58.1%
[STATUS] Fan PWM: 150 → ON
[STATUS] Pump: OFF
[STATUS] Fan: ON
----------------------------------------
```

**Chuyển pha (Pha 2 → Pha 3):**
```
[PHASE] Transition: Thermophilic → Maturation
[PHASE] New target: 35.0°C
[PHASE] Thermophilic duration: 15.3 days
----------------------------------------
[STATUS] Phase: 3 (Target: 35.0°C)
[STATUS] Temperature: 38.5°C
[STATUS] Moisture: 45.2%
----------------------------------------
```

### 2.3. Thoát Serial Monitor
```
Ctrl + C
```

---

## 🌐 BƯỚC 3: KIỂM TRA DASHBOARD

### 3.1. Truy cập Dashboard
Mở trình duyệt và truy cập:
```
https://bioloop-monitor.onrender.com
```

### 3.2. Kiểm tra các chỉ số

#### ✅ Giai Đoạn Sinh Học:
- Tên pha hiển thị bằng tiếng Việt
- Màu sắc:
  - Pha 1 (Mesophilic): Xanh dương
  - Pha 2 (Thermophilic): Cam
  - Pha 3 (Maturation): Xanh lá
- Mô tả bao gồm nhiệt độ mục tiêu
- Thanh tiến trình 3 pha hoạt động

#### ✅ Thẻ trạng thái:
- Nhiệt độ hiện tại (°C)
- Độ ẩm hiện tại (%)
- Tốc độ quạt (PWM)
- Trạng thái máy bơm (BẬT/TẮT)

#### ✅ Biểu đồ:
- Nhiệt độ theo thời gian
- Độ ẩm theo thời gian
- Fan PWM theo thời gian
- Vùng tối ưu (màu xanh lá)

#### ✅ Hiệu suất 24h:
- Điểm số (0-100)
- Xếp hạng sao (1-5 ⭐)
- Thống kê chi tiết

### 3.3. Kiểm tra Chatbot

Click vào biểu tượng chat (góc dưới bên phải) và thử các câu hỏi:

1. **"Nhiệt độ hiện tại là bao nhiêu?"**
   - Kết quả: Hiển thị nhiệt độ, pha hiện tại, và mục tiêu

2. **"Đang ở giai đoạn nào?"**
   - Kết quả: Mô tả chi tiết pha hiện tại

3. **"Hệ thống chuyển pha như thế nào?"**
   - Kết quả: Giải thích cơ chế chuyển pha tự động

4. **"Tại sao máy bơm bật?"**
   - Kết quả: Trạng thái máy bơm và lý do

---

## 🔧 BƯỚC 4: TRIỂN KHAI SERVER (NẾU CẦN)

### 4.1. Commit và Push code
```bash
cd server
git add .
git commit -m "feat: Add 3-phase adaptive composting control"
git push origin main
```

### 4.2. Kiểm tra Render.com
1. Truy cập: https://dashboard.render.com
2. Chọn service: `bioloop-monitor`
3. Kiểm tra "Events" tab
4. Đợi deploy hoàn tất (~2-3 phút)

### 4.3. Kiểm tra logs
```bash
# Trong Render.com Dashboard
Logs → View Logs

# Kết quả mong đợi:
[DB] SQLite initialized at /opt/render/project/src/bioloop.db
[DB] Table ready
[SERVER] Listening on port 3000
```

---

## ✅ BƯỚC 5: KIỂM TRA HOẠT ĐỘNG

### 5.1. Checklist ESP32:
- [ ] WiFi kết nối thành công
- [ ] Cảm biến nhiệt độ đọc được (không phải -127 hoặc 85)
- [ ] Cảm biến độ ẩm đọc được (0-100%)
- [ ] Pha hiện tại hiển thị (1, 2, hoặc 3)
- [ ] Nhiệt độ mục tiêu thay đổi theo pha
- [ ] HTTP POST thành công (200 OK)

### 5.2. Checklist Dashboard:
- [ ] Dữ liệu cập nhật real-time (mỗi 3 giây)
- [ ] Giai đoạn sinh học hiển thị đúng
- [ ] Biểu đồ vẽ được
- [ ] Hiệu suất 24h tính toán đúng
- [ ] Chatbot trả lời đúng

### 5.3. Checklist Database:
- [ ] Cột `phase` có giá trị (1, 2, 3)
- [ ] Cột `target_temp` có giá trị (35.0, 52.5)
- [ ] Timestamp đúng múi giờ Việt Nam (GMT+7)

---

## 🐛 XỬ LÝ SỰ CỐ

### Sự cố 1: ESP32 không kết nối WiFi
**Triệu chứng:**
```
[WIFI] Connection failed! Running in offline mode.
```

**Giải pháp:**
1. Kiểm tra SSID và password trong `src/main.cpp`:
   ```cpp
   const char* WIFI_SSID = "Quoc Viet";
   const char* WIFI_PASSWORD = "Vy@020514";
   ```
2. Kiểm tra router WiFi đang bật
3. Kiểm tra ESP32 trong phạm vi WiFi

---

### Sự cố 2: Cảm biến nhiệt độ trả về -127°C
**Triệu chứng:**
```
[STATUS] Temperature: -127.0°C (INVALID)
```

**Giải pháp:**
1. Kiểm tra kết nối DS18B20:
   - VCC → 3.3V
   - GND → GND
   - DATA → GPIO 21
2. Kiểm tra điện trở pull-up 4.7kΩ (DATA → VCC)
3. Thử cảm biến khác

---

### Sự cố 3: Độ ẩm nhảy loạn (0% → 100%)
**Triệu chứng:**
```
[DEBUG] Moisture ADC raw: 4095, filtered: 4095
[DEBUG] Moisture INVALID: ADC out of range
```

**Giải pháp:**
1. Đã được fix bằng:
   - Oversampling (đọc 10 lần)
   - Moving Average Filter (5 mẫu)
   - Range Validation (100-4000)
   - Hysteresis (35% ON, 25% OFF)
2. Nếu vẫn lỗi, thử GPIO khác (32, 33, 36, 39)

---

### Sự cố 4: Dashboard không hiển thị pha
**Triệu chứng:**
- Pha hiển thị "undefined"
- Mục tiêu không thay đổi

**Giải pháp:**
1. Kiểm tra JSON payload từ ESP32:
   ```json
   {"phase":1,"target_temp":35.0}
   ```
2. Kiểm tra database có cột `phase` và `target_temp`
3. Xóa cache trình duyệt (Ctrl + Shift + R)

---

### Sự cố 5: Không chuyển sang Pha 3
**Triệu chứng:**
```
[PHASE] Temperature dropped but Phase 2 duration not met - maintaining Thermophilic
```

**Giải thích:**
- Đây là tính năng, không phải lỗi!
- Hệ thống yêu cầu Pha 2 kéo dài ≥ 14 ngày
- Đảm bảo tiêu diệt hoàn toàn mầm bệnh

**Hành động:**
- Đợi đủ 14 ngày
- Hoặc thêm chất hữu cơ mới để tăng nhiệt độ trở lại

---

## 📚 TÀI LIỆU THAM KHẢO

1. **HE_THONG_3_PHA_TU_DONG.md**
   - Mô tả chi tiết 3 giai đoạn
   - Cơ chế chuyển pha
   - Lợi ích và so sánh

2. **CHANGELOG_3_PHASE_SYSTEM.md**
   - Tổng hợp tất cả thay đổi code
   - Checklist kiểm tra
   - Metrics theo dõi

3. **FUZZY_CONTROL_IMPLEMENTATION.md**
   - Giải thích Fuzzy Logic
   - Cấu trúc code
   - Tối ưu hóa

---

## 🎓 KIẾN THỨC BỔ SUNG

### Tại sao cần 3 pha?

**Pha 1 (Mesophilic):**
- Vi sinh vật ưa ấm phân hủy đường, tinh bột
- Nhiệt độ tăng dần tự nhiên
- Không nên làm mát quá mức (lãng phí năng lượng)

**Pha 2 (Thermophilic):**
- Vi sinh vật ưa nhiệt phân hủy cellulose, lignin
- Nhiệt độ 55-60°C tiêu diệt mầm bệnh
- Cần duy trì ≥ 14 ngày (tiêu chuẩn compost)

**Pha 3 (Maturation):**
- Vi sinh vật mesophilic trở lại
- Hình thành humus ổn định
- Giảm độ ẩm để dễ bảo quản

### Lợi ích so với hệ thống cũ:

| Tiêu chí | Cũ (50°C cố định) | Mới (3 pha) |
|----------|------------------|-------------|
| Tiêu thụ năng lượng | 100% | 60-70% |
| Chất lượng compost | Tốt | Xuất sắc |
| Phù hợp sinh học | Trung bình | Cao |
| Tuổi thọ thiết bị | Trung bình | Cao hơn |

---

## 📞 HỖ TRỢ

Nếu gặp vấn đề, hãy:

1. **Kiểm tra Serial Monitor** để xem log chi tiết
2. **Kiểm tra Dashboard** để xem dữ liệu real-time
3. **Đọc tài liệu** HE_THONG_3_PHA_TU_DONG.md
4. **Liên hệ:** BioLoop Team

---

**Chúc bạn thành công! 🎉**

**Tác giả:** BioLoop Team  
**Ngày cập nhật:** 17/01/2026  
**Phiên bản:** 2.0 - Adaptive 3-Phase Control
