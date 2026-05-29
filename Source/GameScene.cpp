#include "GameScene.h"
#include "Utility.h"
#include "DxLib.h"
#include "Master.h"
#include "inputManager.h"
#include "SkyBox.h"
#include "Player3D.h"
#include "Stage.h"
#include "Enemy3D.h"
#include "ObjectManager.h"
#include "SceneManager.h"
#include <fstream>

namespace
{
	Stage* CreateStage(
		int modelHandle,
		int collHandle,
		const char* modelPath,
		const char* collPath,
		float scale,
		VECTOR pos,
		Stage::StageType type = Stage::StageType::Static)
	{
		Stage* stage = nullptr;

		if (modelHandle != -1 && collHandle != -1)
		{
			stage = new Stage(modelHandle, collHandle, pos, type);
		}
		else
		{
			stage = new Stage(modelPath, collPath, pos, type);
		}

		stage->SetScale(scale);
		return stage;
	}
}

//--------------------------------------------------
// コンストラクタ
//--------------------------------------------------
GameScene::GameScene()
	: Scene()
	, mvGoalPosition(VGet(15590.0f, 2600.0f, 14430.0f))
{
	SetFontSize(60);

	mnGoalHandle = LoadGraph("Resource/2D/Goal.png");
	if (mnGoalHandle == -1) printfDx("ゴール画像ない");
	mnOptionHandle = LoadGraph("Resource/2D/Option.png");
	if (mnOptionHandle == -1) printfDx("操作画像ない");
}

GameScene::~GameScene() {}


//--------------------------------------------------
// 初期化
//--------------------------------------------------
void GameScene::Initialize()
{
	m_enemyList.clear();
	m_savedEnemyList.clear();
	m_selectedId = 0;

	Master::mpCamera->Reset();
	Master::mpCamera->SetTitleMode(false);

	// SkyBox
	if (Master::mnSkyModelHandle != -1)
	{
		auto* sky = new SkyBox(Master::mnSkyModelHandle);
		sky->SetScale(12.0f);
	}
	else
	{
		auto* sky = new SkyBox("Resource/3D/SkyBox/sky.mqo");
		sky->SetScale(12.0f);
	}

	Player3D* player = new Player3D(VGet(0, 100, 0), "Resource/3D/Player/octopus.mqo");


	std::vector<EnemySpawnData> enemySpawnList =
	{
		{ VGet(3359.66f,  -9.249f,   -3.62714f), Enemy3D::EnemyType::Jumper }, // ID:0
		{ VGet(9068.31f,  126.234f,  56.1533f),  Enemy3D::EnemyType::Runner }, // ID:1
		{ VGet(9524.75f,  126.234f,  667.882f),  Enemy3D::EnemyType::Jumper }, // ID:2
		{ VGet(9280.21f,  126.234f,  1281.94f),  Enemy3D::EnemyType::Runner }, // ID:3
		{ VGet(9812.92f,  891.537f,  3717.11f),  Enemy3D::EnemyType::Runner }, // ID:4
		{ VGet(9238.21f,  891.538f,  4081.83f),  Enemy3D::EnemyType::Attacker }, // ID:5
		{ VGet(4149.27f,  1520.65f,  3620.09f),  Enemy3D::EnemyType::Runner }, // ID:6
		{ VGet(4685.61f,  1520.65f,  4264.24f),  Enemy3D::EnemyType::Attacker }, // ID:7
		{ VGet(4872.89f,  1516.53f,  9233.34f),  Enemy3D::EnemyType::Runner }, // ID:8
		{ VGet(3907.02f,  1516.53f,  9254.33f),  Enemy3D::EnemyType::Attacker }, // ID:9
		{ VGet(7761.36f,  1265.33f,  8820.87f),  Enemy3D::EnemyType::Runner }, // ID:10
		{ VGet(7865.66f,  1265.33f,  9675.2f),   Enemy3D::EnemyType::Attacker }, // ID:11
		{ VGet(7258.23f,  1265.33f,  9237.93f),  Enemy3D::EnemyType::Attacker }, // ID:12
		{ VGet(15778.7f,  2254.78f,  9494.45f),  Enemy3D::EnemyType::Runner }, // ID:13
		{ VGet(15231.2f,  2254.78f,  8963.72f),  Enemy3D::EnemyType::Attacker }, // ID:14
		{ VGet(15390.4f,  2318.83f,  10858.9f),  Enemy3D::EnemyType::Attacker }  // ID:15
	};

	int id = 0;
	for (auto& spawn : enemySpawnList)
	{
		Enemy3D* enemy = new Enemy3D(spawn.pos, spawn.type);
		enemy->SetID(id++);
		m_enemyList.push_back(enemy);
	}

	// 土台ステージ
	CreateStage(
		Master::mnStageModelHandle,
		Master::mnStageCollisionHandle,
		"Resource/3D/Stage1/stage.mqo",
		"Resource/3D/Stage1/stageColl.mqo",
		5.0f,
		VGet(0.0f, 0.0f, 0.0f),
		Stage::StageType::Static
	);

	// 左右移動ステージ
	CreateStage(
		Master::mnSlideStageHandle,
		Master::mnSlideStageCollHandle,
		"Resource/3D/Stage1/slideStage.mqo",
		"Resource/3D/Stage1/slideStage.mqo",
		5.0f,
		VGet(0.0f, 0.0f, 0.0f),
		Stage::StageType::MoveSide
	);

	// 上下移動ステージ
	CreateStage(
		Master::mnUpdownStageHandle,
		Master::mnUpdownStageCollHandle,
		"Resource/3D/Stage1/updownStage.mqo",
		"Resource/3D/Stage1/updownStage.mqo",
		5.0f,
		VGet(0.0f, 0.0f, 0.0f),
		Stage::StageType::MoveUpDown
	);

	// 回転ステージ
	Stage* rotaBase = CreateStage(
		Master::mnRotaStageHandle,
		Master::mnRotaStageCollHandle,
		"Resource/3D/Stage1/rotaStage.mqo",
		"Resource/3D/Stage1/rotaStage.mqo",
		5.0f,
		VGet(4500.0f, 1520.0f, 6877.0f),
		Stage::StageType::Rotate
	);
	rotaBase->SetRotateParam(
		1.1f,                            // speed
		0.18f,                           // range
		Stage::RotateAxis::Z             // 軸
	);



	// でかめの床	   手前
	Stage* rota1 = new Stage("Resource/3D/Stage1/rotaStage1.mqo", "Resource/3D/Stage1/rotaStage1.mqo"
		, VGet(7605.0f, 600.0f, 3800.0f),Stage::StageType::Rotate);
	rota1->SetScale(5.0f);

	rota1->SetRotateParam(
		2.7f,                            // speed
		0.2f,                           // range
		Stage::RotateAxis::X             // 軸
	);



	// でかめの床	   奥
	Stage* rota2 = new Stage("Resource/3D/Stage1/rotaStage2.mqo", "Resource/3D/Stage1/rotaStage2.mqo"
		, VGet(7605.0f, 600.0f, 3800.0f),Stage::StageType::Rotate);
	rota2->SetScale(5.0f);

	rota2->SetRotateParam(
		1.3f,                            // speed
		0.40f,                           // range
		Stage::RotateAxis::X             // 軸
	);



   // ------ フォグ設定 ------
	SetFogEnable(TRUE); // フォグ有効
	SetFogMode(DX_FOGMODE_LINEAR); // 線形フォグ
	SetFogStartEnd(7500.0f, 14000.0f); // 開始距離と終了距離
	SetFogColor(255, 150, 80);
	SetFogDensity(0.001f);

}


