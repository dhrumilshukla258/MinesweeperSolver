#include "Field.h"
#include "ReadScreen.h"
#include <thread>         // std::this_thread::sleep_for
#include <chrono>
#include <string>
#include <iostream>

extern ReadScreen* rS;

Field::Field(int totalMine) : 
	mBoard(nullptr),
	mStatus(true) ,
	mRow(rS->mRow),
	mCol(rS->mCol),
	mTotalMines(totalMine), 
	mMarkedMines(0),
	mZeros(0),
	mOpenedValue()
{
	// All Values Unknown initially
	mBoard = new int* [mRow];
	for (int i = 0; i < mRow; ++i) {
		mBoard[i] = new int[mCol];
		for (int j = 0; j < mCol; ++j) {
			mBoard[i][j] = -10;
		}
	}
}

Field::~Field()
{
	for (int i = 0; i < mRow; ++i) { delete[] mBoard[i]; }
	delete[] mBoard;
	mOpenedValue.clear();
}

Field::Field(const Field& rhs) : 
	mBoard(new int* [rhs.mRow]),
	mStatus(rhs.mStatus),
	mRow(rhs.mRow),
	mCol(rhs.mCol),
	mTotalMines(rhs.mTotalMines),
	mMarkedMines(rhs.mMarkedMines),
	mZeros(rhs.mZeros),
	mOpenedValue(rhs.mOpenedValue)
{
	mRow = rhs.mRow;
	mCol = rhs.mCol;
	for (int x = 0; x < mRow; ++x) {
		mBoard[x] = new int[mCol];
		for (int y = 0; y < mCol; ++y) mBoard[x][y] = rhs.mBoard[x][y];  /*init*/
	}
}

Field& Field::operator=(const Field& rhs)
{
	mOpenedValue = rhs.mOpenedValue;
	mRow = rhs.mRow;
	mCol = rhs.mCol;
	mZeros = rhs.mZeros;
	mBoard = new int* [rhs.mRow];
	for (int x = 0; x < mRow; ++x) {
		mBoard[x] = new int[mCol];
		for (int y = 0; y < mCol; ++y) mBoard[x][y] = rhs.mBoard[x][y];  /*init*/
	}
	return *this;
}

bool Field::IsUnknown(int x, int y) 
{
	return mBoard[x][y] == -10;
}

bool Field::IsMarkedSafe(int x, int y) 
{
	return mBoard[x][y] == -2;
}

void Field::OpenLocation(int x, int y, bool last) {
	if ( ( !IsUnknown(x, y) || !GetStatus() ) && !last)
		return;

	cv::Mat screenBefore = rS->ScreenShot();
	rS->CropImage(screenBefore);
	//cv::imwrite("ss1.png", screenBefore);
	rS->LeftClick(std::get<0>(rS->mIndexToCoordinates[x][y]), std::get<1>(rS->mIndexToCoordinates[x][y]));
	
	//std::this_thread::sleep_for(std::chrono::milliseconds(100));

	cv::Mat screenAfter = rS->ScreenShot();
	rS->CropImage(screenAfter);
	bool eq = true;
	
	std::vector<int> needed_to_break_loop;
	while (eq && needed_to_break_loop .size()<5) {
		screenAfter = rS->ScreenShot();
		rS->CropImage(screenAfter);
		eq = std::equal(screenBefore.begin<uchar>(), screenBefore.end<uchar>(), screenAfter.begin<uchar>());
		needed_to_break_loop.push_back(0);
	}
	//std::cout << " ";
	//std::this_thread::sleep_for(std::chrono::milliseconds(100));

	cv::Mat res = screenBefore ^ screenAfter;
	
	for (int n=0; n< rS->mNumbers.size(); ++n) {
		std::vector<std::vector<cv::Point>> contours;
		rS->TemplateMatching(res, rS->mNumbers[n], contours, 0.9);
		
		for (int i = 0; i < contours.size(); ++i) {
			std::unordered_map< std::tuple<int, int>, std::tuple<int, int> >::iterator got = rS->mCoordinatesToIndex.find(std::make_tuple(contours[i][0].x, contours[i][0].y));
			if (got == rS->mCoordinatesToIndex.end()) {
				std::cerr << " Didn't find coordinates -- OpenLocation Error";
			}
			std::tuple<int,int> pt = got->second;
			mBoard[std::get<0>(pt)][std::get<1>(pt)] = n;
			//std::cout << "OpenedValue: (" <<std::get<0>(pt) << "," << std::get<1>(pt) << ") " << mBoard[std::get<0>(pt)][std::get<1>(pt)] << "\n";
			CheckValue( std::get<0>(pt), std::get<1>(pt) );
		}
	}
}

