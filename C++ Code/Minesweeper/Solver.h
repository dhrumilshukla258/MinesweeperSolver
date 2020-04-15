#pragma once
#include "Field.h"

class Solver
{
private:
	Field mField;
public:
	int mCount;
public:
	Solver(int);
	
	// Used in proof by contradiction
	void Rule12(Field&,int, int);
	void Rule3(Field& ,int, int, int, int);
	std::tuple<bool, bool> Rule45(Field&, int, int);
	bool ApplyRules(Field&);
	bool IsMine(int, int);
	bool IsSafe(int, int);

	// Used to open safe blocks and mark mine on screen
	void Rule12_Aux(int, int);
	void Rule3_Aux(int, int, int, int);
	int ApplyRules_Aux();

	int Open_Any_XY();
	bool LoopCheck();
	int Solve_Aux();
	bool Solve();
};

