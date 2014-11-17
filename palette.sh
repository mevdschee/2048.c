#!/bin/bash
echo "[8 colors] => ^[[4#m"
for color in {0..7} ; do
	#Display the color
	number=$(printf " %-5d " "$color")
	echo -en "\e[4${color}m${number}\e[0m"
	#Display 8 colors per line
	if [ $((($color + 1) % 8)) == 0 ] ; then
		echo #New line
	fi
done
echo "[16 colors] => ^[[48;5;#m"
for color in {0..15} ; do
	#Display the color
	number=$(printf " %-5d " "$color")
	echo -en "\e[48;5;${color}m${number}\e[0m"
	#Display 8 colors per line
	if [ $((($color + 1) % 8)) == 0 ] ; then
		echo #New line
	fi
done
echo "[216 colors] => ^[[48;5;#m"
for color in {16..231} ; do
	#Display the color
	number=$(printf " %-7d " "$color")
	echo -en "\e[48;5;${color}m${number}\e[0m"
	#Display 6 colors per line
	if [ $((($color - 15) % 6)) == 0 ] ; then
		echo #New line
	fi
done
echo "[24 shades of gray] => ^[[48;5;#m"
for color in {232..255} ; do
	#Display the color
	number=$(printf " %-3d " "$color")
	echo -en "\e[48;5;${color}m${number}\e[0m"
	#Display 12 colors per line
	if [ $((($color - 231) % 12)) == 0 ] ; then
		echo #New line
	fi
done
