
#include "ADActionSequence.h"

#include "AeGameSystem.h"

#include "Manager/GCManagerRegistry.h"

#include "GameCore/Manager/ADContentsManager.h"
#include "GameCore/Helper/ADContentsHelper.h"
#include "GameCore/Manager/ADEditorManager.h"

#include "Game/Keyword/ADContentsKeyword.h"
#include "Game/UI/UMG/ADEditor.h"
#include "Game/UI/UMG/DecalEditor.h"

#include "Game/UI/Editor/Clothes/ClothesEditor.h"

void UADActionSeq_DecalTransform::SetUndoSnapshot(const DecalLayerId decalLayerId, const FDecalTransform& decalLayoutInfo)
{
	_DecalLayerId = decalLayerId;
	_UndoTransformSnapShot = decalLayoutInfo;
}

void UADActionSeq_DecalTransform::SetRedoSnapshot(const DecalLayerId decalLayerId, const FDecalTransform& decalLayoutInfo)
{
	if (_DecalLayerId != decalLayerId)
	{
		check(false);
	}

	_RedoTransformSnapShot = decalLayoutInfo;
}

bool UADActionSeq_DecalTransform::Redo()
{
	return ExecuteSnapShot(_RedoTransformSnapShot);
}

bool UADActionSeq_DecalTransform::Undo()
{
	return ExecuteSnapShot(_UndoTransformSnapShot);
}

const bool UADActionSeq_DecalTransform::ExecuteSnapShot(const FDecalTransform& decalTransform)
{
	auto& decalModeSystem = DECAL_MODE_SYSTEM();
	decalModeSystem.SetSelectLayerId(_DecalLayerId);
	decalModeSystem.SetLayerTransform(_DecalLayerId, decalTransform);

	return true;
}

void UADActionSeq_ChangeDecalColor::SetActionData(const DecalLayerId decalLayerId, const FLinearColor& undoColor, const FLinearColor& redoColor)
{
	_DecalLayerId = decalLayerId;

	_UndoColor = undoColor;
	_RedoColor = redoColor;
}

bool UADActionSeq_ChangeDecalColor::Redo()
{
	return Execute(_RedoColor);
}

bool UADActionSeq_ChangeDecalColor::Undo()
{
	return Execute(_UndoColor);
}

const bool UADActionSeq_ChangeDecalColor::Execute(const FLinearColor color)
{
	DECAL_MODE_SYSTEM().SetSelectLayerId(_DecalLayerId);
	DECAL_MODE_SYSTEM().SetLayerTint(_DecalLayerId, color);

	return true;
}

void UADActionSeq_SwapDecalLayer::SetActionData(const DecalLayerId decalLayerId, const int32 undoOrder, const int32 redoOrder)
{
	_DecalLayerId = decalLayerId;

	_UndoOrder = undoOrder;
	_RedoOrder = redoOrder;
}

bool UADActionSeq_SwapDecalLayer::Redo()
{
	return Execute(_UndoOrder, _RedoOrder);
}

bool UADActionSeq_SwapDecalLayer::Undo()
{
	return Execute(_RedoOrder, _UndoOrder);
}

const bool UADActionSeq_SwapDecalLayer::Execute(const int32 currentOrder, const int32 changeOrder)
{
	UADEditor* pADEditor = AE::FADContentsHelper::GetCurrentActiveEditorWidget();
	if (!pADEditor)
	{
		check(false);
		return false;
	}

	DECAL_MODE_SYSTEM().SwapLayer(currentOrder, changeOrder);
	return true;
}

void UADActionSeq_TextureData::SetTextureInfo(const DecalLayerId decalLayerId, const int32 insertIdx, const FDecalLayerStruct& newLayerInfo)
{
	_DecalLayerId = decalLayerId;
	_DecalLayerIdx = insertIdx;

	_DecalLayerInfo.SetCategory(newLayerInfo.GetCategoryIdx());
	_DecalLayerInfo.SetDecalKey(newLayerInfo.GetDecalKey());
	_DecalLayerInfo.SetTrasform(newLayerInfo.GetTransform());
	_DecalLayerInfo.SetTintColor(newLayerInfo.GetColor());
}

const bool UADActionSeq_TextureData::AddDecalTexture()
{
	DECAL_MODE_SYSTEM().AddDecalLayer(_DecalLayerInfo, _DecalLayerIdx, _DecalLayerId);
	return true;
}

const bool UADActionSeq_TextureData::RemoveDecalTexture()
{
	DECAL_MODE_SYSTEM().RemoveDecalLayer(_DecalLayerId);
	return true;
}

