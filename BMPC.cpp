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
  * and two lists of going to and coming from)[done]
  * Two crosses dont work together(fix it) [done]
  * Problem with non-square images
  * cant open large images, i get an error
  * clicking outside picture crashes it
  * The wire finding and fixing function is soooo slloooooooooowwww [fixed to some extend, not great though]
  * [actually adding optimization flag made it superfast]
  * wires touching boundary crashes it
  * crashes when a wire is at boundary
  * the wires filcker due to all off them being powered at the same time
  * */

#include <iostream>
#include <cstdio>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <cstdlib>

#include <SFML/Graphics.hpp>

using namespace std;

// Optimizing the speed
int CW, CH;
inline int to_one(int x, int y){
	return y*CH+x;
}
inline vector<int> to_two(int i){
	return {i%CW, i/CW};
}
inline bool found_it(vector<int> l, int i){
	return std::find(l.begin(), l.end(), i) != l.end();
}
// Structures --------------------------
struct wire{
	vector<int> parts;
	bool in = false;
	bool out = false;
	bool find(int i){
		return found_it(parts, i);
	}
};

struct gate{
	//vector<int> parts;
	int input_wire_index;
	int output_wire_index;
};
// End Of Structures -------------------

// Constants ---------------------------

// screen size
const int width = 800;
const int height = 600;
// simulation stuff
int s_fps = 10;		// simulation per frame
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

