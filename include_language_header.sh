lang=$(locale | grep LANGUAGE | cut -d= -f2 | cut -d_ -f1)
if test -f language_header.h; then
	rm language_header.h
fi

echo "#include \"languages/$lang.h\"" >> language_header.h

if test ! -f languages/$lang.h; then
	echo "// To get a translated version of the game replace/translate the words between braces in the same order" >> languages/$lang.h
	echo "// and then compile 2048.c (execute \$gcc -o 2048 2048.c)" >> languages/$lang.h
	echo "const char * LOCALE_WORDS[] = {\"or\",\"TERMINATED\",\"GAME OVER\",\"QUIT?\",\"RESET?\"};" >> languages/$lang.h
	echo "languages/$lang.h file was created. Edit it and then compile 2048.c (execute \$gcc -o 2048 2048.c) to get a traslated version of the game"
fi



