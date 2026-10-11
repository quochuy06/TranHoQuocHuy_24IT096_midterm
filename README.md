# Midterm Project - Implement `ls(1)`

## Thông Tin Sinh Viên

- Họ và tên: **Trần Hồ Quốc Huy**
- Mã sinh viên: **24IT096**
- Lớp học phần: **Lập trình hệ thống 4**
- GitHub repository: **https://github.com/quochuy06/TranHoQuocHuy_24IT096_midterm**

## Giới Thiệu

Dự án này cài đặt một phiên bản rút gọn của lệnh UNIX `ls(1)` bằng ngôn ngữ C.
Chương trình được viết dựa trên manual `ls(1)` NetBSD 10.1 do giảng viên cung
cấp. Tên chương trình khi biên dịch là `myls` để tránh trùng với lệnh `ls` có
sẵn của hệ thống.

Mục tiêu chính:

- Đọc metadata của tệp và thư mục bằng các system call POSIX/UNIX.
- Liệt kê nội dung thư mục, sắp xếp và định dạng kết quả theo các tùy chọn được giao.
- Xử lý lỗi và các trường hợp biên một cách ổn định.
- Tổ chức mã nguồn thành nhiều file `.c` và header `.h`.
- Cung cấp `Makefile`, README/report và `.gitignore` đúng yêu cầu nộp bài.

Chương trình không gọi `system("ls")`, `popen()` hay thực thi lệnh `ls` của hệ
thống để tạo kết quả.

## Môi Trường Biên Dịch

Đã biên dịch thành công trên:

- NetBSD 10.1 amd64
- Trình biên dịch hệ thống `cc`
- `make` của NetBSD

Chương trình sử dụng các API UNIX/POSIX như `opendir()`, `readdir()`, `lstat()`,
`stat()`, `readlink()`, `getpwuid()`, `getgrgid()` và `localtime_r()`. Vì vậy,
nên biên dịch trên NetBSD, Linux, WSL hoặc môi trường UNIX tương thích POSIX.
Không nên biên dịch trực tiếp bằng môi trường Windows thông thường.

## Cách Biên Dịch Và Chạy

Tại thư mục gốc của dự án:

```sh
make
```

Lệnh trên tạo executable:

```sh
./myls
```

Một số ví dụ chạy:

```sh
./myls
./myls -A
./myls -al
./myls -F src
./myls -R src
./myls -dF src
./myls -n README.md
./myls -i -s README.md
```

Xóa file biên dịch:

```sh
make clean
```

## Các Tùy Chọn Đã Hỗ Trợ

Chương trình hỗ trợ đầy đủ các tùy chọn trong manual rút gọn được giao:

| Tùy chọn | Chức năng |
|---|---|
| `-A` | Liệt kê tất cả entry trừ `.` và `..`; tự bật khi chạy bằng super-user. |
| `-a` | Liệt kê cả các entry bắt đầu bằng dấu chấm, gồm `.` và `..`. |
| `-c` | Dùng thời gian thay đổi status (`ctime`) cho `-t` và long format. |
| `-d` | In thư mục như một tệp thường, không liệt kê nội dung bên trong. |
| `-F` | Thêm ký hiệu phân loại: `/`, `*`, `@`, `%`, `=`, `|`. |
| `-f` | Không sắp xếp, giữ thứ tự đọc được. |
| `-h` | In kích thước/block theo định dạng dễ đọc, ví dụ `4.9K`. |
| `-i` | In inode number. |
| `-k` | In block theo đơn vị 1024 byte cho `-s`. |
| `-l` | In long format với owner/group name nếu tra cứu được. |
| `-n` | Giống `-l` nhưng in UID/GID bằng số. |
| `-q` | Thay ký tự không in được trong tên file bằng `?`. |
| `-R` | Liệt kê đệ quy các thư mục con. |
| `-r` | Đảo ngược thứ tự sắp xếp. |
| `-S` | Sắp xếp theo kích thước, file lớn hơn đứng trước. |
| `-s` | In số block thật sự được cấp phát cho mỗi file. |
| `-t` | Sắp xếp theo thời gian, mới hơn đứng trước. |
| `-u` | Dùng access time (`atime`) cho `-t` và long format. |
| `-w` | In raw tên file, không thay ký tự không in được. |

## Quy Tắc Xử Lý Chính

