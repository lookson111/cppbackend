#pragma once

#include <map>
#include <string>

namespace model {
	class ExtraData {
	public:
		void SetLootTypes(std::string mapID, const std::string &loot_types, int cnt_loot_types) {
			loot_types_in_map_.emplace(std::move(mapID) , std::make_pair(std::move(loot_types), cnt_loot_types));
		}
		std::string_view GetLootTypes(const std::string& mapID) const { 
			auto it = loot_types_in_map_.find(mapID);
			return it->second.first;
		}
		int GetCntLootTypes(const std::string& mapID) const {
			auto it = loot_types_in_map_.find(mapID);
			return it->second.second;
		}
	private:
		std::map<std::string, std::pair<std::string, int>> loot_types_in_map_;
	};
}