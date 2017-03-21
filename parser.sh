#!/bin/bash

FILENAME=$1
total=0
while IFS=: read xx yy;do
    a=( $yy )
    total=$(echo "scale=2;$total + ${a[0]}"|bc)
done < ${FILENAME}
echo "$total / $2" | bc -l