void GameScene::Update()
{
	Master::mpCamera->SetStop(false); // カメラ停止

	// プレイヤー取得 & 勝敗判定
	auto playerList =
		Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()
		->GetObject3DListByTag(Object3D::T_Player3D);

	bool isAlive = false;

	for (auto* obj : playerList)
	{
		Player3D* player = dynamic_cast<Player3D*>(obj);
		if (!player || player->IsDeleteFlag()) continue;

		isAlive = true;


		// ゴール判定 (球判定)
		VECTOR playerPos = player->GetPosition();

		// プレイヤーとゴール間の距離を計算
		float dist = VSize(VSub(playerPos, mvGoalPosition));

		if (dist < mfGoalRadius)
		{
			// ゴール
			Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::WIN_RESULT_3D);
			return;
		}

		// 落下敗北
		if (player->GetPosition().y < -3000.0f)
		{
			Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::LOSE_RESULT_3D);
			return;
		}
	}

	// 全滅チェック
	if (!isAlive)
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::LOSE_RESULT_3D);
		return;
	}

	SaveEnemyDataToFile(); // 位置をファイル保存デバッグで使用した

	Scene::Update();
}


// 描画
void GameScene::Draw()
{
	/*DrawFormatString(
		0, 0,
		GetColor(255, 255, 255),
		"Selected Enemy ID: %d / %d",
		m_selectedId,
		(int)m_enemyList.size() - 1
	);*/


	Scene::Draw();


	//少し透けて描画する
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 230);
	DrawGraph(0, 0, mnOptionHandle, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);




	//ゴールに画像を表示
	VECTOR goalPos = VGet(15590.0f, 3100.0f, 14430.0f);
	// DrawBillboard3D で描画
	DrawBillboard3D(goalPos, 0.5f, 0.5f, 800.0f, 0.0f, mnGoalHandle, TRUE);

	// goalのあたり判定デバッグ用
	// DrawSphere3D(mvGoalPosition, mfGoalRadius, 16, GetColor(0, 255, 0), GetColor(0, 255, 0), FALSE);
}


// 終了
void GameScene::Finalize()
{
	if (Master::mpCamera)
	{
		Master::mpCamera->Reset();
	}
}


// 敵データ保存
void GameScene::SaveEnemyDataToFile()
{
	// 保存するtxt名前
	std::ofstream ofs("EnemyPlacement.txt");
	if (!ofs) return;

	// 順に入力
	for (auto& d : m_savedEnemyList)
	{
		ofs << "ID: " << d.id
			<< " Pos: "
			<< d.pos.x << ", "
			<< d.pos.y << ", "
			<< d.pos.z
			<< std::endl;
	}
}