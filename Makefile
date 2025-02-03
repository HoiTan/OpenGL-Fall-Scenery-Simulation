sample:		sample.cpp
		g++ -framework OpenGL -framework GLUT sample.cpp -o sample -I. -Wno-deprecated
Project2:		Project2.cpp
		g++ -framework OpenGL -framework GLUT Project2.cpp -o Project2 -I. -Wno-deprecated
Project3:		Project3.cpp
		g++ -framework OpenGL -framework GLUT Project3.cpp -o Project3 -I. -Wno-deprecated
Project4:		Project4.cpp
		g++ -framework OpenGL -framework GLUT Project4.cpp -o Project4 -I. -Wno-deprecated

Project5:		Project5.cpp
		g++ -framework OpenGL -framework GLUT Project5.cpp -o Project5 -I. -Wno-deprecated

Project6:		Project6.cpp
		g++ -framework OpenGL -framework GLUT Project6.cpp -o Project6 -Wno-deprecated
		# g++ -framework OpenGL -framework GLUT Project6.cpp -o Project6 -I. -Wno-deprecated


FinalProject:		FinalProject.cpp
		g++ -std=c++11  -framework OpenGL -framework GLUT FinalProject.cpp TreeBody/LSystem.cpp TreeBody/Turtle.cpp -o FinalProject -Wno-deprecated
		# g++ -std=c++11 -framework OpenGL -framework GLUT FinalProject.cpp -o FinalProject -Wno-deprecated
		# g++ -framework OpenGL -framework GLUT FinalProject.cpp -o FinalProject -I. -Wno-deprecated



TransBlend:		TransBlend.cpp
		g++ -framework OpenGL -framework GLUT TransBlend.cpp -o TransBlend -I. -Wno-deprecated