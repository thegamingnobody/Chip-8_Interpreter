#pragma once  
#include "Singleton.h"  
#include <string>
#include <vector>

namespace Chip8  
{  
	struct Quirks  
	{  
		bool shiftQuirk;  
		bool loadStoreQuirk;  
		bool wrapQuirk;  
		bool jumpQuirk;  
		bool vblankQuirk;  
	};  

	struct Rom  
	{  
		std::string hash{};
		std::string file{};
		std::vector<std::string> platforms{};
		std::string embeddedTitle{};
		std::vector<int> keys{};

		Rom() = default;

		Rom(const std::string& hash, const std::string& file, const std::vector<std::string>& platforms,  
			const std::string& embeddedTitle, const std::vector<int>& keys)  
			: hash(hash)
			, file(file)
			, platforms(platforms)
			, embeddedTitle(embeddedTitle)
			, keys(keys) 
		{  
		}  
	};  

	struct GameInfo  
	{  
		std::string title{};
		std::string description{};
		std::vector<std::string> authors{};
		std::string date{};
		std::vector<Rom> roms{};

		GameInfo() = default;

		GameInfo(const std::string& title, const std::string& description, const std::vector<std::string>& authors,  
			const std::string& date, const std::vector<Rom>& roms)  
			: title(title)
			, description(description)
			, authors(authors)
			, date(date)
			, roms(roms) 
		{  
		}  
	};  

	class QuirkManager : public Singleton<QuirkManager>
	{  
	public:  
		void LoadGameQuirks(const std::string& hash);  

		GameInfo GetGameInfo() const { return m_GameInfo; }

        Quirks& GetQuirks() { return m_Quirks; }
		void SetQuirks(const Quirks& quirks) { m_Quirks = quirks; }

		bool GetShiftQuirk() const { return m_Quirks.shiftQuirk; }
		void SetShiftQuirk(bool value) { m_Quirks.shiftQuirk = value; }

		bool GetLoadStoreQuirk() const { return m_Quirks.loadStoreQuirk; }
		void SetLoadStoreQuirk(bool value) { m_Quirks.loadStoreQuirk = value; }
		
		bool GetWrapQuirk() const { return m_Quirks.wrapQuirk; }
		void SetWrapQuirk(bool value) { m_Quirks.wrapQuirk = value; }
		
		bool GetJumpQuirk() const { return m_Quirks.jumpQuirk; }
		void SetJumpQuirk(bool value) { m_Quirks.jumpQuirk = value; }
		
		bool GetVblankQuirk() const { return m_Quirks.vblankQuirk; }
		void SetVblankQuirk(bool value) { m_Quirks.vblankQuirk = value; }

	private:
		Chip8::GameInfo CreateGameInfo(std::string path, int gameID);

		Quirks m_Quirks{};
		GameInfo m_GameInfo{};
	};  
}
