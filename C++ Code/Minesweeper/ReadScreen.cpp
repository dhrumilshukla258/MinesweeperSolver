#include "ReadScreen.h"
#include <windows.h>
#include <iostream>

ReadScreen::ReadScreen(bool flag) : mBoxWidth(0), mBoxHeight(0), mRow(0), mCol(0), mFlag(flag)
{
	// Reading All Images
	mClosedBox = cv::imread("Images\\closedbox.png", cv::IMREAD_GRAYSCALE);
	if (mClosedBox.empty()) {
		std::cerr << "Error Reading Image Closedbox.png\n";
	}

	for (int i = 0; i < 11; ++i) {
		mNumbers.push_back(cv::imread("Images\\" + std::to_string(i) + ".png", cv::IMREAD_GRAYSCALE));
		if (mNumbers[i].empty()) {
			std::cerr << "Error Reading Image:- " << i << ".png\n";
		}
	}

	mSmiley = cv::imread("Images\\smiley.png", cv::IMREAD_GRAYSCALE);
	if (mSmiley.empty()) {
		std::cerr << "Error Reading Image Smiley.png\n";
	}
	FindFieldCoordinates();
}

void ReadScreen::TemplateMatching(cv::Mat& mainImg, cv::Mat& temImg, std::vector<std::vector<cv::Point>>& contours, double thresh = 0.9)
{
	cv::Mat match;
	cv::matchTemplate(mainImg, temImg, match, cv::TM_CCOEFF_NORMED);

	cv::threshold(match, match, thresh, 1., cv::THRESH_BINARY);

	cv::Mat resb;
	match.convertTo(resb, CV_8U, 255);

	cv::findContours(resb, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

	match.release();
	resb.release();
}

void ReadScreen::FindFieldCoordinates()
{
	cv::Mat currScreen = ScreenShot();
	//cv::Mat currScreen = cv::imread("map1.png", cv::IMREAD_GRAYSCALE);

	std::vector<std::vector<cv::Point>> contours;
	TemplateMatching(currScreen, mClosedBox, contours);

	/* Contours will always be sorted in decreasing order
	the shape of Contours will be len(contours) x 1 */
	mMineField.x = contours[contours.size() - 1][0].x;
	//std::cout << contours.size() << "\n";
	mMineField.y = contours[contours.size() - 1][0].y;
	mBoxWidth = mClosedBox.cols;
	mBoxHeight = mClosedBox.rows;
	mMineField.width = contours[0][0].x - contours[contours.size() - 1][0].x + mBoxWidth;
	mMineField.height = contours[0][0].y - contours[contours.size() - 1][0].y + mBoxHeight;
	mCol = mMineField.width / mBoxWidth;
	mRow = mMineField.height / mBoxHeight;

	// Initializing vector of vector of tuples
	for (int i = 0; i < mRow; ++i) {
		std::vector < std::tuple<int, int> > temp;
		for (int j = 0; j < mCol; ++j) {
			temp.push_back(std::make_tuple(0, 0));
		}
		mIndexToCoordinates.push_back(temp);
	}

	// Mapping Coordinates and indices with one another
	int r = mRow - 1;
	int c = mCol - 1;
	for (int i = 0; i < contours.size(); ++i) {
		//std::cout << i << " -- " << r << " -- " << c << "\n";
		mCoordinatesToIndex[std::make_tuple(contours[i][0].x - mMineField.x, contours[i][0].y - mMineField.y)] = std::make_tuple(r, c);
		std::get<0>(mIndexToCoordinates[r][c]) = contours[i][0].x + (mBoxWidth / 2);
		std::get<1>(mIndexToCoordinates[r][c]) = contours[i][0].y + (mBoxHeight / 2);

		if (c == 0) {
			c = mCol;
			--r;
		}
		--c;
	}

	std::vector<std::vector<cv::Point>> restartcontours;
	TemplateMatching(currScreen, mSmiley, restartcontours);
	// There will always be only one Restart Button ( i.e. a Smiley Face )
	mRestart = std::make_pair(restartcontours[0][0].x + (mSmiley.cols / 2), restartcontours[0][0].y + (mSmiley.rows / 2));

	currScreen.release();
}

void ReadScreen::CropImage(cv::Mat& img)
{
	img = img(mMineField);
}

void ReadScreen::LeftClick(int x, int y)
{
	SetCursorPos(x, y);
	//std::cout << x << " " << y << "\n";
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	SetCursorPos(std::get<0>(mRestart), std::get<1>(mRestart));
}

void ReadScreen::RightClick(int x, int y)
{
	SetCursorPos(x, y);
	//std::cout << x << " " << y << "\n";
	mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
	SetCursorPos(std::get<0>(mRestart), std::get<1>(mRestart));
}

cv::Mat ReadScreen::ScreenShot() {

	HWND hwnd = GetDesktopWindow();
	HDC hwindowDC, hwindowCompatibleDC;

	HBITMAP hbwindow;
	cv::Mat src;
	BITMAPINFOHEADER  bi;

	hwindowDC = GetDC(hwnd);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	RECT windowsize;
	GetWindowRect(GetDesktopWindow(), &windowsize);
	int srcheight = windowsize.bottom;
	int srcwidth = windowsize.right;

	int height = srcheight;  //change this to whatever size you want to resize to
	int width = srcwidth;

	src.create(height, width, CV_8UC4);

	// create a bitmap
	hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

	// avoid memory leak
	DeleteObject(hbwindow); DeleteDC(hwindowCompatibleDC); ReleaseDC(hwnd, hwindowDC);

	cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);
	return src;
}

void ReadScreen::Restart()
{
	LeftClick(std::get<0>(mRestart), std::get<1>(mRestart));
}

void ReadScreen::SetFlag(int x, int y)
{
	if (mFlag) {
		RightClick(std::get<0>(mIndexToCoordinates[x][y]), std::get<1>(mIndexToCoordinates[x][y]));
	}
}

bool ReadScreen::IsKeyPressed()
{
	return GetKeyState(VK_SHIFT) & 1;
}