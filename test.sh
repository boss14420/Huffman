#!/bin/bash - 
#===============================================================================
#
#          FILE: test.sh
# 
#         USAGE: ./test.sh 
# 
#   DESCRIPTION: 
# 
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: BOSS14420 (), 
#  ORGANIZATION: 
#       CREATED: 09/09/2014 08:34
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

word_length=16

python2 -c "print '|' + '-'*17 + '|' + '-'*12 + '|' + '-'*10 + \
                '|' + '-'*11 + '|' + '-'*15 + '|' + '-'*10 + '|' + '-'*9 + '|'"
printf "| %-15s | %-10s | %-8s | %-9s | %-13s | %-8s | %-6s |\n" \
    "File" "Dung lượng" "File nén" "Tỉ lệ nén" "Thời gian nén" "Giải nén" "K/t MD5"
python2 -c "print '|' + '-'*17 + '|' + '-'*12 + '|' + '-'*10 + \
                '|' + '-'*11 + '|' + '-'*15 + '|' + '-'*10 + '|' + '-'*9 + '|'"

for file in "$@"
do
    filename=$(basename "$file")
    hsz=$(/bin/ls -lh "$file" | cut -d" " -f5)iB
    printf "| %-15s | %-10s |" "$filename" $hsz

    ctime=$(/usr/bin/time -f "%e" ./hz c "$file" "$file".hz $word_length 2>&1 >/dev/null)
    hcsz=$(/bin/ls -lh "$file".hz | cut -d" " -f5)iB
    sz=$(stat --printf "%s" "$file")
    csz=$(stat --printf "%s" "$file".hz)
    ratio=$(echo "$sz / $csz" | bc -l)
    ratio=$(printf "%.03f" $ratio)
    printf " %-8s | %-09s | %-13s |" $hcsz $ratio "$ctime"s

    xtime=$(/usr/bin/time -f "%e" ./hz x "$file".hz "$file".ex 2>&1 >/dev/null)
    printf " %-8s |" "$xtime"s
    #printf "| %-15s | %-10s | %-8s | %-09s | %-13s | %-8s |\n" \
    #    "$filename" $hsz $hcsz $ratio "$ctime"s "$xtime"s

    md51=$(md5sum "$file" | cut -d" " -f1)
    md52=$(md5sum "$file".ex | cut -d" " -f1)
    if [[ $md51 != $md52 ]]; then
        printf " %-7s |\n" "Lỗi"
    else
        printf " %-7s |\n" "OK"
        rm "$file".hz "$file".ex
    fi
done

python2 -c "print '|' + '-'*17 + '|' + '-'*12 + '|' + '-'*10 + \
                '|' + '-'*11 + '|' + '-'*15 + '|' + '-'*10 + '|' + '-'*9 + '|'"
