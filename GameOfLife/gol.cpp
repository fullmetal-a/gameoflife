#include <iostream>
#include <thread>
#include <string>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <mutex>
#include "gol.h"


namespace GoL
{


	//------------------------------------------------------------------------------
	// Class name: CMarker
	// This is an object on field for changing lifestate of selected state.
	//------------------------------------------------------------------------------

	CMarker::CMarker()
	{
		m_x = 0;
		m_y = 0;
	}

	void CMarker::MoveMarker(Direction_t dir)	//This method moves marker towards 'dir' direction.
	{
		switch (dir)
		{
		case Direction_t::UP:
			m_y--;
			break;
		case Direction_t::DOWN:
			m_y++;
			break;
		case Direction_t::LEFT:
			m_x--;
			break;
		case Direction_t::RIGHT:
			m_x++;
			break;
		}
	}

	void CMarker::GetMarkerPos(int& _x, int& _y)
	{
		_x = m_x;
		_y = m_y;
	}

	//------------------------------------------------------------------------------
	// Class name: CCell
	// Single life cell on field.
	//------------------------------------------------------------------------------

	CCell::CCell()
	{
		m_alive = false;
	}

	bool CCell::IsAlive() const
	{
		return m_alive;
	}

	CCell::operator bool() const
	{
		return m_alive;
	}

	void CCell::SetLifeState(bool is_Alive)
	{
		m_alive = is_Alive;
	}

	void CCell::InvertLife()	//Set opposite life state
	{
		m_alive = !m_alive;
	}

	bool CCell::GetNextLifeState(int neighborCount)		//Actual rules of the Game of Life. Returns life state of next generation cell.
	{
		if (m_alive)
		{
			if (neighborCount < 2 || neighborCount > 3)	//Live cell with lesser than 2 or higher than 3 neighbors will die.
				return false;
			else
				return true;
		}
		else
		{
			if (neighborCount == 3)	//Dead cell with exactly 3 neighbors will become alive.
				return true;
			else
				return false;
		}
	}

	//------------------------------------------------------------------------------
	// Class name: CField
	// Main class of this game. Contains matrix of cells and inherited from CMarker.
	//------------------------------------------------------------------------------

	CField::CField(size_t w, size_t h)
	{
		m_field = new CCell*[h];
		for (size_t i = 0; i < h; i++)
			m_field[i] = new CCell[w];
		m_width = w;
		m_height = h;
		m_paused = true;
		m_waitInterval = 100;	//Speed is 100ms by default
	}

	CField::~CField()
	{
		_destroy();
	}

	void CField::_destroy()	//Destroy cell memory container
	{
		for (size_t i = 0; i < m_height; i++)
			delete[] m_field[i];
		delete[] m_field;
	}

	int CField::GetAliveNeighborCount(size_t x, size_t y) const		//Count neighbors of specific cell
	{
		int count = 0;
		int start_x = x - 1, start_y = y - 1;	//Set beginning point of count as cell in top left corner

												//If start pos is lesser than 0 - move it to other side of field (to cycle it)
		if (start_x < 0)
			start_x = m_width - 1;

		if (start_y < 0)
			start_y = m_height - 1;

		int n = 0, m = 0;		  //Variables for iterations limit (i and j could become zero in some cases, so we use another vars to count iterations).
		for (size_t i = start_y; n < 3; i++)	//Height
		{
			if (i >= m_height)	  //If y is higher than field height: move to zero
				i = 0;
			for (size_t j = start_x; m < 3; j++)	//Width
			{
				if (j >= m_width) //If x is higher than field width: move to zero
					j = 0;
				if (m_field[i][j].IsAlive() && (i != y || j != x))	//If alive neighbor cell found and this cell is not current cell
					count++;	 //Increase counter if alive cell found in square around (exepting current cell)
				m++;
			}
			n++; m = 0;
		}

		return count;
	}

	void CField::Print() const	//Output of game field
	{
		m_mutex.lock();

		std::string str("");

		for (size_t i = 0; i < m_height; i++)
		{
			for (size_t j = 0; j < m_width; j++)
			{
				if (i == m_y && j == m_x && m_paused)	//Draw marker when paused.
					str.push_back('#');
				else if (m_field[i][j].IsAlive())		//Draw alive cell as 'O' letter.
					str.push_back('O');
				else
					str.push_back(' ');					//Dead cells is space.
			}
			str += "|\n";		//Add vertical frame
		}

		for (size_t k = 0; k < m_width; k++)
			str.push_back('_');	//Add horizontal frame
		str.push_back('\n');

		//Display additional info
		str += "State: ";

		if (m_paused)
			str += "Paused\t";
		else
			str += "Running\t";

		char tmpbuf[64];
		sprintf_s(tmpbuf, "(%i, %i)\t", m_x, m_y);		//Save integer values to temp. buffer
		str += "Marker pos: " + std::string(tmpbuf);	//Append buffer to output string

		sprintf_s(tmpbuf, "%ims\n", m_waitInterval);
		str += "Update period(ms): " + std::string(tmpbuf);

		std::cout << str;	//Draw the game
		m_mutex.unlock();
	}

	bool CField::IsPaused() const
	{
		return m_paused;
	}

	int CField::GetWaitInterval() const
	{
		return m_waitInterval;
	}

	void nextgenThreadRoutine(CField* obj, CField::CellMatrix_t newField, size_t from, size_t to)	//Thread working function.
	{
		for (size_t i = from; i <= to; i++)
		{
			for (size_t j = 0; j < obj->m_width; j++)
				newField[i][j].SetLifeState(obj->m_field[i][j].GetNextLifeState(obj->GetAliveNeighborCount(j, i)));	//Set life state of cell in new grid as next life state of same cell in old grid
		}
	}

