#include "Player3D.h"
#include "Model.h"
#include "InputManager.h"
#include "DxLib.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Stage.h"
#include "ObjectManager.h"
#include <algorithm>
#include "Enemy3D.h"
#include "Bullet3D.h"

// コンストラクタ：Character3D の判定パラメータはここで調整する
Player3D::Player3D(VECTOR initPos, std::string filename)
	: Character3D(initPos, 5, Team::Player, 30.0f) // maxHp, team, radius
	, mfAngle(0.0f)
	, mfTargetAngle(0.0f)
{
	mpModel = new Model(filename, initPos);
	SetTag(Tag3D::T_Player3D);

	// Player 固有の当たり判定パラメータ調整（Character3D の protected メンバへ代入）

	m_radius = 30.0f;             // プレイヤー本体の当たり判定半径
	m_ceilRadius = 45.0f;         // 天井判定用の半径

	m_floorCapsuleMinY = 3.0f;    // 床判定カプセルの下端Y座標
	m_floorCapsuleMaxY = 40.0f;   // 床判定カプセルの上端Y座標
	m_floorLinePos = 26.0f;       // 床判定ラインの基準位置
	m_floorLineMinY = 80.0f;      // 床判定ラインの開始位置
	m_floorLineMaxY = -100.0f;    // 床判定ラインの終了位置（下方向）

	m_wallCapsuleMinY = 40.0f;    // 壁判定カプセルの下端Y座標
	m_wallCapsuleMaxY = 50.0f;    // 壁判定カプセルの上端Y座標

	m_ceilCapsuleMinY = 50.0f;    // 天井判定カプセルの下端Y座標
	m_ceilCapsuleMaxY = 60.0f;    // 天井判定カプセルの上端Y座標
	m_ceilLinePos = 30.0f;        // 天井判定ラインの基準位置
	m_ceilLineMinY = 70.0f;       // 天井判定ラインの開始位置
	m_ceilLineMaxY = 100.0f;      // 天井判定ラインの終了位置（上方向）

	// 画像の読み込み
	mnHeartFullImg = LoadGraph("Resource/2D/Life.png"); if (mnHeartFullImg == -1) printfDx("画像ない");
	mnHeartEmptyImg = LoadGraph("Resource/2D/Damage.png"); if (mnHeartEmptyImg == -1) printfDx("画像ない");
	mnHpBox = LoadGraph("Resource/2D/HpBox.png"); if (mnHpBox == -1) printfDx("画像ない");

	SetFontSize(20);
}

Player3D::~Player3D()
{
	if (mpModel) { delete mpModel; mpModel = nullptr; }
}

void Player3D::Update()
{
	// 落下したときは削除
	if (mvPosition.y < -3000.0f) SetDeleteFlag(true);

	if (CheckHitKey(KEY_INPUT_1))
	{
		mvPosition = VGet(0,0,0);
		mfYVelocity = 0.0f;
		mbIsGround = false;
	}

	mvOldPosition = mvPosition;

	MoveEx();
	Jump();
	RotationByMove();

	if (mfShotTimer > 0.0f) mfShotTimer -= 1.0f;
	if (CheckHitKey(KEY_INPUT_B) || (GetJoypadInputState(DX_INPUT_PAD1) & PAD_INPUT_2)) Shot();

	// 共通化した Character3D::ResolveCollision3D を利用する
	ResolveCollision3D();

	// キャラ同士の押し戻し
	ResolveCharacterPush();

	// モデル同期（位置・回転をモデルに反映）
	SyncModel();

	Object3D::Update();
}

void Player3D::Draw()
{
	// ここで共通デバッグ描画を呼ぶ（Character3D::DebugDraw）
	DebugDraw();

	DrawHp(); // HPの描画呼び出し

	// HP 表示など Player 固有の情報
	DrawFormatString(0, 40, GetColor(255,255,0), "HP: %d", m_hp);
	DrawFormatString(0, 10, GetColor(255,255,0), "%.1f, %.1f, %.1f", mvPosition.x, mvPosition.y, mvPosition.z);


	if (mpModel) mpModel->Draw();
	Object3D::Draw();
}

