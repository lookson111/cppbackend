#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <iostream>
namespace model {
	class ExtraData {
	public:
		void SetLootTypes(const std::string &mapID, const std::string &loot_types, int cnt_loot_types) {
            std::cout << "MAP id " << mapID << std::endl; 
            std::cout << "loot_type  " << loot_types << std::endl; 
			loot_types_in_map_.emplace(std::move(mapID) , std::make_pair(std::move(loot_types), cnt_loot_types));
		}
		std::string_view GetLootTypes(const std::string& mapID) const { 
            std::cout << "MAP id " << mapID << std::endl; 
			auto it = loot_types_in_map_.find(mapID);
            if (it ==  loot_types_in_map_.end())
                throw std::logic_error("GetLoot map not found");
			return it->second.first;
		}
		int GetCntLootTypes(const std::string& mapID) const {
            std::cout << "MAP id " << mapID << std::endl; 
			auto it = loot_types_in_map_.find(mapID);
            if (it ==  loot_types_in_map_.end())
                throw std::logic_error("GetCntLoot map not found");
			return it->second.second;
		}
	private:
		std::map<std::string, std::pair<std::string, int>> loot_types_in_map_;
	};
}
