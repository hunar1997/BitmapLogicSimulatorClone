/*
 * re creating the bitmap logic simulator, original simulator:
 * https://realhet.wordpress.com/2015/09/02/bitmap-logic-simulator/
 * he wrote it in lua (i think) i'm rewriting it in c++ using SFML
 * i don't have (and don't know where to get) his source code
 * so i think it will be a chalanging task
 * here it goes
 * 
 * [one day passes] i actually found the code BUT it contains 12907 lines of code
 * (of a language that i dont know what it is), there is no way of reading and understanding it
 * i already completed all the displays and its 176 lines of c++ with comments included, i'll invent
 * my own logic instead :D
 * */
 
 /* Issues
  * the wire-gate-cross index is stupid, make a better version [done: changed the code]
  * decide how to store the data, (ex. wires should hold their childs to be updated later
  * and two lists of going to and coming from)
  * Two crosses dont work together(fix it) [done]
  * Problem with non-square images
  * cant open large images, i get an error
  * clicking outside picture crashes it
  * The wire finding and fixing function is soooo slloooooooooowwww [fixed to some extend, not great though]
  * wires touching boundary crashes it
  * */

#include <iostream>
#include <cstdio>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>

#include <SFML/Graphics.hpp>

using namespace std;


// Structures --------------------------
struct wire{
	vector<vector<int>> parts;
	bool in = false;
	bool out = false;
	bool find(int x, int y){
		return std::find(parts.begin(), parts.end(), vector<int>{x,y}) != parts.end();
	}
};

struct gate{
	vector<vector<int>> parts;
	int input_wire_index;
	int output_wire_index;
};
// End Of Structures -------------------

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

vector<vector<int>> circuit_details;

vector<wire> wires;
vector<gate> gates;

wire empty_wire;

// Functions
void find_wires(int x, int y, bool global_call=true, wire &w=empty_wire){
	if (global_call){
		wire this_wire;
		find_wires(x,y, false, this_wire);
		wires.push_back(this_wire);
	}else{
		if (x>0 and x<circuit_details[0].size()){
			if (y>0 and y<circuit_details.size()){
				if (circuit_details[x][y]==1){
					w.parts.push_back({x,y});
					circuit_details[x][y]=2;
					// search neighbours
					find_wires(x+1, y  , false, w);
					find_wires(x-1, y  , false, w);
					find_wires(x  , y+1, false, w);
					find_wires(x  , y-1, false, w);
				}
			}
		}
	}
}
void find_wire_cross(int x, int y){
	auto &c = circuit_details;
	int r1 = c[x-1][y-1]*100 + c[x][y-1]*10 + c[x+1][y-1];
	int r2 = c[x-1][y  ]*100 + c[x][y  ]*10 + c[x+1][y  ];
	int r3 = c[x-1][y+1]*100 + c[x][y+1]*10 + c[x+1][y+1];
	if ( r1==20 and r2==202 and r3==20 ){
		// fixing the wireings
		int left,right,up,down;
		int found = 0;
		for (int i=0; i<wires.size(); i++){
			// Maybe compare one number instead of two (x,y) to speed it up
			double dist = sqrt( pow(wires[i].parts[0][0]-x,2) + pow(wires[i].parts[0][1]-y,2) );
			int d = int(dist)-2-2*wires[i].parts.size();
			if (d>0){
				continue;
			}
			
			auto this_wire = wires[i];
			if (this_wire.find(x-1,y)){
				left = i;
				found++;
			}
			if (this_wire.find(x+1,y)){
				right = i;
				found++;
			}
			if (this_wire.find(x,y-1)){
				up = i;
				found++;
			}
			if (this_wire.find(x,y+1)){
				down = i;
				found++;
			}
			if (found==4) break;
		}
		if (found==4){
			if (left != right)
				wires[left].parts.insert(wires[left].parts.end(), wires[right].parts.begin(), wires[right].parts.end());
			if (up != down)
				wires[up].parts.insert(wires[up].parts.end(), wires[down].parts.begin(), wires[down].parts.end());
			if (left != right and up != down){
				if (down>right){
					wires.erase(wires.begin()+down);
					wires.erase(wires.begin()+right);
				}else{
					wires.erase(wires.begin()+right);
					wires.erase(wires.begin()+down);
				}
			}else if(left != right){
				wires.erase(wires.begin()+right);
			}else if(up != down){
				wires.erase(wires.begin()+down);
			}
			// else all neighbour wires are the same so nothing to do
		}else{
			cout << "ERROR: found a wire cross without finding the four neighbouring wires " << found << endl;
			cout << "Error at position: " << x << " " << y << endl;
		}
	}
}

