#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Containers/Deque.h"
#include "GCActionSequence.generated.h"

/// <summary>
/// 특정 액션의 Redo/Undo를 실행하는 클래스
/// 
/// - 제한 된 스택 사이즈 만큼 그간의 액션들이 적재되어야 한다.
/// - Redo / Undo를 위한 컨트롤을 했을 때 인덱스에 따라 해당 액션이 적용되어야 한다.
/// - 액션은 각 하위 SequenceBase에서 담당한다.
/// 
/// </summary>


// 적재 될 액션에 대한 오브젝트
UCLASS()
class UActionSequenceBase : public UObject
{
	GENERATED_BODY()
	
public:
	virtual bool Redo() { return false; }
	virtual bool Undo() { return false; }
};

// 액션시퀀스 오브젝트를 관리할 주체
UCLASS()
class UActionSequenceController : public UObject
{
	GENERATED_BODY()

public:
	UActionSequenceController(const FObjectInitializer& ObjectInitializer);
	
protected:

	// 액션 시퀀스를 담을 스토리지
		TDeque<UActionSequenceBase*>	_ActionSequenceStorage;

	// 현재 인덱스
	UPROPERTY(transient)
		int				_Index			= 0;

	// 스토리지의 사이즈
	UPROPERTY(transient)
		uint8			_StorageSize	= 50;

public:
	virtual void BeginDestroy() override;

public:

	// 액션 시퀀스는 각 액션에 맞게 캐스팅해서 사용한다.
	static UActionSequenceBase* MakeActionSequence(UObject* pOuter, TSubclassOf<UActionSequenceBase> newActionSequClass);

	template<class T>
	static T* MakeActionSequence(UObject* pOuter)
	{
		return Cast<T>(MakeActionSequence(pOuter, T::StaticClass()));
	}

public:

	void			SetStorageSize(const uint8 storageSize) { _StorageSize = storageSize; }

public:
	virtual void	ClearStorage();

	virtual bool	PushSequence(UActionSequenceBase* pActionSeqBase);
	virtual void	PopSequence(const uint8 count);

	virtual bool	RedoAction();
	virtual bool	UndoAction();

	virtual bool	EnableRedoAction() { return _Index < _ActionSequenceStorage.Num(); }
	virtual bool	EnableUndoAction() { return _Index > 0; }

private:
	bool			DestroySequence(UActionSequenceBase* pActionSeqBase) const;

};