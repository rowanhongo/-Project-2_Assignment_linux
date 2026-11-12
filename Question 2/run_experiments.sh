#!/bin/bash
# builds both programs, makes a 100 MB file and runs every buffer size 3 times
# usage: ./run_experiments.sh   (run it from a folder on the linux filesystem, not /mnt/c)

gcc -Wall -Wextra -o copy_sys copy_sys.c
gcc -Wall -Wextra -o copy_stdio copy_stdio.c

dd if=/dev/urandom of=big.bin bs=1M count=100 2> /dev/null
echo "source checksum:"
sha256sum big.bin

echo
echo "== timing runs (seconds), 3 runs each"

for prog in copy_sys copy_stdio
do
    for size in 512 4096 65536
    do
        total=0
        echo "$prog buffer $size"

        for run in 1 2 3
        do
            t=$(./$prog big.bin out.bin $size | awk '{print $3}')
            echo "  run $run: $t"
            total=$(echo "$total $t" | awk '{print $1 + $2}')

            if cmp -s big.bin out.bin
            then
                echo "  cmp: identical"
            else
                echo "  cmp: DIFFERENT"
            fi
        done

        echo "  average: $(echo "$total" | awk '{printf "%.6f", $1 / 3}')"
    done
done

echo
echo "== error handling"
./copy_sys missing.bin out.bin 4096
./copy_stdio missing.bin out.bin 4096

rm -f out.bin
