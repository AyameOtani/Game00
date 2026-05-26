#include "LoseResultScene.h"
#include "Utility.h" // 呼び出すと、SCREEN_WIDTHとかを使える
#include "DxLib.h"
#include "Master.h"
#include "inputManager.h"

LoseResultScene::LoseResultScene()
	: Scene()     // 基底クラスのコンストラクタを呼び出しておく
{
	SetFontSize(70); // 文字の大きさ

	mnRogoHandle = LoadGraph("Resource/2D/Lose.png");
	if (mnRogoHandle == -1) printfDx("画像ない");

	mnBagHandle = LoadGraph("Resource/2D/TitleBag.png");
	if (mnBagHandle == -1) printfDx("画像ない");

	mnBoxHandle = LoadGraph("Resource/2D/TitleBox.png");
	if (mnBoxHandle == -1) printfDx("画像ない");
}

LoseResultScene::~LoseResultScene()
{

}

void LoseResultScene::Initialize()
{
	// ここでカメラを元の位置に戻している
	//Master::mpCamera->Reset();
}



void LoseResultScene::Update()
{
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::TITLE_3D);
	}

	// 基底クラスの更新処理を呼びだす
	Scene::Update();
}


void LoseResultScene::Draw()
{
	// 2D用に設定
	SetUseZBufferFlag(FALSE);
	SetWriteZBufferFlag(FALSE);

	// 背景の描画
	DrawGraph(0, 0, mnBagHandle, TRUE);

	// 文字の背景
	DrawRotaGraph(Utility::SCREEN_WIDTH / 2, 910, 0.6f, 0.0f, mnBoxHandle, TRUE);

	// ロゴの描画
	DrawRotaGraph(Utility::SCREEN_WIDTH / 2, Utility::SCREEN_HEIGHT / 2 - 110, 0.85f, 0.0f, mnRogoHandle, TRUE);

	DrawFormatString(750, 880, GetColor(255, 255, 255), "EnterでTITLE");

	// 3D用に設定
	SetUseZBufferFlag(TRUE);
	SetWriteZBufferFlag(TRUE);

	// 基底クラスの更新処理を呼びだす
	Scene::Draw();
}


void LoseResultScene::Finalize()
{

}