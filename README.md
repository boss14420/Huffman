# CHƯƠNG TRÌNH NÉN VÀ GIẢI NÉN FILE

## Chạy chương trình
    
Để nén một file `filename` và tạo ra file nén `filename.hz`, ta sử dụng
lệnh:
    `./hz c filename filename.hz`

Để thay đổi độ dài từ (mặc định là 8bit), ta thêm độ dài từ vào sau câu lệnh
nén:
    `./hz c filename filename.hz 16`

Giải nén file nén trên thành file `filename.ex`, ta sử dụng lệnh:
    `./hz x filename.hz filename.ex`

## Kết quả chạy

Chương trình được chạy thử trên máy tính với CPU Intel Pentium B940 2GHz, RAM 3GiB, tốc độ
quay của HDD là 5400rpm. Hệ điều hành Linux 3.16.1, 64bit.

Độ dài mỗi từ là 16bit.

|
|-----------------|------------|----------|-----------|---------------|----------|---------|
| File            | Dung lượng | File nén | Tỉ lệ nén | Thời gian nén | Giải nén | K/t MD5 |
|-----------------|------------|----------|-----------|---------------|----------|---------|
| E.coli          | 4.5MiB     | 1.2MiB   | 4.000     | 0.07s         | 0.05s    | OK      |
| alice29.txt     | 149KiB     | 77KiB    | 1.939     | 0.02s         | 0.02s    | OK      |
| asyoulik.txt    | 123KiB     | 66KiB    | 1.877     | 0.02s         | 0.02s    | OK      |
| bible.txt       | 3.9MiB     | 1.9MiB   | 2.092     | 0.06s         | 0.11s    | OK      |
| cp.html         | 25KiB      | 16KiB    | 1.560     | 0.02s         | 0.01s    | OK      |
| fields.c        | 11KiB      | 6.8KiB   | 1.623     | 0.01s         | 0.02s    | OK      |
| grammar.lsp     | 3.7KiB     | 2.5KiB   | 1.513     | 0.02s         | 0.02s    | OK      |
| kennedy.xls     | 1006KiB    | 405KiB   | 2.488     | 0.03s         | 0.03s    | OK      |
| lcet10.txt      | 417KiB     | 217KiB   | 1.922     | 0.02s         | 0.02s    | OK      |
| plrabn12.txt    | 471KiB     | 236KiB   | 1.999     | 0.02s         | 0.02s    | OK      |
| ptt5            | 502KiB     | 80KiB    | 6.318     | 0.02s         | 0.02s    | OK      |
| sum             | 38KiB      | 25KiB    | 1.541     | 0.02s         | 0.01s    | OK      |
| world192.txt    | 2.4MiB     | 1.3MiB   | 1.834     | 0.05s         | 0.08s    | OK      |
| xargs.1         | 4.2KiB     | 3.0KiB   | 1.390     | 0.02s         | 0.01s    | OK      |
|-----------------|------------|----------|-----------|---------------|----------|---------|

|-----------------|------------|----------|-----------|---------------|----------|---------|
| File            | Dung lượng | File nén | Tỉ lệ nén | Thời gian nén | Giải nén | K/t MD5 |
|-----------------|------------|----------|-----------|---------------|----------|---------|
| music.flac      | 38MiB      | 38MiB    | 0.997     | 0.65s         | 0.85s    | OK      |
| leipzig1M.txt   | 124MiB     | 64MiB    | 1.956     | 1.44s         | 3.11s    | OK      |
| img.png         | 2.2MiB     | 2.3MiB   | 0.948     | 0.13s         | 0.06s    | OK      |
| doc.pdf         | 253MiB     | 239MiB   | 1.058     | 3.99s         | 8.65s    | OK      |
| img.png         | 2.2MiB     | 2.3MiB   | 0.948     | 0.15s         | 0.06s    | OK      |
|-----------------|------------|----------|-----------|---------------|----------|---------|

Bảng kết quả trên được tạo ra bằng cách chạy script `test.sh` đi kèm:
    `./test.sh <all test file>`
Với `<all test file>` là các file cần nén.

## Thiết kế chương trình

### Thuật toán

Thuật toán nén và giải nén được sử dụng trong chương trình là một thuật toán
dựa trến thuật toán Huffman. Thuật toán này nhận đầu vào là bảng phân bố tần
suất của các từ và trả về độ dài của từ mã dùng để mã hóa từ đó. Thuật toán này
được cài đặt trong phương thức `Huffman::calculate_code_length()`

Sau khi đã có các độ dài từ mã, các từ mã sẽ được sinh ra bằng phương thức
`Huffman::gen_codewords()`. Các từ mã này là mã tiền tổ chuẩn.

