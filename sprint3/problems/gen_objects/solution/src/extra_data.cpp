#include "extra_data.h"

namespace model {
void ExtraData::SetLootTypes(const std::string& mapID, const std::string& loot_types, int cnt_loot_types)
{
	//std::cout << "MAP id \'" << mapID << "\'" << std::endl;
	//std::cout << "loot_type  " << loot_types << std::endl;
	loot_types_in_map_[mapID] = std::make_pair(loot_types, cnt_loot_types);
}
std::string_view ExtraData::GetLootTypes(const std::string& mapID) const {
	//std::cout << "MAP id \'" << mapID << "\'" << std::endl;
	//for (auto loott : loot_types_in_map_) {
	//	std::cout << loott.first << std::endl;
	//}
	auto it = loot_types_in_map_.find(mapID);
	if (it == loot_types_in_map_.end())
		throw std::logic_error("GetLoot map not found");
	return it->second.first;
}
int ExtraData::GetCntLootTypes(const std::string& mapID) const {
	//std::cout << "MAP id \'" << mapID << "\'" << std::endl;
	auto it = loot_types_in_map_.find(mapID);
	if (it == loot_types_in_map_.end())
		throw std::logic_error("GetCntLoot map not found");
	return it->second.second;
}
}