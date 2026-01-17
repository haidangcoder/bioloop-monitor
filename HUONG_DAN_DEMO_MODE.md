# HƯỚNG DẪN DEMO MODE - ĐIỀU KHIỂN BƠM/QUẠT THẬT

## 🎯 TỔNG QUAN

Demo Mode cho phép điều khiển bơm/quạt thật từ web interface mà không cần thay đổi cảm biến. Hữu ích cho:
- Demo cho giám khảo
- Test 3 case Fuzzy Logic
- Kiểm tra phần cứng

---

## 🚀 CÁCH SỬ DỤNG

### Bước 1: Mở trang Demo
```
https://bioloop-monitor.onrender.com/demo.html
```

### Bước 2: Chọn Case
- **Case 1:** Nóng + Khô → Quạt 100%, Bơm BẬT
- **Case 2:** Lý tưởng → Quạt 35%, Bơm TẮT  
- **Case 3:** Lạnh + Khô → Quạt 4%, Bơm BẬT

### Bước 3: Nhấn "Chạy Case X"
- ESP32 sẽ nhận lệnh từ server
- Bơm/quạt thật sự hoạt động theo case
- Tự động tắt sau 5 phút

### Bước 4: Tắt Demo Mode
- Nhấn "🛑 Tắt Demo Mode"
- Hệ thống quay về chế độ tự động

---

## 🔧 CẤU TRÚC HỆ THỐNG

### 1. Demo Page (`/demo.html`)
- Giao diện 3 case
- Gửi lệnh override lên server

### 2. Server API (`/api/demo/override`)
- Lưu lệnh override vào database
- ESP32 đọc lệnh từ đây

### 3. ESP32 (Cần cập nhật code)
- Đọc override từ server mỗi 10 giây
- Nếu có override: Điều khiển theo lệnh
- Nếu không: Chạy Fuzzy Logic bình thường

---

## 📝 CODE ESP32 CẦN THÊM

Thêm vào `src/main.cpp`:

```cpp
// ============================================
// DEMO MODE - Check override from server
// ============================================
bool demoModeActive = false;
uint8_t demoFanPwm = 0;
bool demoPumpActive = false;
unsigned long lastDemoCheck = 0;
#define DEMO_CHECK_INTERVAL 10000  // Check every 10 seconds

void checkDemoOverride() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastDemoCheck < DEMO_CHECK_INTERVAL) {
    return;  // Not time to check yet
  }
  
  lastDemoCheck = currentMillis;
  
  if (!wifiConnected) return;
  
  HTTPClient http;
  http.begin("https://bioloop-monitor.onrender.com/api/demo/override");
  
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    
    // Parse JSON (simple parsing)
    if (payload.indexOf("\"active\":true") > 0) {
      demoModeActive = true;
      
      // Extract fan_pwm
      int fanIdx = payload.indexOf("\"fan_pwm\":");
      if (fanIdx > 0) {
        demoFanPwm = payload.substring(fanIdx + 10, fanIdx + 13).toInt();
      }
      
      // Extract pump_active
      int pumpIdx = payload.indexOf("\"pump_active\":");
      if (pumpIdx > 0) {
        demoPumpActive = (payload.indexOf("true", pumpIdx) > 0);
      }
      
      Serial.println("[DEMO] Override active!");
      Serial.printf("[DEMO] Fan: %d PWM, Pump: %s\n", demoFanPwm, demoPumpActive ? "ON" : "OFF");
    } else {
      demoModeActive = false;
    }
  }
  
  http.end();
}

// ============================================
// MAIN LOOP (with demo mode check)
// ============================================
void loop() {
  unsigned long currentMillis = millis();
  
  // Check demo override from server
  checkDemoOverride();
  
  // Read sensors at regular intervals
  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = currentMillis;
    readSensors();
    
    // Execute control logic
    if (demoModeActive) {
      // DEMO MODE: Use override values
      Serial.println("[DEMO] Using override control");
      setFan(demoFanPwm > FAN_PWM_THRESHOLD);
      setPump(demoPumpActive);
      lastFanPwm = demoFanPwm;  // For logging
    } else {
      // NORMAL MODE: Use Fuzzy Logic
      runFuzzyControl();
    }
  }
  
  // ... rest of loop code
}
```

---

## ⚠️ LƯU Ý

1. **Tự động tắt sau 5 phút:** Demo mode tự động expire để tránh quên tắt
2. **Ưu tiên an toàn:** Nếu mất kết nối, quay về chế độ tự động
3. **Không ảnh hưởng dữ liệu:** Sensor vẫn đọc và ghi bình thường

---

## 🎬 DEMO CHO GIÁM KHẢO

### Script demo:

**"Bây giờ tôi sẽ demo 3 tình huống để thấy rõ ưu điểm của Fuzzy Logic"**

1. **Case 1 - Critical:**
   - "Giả sử nhiệt độ 58°C, độ ẩm 35% - rất nguy hiểm"
   - *Nhấn Case 1*
   - "Fuzzy Logic bật quạt 100% VÀ bơm cùng lúc"
   - "Hard threshold chỉ bật 1 trong 2"

2. **Case 2 - Optimal:**
   - "Điều kiện lý tưởng: 52°C, 55%"
   - *Nhấn Case 2*
   - "Fuzzy Logic chỉ duy trì nhẹ 35%"
   - "Hard threshold sẽ bật/tắt liên tục"

3. **Case 3 - Recovery:**
   - "Nhiệt độ thấp 35°C, độ ẩm 30%"
   - *Nhấn Case 3*
   - "Fuzzy Logic tắt quạt, chỉ bật bơm"
   - "Hard threshold có thể làm lạnh thêm - sai!"

**"Như các bạn thấy, Fuzzy Logic hiểu được mối quan hệ nhiệt độ-độ ẩm và phối hợp 2 actuator thông minh hơn"**

---

## 📊 KẾT QUẢ MONG ĐỢI

| Case | Quạt | Bơm | Lý do |
|------|------|-----|-------|
| 1 | 255 PWM (100%) | BẬT | Làm mát + tưới nước khẩn cấp |
| 2 | 90 PWM (35%) | TẮT | Duy trì ổn định |
| 3 | 10 PWM (4%) | BẬT | Không làm lạnh thêm, chỉ tưới |

---

**Tác giả:** BioLoop Team  
**Ngày:** 17/01/2026  
**Phiên bản:** 1.0
