//main header
#include "Slider.h"

//for isnan()
#include <math.h>

inline double is_in_slider(double value) {
	if (value < -0.5) {
		value = -0.5;
	}
	if (value > 0.462) {
		value = 0.462;
	}
	return value;
}
/*
<distance> <calc_area> <position> < calc > <IsInTriangle>
 -- are used for input proccesing

 -- use IsInTriangle to check for where is a point

*/
// calculate distance beetween (x1,y1) ----- (x2,y2)
inline float distance(float x1, float y1, float x2, float y2)
{
	return(sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2)));
}
// calculates the surface of a triangle with points a b c
inline float calc_area(float a, float b, float c)
{
	float S;

	S = (a + b + c) / 2.0;

	return(sqrt(S * (S - a) * (S - b) * (S - c)));
}
// modulus
inline int position(float area, float A, float B, float C)
{
	float res = area - (A + B + C);

	if (res < 0)
	{
		res *= -1;
	}

	if (res == 0 || res < 0.001)
	{
		return(1);
	}
	else
	{
		return(0);
	}
}
//calculates if a point (x,y) is in the triangle A(x1,y1) B(x2,y2) , C(x3,y3)
inline void calc(float x1, float y1, float x2, float y2
	, float x3, float y3, float x, float y, int* flag, float* area) {
	/*
	* https://www.youtube.com/watch?v=ntjM9YZP0qk
	*
	* the link for the whole IsInTriangle() function explanation

	*/
	float A, B, C, a, b, c, d, e, f;

	a = distance(x1, y1, x2, y2);
	b = distance(x2, y2, x3, y3);
	c = distance(x3, y3, x1, y1);
	*area = calc_area(a, b, c);

	d = distance(x1, y1, x, y);
	e = distance(x2, y2, x, y);
	f = distance(x3, y3, x, y);

	A = calc_area(d, a, e);
	B = calc_area(e, b, f);
	C = calc_area(f, c, d);

	*flag = position(*area, A, B, C);
}
// C++ implementation of C function calc()
inline bool isInTriangle(glm::vec4 A, glm::vec4 B, glm::vec4 C, glm::vec4 point) {
	int   flag = 0;
	float area = 0;

	calc(A.x, A.y, B.x, B.y, C.x, C.y, point.x, point.y, &flag, &area);

	if (flag) return true;
	else  return false;
}


// changes position on screen
void Slider::change_position(double x, double y) {
	posx = x;
	posy = y;
}
// slider constructor,
/*
Shader* _shader, --> shader to render
WINDOW* _windowobj,  --> window on which to render
const char* path, --> Texture path
double _posx, --- /
double _posy,   --   /   screen data
double _scale    --  /
*/
Slider::Slider(Shader* _shader, WINDOW* _windowobj, const char* path, double _posx, double _posy, double _scale) {
	shader = _shader;
	windowobj = _windowobj;
	posx = _posx;
	posy = _posy;
	scale = _scale;
	txcounter = new Text(*windowobj, "resources/fonts/Antonio-Light.ttf");



	dposy = _posy + 1;
	dposx = _posx - 237;
	int width, height;
	// NDC conversion
	glfwGetWindowSize(windowobj->window, &width, &height);
	dposx = dposx / width * 2 - 1;
	dposy = dposy / height * 2 - 1;

	dscale = 0.03f;
	TEXTURE* temp = new TEXTURE(std::string(path));
	ID = temp->ID;
}
// returns dot postion based on song lengh
double Slider::return_pos(double song_lenght) {
	double pos = (this->setdotpos + 0.5) * song_lenght;
	return pos;
}
// sets dot postion at <seconds> position , and <song_lenght> sample lengh
void Slider::set_pos(double seconds, double song_lenght) {
	crtsongtime = seconds;
	fullsongtime = song_lenght;
	this->setdotpos = (seconds / song_lenght) - 0.5;
	if (isnan(this->setdotpos)) {
		this->setdotpos = -1; // if is nan this->setdotpos 
		//will be reduced to -0.5 by is_in_slider , any value < -0.5 is good...
	}
	this->setdotpos = is_in_slider(this->setdotpos);
}
// creates a SLIDER using class variables
void Slider::create_panel(Shader* texture_shader, WINDOW* windowobj, VertexArrayObject* VAO,
	double posx, double posy, double scale) {
	texture_shader->use();
	//rendering object1
	this->model = texture_shader->transform(windowobj->window, posx, posy, scale); // 200 -xpos , 100 -ypos , 0.2 -scale;
	glBindTexture(GL_TEXTURE_2D, ID);// pick texture
	VAO->use(); // pick vao
	DRAW(6); // draw 6 vertices
	texture_shader->transform(windowobj->window, posx, posy, scale);
	//dot drawing
	texture_shader->notnormal_transform(windowobj->window, this->setdotpos, dposy, 0.03f);
	dotTexture.use();
	DOT.use();
	DRAW(6);
	glm::vec3 white_clr = glm::vec3(1.0f, 1.0f, 1.0f);
	std::string sngtime = std::to_string(fullsongtime / 60);
	std::string curtime = std::to_string(crtsongtime / 60);// minute transformation
	//txcounter->drawText(curtime.erase(curtime.length() - 4 , curtime.length()), posx - 267, posy - 2, 0.25f, white_clr);
	//txcounter->drawText(sngtime.erase(sngtime.length() - 4, sngtime.length()), posx + 250, posy - 2, 0.25f, white_clr);
}
// accepts  user input
bool Slider::accept_input(glm::vec4 point) {
	std::vector<glm::vec3> raw = VAO.vec4_vector;
	glm::vec4 A = this->model * glm::vec4(raw[0], 1.0f);
	glm::vec4 B = this->model * glm::vec4(raw[1], 1.0f);
	glm::vec4 C = this->model * glm::vec4(raw[2], 1.0f);
	glm::vec4 A1 = this->model * glm::vec4(raw[3], 1.0f);
	glm::vec4 B1 = this->model * glm::vec4(raw[4], 1.0f);
	glm::vec4 C1 = this->model * glm::vec4(raw[5], 1.0f);

	if (isInTriangle(A, B, C, point) or isInTriangle(A1, B1, C1, point)) {
		double value = is_in_slider(point.x);
		if (dposx != value) {
			this->setdotpos = value;
		}
		return true;
	}
	return false;
}
// create_panel invoker
void Slider::render() {
	
	create_panel(shader, windowobj, &VAO, posx, posy, scale);
}
// not usable --------------------------------------------------------------------------
void Slider::setText(Text* font, std::string text, float scale, float R, float G, float B) {
	font->drawText(text, posx, posy, scale, glm::vec3(R, G, B));
}