echo Generating packets.inc for including packets

# For generation time
NOW=$(date)
{

echo "typedef struct packet_list { unsigned int len, unsigned char* packet} packet_list_t;" > .packets.inc
echo "const packet_list_t p = {" >> .packets.inc


LINECNT=0
echo "/* Generated for at $NOW by $USER */"
while read p; do
#echo '{'
echo "// $p"
echo -n "const "
echo "$p" | xxd -r -p > .packet.tmp
xxd -i -c 16 -n packet_${LINECNT} .packet.tmp
LINECNT=`expr $LINECNT + 1`
echo "{ packet_${LINECNT}_len, packet_${LINECNT}}," >> .packets.inc
#echo "},"
done < packets.txt
} > packets.inc


echo "};" >> .packets.inc

cat .packets.inc >> packets.inc

rm .packets.inc
rm .packet.tmp
