#include "GameScene.h"
#include "Utility.h" // 呼び出すと、SCREEN_WIDTHとかを使える
#include "DxLib.h"
#include "Master.h"
#include "inputManager.h"
#include "SkyBox.h"
#include "Player3D.h"
#include "Stage.h"
#include "Enemy3D.h"
#include "ObjectManager.h"
#include "SceneManager.h"
#include <fstream> // ファイル出力用に必要

// namespace を使うことで外部ファイルから参照されなくなる
namespace
{
	//---------------------------------------------------------
	// ステージ生成共通処理
	// ・共有モデルハンドルが存在する場合
	//       ローディングシーンで読み込んだモデルを使用
	// ・共有ハンドルが無い場合
	//       ファイルから直接ロードして生成
	// ・生成後にスケール設定を行う
	//---------------------------------------------------------

	Stage* CreateStage(
		int modelHandle,       // 描画用モデルの共有ハンドル（-1なら未ロード）
		int collHandle,		   // 当たり判定用モデルの共有ハンドル（-1なら未ロード）
		const char* modelPath, // 描画用モデルのファイルパス（共有ハンドルが無い場合に使用）
		const char* collPath,  // 当たり判定用モデルのファイルパス（共有ハンドルが無い場合に使用）
		float scale,
		Stage::StageType type = Stage::StageType::Static)
	{
		Stage* stage = nullptr;

		// 共有ハンドルが存在する場合
		if (modelHandle != -1 && collHandle != -1)
		{
			stage = new Stage(modelHandle, collHandle, type);
		}
		// 無い場合はファイルから直接生成
		else
		{
			stage = new Stage(modelPath, collPath, type);
		}

		// スケール設定
		stage->SetScale(scale);

		return stage;
	}
}


GameScene::GameScene()
	: Scene()     // 基底クラスのコンストラクタを呼び出しておく
{
	SetFontSize(60); // 文字の大きさ
}

GameScene::~GameScene()
{

}

Enemy3D::EnemyType GameScene::GetEnemyTypeFromID(int id)
{
	if (id >= 0 && id <= 4)
	{
		return Enemy3D::Attacker;
	}
	else if (id <= 7)
	{
		return Enemy3D::Jumper;
	}
	else
	{
		return Enemy3D::Runner;
	}
}

void GameScene::Initialize()
{
	m_recordedEnemyPositions.clear(); // 初期化


	Master::mpCamera->Reset();
	Master::mpCamera->SetTitleMode(false); // タイトルモードを解除

	SkyBox* pSkyBox = new SkyBox("Resource/3D/SkyBox/sky.mqo");
	pSkyBox->SetScale(10.0f);  // 最大で9.0ぐらい？ 1222

	// プレイヤーの生成
	Player3D* player = new Player3D(VGet(0, 100, 0), "Resource/3D/Player/octopus.mqo");

	// 敵の生成リスト
	std::vector<VECTOR> enemyPosList =
	{
		VGet(-5.86604f, -146.696f, 2920.01f),
		VGet(204.082f,  -146.696f, 4660.06f),
		VGet(-203.793f,  -78.395f, 5530.63f),
		VGet(213.696f,   -78.395f, 5533.55f),
		VGet(22.8382f,   -78.395f, 6059.9f),
		VGet(1.87232f,  -629.002f, 8687.09f),
		VGet(346.933f,  -629.002f, 9014.21f),
		VGet(-320.357f, -629.002f, 9121.35f),
		VGet(6.37415f,  -472.422f, 11401.8f)
	};

	int count = 0;
	for (const auto& pos : enemyPosList)
	{
		Enemy3D::EnemyType type = GetEnemyTypeFromID(count);

		Enemy3D* enemy = new Enemy3D(pos, type);
		enemy->SetID(count++);
		m_enemyList.push_back(enemy);
	}

	// 共有ハンドルがあればハンドル版コンストラクタを使う（ローディングシーンで読み込まれている）
	if (Master::mnSkyModelHandle != -1)
	{
		SkyBox* pSkyBox = new SkyBox(Master::mnSkyModelHandle);
		pSkyBox->SetScale(10.0f);
	}
	else
	{
		SkyBox* pSkyBox = new SkyBox("Resource/3D/SkyBox/sky.mqo");
		pSkyBox->SetScale(10.0f);
	}


	float scale = 5.0f;

	// 通常ステージ
	CreateStage(
		Master::mnStageModelHandle,
		Master::mnStageCollisionHandle,
		"Resource/3D/Stage1/stage.mqo",
		"Resource/3D/Stage1/stageColl.mqo",
		scale
	);

	//// 移動ステージ
	//CreateStage(
	//	Master::mnStageMoveHandle,
	//	Master::mnStageMoveCollHandle,
	//	"Resource/3D/Stage1/moveStage.mqo",
	//	"Resource/3D/Stage1/moveStageColl.mqo",
	//	scale,
	//	Stage::StageType::Moving
	//);

	//// 回転ステージ
	//CreateStage(
	//	Master::mnStageRotaHandle,
	//	Master::mnStageRotaCollHandle,
	//	"Resource/3D/Stage1/rotaStage.mqo",
	//	"Resource/3D/Stage1/rotaStageColl.mqo",
	//	scale,
	//	Stage::StageType::Rotating
	//);

	//// 小回転ステージ
	//CreateStage(
	//	Master::mnStageLittleRotaHandle,
	//	Master::mnStageLittleCollRotaHandle,
	//	"Resource/3D/Stage1/littleRota.mqo",
	//	"Resource/3D/Stage1/littleRotaColl.mqo",
	//	scale,
	//	Stage::StageType::LittleRotation
	//);
}

