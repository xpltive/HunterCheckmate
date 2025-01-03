#include "ThpPlayerProfile.h"

namespace HunterCheckmate_FileAnalyzer
{
	PPWeaponData::PPWeaponData(uint32_t equipment_hash, uint32_t purchase_timestamp, uint32_t shots_fired,
		uint32_t shots_missed, uint32_t total_kills, uint32_t total_one_shot_kills, float record_kill_score,
		uint32_t record_kill_species, float furthest_vital_organ_shot)
	{
		this->equipment_hash = equipment_hash;
		this->purchase_timestamp = purchase_timestamp;
		this->shots_fired = shots_fired;
		this->shots_missed = shots_missed;
		this->total_kills = total_kills;
		this->total_one_shot_kills = total_one_shot_kills;
		this->record_kill_score = record_kill_score;
		this->record_kill_species = record_kill_species;
		this->furthest_vital_organ_shot = furthest_vital_organ_shot;
	}

	ThpPlayerProfile::ThpPlayerProfile(std::shared_ptr<FileHandler> file_handler, std::shared_ptr<FileHandler> json_handler)
		: AdfFile(std::move(file_handler)), m_json_handler(std::move(json_handler)) {}

	void ThpPlayerProfile::Test()
	{
		// 0x00007730 swap 0x00007738
		inventory_slot = GetInventorySlot();

		uint32_t fst = inventory_slot.at(9).at(0);
		uint32_t snd = inventory_slot.at(10).at(0);

		std::vector<char> buffer_fst(sizeof(uint32_t));
		std::memcpy(buffer_fst.data(), &fst, sizeof(fst));

		std::vector<char> buffer_snd(sizeof(uint32_t));
		std::memcpy(buffer_snd.data(), &snd, sizeof(snd));

		m_file_handler->write(&buffer_fst, 0x00007738, buffer_fst.size());
		m_file_handler->write(&buffer_snd, 0x00007730, buffer_snd.size());
	}

	bool ThpPlayerProfile::SerializeJson()
	{
		this->equipment_back_pack = GetEquipmentBackPack();
		this->inventory_slot = GetInventorySlot();

		json::object loadouts;

		json::object json_equipment_back_pack;
		json::array arr_eq;
		auto it_beg_eq = equipment_back_pack.begin();
		auto it_end_eq = equipment_back_pack.end();
		for (; it_beg_eq != it_end_eq; ++it_beg_eq)
			arr_eq.emplace_back(*it_beg_eq);

		json::object json_inventory_slots;
		json::object json_inventory_slot;
		auto it_beg_in = inventory_slot.begin();
		auto it_end_in = inventory_slot.end();

		std::string slotname = "slot";
		int slotcounter = 1;
		for (; it_beg_in != it_end_in; ++it_beg_in)
		{
			auto it_beg_in_deep = it_beg_in->begin();
			auto it_end_in_deep = it_beg_in->end();
			std::string sub_slotname = "subslot";
			int sub_slotcounter = 1;
			for(; it_beg_in_deep != it_end_in_deep; ++it_beg_in_deep)
			{
				sub_slotname = sub_slotname + std::to_string(sub_slotcounter);
				json_inventory_slot.emplace(sub_slotname, *it_beg_in_deep);
				sub_slotcounter++;
			}

			slotname = slotname + std::to_string(slotcounter);
			json_inventory_slots.emplace(slotname, json_inventory_slot);
			slotcounter++;
		}

		json::object json_backpack_inventory;
		json_backpack_inventory.emplace("equipment_back_pack", arr_eq);
		json_backpack_inventory.emplace("invetory_slot", json_inventory_slots);
		loadouts.emplace("loadout1", json_backpack_inventory);

		std::string yeyeye = json::serialize(loadouts);

		m_json_handler->write_json(yeyeye);
		
		return true;
	}

	bool ThpPlayerProfile::DeserializeJson()
	{

		return true;
	}

	uint8_t ThpPlayerProfile::GetAlreadyConverted()
	{
		std::vector<char> data = instances.at(0).members.at(0).data;
		return *reinterpret_cast<uint8_t*>(data.data());
	}

