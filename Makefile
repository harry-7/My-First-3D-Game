all: sample3D

sample3D: Sample_GL3_2D.cpp glad.c
	g++ -o sample3D Sample_GL3_2D.cpp glad.c -lGL -lglfw -ldl -lftgl -lSOIL -O2 -lmpg123 -lao -lpthread -I /usr/local/include/freetype2 -L"/usr/local/lib" ./libIrrKlang.so ./ikpMP3.so -pthread -I ./include/
clean:
	rm sample3D
