/*
 * re creating the bitmap logic simulator, original simulator:
 * https://realhet.wordpress.com/2015/09/02/bitmap-logic-simulator/
 * he wrote it in lua (i think) i'm rewriting it in c++ using SFML
 * i don't have (and don't know where to get) his source code
 * so i think it will be a chalanging task
 * here it goes
 * */

#include <iostream>
#include <SFML/Graphics.hpp>

using namespace std;

int main()
{
	sf::RenderWindow window(sf::VideoMode(500, 500), "Bitmap Logic Simulator Clone");
	window.setFramerateLimit(5);
	
	sf::Texture circuit;
	
	
	if (!circuit.loadFromFile("Circuit.bmp")){
		cout << "failed to load image" << endl;
	}else{
		cout << "image loaded yay" << endl;
	}
	
	while (window.isOpen()){
		sf::Event event;
		while (window.pollEvent(event)){
			if (event.type == sf::Event::Closed)
				window.close();
		}
		
	}
	window.clear();
	window.display();
	
	return 0;
}