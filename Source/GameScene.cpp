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
	// Masterに渡したものをStatus* statusのやつに設定しているからこんな引数
	Player3D* player = new Player3D(VGet(0, 0, 0), "Resource/3D/Player/octopus.mqo");

	// 敵の生成
	//Enemy3D* enemy = new Enemy3D(VGet(0, 0, 0), "Resource/3D/Enemy/broccoli.mqo", Enemy3D::EnemyType::Attacker);



	//// 【ここをリストにして自動生成】
	//std::vector<VECTOR> enemyPosList =
	//{
	//		{0, VGet(0.0f,      -146.696f, 2863.33f)},
	//{1, VGet(205.116f,  -146.696f, 4645.31f)},
	//{2, VGet(233.333f,   -78.395f, 5563.38f)},
	//{3, VGet(-207.501f,  -78.395f, 5563.38f)},
	//{4, VGet(15.7844f,   -78.395f, 6122.25f)},
	//{5, VGet(377.92f,   -629.002f, 9382.4f)},
	//{6, VGet(-201.994f, -629.002f, 9393.34f)},
	//{7, VGet(42.196f,   -629.002f, 8728.18f)},
	//{8, VGet(394.061f,  -629.002f, 8677.92f)}
	//};


//ID: 0 Pos : 0, -146.696, 2863.33
//ID : 1 Pos : 205.116, -146.696, 4645.31
//ID : 2 Pos : 233.333, -78.395, 5563.38
//ID : 3 Pos : -207.501, -78.395, 5563.38
//ID : 4 Pos : 15.7844, -78.395, 6122.25
//ID : 5 Pos : 377.92, -629.002, 9382.4
//ID : 6 Pos : -201.994, -629.002, 9393.34
//ID : 7 Pos : 42.196, -629.002, 8728.18
//ID : 8 Pos : 394.061, -629.002, 8677.92


	int count = 0;
	for (const auto& pos : enemyPosList)
	{
		Enemy3D* enemy = new Enemy3D(pos, "Resource/3D/Enemy/broccoli.mqo", Enemy3D::EnemyType::Attacker);
		enemy->SetID(count++); // 0, 1, 2...とIDを振る
		m_enemyList.push_back(enemy); // 管理リストに入れる
	}



	Stage* stage = new Stage ("Resource/3D/Stage1/stage.mqo", "Resource/3D/Stage1/stage.mqo");
	stage->SetScale(2.3f); // 大きさ調整

	// ここでカメラを元の位置に戻している
	Master::mpCamera->Reset();
}



void GameScene::Update()
{
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::WIN_RESULT_3D);
	}

	// --- プレイヤーを取得する ---
	auto playerList = Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()->GetObject3DListByTag(Object3D::T_Player3D);

	// プレイヤーが一人でも存在する場合のみ操作可能にする
	if (!playerList.empty())
	{
		// リストの先頭をプレイヤーとして扱う
		Player3D* pPlayer = dynamic_cast<Player3D*>(playerList[0]);

		// --- ID切り替えの操作 (6キーで戻る, 7キーで進む) ---
		if (InputManager::CheckDownKey(KEY_INPUT_6)) m_selectedId--;
		if (InputManager::CheckDownKey(KEY_INPUT_7)) m_selectedId++;

		// 範囲制限
		if (m_selectedId < 0) m_selectedId = 0;
		if (m_selectedId >= (int)m_enemyList.size()) m_selectedId = (int)m_enemyList.size() - 1;

		// --- 3キーで移動処理 ---
		// 3キーが押された時の移動処理内
		if (InputManager::CheckDownKey(KEY_INPUT_3))
		{
			if (m_selectedId >= 0 && m_selectedId < (int)m_enemyList.size())
			{
				// 敵の座標を移動
				m_enemyList[m_selectedId]->SetPosition(pPlayer->GetPosition());

				// 保存用リストを更新する（もし既にIDがあれば更新、なければ追加）
				bool found = false;
				for (auto& data : m_savedEnemyList) {
					if (data.id == m_selectedId) {
						data.pos = pPlayer->GetPosition();
						found = true;
						break;
					}
				}
				if (!found) {
					m_savedEnemyList.push_back({ m_selectedId, pPlayer->GetPosition() });
				}
			}
		}
	}

	SaveEnemyDataToFile();
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

	// 現在の選択IDを表示
	DrawFormatString(0, 0, GetColor(255, 255, 255), "Selected Enemy ID: %d / %d", m_selectedId, (int)m_enemyList.size() - 1);

	// 基底クラスの更新処理を呼びだす
	Scene::Draw();
}


void GameScene::Finalize()
{

}

void GameScene::SaveEnemyDataToFile()
{
	std::ofstream ofs("EnemyPlacement.txt");
	for (const auto& data : m_savedEnemyList)
	{
		// IDと座標をテキストで保存
		ofs << "ID: " << data.id
			<< " Pos: " << data.pos.x << ", " << data.pos.y << ", " << data.pos.z
			<< std::endl;
	}
}