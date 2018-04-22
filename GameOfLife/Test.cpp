#include <fstream>
#include <string>
#include <streambuf>
#include <sstream>
#include <assert.h>
#include "gol.h"
#include "Test.h"

namespace Tests
{
	bool fileExists(const char *fileName)
	{
		std::ifstream infile(fileName);
		return infile.good();
	}

	void CTestSavedGameFile::Test(const char* fileName, const GoL::CField* pField)
	{
		assert(fileExists(fileName));	//Check if file exists

		std::ifstream fs;
		fs.open(fileName, std::ios::in);	//Open file for reading
		std::string fstr((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());	//Read file to string

		std::ostringstream oss;	//Create string stream buffer
		oss << pField->m_width << '|' << pField->m_height << ':';	//Re-create correct file buffer from CField in string stream buffer.

		for (size_t y = 0; y < pField->m_height; y++)
		{
			for (size_t x = 0; x < pField->m_width; x++)
				oss << bool(pField->m_field[y][x]);
		}

		assert(oss.str() == fstr);	//Check if file content matches correct sample.
		
	}
}