	void CField::NextGen(size_t threadCount)	//Makes next generation of life
	{
		m_mutex.lock();
		CellMatrix_t nextGenField;		//Allocate new empty field for new life generation.
		nextGenField = new CCell*[m_height];
		for (size_t i = 0; i < m_height; i++)
			nextGenField[i] = new CCell[m_width];

		if (threadCount < 1)		//Minumum thread count is always 1
			threadCount = 1;
		if (threadCount > m_height)	//Maximum thread count must equal field height.
			threadCount = m_height;

		size_t rows_per_thread = m_height / threadCount;	//Get average row count per thread
		size_t tt, t = 0;	//t is 'from' index, tt is 'to' index.
		std::thread* threads = new std::thread[threadCount];	//Create threads array

		for (size_t i = 0; i < threadCount; i++)
		{
			tt = t + rows_per_thread;
			if (i == threadCount - 1)	//If last index - set last index of last thread to maximum.
				tt = m_height - 1;
			threads[i] = std::thread(nextgenThreadRoutine, this, nextGenField, t, tt);	//Start thread. Passing this object, nextGen field and range of indexes.
			t = tt + 1;
		}

		for (size_t i = 0; i < threadCount; i++)	//Wait untill all threads will finish work.
			threads[i].join();

		delete[] threads;	//Clean-up memory

		_destroy(); //Remove old generation
		m_field = nextGenField;	//Save next generation to field member.
		m_mutex.unlock();
	}

	void CField::MarkCell()	//Mark cell on marker point
	{
		m_mutex.lock();
		if (m_paused)
			m_field[m_y][m_x].InvertLife();
		m_mutex.unlock();
	}

	void CField::Pause()	//Invert pause state of life generation
	{
		m_mutex.lock();
		m_paused = !m_paused;
		m_mutex.unlock();
	}

	void CField::Pause(bool pause)	//Set or unset pause of life generation
	{
		m_mutex.lock();
		m_paused = pause;
		m_mutex.unlock();
	}

	void CField::SpeedUp()		//Increase generation speed
	{
		m_mutex.lock();
		m_waitInterval -= 10;

		if (m_waitInterval < 10)
			m_waitInterval = 10;
		m_mutex.unlock();
	}

	void CField::SlowDown()		//Decrease generation speed
	{
		m_mutex.lock();
		m_waitInterval += 10;

		if (m_waitInterval > 1000)
			m_waitInterval = 1000;
		m_mutex.unlock();
	}

	void CField::_reset(size_t w, size_t h)	//Reset field cells
	{
		_destroy();
		m_field = new CCell*[h];
		for (size_t i = 0; i < h; i++)
			m_field[i] = new CCell[w];
		m_width = w;
		m_height = h;
	}

	void CField::SaveToTheFile(const char* fileName)
	{
		m_mutex.lock();
		if (!m_paused)	//Do not save file when processing life
		{
			m_mutex.unlock();
			return;
		}

		std::ofstream fs;	//Create file and clear it if exists.
		fs.open(fileName, std::ios::out | std::ios::trunc);

		//Filling file with data
		fs << m_width << '|' << m_height << ':';	

		for (size_t y = 0; y < m_height; y++)
		{
			for (size_t x = 0; x < m_width; x++)
				fs << bool(m_field[y][x]);
		}

		fs.close();	//Release the file
		m_mutex.unlock();
	}

	bool fileExists(const char *fileName)	//Checking existance of file
	{
		std::ifstream infile(fileName);
		return infile.good();
	}

	bool CharToBool(char c)	//Converts chars from string to bool
	{
		if (c == '0')
			return false;
		else
			return true;
	}

	void CField::LoadFromFile(const char* fileName)
	{
		m_mutex.lock();
		if (!m_paused) //Do not read file when processing life
		{
			m_mutex.unlock();
			return;
		}

		std::ifstream fs;	//File stream object
		std::string fstr;	//Buffer for entire file
		std::stringstream ss;	//String stream for easy data conversion
		size_t h, w;	//Future height and width of field
		

		try
		{
			if (!fileExists(fileName))	//Check if file exists
				throw 1;				//Throw exeption if not

			fs.open(fileName, std::ios::in);	//Open file for reading
			fstr = std::string((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());	//Read file to string

			size_t a, b;
			a = fstr.find("|");	//Search for | delimiter
			if (a == std::string::npos)	//Throw exeption if not found
				throw 2;

			ss << fstr.substr(0, a);	//Get width
			ss >> w;	//Convert to integer
			ss.clear();	//Clean ss buffer

			b = fstr.find(":");	//Search for : delimiter
			if (b == std::string::npos) //Throw exeption if not found
				throw 3;

			ss << fstr.substr(a + 1, b - (a + 1));	//Get height
			ss >> h; //Convert to integer

			std::string cells = fstr.substr(b + 1, fstr.length() - (b + 1));	//Get cells buffer

			if (cells.length() != h*w)	//Check if number of cells in file matches their h and w
				throw 4;				//Throw exeption it not.

			_reset(w, h);				//Reset field matrix.
			size_t k = 0;
			for (size_t i = 0; i < h; i++)
			{
				for (size_t j = 0; j < w; j++)
				{
					m_field[i][j].SetLifeState(CharToBool(cells[k]));	//Write cells from cell buffer to field matrix.
					k++;
				}
			}

		}

		catch (int)	//Release mutex and file, interrupt function on any exeption.
		{
			fs.close(); 
			m_mutex.unlock();
			return;
		}


		fs.close(); //Release the file
		m_mutex.unlock();
	}
}