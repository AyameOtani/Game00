#include "TitleScene.h"
#include "Utility.h" // 呼び出すと、SCREEN_WIDTHとかを使える
#include "DxLib.h"
#include "Master.h"
#include "inputManager.h"
#include "Stage.h"
#include "SkyBox.h"

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


	// 文字作成
	mnTitleFontHandle = CreateFontToHandle(
		NULL,
		100,
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


	Stage* stage = new Stage("Resource/3D/Stage1/TitleStage.mqo", "Resource/3D/Stage1/TitleStage.mqo");
	SkyBox* pSkyBox = new SkyBox("Resource/3D/SkyBox/sky.mqo");
	pSkyBox->SetScale(10.0f);  // 最大で9.0ぐらい？ 1222

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

	// 基底クラスの更新処理を呼びだす
	Scene::Draw();

	// 2D用に設定
	SetUseZBufferFlag(FALSE);
	SetWriteZBufferFlag(FALSE);

	const char* text = "たこさんウインナーは弁当箱に帰りたい";

	int width = GetDrawStringWidthToHandle(
		text,
		strlen(text),
		mnTitleFontHandle
	);

	// 中央を500に合わせる
	int x = (Utility::SCREEN_WIDTH - width) / 2;

	DrawFormatStringToHandle(
		x, 400,
		GetColor(255, 255, 255),
		mnTitleFontHandle,
		text
	);

	DrawFormatString(750, 880, GetColor(255, 255, 255), "EnterでSTART");

	// 3D用に設定
	SetUseZBufferFlag(TRUE);
	SetWriteZBufferFlag(TRUE);

}


void TitleScene::Finalize()
{
	// フォントハンドルの削除
	DeleteFontToHandle(mnTitleFontHandle);
}