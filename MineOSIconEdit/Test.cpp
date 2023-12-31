#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <deque>
#include <vector>
#include <cstdlib>
#include <ctime>

//===========================================

const std::vector<std::string> FirstWords = {
	"Pidor",
	"Uebok",
	"Pizda",
	"Vagina",
	"Zalupa",
	"Parasha"
};

const std::vector<std::string> LastWords = {
	"Govna",
	"Mochi",
	"Pizdy",
	"Zalupy",
	"Pidora",
	"Uebka",
	"Vaginy",
	"Spermy"
};

const std::vector<std::string> Extensions = {
	"txt",
	"png",
	"pic",
	"gif",
	"zip",
	"rar",
	"iso",
	"3dm",
	"als",
	"bpm",
	"webp",
	"webm",
	"cpp",
	"html",
	"hpp"
};

std::filesystem::path RecentFilesListSaveLocation = "recent.txt";
std::deque<std::filesystem::path> RecentFilesList;

std::filesystem::path MakeRandomFilename();
void LoadRecentFilesList();
void SaveRecentFilesList();
void PrintRecentFilesList();

//===========================================

int main()
{
	srand(time(0));

	LoadRecentFilesList();
	RecentFilesList.push_front(MakeRandomFilename());
	PrintRecentFilesList();
	SaveRecentFilesList();
}

//===========================================

std::filesystem::path MakeRandomFilename()
{
	return FirstWords[rand() % FirstWords.size()] + LastWords[rand() % LastWords.size()] + "." + Extensions[rand() % Extensions.size()];
}

void LoadRecentFilesList()
{
	std::ifstream stream(RecentFilesListSaveLocation, std::ios::in);
	if (stream.good())
	{
		RecentFilesList.clear();

		std::string line;
		while (std::getline(stream, line))
			RecentFilesList.push_front(line);
	}
}

void SaveRecentFilesList()
{
	std::ofstream stream(RecentFilesListSaveLocation, std::ios::out);
	if (!stream.good())
	{
		std::cerr << "Unable to save recent files list" << std::endl;
		return;
	}

	for (const auto& path: RecentFilesList)
		stream << path.string() << std::endl;
}

void PrintRecentFilesList()
{
	if (RecentFilesList.empty())
	{
		std::cout << "Recent files list is empty" << std::endl;
		return;
	}

	std::cout << "Recent files:" << std::endl;
	for (const auto& path: RecentFilesList)
		std::cout << '\t' << path << std::endl;

	std::cout << std::endl;
}

//===========================================