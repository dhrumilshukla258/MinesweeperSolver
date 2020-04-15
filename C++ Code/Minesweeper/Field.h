#pragma once
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>

#include <vector>
#include <tuple>
#include <map>
#include <set>

//--------------------------------------------------------------------------

class Field 
{
private:
	int** mBoard;
	bool mStatus;
	int mRow, mCol;
	int mTotalMines;
	int mMarkedMines;
	int mZeros;
	std::vector<std::tuple<int, int>> mOpenedValue;

public:
	Field(int);
	~Field();
	Field(const Field& rhs);
	Field& operator=(const Field& rhs);
	friend	std::ostream& operator<<(std::ostream& os, const Field& msf);
	void CheckValue(int, int);
	
	int TotalUknownLocations();
	int TotalMarkedMines();
	int TotalMines();

	void OpenLocation(int, int, bool last=false);
	bool IsUnknown(int, int);
	bool IsMarkedMine(int, int);
	bool IsMarkedSafe(int, int);
	int GetRow();
	int GetCol();
	bool GetStatus();
	const std::vector<std::tuple<int, int>>& GetClickedValues();
	void MarkAsSafe(int, int);
	void MarkAsMine(int, int);
	void MarkAsMineOnScreen(int, int);

	int GetMineCount(int, int);
	std::tuple < std::set< std::tuple<int, int> >,
		int  >
		UnKnownLocations_And_KnownMine(int, int);
	std::tuple<int, int>
		UnknownLocation_And_KnownMine_Count(int, int);
};

