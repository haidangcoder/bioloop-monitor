# 🚀 Hướng Dẫn Deploy BioLoop Monitor lên Render.com

## Bước 1: Tạo tài khoản Render (2 phút)

1. Truy cập: https://render.com
2. Click **"Get Started for Free"**
3. Đăng ký bằng:
   - GitHub (khuyến nghị - dễ deploy)
   - Hoặc Email

---

## Bước 2: Chuẩn bị code (ĐÃ XONG ✅)

Code đã được chuẩn bị sẵn với:
- ✅ `render.yaml` - File cấu hình Render
- ✅ `package.json` - Dependencies
- ✅ PORT động từ environment variable
- ✅ Database SQLite

---

## Bước 3: Deploy lên Render

### Option A: Deploy từ GitHub (Khuyến nghị)

#### 3.1. Tạo GitHub Repository
1. Truy cập: https://github.com/new
2. Tên repo: `bioloop-monitor`
3. Chọn **Public** hoặc **Private**
4. Click **"Create repository"**

#### 3.2. Push code lên GitHub
Chạy các lệnh sau trong PowerShell tại thư mục `E:\BioLoop Monitor`:

```powershell
# Khởi tạo git (nếu chưa có)
git init

# Add tất cả file
git add .

# Commit
git commit -m "Initial commit - BioLoop Monitor"

# Add remote (thay YOUR_USERNAME bằng username GitHub của bạn)
git remote add origin https://github.com/YOUR_USERNAME/bioloop-monitor.git

# Push lên GitHub
git branch -M main
git push -u origin main
```

#### 3.3. Deploy từ Render Dashboard
1. Đăng nhập Render: https://dashboard.render.com
2. Click **"New +"** → **"Web Service"**
3. Connect GitHub repository `bioloop-monitor`
4. Cấu hình:
   - **Name**: `bioloop-monitor`
   - **Region**: `Singapore` (gần Việt Nam nhất)
   - **Branch**: `main`
   - **Root Directory**: `server`
   - **Environment**: `Node`
   - **Build Command**: `npm install`
   - **Start Command**: `node index.js`
   - **Instance Type**: `Free`
5. Click **"Create Web Service"**

---

### Option B: Deploy Manual (Không cần GitHub)

1. Đăng nhập Render: https://dashboard.render.com
2. Click **"New +"** → **"Web Service"**
3. Chọn **"Deploy an existing image from a registry"** → **"Public Git repository"**
4. Paste URL: `https://github.com/YOUR_USERNAME/bioloop-monitor` (hoặc upload manual)
5. Làm theo bước 3.3 ở trên

---

## Bước 4: Lấy URL của bạn

Sau khi deploy xong (3-5 phút), bạn sẽ có URL dạng:
```
https://bioloop-monitor.onrender.com
```

Hoặc:
```
https://bioloop-monitor-abc123.onrender.com
```

**Copy URL này!** Bạn sẽ cần nó cho bước tiếp theo.

---

## Bước 5: Cập nhật code ESP32

Sau khi có URL Render, tôi sẽ:
1. Cập nhật URL trong `src/main.cpp`
2. Build code mới
3. Upload lên ESP32

**URL mới sẽ có dạng:**
```cpp
const char* serverUrl = "https://bioloop-monitor.onrender.com/api/data";
```

---

## Bước 6: Test

1. Truy cập: `https://YOUR_URL.onrender.com`
2. Kiểm tra dashboard hiển thị
3. Đợi ESP32 gửi data (sau khi upload code mới)
4. Refresh dashboard để thấy data real-time

---

## ⚠️ Lưu ý quan trọng:

### Cold Start (Free Tier)
- Server sẽ **sleep sau 15 phút không có traffic**
- Lần truy cập đầu tiên sau khi sleep: **chờ 30-60 giây**
- Giải pháp: Dùng cron job ping mỗi 10 phút (miễn phí)

### Database SQLite
- Database sẽ **reset mỗi khi deploy lại**
- Dữ liệu cũ sẽ mất
- Giải pháp: Nâng cấp lên PostgreSQL (miễn phí trên Render)

### HTTPS
- Render tự động cung cấp HTTPS
- ESP32 cần hỗ trợ HTTPS (đã có trong code)

---

## 🎯 Sau khi deploy xong:

✅ Web online 24/7 không cần máy tính bật
✅ URL cố định không đổi
✅ HTTPS tự động
✅ Auto-restart khi crash
✅ Miễn phí mãi mãi (750 giờ/tháng)

---

## Cần hỗ trợ?

Nếu gặp lỗi, gửi cho tôi:
1. Screenshot lỗi từ Render dashboard
2. Logs từ Render (tab "Logs")
3. Tôi sẽ fix ngay!

---

## Bước tiếp theo:

**Sau khi bạn deploy xong và có URL, hãy gửi URL cho tôi.**
Tôi sẽ cập nhật code ESP32 và upload lại!
