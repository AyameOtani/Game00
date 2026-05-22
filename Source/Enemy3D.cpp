#include "Enemy3D.h"
#include "Model.h"
#include "Bullet3D.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "ObjectManager.h"
#include <cmath>


// =========================================================================
// コンストラクタ
// =========================================================================
Enemy3D::Enemy3D(VECTOR initPos, std::string filename, EnemyType type)
	: Character3D(initPos, 20, Team::Enemy, 30.0f) // HP=20, チーム=Player, 半径=30
	, m_type(type) // 敵タイプを初期化
	, mfAngle(0.0f)
	, mfTargetAngle(0.0f)
	, mvOldPosition(initPos)
	, mfShotTimer(0.0f)
{
	// 自身の mnTag に敵のタグ（T_Enemy3D）をセットして識別できるようにする
	SetTag(Object3D::T_Enemy3D);
	// 敵用の3Dモデルを生成して初期位置に配置
	mpModel = new Model(filename, initPos);



	m_radius = 30.0f; // 敵の当たり判定半径

	// 床判定用の各高さ設定
	m_floorCapsuleMinY = 3.0f;
	m_floorCapsuleMaxY = 40.0f;
	m_floorLinePos = 25.0f;
	m_floorLineMinY = 20.0f;
	m_floorLineMaxY = -300.0f;

	// 壁判定用の各高さ設定
	m_wallCapsuleMinY = 40.0f;
	m_wallCapsuleMaxY = 60.0f;

	// 天井判定用の各高さ設定
	m_ceilCapsuleMinY = 60.0f;
	m_ceilCapsuleMaxY = 80.0f;
	m_ceilLinePos = 15.0f;
	m_ceilLineMinY = 70.0f;
	m_ceilLineMaxY = 100.0f;

	SetFontSize(20);
}

// =========================================================================
// デストラクタ
// =========================================================================
Enemy3D::~Enemy3D()
{
	// 生成した3Dモデルを安全に削除
	if (mpModel != nullptr)
	{
		delete mpModel;
		mpModel = nullptr;
	}
}

// =========================================================================
// 更新処理（今は位置を同期させるだけ）
// =========================================================================
void Enemy3D::Update()
{

	// Attackerタイプなら攻撃タイマーを更新
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


	// モデルの位置を現在の座標（mvPosition）に同期
	if (mpModel != nullptr)
	{
		mpModel->SetPosition(mvPosition);
		mpModel->Update();
	}
	
	// 基底クラス（Object3D）のUpdateを呼び出す
	Object3D::Update();
}

// =========================================================================
// 描画処理
// =========================================================================
void Enemy3D::Draw()
{
	// 敵の3Dモデルを描画
	if (mpModel != nullptr)
	{
		mpModel->Draw();
	}

	DebugDraw(); // デバッグ用の描画

	// 基底クラス（Object3D）のDrawを呼び出す
	Object3D::Draw();
}

void Enemy3D::DebugDraw()
{
	// デバッグ用：敵のあたり判定の球体をワイヤーフレームで描画（緑色）
	float bulletRadius = 40.0f;
	DrawSphere3D(mvPosition, bulletRadius, 8, GetColor(0, 255, 0), GetColor(0, 255, 0), false);


	// この敵インスタンス自体のメモリアドレスを数値として扱う
	uintptr_t address = reinterpret_cast<uintptr_t>(this);

	// アドレスを使って、赤・緑・青の値をランダムっぽく作る
	int r = static_cast<int>((address % 3) * 127 + 128); // 128～255の間
	int g = static_cast<int>(((address / 100) % 3) * 127 + 128);
	int b = static_cast<int>(((address / 10000) % 3) * 127 + 128);

	unsigned int color = GetColor(r, g, b);

	// 敵の頭上の座標へ
	VECTOR posForLabel = VAdd(mvPosition, VGet(0.0f, 60.0f, 0.0f));
	VECTOR screenPos = DxLib::ConvWorldPosToScreenPos(posForLabel);

	if (screenPos.z >= 0.0f)
	{
		DrawFormatString(
			static_cast<int>(screenPos.x),
			static_cast<int>(screenPos.y),
			color, // ★ここで計算した色を使う
			"Enemy HP: %d",
			m_hp
		);
	}
}

// =========================================================================
// 弾の発射処理
// =========================================================================
void Enemy3D::Shot()
{
	// Attacker以外なら発射しない
	if (m_type != Attacker) return;

	// 弾の発射位置と方向の計算
	VECTOR spawnPos = VAdd(mvPosition, VGet(0.0f, 30.0f, 0.0f));
	VECTOR shotDir = VGet(sinf(mfAngle), 0.0f, cosf(mfAngle));

	// 弾の生成（敵陣営として）
	new Bullet3D(spawnPos, "Resource/3D/Bullet/EnemyBullet.mqo", shotDir, Team::Enemy);
}