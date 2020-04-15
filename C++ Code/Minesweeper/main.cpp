#include <iostream>
#include <windows.h>

#include "Solver.h"
#include "ReadScreen.h"
#include "Field.h"
ReadScreen* rS = nullptr;



int main() {
	
	std::cout << " ----------------------------------------- \n" <<
		" Make sure Minesweeeper is fully visible before starting and\n" <<
		" do not move the Window onces when the application is started\n" <<
		" To Close the program in the middle Press Left Shift\n"
		" ----------------------------------------- \n" <<
		" Enter Total Mines: ";

	int totalMines;
	std::string strFlag;
	std::cin >> totalMines;
	std::cout << "Would you like to SetFlag (y/n): ";
	std::cin >> strFlag;

	if (strFlag == "y" || strFlag == "Y") {
		rS = new ReadScreen(true);
	}
	else {
		rS = new ReadScreen(false);
	}
	
	if (rS->IsKeyPressed()) {
		INPUT ip;

		// Set up a generic keyboard event.
		ip.type = INPUT_KEYBOARD;
		ip.ki.wScan = 0; // hardware scan code for key
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;

		// Press the "A" key
		ip.ki.wVk = 0xA0; // virtual-key code for the "a" key
		ip.ki.dwFlags = 0; // 0 for key press
		SendInput(1, &ip, sizeof(INPUT));

		ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
		SendInput(1, &ip, sizeof(INPUT));
	}

	bool solved = false;
	std::vector<int> tot;
	while (!solved && tot.size() < 15 && !rS->IsKeyPressed()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		std::cout << "\n=============================================\n";
		Solver sol(totalMines);
		if (!sol.Solve()) {
			rS->Restart();
		}
		else {
			solved = true;
		}
		tot.push_back(0);
	}
	
	return 0;
	
}
