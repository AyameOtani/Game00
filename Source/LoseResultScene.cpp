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
		Master::mpSoundManager->PlaySE(SoundManager::SE_ENTER, 255);
	}

	mBlinkCounter++;

	// 30フレームごとに切り替え（約0.5秒）
	if (mBlinkCounter > 30) {
		mShowText = !mShowText;
		mBlinkCounter = 0;
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


	int frameWidth = 1420;
	int frameHeight = 820;

	// UI背景（影）
	DrawBox(
		Utility::SCREEN_WIDTH / 2 - frameWidth / 2,
		Utility::SCREEN_HEIGHT / 2 - frameHeight / 2,
		Utility::SCREEN_WIDTH / 2 + frameWidth / 2,
		Utility::SCREEN_HEIGHT / 2 + frameHeight / 2,
		GetColor(139, 69, 19),
		TRUE
	);


	int boxWidth = 1400;
	int boxHeight = 800;
	// UI本体
	DrawBox(
		Utility::SCREEN_WIDTH / 2 - boxWidth / 2,
		Utility::SCREEN_HEIGHT / 2 - boxHeight / 2,
		Utility::SCREEN_WIDTH / 2 + boxWidth / 2,
		Utility::SCREEN_HEIGHT / 2 + boxHeight / 2,
		GetColor(207, 170, 132),
		TRUE
	);


	// ロゴの描画
	DrawRotaGraph(Utility::SCREEN_WIDTH / 2, Utility::SCREEN_HEIGHT / 2 - 180, 0.65f, 0.0f, mnRogoHandle, TRUE);


	// 文字の背景
	DrawRotaGraph(Utility::SCREEN_WIDTH / 2, 840, 0.6f, 0.0f, mnBoxHandle, TRUE);
	int color = GetColor(255, 255, 255);
	if (mShowText)
	{
		DrawFormatString(750, 810, color, "EnterでTITLE");
	}

	DrawFormatString(Utility::SCREEN_WIDTH / 2 - 350, Utility::SCREEN_HEIGHT / 2+100, color, "倒したおかずの数   %d", Master::mnDeleteEnemyCount);


	// 3D用に設定
	SetUseZBufferFlag(TRUE);
	SetWriteZBufferFlag(TRUE);

	// 基底クラスの更新処理を呼びだす
	Scene::Draw();

}


void LoseResultScene::Finalize()
{

}