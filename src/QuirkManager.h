#pragma once  
#include "Singleton.h"  
#include <string>
#include <vector>

namespace Chip8  
{  
	struct Quirks  
	{  
		bool shiftQuirk;  
		bool loadStoreQuirkIncrement;  
		bool loadStoreQuirkUnchanged;  
		bool wrapQuirk;  
		bool jumpQuirk;  
		bool vblankQuirk;  
		bool vFResetQuirk;
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

		bool GetLoadStoreQuirkIncrement() const { return m_Quirks.loadStoreQuirkIncrement; }
		void SetLoadStoreQuirkIncrement(bool value) { m_Quirks.loadStoreQuirkIncrement = value; }

		bool GetLoadStoreQuirkUnchanged() const { return m_Quirks.loadStoreQuirkUnchanged; }
		void SetLoadStoreQuirkUnchanged(bool value) { m_Quirks.loadStoreQuirkUnchanged = value; }
		
		bool GetWrapQuirk() const { return m_Quirks.wrapQuirk; }
		void SetWrapQuirk(bool value) { m_Quirks.wrapQuirk = value; }
		
		bool GetJumpQuirk() const { return m_Quirks.jumpQuirk; }
		void SetJumpQuirk(bool value) { m_Quirks.jumpQuirk = value; }
		
		bool GetVblankQuirk() const { return m_Quirks.vblankQuirk; }
		void SetVblankQuirk(bool value) { m_Quirks.vblankQuirk = value; }

		bool GetVfResetQuirk() const { return m_Quirks.vFResetQuirk; }
		void SetVfResetQuirk(bool value) { m_Quirks.vFResetQuirk = value; }
	private:
		Chip8::GameInfo CreateGameInfo(std::string path, int gameID);

		Quirks m_Quirks{};
		GameInfo m_GameInfo{};
	};  
}
