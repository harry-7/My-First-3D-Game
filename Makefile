all: my3Dgame

sample3D: Sample_GL3_2D.cpp glad.c
	g++ -o sample3D Sample_GL3_2D.cpp glad.c -lGL -lglfw -ldl -lftgl -lSOIL -I /usr/local/include/freetype2 -L/usr/local/lib
clean:
	rm my3Dgame
