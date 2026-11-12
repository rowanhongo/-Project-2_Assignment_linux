#!/bin/bash
# runs strace -c on both programs for every buffer size
# needs big.bin and the compiled programs from run_experiments.sh

for prog in copy_sys copy_stdio
do
    for size in 512 4096 65536
    do
        echo "===== strace -c ./$prog big.bin out.bin $size"
        strace -c ./$prog big.bin out.bin $size 2>&1
        echo
    done
done

rm -f out.bin
