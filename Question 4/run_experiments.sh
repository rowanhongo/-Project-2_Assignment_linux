#!/bin/bash
# runs logscan with 1, 2, 4 and 8 threads, 3 runs each, then checks the total with grep
# usage: ./run_experiments.sh   (run it from a folder on the linux filesystem, not /mnt/c)

gcc -Wall -Wextra -pthread -o logscan logscan.c

echo "== log files"
ls -l logs
echo

echo "== grep check"
grep -c ERROR logs/*.log
grep -c ERROR logs/*.log | awk -F: '{sum += $2} END {print "grep total:", sum}'
echo

# one run first so the files are already in the page cache for every thread count
./logscan ERROR report.txt 1 logs/*.log > /dev/null

echo "== timing runs (seconds), 3 runs each"

for t in 1 2 4 8
do
    total=0
    echo "threads $t"

    for run in 1 2 3
    do
        out=$(./logscan ERROR report.txt $t logs/*.log)
        time=$(echo "$out" | grep Elapsed | awk '{print $3}')
        echo "  run $run: $time   $(echo "$out" | head -1)"
        total=$(echo "$total $time" | awk '{print $1 + $2}')
    done

    avg=$(echo "$total" | awk '{printf "%.6f", $1 / 3}')
    echo "  average: $avg"
    echo "$t $avg" >> averages.tmp
done

echo
echo "== speedup (T1 / Tn)"
awk 'NR == 1 {t1 = $2} {printf "threads %d: average %.6f s, speedup %.2f\n", $1, $2, t1 / $2}' averages.tmp
rm averages.tmp

echo
echo "== report.txt from the last 8 thread run"
cat report.txt

echo
echo "== missing file test"
./logscan ERROR missing_report.txt 4 logs/server01.log logs/nothere.log logs/server02.log
cat missing_report.txt

rm -f logscan
