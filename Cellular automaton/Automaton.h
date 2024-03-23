#pragma once
#ifndef automaton
#define automaton

#include "SFML/Graphics.hpp"

#define FIELD_W 160
#define FIELD_H 160
#define RULESIZE 512

using namespace sf;

class Automaton
{
	bool* rule;
	bool** next_state;

	Clock delay;

public:
	bool** state;

	Automaton();
	void set_conway();
	void step();
	void draw(Image*, int fps);
	void draw(Image*);
};


#endif // !automaton
