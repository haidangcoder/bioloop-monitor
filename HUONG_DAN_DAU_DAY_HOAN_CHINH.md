# 🔌 HƯỚNG DẪN ĐẤU DÂY HOÀN CHỈNH - BIOLOOP MONITOR

## 📋 MỤC LỤC
1. [Danh sách linh kiện](#danh-sách-linh-kiện)
2. [Chuẩn bị trước khi đấu dây](#chuẩn-bị)
3. [Bước 1: Nguồn điện](#bước-1-nguồn-điện)
4. [Bước 2: ESP32](#bước-2-esp32)
5. [Bước 3: Relay module](#bước-3-relay-module)
6. [Bước 4: Bơm nước](#bước-4-bơm-nước)
7. [Bước 5: Quạt](#bước-5-quạt)
8. [Bước 6: DS18B20](#bước-6-ds18b20)
9. [Bước 7: Soil sensor](#bước-7-soil-sensor)
10. [Kiểm tra và test](#kiểm-tra-và-test)
11. [Troubleshooting](#troubleshooting)

---

## 📦 DANH SÁCH LINH KIỆN

### Linh kiện chính:
- [ ] ESP32 DevKit (30 chân)
- [ ] Adapter 12V 2A
- [ ] Module hạ áp LM2596 (DC-DC buck converter)
- [ ] Relay module 2 kênh 5V
- [ ] Bơm nước DC 5V
- [ ] Quạt DC 5V
- [ ] Cảm biến nhiệt độ DS18B20 (chống nước)
- [ ] Cảm biến độ ẩm đất (Capacitive Soil Moisture)

### Linh kiện phụ:
- [ ] Điện trở 4.7kΩ (1 cái) - cho DS18B20
- [ ] Diode 1N5819 hoặc 1N4007 (2 cái) - bảo vệ bơm/quạt
- [ ] Dây điện đơn (đỏ, đen, vàng, xanh)
- [ ] Đồng hồ vạn năng (multimeter)
- [ ] Breadboard hoặc mạch hàn
- [ ] Đầu cos (terminal block) - tùy chọn

---

## 🛠️ CHUẨN BỊ

### 1. Công cụ cần thiết:
- Đồng hồ vạn năng
- Tuốc nơ vít nhỏ (chỉnh LM2596)
- Kìm cắt dây
- Kìm tuốt dây
- Mỏ hàn + thiếc (nếu hàn)

### 2. Kiến thức cần biết:

**Điện áp:**
- 12V: Từ adapter (nguồn gốc)
- 5V: Sau LM2596 (cho ESP32, relay, bơm, quạt)
- 3.3V: Từ ESP32 (cho DS18B20, soil sensor)

**Dòng điện ước tính:**
- ESP32: ~200mA
- Relay module: ~100mA
- Bơm: ~500mA
- Quạt: ~300mA
- **Tổng: ~1.1A** → Adapter 12V 2A đủ

**GND (Ground):**
- TẤT CẢ thiết bị phải chung GND
- Không chung GND = không hoạt động

---


## 🔋 BƯỚC 1: NGUỒN ĐIỆN (12V → 5V)

### 1.1. Kết nối Adapter → LM2596

**Xác định chân LM2596:**
```
┌─────────────────────┐
│  LM2596 Module      │
│                     │
│  IN+  IN-  OUT+ OUT-│
│   ●    ●    ●    ●  │
│         [POT]       │  ← Biến trở chỉnh điện áp
└─────────────────────┘
```

**Đấu dây:**
```
Adapter 12V (+) dây đỏ   →  LM2596 IN+
Adapter 12V (-) dây đen  →  LM2596 IN-
```

**⚠️ CẢNH BÁO:**
- Chưa cắm adapter vào ổ điện
- Chưa nối OUT+ và OUT- với bất kỳ thiết bị nào

### 1.2. Chỉnh LM2596 xuống 5V

**Bước thực hiện:**

1. **Cắm adapter vào ổ điện** (chỉ LM2596, chưa có gì khác)

2. **Đo điện áp OUT:**
   - Đồng hồ chế độ DC Voltage (20V)
   - Que đỏ → OUT+
   - Que đen → OUT-
   - Đọc số trên màn hình

3. **Chỉnh biến trở:**
   - Nếu < 5V: Vặn tuốc nơ vít theo chiều kim đồng hồ (tăng)
   - Nếu > 5V: Vặn ngược chiều kim đồng hồ (giảm)
   - Mục tiêu: **5.00V ± 0.05V**

4. **Kiểm tra lại:**
   - Rút adapter
   - Cắm lại
   - Đo lại → Phải vẫn là 5.00V

**✅ Hoàn thành khi:**
- OUT+ = 5.00V
- OUT- = 0V (GND)

**⛔ CHƯA ĐƯỢC:**
- Cắm ESP32
- Nối bất kỳ thiết bị nào vào OUT

---

## 🧠 BƯỚC 2: ESP32

### 2.1. Sơ đồ chân ESP32

```
                    ESP32 DevKit (30 chân)
                    
     3V3  ●                              ● GND
     EN   ●                              ● GPIO23
  GPIO36  ●                              ● GPIO22
  GPIO39  ●                              ● GPIO1 (TX)
  GPIO34  ●                              ● GPIO3 (RX)
  GPIO35  ● ← Soil sensor AO            ● GPIO21 ← DS18B20 DQ
  GPIO32  ●                              ● GND
  GPIO33  ●                              ● GPIO19
  GPIO25  ●                              ● GPIO18
  GPIO26  ● ← Relay IN1 (BƠM)           ● GPIO5
  GPIO27  ● ← Relay IN2 (QUẠT)          ● GPIO17
  GPIO14  ●                              ● GPIO16
  GPIO12  ●                              ● GPIO4
     GND  ●                              ● GPIO0
  GPIO13  ●                              ● GPIO2
     SD2  ●                              ● GPIO15
     SD3  ●                              ● SD1
     CMD  ●                              ● SD0
     5V   ● ← LM2596 OUT+ (5V)          ● CLK
```

### 2.2. Kết nối ESP32 với nguồn

**Đấu dây:**
```
LM2596 OUT+ (5V)  →  ESP32 VIN (hoặc 5V)
LM2596 OUT- (GND) →  ESP32 GND
```

**Lưu ý:**
- ESP32 có nhiều chân GND, dùng chân nào cũng được
- VIN và 5V là cùng một chân trên một số board

### 2.3. Test ESP32

1. **Cắm adapter vào ổ điện**
2. **Đèn LED trên ESP32 phải sáng**
3. **Cắm USB vào máy tính**
4. **Mở Serial Monitor:**
   ```bash
   python -m platformio device monitor
   ```
5. **Xem log:**
   ```
   ========================================
     BioLoop Monitor - Composting System
   ========================================
   [WIFI] Connecting to WiFi...
   ```

**✅ Hoàn thành khi:**
- Đèn LED sáng
- Serial Monitor hiển thị log
- WiFi connected

---


## 🔌 BƯỚC 3: RELAY MODULE 2 KÊNH

### 3.1. Hiểu về Relay Module

**Relay có 2 phần:**

1. **Phần điều khiển (logic):** VCC, GND, IN1, IN2
2. **Phần công suất (tiếp điểm):** COM, NO, NC (mỗi kênh)

```
┌─────────────────────────────────────┐
│         RELAY MODULE 2 KÊNH         │
├─────────────────────────────────────┤
│  Phần điều khiển:                   │
│  VCC  GND  IN1  IN2                 │
│   ●    ●    ●    ●                  │
│                                     │
│  Kênh 1 (CH1):        Kênh 2 (CH2):│
│  COM  NO  NC          COM  NO  NC   │
│   ●   ●   ●            ●   ●   ●   │
└─────────────────────────────────────┘
```

**Giải thích tiếp điểm:**
- **COM** (Common): Chân chung - nối nguồn
- **NO** (Normally Open): Hở khi relay OFF, đóng khi relay ON
- **NC** (Normally Closed): Đóng khi relay OFF, hở khi relay ON (không dùng)

### 3.2. Xác định ACTIVE HIGH hay ACTIVE LOW

**Có 2 loại relay:**

**ACTIVE HIGH:**
- GPIO HIGH (3.3V) → Relay ON
- GPIO LOW (0V) → Relay OFF
- Code hiện tại dùng loại này

**ACTIVE LOW:**
- GPIO LOW (0V) → Relay ON
- GPIO HIGH (3.3V) → Relay OFF

**Cách xác định:**

Nhìn vào relay module, tìm:
- Có transistor NPN + LED → ACTIVE HIGH
- Có optocoupler → ACTIVE LOW (phổ biến hơn)

Hoặc test bằng code (xem phần 10.3)

### 3.3. Kết nối phần điều khiển

**Đấu dây:**
```
LM2596 OUT+ (5V)  →  Relay VCC
LM2596 OUT- (GND) →  Relay GND
ESP32 GPIO 26     →  Relay IN1 (điều khiển bơm)
ESP32 GPIO 27     →  Relay IN2 (điều khiển quạt)
```

**Lưu ý:**
- IN1 điều khiển kênh 1 (CH1)
- IN2 điều khiển kênh 2 (CH2)

### 3.4. Kết nối phần công suất

**⚠️ QUAN TRỌNG: Relay chỉ đóng/ngắt mạch, KHÔNG cấp nguồn!**

**Nguyên lý:**
```
Nguồn 5V → COM → (relay đóng) → NO → Tải (+) → Tải (-) → GND
```

**Đấu dây cụ thể:**

**Kênh 1 (CH1) - Bơm:**
```
LM2596 OUT+ (5V) →  Relay CH1 COM
Relay CH1 NO     →  Bơm (+)
Bơm (-)          →  LM2596 OUT- (GND)
```

**Kênh 2 (CH2) - Quạt:**
```
LM2596 OUT+ (5V) →  Relay CH2 COM
Relay CH2 NO     →  Quạt (+)
Quạt (-)         →  LM2596 OUT- (GND)
```

**Chân NC:** Không nối gì cả

**✅ Hoàn thành khi:**
- 4 dây nối phần điều khiển (VCC, GND, IN1, IN2)
- 2 dây nối COM (cả 2 kênh nối nguồn 5V)
- Chưa nối NO (sẽ nối ở bước 4 và 5)

---

## 💧 BƯỚC 4: BƠM NƯỚC

### 4.1. Kiểm tra bơm

**Trước khi đấu dây:**

1. **Xác định cực (+) và (-):**
   - Thường có dấu + - trên bơm
   - Hoặc dây đỏ (+), dây đen (-)

2. **Test bơm trực tiếp:**
   - Nối bơm (+) → LM2596 OUT+
   - Nối bơm (-) → LM2596 OUT-
   - Bơm phải chạy
   - Nếu không chạy → bơm hỏng hoặc sai cực

### 4.2. Gắn diode bảo vệ

**Tại sao cần diode?**
- Bơm là tải cảm (có cuộn dây)
- Khi tắt, sinh điện áp ngược → hỏng relay/ESP32
- Diode chặn điện áp ngược

**Loại diode:**
- 1N5819 (Schottky, tốt hơn)
- 1N4007 (thông dụng)

**Cách gắn:**

```
        Bơm
    ┌────────┐
(+) ●        ● (-)
    │        │
    │  ┌──┐  │
    └──┤▶├──┘  ← Diode (song song)
       └──┘
       │  │
    Cathode Anode
    (vạch)
```

**Đấu dây:**
```
Diode Cathode (vạch trắng) →  Bơm (+)
Diode Anode                →  Bơm (-) / GND
```

**Lưu ý:**
- Diode gắn SONG SONG với bơm
- KHÔNG nối tiếp
- Sai chiều → không bảo vệ

### 4.3. Kết nối bơm vào relay

**Đấu dây hoàn chỉnh:**
```
LM2596 OUT+ (5V) →  Relay CH1 COM
Relay CH1 NO     →  Bơm (+)
                    Diode Cathode (vạch trắng)
Bơm (-)          →  LM2596 OUT- (GND)
                    Diode Anode
```

**Sơ đồ chi tiết:**
```
5V ──→ COM ──→ NO ──→ Bơm (+) ──┐
                         ↑       │
                      Diode      │
                      (song      │
                       song)     │
                         ↓       │
GND ←──────────────── Bơm (-) ←─┘
```

**✅ Hoàn thành khi:**
- Bơm (+) nối Relay CH1 NO
- Bơm (-) nối GND
- Diode gắn song song, đúng chiều

---


## 🌀 BƯỚC 5: QUẠT

### 5.1. Kiểm tra quạt

**Trước khi đấu dây:**

1. **Xác định cực (+) và (-):**
   - Dây đỏ: (+)
   - Dây đen: (-)

2. **Test quạt trực tiếp:**
   - Nối quạt (+) → LM2596 OUT+
   - Nối quạt (-) → LM2596 OUT-
   - Quạt phải quay
   - Nếu không quay → quạt hỏng hoặc sai cực

### 5.2. Gắn diode bảo vệ

**Giống như bơm:**

```
Diode Cathode (vạch trắng) →  Quạt (+)
Diode Anode                →  Quạt (-) / GND
```

### 5.3. Kết nối quạt vào relay

**Đấu dây hoàn chỉnh:**
```
LM2596 OUT+ (5V) →  Relay CH2 COM
Relay CH2 NO     →  Quạt (+)
                    Diode Cathode (vạch trắng)
Quạt (-)         →  LM2596 OUT- (GND)
                    Diode Anode
```

**✅ Hoàn thành khi:**
- Quạt (+) nối Relay CH2 NO
- Quạt (-) nối GND
- Diode gắn song song, đúng chiều

---

## 🌡️ BƯỚC 6: DS18B20 - CẢM BIẾN NHIỆT ĐỘ

### 6.1. Xác định chân DS18B20

**DS18B20 có 3 chân (mặt chữ hướng về bạn):**

```
   ┌─────────┐
   │ DS18B20 │
   │         │
   └─┬───┬───┬┘
     1   2   3
     │   │   │
    VCC DQ GND
```

**Chân 1 (trái):** VCC (nguồn)
**Chân 2 (giữa):** DQ (data)
**Chân 3 (phải):** GND

### 6.2. Điện trở pull-up 4.7kΩ

**Tại sao cần điện trở?**
- DS18B20 dùng giao thức 1-Wire
- Cần điện trở kéo lên (pull-up) giữa DQ và VCC
- Không có → nhiệt độ = -127°C (lỗi)

**Cách gắn:**
```
        4.7kΩ
    ┌────/\/\/\────┐
    │              │
   3V3            DQ (GPIO 21)
    │              │
    └──────┬───────┘
           │
        DS18B20
```

### 6.3. Kết nối DS18B20

**Đấu dây:**
```
DS18B20 Chân 1 (VCC) →  ESP32 3V3
DS18B20 Chân 2 (DQ)  →  ESP32 GPIO 21
DS18B20 Chân 3 (GND) →  ESP32 GND

Điện trở 4.7kΩ:
  Một đầu → GPIO 21
  Một đầu → 3V3
```

**Lưu ý:**
- Dùng 3V3, không dùng 5V
- Điện trở BẮT BUỘC
- Nếu dùng DS18B20 chống nước, có thể có 3 dây màu:
  - Đỏ: VCC
  - Vàng: DQ
  - Đen: GND

**✅ Hoàn thành khi:**
- 3 dây nối đúng chân
- Điện trở 4.7kΩ nối giữa GPIO 21 và 3V3

### 6.4. Test DS18B20

**Mở Serial Monitor:**
```bash
python -m platformio device monitor
```

**Xem log:**
```
[SENSORS] Found 1 DS18B20 sensor(s)
[STATUS] Temperature: 26.8°C
```

**Nếu lỗi:**
- `-127°C` → Thiếu điện trở hoặc dây DQ đứt
- `0 sensors` → Sai chân hoặc sensor hỏng

---

## 💦 BƯỚC 7: CẢM BIẾN ĐỘ ẨM ĐẤT

### 7.1. Xác định chân Soil Sensor

**Capacitive Soil Moisture Sensor có 3-4 chân:**

```
┌─────────────────┐
│  Soil Sensor    │
│                 │
│ VCC GND AO (DO) │
│  ●   ●   ●  (●) │
└─────────────────┘
```

**VCC:** Nguồn (3.3V)
**GND:** Ground
**AO:** Analog Output (nối ESP32)
**DO:** Digital Output (không dùng)

### 7.2. Kết nối Soil Sensor

**Đấu dây:**
```
Soil Sensor VCC →  ESP32 3V3
Soil Sensor GND →  ESP32 GND
Soil Sensor AO  →  ESP32 GPIO 35
```

**⚠️ CẢNH BÁO QUAN TRỌNG:**
- **PHẢI dùng 3.3V**, KHÔNG dùng 5V
- ADC của ESP32 chỉ chịu được 3.3V
- Cấp 5V → hỏng ESP32 vĩnh viễn

**⚠️ GPIO 35, KHÔNG PHẢI GPIO 34:**
- Code đã đổi từ GPIO 34 sang GPIO 35
- GPIO 34 bị nhiễu WiFi
- Nếu dùng GPIO 34 → độ ẩm nhảy loạn

### 7.3. Test Soil Sensor

**Mở Serial Monitor:**
```bash
python -m platformio device monitor
```

**Xem log:**
```
[STATUS] Moisture: 43.2%
```

**Test thử:**
- Để sensor trong không khí → ~0-20%
- Nhúng vào nước → ~80-100%
- Nếu luôn 0% hoặc 100% → sensor hỏng hoặc sai điện áp

**✅ Hoàn thành khi:**
- Sensor hiển thị giá trị hợp lý
- Giá trị thay đổi khi nhúng vào nước

---


## ✅ BƯỚC 8: KIỂM TRA TỔNG THỂ

### 8.1. Checklist trước khi bật nguồn

**Nguồn điện:**
- [ ] LM2596 OUT+ = 5.00V (đã đo)
- [ ] Adapter 12V 2A
- [ ] Tất cả dây nguồn nối chắc chắn

**ESP32:**
- [ ] VIN nối LM2596 OUT+ (5V)
- [ ] GND nối LM2596 OUT-
- [ ] Code đã nạp vào ESP32

**Relay:**
- [ ] VCC nối 5V
- [ ] GND nối GND
- [ ] IN1 nối GPIO 26
- [ ] IN2 nối GPIO 27
- [ ] COM (cả 2 kênh) nối 5V

**Bơm:**
- [ ] (+) nối Relay CH1 NO
- [ ] (-) nối GND
- [ ] Diode gắn song song, đúng chiều

**Quạt:**
- [ ] (+) nối Relay CH2 NO
- [ ] (-) nối GND
- [ ] Diode gắn song song, đúng chiều

**DS18B20:**
- [ ] VCC nối 3V3
- [ ] DQ nối GPIO 21
- [ ] GND nối GND
- [ ] Điện trở 4.7kΩ giữa GPIO 21 và 3V3

**Soil Sensor:**
- [ ] VCC nối 3V3 (KHÔNG phải 5V)
- [ ] GND nối GND
- [ ] AO nối GPIO 35 (KHÔNG phải GPIO 34)

**An toàn:**
- [ ] Không có dây trần chạm nhau
- [ ] Không có dây chạm vỏ kim loại
- [ ] Bơm/quạt không chạm nước (nếu test trong nhà)

### 8.2. Sơ đồ tổng thể

```
┌──────────────┐
│ Adapter 12V  │
└──────┬───────┘
       │
   ┌───▼────┐
   │ LM2596 │ OUT+ = 5V
   └───┬────┘
       │ 5V
       ├─────────────┬─────────────┬─────────────┬─────────────┐
       │             │             │             │             │
   ┌───▼────┐   ┌───▼────┐   ┌───▼────┐   ┌───▼────┐   ┌───▼────┐
   │ ESP32  │   │ Relay  │   │ Relay  │   │  Bơm   │   │  Quạt  │
   │  VIN   │   │  VCC   │   │ CH1 COM│   │   +    │   │   +    │
   │        │   │        │   │ CH2 COM│   │ Diode  │   │ Diode  │
   │ GPIO26─┼──→│  IN1   │   │        │   └────────┘   └────────┘
   │ GPIO27─┼──→│  IN2   │   │ CH1 NO─┼──→ Bơm (+)
   │ GPIO21─┼──→│DS18B20 │   │ CH2 NO─┼──→ Quạt (+)
   │ GPIO35─┼──→│ Soil   │   └────────┘
   │  3V3  ─┼──→│DS18B20 │
   │  3V3  ─┼──→│ Soil   │
   └────────┘   └────────┘

   GND: TẤT CẢ NỐI CHUNG (LM2596, ESP32, Relay, Bơm, Quạt, DS18B20, Soil)
```

### 8.3. Bảng kết nối đầy đủ

| Từ                | Đến                | Ghi chú                    |
|-------------------|--------------------|-----------------------------|
| Adapter 12V (+)   | LM2596 IN+         | Nguồn gốc                   |
| Adapter 12V (-)   | LM2596 IN-         | GND nguồn                   |
| LM2596 OUT+       | ESP32 VIN          | 5V                          |
| LM2596 OUT+       | Relay VCC          | 5V                          |
| LM2596 OUT+       | Relay CH1 COM      | 5V cho bơm                  |
| LM2596 OUT+       | Relay CH2 COM      | 5V cho quạt                 |
| LM2596 OUT-       | ESP32 GND          | GND chung                   |
| LM2596 OUT-       | Relay GND          | GND chung                   |
| LM2596 OUT-       | Bơm (-)            | GND chung                   |
| LM2596 OUT-       | Quạt (-)           | GND chung                   |
| LM2596 OUT-       | DS18B20 GND        | GND chung                   |
| LM2596 OUT-       | Soil Sensor GND    | GND chung                   |
| ESP32 GPIO 26     | Relay IN1          | Điều khiển bơm              |
| ESP32 GPIO 27     | Relay IN2          | Điều khiển quạt             |
| ESP32 GPIO 21     | DS18B20 DQ         | Đọc nhiệt độ                |
| ESP32 GPIO 35     | Soil Sensor AO     | Đọc độ ẩm                   |
| ESP32 3V3         | DS18B20 VCC        | Nguồn 3.3V                  |
| ESP32 3V3         | Soil Sensor VCC    | Nguồn 3.3V                  |
| ESP32 3V3         | Điện trở 4.7kΩ     | Pull-up DS18B20             |
| GPIO 21           | Điện trở 4.7kΩ     | Pull-up DS18B20             |
| Relay CH1 NO      | Bơm (+)            | Đóng/ngắt bơm               |
| Relay CH2 NO      | Quạt (+)           | Đóng/ngắt quạt              |
| Bơm (+)           | Diode Cathode      | Bảo vệ                      |
| Bơm (-)           | Diode Anode        | Bảo vệ                      |
| Quạt (+)          | Diode Cathode      | Bảo vệ                      |
| Quạt (-)          | Diode Anode        | Bảo vệ                      |

---

## 🧪 BƯỚC 9: TEST TỪNG PHẦN

### 9.1. Test nguồn

**Mục tiêu:** Đảm bảo nguồn 5V ổn định

**Cách test:**
1. Cắm adapter vào ổ điện
2. Đo LM2596 OUT+ = 5.00V
3. Đo ESP32 VIN = 5.00V
4. Đo Relay VCC = 5.00V

**✅ Pass:** Tất cả đều 5.00V ± 0.1V

### 9.2. Test ESP32

**Mục tiêu:** ESP32 khởi động và kết nối WiFi

**Cách test:**
1. Cắm USB vào máy tính
2. Mở Serial Monitor:
   ```bash
   python -m platformio device monitor
   ```
3. Xem log:
   ```
   [WIFI] Connected!
   [WIFI] IP Address: 192.168.1.86
   ```

**✅ Pass:** WiFi connected, có IP

### 9.3. Test DS18B20

**Mục tiêu:** Đọc nhiệt độ chính xác

**Cách test:**
1. Xem Serial Monitor:
   ```
   [STATUS] Temperature: 26.8°C
   ```
2. Cầm DS18B20 trong tay → nhiệt độ tăng lên ~30-35°C
3. Thả ra → nhiệt độ giảm về nhiệt độ phòng

**✅ Pass:** Nhiệt độ hợp lý, thay đổi khi cầm tay

**❌ Fail:**
- `-127°C` → Thiếu điện trở 4.7kΩ
- `0 sensors` → Sai chân hoặc sensor hỏng

### 9.4. Test Soil Sensor

**Mục tiêu:** Đọc độ ẩm chính xác

**Cách test:**
1. Xem Serial Monitor:
   ```
   [STATUS] Moisture: 43.2%
   ```
2. Để sensor trong không khí → ~0-20%
3. Nhúng vào nước → ~80-100%

**✅ Pass:** Giá trị thay đổi khi nhúng nước

**❌ Fail:**
- Luôn 0% hoặc 100% → Sensor hỏng hoặc sai điện áp
- Nhảy loạn → Dùng GPIO 34 thay vì GPIO 35

### 9.5. Test Relay (không tải)

**Mục tiêu:** Relay click khi điều khiển

**Cách test:**
1. Chưa nối bơm/quạt vào relay
2. Gửi lệnh demo:
   ```bash
   Invoke-RestMethod -Uri "https://bioloop-monitor.onrender.com/api/demo/override" -Method Post -ContentType "application/json" -Body '{"case_num":1,"fan_pwm":255,"pump_active":true}'
   ```
3. Nghe relay click (tiếng "tách")
4. Xem Serial Monitor:
   ```
   [DEMO] ⚠️  OVERRIDE ACTIVATED
   [DEBUG] Pump: state=ON → GPIO=HIGH
   [DEBUG] Fan: state=ON → GPIO=HIGH
   ```

**✅ Pass:** Relay click, log hiển thị ON

**❌ Fail:**
- Không click → Kiểm tra VCC, GND, IN1, IN2
- Click nhưng log OFF → Sai ACTIVE HIGH/LOW

### 9.6. Xác định ACTIVE HIGH/LOW

**Cách test:**

1. **Quan sát đèn LED trên relay:**
   - Khi log `GPIO=HIGH`, đèn LED sáng → ACTIVE HIGH ✅
   - Khi log `GPIO=LOW`, đèn LED sáng → ACTIVE LOW ❌

2. **Nếu là ACTIVE LOW, sửa code:**

Mở `src/main.cpp`, tìm:
```cpp
void setPump(bool state) {
  pumpActive = state;
  digitalWrite(PIN_PUMP_RELAY, state ? HIGH : LOW);  // ← Dòng này
```

Đổi thành:
```cpp
void setPump(bool state) {
  pumpActive = state;
  digitalWrite(PIN_PUMP_RELAY, state ? LOW : HIGH);  // ← Đảo ngược
```

Làm tương tự với `setFan()`.

3. **Nạp lại code:**
```bash
python -m platformio run -t upload
```

### 9.7. Test Bơm

**Mục tiêu:** Bơm chạy khi relay bật

**Cách test:**
1. Nối bơm vào relay CH1
2. Gửi lệnh demo (như 9.5)
3. Bơm phải chạy

**✅ Pass:** Bơm chạy

**❌ Fail:**
- Không chạy → Kiểm tra COM có nối 5V không
- Chạy ngược → Đổi cực (+) (-)

### 9.8. Test Quạt

**Mục tiêu:** Quạt quay khi relay bật

**Cách test:**
1. Nối quạt vào relay CH2
2. Gửi lệnh demo
3. Quạt phải quay

**✅ Pass:** Quạt quay

**❌ Fail:**
- Không quay → Kiểm tra COM có nối 5V không
- Quay ngược → Đổi cực (+) (-)

### 9.9. Test Demo Mode (tổng hợp)

**Mục tiêu:** Toàn bộ hệ thống hoạt động

**Cách test:**
1. Mở demo page: https://bioloop-monitor.onrender.com/demo.html
2. Click Case 1 (Critical)
3. Quan sát:
   - Serial Monitor: `[DEMO] OVERRIDE ACTIVATED`
   - Relay: Click
   - Bơm: Chạy
   - Quạt: Quay
   - Dashboard: Hiển thị Fan PWM 255, Pump ON
4. Đợi 30 giây:
   - Bơm/quạt tắt dần
   - Fan PWM giảm từ 255 → 40
5. Click "Tắt Demo Mode":
   - Bơm/quạt tắt ngay
   - Serial Monitor: `[DEMO] Override cleared`

**✅ Pass:** Tất cả hoạt động như mô tả

---


## 🔧 BƯỚC 10: TROUBLESHOOTING

### 10.1. Vấn đề nguồn điện

**Triệu chứng:** ESP32 không sáng đèn

**Nguyên nhân & Giải pháp:**
- LM2596 chưa chỉnh đúng 5V → Đo và chỉnh lại
- Adapter hỏng → Thử adapter khác
- Dây nguồn đứt → Kiểm tra continuity bằng đồng hồ
- ESP32 hỏng → Thử ESP32 khác

**Triệu chứng:** ESP32 reset liên tục

**Nguyên nhân & Giải pháp:**
- Nguồn không đủ dòng → Dùng adapter 2A trở lên
- Chập mạch → Kiểm tra dây trần
- Bơm/quạt kéo dòng quá lớn → Dùng nguồn riêng cho bơm/quạt

### 10.2. Vấn đề WiFi

**Triệu chứng:** WiFi không kết nối

**Nguyên nhân & Giải pháp:**
- Sai SSID/password → Kiểm tra `src/main.cpp`:
  ```cpp
  const char* WIFI_SSID = "Quoc Viet";
  const char* WIFI_PASSWORD = "Vy@020514";
  ```
- WiFi 5GHz → ESP32 chỉ hỗ trợ 2.4GHz
- Xa router → Di chuyển gần hơn

### 10.3. Vấn đề DS18B20

**Triệu chứng:** Nhiệt độ = -127°C

**Nguyên nhân & Giải pháp:**
- Thiếu điện trở 4.7kΩ → Gắn điện trở giữa GPIO 21 và 3V3
- Dây DQ đứt → Kiểm tra continuity
- Sai chân → Kiểm tra lại chân 1, 2, 3

**Triệu chứng:** Nhiệt độ = 85°C (cố định)

**Nguyên nhân & Giải pháp:**
- Sensor đang khởi động → Đợi vài giây
- Sensor hỏng → Thay sensor

**Triệu chứng:** Nhiệt độ nhảy loạn

**Nguyên nhân & Giải pháp:**
- Dây quá dài → Dùng dây ngắn hơn hoặc cáp chống nhiễu
- Nhiễu điện → Tách xa bơm/quạt

### 10.4. Vấn đề Soil Sensor

**Triệu chứng:** Độ ẩm = 0% hoặc 100% (cố định)

**Nguyên nhân & Giải pháp:**
- Cấp 5V thay vì 3.3V → Đổi sang 3V3 (có thể đã hỏng ESP32)
- Sensor hỏng → Thay sensor
- Dây AO đứt → Kiểm tra continuity

**Triệu chứng:** Độ ẩm nhảy loạn

**Nguyên nhân & Giải pháp:**
- Dùng GPIO 34 → Đổi sang GPIO 35 (code đã đổi)
- Nhiễu WiFi → GPIO 34 bị nhiễu, phải dùng GPIO 35

**Triệu chứng:** Độ ẩm > 100%

**Nguyên nhân & Giải pháp:**
- Chưa calibrate → Chỉnh `MOISTURE_AIR_VALUE` và `MOISTURE_WATER_VALUE` trong code
- Sensor quá ẩm → Lau khô sensor

### 10.5. Vấn đề Relay

**Triệu chứng:** Relay không click

**Nguyên nhân & Giải pháp:**
- Không có nguồn 5V → Kiểm tra VCC nối LM2596 OUT+
- Sai chân IN1/IN2 → Kiểm tra GPIO 26, 27
- Relay hỏng → Thay relay

**Triệu chứng:** Relay click nhưng bơm/quạt không chạy

**Nguyên nhân & Giải pháp:**
- COM không nối nguồn → Nối COM với 5V
- Nối sai NO/NC → Dùng NO, không dùng NC
- Bơm/quạt hỏng → Test trực tiếp bơm/quạt với 5V

**Triệu chứng:** Relay hoạt động ngược (ON khi OFF, OFF khi ON)

**Nguyên nhân & Giải pháp:**
- Relay là ACTIVE LOW → Sửa code (xem 9.6)

### 10.6. Vấn đề Bơm/Quạt

**Triệu chứng:** Bơm/quạt không chạy

**Nguyên nhân & Giải pháp:**
- Relay COM không nối nguồn → Nối COM với 5V
- Sai cực (+) (-) → Đổi cực
- Diode gắn ngược → Kiểm tra cathode (vạch trắng) nối (+)
- Bơm/quạt hỏng → Test trực tiếp với 5V

**Triệu chứng:** Bơm/quạt chạy yếu

**Nguyên nhân & Giải pháp:**
- Nguồn không đủ dòng → Dùng adapter 2A trở lên
- Dây quá dài/mỏng → Dùng dây ngắn hơn, dày hơn
- Bơm/quạt bị tắc → Vệ sinh bơm/quạt

**Triệu chứng:** Relay/ESP32 hỏng sau khi chạy

**Nguyên nhân & Giải pháp:**
- Không có diode bảo vệ → Gắn diode 1N5819/1N4007
- Diode gắn sai chiều → Kiểm tra lại

### 10.7. Vấn đề Demo Mode

**Triệu chứng:** Demo mode không hoạt động

**Nguyên nhân & Giải pháp:**
- Server chưa deploy → Đợi 1-2 phút
- ESP32 không poll server → Kiểm tra log `[DEMO] Checking override`
- WiFi mất → Kiểm tra kết nối WiFi

**Triệu chứng:** Bơm/quạt không tắt dần

**Nguyên nhân & Giải pháp:**
- Web chưa deploy code mới → Đợi deploy hoặc xóa cache browser
- ESP32 poll chậm → Bình thường, poll mỗi 3 giây

**Triệu chứng:** Demo mode không tự tắt

**Nguyên nhân & Giải pháp:**
- Chưa click "Tắt Demo Mode" → Click nút tắt
- Hoặc đợi 5 phút → Auto-expire

---

## 📸 BƯỚC 11: HÌNH ẢNH THAM KHẢO

### 11.1. LM2596 Module

```
Mặt trên:
┌─────────────────────────┐
│      LM2596 Module      │
│                         │
│  IN+  IN-  OUT+ OUT-    │
│   ●    ●    ●    ●      │
│                         │
│      [Biến trở]         │ ← Vặn để chỉnh điện áp
│         ╱               │
│        ╱                │
└─────────────────────────┘
```

### 11.2. Relay Module 2 Kênh

```
Mặt trên:
┌─────────────────────────────────────┐
│         RELAY MODULE 2CH            │
├─────────────────────────────────────┤
│  VCC  GND  IN1  IN2                 │
│   ●    ●    ●    ●                  │
│                                     │
│  [LED1]  [RELAY1]  [LED2]  [RELAY2]│
│                                     │
│  COM  NO  NC       COM  NO  NC      │
│   ●   ●   ●         ●   ●   ●      │
│  └─CH1─────┘       └─CH2─────┘     │
└─────────────────────────────────────┘
```

### 11.3. DS18B20 (mặt chữ hướng về bạn)

```
   ┌─────────┐
   │ DS18B20 │
   │         │
   │  TO-92  │
   └─┬───┬───┬┘
     1   2   3
     │   │   │
    VCC DQ GND
     │   │   │
    3V3 GPIO GND
         21
         │
      4.7kΩ
         │
        3V3
```

### 11.4. Soil Sensor

```
┌─────────────────┐
│  Capacitive     │
│  Soil Moisture  │
│     Sensor      │
│                 │
│ VCC GND AO  DO  │
│  ●   ●   ●  (●) │
│  │   │   │       │
│ 3V3 GND GPIO35  │
└─────────────────┘
```

### 11.5. Diode (1N5819 / 1N4007)

```
Diode:
  ┌──┐
──┤▶├──
  └──┘
  │  │
  │  └─ Anode (không có vạch)
  └──── Cathode (có vạch trắng)

Gắn song song với bơm/quạt:
        Bơm/Quạt
    ┌──────────┐
(+) ●          ● (-)
    │          │
    │   ┌──┐   │
    └───┤▶├───┘
        └──┘
     Cathode Anode
     (vạch)
```

---

## 📝 BƯỚC 12: GHI CHÚ QUAN TRỌNG

### 12.1. Điện áp

- **12V:** Chỉ dùng cho LM2596 IN
- **5V:** ESP32 VIN, Relay VCC, Relay COM, Bơm, Quạt
- **3.3V:** DS18B20, Soil Sensor

**⚠️ KHÔNG BAO GIỜ:**
- Cấp 12V trực tiếp cho ESP32 → Cháy
- Cấp 5V cho Soil Sensor → Hỏng ESP32
- Cấp 5V cho DS18B20 → Có thể hỏng sensor

### 12.2. GND chung

**TẤT CẢ thiết bị phải chung GND:**
- LM2596 OUT-
- ESP32 GND
- Relay GND
- Bơm (-)
- Quạt (-)
- DS18B20 GND
- Soil Sensor GND

**Không chung GND = không hoạt động!**

### 12.3. GPIO

**Đã sử dụng:**
- GPIO 21: DS18B20 DQ
- GPIO 26: Relay IN1 (Bơm)
- GPIO 27: Relay IN2 (Quạt)
- GPIO 35: Soil Sensor AO

**KHÔNG dùng:**
- GPIO 34: Bị nhiễu WiFi
- GPIO 0, 2, 15: Boot pins
- GPIO 6-11: Flash pins

### 12.4. Relay

**Relay chỉ đóng/ngắt mạch, KHÔNG cấp nguồn!**

**Đúng:**
```
5V → COM → (relay đóng) → NO → Tải (+)
```

**Sai:**
```
COM → NO → Tải (+) → 5V  ❌
```

### 12.5. Diode

**Diode gắn SONG SONG, không nối tiếp!**

**Đúng:**
```
    Bơm
┌────────┐
│  ┌──┐  │
└──┤▶├──┘  ← Song song
   └──┘
```

**Sai:**
```
5V → Diode → Bơm → GND  ❌ (nối tiếp)
```

### 12.6. Code

**Thông số quan trọng trong code:**
```cpp
// WiFi
const char* WIFI_SSID = "Quoc Viet";
const char* WIFI_PASSWORD = "Vy@020514";

// GPIO
#define PIN_TEMP_SENSOR   21    // DS18B20
#define PIN_MOISTURE      35    // Soil sensor (ĐÃ ĐỔI từ 34)
#define PIN_PUMP_RELAY    26    // Bơm
#define PIN_FAN_RELAY     27    // Quạt

// Relay logic (ACTIVE HIGH)
digitalWrite(PIN_PUMP_RELAY, state ? HIGH : LOW);
// Nếu relay là ACTIVE LOW, đổi thành:
// digitalWrite(PIN_PUMP_RELAY, state ? LOW : HIGH);
```

---

## 🎯 BƯỚC 13: HOÀN THÀNH

### 13.1. Checklist cuối cùng

- [ ] Tất cả thiết bị hoạt động
- [ ] WiFi connected
- [ ] DS18B20 đọc nhiệt độ chính xác
- [ ] Soil sensor đọc độ ẩm chính xác
- [ ] Relay click khi điều khiển
- [ ] Bơm chạy khi relay bật
- [ ] Quạt quay khi relay bật
- [ ] Demo mode hoạt động
- [ ] Dashboard hiển thị dữ liệu real-time
- [ ] Không có dây trần, không chập mạch

### 13.2. Sử dụng hệ thống

**Chế độ tự động:**
- Hệ thống tự động đọc cảm biến
- Fuzzy logic tự động điều khiển bơm/quạt
- Dữ liệu gửi lên dashboard mỗi 5 giây

**Chế độ demo:**
- Mở: https://bioloop-monitor.onrender.com/demo.html
- Click case để test
- Bơm/quạt hoạt động theo case
- Tự động tắt sau 30 giây hoặc click "Tắt Demo Mode"

**Xem dashboard:**
- Mở: https://bioloop-monitor.onrender.com
- Xem biểu đồ real-time
- Xem giai đoạn ủ hiện tại
- Xem lịch sử dữ liệu

### 13.3. Bảo trì

**Hàng ngày:**
- Kiểm tra bơm có nước không
- Kiểm tra quạt có quay không
- Xem dashboard có dữ liệu không

**Hàng tuần:**
- Vệ sinh soil sensor (lau sạch)
- Kiểm tra dây điện có lỏng không
- Kiểm tra nhiệt độ/độ ẩm có hợp lý không

**Hàng tháng:**
- Vệ sinh bơm (tháo lọc)
- Vệ sinh quạt (thổi bụi)
- Kiểm tra adapter có nóng bất thường không

### 13.4. Liên hệ hỗ trợ

**Nếu gặp vấn đề:**
1. Đọc lại phần Troubleshooting
2. Kiểm tra Serial Monitor để xem log
3. Chụp ảnh đấu dây và gửi để được hỗ trợ

**Thông tin hệ thống:**
- Code: https://github.com/haidangcoder/bioloop-monitor
- Dashboard: https://bioloop-monitor.onrender.com
- Demo: https://bioloop-monitor.onrender.com/demo.html

---

## 🎉 CHÚC MỪNG!

Bạn đã hoàn thành đấu dây hệ thống BioLoop Monitor!

Hệ thống giờ có thể:
- ✅ Tự động đọc nhiệt độ và độ ẩm
- ✅ Tự động điều khiển bơm/quạt bằng Fuzzy Logic
- ✅ Gửi dữ liệu lên cloud real-time
- ✅ Hiển thị dashboard trên web
- ✅ Demo các test case
- ✅ Phát hiện giai đoạn ủ phân

**Chúc bạn ủ phân thành công! 🌱**