float tw=0, tc=0, tg=0;
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
					w.parts.push_back(to_one(x,y));
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
			// Maybe compare one number instead of two (x,y) to speed it up [im doing this right now]
			double dist = sqrt( pow(to_two(wires[i].parts[0])[0]-x,2) + pow(to_two(wires[i].parts[0])[1]-y,2) );
			int d = int(dist)-2-2*wires[i].parts.size();
			if (d>0){
				continue;
			}
			
			auto this_wire = wires[i];
			if (this_wire.find(to_one(x-1,y))){
				left = i;
				found++;
			}
			if (this_wire.find(to_one(x+1,y))){
				right = i;
				found++;
			}
			if (this_wire.find(to_one(x,y-1))){
				up = i;
				found++;
			}
			if (this_wire.find(to_one(x,y+1))){
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
	
	for (int i=0; i<wires.size(); i++){
		// Maybe compare one number instead of two (x,y) to speed it up
		double dist = sqrt( pow(to_two(wires[i].parts[0])[0]-x,2) + pow(to_two(wires[i].parts[0])[1]-y,2) );
		int d = int(dist)-2-2*wires[i].parts.size();
		if (d>0){
			continue;
		}
		if (dir==1){
			if (wires[i].find(to_one(x,y-1))){
				this_gate.output_wire_index = i;
			}if (wires[i].find(to_one(x,y+1))){
				this_gate.input_wire_index = i;
			}
		}else if (dir==2){
			if (wires[i].find(to_one(x+1,y))){
				this_gate.output_wire_index = i;
			}if (wires[i].find(to_one(x-1,y))){
				this_gate.input_wire_index = i;
			}
		}else if (dir==3){
			if (wires[i].find(to_one(x,y+1))){
				this_gate.output_wire_index = i;
			}if (wires[i].find(to_one(x,y-1))){
				this_gate.input_wire_index = i;
			}
		}else if (dir==4){
			if (wires[i].find(to_one(x-1,y))){
				this_gate.output_wire_index = i;
			}if (wires[i].find(to_one(x+1,y))){
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

vector<int> manualy_powered;
int powered_by_click=-1;

int main()
{
	time_t t;
	srand((unsigned) time(&t));
	
	sf::RenderWindow window(sf::VideoMode(width, height), "Bitmap Logic Simulator Clone", sf::Style::Titlebar | sf::Style::Close);
	window.setFramerateLimit(30);

	sf::Image raw_circuit;
	// 8bit_cpu.bmp
	// Circuit3.bmp
	if (!raw_circuit.loadFromFile("8bit_cpu.bmp")){
		cout << "Failed to load image" << endl;
	}else{
		cout << "Image loaded yay" << endl;
		circuit_texture.loadFromImage(raw_circuit);
		circuit_sprite.setTexture(circuit_texture);
		sprite_size = circuit_texture.getSize();
		circuit_sprite.setOrigin(sprite_size.x/2, sprite_size.y/2);
		circuit_sprite.setPosition(width/2, height/2);
		
		for (int y=0; y<sprite_size.y; y++){
			vector<int> this_y;
			for (int x=0; x<sprite_size.x; x++){
				// i still don't know why it should be (y,x) instead of (x,y)
				auto c = raw_circuit.getPixel(y,x);
				if (c.r>223 or c.g>223 or c.b>223){
					this_y.push_back(1);
				}else{
					this_y.push_back(0);
				}
				
			}
			circuit_details.push_back(this_y);
		}
		CW = circuit_details.size();
		CH = circuit_details[0].size();
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
	
	
	
	// fixing the flickering window, adds random state to every wire[doesnt work as good as i imagened]
	for (int i=0; i<wires.size(); i++)
		wires[i].in = rand()%2;

	
	cout << "Done" << endl;
	
	
	bool drag = false;
	int dragX,dragY;
	
	
	while (window.isOpen()){
		// Do the game cycle
		for (int loop=0; loop<s_fps; loop++){
			for (int i=0; i<manualy_powered.size(); i++){
				wires[manualy_powered[i]].in = true;
			}
			if(powered_by_click != -1){
				wires[powered_by_click].in = true;
			}
			//update wires
				for (int i=0; i<wires.size(); i++){
					wires[i].out = wires[i].in;
					wires[i].in = false;
					// Change color of wire parts (todo: dont change if not changed)
					for (int j=0; j<wires[i].parts.size(); j++){
						auto xy = to_two(wires[i].parts[j]);
						sf::Color color(128,128,128);
						if (wires[i].out) color = {255,255,255};
						raw_circuit.setPixel(xy[0],xy[1], color);
					}
				}
			//update gates
				for (int i=0; i<gates.size(); i++){
					if (wires[gates[i].output_wire_index].in==false)
						wires[gates[i].output_wire_index].in = not( wires[gates[i].input_wire_index].out );
				}
		
		circuit_texture.loadFromImage(raw_circuit);
		circuit_sprite.setTexture(circuit_texture);
		}
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
			
			float scale = circuit_sprite.getScale().x;
			float ax = sf::Mouse::getPosition(window).x;
			float ay = sf::Mouse::getPosition(window).y;
			float cx = circuit_sprite.getPosition().x - circuit_sprite.getOrigin().x*scale;
			float cy = circuit_sprite.getPosition().y - circuit_sprite.getOrigin().y*scale;
			float bx = ax-cx;  float by = ay-cy;
			int rx = bx/scale;  int ry = by/scale;
			
			if (event.type == sf::Event::MouseButtonPressed){
				if (event.mouseButton.button==2){
					if (!drag){
						dragX = circuit_sprite.getPosition().x - sf::Mouse::getPosition().x;
						dragY = circuit_sprite.getPosition().y - sf::Mouse::getPosition().y;
					}
					drag = true;
				}
				if (event.mouseButton.button==0){
					for (int i=0; i<wires.size(); i++){
						if (wires[i].find(to_one(rx,ry))){
							if ( not found_it(manualy_powered, i) ){
								powered_by_click = i;
							}else{
								for (int f=0; f<manualy_powered.size(); f++){
									if ( manualy_powered[f]==i ){
										powered_by_click = -1;
										break;
									}
								}
							}
							break;
						}
					}
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
					powered_by_click = -1;
				}else if (event.mouseButton.button==1){
					
					// right click handling -------------------------

				
					// for some reason (auto i:wires) doesnt work :/
					for (int i=0; i<wires.size(); i++){
						if (wires[i].find(to_one(rx,ry))){
							//wires[i].in = not(wires[i].in);
			
							if ( not found_it(manualy_powered, i) ){
								manualy_powered.push_back(i);
							}else{
								for (int f=0; f<manualy_powered.size(); f++){
									if ( manualy_powered[f]==i ){
										manualy_powered.erase(manualy_powered.begin()+f);
										break;
									}
								}
							}
							break;
						}
					}
					// end of right click handing -------------------
				}
			}
			if (drag){
				circuit_sprite.setPosition(
					sf::Mouse::getPosition().x+dragX,
					sf::Mouse::getPosition().y+dragY
				);
			}
		}
		
		window.clear();
		window.draw(circuit_sprite);
		window.display();
	}
	
	
	
	return 0;
}

