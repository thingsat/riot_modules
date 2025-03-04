#!/bin/bash

NCOLS=$1

VAR=$2

FILENAME=$3

echo "// Header for $VAR"
echo "// Generated from $FILENAME"

echo "#ifndef _DEF_$VAR"
echo "#define _DEF_$VAR"

NROWS=0

echo "const static char $VAR[][] = {"

if [ $NCOLS -eq 2 ] 
then

while IFS=';' read -r col1 col2
do
    echo  '{' '/*' $col1 '*/' '"'$col2'",' '},'
    NROWS=$(expr $NROWS + 1)
done < $FILENAME

elif [ $NCOLS -eq 3 ] 
then

while IFS=';' read -r col1 col2 col3
do
    echo  '{' '/*' $col1 '*/' '"'$col2'",' '"'$col3'",' '},'
    NROWS=$(expr $NROWS + 1)
done < $FILENAME

elif [ $NCOLS -eq 4 ] 
then

while IFS=';' read -r col1 col2 col3 col4
do
    echo  '{' '/*' $col1 '*/' '"'$col2'",' '"'$col3'",' '"'$col4'",' '},'
    NROWS=$(expr $NROWS + 1)
done < $FILENAME

else

echo "ERROR: can not proceed $NCOLS columns"
exit -1

fi

echo "};"

echo "const static char ${VAR}_nb_cols = $(expr $NCOLS - 1);"
echo "const static char ${VAR}_nb_rows = $NROWS;"

echo "#endif // _DEF_$VAR"
