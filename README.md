# Hướng dẫn lập trình ngôn ngữ NH của Huy

## Giới thiệu
NH là ngôn ngữ lập trình siêu đơn giản do Huy tự viết, chạy trực tiếp trên Termux.
Hỗ trợ biến số, biến chuỗi, input, if, loop, print, so sánh cơ bản.

## Cú pháp cơ bản

### 1. Biến và gán giá trị
```nh
set ten_bien gia_tri

gia_tri có thể là số nguyên hoặc chuỗi (chuỗi đặt trong dấu " ").

Ví dụ:


set name "Huy"
set age 20
```

### 2. In ra màn hình
```nh
print x y z

In các biến hoặc chuỗi, cách nhau bởi dấu cách.

Ví dụ:


print name age "đang học lập trình"
```
### 3. Nhập dữ liệu từ bàn phím
```nh
input ten_bien

Nhập dữ liệu từ người dùng và lưu vào biến.

Ví dụ:


input age
```
### 4. Câu điều kiện if
```nh
if bien toan_tu gia_tri
    # câu lệnh
end

Toán tử hỗ trợ: ==, !=, >, <, >=, <=

Ví dụ:


if age > 15
    print "Bạn lớn hơn 15 tuổi"
end
```
### 5. Vòng lặp loop
```nh
loop so_lan
    # câu lệnh
end

Lặp lại đoạn code trong loop số lần.

Ví dụ:


loop 3
    print "Hello"
end

Ví dụ chương trình đầy đủ

set name "Huy"
print "Chào" name
input age
if age >= 18
    print "Bạn đủ tuổi"
end
loop 5
    print "Lặp lại đi"
end
```
## Cách chạy chương trình NH trên Termux

### 1. Compile chương trình nh interpreter:
```bash
clang nh.c -o nh
chmod +x nh
```
### 2. Copy nh vào thư mục chạy toàn cục:
```bash
mv nh $PREFIX/bin/
```
### 3. Viết file code NH, ví dụ test.nh, rồi chạy:


```bash
nh test.nh
```
