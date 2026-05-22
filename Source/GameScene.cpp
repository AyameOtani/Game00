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

void GameScene::Initialize()
{
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
		Enemy3D* enemy = new Enemy3D(pos, "Resource/3D/Enemy/broccoli.mqo", Enemy3D::EnemyType::Attacker);
		enemy->SetID(count++);
		m_enemyList.push_back(enemy);
	}

	Stage* stage = new Stage ("Resource/3D/Stage1/stage.mqo", "Resource/3D/Stage1/stage.mqo");
	stage->SetScale(2.3f);

	Master::mpCamera->Reset();
}

void GameScene::Update()
{
	// ショートカットで次シーン指定
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::WIN_RESULT_3D);
	}

	// --- 安全対策 ---
	// ObjectManager から直接返されるリストは将来フレーム中に変わり得る（削除など）。
	// そのためローカルコピー（スナップショット）を作ることで、以降の処理で不整合を防ぐ。
	auto rawPlayerList =
		Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()->GetObject3DListByTag(Object3D::T_Player3D);

	auto rawEnemyList =
		Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()->GetObject3DListByTag(Object3D::T_Enemy3D);

	// スナップショット（コピー）を作成
	std::vector<Object3D*> players = rawPlayerList;
	std::vector<Object3D*> enemies = rawEnemyList;

	bool playersEmpty = players.empty();
	bool enemiesEmpty = enemies.empty();

	// 両方空 or いずれか空 の場合、安全に一度だけシーン切替を予約して早期リターン
	if (playersEmpty || enemiesEmpty)
	{
		// 優先度：プレイヤーが居なければLOSE、それ以外は敵が居なければWIN
		if (playersEmpty && !enemiesEmpty)
		{
			Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::LOSE_RESULT_3D);
		}
		else
		{
			// 敵が居ない、または両方居ない場合は勝利シーンに遷移（要件で変更可）
			Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::WIN_RESULT_3D);
		}

		// 早期 return して以降で削除済みオブジェクトへ触らないようにする
		return;
	}


	// ここ以降は players, enemies を使って参照・処理する（オブジェクトはまだ存在する想定）
	// 例：生存チェックなど
	// SaveEnemyDataToFile は内部で m_enemyList 等を参照するなら安全な実装へ変更してください
	SaveEnemyDataToFile();

	// 基底クラスの更新処理（Scene::Update は内部で ObjectManager を Update/Draw する想定）
	Scene::Update();
}

void GameScene::Draw()
{
	Scene::Draw();
}

void GameScene::Finalize()
{

}

void GameScene::SaveEnemyDataToFile()
{
	// 必要なら安全に参照できるデータのみ書き出す（ポインタ参照は避ける）
	// 現状は何もしない（コメントアウトしてある）
}