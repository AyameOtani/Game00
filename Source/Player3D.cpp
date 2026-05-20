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

// コンストラクタ
Player3D::Player3D(VECTOR initPos, std::string filename)
	: Object3D(initPos)
	, mfAngle(0.0f)
	, mfTargetAngle(0.0f)
	, mvOldPosition(initPos)
{
	mpModel = new Model(filename, initPos);
	SetTag(Tag3D::T_Player3D);

	m_radius = 30.0f;

	m_floorCapsuleMinY = 3.0f;
	m_floorCapsuleMaxY = 40.0f;
	m_floorLinePos = 25.0f;
	m_floorLineMinY = 20.0f;
	m_floorLineMaxY = -300.0f;

	m_wallCapsuleMinY = 40.0f;
	m_wallCapsuleMaxY = 60.0f;

	m_ceilCapsuleMinY = 60.0f;
	m_ceilCapsuleMaxY = 80.0f;
	m_ceilLinePos = 15.0f;
	m_ceilLineMinY = 70.0f;
	m_ceilLineMaxY = 100.0f;
}

// デストラクタ
Player3D::~Player3D()
{
	delete mpModel;
}

// 更新処理
void Player3D::Update()
{
	if (CheckHitKey(KEY_INPUT_1))
	{
		mvPosition = VGet(0.0f, 0.0f, 0.0f);
		mfYVelocity = 0.0f;
		mbIsGround = false;
	}

	mvOldPosition = mvPosition;

	MoveEx();
	Jump();
	RotationByMove();

	// 1. ショットタイマーの更新
	if (mfShotTimer > 0.0f)
	{
		mfShotTimer -= 1.0f;
	}

	// 2. 入力検知で発射
	if (CheckHitKey(KEY_INPUT_B) || (GetJoypadInputState(DX_INPUT_PAD1) & PAD_INPUT_2))
	{
		Shot();
	}

	ResolveCollision3D();

	mpModel->SetPosition(mvPosition);
	mpModel->Update();

	Object3D::Update();
}

// 描画処理
void Player3D::Draw()
{
	// 各当たり判定カプセルの可視化（緑・赤・青）
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

	mpModel->Draw();
	Object3D::Draw();
}

// 弾の発射処理
void Player3D::Shot()
{
	if (mfShotTimer > 0.0f) return;

	// 発射位置の計算（プレイヤーの少し上）
	VECTOR spawnPos = VAdd(mvPosition, VGet(0.0f, 53.0f, 0.0f));

	// プレイヤーの向きから進む方向を計算
	VECTOR shotDir = VGet(sinf(mfAngle), 0.0f, cosf(mfAngle));
	// 弾を作成
	new Bullet3D(spawnPos, "Resource/3D/Bullet/PlayerBullet.mqo", shotDir);

	// 連射タイマーのリセット
	mfShotTimer = SHOT_INTERVAL;
}


