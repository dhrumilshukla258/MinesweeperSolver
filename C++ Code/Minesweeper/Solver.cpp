#include "Solver.h"
#include <iostream>

Solver::Solver(int totalMine) : mField(totalMine), mCount(0)
{
}

void Solver::Rule12(Field& mTempField,int x, int y)
{
	const std::tuple < std::set< std::tuple<int, int> >,
		int  >& ulkm = mTempField.UnKnownLocations_And_KnownMine(x, y);

	int val = mTempField.GetMineCount(x, y);
	int size = std::get<0>(ulkm).size();
	if (val - std::get<1>(ulkm) == size) {
		for (const std::tuple<int, int>& loc : std::get<0>(ulkm)) {
			mTempField.MarkAsMine(std::get<0>(loc), std::get<1>(loc));
		}
	}
	if (val == std::get<1>(ulkm)) {
		for (const std::tuple<int, int>& loc : std::get<0>(ulkm)) {
			mTempField.MarkAsSafe(std::get<0>(loc), std::get<1>(loc));
		}
	}
}

void Solver::Rule3(Field& mTempField, int x1, int y1, int x2, int y2)
{
	const std::tuple < std::set< std::tuple<int, int> >,
		int  >& ulkm_x = mTempField.UnKnownLocations_And_KnownMine(x1, y1);

	int val_x = mTempField.GetMineCount(x1,y1);

	const std::tuple < std::set< std::tuple<int, int> >,
		int  >& ulkm_y = mTempField.UnKnownLocations_And_KnownMine(x2, y2);

	int val_y = mTempField.GetMineCount(x2, y2);

	std::set< std::tuple<int, int > > mine = std::get<0>(ulkm_x);
	std::set< std::tuple<int, int > > safe = std::get<0>(ulkm_y);;

	std::set< std::tuple<int, int> >::iterator it;
	for (const std::tuple<int, int>& loc_y : std::get<0>(ulkm_y)) {
		it = mine.find(loc_y);
		if (mine.end() != it)
			mine.erase(it);
	}
	for (const std::tuple<int, int>& loc_x : std::get<0>(ulkm_x)) {
		it = safe.find(loc_x);
		if (safe.end() != it)
			safe.erase(it);
	}

	int mine_size = mine.size();
	int safe_size = safe.size();
	if ( (val_x - std::get<1>(ulkm_x)) - (val_y - std::get<1>(ulkm_y) ) == mine_size) {
		for (const std::tuple<int, int>& loc : mine)
			mTempField.MarkAsMine(std::get<0>(loc), std::get<1>(loc));

		for (const std::tuple<int, int>& loc : safe)
			mTempField.MarkAsSafe(std::get<0>(loc), std::get<1>(loc));

	}
	
	if ( (val_y - std::get<1>(ulkm_y)) - (val_x - std::get<1>(ulkm_x) ) == safe_size) {
		//The Opposite of what we did above to avoid multiple calculations
		for (const std::tuple<int, int>& loc : safe)
			mTempField.MarkAsMine(std::get<0>(loc), std::get<1>(loc));

		for (const std::tuple<int, int>& loc : mine)
			mTempField.MarkAsSafe(std::get<0>(loc), std::get<1>(loc));
	}

}

std::tuple<bool, bool> Solver::Rule45(Field& mTempField, int x, int y)
{
	const std::tuple < int, int >& ulkm = mTempField.UnknownLocation_And_KnownMine_Count(x, y);
	int val = mTempField.GetMineCount(x, y);
	return std::make_tuple( val < std::get<1>(ulkm), val > std::get<1>(ulkm) + std::get<0>(ulkm));
}

