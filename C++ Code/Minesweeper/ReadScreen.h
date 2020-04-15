#pragma once
#include "tupleHash.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <unordered_map>
#include <tuple>

class ReadScreen {
private:
	// Stored Images
	cv::Mat mClosedBox;
	cv::Mat mSmiley;

	// Screen MineField Size will be used in cropping Screen Shot
	cv::Rect mMineField;

	// Screen and Box lengths
	std::tuple<int, int> mRestart;

	bool mFlag = false;
public:
	int mBoxWidth, mBoxHeight;
	std::vector< cv::Mat > mNumbers;
	int mRow, mCol;
	// Will be used when we match Previous and Current  minefield
	// The difference will provide us contours ( pixel points where field matches )
	// Those points will be searched in this map to get the indices
	std::unordered_map< std::tuple<int, int>, std::tuple<int, int> > mCoordinatesToIndex;

	// Will be used to click on a specific field
	std::vector< std::vector< std::tuple<int, int> > >  mIndexToCoordinates;
private:
	void FindFieldCoordinates();
	void RightClick(int, int);
public:
	ReadScreen(bool);
	void Restart();
	bool IsKeyPressed();
	cv::Mat ScreenShot();
	void SetFlag(int, int);
	void LeftClick(int, int);
	void CropImage(cv::Mat&);
	void TemplateMatching(cv::Mat&, cv::Mat&, std::vector<std::vector<cv::Point>>&, double);
};