Với phương pháp sinh các từ mã này, trong trường hợp tất cả các từ mã được tính
ra đều có cùng độ dài với độ dài từ (tức là không nén được file) thì giá trị của một từ cũng
chính là giá trị của từ mã tương ứng với nó. Do đó nội dung của file nén giống
hệt nội dung file gốc (trừ phần tiêu đề).

#### Giải nén
Để giải nén, ta sử dụng hai mảng `limit` và `base` và mảng các từ
`words_string` (đây đều là thuộc tính của lớp `Huffman`). Trong đó:

1. `limit`: `limit[l]` là giá trị nhị phân lớn nhất của các từ mã độ dài `l`.
Nếu không có từ mã độ dài `l` thì có thể gán `limit[l] = (limit[l-1] + 1)*2 - 1`
2. `words_string`: mảng các từ được sử dụng trong file gốc, được sắp xếp theo
thứ tự:
    1. Từ có từ mã tương ứng độ dài nhỏ xếp trước từ có từ mã độ dài lớn
    2. Nếu độ dài từ mã bằng nhau thì từ có giá trị nhị phân của từ mã lớn hơn
    đứng trước.
3. `base`: `base[l]` là vị trí của từ có từ mã tương ứng độ dài `l` đầu tiên
trong `words_string`.

Ví dụ: xét các từ và từ mã tương ứng:

    q       0
    w       100
    e       101
    r       110
    t       1110
    y       1111

Khi đó:

    limit           = 0     01   101    1111 
    
    words_string    = qreqyt

    base            = 0(q)  0    1(r)   3(y)

Với các cấu trúc dữ liệu như trên, từ mã có giá trị `v` với độ dài `l` sẽ tương ứng với từ
`words_string[base[l] + limit[l] - v]`

Vì từ mã là mã tiền tố chuẩn nên trong quá trình giải mã, nếu được giá trị `v`
với độ dài `l`, ta có thể kiểm tra `v` có phải là một từ mã độ dài `l` hay
không bằng cách so sánh với `limit[l]`, nếu bé hơn hoặc bằng thì là từ mã, nếu
lớn hơn thì đọc tiếp các bit tiếp theo và kiểm tra với các từ mã dài hơn.
Ta có thuật toán giải mã như sau:

    v = read min_codelength bits
    l = min_codelength
    while not eof:
        if v <= limit[l]:
            output words_string[base[l] + limit[l] - v]
            v = read min_codelength bits
            l = min_codelength
        else:
            v = (v << 1) | read 1 bits
            l = l + 1

### Cấu trúc file nén
Nội dung 1 file nén bao gồm:

1. Phần tiêu đề (xem phương thức `Huffman::write_header()`)
    1. Magic number: từ `COMPRESS` (8 byte)
    2. Kích thước phần tiêu đề tính theo byte (4 byte, Big Endian)
    3. Kích thước file gốc: 8 byte, được ghi theo kiểu Big Endian
    4. Số lượng từ trong file gốc: 8 byte, Big Endian
    5. Độ dài một từ (`wl` tính theo bit): 1 byte
    6. Độ dài nhỏ nhất của các từ mã (`min_codelength`): 1 byte
    7. Độ dài lớn nhất của các từ mã (`max_codelength`): 1 byte
    8. Danh sách số lượng các từ mã với độ dài từ `min_codelength` cho đên
    `max_codelength`. Mỗi một số được ghi bởi `wl` bit.
    9. Chuỗi `words_string`, mỗi một từ được ghi bởi `wl` bit.
    10. `offset`, vị trí của bit đầu tiên trong file gốc được mã hóa: 1 byte
    11. Nếu 9 phần trên chưa tròn byte thì chèn thêm các bit 0 vào cuối phần
    tiêu đề cho tròn byte.

Chú ý: Nếu `wl == min_codelength` (file không mã hóa được) thì tiêu đề
không có phần 8, 9, 10

2. Phần nội dung:
    1. `offset` bit đầu tiên trong file gốc
    2. Dãy các từ mã của các từ tương ứng trong file gốc
    3. Nếu sau khi mã hóa các từ mà vẫn còn lại một số bit lẻ ít hơn `wl` bit
    thì ghi nội dung các bit này vào file nén (không mã hóa)
    4. Chèn thêm các bit 0 vào sau nếu chưa đủ 1 byte.

Chương trình giải nén cần đọc thông tin từ phần tiêu để đề khôi phục các cấu
trúc dữ liệu cần cho việc giải nén theo thuật toán đã nói trên. Phương thức
`Huffman::read_header()` sẽ làm việc này.
