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



// コンストラクタ：プレイヤーの初期化
Player3D::Player3D(VECTOR initPos, std::string filename)
	: Character3D(initPos, 50, Team::Player, 30.0f) // HP=100, チーム=Player, 半径=30
	, mfAngle(0.0f)
	, mfTargetAngle(0.0f)
	, mvOldPosition(initPos)
{
	mpModel = new Model(filename, initPos);
	SetTag(Tag3D::T_Player3D);

	m_radius = 30.0f; // プレイヤーの当たり判定半径


	// 床判定用：足元から少し上の範囲をカプセルでチェック
	m_floorCapsuleMinY = 3.0f;  // 床判定カプセルの下端
	m_floorCapsuleMaxY = 40.0f; // 床判定カプセルの上端
	m_floorLinePos = 30.0f;     // 床との接地を測るラインの横位置オフセット
	m_floorLineMinY = 20.0f;    // 判定ラインの開始高さ
	m_floorLineMaxY = -300.0f;  // 判定ラインの終了高さ（これだけ地面方向に長い）

	// 壁判定用：キャラの胴体部分で壁（障害物）をチェック
	m_wallCapsuleMinY = 40.0f;  // 壁判定カプセルの下端
	m_wallCapsuleMaxY = 50.0f;  // 壁判定カプセルの上端

	// 天井判定用：キャラの頭上付近で天井をチェック
	m_ceilCapsuleMinY = 50.0f;  // 天井判定カプセルの下端
	m_ceilCapsuleMaxY = 60.0f;  // 天井判定カプセルの上端
	m_ceilLinePos = 30.0f;      // 天井との衝突を測るラインの横位置オフセット
	m_ceilLineMinY = 70.0f;     // 判定ラインの開始高さ
	m_ceilLineMaxY = 100.0f;    // 判定ラインの終了高さ（キャラの頭上までチェック）

	SetFontSize(20);
}

// デストラクタ
Player3D::~Player3D()
{
	delete mpModel; // モデルメモリの解放
}

// デバッグ用の描画
void Player3D::DebugDraw()
{
	// 床、壁、天井の判定カプセルをそれぞれ色分けして描画
	DrawCapsule3D(
		VAdd(mvPosition, VGet(0.0f, m_floorCapsuleMinY, 0.0f)),
		VAdd(mvPosition, VGet(0.0f, m_floorCapsuleMaxY, 0.0f)),
		m_radius, 8,
		GetColor(0, 255, 0), GetColor(0, 255, 0), false
	);

	DrawCapsule3D(
		VAdd(mvPosition, VGet(0.0f, m_wallCapsuleMinY, 0.0f)),
		VAdd(mvPosition, VGet(0.0f, m_wallCapsuleMaxY, 0.0f)),
		m_radius, 8,
		GetColor(255, 0, 0), GetColor(255, 0, 0), false
	);

	DrawCapsule3D(
		VAdd(mvPosition, VGet(0.0f, m_ceilCapsuleMinY, 0.0f)),
		VAdd(mvPosition, VGet(0.0f, m_ceilCapsuleMaxY, 0.0f)),
		m_radius, 8,
		GetColor(0, 0, 255), GetColor(0, 0, 255), false
	);


	// 全部のあたり判定のやつ
	DrawCapsule3D(
		VAdd(mvPosition, VGet(0.0f, 0.0f, 0.0f)),
		VAdd(mvPosition, VGet(0.0f, m_ceilCapsuleMaxY, 0.0f)),
		m_radius, 8,
		GetColor(255, 255, 255), GetColor(255, 255, 255), false
	);



	int color = GetColor(255, 255, 255);
	DrawFormatString(0, 20, color, "%.1f, %.1f, %.1f", mvPosition.x, mvPosition.y, mvPosition.z);
	DrawFormatString(0, 40, GetColor(255, 255, 0), "HP: %d", m_hp);
}

