#include "GameScene.h"
#include "Utility.h" // 呼び出すと、SCREEN_WIDTHとかを使える
#include "DxLib.h"
#include "Master.h"
#include "inputManager.h"
#include "SkyBox.h"
#include "Player3D.h"
#include "Stage.h"

GameScene::GameScene()
	: Scene()     // 基底クラスのコンストラクタを呼び出しておく
{
	SetFontSize(60); // 文字の大きさ
}

GameScene::~GameScene()
{

}

void GameScene::Initialize()
{
	SkyBox* pSkyBox = new SkyBox("Resource/3D/SkyBox/sky.mqo");
	pSkyBox->SetScale(8.0f);  // 最大で9.0ぐらい？ 1222

	// プレイヤーの生成
	// Masterに渡したものをStatus* statusのやつに設定しているからこんな引数
	Player3D* player = new Player3D(VGet(0, 0, 0), "Resource/3D/Player/octopus.mqo");

	Stage* stage = new Stage ("Resource/3D/Stage1/stage.mqo", "Resource/3D/Stage1/stage.mqo");
	// ここでカメラを元の位置に戻している
	Master::mpCamera->Reset();
}



void GameScene::Update()
{
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::WIN_RESULT_3D);
	}

	// 基底クラスの更新処理を呼びだす
	Scene::Update();
}


void GameScene::Draw()
{
	// 地面に格子状の線を描画する
	// note: 必要がなくなった時はコメントアウトする

	//const int count = 51;          // 個数
	//const float distance = 500.0f; // 距離
	//for (int i = 0; i < count; i++)
	//{
	//	float base = (count / 2 - i) * -distance;

	//	// 横線の描画
	//	DrawLine3D(
	//		VGet(-distance * (count / 2), 0.0f, base),
	//		VGet(distance * (count / 2), 0.0f, base),
	//		GetColor(255, 255, 255)
	//	);

	//	// 縦線の描画
	//	DrawLine3D(
	//		VGet(base, 0.0f , -distance * (count / 2)),
	//		VGet(base, 0.0f, distance * (count / 2)),
	//		GetColor(255, 255, 255)
	//	);
	//}


	// 基底クラスの更新処理を呼びだす
	Scene::Draw();
}


void GameScene::Finalize()
{

}