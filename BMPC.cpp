/*
 * re creating the bitmap logic simulator, original simulator:
 * https://realhet.wordpress.com/2015/09/02/bitmap-logic-simulator/
 * he wrote it in lua (i think) i'm rewriting it in c++ using SFML
 * i don't have (and don't know where to get) his source code
 * so i think it will be a chalanging task
 * here it goes
 * */

#include <iostream>
#include <vector>
#include <chrono>

#include <SFML/Graphics.hpp>

// Constants ---------------------------

// screen size
const int width = 800;
const int height = 600;

// Scale display by zoom_factor amount
float zoom_factor = 1.5;

// End Of Constants --------------------

sf::Texture circuit_texture;
sf::Sprite circuit_sprite;
sf::Vector2u sprite_size;

using namespace std;

int main()
{
	sf::RenderWindow window(sf::VideoMode(width, height), "Bitmap Logic Simulator Clone", sf::Style::Titlebar | sf::Style::Close);
	window.setFramerateLimit(30);
	
	sf::Image raw_circuit;
	if (!raw_circuit.loadFromFile("8bit_cpu.bmp")){
		cout << "failed to load image" << endl;
	}else{
		cout << "image loaded yay" << endl;
		circuit_texture.loadFromImage(raw_circuit);
		circuit_sprite.setTexture(circuit_texture);
		sprite_size = circuit_texture.getSize();
		circuit_sprite.setOrigin(sprite_size.x/2, sprite_size.y/2);
		circuit_sprite.setPosition(width/2, height/2);
	}
 
	bool drag = false;
	int dragX,dragY;
	bool pressed=false;
	
//	for (int y=0; y<sprite_size.y; y++){
//		for (int x=0; x<sprite_size.x; x++){
//			sf::Color c = raw_circuit.getPixel(x,y);
//			if (c==sf::Color::White){
//				c.r = 255;
//			}
//			raw_circuit.setPixel(x,y,c);
//		}	
//	}
	while (window.isOpen()){
		
		circuit_texture.loadFromImage(raw_circuit);
		circuit_sprite.setTexture(circuit_texture);
		
		sf::Event event;
		while (window.pollEvent(event)){
			if (event.type == sf::Event::Closed){
				window.close();
			}
			
			// zooming in and out
			if (event.type == sf::Event::MouseWheelScrolled){
				float d = circuit_sprite.getScale().x;
				if (event.mouseWheelScroll.delta>0){
					d *= zoom_factor;
				}else{
					d /= zoom_factor;
				}
				
				// I spend 4+ hours on this code, just to make the UI more user frendly, i hate algorithms especially when they dont work
				float scale2 = circuit_sprite.getScale().x;
				circuit_sprite.setScale(sf::Vector2f(d,d));
				float scale = circuit_sprite.getScale().x;
				
				float ax = sf::Mouse::getPosition(window).x;
				float ay = sf::Mouse::getPosition(window).y;
				float cx = circuit_sprite.getPosition().x - circuit_sprite.getOrigin().x*scale;
				float cy = circuit_sprite.getPosition().y - circuit_sprite.getOrigin().y*scale;
				float bx = ax-cx;  float by = ay-cy;
				float rx = bx/scale;  float ry = by/scale;
				
				float cx2 = circuit_sprite.getPosition().x - circuit_sprite.getOrigin().x*scale2;
				float cy2 = circuit_sprite.getPosition().y - circuit_sprite.getOrigin().y*scale2;
				float bx2 = ax-cx2;  float by2 = ay-cy2;
				float rx2 = bx2/scale2;  float ry2 = by2/scale2;
				
				if (event.mouseWheelScroll.delta>0){
					circuit_sprite.setOrigin(
						(rx+circuit_sprite.getOrigin().x)/2,
						(ry+circuit_sprite.getOrigin().y)/2
					);
				}else{
					circuit_sprite.setOrigin(
						(3*circuit_sprite.getOrigin().x - rx2)/2,
						(3*circuit_sprite.getOrigin().y - ry2)/2
					);
				}
			}
			
			if (event.type == sf::Event::MouseButtonPressed){
				if (event.mouseButton.button==2){
					if (!drag){
						dragX = circuit_sprite.getPosition().x - sf::Mouse::getPosition().x;
						dragY = circuit_sprite.getPosition().y - sf::Mouse::getPosition().y;
					}
					drag = true;
				}
				if (event.mouseButton.button==0){
					pressed=true;
				}
			}
			if (event.type == sf::Event::MouseButtonReleased){
				if (event.mouseButton.button==2){
					drag = false;
					// I just wasted 3 hours creating this code, although i wont understand it tommorow
					auto origin = circuit_sprite.getOrigin();
					float ox = origin.x; float oy = origin.y;
					float ax = width/2 - circuit_sprite.getPosition().x;
					float ay = height/2 - circuit_sprite.getPosition().y;
					float bx = ox*circuit_sprite.getScale().x;
					float by = oy*circuit_sprite.getScale().y;
					float cx = ax+bx;
					float cy = ay+by;
					circuit_sprite.setOrigin(cx/bx*ox, cy/by*oy);
					circuit_sprite.setPosition(width/2, height/2);
				}
				if (event.mouseButton.button==0){
					pressed=false;
				}
			}
			if (drag){
				circuit_sprite.setPosition(
					sf::Mouse::getPosition().x+dragX,
					sf::Mouse::getPosition().y+dragY
				);
			}
			if (pressed){
				float scale = circuit_sprite.getScale().x;
				float ax = sf::Mouse::getPosition(window).x;
				float ay = sf::Mouse::getPosition(window).y;
				float cx = circuit_sprite.getPosition().x - circuit_sprite.getOrigin().x*scale;
				float cy = circuit_sprite.getPosition().y - circuit_sprite.getOrigin().y*scale;
				float bx = ax-cx;  float by = ay-cy;
				float rx = bx/scale;  float ry = by/scale;
				
				raw_circuit.setPixel(rx,ry,sf::Color::Red);
			}
		}

		
		window.clear();
		window.draw(circuit_sprite);
		window.display();
	}
	
	
	
	return 0;
}


