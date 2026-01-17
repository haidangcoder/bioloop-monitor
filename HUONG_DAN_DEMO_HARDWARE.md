# Hướng dẫn Demo Mode với Điều khiển Phần cứng Thật

## 🎯 Tổng quan

Demo mode cho phép bạn điều khiển bơm/quạt thật từ web interface để demo fuzzy logic.

## 🔧 Cách hoạt động

```
Web Demo Page → Server API → ESP32 Poll → Điều khiển Bơm/Quạt
     ↓              ↓            ↓              ↓
  Click Case    Lưu lệnh    Đọc lệnh      Thực thi
```

### Flow chi tiết:

1. **User click case trên web** → Gửi lệnh override lên server
2. **Server lưu lệnh** vào database với timestamp
3. **ESP32 poll server** mỗi 3 giây để kiểm tra lệnh mới
4. **ESP32 nhận lệnh** → Bỏ qua fuzzy logic, điều khiển theo lệnh
5. **Auto-expire** sau 5 phút hoặc khi click "Tắt Demo Mode"

## 📋 Bước thực hiện

### 1. Nạp code mới vào ESP32

```bash
# Trong PlatformIO
pio run -t upload

# Hoặc trong VS Code
# Nhấn nút Upload (→) trên thanh công cụ
```

### 2. Mở Serial Monitor để theo dõi

```bash
pio device monitor

# Hoặc trong VS Code: Terminal → Run Task → Monitor
```

Bạn sẽ thấy:
```
[DEMO] ⚠️  OVERRIDE ACTIVATED
[DEMO] Fan PWM: 255, Pump: ON
[STATUS] ⚠️  DEMO MODE ACTIVE - Override from server
```

### 3. Mở Demo Page

Truy cập: https://bioloop-monitor.onrender.com/demo.html

### 4. Chạy Demo

1. **Click vào Case 1, 2, hoặc 3**
2. **Quan sát**:
   - Web: Animation 30 giây
   - Serial Monitor: Thông báo "DEMO MODE ACTIVE"
   - Phần cứng: Bơm/quạt hoạt động theo case
3. **Sau 30 giây**: Hệ thống tự động về trạng thái lý tưởng
4. **Click "Tắt Demo Mode"**: Quay về điều khiển tự động

## ⚠️ An toàn

### Tự động tắt demo:
- ✅ Sau 5 phút (timeout)
- ✅ Khi mất WiFi
- ✅ Khi click "Tắt Demo Mode"
- ✅ Khi ESP32 reset

### Fallback:
- Nếu không kết nối được server → Dùng cảm biến thật
- Nếu WiFi mất → Tự động tắt demo mode

## 🔍 Kiểm tra trạng thái

### Trên Serial Monitor:
```
[STATUS] ⚠️  DEMO MODE ACTIVE - Override from server
[STATUS] Temperature: 58.0°C
[STATUS] Moisture: 35.0%
[STATUS] Fan PWM: 255 → ON
[STATUS] Pump: ON
```

### Trên Dashboard:
- Biểu đồ sẽ hiển thị giá trị demo
- Sau 30 giây chuyển về giá trị lý tưởng

## 🐛 Troubleshooting

### Demo không hoạt động:

1. **Kiểm tra WiFi**:
   ```
   [WIFI] Connected!
   [WIFI] IP Address: 192.168.1.86
   ```

2. **Kiểm tra server connection**:
   ```
   [HTTP] Response (200): {"success":true}
   ```

3. **Kiểm tra demo status**:
   - Mở: https://bioloop-monitor.onrender.com/api/demo/status
   - Nếu `{"active":0}` → Demo đã tắt
   - Nếu `{"active":1,"fan":255,"pump":1}` → Demo đang bật

### Bơm/quạt không phản ứng:

1. **Kiểm tra relay**:
   - Đèn LED trên relay module có sáng không?
   - Relay ACTIVE HIGH: GPIO HIGH = ON

2. **Kiểm tra Serial Monitor**:
   ```
   [DEMO] ⚠️  OVERRIDE ACTIVATED
   [DEMO] Fan PWM: 255, Pump: ON
   ```

3. **Test thủ công**:
   ```bash
   # Gửi lệnh trực tiếp
   curl -X POST https://bioloop-monitor.onrender.com/api/demo/override \
     -H "Content-Type: application/json" \
     -d '{"case_num":1,"fan_pwm":255,"pump_active":true}'
   ```

## 📊 Test Cases

### Case 1: Critical (Nóng + Khô)
- Nhiệt độ: 58°C
- Độ ẩm: 35%
- **Phản ứng**: Fan 100% (255 PWM), Bơm ON

### Case 2: Optimal (Lý tưởng)
- Nhiệt độ: 52°C
- Độ ẩm: 55%
- **Phản ứng**: Fan 35% (90 PWM), Bơm OFF

### Case 3: Recovery (Lạnh + Khô)
- Nhiệt độ: 35°C
- Độ ẩm: 30%
- **Phản ứng**: Fan 4% (10 PWM), Bơm ON

## 🎬 Demo cho khách hàng

1. **Chuẩn bị**:
   - ESP32 đã nạp code mới
   - Kết nối WiFi ổn định
   - Mở Serial Monitor để show log
   - Mở Dashboard trên màn hình lớn

2. **Trình bày**:
   - "Đây là hệ thống ủ phân compost tự động"
   - "Sử dụng Fuzzy Logic thay vì Hard Threshold"
   - "Tôi sẽ demo 3 tình huống khác nhau"

3. **Demo từng case**:
   - Click case → Giải thích tình huống
   - Chỉ vào Serial Monitor → "Hệ thống đang phản ứng"
   - Chỉ vào bơm/quạt thật → "Thiết bị đang hoạt động"
   - Chỉ vào Dashboard → "Dữ liệu real-time"

4. **Kết thúc**:
   - Click "Tắt Demo Mode"
   - "Hệ thống quay về chế độ tự động"
   - "Đọc cảm biến thật và điều khiển theo fuzzy logic"

## 📝 Notes

- Demo mode **KHÔNG ẢNH HƯỞNG** đến quá trình ủ thật nếu không bật
- Cảm biến vẫn được đọc trong demo mode (để gửi lên dashboard)
- Chỉ logic điều khiển bị override
- An toàn tuyệt đối với timeout 5 phút

## 🔗 Links

- Demo Page: https://bioloop-monitor.onrender.com/demo.html
- Dashboard: https://bioloop-monitor.onrender.com
- API Status: https://bioloop-monitor.onrender.com/api/demo/status