void Player3D::DrawHp()
{
	int boxX = 250;
	int boxY = 900;
	// HPの背景ボックスの表示
	DrawRotaGraph(boxX, boxY, 0.4f, 0.0f, mnHpBox, TRUE);
	// ハートの表示
	for (int i = 0; i < m_maxHp; i++)
	{
		int x = 190 + i * 50;

		if (i < m_hp)
		{
			DrawRotaGraph(x, boxY, 0.25f, 0.0f, mnHeartFullImg, TRUE);
		}
		else
		{
			DrawRotaGraph(x, boxY, 0.25f, 0.0f, mnHeartEmptyImg, TRUE);
		}
	}

}

void Player3D::Shot()
{
	// クールタイムが終わっていなければ発射しない
	if (mfShotTimer > 0.0f) return;

	VECTOR spawnPos = VAdd(mvPosition, VGet(0.0f, 43.0f, 0.0f));
	VECTOR shotDir = VGet(sinf(mfAngle), 0.0f, cosf(mfAngle));

	// 弾を生成（位置、モデル名、方向、陣営を渡す）
	// ※Team::Playerを指定することで、味方撃ちを防ぐ
	new Bullet3D(spawnPos, "Resource/3D/Bullet/PlayerBullet.mqo", shotDir, Team::Player);

	// 発射後にクールタイムをセット
	mfShotTimer = SHOT_INTERVAL;
}

// 移動処理：キー入力とスティック入力からベクトルを作成
void Player3D::MoveEx()
{
	VECTOR moveVec = VGet(0.0f, 0.0f, 0.0f);
	VECTOR upMoveVector = VGet(0.0f, 0.0f, 0.0f);
	VECTOR leftMoveVector = VGet(0.0f, 0.0f, 0.0f);

	// カメラの視点から前後左右のベクトルを算出
	{
		upMoveVector = VSub(Master::mpCamera->GetLookAtPosition(), Master::mpCamera->GetPosition());
		upMoveVector.y = 0.0f;
		leftMoveVector = VCross(upMoveVector, VGet(0.0f, 1.0f, 0.0f));
		leftMoveVector.y = 0.0f;
		upMoveVector = VNorm(upMoveVector);
		leftMoveVector = VNorm(leftMoveVector);
	}

	// キーボード入力による移動加算
	if (CheckHitKey(KEY_INPUT_A)) moveVec = VAdd(moveVec, leftMoveVector);
	if (CheckHitKey(KEY_INPUT_D)) moveVec = VAdd(moveVec, VScale(leftMoveVector, -1.0f));
	if (CheckHitKey(KEY_INPUT_W)) moveVec = VAdd(moveVec, upMoveVector);
	if (CheckHitKey(KEY_INPUT_S)) moveVec = VAdd(moveVec, VScale(upMoveVector, -1.0f));

	// ゲームパッドのアナログスティック入力処理
	int StickX, StickY;
	VECTOR moveVecPad = VGet(0.0f, 0.0f, 0.0f);
	GetJoypadAnalogInput(&StickX, &StickY, DX_INPUT_PAD1);
	const int stickDeadZone = 50;

	if (StickX < -stickDeadZone) moveVecPad = VAdd(moveVecPad, leftMoveVector);
	if (StickX > stickDeadZone)  moveVecPad = VAdd(moveVecPad, VScale(leftMoveVector, -1.0f));
	if (StickY < -stickDeadZone) moveVecPad = VAdd(moveVecPad, upMoveVector);
	if (StickY > stickDeadZone)  moveVecPad = VAdd(moveVecPad, VScale(upMoveVector, -1.0f));

	moveVec = VAdd(moveVec, moveVecPad);

	// ターゲット角度（向き）の更新
	VECTOR targetDir = VGet(0, 0, 0);
	float inputMag = VSize(moveVec);
	if (inputMag > 0.0001f)
	{
		targetDir = VNorm(moveVec);
		mfTargetAngle = atan2f(targetDir.x, targetDir.z);
	}

	const float dt = 1.0f / 60.0f;
	VECTOR targetVel = VScale(targetDir, mfSpeed * (inputMag > 0.0001f ? 1.0f : 0.0f));

	// 加減速の計算
	float accel = mbIsGround ? mfAccel : mfAirAccel;
	if (VSize(targetVel) < VSize(mvVelocity)) accel = mbIsGround ? mfDecel : mfAirDecel;

	VECTOR deltaV = VSub(targetVel, mvVelocity);
	float deltaLen = VSize(deltaV);
	if (deltaLen > 0.0001f)
	{
		float maxStep = accel * dt;
		if (deltaLen <= maxStep) mvVelocity = targetVel;
		else mvVelocity = VAdd(mvVelocity, VScale(VNorm(deltaV), maxStep));
	}

	mvPosition = VAdd(mvPosition, VScale(mvVelocity, 1.0f));
}

