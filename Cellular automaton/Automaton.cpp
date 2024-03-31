#include "Automaton.h"
#include <iostream>
#include <thread>

//using namespace std;

Automaton::Automaton()
{
	this->rule = new bool[RULE_SIZE];
	for (int i = 0; i < RULE_SIZE; i++)
		rule[i] = rand() % 2;

	this->state = new bool*[FIELD_W];
	this->prev_state = new bool*[FIELD_W];
	this->next_state = new bool*[FIELD_W];
	for (int i = 0; i < FIELD_W; i++)
	{
		this->state[i] = new bool[FIELD_H];
		this->prev_state[i] = new bool[FIELD_H];
		this->next_state[i] = new bool[FIELD_H];

		for (int j = 0; j < FIELD_H; j++)
			this->state[i][j] = rand() % 2;
	}
}

void Automaton::set_gene_ratio(int ratio)
{
	for (int i = 0; i < RULE_SIZE; i++)
		rule[i] = (rand() % 100) < ratio;
}

// Sets rule to Conway's game of life
void Automaton::set_conway()
{
	int alive, alive_neigbors=0;
	Uint16 s;
	for (int i = 0; i < RULE_SIZE; i++)
	{
		s = i;
		alive = s & 16;
		s &= 0x01EF;
		alive_neigbors = _Popcount(s);
		if ((!alive && alive_neigbors == 3) ||
			(alive && (alive_neigbors == 2 || alive_neigbors == 3)))
			rule[i] = 1;
		else
			rule[i] = 0;
	}
}

void Automaton::fill_random()
{
	for (int i = 0; i < FIELD_W; i++)
	{
		for (int j = 0; j < FIELD_H; j++)
			this->state[i][j] = rand() % 2;
	}
}

void Automaton::fill_ratio(int ratio)
{
	for (int i = 0; i < FIELD_W; i++)
	{
		for (int j = 0; j < FIELD_H; j++)
			this->state[i][j] = (rand() % 100) < ratio;
	}
}

void Automaton::fill_one()
{
	for (int i = 0; i < FIELD_W; i++)
	{
		for (int j = 0; j < FIELD_H; j++)
			this->state[i][j] = 0;
	}
	state[FIELD_W / 2][FIELD_H / 2] = 1;
}

