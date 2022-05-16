make
for FILE in ./testcases/basic/*.in; do
	echo "$FILE";
	tofile=${FILE%.*};
	tofile+=".out";
	echo "$tofile";
	time ./pa2 $FILE $tofile   
#./pa2 ./testcases/basic/case01.in ./case01.out
	./ver $FILE $tofile
done
