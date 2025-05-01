#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "QuirkManager.h"
#include <iostream>
#include <fstream>


void Chip8::QuirkManager::LoadGameQuirks(const std::string& hash)
{
	//Todo: implement loading quirk data based on the hash
	std::string basePath{ "..\\..\\..\\src\\Quirks\\" };
	std::vector<std::string> jsonDataNames = { "sha1-hashes.json", "programs.json" };

	std::ifstream hashFile(basePath + jsonDataNames[0]);

	if (!hashFile.is_open())
	{
		std::cerr << "Failed to open file: " << basePath + jsonDataNames[0] << "\n";
		return;
	}

	json gameIdData = json::parse(hashFile);
	if (gameIdData.find(hash) == gameIdData.end())
	{
		std::cerr << "Game ID not found in the JSON file.\n";
		return;
	}
	auto gameId{ gameIdData[hash].get<int>() };

	std::cout << "Game ID: " << gameId << "\n";
	
	m_GameInfo = CreateGameInfo(basePath + jsonDataNames[1], gameId);
}

Chip8::GameInfo Chip8::QuirkManager::CreateGameInfo(std::string path, int gameID)
{
	std::ifstream programFile(path);
	if (!programFile.is_open())
	{
		std::cerr << "Failed to open file: " << path << "\n";
		return m_GameInfo;
	}

	json programData = json::parse(programFile);
	auto gameInfo = programData.at(gameID).items();

	for (auto item : gameInfo)
	{
		if (item.key() == "title")
		{
			m_GameInfo.title = item.value();
		}
		else if (item.key() == "authors")
		{
			m_GameInfo.authors = item.value().get<std::vector<std::string>>();
		}
		else if (item.key() == "description")
		{
			m_GameInfo.description = item.value();
		}
		else if (item.key() == "release")
		{
			m_GameInfo.date = item.value();
		}
		else if (item.key() == "roms")
		{
			for (auto& rom : item.value().items())
			{
				Rom newRom;
				newRom.hash = rom.key();

				newRom.file = rom.value()["file"];
				newRom.platforms = rom.value()["platforms"].get<std::vector<std::string>>();
				if (rom.value().contains("embeddedTitle") /*&& rom.value()["embeddedTitle"].is_string()*/)
				{
					newRom.embeddedTitle = rom.value()["embeddedTitle"];
				}
				//Todo: add key mapping/reading 
				//auto test = rom.value()["keys"];
				//newRom.keys = rom.value()["keys"];
				m_GameInfo.roms.push_back(newRom);
			}
		}
		else
		{
			std::cerr << "Unknown key: " << item.key() << "\n";
		}

	}

	//std::cout << "Game Title: " << programData.at(gameID)["title"] << "\n";

	return m_GameInfo;
}
