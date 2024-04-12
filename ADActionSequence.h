// Copyright(c) Burning Galaxy LTD. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "ActionSequence/GCActionSequence.h"
#include "Game/Decal/DecalTypes.h"
#include "Game/Keyword/ADContentsKeyword.h"
#include "ADActionSequence.generated.h"

struct FDecalTransform;
class UGCActionSequenceController;

/// <summary>
/// ActionSequenceBase의 상속을 받아 특정 컨텐츠에 맞게 구현한 클래스
/// 
/// - 하나의 액션은 하나의 행동을 담당한다.
/// - 하나의 클래스는 액션에 맞는 파라미터들을 가진다.
/// - Redo() / Undo() 함수를 오버라이딩하여 액션 실행부를 만든다.
/// 
/// </summary>


// Decal의 위치/각도/크기를 변경하는 액션
UCLASS()
class UADActionSeq_DecalTransform : public UActionSequenceBase
{
	GENERATED_BODY()

public:
	void							SetUndoSnapshot(const DecalLayerId decalLayerId, const FDecalTransform& decalLayoutInfo);
	void							SetRedoSnapshot(const DecalLayerId decalLayerId, const FDecalTransform& decalLayoutInfo);

public:
	virtual bool					Redo() override;
	virtual bool					Undo() override;

private:
	const bool						ExecuteSnapShot(const FDecalTransform& decalTransform);

protected:
	DecalLayerId					_DecalLayerId;

	FDecalTransform					_UndoTransformSnapShot;
	FDecalTransform					_RedoTransformSnapShot;
};

// Decal의 색상을 변경하는 액션
UCLASS()
class UADActionSeq_ChangeDecalColor : public UActionSequenceBase
{
	GENERATED_BODY()

public:
	void							SetActionData(const DecalLayerId decalLayerId, const FLinearColor& undoColor, const FLinearColor& redoColor);
public:
	virtual bool					Redo() override;
	virtual bool					Undo() override;

private:
	const bool						Execute(const FLinearColor color);

protected:
	DecalLayerId					_DecalLayerId;

	FLinearColor					_UndoColor;
	FLinearColor					_RedoColor;
};

USTRUCT(Blueprintable)
struct FChangeClothColorData
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY()
	FString			MatParamName;
	UPROPERTY()
	FLinearColor	UndoColor;
	UPROPERTY()
	FLinearColor	RedoColor;
};

// 의상의 색상을 변경하는 액션
UCLASS()
class UADActionSeq_ChangeClothColor : public UActionSequenceBase
{
	GENERATED_BODY()

public:
	void							SetActionData(const TArray<FChangeClothColorData>& changeClothColorDatas);

public:
	virtual bool					Redo() override;
	virtual bool					Undo() override;

private:
	const bool						Execute(const TMap<FString, FLinearColor>& colorSet);

protected:
	TArray<FChangeClothColorData>	_ChangeClothColorDatas;
};

// Decal의 레이어간 순서를 변경하는 액션
UCLASS()
class UADActionSeq_SwapDecalLayer : public UActionSequenceBase
{
	GENERATED_BODY()

public:
	void							SetActionData(const DecalLayerId decalLayerId, const int32 undoOrder, const int32 redoOrder);

public:
	virtual bool					Redo() override;
	virtual bool					Undo() override;

private:
	const bool						Execute(const int32 currentOrder, const int32 changeOrder);

protected:
	DecalLayerId					_DecalLayerId;

	int32							_UndoOrder;
	int32							_RedoOrder;
};

// 레이어를 추가/삭제/복제 하는 액션의 부모가 될 클래스
UCLASS()
class UADActionSeq_TextureData : public UActionSequenceBase
{
	GENERATED_BODY()

public:
	void							SetTextureInfo(const FDecalLayerStruct& newLayerInfo)
									{
										_DecalLayerInfo = newLayerInfo;
									}

	void							SetTextureInfo(const DecalLayerId decalLayerId, const int32 insertIdx, const FDecalLayerStruct& newLayerInfo);
	void							SetTextureRawData(const TArray<uint8>& rawData) { _TextureRawData = rawData; }

protected:
	const bool						AddDecalTexture();
	const bool						RemoveDecalTexture();

protected:
	FDecalLayerStruct				_DecalLayerInfo;
	DecalLayerId					_DecalLayerId;
	int32							_DecalLayerIdx;
	TArray<uint8>					_TextureRawData;
};

UCLASS()
class UADActionSeq_AddDecal : public UADActionSeq_TextureData
{
	GENERATED_BODY()

public:
	virtual bool					Redo() override;
	virtual bool					Undo() override;
};

UCLASS()
class UADActionSeq_RemoveDecal : public UADActionSeq_TextureData
{
	GENERATED_BODY()

public:
	virtual bool					Redo() override;
	virtual bool					Undo() override;
};

UCLASS()
class UADActionSeq_DuplicateDecal : public UADActionSeq_TextureData
{
	GENERATED_BODY()

public:
	virtual bool					Redo() override;
	virtual bool					Undo() override;
private:
	const bool						DuplicateDecalTexture();
};

// 툴 박스에서 사용한 특정 값들을 변경하는 액션 (ex. 패턴, 오파시티 등 )
UCLASS()
class UADActionSeq_DecalTool : public UActionSequenceBase
{
	GENERATED_BODY()

public:
	void							SetActionData(const DecalLayerId decalLayerId, const EADToolKeyword toolType, const float undoValue, const float redoValue);

	void							SetUndoData(const DecalLayerId decalLayerId, const EADToolKeyword toolType, const float value);
	void							SetRedoData(const DecalLayerId decalLayerId, const EADToolKeyword toolType, const float value);

public:
	virtual bool					Redo() override;
	virtual bool					Undo() override;

private:
	const bool						Execute(float value);

protected:
	DecalLayerId					_DecalLayerId;

	EADToolKeyword					_ToolType;

	float							_UndoValue;
	float							_RedoValue;
};

// Decal을 좌우반전 하는 액션
UCLASS()
class UADActionSeq_FlipDecal : public UActionSequenceBase
{
	GENERATED_BODY()

public:
	void							SetActionData(const DecalLayerId decalLayerId, const EFlipType flipType);

public:
	virtual bool					Redo() override;
	virtual bool					Undo() override;

private:
	const bool						Execute();

protected:
	DecalLayerId					_DecalLayerId;
	
	EFlipType						_FlipType;
};
