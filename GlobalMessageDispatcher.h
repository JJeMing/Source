#pragma once

#include "CoreMinimal.h"
#include "Misc/TVariant.h"

enum class EGlobalMessageFilterType : uint8
{
	None,
	Summon,
	Craft,
	Upgrade,
	LuckyDraw,
	Max
};

using FGlobalMessagePayload = TVariant<
	TSharedPtr<class IGameAction>,
	struct FCraftGlobalMessage,
	struct FEvolutionResultNoti,
	struct FLuckyDrawGlobalMessage,
	struct FGuildCraftNoti>;

class UGlobalMessageDispatcher
{
public:
	void BroadcastGlobalMessage(const FGlobalMessagePayload& InMsg);

	FText GetLootSummonMsg(const struct FLootSummonInfo& InInfo);
	FText GetRealmLootSummonMsg(const FString& InServerName, const struct FLootSummonInfo& InInfo);
	FText GetLootItemMsg(const struct FLootItemInfo& InInfo, struct FChatItemInfo& OutChatInfo, const FString& InWorldKey = TEXT(""));
	FText GetCraftMsg(const struct FCraftGlobalMessage& InMsg);
	FText GetUpgradeEquipmentMsg(const struct FEquipmentUpgradeInfo& InInfo, struct FChatItemInfo& OutChatInfo);
	FText GetRealmUpgradeEquipmentMsg(const FString& InServerName, const struct FEquipmentUpgradeInfo& InInfo, struct FChatItemInfo& OutChatInfo);
	FText GetWishRewardMsg(const struct FWishRewardInfo& InInfo, struct FChatItemInfo& OutChatInfo);
	FText GetMailItemMsg(const FString& InName, int32 InType, struct FChatItemInfo& OutChatInfo);
	FText GetEvolutionItemMsg(const struct FEvolutionResultNoti& InPacket, struct FChatItemInfo& OutChatInfo);
	FText GetGuildCraftMsg(const struct FGuildCraftNoti& InPacket, struct FChatItemInfo& OutChatInfo);
	FText GetEventLuckyDrawMsg(const struct FLuckyDrawGlobalMessage& InMsg, struct FChatItemInfo& OutChatInfo);

private:
	bool IsFilterEnabled(EGlobalMessageFilterType InFilterType);
	void AddToChatSystem(const FText& InMessageText, const struct FChatItemInfo& InChatInfo);
};