// 更新処理：毎フレーム呼ばれるメインロジック
void Player3D::Update()
{
	// 落下したときは死亡判定にする
	if (mvPosition.y < -4000.0f)
	{
		SetDeleteFlag(true);
	}

	// リセット処理（デバッグ用：キー1で原点に戻る）
	if (CheckHitKey(KEY_INPUT_1))
	{
		mvPosition = VGet(0.0f, 0.0f, 0.0f);
		mfYVelocity = 0.0f;
		mbIsGround = false;
	}

	mvOldPosition = mvPosition; // 移動前の位置を保存

	MoveEx();        // 移動計算
	Jump();          // 重力・ジャンプ計算
	RotationByMove(); // 移動方向への回転処理

	// ショットのクールタイム更新
	if (mfShotTimer > 0.0f)
	{
		mfShotTimer -= 1.0f;
	}

	// ショット入力の判定
	if (CheckHitKey(KEY_INPUT_B) || (GetJoypadInputState(DX_INPUT_PAD1) & PAD_INPUT_2))
	{
		Shot();
	}

	ResolveCollision3D(); // ステージとの当たり判定解決

	// モデル位置の同期と更新
	mpModel->SetPosition(mvPosition);
	mpModel->Update();

	Object3D::Update();
}

// 描画処理：プレイヤーの状態を可視化
void Player3D::Draw()
{
	DebugDraw(); // デバッグ用の描画

	mpModel->Draw();
	Object3D::Draw();
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

// 向きの補完処理
void Player3D::RotationByMove()
{
	float subAngle = mfTargetAngle - mfAngle;
	// 角度を-PIからPIの範囲に正規化
	if (subAngle < -DX_PI_F) subAngle += DX_TWO_PI_F;
	else if (subAngle > DX_PI_F) subAngle -= DX_TWO_PI_F;

	// スムーズに回転させる処理
	if (subAngle > 0.0f) { subAngle -= ROTATE_SPEED; if (subAngle < 0.0f) subAngle = 0.0f; }
	else if (subAngle < 0.0f) { subAngle += ROTATE_SPEED; if (subAngle > 0.0f) subAngle = 0.0f; }

	mfAngle = mfTargetAngle - subAngle;
	mvRotation.y = mfAngle + DX_PI_F;
	mpModel->SetRotation(mvRotation);
}

// ジャンプと重力の計算
void Player3D::Jump()
{
	if (mbIsGround && (InputManager::CheckDownKey(KEY_INPUT_SPACE) || (GetJoypadInputState(DX_INPUT_PAD1) & PAD_INPUT_1)))
	{
		mfYVelocity = mfJumpPower;
		mbIsGround = false;
	}

	mfYVelocity += mfGravity; // 重力加算
	if (mfYVelocity < -mfMaxFallSpeed) mfYVelocity = -mfMaxFallSpeed; // 落下速度制限
	mvPosition.y += mfYVelocity;

	// デバッグ用
	if (CheckHitKey(KEY_INPUT_0))
	{
		mfYVelocity = 20.0f;
		mbIsGround = false;
	}
}



// 3D環境との当たり判定を解決して、衝突応答を行う関数
void Player3D::ResolveCollision3D()
{
	// ステージ上の全オブジェクトを取得
	// 処理負荷を抑えるため、const参照として保持する
	const auto& stageList = Master::mpSceneManager->GetCurrentScene()->GetObjectManager()->GetObject3DListByTag(Object3D::T_Stage3D);

	// 移動ベクトルを算出し、判定を開始するため一旦位置を直前のものへ巻き戻す
	VECTOR moveVec = VSub(mvPosition, mvOldPosition);
	mvPosition = mvOldPosition;

	// 衝突状態を管理するフラグと、上下判定用の変数
	bool isHitCeiling = false;
	float ceilMinY = FLT_MAX;
	float ceilMaxY = -FLT_MAX;
	float bestFloorY = -FLT_MAX; // 最も高い位置にある床（足場）を特定するため
	bool isHitFloor = false;

	// 壁との衝突判定を行うための中心および前後左右のオフセット設定
	const float step = m_floorLinePos;
	const VECTOR kOffsets[5] =
	{
		{0, 0, 0}, {step, 0, 0}, {-step, 0, 0}, {0, 0, step}, {0, 0, -step}
	};

	
	// 最初に移動目標地点までの壁との衝突を計算し、めり込みを防ぐ
	VECTOR targetPos = VAdd(mvOldPosition, moveVec);
	VECTOR avgNormal = VGet(0, 0, 0); // 複数の壁に接した際、法線を合成して滑らかな挙動を作るため
	int hitCount = 0;

	for (auto obj : stageList)
	{
		Stage* pStage = dynamic_cast<Stage*>(obj);
		if (!pStage) continue;

		VECTOR hitPosWall, hitNormal;
		// 目標地点におけるカプセルと壁の衝突チェック
		if (pStage->CheckHit_Capsule_Wall(VAdd(targetPos, VGet(0, m_wallCapsuleMinY, 0)), VAdd(targetPos, VGet(0, m_wallCapsuleMaxY, 0)), m_radius, hitPosWall, hitNormal))
		{
			// 壁の法線ベクトル（Y成分は壁の傾きを無視するため0にする）
			VECTOR n = VGet(hitNormal.x, 0, hitNormal.z);
			if (VSize(n) > 0.0001f)
			{
				avgNormal = VAdd(avgNormal, VNorm(n));
				hitCount++;

				// 壁からカプセルを押し出す距離を計算し、目標位置を補正
				VECTOR capsuleCenter = VGet(targetPos.x, 0, targetPos.z);
				VECTOR hitPointXZ = VGet(hitPosWall.x, 0, hitPosWall.z);
				VECTOR vToCenter = VSub(capsuleCenter, hitPointXZ);
				float dist = VSize(vToCenter);
				if (dist < m_radius)
				{
					float pushLen = m_radius - dist;
					// 押し出した後に壁との摩擦や微小なめり込みを考慮して余裕を持たせる
					targetPos = VAdd(targetPos, VScale(VNorm(n), pushLen + 0.05f));
				}
			}
		}
	}

	// 壁に沿った滑り移動の計算
	// 壁に接している場合は、移動ベクトルを壁に平行な成分のみに制限する
	if (hitCount > 0 && VSize(avgNormal) > 0.0001f)
	{
		avgNormal = VNorm(avgNormal);
		float dot = VDot(moveVec, avgNormal);
		VECTOR slide = VSub(moveVec, VScale(avgNormal, dot));

		mvPosition.x = mvOldPosition.x + slide.x;
		mvPosition.z = mvOldPosition.z + slide.z;
	}
	else
	{
		// 壁にぶつかっていない場合はそのまま目標地点へ
		mvPosition.x = targetPos.x;
		mvPosition.z = targetPos.z;
	}

	// Y軸方向の判定処理（床・天井の接地判定）
	mvPosition.y = targetPos.y;
	VECTOR floorNormal = VGet(0, 1, 0); // 基本的に床は水平とみなす

	for (auto obj : stageList)
	{
		Stage* pStage = dynamic_cast<Stage*>(obj);
		if (!pStage) continue;

		// 足元の5地点に線分を降ろして床の高さと法線を取得（精度を確保するため）
		for (const auto& offset : kOffsets)
		{
			VECTOR start = VAdd(mvPosition, VAdd(offset, VGet(0, m_floorLineMinY, 0)));
			VECTOR end = VAdd(mvPosition, VAdd(offset, VGet(0, m_floorLineMaxY, 0)));
			VECTOR hitPos = pStage->CheckHit_Line(start, end);

			// デバッグ用に判定ラインを描画
			DrawLine3D(start, end, GetColor(255, 0, 0));

			if (VSize(hitPos) > 0.0001f)
			{
				isHitFloor = true;
				// 最も高い位置を優先して、階段や段差の判定を行う
				if (hitPos.y > bestFloorY)
				{
					bestFloorY = hitPos.y;
					VECTOR tempNormal;
					if (pStage->CheckHit_Line_Normal(start, end, tempNormal)) floorNormal = tempNormal;
				}
			}
		}

		// 頭上の天井判定
		if (pStage->CheckHit_Capsule(VAdd(mvPosition, VGet(0, m_ceilCapsuleMinY, 0)), VAdd(mvPosition, VGet(0, m_ceilCapsuleMaxY, 0)), m_radius))
		{
			for (const auto& offset : kOffsets)
			{
				VECTOR start = VAdd(mvPosition, VAdd(offset, VGet(0, m_ceilLineMinY, 0)));
				VECTOR end = VAdd(mvPosition, VAdd(offset, VGet(0, m_ceilLineMaxY, 0)));

				// デバッグ用に判定ラインを描画
				DrawLine3D(start, end, GetColor(0, 0, 255));

				VECTOR hit = pStage->CheckHit_Line(start, end);
				if (VSize(hit) > 0.0001f) {
					isHitCeiling = true;
					if (hit.y < ceilMinY) ceilMinY = hit.y;
					if (hit.y > ceilMaxY) ceilMaxY = hit.y;
				}
			}
		}
	}


	// 接地している場合、プレイヤーのY座標を床の表面に合わせる
	if (isHitFloor && bestFloorY > -FLT_MAX)
	{
		float footY = mvPosition.y + m_floorCapsuleMinY - m_radius;
		if (mfYVelocity <= 0 && footY <= bestFloorY + 1.0f)
		{
			mvPosition.y = bestFloorY - m_floorCapsuleMinY + m_radius;
			mfYVelocity = 0;
			mbIsGround = true; mbJump = false; mbFall = false;

			// 床の傾きに合わせてモデルを回転させる
			{
				VECTOR fn = floorNormal;
				float worldTiltX = -asinf(fn.z);
				float worldTiltZ = asinf(fn.x);
				float sinY = sinf(mfAngle);
				float cosY = cosf(mfAngle);
				float localTiltX = worldTiltX * cosY - worldTiltZ * sinY;
				float localTiltZ = worldTiltX * sinY + worldTiltZ * cosY;

				// 回転を補間して自然な傾きにする
				mvRotation.x = mvRotation.x * 0.8f + localTiltX * 0.2f;
				mvRotation.z = mvRotation.z * 0.8f + localTiltZ * 0.2f;
				mpModel->SetRotation(mvRotation);
			}

			// 急な坂道であれば、自動的に滑り落ちる力を加える
			if (floorNormal.y < 0.95f)
			{
				VECTOR downDir = VGet(0.0f, -1.0f, 0.0f);
				float dot = VDot(downDir, floorNormal);
				VECTOR slideDir = VSub(downDir, VScale(floorNormal, dot));
				if (VSize(slideDir) > 0.0001f)
				{
					slideDir = VNorm(slideDir);
					float slideStrength = (1.0f - floorNormal.y) * 30.0f;
					if (slideStrength > 50.0f) slideStrength = 50.0f;
					mvPosition = VAdd(mvPosition, VScale(slideDir, slideStrength));
				}
			}
		}
	}
	else
	{
		// 空中にいる場合は接地フラグを解除し、回転を元に戻す
		mbIsGround = false;
		mvRotation.x *= 0.8f;
		mvRotation.z *= 0.8f;
	}

	// 最終調整：天井に頭をぶつけた際の位置補正
	if (!isHitFloor) floorNormal = VGet(0, 1, 0);
	if (isHitCeiling)
	{
		mvPosition.y = ceilMinY - m_ceilLineMaxY;
		if (mfYVelocity > 0) mfYVelocity = 0;
	}
}