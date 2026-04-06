#include "GlobalMessageDispatcher.h"

void UGlobalMessageDispatcher::BroadcastGlobalMessage(const FGlobalMessagePayload& InMsg)
{
	FChatItemInfo ChatInfo;
	EGlobalMessageFilterType FilterType = EGlobalMessageFilterType::Max;

	struct FVisitor
	{
		UGlobalMessageDispatcher* Self;
		FChatItemInfo& OutChatInfo;
		EGlobalMessageFilterType& OutFilterType;

		FText operator()(const TSharedPtr<IGameAction>& InAction) const
		{
			if (!InAction.IsValid()) return FText::GetEmpty();

			auto ActionType = InAction->GetActionType();

			switch (ActionType)
			{
			case EGameActionType::BroadcastLootSummon:
			{
				const auto& Info = InAction->GetData<FLootSummonInfo>();
				OutFilterType = EGlobalMessageFilterType::Summon;
				return Self->GetLootSummonMsg(Info);
			}
			case EGameActionType::BroadcastRealmLootSummon:
			{
				FString RealmWorldName = InAction->GetRealmName();
				const auto& Info = InAction->GetData<FLootSummonInfo>();

				if (RealmWorldName.IsEmpty()) break;

				OutFilterType = EGlobalMessageFilterType::Summon;
				return Self->GetRealmLootSummonMsg(RealmWorldName, Info);
			}
			case EGameActionType::BroadcastLootItem:
			{
				const auto& Info = InAction->GetData<FLootItemInfo>();
				return Self->GetLootItemMsg(Info, OutChatInfo);
			}
			case EGameActionType::BroadcastRealmLootItem:
			{
				FString RealmWorldKey = InAction->GetRealmKey();
				const auto& Info = InAction->GetData<FLootItemInfo>();
				return Self->GetLootItemMsg(Info, OutChatInfo, RealmWorldKey);
			}
			case EGameActionType::BroadcastUpgradeEquipment:
			{
				const auto& Info = InAction->GetData<FEquipmentUpgradeInfo>();
				return Self->GetUpgradeEquipmentMsg(Info, OutChatInfo);
			}
			case EGameActionType::BroadcastRealmUpgradeEquipment:
			{
				FString RealmWorldName = InAction->GetRealmName();
				const auto& Info = InAction->GetData<FEquipmentUpgradeInfo>();

				if (!RealmWorldName.IsEmpty())
				{
					return Self->GetRealmUpgradeEquipmentMsg(RealmWorldName, Info, OutChatInfo);
				}
				break;
			}
			case EGameActionType::BroadcastWishReward:
			{
				const auto& Info = InAction->GetData<FWishRewardInfo>();
				return Self->GetWishRewardMsg(Info, OutChatInfo);
			}
			case EGameActionType::BroadcastMailItem:
			{
				FString TakerName = InAction->GetTakerName();
				int32 ItemType = InAction->GetItemType();
				return Self->GetMailItemMsg(TakerName, ItemType, OutChatInfo);
			}
			}

			return FText::GetEmpty();
		}

		FText operator()(const FCraftGlobalMessage& InMsg) const
		{
			OutFilterType = EGlobalMessageFilterType::Craft;
			return Self->GetCraftMsg(InMsg);
		}

		FText operator()(const FEvolutionResultNoti& InPacket) const
		{
			return Self->GetEvolutionItemMsg(InPacket, OutChatInfo);
		}

		FText operator()(const FGuildCraftNoti& InPacket) const
		{
			OutFilterType = EGlobalMessageFilterType::Craft;
			return Self->GetGuildCraftMsg(InPacket, OutChatInfo);
		}

		FText operator()(const FLuckyDrawGlobalMessage& InMsg) const
		{
			OutFilterType = EGlobalMessageFilterType::LuckyDraw;
			return Self->GetEventLuckyDrawMsg(InMsg, OutChatInfo);
		}
	};

	const FText MessageText = InMsg.Visit(FVisitor{ this, ChatInfo, FilterType });

	if (MessageText.IsEmpty())
	{
		return;
	}

	this->AddToChatSystem(MessageText, ChatInfo);

	if (FilterType != EGlobalMessageFilterType::Max)
	{
		if (this->IsFilterEnabled(FilterType))
		{
			return;
		}
	}

	// 전역 UI 시스템(Toast) 호출 예시
	// GlobalUI->ShowToastMessage(MessageText, FilterType);
}