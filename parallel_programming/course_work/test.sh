for i in `seq 1 3`;
do
	for j in `seq 4 -1 1`;
	do
		echo $i $j
		./main $((i*900)) $j >> results.txt
	done;
done;
