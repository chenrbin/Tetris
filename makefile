default:
	g++ *.cpp -o tetris.exe *.cpp -Isfml-2.5.1/include -Lsfml-2.5.1/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio