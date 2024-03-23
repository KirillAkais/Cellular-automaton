#include "Automaton.h"
#include <iostream>


using namespace std;

Automaton::Automaton()
{
	this->rule = new bool[RULESIZE];
	for (int i = 0; i < RULESIZE; i++)
		rule[i] = rand() % 2;

	this->state = new bool*[FIELD_W];
	this->next_state = new bool*[FIELD_W];
	for (int i = 0; i < FIELD_W; i++)
	{
		this->state[i] = new bool[FIELD_H];
		this->next_state[i] = new bool[FIELD_H];

		for (int j = 0; j < FIELD_H; j++)
			this->state[i][j] = rand() % 2;
	}
}

// Sets rule to Conway's game of life
void Automaton::set_conway()
{
	int s, alive, alive_neigbors;
	for (int i = 0; i < RULESIZE; i++)
	{
		s = i;
		alive = s & 16;
		s = s & 495;
		alive_neigbors = 0;
		for (int j = 0; j < 10; j++)
		{
			alive_neigbors += s & 1;
			s = s >> 1;
		}
		if ((!alive && alive_neigbors == 3) ||
			(alive && (alive_neigbors == 2 || alive_neigbors == 3)))
			rule[i] = 1;
		else
			rule[i] = 0;
	}
}

// Iterates the automaton
void Automaton::step()
{
	int n, xm, xp, ym, yp;
	for (int x = 0; x < FIELD_W; x++)
	{
		xm = !x ? FIELD_W - 1 : x - 1;
		xp = x == FIELD_W - 1 ? 0 : x + 1;
		for (int y = 0; y < FIELD_H; y++)
		{
			ym = !y ? FIELD_H - 1 : y - 1;
			yp = y == FIELD_H - 1 ? 0 : y + 1;

			n = state[xm][ym];
			n = (n << 1) + state[x][ym];
			n = (n << 1) + state[xp][ym];
			n = (n << 1) + state[xm][y];
			n = (n << 1) + state[x][y];
			n = (n << 1) + state[xp][y];
			n = (n << 1) + state[xm][yp];
			n = (n << 1) + state[x][yp];
			n = (n << 1) + state[xp][yp];
			
			next_state[x][y] = rule[n];
			n = 0;
		}
		n = 0;
	}
	for (int i = 0; i < FIELD_W; i++)
	{
		for (int j = 0; j < FIELD_H; j++)
			state[i][j] = next_state[i][j];
	}
}

// Continuously iterates and draws the automaton
void Automaton::draw(Image* out, int fps)
{
	if (delay.getElapsedTime().asMilliseconds() > 1000.0 / fps)
	{
		step();
		for (int x = 0; x < FIELD_W; x++)
		{
			for (int y = 0; y < FIELD_H; y++)
			{
				if (state[x][y])
					out->setPixel(x, y, Color::Black);
				else
					out->setPixel(x, y, Color::White);
			}
		}
		delay.restart();
	}
}

// Draws the automaton only once (for debugging and future features)
void Automaton::draw(Image* out)
{
	for (int x = 0; x < FIELD_W; x++)
	{
		for (int y = 0; y < FIELD_H; y++)
		{
			if (state[x][y])
				out->setPixel(x, y, Color::Black);
			else
				out->setPixel(x, y, Color::White);
		}
	}
}
