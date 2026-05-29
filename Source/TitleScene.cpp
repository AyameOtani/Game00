#include "TitleScene.h"
#include "Utility.h" // 呼び出すと、SCREEN_WIDTHとかを使える
#include "DxLib.h"
#include "Master.h"
#include "inputManager.h"
#include "Stage.h"
#include "SkyBox.h"
#include "SceneManager.h"

TitleScene::TitleScene()
	: Scene()     // 基底クラスのコンストラクタを呼び出しておく
{
	SetFontSize(70); // 文字の大きさ

	mnRogoHandle = LoadGraph("Resource/2D/Title.png");
	if (mnRogoHandle == -1) printfDx("画像ない");



	// 文字作成
	mnTitleFontHandle = CreateFontToHandle(
		NULL,
		100,
		5
	);

	// 文字作成
	mnSmallFontHandle = CreateFontToHandle(
		NULL,
		30,
		5
	);
}



TitleScene::~TitleScene()
{

}

void TitleScene::Initialize()
{
	// ここでカメラを元の位置に戻している
	Master::mpCamera->Reset();
	Master::mpCamera->SetTitleMode(true);


	// タイトルでは既存のファイル名版を使用する（描画用）
	Stage* stage = new Stage("Resource/3D/Stage1/TitleStage.mqo", "Resource/3D/Stage1/TitleStage.mqo");
	SkyBox* pSkyBox = new SkyBox("Resource/3D/SkyBox/sky.mqo");
	pSkyBox->SetScale(10.0f);  // 最大で9.0ぐらい？ 1222


	// ずっとループだからfalseにしている
	Master::mpSoundManager->PlayBGM(SoundManager::BGM_TITLE, false, 170);


}



void TitleScene::Update()
{
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		// 直接ゲームに飛ばすのではなくローディングシーンを挟む
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::LOADING_3D);
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

void TitleScene::Draw()
{

	// 基底クラスの更新処理を呼びだす
	Scene::Draw();

	// 2D用に設定
	SetUseZBufferFlag(FALSE);
	SetWriteZBufferFlag(FALSE);

	DrawRotaGraph(Utility::SCREEN_WIDTH / 2, Utility::SCREEN_HEIGHT / 2-150, 1.0f, 0.0f, mnRogoHandle, TRUE);

	int color = GetColor(255, 255, 255);
	if (mShowText)
	{
		DrawFormatString(750, 880, GetColor(255, 255, 255), "EnterでSTART");
	}

	DrawFormatStringToHandle(1700, 1020, GetColor(255, 255, 255),mnSmallFontHandle, "音楽：魔王魂");

	// 3D用に設定
	SetUseZBufferFlag(TRUE);
	SetWriteZBufferFlag(TRUE);

}


void TitleScene::Finalize()
{
	// フォントハンドルの削除
	DeleteFontToHandle(mnTitleFontHandle);
	DeleteFontToHandle(mnSmallFontHandle);
	// 画像の削除
	DeleteGraph(mnRogoHandle);
}