void GameScene::Update()
{
	// ショートカットで次シーン指定
	if (InputManager::CheckDownKey(KEY_INPUT_9))
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::WIN_RESULT_3D);
	}

	auto pPlayerList =
		Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()->GetObject3DListByTag(Object3D::T_Player3D);


	// プレイヤーが生きているかのフラグ
	bool isPlayerAlive = false;

	// プレイヤー一覧を走査して「生存判定」と「勝敗条件」をチェックする
	for (auto* obj : pPlayerList)
	{
		Player3D* player = dynamic_cast<Player3D*>(obj);
		if (!player) continue;

		// HPがある & 削除フラグが立っていない＝生存中
		if (player && !player->IsDeleteFlag())
		{
			isPlayerAlive = true;

			// 勝利条件チェック
			if (player->GetPosition().x > 15500.0f
				&& player->GetPosition().y > 1050
				&& player->GetPosition().z > 14400.0f)
			{
				Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::WIN_RESULT_3D);
				return;
			}

			// 敗北条件チェック（落下）
			if (player->GetPosition().y < -3000.0f)
			{
				Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::LOSE_RESULT_3D);
				return;
			}
		}
	}

	// プレイヤーが1人も生存していない場合は敗北
	if (!isPlayerAlive)
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::LOSE_RESULT_3D);
		return;
	}


	// 3キーが押されたらプレイヤーの現在位置を記録
	if (InputManager::CheckDownKey(KEY_INPUT_3))
	{
		// 現在のシーン自身の ObjectManager を直接参照する
		auto pPlayerList = this->GetObjectManager()->GetObject3DListByTag(Object3D::T_Player3D);

		if (pPlayerList.empty()) {
			printf("デバッグ: プレイヤーが見つかりません\n");
		}

		for (auto* obj : pPlayerList)
		{
			Player3D* player = dynamic_cast<Player3D*>(obj);
			if (player && m_recordedEnemyPositions.size() < 20)
			{
				m_recordedEnemyPositions.push_back(player->GetPosition());
				printf("Recorded: %d / 20\n", (int)m_recordedEnemyPositions.size());

				if (m_recordedEnemyPositions.size() == 20)
				{
					SaveEnemyDataToFile();
				}
			}
		}
	}

	// 基底クラスの更新処理（Scene::Update は内部で ObjectManager を Update/Draw する想定）
	Scene::Update();
}

void GameScene::Draw()
{
	Scene::Draw();
}

void GameScene::Finalize()
{
	// 次のシーンへ行く前にカメラを強制的にリセットする
	if (Master::mpCamera)
	{
		Master::mpCamera->Reset();
	}
}

void GameScene::SaveEnemyDataToFile()
{
	std::ofstream ofs("EnemyPlacement.txt");
	if (!ofs) return;

	for (const auto& pos : m_recordedEnemyPositions)
	{
		// 座標をテキストファイルに書き出す
		ofs << "VGet(" << pos.x << "f, " << pos.y << "f, " << pos.z << "f)," << std::endl;
	}

	ofs.close();
	printf("データを EnemyPlacement.txt に保存しました\n");
}

//float scale = 2.3f; // ステージの拡大率
//Stage* stage = new Stage ("Resource/3D/Stage1/stage.mqo", "Resource/3D/Stage1/stage.mqo");
//Stage* moveStage = new Stage ("Resource/3D/Stage1/moveStage.mqo", "Resource/3D/Stage1/moveStage.mqo", Stage::StageType::Moving);
//Stage* rotaStage = new Stage ("Resource/3D/Stage1/rotaStage.mqo", "Resource/3D/Stage1/rotaStage.mqo", Stage::StageType::Rotating);
//Stage* littleRotaStage = new Stage ("Resource/3D/Stage1/littleRota.mqo", "Resource/3D/Stage1/littleRota.mqo", Stage::StageType::LittleRotation);
//stage->SetScale(scale);
//moveStage->SetScale(scale);
//rotaStage->SetScale(scale);
//littleRotaStage->SetScale(scale);
