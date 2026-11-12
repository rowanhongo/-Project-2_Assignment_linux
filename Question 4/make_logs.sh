#!/bin/bash
# makes 12 log files from 1 MB to 50 MB in the logs folder

mkdir -p logs

i=0
for mb in 1 2 3 5 8 10 15 20 25 30 40 50
do
    i=$((i + 1))
    name=$(printf "logs/server%02d.log" $i)

    awk -v mb=$mb -v seed=$i 'BEGIN {
        srand(seed)
        split("INFO WARN ERROR DEBUG", level, " ")
        while (bytes < mb * 1048576)
        {
            line = sprintf("2024-05-%02d 12:%02d:%02d %s request %d handled in %dms",
                int(rand() * 28) + 1, int(rand() * 60), int(rand() * 60),
                level[int(rand() * 4) + 1], int(rand() * 100000), int(rand() * 900))
            print line
            bytes += length(line) + 1
        }
    }' > $name
done

ls -l logs
