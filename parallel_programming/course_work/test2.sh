for i in `seq 1 3`;
do
	for j in `seq 40 -10 10`;
	do
		echo $i $j
		sh exec.sh $((i*960)) $j >> results2.txt
	done;
done;
