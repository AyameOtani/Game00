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



	//リストにして生成
	std::vector<VECTOR> enemyPosList =
	{
		VGet(-5.86604f, -146.696f, 2920.01f),   // ID:0
		VGet(204.082f,  -146.696f, 4660.06f),   // ID:1
		VGet(-203.793f,  -78.395f, 5530.63f),   // ID:2
		VGet(213.696f,   -78.395f, 5533.55f),   // ID:3
		VGet(22.8382f,   -78.395f, 6059.9f),    // ID:4
		VGet(1.87232f,  -629.002f, 8687.09f),   // ID:5
		VGet(346.933f,  -629.002f, 9014.21f),   // ID:6
		VGet(-320.357f, -629.002f, 9121.35f),   // ID:7
		VGet(6.37415f,  -472.422f, 11401.8f)    // ID:8
	};


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

	//// プレイヤーが一人でも存在する場合のみ操作可能にする
	//if (!playerList.empty())
	//{
	//	// リストの先頭をプレイヤーとして扱う
	//	Player3D* pPlayer = dynamic_cast<Player3D*>(playerList[0]);

	//	// --- ID切り替えの操作 (6キーで戻る, 7キーで進む) ---
	//	if (InputManager::CheckDownKey(KEY_INPUT_6)) m_selectedId--;
	//	if (InputManager::CheckDownKey(KEY_INPUT_7)) m_selectedId++;

	//	// 範囲制限
	//	if (m_selectedId < 0) m_selectedId = 0;
	//	if (m_selectedId >= (int)m_enemyList.size()) m_selectedId = (int)m_enemyList.size() - 1;

	//	// --- 3キーで移動処理 ---
	//	// 3キーが押された時の移動処理内
	//	if (InputManager::CheckDownKey(KEY_INPUT_3))
	//	{
	//		if (m_selectedId >= 0 && m_selectedId < (int)m_enemyList.size())
	//		{
	//			// 敵の座標を移動
	//			m_enemyList[m_selectedId]->SetPosition(pPlayer->GetPosition());

	//			// 保存用リストを更新する（もし既にIDがあれば更新、なければ追加）
	//			bool found = false;
	//			for (auto& data : m_savedEnemyList) {
	//				if (data.id == m_selectedId) {
	//					data.pos = pPlayer->GetPosition();
	//					found = true;
	//					break;
	//				}
	//			}
	//			if (!found) {
	//				m_savedEnemyList.push_back({ m_selectedId, pPlayer->GetPosition() });
	//			}
	//		}
	//	}
	//}

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
	//DrawFormatString(0, 0, GetColor(255, 255, 255), "Selected Enemy ID: %d / %d", m_selectedId, (int)m_enemyList.size() - 1);

	// 基底クラスの更新処理を呼びだす
	Scene::Draw();
}


void GameScene::Finalize()
{

}

void GameScene::SaveEnemyDataToFile()
{
	//std::ofstream ofs("EnemyPlacement.txt");
	//for (const auto& data : m_savedEnemyList)
	//{
	//	// IDと座標をテキストで保存
	//	ofs << "ID: " << data.id
	//		<< " Pos: " << data.pos.x << ", " << data.pos.y << ", " << data.pos.z
	//		<< std::endl;
	//}
}