bool UADActionSeq_AddDecal::Redo()
{
	return AddDecalTexture();
}

bool UADActionSeq_AddDecal::Undo()
{
	return RemoveDecalTexture();
}

bool UADActionSeq_RemoveDecal::Redo()
{
	return RemoveDecalTexture();
}

bool UADActionSeq_RemoveDecal::Undo()
{
	return AddDecalTexture();
}

bool UADActionSeq_DuplicateDecal::Redo()
{
	return DuplicateDecalTexture();
}

bool UADActionSeq_DuplicateDecal::Undo()
{
	return RemoveDecalTexture();
}

const bool UADActionSeq_DuplicateDecal::DuplicateDecalTexture()
{
	const DecalLayerId targetDecalLayerId = DECAL_MODE_SYSTEM().GetSelectLayerId();	
	DECAL_MODE_SYSTEM().DuplicateLayer(targetDecalLayerId, _DecalLayerId);

	return true;
}

void UADActionSeq_ChangeClothColor::SetActionData(const TArray<FChangeClothColorData>& changeClothColorDatas)
{
	_ChangeClothColorDatas = changeClothColorDatas;
}

bool UADActionSeq_ChangeClothColor::Redo()
{
	TMap<FString, FLinearColor> redoColorSet;
	for (const auto& iter : _ChangeClothColorDatas)
	{
		redoColorSet.Emplace(iter.MatParamName, iter.RedoColor);
	}

	if (redoColorSet.IsEmpty())
	{
		return false;
	}

	return Execute(MoveTempIfPossible(redoColorSet));
}

bool UADActionSeq_ChangeClothColor::Undo()
{
	TMap<FString, FLinearColor> undoColorSet;
	for (auto& iter : _ChangeClothColorDatas)
	{
		undoColorSet.Emplace(iter.MatParamName, iter.UndoColor);
	}

	if (undoColorSet.IsEmpty())
	{
		return false;
	}

	return Execute(MoveTempIfPossible(undoColorSet));
}

const bool UADActionSeq_ChangeClothColor::Execute(const TMap<FString, FLinearColor>& colorSet)
{
	UADEditor* pADEditor = AE::FADContentsHelper::GetCurrentActiveEditorWidget();
	if (!pADEditor)
	{
		return false;
	}

	UClothesEditor* pClothesEditor = Cast<UClothesEditor>(pADEditor);
	if (!pClothesEditor)
	{
		return false;
	}

	pClothesEditor->UpdateMeshColorFromColorSet(colorSet);
	return true;
}

void UADActionSeq_DecalTool::SetActionData(const DecalLayerId decalLayerId, const EADToolKeyword toolType, const float undoValue, const float redoValue)
{
	SetUndoData(decalLayerId, toolType, undoValue);
	SetRedoData(decalLayerId, toolType, redoValue);
}

void UADActionSeq_DecalTool::SetUndoData(const DecalLayerId decalLayerId, const EADToolKeyword toolType, const float value)
{
	_DecalLayerId = decalLayerId;
	_ToolType = toolType;
	_UndoValue = value;
}

void UADActionSeq_DecalTool::SetRedoData(const DecalLayerId decalLayerId, const EADToolKeyword toolType, const float value)
{
	if (_DecalLayerId != decalLayerId || _ToolType != toolType)
	{
		check(false);
		return;
	}

	_RedoValue = value;
}

bool UADActionSeq_DecalTool::Redo()
{
	return Execute(_RedoValue);
}

bool UADActionSeq_DecalTool::Undo()
{
	return Execute(_UndoValue);
}

const bool UADActionSeq_DecalTool::Execute(float value)
{
	UADEditor* pADEditor = AE::FADContentsHelper::GetCurrentActiveEditorWidget();
	if (!pADEditor)
	{
		return false;
	}

	DECAL_MODE_SYSTEM().SetSelectLayerId(_DecalLayerId);
	pADEditor->SetValueByToolType(_ToolType, value);

	return true;
}

void UADActionSeq_FlipDecal::SetActionData(const DecalLayerId decalLayerId, const EFlipType flipType)
{
	_DecalLayerId = decalLayerId;
	_FlipType = flipType;
}

bool UADActionSeq_FlipDecal::Redo()
{
	return Execute();
}

bool UADActionSeq_FlipDecal::Undo()
{
	return Execute();
}

const bool UADActionSeq_FlipDecal::Execute()
{
	DECAL_MODE_SYSTEM().FlipLayer(_DecalLayerId, _FlipType);
	return true;
}
