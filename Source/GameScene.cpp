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
		return Enemy3D::Attacker;
	}
}

void GameScene::Initialize()
{
	Master::mpCamera->Reset();
	Master::mpCamera->SetTitleMode(false); // タイトルモードを解除

	SkyBox* pSkyBox = new SkyBox("Resource/3D/SkyBox/sky.mqo");
	pSkyBox->SetScale(10.0f);  // 最大で9.0ぐらい？ 1222

	// プレイヤーの生成
	Player3D* player = new Player3D(VGet(0, 0, 0), "Resource/3D/Player/octopus.mqo");

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

	float scale = 2.3f; // ステージの拡大率
	Stage* stage = new Stage ("Resource/3D/Stage1/stage.mqo", "Resource/3D/Stage1/stage.mqo");
	Stage* moveStage = new Stage ("Resource/3D/Stage1/moveStage.mqo", "Resource/3D/Stage1/moveStage.mqo", Stage::StageType::Moving);
	Stage* rotaStage = new Stage ("Resource/3D/Stage1/rotaStage.mqo", "Resource/3D/Stage1/rotaStage.mqo", Stage::StageType::Rotating);
	Stage* littleRotaStage = new Stage ("Resource/3D/Stage1/littleRota.mqo", "Resource/3D/Stage1/littleRota.mqo", Stage::StageType::LittleRotation);
	stage->SetScale(scale);
	moveStage->SetScale(scale);
	rotaStage->SetScale(scale);
	littleRotaStage->SetScale(scale);
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
			if (player->GetPosition().z > 15500.0f &&
				player->GetPosition().y > 500)
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
	// 必要なら安全に参照できるデータのみ書き出す（ポインタ参照は避ける）
	// 現状は何もしない（コメントアウトしてある）
}