#!/bin/bash

CSV2H=./cvs2h.sh

generate_values(){

SRCDIR=data/values
TRGDIR=build/include/values

NAME=camf_18_3_1; $CSV2H 2 $NAME $SRCDIR/18.3.1.csv > $TRGDIR/$NAME.h
NAME=camf_18_3_2; $CSV2H 2 $NAME $SRCDIR/18.3.2.csv > $TRGDIR/$NAME.h
NAME=camf_18_3_3; $CSV2H 2 $NAME $SRCDIR/18.3.3.csv > $TRGDIR/$NAME.h

}

generate_i18n(){

LANG=$1
SRCDIR=data/i18n/$LANG
TRGDIR=build/include/i18n/$LANG

NAME=camf_11_a_$LANG; $CSV2H 3 $NAME $SRCDIR/11.a.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_1_$LANG; $CSV2H 3 $NAME $SRCDIR/18.4.35.1.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_10_$LANG; $CSV2H 4 $NAME $SRCDIR/18.4.35.10.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_12_$LANG; $CSV2H 3 $NAME $SRCDIR/18.4.35.12.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_13_$LANG; $CSV2H 3 $NAME $SRCDIR/18.4.35.13.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_19_$LANG; $CSV2H 3 $NAME $SRCDIR/18.4.35.19.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_22_$LANG; $CSV2H 2 $NAME $SRCDIR/18.4.35.22.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_24_$LANG; $CSV2H 2 $NAME $SRCDIR/18.4.35.24.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_27_$LANG; $CSV2H 2 $NAME $SRCDIR/18.4.35.27.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_28_$LANG; $CSV2H 2 $NAME $SRCDIR/18.4.35.28.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_29_$LANG; $CSV2H 2 $NAME $SRCDIR/18.4.35.29.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_31_$LANG; $CSV2H 2 $NAME $SRCDIR/18.4.35.31.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_32_$LANG; $CSV2H 3 $NAME $SRCDIR/18.4.35.32.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_34_$LANG; $CSV2H 3 $NAME $SRCDIR/18.4.35.34.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_35_$LANG; $CSV2H 2 $NAME $SRCDIR/18.4.35.35.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_36_$LANG; $CSV2H 2 $NAME $SRCDIR/18.4.35.36.csv > $TRGDIR/$NAME.h
NAME=camf_18_4_35_8_$LANG; $CSV2H 3 $NAME $SRCDIR/18.4.35.8.csv > $TRGDIR/$NAME.h
NAME=camf_a1_$LANG; $CSV2H 2 $NAME $SRCDIR/a1.csv > $TRGDIR/$NAME.h
NAME=camf_a4_$LANG; $CSV2H 4 $NAME $SRCDIR/a4.csv > $TRGDIR/$NAME.h
# codes.csv
}

mkdir -p build/include/values
generate_values

mkdir -p build/include/i18n/en
generate_i18n en

mkdir -p build/include/i18n/fr
generate_i18n fr
