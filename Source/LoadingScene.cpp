#include "LoadingScene.h"
#include "DxLib.h"
#include "Master.h"
#include "SceneManager.h"
#include "Utility.h"

// ロード対象ファイル
static const char* kSkyModelPath = "Resource/3D/SkyBox/sky.mqo";

static const char* kStageModelPath = "Resource/3D/Stage1/stage.mqo";
static const char* kStageCollisionPath = "Resource/3D/Stage1/stage.mqo";

static const char* kMoveStagePath = "Resource/3D/Stage1/moveStage.mqo";
static const char* kMoveStageCollPath = "Resource/3D/Stage1/moveStage.mqo";

static const char* kRotaStagePath = "Resource/3D/Stage1/rotaStage.mqo";
static const char* kRotaStageCollPath = "Resource/3D/Stage1/rotaStage.mqo";

static const char* kLittleRotaStagePath = "Resource/3D/Stage1/littleRota.mqo";
static const char* kLittleRotaCollPath = "Resource/3D/Stage1/littleRota.mqo";

LoadingScene::LoadingScene()
	: mbLoaded(false)
	, mnProgressCounter(0)
	, mMinWaitFrame(0)   // ★追加
{
}

LoadingScene::~LoadingScene()
{
}

void LoadingScene::Initialize()
{
	mbLoaded = false;
	mnProgressCounter = 0;

	mMinWaitFrame = 30; // ★最低1秒表示（60FPS想定）
}

void LoadingScene::Update()
{
	//========================================
	// ロード完了後：最低表示時間を待つ
	//========================================
	if (mnProgressCounter > 8)
	{
		if (mMinWaitFrame > 0)
		{
			mMinWaitFrame--;
			return;
		}

		mbLoaded = true;
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::SCENE_3D);
		return;
	}

	//========================================
	// ロード処理
	//========================================
	if (mnProgressCounter == 0)
	{
		if (Master::mnSkyModelHandle == -1)
			Master::mnSkyModelHandle = MV1LoadModel(kSkyModelPath);
	}
	else if (mnProgressCounter == 1)
	{
		if (Master::mnStageModelHandle == -1)
			Master::mnStageModelHandle = MV1LoadModel(kStageModelPath);
	}
	else if (mnProgressCounter == 2)
	{
		if (Master::mnStageCollisionHandle == -1)
		{
			Master::mnStageCollisionHandle = MV1LoadModel(kStageCollisionPath);

			if (Master::mnStageCollisionHandle != -1)
				MV1SetupCollInfo(Master::mnStageCollisionHandle, -1);
		}
	}
	else if (mnProgressCounter == 3)
	{
		if (Master::mnStageMoveHandle == -1)
			Master::mnStageMoveHandle = MV1LoadModel(kMoveStagePath);
	}
	else if (mnProgressCounter == 4)
	{
		if (Master::mnStageMoveCollHandle == -1)
		{
			Master::mnStageMoveCollHandle = MV1LoadModel(kMoveStageCollPath);

			if (Master::mnStageMoveCollHandle != -1)
				MV1SetupCollInfo(Master::mnStageMoveCollHandle, -1);
		}
	}
	else if (mnProgressCounter == 5)
	{
		if (Master::mnStageRotaHandle == -1)
			Master::mnStageRotaHandle = MV1LoadModel(kRotaStagePath);
	}
	else if (mnProgressCounter == 6)
	{
		if (Master::mnStageRotaCollHandle == -1)
		{
			Master::mnStageRotaCollHandle = MV1LoadModel(kRotaStageCollPath);

			if (Master::mnStageRotaCollHandle != -1)
				MV1SetupCollInfo(Master::mnStageRotaCollHandle, -1);
		}
	}
	else if (mnProgressCounter == 7)
	{
		if (Master::mnStageLittleRotaHandle == -1)
			Master::mnStageLittleRotaHandle = MV1LoadModel(kLittleRotaStagePath);
	}
	else if (mnProgressCounter == 8)
	{
		if (Master::mnStageLittleCollRotaHandle == -1)
		{
			Master::mnStageLittleCollRotaHandle = MV1LoadModel(kLittleRotaCollPath);

			if (Master::mnStageLittleCollRotaHandle != -1)
				MV1SetupCollInfo(Master::mnStageLittleCollRotaHandle, -1);
		}
	}

	// 進捗更新
	mnProgressCounter++;
}

void LoadingScene::Draw()
{
	SetUseZBufferFlag(FALSE);
	SetWriteZBufferFlag(FALSE);

	DrawFormatString(
		Utility::SCREEN_WIDTH / 2 - 100,
		Utility::SCREEN_HEIGHT / 2,
		GetColor(255, 255, 255),
		"Loading..."
	);

	SetUseZBufferFlag(TRUE);
	SetWriteZBufferFlag(TRUE);
}

void LoadingScene::Finalize()
{
	// Master管理なので解放しない
}