// 移動処理
void Player3D::MoveEx()
{
	// 移動方向をきめて速度をかける
	VECTOR moveVec = VGet(0.0f, 0.0f, 0.0f); // 移動方向
	VECTOR upMoveVector = VGet(0.0f, 0.0f, 0.0f); // カメラの上方向（奥方向）ベクトル
	VECTOR leftMoveVector = VGet(0.0f, 0.0f, 0.0f); // カメラの左方向ベクトル

	// カメラの向きから移動ベクトルを求める
	{
		upMoveVector = VSub(Master::mpCamera->GetLookAtPosition(), Master::mpCamera->GetPosition());
		upMoveVector.y = 0.0f;

		leftMoveVector = VCross(upMoveVector, VGet(0.0f, 1.0f, 0.0f));
		leftMoveVector.y = 0.0f;

		upMoveVector = VNorm(upMoveVector);
		leftMoveVector = VNorm(leftMoveVector);
	}

	if (CheckHitKey(KEY_INPUT_A)) // 左方向
	{
		moveVec = VAdd(moveVec, leftMoveVector);
	}
	if (CheckHitKey(KEY_INPUT_D)) // 右方向
	{
		moveVec = VAdd(moveVec, VScale(leftMoveVector, -1.0f));
	}
	if (CheckHitKey(KEY_INPUT_W)) // 奥方向
	{
		moveVec = VAdd(moveVec, upMoveVector);
	}
	if (CheckHitKey(KEY_INPUT_S)) // 手前方向
	{
		moveVec = VAdd(moveVec, VScale(upMoveVector, -1.0f));
	}

	// -------------------- ゲームパッドのスティック入力 --------------------
	int StickX, StickY; // XとYを入れる変数
	VECTOR moveVecPad = VGet(0.0f, 0.0f, 0.0f);  // スティック用

	GetJoypadAnalogInput(&StickX, &StickY, DX_INPUT_PAD1); // 左スティック
	const int stickDeadZone = 50; // ここで少し触れただけで動くのを制御する

	if (StickX < -stickDeadZone) // 左
	{
		moveVecPad = VAdd(moveVecPad, leftMoveVector);
	}
	if (StickX > stickDeadZone)  // 右
	{
		moveVecPad = VAdd(moveVecPad, VScale(leftMoveVector, -1.0f));
	}
	if (StickY < -stickDeadZone) // 奥（上方向）
	{
		moveVecPad = VAdd(moveVecPad, upMoveVector);
	}
	if (StickY > stickDeadZone)  // 手前（下方向）
	{
		moveVecPad = VAdd(moveVecPad, VScale(upMoveVector, -1.0f));
	}

	// -------------------- moveVec と moveVec2 の統合 --------------------
	moveVec = VAdd(moveVec, moveVecPad);

	VECTOR targetDir = VGet(0, 0, 0);
	float inputMag = VSize(moveVec);
	if (inputMag > 0.0001f)
	{
		targetDir = VNorm(moveVec);
		mfTargetAngle = atan2f(targetDir.x, targetDir.z);
	}

	// デルタタイム
	const float dt = 1.0f / 60.0f;

	VECTOR targetVel = VScale(targetDir, mfSpeed * (inputMag > 0.0001f ? 1.0f : 0.0f));

	float accel = mbIsGround ? mfAccel : mfAirAccel;
	if (VSize(targetVel) < VSize(mvVelocity))
	{
		accel = mbIsGround ? mfDecel : mfAirDecel;
	}

	VECTOR deltaV = VSub(targetVel, mvVelocity);
	float deltaLen = VSize(deltaV);
	if (deltaLen > 0.0001f)
	{
		float maxStep = accel * dt;
		if (deltaLen <= maxStep)
			mvVelocity = targetVel;
		else
			mvVelocity = VAdd(mvVelocity, VScale(VNorm(deltaV), maxStep));
	}

	mvPosition = VAdd(mvPosition, VScale(mvVelocity, 1.0f));
}

