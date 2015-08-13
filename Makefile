
play_icfp2015: code/* posix/HoneyRunner/*
	clang -Icode -Iposix/HoneyRunner -I/usr/include/malloc -m32 -stdlib=libstdc++ -L/usr/lib -lstdc++ -o play_icfp2015 code/Honey*.cpp code/loader.cpp posix/HoneyRunner/*.cpp

clean:
	rm play_icfp2015
