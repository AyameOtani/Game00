#include "WinResultScene.h"
#include "Utility.h" // 呼び出すと、SCREEN_WIDTHとかを使える
#include "DxLib.h"
#include "Master.h"
#include "inputManager.h"

WinResultScene::WinResultScene()
	: Scene()     // 基底クラスのコンストラクタを呼び出しておく
{
	SetFontSize(60); // 文字の大きさ
}

WinResultScene::~WinResultScene()
{

}

void WinResultScene::Initialize()
{
	// ここでカメラを元の位置に戻している
	Master::mpCamera->Reset();
}



void WinResultScene::Update()
{
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::TITLE_3D);
	}

	// 基底クラスの更新処理を呼びだす
	Scene::Update();
}


void WinResultScene::Draw()
{
	DrawString(0, 0, "勝利画面", GetColor(255, 255, 255));

	// 基底クラスの更新処理を呼びだす
	Scene::Draw();
}


void WinResultScene::Finalize()
{

}