// 移動による回転処理（既存のまま）
void Player3D::RotationByMove()
{
	float subAngle = mfTargetAngle - mfAngle;

	if (subAngle < -DX_PI_F)
	{
		subAngle += DX_TWO_PI_F;
	}
	else if (subAngle > DX_PI_F)
	{
		subAngle -= DX_TWO_PI_F;
	}

	if (subAngle > 0.0f)
	{
		subAngle -= ROTATE_SPEED;
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

// ジャンプ（既存のまま）
void Player3D::Jump()
{
	if (mbIsGround && InputManager::CheckDownKey(KEY_INPUT_SPACE))
	{
		mfYVelocity = mfJumpPower;
		mbIsGround = false;
	}

	if (mbIsGround && (GetJoypadInputState(DX_INPUT_PAD1) & PAD_INPUT_1))
	{
		mfYVelocity = mfJumpPower;
		mbIsGround = false;
	}

	mfYVelocity += mfGravity;

	if (mfYVelocity < -mfMaxFallSpeed) mfYVelocity = -mfMaxFallSpeed;

	mvPosition.y += mfYVelocity;
}



// 当たり判定（既存のまま）
void Player3D::ResolveCollision3D()
{
	VECTOR moveVec = VSub(mvPosition, mvOldPosition);
	mvPosition = mvOldPosition;

	bool isHitCeiling = false;
	float ceilMinY = FLT_MAX;
	float ceilMaxY = -FLT_MAX;
	float bestFloorY = -FLT_MAX;
	bool isHitFloor = false;

	auto stageList = Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()->GetObject3DListByTag(Object3D::T_Stage3D);

	auto CollidesAt = [&](VECTOR pos) -> bool
		{
			for (auto obj : stageList)
			{
				Stage* wall = dynamic_cast<Stage*>(obj);
				if (!wall) continue;

				VECTOR hp, hn;
				if (wall->CheckHit_Capsule_Wall(
					VAdd(pos, VGet(0, m_wallCapsuleMinY, 0)),
					VAdd(pos, VGet(0, m_wallCapsuleMaxY, 0)),
					m_radius, hp, hn))
				{
					return true;
				}
			}
			return false;
		};

	VECTOR targetPos = VAdd(mvOldPosition, moveVec);
	VECTOR avgNormal = VGet(0, 0, 0);
	int hitCount = 0;

	for (auto obj : stageList)
	{
		Stage* pStage = dynamic_cast<Stage*>(obj);
		if (!pStage) continue;

		VECTOR hitPosWall, hitNormal;
		if (pStage->CheckHit_Capsule_Wall(
			VAdd(targetPos, VGet(0, m_wallCapsuleMinY, 0)),
			VAdd(targetPos, VGet(0, m_wallCapsuleMaxY, 0)),
			m_radius, hitPosWall, hitNormal))
		{
			VECTOR n = VGet(hitNormal.x, 0, hitNormal.z);
			if (VSize(n) > 0.0001f)
			{
				avgNormal = VAdd(avgNormal, VNorm(n));
				hitCount++;

				VECTOR capsuleCenter = VGet(targetPos.x, 0, targetPos.z);
				VECTOR hitPointXZ = VGet(hitPosWall.x, 0, hitPosWall.z);
				VECTOR vToCenter = VSub(capsuleCenter, hitPointXZ);
				float dist = VSize(vToCenter);

				if (dist < m_radius)
				{
					float pushLen = m_radius - dist;
					targetPos = VAdd(targetPos, VScale(VNorm(n), pushLen + 0.05f));
				}
			}
		}
	}

	if (hitCount > 0 && VSize(avgNormal) > 0.0001f)
	{
		avgNormal = VNorm(avgNormal);
		float dot = VDot(moveVec, avgNormal);
		VECTOR slide = VSub(moveVec, VScale(avgNormal, dot));
		VECTOR nextPos = VAdd(mvOldPosition, slide);

		if (!CollidesAt(nextPos))
		{
			mvPosition.x = nextPos.x;
			mvPosition.z = nextPos.z;
		}
		else
		{
			VECTOR tryX = mvOldPosition;
			tryX.x += slide.x;
			if (!CollidesAt(tryX)) mvPosition.x = tryX.x;

			VECTOR tryZ = mvPosition;
			tryZ.z += slide.z;
			if (!CollidesAt(tryZ)) mvPosition.z = tryZ.z;
		}
	}
	else
	{
		mvPosition.x = targetPos.x;
		mvPosition.z = targetPos.z;
	}

	mvPosition.y = targetPos.y;
	VECTOR floorNormal = VGet(0, 1, 0);

	for (auto obj : stageList)
	{
		Stage* pStage = dynamic_cast<Stage*>(obj);
		if (!pStage) continue;

		float step = m_floorLinePos;
		VECTOR offsets[5] =
		{
			VGet(0, 0, 0),
			VGet(step, 0, 0),
			VGet(-step, 0, 0),
			VGet(0, 0, step),
			VGet(0, 0, -step)
		};

		for (int i = 0; i < 5; ++i)
		{
			VECTOR start = VAdd(mvPosition, VAdd(offsets[i], VGet(0, m_floorLineMinY, 0)));
			VECTOR end = VAdd(mvPosition, VAdd(offsets[i], VGet(0, m_floorLineMaxY, 0)));
			VECTOR hitPos = pStage->CheckHit_Line(start, end);

			DrawLine3D(start, end, GetColor(255, 0, 0));

			if (VSize(hitPos) > 0.0001f)
			{
				isHitFloor = true;
				if (hitPos.y > bestFloorY)
				{
					bestFloorY = hitPos.y;
					VECTOR tempNormal;
					if (pStage->CheckHit_Line_Normal(start, end, tempNormal))
					{
						floorNormal = tempNormal;
					}
				}
			}
		}

		if (pStage->CheckHit_Capsule(
			VAdd(mvPosition, VGet(0, m_ceilCapsuleMinY, 0)),
			VAdd(mvPosition, VGet(0, m_ceilCapsuleMaxY, 0)),
			m_radius))
		{
			float m_ceilLinePos_val = m_ceilLinePos;
			VECTOR LineSet[5] =
			{
				VGet(0,0,0),
				VGet(m_ceilLinePos_val,0,0),
				VGet(-m_ceilLinePos_val,0,0),
				VGet(0,0,m_ceilLinePos_val),
				VGet(0,0,-m_ceilLinePos_val)
			};

			for (int i = 0; i < 5; i++)
			{
				VECTOR start = VAdd(mvPosition, VAdd(LineSet[i], VGet(0, m_ceilLineMinY, 0)));
				VECTOR end = VAdd(mvPosition, VAdd(LineSet[i], VGet(0, m_ceilLineMaxY, 0)));

				DrawLine3D(start, end, GetColor(0, 0, 255));
				VECTOR hit = pStage->CheckHit_Line(start, end);

				if (VSize(hit) > 0.0001f)
				{
					isHitCeiling = true;
					if (hit.y < ceilMinY) ceilMinY = hit.y;
					if (hit.y > ceilMaxY) ceilMaxY = hit.y;
				}
			}
		}
	}

	if (isHitFloor && bestFloorY > -FLT_MAX)
	{
		float footY = mvPosition.y + m_floorCapsuleMinY - m_radius;
		if (mfYVelocity <= 0 && footY <= bestFloorY + 1.0f)
		{
			mvPosition.y = bestFloorY - m_floorCapsuleMinY + m_radius;
			mfYVelocity = 0;
			mbIsGround = true;
			mbJump = false;
			mbFall = false;

			{
				VECTOR fn = floorNormal;
				float worldTiltX = -asinf(fn.z);
				float worldTiltZ = asinf(fn.x);
				float sinY = sinf(mfAngle);
				float cosY = cosf(mfAngle);
				float localTiltX = worldTiltX * cosY - worldTiltZ * sinY;
				float localTiltZ = worldTiltX * sinY + worldTiltZ * cosY;

				mvRotation.x = mvRotation.x * 0.8f + localTiltX * 0.2f;
				mvRotation.z = mvRotation.z * 0.8f + localTiltZ * 0.2f;
				mpModel->SetRotation(mvRotation);
			}

			const float slideThreshold = 0.95f;
			if (floorNormal.y < slideThreshold)
			{
				VECTOR downDir = VGet(0.0f, -1.0f, 0.0f);
				float dot = VDot(downDir, floorNormal);
				VECTOR slideDir = VSub(downDir, VScale(floorNormal, dot));

				if (VSize(slideDir) > 0.0001f)
				{
					slideDir = VNorm(slideDir);
					float slideStrength = (1.0f - floorNormal.y) * 30.0f;

					if (slideStrength > 50.0f) slideStrength = 50.0f;

					float horSpeed = VSize(VGet(moveVec.x, 0.0f, moveVec.z));
					if (horSpeed > 0.001f)
					{
						VECTOR inputDir = VNorm(VGet(moveVec.x, 0.0f, moveVec.z));
						float align = VDot(inputDir, slideDir);
						if (align < 0.0f) slideStrength *= 0.5f;
					}
					mvPosition = VAdd(mvPosition, VScale(slideDir, slideStrength));
				}
			}
		}
	}
	else
	{
		mbIsGround = false;
		mvRotation.x *= 0.8f;
		mvRotation.z *= 0.8f;
	}

	if (!isHitFloor)
	{
		floorNormal = VGet(0, 1, 0);
	}

	if (isHitCeiling)
	{
		mvPosition.y = ceilMinY - m_ceilLineMaxY;
		if (mfYVelocity > 0) mfYVelocity = 0;
	}
}