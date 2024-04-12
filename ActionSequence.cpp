#include "ActionSequence/GCActionSequence.h"


UActionSequenceController::UActionSequenceController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	_ActionSequenceStorage.Reserve(_StorageSize);
}

void UActionSequenceController::BeginDestroy()
{
	ClearStorage();
	Super::BeginDestroy();
}

// 액션 시퀀스 클래스를 생성하여 반환한다.
UActionSequenceBase* UActionSequenceController::MakeActionSequence(UObject* pOuter, TSubclassOf<UActionSequenceBase> newActionSequClass)
{
	if (UActionSequenceBase* pNewActionSeqBase = NewObject<UActionSequenceBase>(pOuter, newActionSequClass))
	{
		pNewActionSeqBase->AddToRoot();
		return pNewActionSeqBase;
	}
	return nullptr;
}

// 스토리지 전체 클리어
void UActionSequenceController::ClearStorage()
{
	PopSequence(_ActionSequenceStorage.Num());

	_ActionSequenceStorage.Empty();
	_Index = 0;
}

// 액션을 하게되면 시퀀스를 스토리지에 적재한다.
// 이미 최대 사이즈라면, 가장 최초의 것을 삭제하고 추가한다.
bool UActionSequenceController::PushSequence(UActionSequenceBase* pActionSeqBase)
{
	if (!pActionSeqBase)
		return false;

	if (EnableRedoAction())
	{
		PopSequence(_ActionSequenceStorage.Num() - _Index);
	}

	if (_ActionSequenceStorage.Num() == _StorageSize)
	{
		if (DestroySequence(_ActionSequenceStorage.First()) == false)
		{
			check(false);
		}

		_ActionSequenceStorage.PopFirst();
	}

	_ActionSequenceStorage.PushLast(pActionSeqBase);
	_Index = _ActionSequenceStorage.Num();

	return true;
}

// 스토리지에서 시퀀스를 제거한다.
void UActionSequenceController::PopSequence(const uint8 count)
{
	if(count == 0)
		return;

	for (uint8 i = 1; i <= count; i++)
	{
		if (DestroySequence(_ActionSequenceStorage.Last()) == false)
		{
			continue;
		}

		_ActionSequenceStorage.PopLast();
	}
}

// Redo가 가능한지 체크하고, ActionSequnce의 Redo 함수를 호출한다.
bool UActionSequenceController::RedoAction()
{
	if (EnableRedoAction() == false)
	{
		return false;
	}

	UActionSequenceBase* redoAction = _ActionSequenceStorage[_Index];
	if (redoAction->IsValidLowLevel() == false)
	{
		check(false);
		return false;
	}

	UE_LOG_WITH_SCREEN_MSG_PRESET_LOG(*FString::Format(TEXT("[UGCActionSequenceController::RedoAction] TargetIndex {0}"), { _Index }));

	redoAction->Redo();
	_Index++;

	return true;
}

bool UActionSequenceController::UndoAction()
{
	if (EnableUndoAction() == false )
	{
		return false;
	}

	UActionSequenceBase* undoAction = _ActionSequenceStorage[_Index - 1];
	if (undoAction->IsValidLowLevel() == false)
	{
		check(false);
		return false;
	}

	UE_LOG_WITH_SCREEN_MSG_PRESET_LOG(*FString::Format(TEXT("[UGCActionSequenceController::UndoAction] TargetIndex {0}"), { _Index - 1 }));
	
	undoAction->Undo();
	_Index--;

	GEngine->ForceGarbageCollection(true);

	return true;
}

bool UActionSequenceController::DestroySequence(UActionSequenceBase* pActionSeqBase) const
{
	if (pActionSeqBase == nullptr || pActionSeqBase->IsValidLowLevel() == false)
	{
		return false;
	}

	pActionSeqBase->RemoveFromRoot();
	pActionSeqBase->ConditionalBeginDestroy();

	return true;
}
