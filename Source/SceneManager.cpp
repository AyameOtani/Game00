#include "SceneManager.h"
#include "Scene.h"
#include "Master.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "WinResultScene.h"
#include "LoseResultScene.h"
#include "LoadingScene.h" // 追加

SceneManager::SceneManager()
	: mnSceneType(SCENE_TYPE::SCENE_NONE)
	, mnNextSceneType(SCENE_TYPE::SCENE_NONE)
	, mpCurrentScene(nullptr)
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::Initialize()
{
	mnNextSceneType = SCENE_TYPE::TITLE_3D;
	ChangeSceneIfNeeded();
}

void SceneManager::Update()
{
	if (mpCurrentScene) mpCurrentScene->Update();
}

void SceneManager::Draw()
{
	if (mpCurrentScene) mpCurrentScene->Draw();
}

void SceneManager::Finalize()
{
}

void SceneManager::ChangeSceneIfNeeded()
{
	if (mnSceneType == mnNextSceneType) return;

	if (mpCurrentScene != nullptr)
	{
		mpCurrentScene->Finalize();
		delete mpCurrentScene;
		mpCurrentScene = nullptr;
	}

	mnSceneType = mnNextSceneType;

	switch (mnSceneType)
	{
	case SCENE_TYPE::SCENE_3D:
		mpCurrentScene = new GameScene();
		break;
	case SCENE_TYPE::TITLE_3D:
		mpCurrentScene = new TitleScene();
		break;
	case SCENE_TYPE::LOADING_3D:
		mpCurrentScene = new LoadingScene();
		break;
	case SCENE_TYPE::WIN_RESULT_3D:
		mpCurrentScene = new WinResultScene();
		break;
	case SCENE_TYPE::LOSE_RESULT_3D:
		mpCurrentScene = new LoseResultScene();
		break;
	default:
		mpCurrentScene = nullptr;
		break;
	}

	if (mpCurrentScene) mpCurrentScene->Initialize();
}