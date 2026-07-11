#!/bin/bash
#
# Generate packets.inc and unvalid_packets.inc from packets.txt / unvalid_packets.txt
#
# Original author: Didier DONSEZ
# Fixed & extended: valid C output (typedef, arrays, sizeof-based list),
# portable (no xxd needed), blank lines and #comments skipped,
# unvalid_packets.inc generation implemented.
#
# Usage: ./generate_inc.sh
#
set -e

NOW=$(date)
GEN_BY="${USER:-unknown}"

# ----------------------------------------------------------------------------
# gen_list <input.txt> <output.inc> <array_prefix> <list_name>
# ----------------------------------------------------------------------------
gen_list () {
	local in="$1" out="$2" prefix="$3" list="$4"
	local n=0

	{
	echo "/* Generated at $NOW by $GEN_BY from $in by generate_inc.sh -- DO NOT EDIT */"
	echo ""
	echo "#ifndef MESHCORE_PACKET_LIST_T_DEFINED"
	echo "#define MESHCORE_PACKET_LIST_T_DEFINED"
	echo "typedef struct packet_list { unsigned int len; const unsigned char *packet; } packet_list_t;"
	echo "#endif"
	echo ""

	if [ -f "$in" ]; then
		while read -r p; do
			# strip Windows carriage return if present (CRLF files)
			p="${p%$'\r'}"
			# skip blank lines and comments
			case "$p" in ""|\#*) continue ;; esac
			echo "// $p"
			echo "static const unsigned char ${prefix}_${n}[] = {"
			echo "$p" | awk '{
				s = $0; gsub(/\r/, "", s);
				line = ""; c = 0;
				for (i = 1; i <= length(s); i += 2) {
					line = line "0x" tolower(substr(s, i, 2)) ", ";
					c++;
					if (c == 16) { printf "  %s\n", line; line = ""; c = 0; }
				}
				if (line != "") printf "  %s\n", line;
			}'
			echo "};"
			echo ""
			n=$((n + 1))
		done < "$in"
	fi

	echo "static const packet_list_t ${list}[] = {"
	if [ "$n" -eq 0 ]; then
		echo "  { 0u, (const unsigned char *) 0 },  /* placeholder : empty list */"
	else
		local i=0
		while [ "$i" -lt "$n" ]; do
			echo "  { (unsigned int) sizeof(${prefix}_${i}), ${prefix}_${i} },"
			i=$((i + 1))
		done
	fi
	echo "};"
	echo ""
	echo "static const unsigned int ${list}_len = ${n}u;"
	} > "$out"

	echo "Generated $out : $n packet(s)"
}

# ----------------------------------------------------------------------------

echo "Generating packets.inc from packets.txt"
gen_list packets.txt packets.inc packet packet_list

echo "Generating unvalid_packets.inc from unvalid_packets.txt"
gen_list unvalid_packets.txt unvalid_packets.inc unvalid_packet unvalid_packet_list

# TODO Generating channels.inc for populating known channels (channels.csv)
