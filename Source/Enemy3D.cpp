#include "Enemy3D.h"
#include "Model.h"
#include "Bullet3D.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "ObjectManager.h"
#include <cmath>


int Enemy3D::s_nextEnemyID = 0;

// コンストラクタ
Enemy3D::Enemy3D(VECTOR initPos, EnemyType type)
	: Character3D(initPos, 1, Team::Enemy, 60.0f)
	, m_type(type)
	, mfAngle(0.0f)
	, mfTargetAngle(0.0f)
	, mfShotTimer(0.0f)
	, mpModel(nullptr)
{
	SetTag(Object3D::T_Enemy3D);

	// 敵のパスを読み込む
	std::string filename = GetModelPath(type);
	// モデル生成
	mpModel = new Model(filename, initPos);

	// 判定用パラメータを派生で設定（天井含む）
	m_radius = 60.0f;
	m_ceilRadius = 75.0f; // 天井判定用半径

	this->mfJumpPower = 30.0f;
	// 床判定
	m_floorCapsuleMinY = 3.0f;
	m_floorCapsuleMaxY = 40.0f;
	m_floorLinePos = 30.0f;
	m_floorLineMinY = 60.0f;
	m_floorLineMaxY = -100.0f;

	// 壁判定
	m_wallCapsuleMinY = 40.0f;
	m_wallCapsuleMaxY = 60.0f;

	// 天井判定
	m_ceilCapsuleMinY = 50.0f;
	m_ceilCapsuleMaxY = 80.0f;
	m_ceilLinePos = 30.0f;
	m_ceilLineMinY = 70.0f;
	m_ceilLineMaxY = 100.0f;


	m_id = s_nextEnemyID++;

	// 動く敵の位置保存
	mvBasePosition = initPos;
	mvTargetPosition = initPos;

	SetFontSize(20);
}

// タイプによってモデルを分ける処理
std::string Enemy3D::GetModelPath(Enemy3D::EnemyType type)
{
	switch (type)
	{
	case Attacker:
		return "Resource/3D/Enemy/broccoli.mqo";

	case Jumper:
		return "Resource/3D/Enemy/omelette.mqo";

	case Runner:
		return "Resource/3D/Enemy/tomato.mqo";

	default:
		return "Resource/3D/Enemy/Default.mqo";
	}
}



// デストラクタ
Enemy3D::~Enemy3D()
{
	if (mpModel)
	{
		delete mpModel;
		mpModel = nullptr;
	}
}

void Enemy3D::Jump()
{
	if (m_type != Jumper) return;

	// 地面にいて、かつタイマーが0以下ならジャンプ
	if (mbIsGround)
	{
		mfJumpTimer -= 1.0f; // 毎フレーム減らす
		if (mfJumpTimer <= 0.0f)
		{
			mfYVelocity = mfJumpPower; // ジャンプパワー（必要に応じて調整）
			mbIsGround = false;
			mfJumpTimer = 120.0f; // 次のジャンプまでの間隔（約2秒）
		}
	}
}

// 更新処理
void Enemy3D::Update()
{
	// フレーム開始時の位置を保存（ResolveCollision3D が参照する）
	mvOldPosition = mvPosition;

	Jump();

	UpdateRunner();

	// 重力 これで落下ありになる
	mfYVelocity += mfGravity;
	mvPosition.y += mfYVelocity;

	// 落下したときは削除
	if (mvPosition.y < -4000.0f) SetDeleteFlag(true);

	// シンプルな射撃タイマー（Attacker のみ）
	if (mfShotTimer > 0.0f)
	{
		mfShotTimer -= 1.0f;
	}
	else
	{
		Shot();
		mfShotTimer = SHOT_INTERVAL;
	}

	// 共通の衝突解決（床・壁・天井）
	ResolveCollision3D();

	// モデルの位置・回転同期（派生で実装）
	SyncModel();

	// 基底更新
	Object3D::Update();
}

// 描画処理
void Enemy3D::Draw()
{
	if (mpModel) mpModel->Draw();

	// 共通デバッグ（Character3D::DebugDraw を利用）
	//DebugDraw();

	//// 移動可能範囲の可視化（デバッグ用）
	//DrawSphere3D(
	//	mvBasePosition,   // 中心（元の位置）
	//	200.0f,            // 移動可能半径
	//	16,
	//	GetColor(0, 255, 0),
	//	GetColor(0, 255, 0),
	//	FALSE
	//);

	Object3D::Draw();
}

