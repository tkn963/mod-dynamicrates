#include "Chat.h"
#include "Config.h"
#include "Group.h"
#include "Player.h"
#include "ScriptMgr.h"

enum DynamicRateRange
{
    DYNAMIC_RANGE_1_59  = 0,
    DYNAMIC_RANGE_60_69 = 1,
    DYNAMIC_RANGE_70_79 = 2,
    DYNAMIC_RANGE_MAX   = 3,
};

float dynamicExperienceRate[DYNAMIC_RANGE_MAX];
float dynamicReputationRate[DYNAMIC_RANGE_MAX];
float dynamicMoneyRate[DYNAMIC_RANGE_MAX];

bool dynamicRatesShowInfo;

class DynamicRates : public PlayerScript
{
public:
    DynamicRates() : PlayerScript("DynamicRates") {}

    void OnGiveXP(Player* player, uint32& amount, Unit* /*victim*/) override
    {
        uint32 playerLevel = player->getLevel();
        if (playerLevel < 60)
        {
            if (dynamicExperienceRate[DYNAMIC_RANGE_1_59] > 1.0f)
                amount = round(amount * dynamicExperienceRate[DYNAMIC_RANGE_1_59]);
        }
        else if (playerLevel < 70)
        {
            if (dynamicExperienceRate[DYNAMIC_RANGE_60_69] > 1.0f)
                amount = round(amount * dynamicExperienceRate[DYNAMIC_RANGE_60_69]);
        }
        else if (playerLevel < 80)
        {
            if (dynamicExperienceRate[DYNAMIC_RANGE_70_79] > 1.0f)
                amount = round(amount * dynamicExperienceRate[DYNAMIC_RANGE_70_79]);
        }
    }

    bool OnReputationChange(Player* player, uint32 /*factionID*/, int32& standing, bool /*incremental*/) override
    {
        uint32 playerLevel = player->getLevel();
        if (playerLevel < 60)
        {
            if (dynamicReputationRate[DYNAMIC_RANGE_1_59] > 1.0f)
                standing = round(standing * dynamicReputationRate[DYNAMIC_RANGE_1_59]);
        }
        else if (playerLevel < 70)
        {
            if (dynamicReputationRate[DYNAMIC_RANGE_60_69] > 1.0f)
                standing = round(standing * dynamicReputationRate[DYNAMIC_RANGE_60_69]);
        }
        else if (playerLevel < 80)
        {
            if (dynamicReputationRate[DYNAMIC_RANGE_70_79] > 1.0f)
                standing = round(standing * dynamicReputationRate[DYNAMIC_RANGE_70_79]);
        }

        return true;
    }

    void OnPlayerCompleteQuest(Player* player, Quest const* quest) override
    {
        uint32 playerLevel = player->getLevel();
        uint32 baseMoney = quest->GetRewOrReqMoney(playerLevel);
        uint32 rewardMoney = 0;

        if (playerLevel < 60)
        {
            if (dynamicMoneyRate[DYNAMIC_RANGE_1_59] > 1.0f)
                rewardMoney = round((baseMoney * dynamicMoneyRate[DYNAMIC_RANGE_1_59]) - baseMoney);
        }
        else if (playerLevel < 70)
        {
            if (dynamicMoneyRate[DYNAMIC_RANGE_60_69] > 1.0f)
                rewardMoney = round((baseMoney * dynamicMoneyRate[DYNAMIC_RANGE_60_69]) - baseMoney);
        }
        else if (playerLevel < 80)
        {
            if (dynamicMoneyRate[DYNAMIC_RANGE_70_79] > 1.0f)
                rewardMoney = round((baseMoney * dynamicMoneyRate[DYNAMIC_RANGE_70_79]) - baseMoney);
        }

        if (rewardMoney > 0)
        {
            player->ModifyMoney(rewardMoney);

            if (dynamicRatesShowInfo)
                SendDynamicRateInfo(player, rewardMoney);
        }
    }
private:
    void SendDynamicRateInfo(Player* player, uint32 money)
    {
        uint32 gold = money / GOLD;
        uint32 silver = (money % GOLD) / SILVER;
        uint32 copper = (money % GOLD) % SILVER;
        std::string info;

        if (money < 100)
            info = Acore::StringFormat("You receive an additional %i copper.", copper);
        else if (money < 10000)
            info = Acore::StringFormat("You receive an additional %i silver, %i copper.", silver, copper);
        else
            info = Acore::StringFormat("You receive an additional %i gold, %i silver, %i copper.", gold, silver, copper);

        ChatHandler(player->GetSession()).SendSysMessage(info);
    }
};

