#include "TitleScene.h"
#include "Utility.h" // 呼び出すと、SCREEN_WIDTHとかを使える
#include "DxLib.h"
#include "Master.h"
#include "inputManager.h"

TitleScene::TitleScene()
	: Scene()     // 基底クラスのコンストラクタを呼び出しておく
{
	SetFontSize(70); // 文字の大きさ

	mnRogoHandle = LoadGraph("Resource/2D/Title1.png");
	if (mnRogoHandle == -1) printfDx("画像ない");

	mnBagHandle = LoadGraph("Resource/2D/TitleBag.png");
	if (mnBagHandle == -1) printfDx("画像ない");

	mnBoxHandle = LoadGraph("Resource/2D/TitleBox.png");
	if (mnBoxHandle == -1) printfDx("画像ない");
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
	// 背景の描画
	DrawGraph(0, 0, mnBagHandle, TRUE);

	// 文字の背景
	DrawRotaGraph(Utility::SCREEN_WIDTH / 2, 910, 0.6f, 0.0f, mnBoxHandle, TRUE);

	// ロゴの描画
	DrawRotaGraph(Utility::SCREEN_WIDTH / 2, Utility::SCREEN_HEIGHT / 2 - 110, 0.85f, 0.0f, mnRogoHandle, TRUE);

	DrawFormatString(750, 880, GetColor(255, 255, 255), "EnterでSTART");

	// 基底クラスの更新処理を呼びだす
	Scene::Draw();
}


void TitleScene::Finalize()
{

}