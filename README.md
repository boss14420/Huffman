# CHƯƠNG TRÌNH NÉN VÀ GIẢI NÉN FILE

## Chạy chương trình
    
Để nén một file `filename` và tạo ra file nén `filename.compress`, ta sử dụng
lệnh:
    `./huffman c filename filename.compress`

Giải nén file nén trên thành file `filename.extract`, ta sử dụng lệnh:
    `./huffman x filename.compress filename.extract`

## Kết quả chạy

| file         | dung lượng | tỉ lệ nén | thời gian |
|--------------|------------|-----------|-----------|
| E.coli       | 4.5 MiB    |  4        | 0.591s    |
| alice29.txt  | 149 KiB    |  1.73     | 0.034s    |
| asyoulik.txt | 123 KiB    |  1.65     | 0.019s    |
| bible.txt    | 3.9 MiB    |  1.82     | 0.385s    |
| cp.html      | 25  KiB    |  1.51     | 0.011s    |
| fields.c     | 11  KiB    |  1.56     | 0.006s    |
| grammar.lsp  | 3.7 KiB    |  1.63     | 0.007s    |
| kennedy.xls  | 1006 KiB   |  2.22     | 0.081s    |
| lcet10.txt   | 417 KiB    |  1.70     | 0.073s    |
| plrabn12.txt | 471 KiB    |  1.74     | 0.040s    |
| ptt5         | 502 KiB    |  4.81     | 0.041s    |
| sum          |  38 KiB    |  1.47     | 0.008s    |
| world192.txt | 2.4 MiB    |  1.59     | 0.184s    |
| xargs.1      | 4.2 KiB    |  1.56     | 0.005s    |

## Thiết kế chương trình

### Thuật toán

Thuật toán nén và giải nén được sử dụng trong chương trình là một thuật toán
dựa trến thuật toán Huffman. Thuật toán này nhận đầu vào là bảng phân bố tần
suất của các từ và trả về độ dài của từ mã dùng để mã hóa từ đó. Thuật toán này
được cài đặt trong phương thức `Huffman::calculate_code_length()`

Sau khi đã có các độ dài từ mã, các từ mã sẽ được sinh ra bằng phương thức
`Huffman::gen_codewords()`. Các từ mã này là mã tiền tổ chuẩn.

Với phương pháp sinh các từ mã này, trong trường hợp tất cả các từ mã được tính
ra đều có cùng độ dài (tức là không nén được file) thì giá trị của một từ cũng
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
Nội dung 1 file bao gồm:

1. Phần tiêu đề (xem phương thức `Huffman::write_header()`)
    1. Magic number: từ `COMPRESS` (6 byte)
    2. Kích thước file gốc: 8 byte, được ghi theo kiểu Big Endian
    3. Số lượng từ trong file gốc: 8 byte, Big Endian
    4. Độ dài một từ (`wl` tính theo bit): 1 byte
    5. Độ dài nhỏ nhất của các từ mã (`min_codelength`): 1 byte
    6. Độ dài lớn nhất của các từ mã (`max_codelength`): 1 byte
    7. Danh sách số lượng các từ mã với độ dài từ `min_codelength` cho đên
    `max_codelength`. Mỗi một số được ghi bở `wl` bit.
    8. Chuỗi `words_string`, mỗi một từ được ghi bởi `wl` bit.
2. Phần nội dung: chuỗi các từ mã tương ứng với mỗi từ trong file gốc.

Chương trình giải nén cần đọc thông tin từ phần tiêu để đề khôi phục các cấu
trúc dữ liệu cần cho việc giải nén theo thuật toán đã nói trên. Phương thức
`Huffman::read_header()` sẽ làm việc này.