void make_gate(int x, int y, int dir){
	gate this_gate;
	
	// add the gate parts
	for (int j=-1; j<=1; j++){
		for (int i=-1; i<=1; i++){
			if ( circuit_details[x+i][y+j] == 2 ){
				this_gate.parts.push_back({x+i, y+j});
			}
		}
	}
	
	for (int i=0; i<wires.size(); i++){
		// Maybe compare one number instead of two (x,y) to speed it up
		double dist = sqrt( pow(wires[i].parts[0][0]-x,2) + pow(wires[i].parts[0][1]-y,2) );
		int d = int(dist)-2-2*wires[i].parts.size();
		if (d>0){
			continue;
		}
		if (dir==1){
			if (wires[i].find(x,y-1)){
				this_gate.output_wire_index = i;
			}if (wires[i].find(x,y+1)){
				this_gate.input_wire_index = i;
			}
		}else if (dir==2){
			if (wires[i].find(x+1,y)){
				this_gate.output_wire_index = i;
			}if (wires[i].find(x-1,y)){
				this_gate.input_wire_index = i;
			}
		}else if (dir==3){
			if (wires[i].find(x,y+1)){
				this_gate.output_wire_index = i;
			}if (wires[i].find(x,y-1)){
				this_gate.input_wire_index = i;
			}
		}else if (dir==4){
			if (wires[i].find(x-1,y)){
				this_gate.output_wire_index = i;
			}if (wires[i].find(x+1,y)){
				this_gate.input_wire_index = i;
			}
		}
	}
	gates.push_back(this_gate);
}

void find_gates(int x, int y){
	auto &c = circuit_details;
	int r1 = c[x-1][y-1]*100 + c[x][y-1]*10 + c[x+1][y-1];
	int r2 = c[x-1][y  ]*100 + c[x][y  ]*10 + c[x+1][y  ];
	int r3 = c[x-1][y+1]*100 + c[x][y+1]*10 + c[x+1][y+1];
	if ( (r1==220) and (r2==202) and (r3==220) ){
		make_gate(x,y, 2);
	}else if ( r1==22 and r2==202 and r3==22 ){
		make_gate(x,y, 4);
	}else if ( r1==20 and r2==202 and r3==222 ){
		make_gate(x,y, 1);
	}else if ( r1==222 and r2==202 and r3==20 ){
		make_gate(x,y, 3);
	}
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(width, height), "Bitmap Logic Simulator Clone", sf::Style::Titlebar | sf::Style::Close);
	window.setFramerateLimit(30);
	
	sf::Image raw_circuit;
	// 8bit_cpu.bmp
	// Circuit3.bmp
	if (!raw_circuit.loadFromFile("Circuit3.bmp")){
		cout << "failed to load image" << endl;
	}else{
		cout << "image loaded yay" << endl;
		circuit_texture.loadFromImage(raw_circuit);
		circuit_sprite.setTexture(circuit_texture);
		sprite_size = circuit_texture.getSize();
		circuit_sprite.setOrigin(sprite_size.x/2, sprite_size.y/2);
		circuit_sprite.setPosition(width/2, height/2);
		
		for (int y=0; y<sprite_size.y; y++){
			vector<int> this_y;
			for (int x=0; x<sprite_size.x; x++){
				// i still don't know why it should be (y,x) instead of (x,y)
				this_y.push_back( (raw_circuit.getPixel(y,x)==sf::Color::White)?1:0 );
			}
			circuit_details.push_back(this_y);
		}
	}
	
	
	// find wires
	for (int y=0; y<sprite_size.y; y++){
		for (int x=0; x<sprite_size.x; x++){
			if (circuit_details[x][y]==1){
				find_wires(x,y);
			}
		}
	}
	cout << "Found " << wires.size() << " wires before calculating crosses\n";
	
	// find wire crosses and connect them
	cout << "finding wire crosses:\n";
	for (int y=1; y<sprite_size.y-1; y++){
		cout << "\b\b\b" << int(100*y/sprite_size.y) << "%"; fflush(stdout);
		for (int x=1; x<sprite_size.x-1; x++){
			if (circuit_details[x][y]==0){
				find_wire_cross(x,y);
			}
		}
	}
	cout << "\nFound " << wires.size() << " wires after calculating crosses\n";
	
	
	// find gates and attach the wires to them
	cout << "finding gates:\n";
	for (int y=1; y<sprite_size.y-1; y++){
		cout << "\b\b\b" << int(100*y/sprite_size.y) << "%"; fflush(stdout);
		for (int x=1; x<sprite_size.x-1; x++){
			if (circuit_details[x][y]==0){
				find_gates(x,y);
			}
		}
	}
	cout << "\nFound " << gates.size() << " gates\n";
	for (auto i:gates){
		for (auto j:i.parts){
			circuit_details[j[0]][j[1]] = 3;
		}
	}
	
	cout << "done" << endl;
	// temp
	for (int y=0; y<sprite_size.y; y++){
		for (int x=0; x<sprite_size.x; x++){
			if (circuit_details[x][y]==1){
				raw_circuit.setPixel(x,y, sf::Color::Blue);
			}
			if (circuit_details[x][y]==2){
				raw_circuit.setPixel(x,y, sf::Color::Green);
			}
			if (circuit_details[x][y]==3){
				raw_circuit.setPixel(x,y, sf::Color::Cyan);
			}
		}
	}
	
	bool drag = false;
	int dragX,dragY;
	bool pressed=false;
	
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


