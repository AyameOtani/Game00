#include "TitleScene.h"
#include "Utility.h" // 呼び出すと、SCREEN_WIDTHとかを使える
#include "DxLib.h"
#include "Master.h"
#include "inputManager.h"

TitleScene::TitleScene()
	: Scene()     // 基底クラスのコンストラクタを呼び出しておく
{
	SetFontSize(60); // 文字の大きさ
}

TitleScene::~TitleScene()
{

}

void TitleScene::Initialize()
{
	// ここでカメラを元の位置に戻している
	Master::mpCamera->Reset();
}



void TitleScene::Update()
{
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::SCENE_3D);
	}

	// 基底クラスの更新処理を呼びだす
	Scene::Update();
}


void TitleScene::Draw()
{
	DrawString(0, 0, "タコさんウインナーは弁当箱に帰りたい", GetColor(255, 255, 255));
	// 基底クラスの更新処理を呼びだす
	Scene::Draw();
}


void TitleScene::Finalize()
{

}