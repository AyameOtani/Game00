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
	: Character3D(initPos, 2, Team::Enemy, 60.0f)
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

	SetFontSize(20);
}

std::string Enemy3D::GetModelPath(Enemy3D::EnemyType type)
{
	switch (type)
	{
	case Attacker:
		return "Resource/3D/Enemy/broccoli.mqo";

	case Jumper:
		return "Resource/3D/Enemy/tomato.mqo";

	case Runner:
		return "Resource/3D/Enemy/Tank.mqo";

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

// 更新処理
void Enemy3D::Update()
{
	// フレーム開始時の位置を保存（ResolveCollision3D が参照する）
	mvOldPosition = mvPosition;


	// 重力 これで落下ありになる
	mfYVelocity += mfGravity;
	mvPosition.y += mfYVelocity;

	// 落下したときは削除
	if (mvPosition.y < -4000.0f) SetDeleteFlag(true);


	// シンプルな射撃タイマー（Attacker のみ）
	if (m_type == Attacker)
	{
		if (mfShotTimer > 0.0f)
		{
			mfShotTimer -= 1.0f;
		}
		else
		{
			Shot();
			mfShotTimer = SHOT_INTERVAL;
		}
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
	DebugDraw();

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

	VECTOR spawnPos = VAdd(mvPosition, VGet(0.0f, 30.0f, 0.0f));
	VECTOR shotDir = VGet(sinf(mfAngle), 0.0f, cosf(mfAngle));

	new Bullet3D(spawnPos, "Resource/3D/Bullet/EnemyBullet.mqo", shotDir, Team::Enemy);
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