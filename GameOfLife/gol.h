#pragma once
#include <mutex>

namespace Tests
{
	class CTestSavedGameFile;
}

namespace GoL		//Packing all classes to Game of Life namespace.
{
	class CMarker
	{
	public:
		enum Direction_t	//Definitions of marker movement directions
		{
			UP,
			DOWN,
			LEFT,
			RIGHT
		};

	protected:
		int m_x;	//Marker current coordinates
		int m_y;	

	public:
		CMarker();
		void MoveMarker(Direction_t dir);		//Move marker in choosen direction.
		void GetMarkerPos(int& _x, int& _y);	//Get current coordinates of marker.
	};

	class CCell
	{
	protected:
		bool m_alive;	//Life state of cell
	public:
		CCell();
		bool IsAlive() const;	//Get life state
		operator bool() const; //Cast CCell objects as bool when necessary

		void SetLifeState(bool is_Alive);	//Set life state to specific
		void InvertLife();					//Set life state to opposite
		bool GetNextLifeState(int ncount);	//Get life of this cell in next generation (depending on it's neighbor count)

	};

	class CField : public CMarker
	{
	public:
		typedef CCell** CellMatrix_t;	//Defining cell matrix type for convenient usage
	private:
		friend class Tests::CTestSavedGameFile;		//Allow test access the class members

	protected:
		CellMatrix_t m_field;	//Field of cells
		size_t m_width;		//Dims of field
		size_t m_height;
		unsigned int m_waitInterval;	//Update perion interval in milliseconds
		bool m_paused;		//Game state. Paused or not.
		mutable std::mutex m_mutex;	//Synchronization object for thread-safe working. This object must be able to change it's state even in const methods.

		//Internal methods. For resetting cell matrix and destroying it.
		void _reset(size_t w, size_t h);	
		void _destroy();
		
		int GetAliveNeighborCount(size_t x, size_t y) const;	//Get count of alive neighbor cells for cell by coordinates. Also internal method.
		
	public:

		CField(size_t w, size_t h);	//There is no default constructor 
		~CField();

		void Print() const;		//Print entire field to console and some additional information. 
		bool IsPaused() const;	//Check game state (Paused or not)
		int GetWaitInterval() const;	//Get waiting time period

		void NextGen(size_t threadCount = 1);		//Go to next life generation. threadCount - number of threads for life processing.
		void MarkCell();	//Change state of cell to opposite in marker's place. Works only in paused state.
		void Pause();		//Pause or resume game.
		void Pause(bool pause);	//Set specific game state.

		void SpeedUp();		//Increases game speed
		void SlowDown();	//Decreases game speed

		void SaveToTheFile(const char* fileName);	//Save current field to the file. Works only in paused state.
		void LoadFromFile(const char* fileName);	//Load cell matrix from file. Works only in paused state.

		friend void nextgenThreadRoutine(const CField* obj, CellMatrix_t newField, size_t from, size_t to);	//Multithread generation processing function must have access to CField members.
	};


}