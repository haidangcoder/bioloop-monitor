# Hướng Dẫn Đưa Web BioLoop Lên Online (MIỄN PHÍ)

## Cách 1: Ngrok (Nhanh nhất - 5 phút)

### Bước 1: Tải Ngrok
1. Truy cập: https://ngrok.com/download
2. Chọn "Windows (64-bit)" → Download file ZIP
3. Giải nén file `ngrok.exe` vào thư mục `E:\BioLoop Monitor`

### Bước 2: Đăng ký tài khoản (miễn phí)
1. Truy cập: https://dashboard.ngrok.com/signup
2. Đăng ký bằng email hoặc Google
3. Sau khi đăng nhập, copy "Authtoken" tại: https://dashboard.ngrok.com/get-started/your-authtoken

### Bước 3: Kích hoạt Ngrok
Mở PowerShell tại thư mục `E:\BioLoop Monitor` và chạy:
```powershell
.\ngrok.exe config add-authtoken YOUR_TOKEN_HERE
```
(Thay YOUR_TOKEN_HERE bằng token bạn vừa copy)

### Bước 4: Chạy Ngrok
```powershell
.\ngrok.exe http 3000
```

### Bước 5: Lấy URL Public
Ngrok sẽ hiển thị:
```
Forwarding   https://abc123xyz.ngrok-free.app -> http://localhost:3000
```

**URL này là URL public của bạn!** Chia sẻ với bất kỳ ai trên thế giới.

### Lưu ý:
- Server Node.js phải đang chạy (port 3000)
- Máy tính phải bật và kết nối internet
- URL sẽ thay đổi mỗi lần restart ngrok (trừ khi nâng cấp plan trả phí)
- Miễn phí: 1 tunnel, không giới hạn băng thông

---

## Cách 2: Render.com (Deploy lâu dài - 15 phút)

### Ưu điểm:
- Không cần máy tính bật 24/7
- URL cố định không đổi
- Tự động restart khi crash
- Miễn phí 750 giờ/tháng

### Nhược điểm:
- Phải đổi code ESP32 để gửi data tới URL mới
- Phải upload code lại ESP32

### Các bước:
1. Tạo tài khoản tại: https://render.com
2. Tạo GitHub repository cho code
3. Connect Render với GitHub
4. Deploy Node.js app
5. Cập nhật URL trong ESP32
6. Upload code mới lên ESP32

---

## Khuyến nghị cho Demo:

**Dùng Ngrok** vì:
- Setup 5 phút
- Không cần đổi code ESP32
- ESP32 vẫn gửi về `192.168.1.248:3000` (local)
- Ngrok tạo tunnel public tự động
- Phù hợp cho demo ngắn hạn

**Sau khi thi xong**, nếu muốn triển khai thực tế, hãy dùng Render.com hoặc VPS.

---

## Cần hỗ trợ?
Nếu gặp khó khăn, hãy:
1. Chạy lệnh download ngrok thủ công
2. Hoặc cho tôi biết bạn muốn deploy lên Render.com (tôi sẽ setup tự động)
