for i in `seq 1 3`;
do
	for j in `seq 10 -2 4`;
	do
		echo $i $j
		./main $((i*1200)) $j >> results.txt
	done;
done;