void Player3D::RotationByMove()
{
	float subAngle = mfTargetAngle - mfAngle;
	// 角度を-PIからPIの範囲に正規化
	if (subAngle < -DX_PI_F)
	{
		subAngle += DX_TWO_PI_F;
	}
	else if (subAngle > DX_PI_F)
	{
		subAngle -= DX_TWO_PI_F;
	}

	// スムーズに回転させる処理
	if (subAngle > 0.0f)

	{	subAngle -= ROTATE_SPEED;

		if (subAngle < 0.0f)
		{
			subAngle = 0.0f;
		}
	}
	else if (subAngle < 0.0f)
	{
		subAngle += ROTATE_SPEED;
		if (subAngle > 0.0f)
		{
			subAngle = 0.0f;
		}
	}

	mfAngle = mfTargetAngle - subAngle;
	mvRotation.y = mfAngle + DX_PI_F;
	mpModel->SetRotation(mvRotation);
}

void Player3D::Jump()
{
	// スペースを押していたらジャンプ（地面にいるときのみ）
	if (mbIsGround && (InputManager::CheckDownKey(KEY_INPUT_SPACE) || (GetJoypadInputState(DX_INPUT_PAD1) & PAD_INPUT_1)))
	{
		mfYVelocity = mfJumpPower;
		mbIsGround = false;
	}

	mfYVelocity += mfGravity; // 重力加算
	if (mfYVelocity < -mfMaxFallSpeed) mfYVelocity = -mfMaxFallSpeed; // 落下速度制限
	mvPosition.y += mfYVelocity;

	// デバッグ用 飛べる
	if (CheckHitKey(KEY_INPUT_0))
	{
		mfYVelocity = 20.0f;
		mbIsGround = false;
	}
}

// SyncModel: モデルを持つ派生クラスはここで位置・回転を反映する
void Player3D::SyncModel()
{
	// 位置をセットする
	if (mpModel)
	{
		mpModel->SetPosition(mvPosition);
		mpModel->SetRotation(mvRotation);
		mpModel->Update();
	}
}


// キャラクター同士の押し戻し処理（Player vs Enemy）
void Player3D::ResolveCharacterPush()
{
	// Enemy一覧を取得
	auto enemyList =
		Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()
		->GetObject3DListByTag(Object3D::T_Enemy3D);

	// 衝突判定用の許容誤差
	constexpr float kEpsilon = 1.0f / 1000.0f;

	// 敵のリスト回す
	for (auto obj : enemyList)
	{
		Enemy3D* enemy = dynamic_cast<Enemy3D*>(obj);
		if (!enemy) continue;

		// Player から Enemy 方向ベクトル
		VECTOR diff = VSub(mvPosition, enemy->GetPosition());
		float dist = VSize(diff);

		float playerRadius = m_radius;
		float enemyRadius = enemy->GetRadius(); // 敵の当たり判定の半径
		float minDist = playerRadius + enemyRadius;

		// 距離が近すぎる そもそも当たっていない場合は処理しない
		if (dist < kEpsilon || dist >= minDist)
			continue;

		// 押し戻し方向  正規化ベクトル
		VECTOR dir = VScale(diff, 1.0f / dist);

		// めり込み量  重なりの深さ
		float penetration = minDist - dist;

		// Playerのみ押し戻す
		mvPosition = VAdd(
			mvPosition,
			VScale(dir, penetration)
		);

		 // Enemyも動かす場合の拡張例
		 enemy->SetPosition(
		     VSub(enemy->GetPosition(),
		         VScale(dir, penetration * 0.5f))
		 );
	}
}