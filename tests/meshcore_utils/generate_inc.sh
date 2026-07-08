# Author: Didier DONSEZ

# For generation time
NOW=$(date)

// ----------

echo "TODO Generating channels.inc for populating known channels"

{
cat channels.csv
}

// ----------

echo "Generating packets.inc for testing packets (including advert)"

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
echo "{ packet_${LINECNT}_len, packet_${LINECNT}}," >> .packets.inc
LINECNT=`expr $LINECNT + 1`
#echo "},"
done < packets.txt
} > packets.inc


echo "};" >> .packets.inc
echo "const unsigned int packet_list_len = ${LINECNT};" >> .packets.inc

cat .packets.inc >> packets.inc

rm .packets.inc
rm .packet.tmp

// ----------

echo TODO Generating unvalid_packets.inc for testing unvalid packets