class DynamicRatesConfig : WorldScript
{
public:
    DynamicRatesConfig() : WorldScript("DynamicRatesConfig") {}

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        dynamicExperienceRate[DYNAMIC_RANGE_1_59] = sConfigMgr->GetOption<float>("DynamicRates.Level.1-59.Experience", 1.0f);
        dynamicReputationRate[DYNAMIC_RANGE_1_59] = sConfigMgr->GetOption<float>("DynamicRates.Level.1-59.Reputation", 1.0f);
        dynamicMoneyRate[DYNAMIC_RANGE_1_59] = sConfigMgr->GetOption<float>("DynamicRates.Level.1-59.Money", 1.0f);

        dynamicExperienceRate[DYNAMIC_RANGE_60_69] = sConfigMgr->GetOption<float>("DynamicRates.Level.60-69.Experience", 1.0f);
        dynamicReputationRate[DYNAMIC_RANGE_60_69] = sConfigMgr->GetOption<float>("DynamicRates.Level.60-69.Reputation", 1.0f);
        dynamicMoneyRate[DYNAMIC_RANGE_60_69] = sConfigMgr->GetOption<float>("DynamicRates.Level.60-69.Money", 1.0f);

        dynamicExperienceRate[DYNAMIC_RANGE_70_79] = sConfigMgr->GetOption<float>("DynamicRates.Level.70-79.Experience", 1.0f);
        dynamicReputationRate[DYNAMIC_RANGE_70_79] = sConfigMgr->GetOption<float>("DynamicRates.Level.70-79.Reputation", 1.0f);
        dynamicMoneyRate[DYNAMIC_RANGE_70_79] = sConfigMgr->GetOption<float>("DynamicRates.Level.70-79.Money", 1.0f);

        dynamicRatesShowInfo = sConfigMgr->GetOption<bool>("DynamicRates.ShowInfo", false);
    }
};

class DynamicRatesLoot : LootScript
{
public:
    DynamicRatesLoot() : LootScript("DynamicRatesLoot") {}

    void OnLootMoney(Player* player, uint32 gold) override
    {
        uint32 playerLevel = player->getLevel();
        uint32 extraGold = 0;
        if (playerLevel < 60)
        {
            if (dynamicMoneyRate[DYNAMIC_RANGE_1_59] > 1.0f)
                extraGold = round((gold * dynamicMoneyRate[DYNAMIC_RANGE_1_59]) - gold);
        }
        else if (playerLevel < 70)
        {
            if (dynamicMoneyRate[DYNAMIC_RANGE_60_69] > 1.0f)
                extraGold = round((gold * dynamicMoneyRate[DYNAMIC_RANGE_60_69]) - gold);
        }
        else if (playerLevel < 80)
        {
            if (dynamicMoneyRate[DYNAMIC_RANGE_70_79] > 1.0f)
                extraGold = round((gold * dynamicMoneyRate[DYNAMIC_RANGE_70_79]) - gold);
        }

        if (extraGold > 0)
        {
            player->ModifyMoney(extraGold);

            if (dynamicRatesShowInfo)
                SendDynamicRateInfo(player, extraGold);

            if (Group* group = player->GetGroup())
            {
                for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
                {
                    if (Player* member = groupRef->GetSource())
                    {
                        if (member->IsInMap(player) && member->IsAtLootRewardDistance(player))
                        {
                            if (member == player)
                                continue;

                            member->ModifyMoney(extraGold);

                            if (dynamicRatesShowInfo)
                                SendDynamicRateInfo(member, extraGold);
                        }
                    }
                }
            }
        }
    }

private:
    void SendDynamicRateInfo(Player* player, uint32 money)
    {
        uint32 gold = money / GOLD;
        uint32 silver = (money % GOLD) / SILVER;
        uint32 copper = (money % GOLD) % SILVER;
        std::string info;

        if (money < 100)
            info = Acore::StringFormat("You receive an additional %i copper.", copper);
        else if (money < 10000)
            info = Acore::StringFormat("You receive an additional %i silver, %i copper.", silver, copper);
        else
            info = Acore::StringFormat("You receive an additional %i gold, %i silver, %i copper.", gold, silver, copper);

        ChatHandler(player->GetSession()).SendSysMessage(info);
    }
};

void AddDynamicRatesScripts()
{
    new DynamicRates();
    new DynamicRatesConfig();
    new DynamicRatesLoot();
}