// デバッグ描画
void Enemy3D::DebugDraw()
{
	// 当たり判定の球体（可視化）
	DrawSphere3D(VAdd(mvPosition, VGet(0.0f, 10.0f, 0.0f)), m_radius, 8,
		GetColor(255, 255, 255), GetColor(0, 255, 0), false);

	// HP ラベル（スクリーン座標へ変換）
	VECTOR posForLabel = VAdd(mvPosition, VGet(0.0f, 80.0f, 0.0f));
	VECTOR screenPos = DxLib::ConvWorldPosToScreenPos(posForLabel);
	if (screenPos.z >= 0.0f)
	{
		DrawFormatString(
			static_cast<int>(screenPos.x),
			static_cast<int>(screenPos.y),
			GetColor(255, 200, 100),
			"ID:%d HP:%d",
			m_id,
			m_hp
		);
	}


	auto rawEnemyList =
		Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()->GetObject3DListByTag(Object3D::T_Enemy3D);

	int i = 0;

	for (auto* obj : rawEnemyList)
	{
		Enemy3D* e = dynamic_cast<Enemy3D*>(obj);
		if (!e) continue;

		VECTOR p = e->GetPosition();

		DrawFormatString(
			10, 100 + i * 20,
			GetColor(255, 255, 0),
			"ID:%d Pos:(%.1f,%.1f,%.1f)",
			e->GetID(),
			p.x, p.y, p.z
		);

		i++;
	}
}

// 弾発射
void Enemy3D::Shot()
{
	if (m_type != Attacker) return;

	// プレイヤー取得
	auto playerList =
		Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()
		->GetObject3DListByTag(Object3D::T_Player3D);

	// プレイヤーがいない場合は撃たない
	if (playerList.empty()) return;

	Object3D* player = playerList[0];
	VECTOR playerPos = player->GetPosition();

	// 距離チェック
	float distance = VSize(VSub(playerPos, mvPosition));
	if (distance > 1500.0f)
	{
		return;
	}

	// 弾のスポーン位置（少し上から発射）
	VECTOR spawnPos = VAdd(mvPosition, VGet(0.0f, 30.0f, 0.0f));

	// プレイヤーへの向き（水平射撃）
	VECTOR dir = VSub(playerPos, mvPosition);
	dir.y = 0.0f;
	dir = VNorm(dir);

	// 弾生成
	new Bullet3D(spawnPos, "Resource/3D/Bullet/EnemyBullet.mqo", dir, Team::Enemy);
}
// モデル同期（Character3D::SyncModel の実装）
void Enemy3D::SyncModel()
{
	if (mpModel)
	{
		mpModel->SetPosition(mvPosition);
		mpModel->SetRotation(mvRotation);
		mpModel->Update();
	}
}

// ヒット判定用中心
VECTOR Enemy3D::GetHitCenter() const
{
	return VAdd(mvPosition, VGet(0.0f, 10.0f, 0.0f));
}

float Enemy3D::GetRadius() const
{
	return Character3D::GetRadius();
}

void Enemy3D::UpdateRunner()
{
	if (m_type != Runner) return;

	mfMoveTimer -= 1.0f;

	// 一定時間ごとに新しい目標
	if (mfMoveTimer <= 0.0f)
	{
		// 180.0 を 180.0f に変更
		float angle = (float)(rand() % 360) * (float)DX_PI / 180.0f;

		float radius = (float)(rand() % 200);

		mvTargetPosition.x = (float)(mvBasePosition.x + cosf(angle) * radius);
		mvTargetPosition.z = (float)(mvBasePosition.z + sinf(angle) * radius);
		mvTargetPosition.y = mvBasePosition.y;

		mfMoveTimer = 120.0f;
	}

	// 目標までのベクトル
	VECTOR toTarget = VSub(mvTargetPosition, mvPosition);
	float dist = VSize(toTarget);

	// 近すぎるなら止める
	if (dist < 3.0f)
	{
		mvPosition = mvTargetPosition;
		return;
	}

	// 移動（常に動かす・条件で止めない）
	VECTOR dir = VNorm(toTarget);
	mvPosition = VAdd(mvPosition, VScale(dir, 1.5f));
}