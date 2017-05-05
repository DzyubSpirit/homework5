for i in `seq 1 3`;
do
	for j in `seq 10 -2 4`;
	do
		echo $i $j
		sh exec.sh $((i*900)) $j >> results2.txt
	done;
done;
