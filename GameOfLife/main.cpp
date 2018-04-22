#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <string>
#include <conio.h>
#include "gol.h"
#include "Test.h"

void routine(GoL::CField* pField, std::mutex* mt)
{
	while (true)
	{
		mt->lock();	
		system("cls");
		if(!pField->IsPaused())	//If not paused - make next generation of life.
			pField->NextGen();
		pField->Print();	//Draw frame of game
		std::cout << "WASD - Move marker. Spacebar - mark cell. Enter - Pause. R - Speed(-), T - Speed(+), Z - Save, X - Load, Q - Exit\n";	//Show tooltip
		mt->unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(pField->GetWaitInterval()));	//Get wait time of game and wait for it.
	}
}

int main()
{
	GoL::CField gameField(50, 25);	//Creating field object
	std::mutex mt;	//Mutex for pausing thread when keyboard input required.
	std::thread thread_routine(routine, &gameField, &mt);	//Running an updating thread.
	std::string fileName;	//Buffer for file name
	Tests::CTestSavedGameFile test1;	//Test unit for save to the file testing
	
	char c = 0;
	while (c != 'q' && c != 'Q')	//Close application when Q pressed.
	{
		c = _getch();
		switch (c)
		{
		case 'w':	//Moving UP on W
		case 'W':
			gameField.MoveMarker(GoL::CMarker::Direction_t::UP);
			break;

		case 'a':	//Moving LEFT on A
		case 'A':
			gameField.MoveMarker(GoL::CMarker::Direction_t::LEFT);
			break;

		case 's':	//Moving DOWN on S
		case 'S':
			gameField.MoveMarker(GoL::CMarker::Direction_t::DOWN);
			break;
		case 'd':	//Moving RIGHT on D
		case 'D':
			gameField.MoveMarker(GoL::CMarker::Direction_t::RIGHT);
			break;

		case 'r':	//Slowdown on R
		case 'R':
			gameField.SlowDown();
			break;

		case 't':	//Speedup on T
		case 'T':
			gameField.SpeedUp();
			break;

		case '\r':	//Pause or Unpause life process
			gameField.Pause();
			break;

		case ' ':	//Set or unset cell's life state on spacebar pressed
			gameField.MarkCell();
			break;

		case 'z':	//Save to file on Z
		case 'Z':
			mt.lock();	//Pause updating thread to let user enter file name.
			gameField.Pause(true);	//Pause generation and stop printing
			system("cls");	//Clear screen and print data for convinience
			gameField.Print();	
			std::cout << "Enter name of file: ";
			std::cin >> fileName;	//Ask for a file name
			gameField.SaveToTheFile(fileName.c_str());	//Save field to the file
			test1.Test(fileName.c_str(), &gameField);	//Test it
			mt.unlock();	//Resume updating thread
			break;

		case 'x':	//Load from file on X
		case 'X':
			mt.lock(); //Pause updating thread to let user enter file name.
			gameField.Pause(true); //Pause generation and stop printing
			system("cls"); //Clear screen and print data for convinience
			gameField.Print();
			std::cout << "Enter name of file: ";
			std::cin >> fileName;  //Ask for a file name
			gameField.LoadFromFile(fileName.c_str()); //Load field from file
			mt.unlock();	//Resume updating thread
			break;
		}
	}

	thread_routine.detach();	//Stop updating thread
	return 0;
}