void Field::CheckValue(int x, int y)
{
	// Assigning value in list for fast iteration
	if (mBoard[x][y] > 0 && mBoard[x][y] <= 8) {
		mOpenedValue.push_back(std::make_tuple(x, y));
	}
	else if (mBoard[x][y] == 0) {
		++mZeros;
	}
	else if (mBoard[x][y] == 9 || mBoard[x][y] == 10) {
		mStatus = false;
	}
}

//Used by Solver Class
int Field::GetRow()
{
	return mRow;
}

int Field::GetCol()
{
	return mCol;
}

bool Field::GetStatus()
{
	int totalVal = (mRow * mCol);
	return mStatus &&
		  !rS->IsKeyPressed() &&
		  (mTotalMines != totalVal - mOpenedValue.size() - mZeros) &&
		  (mTotalMines != mMarkedMines);
}

const std::vector<std::tuple<int, int>>& Field::GetClickedValues() 
{
	return mOpenedValue;
}

int Field::TotalUknownLocations()
{
	return (mRow * mCol) - mOpenedValue.size() - mMarkedMines - mZeros;
}

int Field::TotalMarkedMines()
{
	return mMarkedMines;
}

int Field::TotalMines()
{
	return mTotalMines;
}

void Field::MarkAsSafe(int x, int y)
{
	//std::cout << "\nMarkAsSafe: (" << x << "," << y << ") : "<< mBoard[x][y];

	mBoard[x][y] = -2;
}

void Field::MarkAsMine(int x, int y) {
	//std::cout << "\nMarkAsMine: (" << x << "," << y << ") : " << mBoard[x][y];
	mBoard[x][y] = -1;
}
	
void Field::MarkAsMineOnScreen(int x, int y)
{
	++mMarkedMines;
	MarkAsMine(x, y);
	rS->SetFlag(x, y);
}

bool Field::IsMarkedMine(int x, int y)
{
	return mBoard[x][y] == -1;
}

std::tuple < std::set< std::tuple<int, int> >,
	int  >
	Field::UnKnownLocations_And_KnownMine(int x, int y) {

	std::set<std::tuple<int, int> > ul;
	int km=0;
	for (int i = -1; i < 2; ++i) {
		for (int j = -1; j < 2; ++j) {
			if ((i || j) && x + i<mRow && x + i>-1 && y + j<mCol && y + j>-1) {
				if (IsUnknown(x + i, y + j)) {
					ul.insert(std::make_tuple(x + i, y + j));
				}
				if (IsMarkedMine(x + i, y + j)) {
					++km;
				}
			}
		}
	}
	return std::make_tuple(ul,km);
}

std::tuple<int,int> Field::UnknownLocation_And_KnownMine_Count(int x, int y) {
	int ul=0,km=0;
	for (int i = -1; i < 2; ++i) {
		for (int j = -1; j < 2; ++j) {
			if ((i || j) && x + i<mRow && x + i>-1 && y + j<mCol && y + j>-1) {
				if (IsUnknown(x + i, y + j)) {
					++ul;
				}
				if (IsMarkedMine(x + i, y + j)) {
					++km;
				}
			}
		}
	}
	return std::make_tuple(ul,km);
}

int Field::GetMineCount(int x,int y)
{
	if ( IsUnknown(x, y) ) {
		std::cerr << x << "," << y << " Mine Count Asked before opening the location";
	}
	return mBoard[x][y];
}

std::ostream& operator<<(std::ostream& os, const Field& f)
{
	os << "=================================================================\n";
	for (int i = 0; i < f.mRow; ++i) {
		for (int j = 0; j < f.mCol; ++j) {
			os << f.mBoard[i][j] << "  ";
		}
		os << "\n";
	}
	os << "=================================================================\n";
	return os;
}

/*
 Code to draw Square on the matching area
cv::Mat1b mask(match.rows, match.cols, uchar(0));
drawContours(mask, contours, i, cv::Scalar(255), cv::FILLED);

cv::Point max_point;
double max_val;
minMaxLoc(match, NULL, &max_val, NULL, &max_point, mask);

cv::rectangle(greyScreen, cv::Rect rec(max_point.x, max_point.y, mClosedBox.cols, mClosedBox.rows), cv::Scalar(0, 0, 255), 2);
*/

/*



for (int i = 0; i < mRow; ++i) {
	for (int j = 0; j < mCol; ++j) {
		LeftClick(std::get<0>(mIndexToCoordinates[i][j]), std::get<1>(mIndexToCoordinates[i][j]));
	}
}
*/