	uint8_t ThpPlayerProfile::GetIsSaveGameAvailable() const
	{
		std::vector<char> data = this->instances.at(0).members.at(1).data;
		return *reinterpret_cast<uint8_t*>(data.data());
	}

	uint8_t ThpPlayerProfile::GetGender() const
	{
		std::vector<char> data = this->instances.at(0).members.at(2).data;
		return *reinterpret_cast<uint8_t*>(data.data());
	}

	uint32_t ThpPlayerProfile::GetSkinToneHash() const
	{
		std::vector<char> data = this->instances.at(0).members.at(3).data;
		return *reinterpret_cast<uint32_t*>(data.data());
	}

	uint32_t ThpPlayerProfile::GetOutfitVariantHash() const
	{
		std::vector<char> data = this->instances.at(0).members.at(4).data;
		return *reinterpret_cast<uint32_t*>(data.data());
	}

	uint32_t ThpPlayerProfile::GetLevel() const
	{
		std::vector<char> data = this->instances.at(0).members.at(5).data;
		return *reinterpret_cast<uint32_t*>(data.data());
	}

	uint32_t ThpPlayerProfile::GetXP() const
	{
		std::vector<char> data = this->instances.at(0).members.at(6).data;
		return *reinterpret_cast<uint32_t*>(data.data());
	}

	uint32_t ThpPlayerProfile::GetSkillPoints() const
	{
		std::vector<char> data = this->instances.at(0).members.at(7).data;
		return *reinterpret_cast<uint32_t*>(data.data());
	}

	uint32_t ThpPlayerProfile::GetPerkPoints() const
	{
		std::vector<char> data = this->instances.at(0).members.at(8).data;
		return *reinterpret_cast<uint32_t*>(data.data());
	}

	std::vector<uint32_t> ThpPlayerProfile::GetSkillPointsSpent() const
	{
		std::vector<uint32_t> buffer(16);
		std::vector<Member> instance_members = this->instances.at(0).members.at(9).sub_members;
		for (auto it = instance_members.begin(); it != instance_members.end(); ++it)
		{
			uint32_t idx = it - instance_members.begin();
			buffer.at(idx) = *reinterpret_cast<uint32_t*>(it->data.data());
		}
		return buffer;
	}

	std::vector<uint32_t> ThpPlayerProfile::GetPerkPointsSpent() const
	{
		std::vector<uint32_t> buffer(16);
		std::vector<Member> instance_members = this->instances.at(0).members.at(10).sub_members;
		for (auto it = instance_members.begin(); it != instance_members.end(); ++it)
		{
			uint32_t idx = it - instance_members.begin();
			buffer.at(idx) = *reinterpret_cast<uint32_t*>(it->data.data());
		}
		return buffer;
	}

	uint32_t ThpPlayerProfile::GetCash() const
	{
		std::vector<char> data = this->instances.at(0).members.at(11).data;
		return *reinterpret_cast<uint32_t*>(data.data());
	}

	std::vector<uint32_t> ThpPlayerProfile::GetWeaponLevel() const
	{
		std::vector<uint32_t> buffer(4);
		std::vector<Member> instance_members = this->instances.at(0).members.at(12).sub_members;
		for (auto it = instance_members.begin(); it != instance_members.end(); ++it)
		{
			uint32_t idx = it - instance_members.begin();
			buffer.at(idx) = *reinterpret_cast<uint32_t*>(it->data.data());
		}
		return buffer;
	}

	std::vector<uint32_t> ThpPlayerProfile::GetWeaponScore() const
	{
		std::vector<uint32_t> buffer(4);
		std::vector<Member> instance_members = this->instances.at(0).members.at(13).sub_members;
		for (auto it = instance_members.begin(); it != instance_members.end(); ++it)
		{
			uint32_t idx = it - instance_members.begin();
			buffer.at(idx) = *reinterpret_cast<uint32_t*>(it->data.data());
		}
		return buffer;
	}

	std::vector<uint32_t> ThpPlayerProfile::GetEquipmentUnlocked() const
	{
		std::vector<uint32_t> buffer(512);
		std::vector<Member> instance_members = this->instances.at(0).members.at(14).sub_members;
		for (auto it = instance_members.begin(); it != instance_members.end(); ++it)
		{
			uint32_t idx = it - instance_members.begin();
			buffer.at(idx) = *reinterpret_cast<uint32_t*>(it->data.data());
		}
		return buffer;
	}