bool Solver::ApplyRules(Field& mTempField)
{
	const std::vector<std::tuple<int, int>>& visitedIndex = mTempField.GetClickedValues();
	
	int i,j, k, x1, y1;
	for ( i=0; i<visitedIndex.size(); ++i) {
		x1 = std::get<0>(visitedIndex[i]);
		y1 = std::get<1>(visitedIndex[i]);

		//Total of 15 + 8  Positions checked
		for (j = -2; j < 3; ++j) {
			for (k = -2; k < 3; ++k) {
				if ((k || j) && x1 + j<mField.GetRow() && x1 + j>-1 && y1 + k<mField.GetCol() && y1 + k>-1) {
					if (!mField.IsUnknown(x1 + j, y1 + k) && !mField.IsMarkedMine(x1 + j, y1 + k) && !mField.IsMarkedSafe(x1 + j, y1 + k)) {
						if (mField.GetMineCount(x1 + j, y1 + k))
							Rule3_Aux(x1, y1, x1 + j, y1 + k);
					}
				}
			}
		}
		
		for (j = 0; j < visitedIndex.size(); ++j) {
			Rule12(mTempField, std::get<0>(visitedIndex[j]), std::get<1>(visitedIndex[j]));
			std::tuple<bool, bool> ans = Rule45(mTempField, std::get<0>(visitedIndex[j]), std::get<1>(visitedIndex[j]));
			if (std::get<0>(ans) || std::get<1>(ans))
				return true;
		}
	}

	for (j = 0; j < visitedIndex.size(); ++j) {
		Rule12(mTempField, std::get<0>(visitedIndex[j]), std::get<1>(visitedIndex[j]));
		std::tuple<bool, bool> ans = Rule45(mTempField, std::get<0>(visitedIndex[j]), std::get<1>(visitedIndex[j]));
		if (std::get<0>(ans) || std::get<1>(ans))
			return true;
	}
	
	return false;
}

bool Solver::IsMine(int x, int y)
{
	Field mTempField = mField;
	mTempField.MarkAsSafe(x, y);
	return ApplyRules(mTempField);
}

bool Solver::IsSafe(int x, int y)
{
	Field mTempField = mField;
	mTempField.MarkAsMine(x, y);
	return ApplyRules(mTempField);
}

void Solver::Rule12_Aux(int x, int y)
{
	const std::tuple < std::set< std::tuple<int, int> >,
		int  >& ulkm = mField.UnKnownLocations_And_KnownMine(x, y);

	int val = mField.GetMineCount(x, y);
	int size = std::get<0>(ulkm).size();
	if (val - std::get<1>(ulkm) == size) {
		for (const std::tuple<int, int>& loc : std::get<0>(ulkm)) {
			mField.MarkAsMineOnScreen(std::get<0>(loc), std::get<1>(loc));
		}
	}
	if (val == std::get<1>(ulkm)) {
		for (const std::tuple<int, int>& loc : std::get<0>(ulkm)) {
			mField.OpenLocation(std::get<0>(loc), std::get<1>(loc));
		}
	}
}

void Solver::Rule3_Aux(int x1, int y1, int x2, int y2)
{
	const std::tuple < std::set< std::tuple<int, int> >,
		int  >& ulkm_x = mField.UnKnownLocations_And_KnownMine(x1, y1);

	int val_x = mField.GetMineCount(x1, y1);

	const std::tuple < std::set< std::tuple<int, int> >,
		int  >& ulkm_y = mField.UnKnownLocations_And_KnownMine(x2, y2);

	int val_y = mField.GetMineCount(x2, y2);

	std::set< std::tuple<int, int > > mine = std::get<0>(ulkm_x);
	std::set< std::tuple<int, int > > safe = std::get<0>(ulkm_y);;

	std::set< std::tuple<int, int> >::iterator it;
	for (const std::tuple<int, int>& loc_y : std::get<0>(ulkm_y)) {
		it = mine.find(loc_y);
		if (mine.end() != it)
			mine.erase(it);
	}
	for (const std::tuple<int, int>& loc_x : std::get<0>(ulkm_x)) {
		it = safe.find(loc_x);
		if (safe.end() != it)
			safe.erase(it);
	}

	int mine_size = mine.size();
	int safe_size = safe.size();
	if (((val_x - std::get<1>(ulkm_x)) - (val_y - std::get<1>(ulkm_y))) == (mine_size)) {
		for (const std::tuple<int, int>& loc : mine)
			mField.MarkAsMineOnScreen(std::get<0>(loc), std::get<1>(loc));

		for (const std::tuple<int, int>& loc : safe)
			mField.OpenLocation(std::get<0>(loc), std::get<1>(loc));
	}
	else if (((val_y - std::get<1>(ulkm_y)) - (val_x - std::get<1>(ulkm_x))) == (safe_size)) {
		//The Opposite of what we did above to avoid multiple calculations
		for (const std::tuple<int, int>& loc : safe)
			mField.MarkAsMineOnScreen(std::get<0>(loc), std::get<1>(loc));

		for (const std::tuple<int, int>& loc : mine)
			mField.OpenLocation(std::get<0>(loc), std::get<1>(loc));
	}

}