// Iterates the automaton
void Automaton::step()
{
	int n, xm, xp, ym, yp, x, y;
	for (x = 0; x < FIELD_W; x++)
	{
		xm = !x ? FIELD_W - 1 : x - 1;
		xp = x == FIELD_W - 1 ? 0 : x + 1;
		for (y = 0; y < FIELD_H; y++)
		{
			ym = !y ? FIELD_H - 1 : y - 1;
			yp = y == FIELD_H - 1 ? 0 : y + 1;

			n = state[xm][ym]
				| (state[x][ym] << 1)
				| (state[xp][ym] << 2)
				| (state[xm][y] << 3)
				| (state[x][y] << 4)
				| (state[xp][y] << 5)
				| (state[xm][yp] << 6)
				| (state[x][yp] << 7)
				| (state[xp][yp] << 8);
				//| (prev_state[x][y] << 9);
			
			next_state[x][y] = rule[n];
		}
	}
	swap(prev_state, state);
	swap(next_state, state);
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

Population::Population()
{
	this->automata = new Automaton*[POPULATION_SIZE];
	this->fitness = new int[POPULATION_SIZE];
	for (int i = 0; i < POPULATION_SIZE; i++)
	{
		automata[i] = new Automaton;
		this->fitness[i] = 0;
	}
}

void Population::set_gene_ratio(int ratio)
{
	for (int i = 0; i < POPULATION_SIZE; i++)
		automata[i]->set_gene_ratio(ratio);
}

void Population::set_conway()
{
	for (int i = 0; i < POPULATION_SIZE; i++)
		automata[i]->set_conway();
}

void Population::fill_random()
{
	for (int i = 0; i < POPULATION_SIZE; i++)
		automata[i]->fill_random();
}

void Population::fill_ratio(int ratio)
{
	for (int i = 0; i < POPULATION_SIZE; i++)
		automata[i]->fill_ratio(ratio);
}

void Population::fill_one()
{
	for (int i = 0; i < POPULATION_SIZE; i++)
		automata[i]->fill_one();
}

void Population::select_dynamic()
{
	int fit, count1, count2;
	fill_random();
	//cout << "1.1" << endl;
	step(100);
	//cout << "1.2" << endl;
	for (int i = 0; i < POPULATION_SIZE; i++)
	{
		//cout << "1.3: " << i << endl;
		count1 = 0;
		count2 = 0;
		fit = 0;
		for (int x = 0; x < FIELD_W; x++)
		{
			for (int y = 0; y < FIELD_H; y++)
			{
				fit += automata[i]->state[x][y] == automata[i]->prev_state[x][y];
				count1 += automata[i]->state[x][y];
			}
		}
		count2 = FIELD_SIZE - count1;
		count1 = abs(count1 - count2);
		if (count1 > 100)
			fit = 0;
		fitness[i] = fit;
	}
}

void Population::select_pattern(vector<int> pattern, int mistakes)
{
	int n, fit, xm, xmm, xp, xpp, ym, ymm, yp, ypp;
	fill_random();
	//step(20);
	for (int i = 0; i < 30; i++)
	{
		for (int j = 0; j < POPULATION_SIZE; j++)
		{
			fit = 0;
			for (int x = 0; x < FIELD_W; x++)
			{
				xm = !x ? FIELD_W - 1 : x - 1;
				xp = x == FIELD_W - 1 ? 0 : x + 1;
				xmm = x < 2 ? FIELD_W - 2 + x : x - 2;
				xpp = x > FIELD_W - 3 ? x - FIELD_W + 2 : x + 2;
				for (int y = 0; y < FIELD_H; y++)
				{
					ym = !y ? FIELD_H - 1 : y - 1;
					yp = y == FIELD_H - 1 ? 0 : y + 1;
					ymm = y < 2 ? FIELD_H - 2 + y : y - 2;
					ypp = y > FIELD_H - 3 ? y - FIELD_H + 2 : y + 2;

					n = automata[j]->state[xmm][ymm]
						| (automata[j]->state[xm][ymm] << 1)
						| (automata[j]->state[x][ymm] << 2)
						| (automata[j]->state[xp][ymm] << 3)
						| (automata[j]->state[xpp][ymm] << 4)
						| (automata[j]->state[xmm][ym] << 5)
						| (automata[j]->state[xm][ym] << 6)
						| (automata[j]->state[x][ym] << 7)
						| (automata[j]->state[xp][ym] << 8)
						| (automata[j]->state[xpp][ym] << 9)
						| (automata[j]->state[xmm][y] << 10)
						| (automata[j]->state[xm][y] << 11)
						| (automata[j]->state[x][y] << 12)
						| (automata[j]->state[xp][y] << 13)
						| (automata[j]->state[xpp][y] << 14)
						| (automata[j]->state[xmm][yp] << 15)
						| (automata[j]->state[xm][yp] << 16)
						| (automata[j]->state[x][yp] << 17)
						| (automata[j]->state[xp][yp] << 18)
						| (automata[j]->state[xpp][yp] << 19)
						| (automata[j]->state[xmm][ypp] << 20)
						| (automata[j]->state[xm][ypp] << 21)
						| (automata[j]->state[x][ypp] << 22)
						| (automata[j]->state[xp][ypp] << 23)
						| (automata[j]->state[xpp][ypp] << 24);
					for (int i = 0; i < pattern.size(); i++)
					{
						Uint32 res = n ^ pattern[i];
						res = _Popcount(res);
						if (res > 24 - mistakes)
							fit += res - 24 + mistakes;
					}
				}
			}
			fitness[j] = fit;
		}
		step();
	}
}


void Population::select_id(vector<int>* id, vector<int>* fit)
{
}

void Population::sort_fitness()
{
	int i1, i2, end;
	int* a;
	Automaton** b;
	int* merged = new int[POPULATION_SIZE];
	Automaton** merged_automata = new Automaton*[POPULATION_SIZE];

	for (int i = 1; i < POPULATION_SIZE; i <<= 1)
	{
		for (int j = 0; j < POPULATION_SIZE; j += i << 1)
		{
			i1 = 0;
			i2 = 0;
			end = j + (i << 1);
			while (i1 < i && i2 < i)
			{
				if (fitness[j + i1] > fitness[j + i + i2])
				{
					merged[i1 + i2] = fitness[j + i1];
					merged_automata[i1 + i2] = automata[j + i1];
					i1++;
				}
				else
				{
					merged[i1 + i2] = fitness[j + i + i2];
					merged_automata[i1 + i2] = automata[j + i + i2];
					i2++;
				}
			}
			while (i1 < i)
			{
				merged[i1 + i2] = fitness[j + i1];
				merged_automata[i1 + i2] = automata[j + i1];
				i1++;
			}
			while (i2 < i)
			{
				merged[i1 + i2] = fitness[j + i + i2];
				merged_automata[i1 + i2] = automata[j + i + i2];
				i2++;
			}
			for (int k = 0; k < i1 + i2; k++)
			{
				fitness[j + k] = merged[k];
				automata[j + k] = merged_automata[k];
			}
		}
	}
}

void Population::evolute(int type, int mutation_chance, int mutation_amount, vector<int>* param1, int* param2)
{
	int* ids = new int[POPULATION_SIZE / 2];
	thread* t = new thread[POPULATION_SIZE];
	switch (type)
	{
	case EVOLUTE_DYNAMIC:
		//cout << "1" << endl;
		select_dynamic();
		
		//cout << "2" << endl;
		break;
	case EVOLUTE_PATTERN:
		select_pattern(*param1, *param2);
		break;
	default:
		break;
	}
	sort_fitness();
	//cout << "3" << endl;

	for (int i = 0; i < POPULATION_SIZE / 2; i++)
		ids[i] = i;
	random_shuffle(ids, ids + POPULATION_SIZE / 2);
	//cout << "4" << endl;

	for (int i = 0; i < POPULATION_SIZE / 2; i += 2)
	{
		//cout << "5: " << i << endl;
		for (int j = 0; j < RULE_SIZE; j++)
		{
			if (rand() % 2)
			{
				automata[i + POPULATION_SIZE / 2]->rule[j] = automata[ids[i]]->rule[j];
				automata[i + POPULATION_SIZE / 2 + 1]->rule[j] = automata[ids[i + 1]]->rule[j];
			}
			else
			{
				automata[i + POPULATION_SIZE / 2]->rule[j] = automata[ids[i + 1]]->rule[j];
				automata[i + POPULATION_SIZE / 2 + 1]->rule[j] = automata[ids[i]]->rule[j];
			}
		}
		if (rand() % 100 < mutation_chance)
		{
			for (int i = 0; i < mutation_amount; i++)
			{
				int j = rand() % RULE_SIZE;
				automata[i + POPULATION_SIZE / 2]->rule[j] = !automata[i + POPULATION_SIZE / 2]->rule[j];
			}
		}
		if (rand() % 100 < mutation_chance)
		{
			for (int i = 0; i < mutation_amount; i++)
			{
				int j = rand() % RULE_SIZE;
				automata[i + POPULATION_SIZE / 2 + 1]->rule[j] = !automata[i + POPULATION_SIZE / 2 + 1]->rule[j];
			}
		}
	}
}

void Population::evolute(int times, int type, int mutation_chance, int mutation_amount, vector<int>* param1, int* param2)
{
	for (int i = 0; i < times; i++)
	{
		evolute(type, mutation_chance, mutation_amount, param1, param2);
		if (!(i % 100))
			cout << i << "/" << times << "Best fitness: " << fitness[0] << endl;
	}
	sort_fitness();
}

void Population::step()
{
	for (int i = 0; i < POPULATION_SIZE; i++)
		automata[i]->step();
}

void Population::step(int times)
{
	for (int i = 0; i < times; i++)
		for (int i = 0; i < POPULATION_SIZE; i++)
			automata[i]->step();
}

void Population::draw(int id_begin, int id_end, Image** out, int fps)
{
	if (delay.getElapsedTime().asMilliseconds() > 1000.0 / fps)
	{
		step();
		for (int i = id_begin; i < id_end; i++)
		{
			draw(id_begin, id_end, out);
		}
		delay.restart();
	}
}

void Population::draw(int id_begin, int id_end, Image** out)
{
	for (int i = id_begin; i < id_end; i++)
		automata[i]->draw(out[i]);
}