- Nếu không có operand, chương trình liệt kê thư mục hiện tại (`.`).
- Nếu có nhiều operand, non-directory được in trước directory; mỗi nhóm được sắp xếp riêng.
- Tên file ẩn không được hiển thị mặc định, trừ khi dùng `-a`, `-A` hoặc chạy bằng super-user.
- Long format in mode, hard link count, owner, group, size, thời gian và tên file.
- Symlink trong long format được in thêm `-> target`.
- Symlink tới thư mục khi là operand được xử lý tương tự NetBSD `ls`:
  - Chế độ mặc định: đi vào thư mục đích.
  - Có `-F`, `-d` hoặc long format: giữ bản thân symlink và in dấu `@` nếu cần.
- Với `-R`, chương trình không đi theo symlink con bên trong thư mục, tránh lặp vô hạn.
- Chương trình phát hiện chu trình thư mục bằng cặp device/inode trong chuỗi tổ tiên.
- Lỗi với một operand được báo qua `stderr`, các operand khác vẫn tiếp tục được xử lý.
- Exit status là `0` khi thành công và khác `0` khi có lỗi.

## BLOCKSIZE Và Thời Gian

`BLOCKSIZE` ảnh hưởng đến block count khi không dùng `-h` hoặc `-k`.
Giá trị mặc định là 512 byte.

Ví dụ:

```sh
BLOCKSIZE=2048 ./myls -s
BLOCKSIZE=2K ./myls -l
```

Biến môi trường `TZ` được hệ thống dùng khi định dạng thời gian:

```sh
TZ=UTC ./myls -l
TZ=Asia/Ho_Chi_Minh ./myls -l
```

## Cấu Trúc Thư Mục

```text
ls_midterm/
  include/
    types.h
    options.h
    util.h
    entries.h
    format.h
    listing.h
  src/
    main.c
    options.c
    util.c
    entries.c
    format.c
    listing.c
  Makefile
  README.md
  .gitignore
```

Vai trò các file chính:

| File | Vai trò |
|---|---|
| `src/main.c` | Khởi tạo locale, parse option, điều phối chương trình và trả exit status. |
| `src/options.c` | Xử lý option bằng `getopt()` và các quy tắc ưu tiên giữa option. |
| `src/util.c` | Cấp phát bộ nhớ an toàn, ghép path, báo lỗi, in tên file theo `-q`/`-w`. |
| `src/entries.c` | Lưu danh sách entry, chọn thời gian, sắp xếp bằng merge sort. |
| `src/format.c` | Định dạng long format, permission, size, block, inode và symlink target. |
| `src/listing.c` | Phân nhóm operand, đọc thư mục, lọc hidden file và xử lý đệ quy. |
| `include/*.h` | Khai báo kiểu dữ liệu và hàm dùng chung giữa các module. |

## Kiểm Thử

### Kiểm Thử Trên NetBSD

Đã kiểm tra trên NetBSD 10.1:

```sh
make clean
make
```

Kết quả: biên dịch thành công bằng `cc` của hệ thống NetBSD.

Đã chạy thủ công các trường hợp:

```sh
./myls -A
./myls -al
./myls -R src
./myls -dF src
./myls -n README.md
```

Ngoài ra đã tạo thư mục test trong `/tmp/mylscheck` để kiểm tra:

- File thường và file ẩn.
- Thư mục con và đệ quy `-R`.
- File executable với `-F`.
- Symlink thường.
- Symlink hỏng.
- Symlink tới thư mục.
- FIFO.
- Long format `-l` và numeric format `-n`.
- Block count `-s`.
- Operand không tồn tại.

Một số trường hợp quan trọng được so sánh trực tiếp với `/bin/ls` của NetBSD,
đặc biệt là symlink tới thư mục với chế độ mặc định, `-F` và `-dF`.

## Giới Hạn

- NetBSD và Linux có khác biệt nhỏ về một số loại file đặc biệt. Chương trình
  có xử lý `whiteout` nếu hệ thống cung cấp macro `S_ISWHT`.
- Archive-state type `a`/`A` được manual NetBSD liệt kê nhưng không phổ biến
  trên môi trường kiểm tra; chương trình không tạo riêng loại file này.
- Định dạng human-readable được cài đặt theo cơ số 1024 và có thể khác nhỏ so
  với `humanize_number()` của NetBSD trong một vài trường hợp làm tròn.
