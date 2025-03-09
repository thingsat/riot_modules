#!/bin/bash

NCOLS=$1

VAR=$2

FILENAME=$3

echo "// Header for $VAR"
echo "// Generated from $FILENAME"

echo "#ifndef _DEF_$VAR"
echo "#define _DEF_$VAR"

NROWS=0

echo "#define ${VAR}_nb_cols $(expr $NCOLS - 1)"

echo "const char* $VAR[][${VAR}_nb_cols] = {"

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

elif [ $NCOLS -eq 5 ] 
then

while IFS=';' read -r col1 col2 col3 col4 col5
do
    echo  '{' '/*' $col1 '*/' '"'$col2'",' '"'$col3'",' '"'$col4'",' '"'$col5'",' '},'
    NROWS=$(expr $NROWS + 1)
done < $FILENAME

elif [ $NCOLS -eq 6 ] 
then

while IFS=';' read -r col1 col2 col3 col4 col5 col6
do
    echo  '{' '/*' $col1 '*/' '"'$col2'",' '"'$col3'",' '"'$col4'",' '"'$col5'",' '"'$col6'",' '},'
    NROWS=$(expr $NROWS + 1)
done < $FILENAME

else


echo "ERROR: can not proceed $NCOLS columns"
exit -1

fi

echo "};"

echo "#define ${VAR}_nb_rows  $NROWS"

echo "#endif // _DEF_$VAR"