	std::vector<uint32_t> ThpPlayerProfile::GetEquipmentOwned() const
	{
		std::vector<uint32_t> buffer(512);
		std::vector<Member> instance_members = this->instances.at(0).members.at(15).sub_members;
		for (auto it = instance_members.begin(); it != instance_members.end(); ++it)
		{
			uint32_t idx = it - instance_members.begin();
			buffer.at(idx) = *reinterpret_cast<uint32_t*>(it->data.data());
		}
		return buffer;
	}

	std::vector<uint32_t> ThpPlayerProfile::GetEquipmentAmount() const
	{
		std::vector<uint32_t> buffer(512);
		std::vector<Member> instance_members = this->instances.at(0).members.at(16).sub_members;
		for (auto it = instance_members.begin(); it != instance_members.end(); ++it)
		{
			uint32_t idx = it - instance_members.begin();
			buffer.at(idx) = *reinterpret_cast<uint32_t*>(it->data.data());
		}
		return buffer;
	}

	std::vector<uint32_t> ThpPlayerProfile::GetEquipmentBackPack() const
	{
		std::vector<uint32_t> buffer(512);
		std::vector<Member> instance_members = this->instances.at(0).members.at(17).sub_members;
		for (auto it = instance_members.begin(); it != instance_members.end(); ++it)
		{
			uint32_t idx = it - instance_members.begin();
			buffer.at(idx) = *reinterpret_cast<uint32_t*>(it->data.data());
		}
		return buffer;
	}

	std::vector<uint32_t> ThpPlayerProfile::GetSkills() const
	{
		std::vector<uint32_t> buffer(128);
		std::vector<Member> instance_members = this->instances.at(0).members.at(18).sub_members;
		for (auto it = instance_members.begin(); it != instance_members.end(); ++it)
		{
			uint32_t idx = it - instance_members.begin();
			buffer.at(idx) = *reinterpret_cast<uint32_t*>(it->data.data());
		}
		return buffer;
	}

	std::vector<uint32_t> ThpPlayerProfile::GetSkillLevel() const
	{
		std::vector<uint32_t> buffer(128);
		std::vector<Member> instance_members = this->instances.at(0).members.at(19).sub_members;
		for (auto it = instance_members.begin(); it != instance_members.end(); ++it)
		{
			uint32_t idx = it - instance_members.begin();
			buffer.at(idx) = *reinterpret_cast<uint32_t*>(it->data.data());
		}
		return buffer;
	}

	uint32_t ThpPlayerProfile::GetActiveSkill() const
	{
		std::vector<char> data = this->instances.at(0).members.at(20).data;
		return *reinterpret_cast<uint32_t*>(data.data());
	}

	std::vector<std::vector<uint32_t>> ThpPlayerProfile::GetInventorySlot() const
	{
		std::vector<std::vector<uint32_t>> buffer(16);
		std::vector<Member> instance_members = this->instances.at(0).members.at(21).sub_members;
		for (auto it = instance_members.begin(); it != instance_members.end(); ++it)
		{
			uint32_t idx = it - instance_members.begin();
			uint32_t size = it->sub_members.at(0).sub_members.size();
			std::vector<uint32_t> arr(size);
			buffer.at(idx) = arr;
			for (uint32_t i = 0; i < arr.size(); i++)
			{
				buffer.at(idx).at(i) = *reinterpret_cast<uint32_t*>(it->sub_members.at(0).sub_members.at(i).data.data());
			}
		}
		return buffer;
	}

	uint32_t ThpPlayerProfile::GetRestingCurrentCost() const
	{
		std::vector<char> data = this->instances.at(0).members.at(22).data;
		return *reinterpret_cast<uint32_t*>(data.data());
	}

	float ThpPlayerProfile::GetRemainingTimeRestCostReduction() const
	{
		std::vector<char> data = this->instances.at(0).members.at(23).data;
		return *reinterpret_cast<float*>(data.data());
	}

	// GetVehicleData()
	
