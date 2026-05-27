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
		Stage::StageType type = Stage::StageType::Static)
	{
		Stage* stage = nullptr;

		if (modelHandle != -1 && collHandle != -1)
		{
			stage = new Stage(modelHandle, collHandle, type);
		}
		else
		{
			stage = new Stage(modelPath, collPath, type);
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
{
	SetFontSize(60);
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
		sky->SetScale(10.0f);
	}
	else
	{
		auto* sky = new SkyBox("Resource/3D/SkyBox/sky.mqo");
		sky->SetScale(10.0f);
	}

	// Player
	Player3D* player = new Player3D(VGet(0, 100, 0), "Resource/3D/Player/octopus.mqo");

	// Enemy spawn
	std::vector<VECTOR> enemyPosList =
	{
		VGet(-5.86604f, -146.696f, 2920.01f),   // ID:0
		VGet(204.082f,  -146.696f, 4660.06f),   // ID:1
		VGet(-203.793f, -78.395f, 5530.63f),    // ID:2
		VGet(213.696f,  -78.395f, 5533.55f),    // ID:3
		VGet(22.8382f,  -78.395f, 6059.9f),     // ID:4
		VGet(1.87232f,  -629.002f, 8687.09f),   // ID:5
		VGet(346.933f,  -629.002f, 9014.21f),   // ID:6
		VGet(-320.357f, -629.002f, 9121.35f),   // ID:7

		VGet(-320.357f, -629.002f, 9121.35f),   // ID:8 (duplicate)
		VGet(-320.357f, -629.002f, 9121.35f),   // ID:9
		VGet(-320.357f, -629.002f, 9121.35f),   // ID:10
		VGet(-320.357f, -629.002f, 9121.35f),   // ID:11
		VGet(-320.357f, -629.002f, 9121.35f),   // ID:12
		VGet(-320.357f, -629.002f, 9121.35f),   // ID:13
		VGet(-320.357f, -629.002f, 9121.35f),   // ID:14

		VGet(6.37415f,  -472.422f, 11401.8f)    // ID:15
	};

	int id = 0;
	for (auto& pos : enemyPosList)
	{
		Enemy3D* enemy = new Enemy3D(pos, Enemy3D::EnemyType::Attacker);
		enemy->SetID(id++);
		m_enemyList.push_back(enemy);
	}

	// Stage
	CreateStage(
		Master::mnStageModelHandle,
		Master::mnStageCollisionHandle,
		"Resource/3D/Stage1/stage.mqo",
		"Resource/3D/Stage1/stageColl.mqo",
		5.0f
	);

	//// 移動ステージ
	//CreateStage(
	//	Master::mnStageMoveHandle,
	//	Master::mnStageMoveCollHandle,
	//	"Resource/3D/Stage1/moveStage.mqo",
	//	"Resource/3D/Stage1/moveStageColl.mqo",
	//	5.0f,
	//	Stage::StageType::Moving
	//);

	//// 回転ステージ
	//CreateStage(
	//	Master::mnStageRotaHandle,
	//	Master::mnStageRotaCollHandle,
	//	"Resource/3D/Stage1/rotaStage.mqo",
	//	"Resource/3D/Stage1/rotaStageColl.mqo",
	//	5.0f,
	//	Stage::StageType::Rotating
	//);

	//// 小回転ステージ
	//CreateStage(
	//	Master::mnStageLittleRotaHandle,
	//	Master::mnStageLittleCollRotaHandle,
	//	"Resource/3D/Stage1/littleRota.mqo",
	//	"Resource/3D/Stage1/littleRotaColl.mqo",
	//	5.0f,
	//	Stage::StageType::LittleRotation
	//);
}


//--------------------------------------------------
// 更新
//--------------------------------------------------
void GameScene::Update()
{
	//--------------------------------------------------
	// シーン切り替えショートカット
	//--------------------------------------------------
	if (InputManager::CheckDownKey(KEY_INPUT_9))
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::WIN_RESULT_3D);
	}

	//--------------------------------------------------
	// プレイヤー取得 & 勝敗判定
	//--------------------------------------------------
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

		// 勝利条件
		if (player->GetPosition().x > 15500.0f &&
			player->GetPosition().y > 1050.0f &&
			player->GetPosition().z > 14400.0f)
		{
			Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::WIN_RESULT_3D);
			return;
		}

		// 落下敗北
		if (player->GetPosition().y < -3000.0f)
		{
			Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::LOSE_RESULT_3D);
			return;
		}

		//--------------------------------------------------
		// ★エディタ機能（ID選択＆配置）
		//--------------------------------------------------
		if (InputManager::CheckDownKey(KEY_INPUT_6)) m_selectedId--;
		if (InputManager::CheckDownKey(KEY_INPUT_7)) m_selectedId++;

		if (!m_enemyList.empty())
		{
			if (m_selectedId < 0) m_selectedId = 0;
			if (m_selectedId >= (int)m_enemyList.size())
				m_selectedId = (int)m_enemyList.size() - 1;
		}

		if (InputManager::CheckDownKey(KEY_INPUT_3))
		{
			if (m_selectedId >= 0 && m_selectedId < (int)m_enemyList.size())
			{
				VECTOR pos = player->GetPosition();

				// 敵をプレイヤー位置へ移動
				m_enemyList[m_selectedId]->SetPosition(pos);

				// 保存
				bool found = false;
				for (auto& d : m_savedEnemyList)
				{
					if (d.id == m_selectedId)
					{
						d.pos = pos;
						found = true;
						break;
					}
				}

				if (!found)
				{
					m_savedEnemyList.push_back({ m_selectedId, pos });
				}
			}
		}
	}

	// 全滅チェック
	if (!isAlive)
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::LOSE_RESULT_3D);
		return;
	}

	//--------------------------------------------------
	// 保存
	//--------------------------------------------------
	SaveEnemyDataToFile();

	Scene::Update();
}


//--------------------------------------------------
// 描画
//--------------------------------------------------
void GameScene::Draw()
{
	DrawFormatString(
		0, 0,
		GetColor(255, 255, 255),
		"Selected Enemy ID: %d / %d",
		m_selectedId,
		(int)m_enemyList.size() - 1
	);

	Scene::Draw();
}


//--------------------------------------------------
// 終了
//--------------------------------------------------
void GameScene::Finalize()
{
	if (Master::mpCamera)
	{
		Master::mpCamera->Reset();
	}
}


//--------------------------------------------------
// 敵データ保存
//--------------------------------------------------
void GameScene::SaveEnemyDataToFile()
{
	std::ofstream ofs("EnemyPlacement.txt");
	if (!ofs) return;

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