int Solver::ApplyRules_Aux()
{
	std::cerr << "Using Regular Rules: \n";
	const std::vector<std::tuple<int, int>>& visitedIndex = mField.GetClickedValues();
	int i, j, k, x1, x2, y1, y2;

	for ( i = 0; i < visitedIndex.size(); ++i) {
		Rule12_Aux( std::get<0>(visitedIndex[i]), std::get<1>(visitedIndex[i]));
	}

	for ( i = 0; i < visitedIndex.size(); ++i) {
		x1 = std::get<0>(visitedIndex[i]);
		y1 = std::get<1>(visitedIndex[i]);

		//Total of 15 + 8  Positions checked
		for ( j = -2; j < 3; ++j) {
			for ( k = -2; k < 3; ++k) {
				if ((k || j) && x1 + j<mField.GetRow() && x1 + j>-1 && y1 + k<mField.GetCol() && y1 + k>-1) {
					if (!mField.IsUnknown(x1 + j, y1 + k) && !mField.IsMarkedMine(x1 + j, y1 + k)) {
						if ( mField.GetMineCount(x1 + j, y1 + k))
							Rule3_Aux(x1, y1, x1+j, y1+k);
					}

				}
			}
		}
	}

	return mField.TotalUknownLocations();
}

int Solver::Open_Any_XY()
{
	int i = -1, j = -1;
	for (i = 0; i < mField.GetRow(); ++i) {
		for (j = 0; j < mField.GetCol(); ++j) {
			if (mField.IsUnknown(i, j)) {
				mField.OpenLocation(i, j);
				return mField.TotalUknownLocations();
			}

		}
	}
	return -1;
}

bool Solver::LoopCheck()
{
	return mField.GetStatus() and mCount > 0;
}

bool Solver::Solve()
{
	mField.OpenLocation(mField.GetRow() / 2, mField.GetCol() / 2);
	mCount = mField.TotalUknownLocations();
	int prev_count = -1;

	int loop = 0;
	while (LoopCheck() && mCount != prev_count) {
		prev_count = mCount;
		std::cerr << "Loop Number:- " << loop << "\n";

		mCount = ApplyRules_Aux();
		if (LoopCheck() && prev_count == mCount) {
			mCount = Solve_Aux();
			if (LoopCheck() && prev_count == mCount) {
				std::cerr << "Checked for Random value\n";
				mCount = Open_Any_XY();
			}
		}
		++loop;
	}

	//mCount = Solve_Aux();
	if (mField.TotalUknownLocations() != 0 && mField.TotalMarkedMines() == mField.TotalMines()) {

		for (int i = 0; i < mField.GetRow(); ++i) {
			for (int j = 0; j < mField.GetCol(); ++j) {
				if (mField.IsUnknown(i, j))
				{
					mField.OpenLocation(i, j, true);
				}
			}
		}
		mCount = 0;
	}
	std::cerr << "Total Unknown Locations: " << mCount << "\n";
	std::cerr << "Status of mField: " << mField.GetStatus() << "\n";
	std::cerr << "Total Known Mines: " << mField.TotalMarkedMines() << "\n";
	std::cerr << "This is how the board looked to the AI:\n";
	std::cerr << mField;
	return mCount == 0;
}

int Solver::Solve_Aux() 
{
	std::cerr << "Using Resolution Method: \n";
	for (int i = 0; i < mField.GetRow(); ++i) {
		for (int j = 0; j < mField.GetCol(); ++j) {
			if (mField.IsUnknown(i, j)) {

				const std::tuple < int,	int >& ulkm = mField.UnknownLocation_And_KnownMine_Count(i, j);

				bool xCheck = (i == 0 || j == mField.GetCol() - 1);
				bool yCheck = (j == 0 || i == mField.GetRow() - 1);

				int size = std::get<0>(ulkm);
				bool check = true;
				
				if (xCheck) {
					if ((yCheck) && size == 3)	check = false;
					else if (size == 5)	check = false;
				}
				else if (yCheck) {
					if ((xCheck) && size == 3)	check = false;
					else if (size == 5)	check = false;
				}
				else {
					if (size == 8)	check = false;
				}

				if ( check ){
					
					if (IsMine(i, j)) {
						std::cerr << "(" << i << "," << j << ")" << " IsMine\n";

						mField.MarkAsMineOnScreen(i, j);
						return mField.TotalUknownLocations();
					}
					if (IsSafe(i, j)) {
						std::cerr << "(" << i << "," << j << ")" << " IsSafe\n";
						mField.OpenLocation(i, j);
						return mField.TotalUknownLocations();
					}
				}
			}
		}
	}
	return mField.TotalUknownLocations();
}