	std::vector<uint32_t> ThpPlayerProfile::GetVehicleEquipmentStorage() const
	{
		std::vector<uint32_t> buffer(512);
		std::vector<Member> instance_members = this->instances.at(0).members.at(25).sub_members;
		for (auto it = instance_members.begin(); it != instance_members.end(); ++it)
		{
			uint32_t idx = it - instance_members.begin();
			buffer.at(idx) = *reinterpret_cast<uint32_t*>(it->data.data());
		}
		return buffer;
	}

	uint8_t ThpPlayerProfile::GetVehiclePreferedCameraThirdPerson() const
	{
		std::vector<char> data = this->instances.at(0).members.at(26).data;
		return *reinterpret_cast<uint8_t*>(data.data());
	}

	std::vector<uint32_t> ThpPlayerProfile::GetGivenDLCItems() const
	{
		std::vector<uint32_t> buffer(11);
		std::vector<Member> instance_members = this->instances.at(0).members.at(27).sub_members;
		for (auto it = instance_members.begin(); it != instance_members.end(); ++it)
		{
			uint32_t idx = it - instance_members.begin();
			buffer.at(idx) = *reinterpret_cast<uint32_t*>(it->data.data());
		}
		return buffer;
	}

	uint32_t ThpPlayerProfile::GetEquippedBackpackHash() const
	{
		std::vector<char> data = this->instances.at(0).members.at(28).data;
		return *reinterpret_cast<uint32_t*>(data.data());
	}

	std::vector<uint32_t> ThpPlayerProfile::GetUnlockedHuntClubEquipment() const
	{
		std::vector<uint32_t> buffer(2);
		std::vector<Member> instance_members = this->instances.at(0).members.at(29).sub_members;
		for (auto it = instance_members.begin(); it != instance_members.end(); ++it)
		{
			uint32_t idx = it - instance_members.begin();
			buffer.at(idx) = *reinterpret_cast<uint32_t*>(it->data.data());
		}
		return buffer;
	}

	uint8_t ThpPlayerProfile::GetHuntClubDisabled() const
	{
		std::vector<char> data = this->instances.at(0).members.at(30).data;
		return *reinterpret_cast<uint8_t*>(data.data());
	}

	float ThpPlayerProfile::GetAnimalSensesMultiplier() const
	{
		std::vector<char> data = this->instances.at(0).members.at(31).data;
		return *reinterpret_cast<float*>(data.data());
	}

	// GetModifiedLockModes()
	
	uint8_t ThpPlayerProfile::GetProfileIsNew() const
	{
		std::vector<char> data = this->instances.at(0).members.at(33).data;
		return *reinterpret_cast<uint8_t*>(data.data());
	}

	std::vector<PPWeaponData> ThpPlayerProfile::GetWeaponData() const
	{
		std::vector<PPWeaponData> buffer(512);
		std::vector<Member> instance_members = this->instances.at(0).members.at(34).sub_members;
		for (auto it = instance_members.begin(); it != instance_members.end(); ++it)
		{
			uint32_t idx = it - instance_members.begin();
			PPWeaponData *pp_weapon_data = new PPWeaponData(
				*reinterpret_cast<uint32_t*>(it->sub_members.at(0).data.data()),
				*reinterpret_cast<uint32_t*>(it->sub_members.at(1).data.data()),
				*reinterpret_cast<uint32_t*>(it->sub_members.at(2).data.data()),
				*reinterpret_cast<uint32_t*>(it->sub_members.at(3).data.data()),
				*reinterpret_cast<uint32_t*>(it->sub_members.at(4).data.data()),
				*reinterpret_cast<uint32_t*>(it->sub_members.at(5).data.data()),
				*reinterpret_cast<float*>(it->sub_members.at(6).data.data()),
				*reinterpret_cast<uint32_t*>(it->sub_members.at(7).data.data()),
				*reinterpret_cast<float*>(it->sub_members.at(8).data.data())
			);
			buffer.at(idx) = *pp_weapon_data;
			delete pp_weapon_data;
		}
		return buffer;
	}

	// GetShownHuntClubOptIn()
	// GetHuntClubHash()
	// GetApexConnectAccountDismissed()
}
