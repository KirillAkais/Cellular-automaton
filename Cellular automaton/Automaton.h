#pragma once
#ifndef automaton
#define automaton

#include "SFML/Graphics.hpp"
#include <vector>
#include <fstream>

#define DEFAULT_FIELD_W 64
#define DEFAULT_FIELD_H 64
#define DEFAULT_FIELD_SIZE 4096 //FIELD_W * FIELD_H
#define RULE_SIZE 1024
#define CONWAY_SIZE 20
#define POPULATION_SIZE 128

#define EVOLUTE_DYNAMIC 0
#define EVOLUTE_PATTERN 1
#define EVOLUTE_PATTERN_FAST 2
#define EVOLUTE_PATTERN_STATIC 3
#define EVOLUTE_ID 4

using namespace sf;
using namespace std;

class Automaton
{
	bool** next_state;
	Clock delay;

public:
	bool* conway;
	bool* second_order;
	bool* rule_conway;
	bool* rule;
	bool** state;
	bool** prev_state;
	int FIELD_W;
	int FIELD_H;

	Automaton(bool* conway, bool* second_order);
	Automaton(bool* conway, bool* second_order, int field_w, int field_h);
	void set_gene_ratio(int ratio);
	void set_conway_life();
	void convert_from_conway();
	void fill_random();
	void fill_ratio(int ratio);
	void fill_one();
	void step();
	void write(string name, bool overwrite);
	void read(string name);
	void clone(Automaton* a);
	void draw(Image* out, int fps);
	void draw(Image* out);
};

class Population
{
	Clock delay;
public:
	int* fitness;
	bool* conway;
	bool* second_order;
	Automaton** automata;

	Population();
	void set_gene_ratio(int ratio);
	void set_conway_life();
	void convert_from_conway();
	void fill_random();
	void fill_ratio(int ratio);
	void fill_one();
	void select_dynamic();
	void select_pattern(vector<int> pattern, int mistakes);
	void select_pattern_fast(vector<int> pattern);
	void select_pattern_static(vector<int> pattern);
	void select_id(vector<int>* id, vector<int>* fit);
	void sort_fitness();
	void evolute(int type, int mutation_chance, int mutation_amount, vector<int>* param1, int* param2);
	void evolute(int times, int type, int mutation_chance, int mutation_amount, vector<int>* param1, int* param2);
	void step();
	void step(int times);
	void draw(int id_begin, int id_end, Image** out, int fps);
	void draw(int id_begin, int id_end, Image** out);
};

#endif // !automaton
