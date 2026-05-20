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

// コンストラクタ：プレイヤーの初期化
Player3D::Player3D(VECTOR initPos, std::string filename)
	: Object3D(initPos)
	, mfAngle(0.0f)
	, mfTargetAngle(0.0f)
	, mvOldPosition(initPos)
{
	mpModel = new Model(filename, initPos);
	SetTag(Tag3D::T_Player3D);

	m_radius = 30.0f; // プレイヤーの当たり判定半径

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

	int color = GetColor(255, 255, 255);
	DrawFormatString(0, 20, color, "%.1f, %.1f, %.1f", mvPosition.x, mvPosition.y, mvPosition.z);
}

// 更新処理：毎フレーム呼ばれるメインロジック
void Player3D::Update()
{
	// リセット処理（デバッグ用：キー1で原点に戻る）
	if (CheckHitKey(KEY_INPUT_1))
	{
		mvPosition = VGet(0.0f, 0.0f, -5000.0f);
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

// ショットの発射処理
void Player3D::Shot()
{
	if (mfShotTimer > 0.0f) return;

	// プレイヤーの前方へ弾を生成
	VECTOR spawnPos = VAdd(mvPosition, VGet(0.0f, 53.0f, 0.0f));
	VECTOR shotDir = VGet(sinf(mfAngle), 0.0f, cosf(mfAngle));
	new Bullet3D(spawnPos, "Resource/3D/Bullet/PlayerBullet.mqo", shotDir);

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
}



// 3D環境との当たり判定を解決して、衝突応答を行う関数
void Player3D::ResolveCollision3D()
{
	// 1. 移動ベクトル（今フレームの移動量）を算出する
	VECTOR moveVec = VSub(mvPosition, mvOldPosition);
	// 2. 判定用に一旦プレイヤー位置を移動前に戻す
	mvPosition = mvOldPosition;

	bool isHitCeiling = false;
	float ceilMinY = FLT_MAX;
	float ceilMaxY = -FLT_MAX;
	float bestFloorY = -FLT_MAX;
	bool isHitFloor = false;

	// 3. シーン上の全ステージオブジェクトのリストを取得する
	auto stageList = Master::mpSceneManager->GetCurrentScene()->GetObjectManager()->GetObject3DListByTag(Object3D::T_Stage3D);

	// 4. 壁判定用のラムダ式（指定位置が壁に埋まっているか確認）
	auto CollidesAt = [&](VECTOR pos) -> bool
		{
			for (auto obj : stageList)
			{
				Stage* wall = dynamic_cast<Stage*>(obj);
				if (!wall) continue;

				VECTOR hp, hn;
				if (wall->CheckHit_Capsule_Wall(VAdd(pos, VGet(0, m_wallCapsuleMinY, 0)), VAdd(pos, VGet(0, m_wallCapsuleMaxY, 0)), m_radius, hp, hn))
					return true;
			}
			return false;
		};

	// 5. 壁との衝突を計算し、押し出し先の位置を算出する
	VECTOR targetPos = VAdd(mvOldPosition, moveVec);
	VECTOR avgNormal = VGet(0, 0, 0);
	int hitCount = 0;

	for (auto obj : stageList)
	{
		Stage* pStage = dynamic_cast<Stage*>(obj);
		if (!pStage) continue;

		VECTOR hitPosWall, hitNormal;
		if (pStage->CheckHit_Capsule_Wall(VAdd(targetPos, VGet(0, m_wallCapsuleMinY, 0)), VAdd(targetPos, VGet(0, m_wallCapsuleMaxY, 0)), m_radius, hitPosWall, hitNormal))
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

	// 6. 壁に当たった場合、壁に沿って滑る移動計算を行う
	if (hitCount > 0 && VSize(avgNormal) > 0.0001f)
	{
		avgNormal = VNorm(avgNormal);
		float dot = VDot(moveVec, avgNormal);
		VECTOR slide = VSub(moveVec, VScale(avgNormal, dot));
		VECTOR nextPos = VAdd(mvOldPosition, slide);

		if (!CollidesAt(nextPos)) { mvPosition.x = nextPos.x; mvPosition.z = nextPos.z; }
		else
		{
			VECTOR tryX = mvOldPosition; tryX.x += slide.x;
			if (!CollidesAt(tryX)) mvPosition.x = tryX.x;
			VECTOR tryZ = mvPosition; tryZ.z += slide.z;
			if (!CollidesAt(tryZ)) mvPosition.z = tryZ.z;
		}
	}
	else { mvPosition.x = targetPos.x; mvPosition.z = targetPos.z; }

	// 7. Y軸方向の判定処理（床・天井）
	mvPosition.y = targetPos.y;
	VECTOR floorNormal = VGet(0, 1, 0);

	for (auto obj : stageList)
	{
		Stage* pStage = dynamic_cast<Stage*>(obj);
		if (!pStage) continue;

		// 8. 足元の複数地点に判定線分を降ろす
		float step = m_floorLinePos;
		VECTOR offsets[5] = { VGet(0, 0, 0), VGet(step, 0, 0), VGet(-step, 0, 0), VGet(0, 0, step), VGet(0, 0, -step) };

		for (int i = 0; i < 5; ++i)
		{
			VECTOR start = VAdd(mvPosition, VAdd(offsets[i], VGet(0, m_floorLineMinY, 0)));
			VECTOR end = VAdd(mvPosition, VAdd(offsets[i], VGet(0, m_floorLineMaxY, 0)));
			VECTOR hitPos = pStage->CheckHit_Line(start, end);

			DrawLine3D(start, end, GetColor(255, 0, 0)); // 判定ライン描画

			if (VSize(hitPos) > 0.0001f)
			{
				isHitFloor = true;
				if (hitPos.y > bestFloorY)
				{
					bestFloorY = hitPos.y;
					VECTOR tempNormal;
					if (pStage->CheckHit_Line_Normal(start, end, tempNormal)) floorNormal = tempNormal;
				}
			}
		}

		// 9. 頭上の天井判定を行う
		if (pStage->CheckHit_Capsule(VAdd(mvPosition, VGet(0, m_ceilCapsuleMinY, 0)), VAdd(mvPosition, VGet(0, m_ceilCapsuleMaxY, 0)), m_radius))
		{
			float m_ceilLinePos_val = m_ceilLinePos;
			VECTOR LineSet[5] = { VGet(0,0,0), VGet(m_ceilLinePos_val,0,0), VGet(-m_ceilLinePos_val,0,0), VGet(0,0,m_ceilLinePos_val), VGet(0,0,-m_ceilLinePos_val) };
			for (int i = 0; i < 5; i++)
			{
				VECTOR start = VAdd(mvPosition, VAdd(LineSet[i], VGet(0, m_ceilLineMinY, 0)));
				VECTOR end = VAdd(mvPosition, VAdd(LineSet[i], VGet(0, m_ceilLineMaxY, 0)));
				DrawLine3D(start, end, GetColor(0, 0, 255));
				VECTOR hit = pStage->CheckHit_Line(start, end);
				if (VSize(hit) > 0.0001f) { isHitCeiling = true; if (hit.y < ceilMinY) ceilMinY = hit.y; if (hit.y > ceilMaxY) ceilMaxY = hit.y; }
			}
		}
	}

	// 10. 接地判定があればプレイヤー位置を床に合わせる
	if (isHitFloor && bestFloorY > -FLT_MAX)
	{
		float footY = mvPosition.y + m_floorCapsuleMinY - m_radius;
		if (mfYVelocity <= 0 && footY <= bestFloorY + 1.0f)
		{
			mvPosition.y = bestFloorY - m_floorCapsuleMinY + m_radius;
			mfYVelocity = 0;
			mbIsGround = true; mbJump = false; mbFall = false;

			// 11. モデルの傾きを床に合わせて補正する
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

			// 12. 急な坂道なら滑り落ちる処理を実行する
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
					if (horSpeed > 0.001f) { VECTOR inputDir = VNorm(VGet(moveVec.x, 0.0f, moveVec.z)); float align = VDot(inputDir, slideDir); if (align < 0.0f) slideStrength *= 0.5f; }
					mvPosition = VAdd(mvPosition, VScale(slideDir, slideStrength));
				}
			}
		}
	}
	// 13. 非接地時はフラグを解除し、回転を元に戻す
	else { mbIsGround = false; mvRotation.x *= 0.8f; mvRotation.z *= 0.8f; }

	// 14. 法線リセットと天井衝突時のめり込み防止処理
	if (!isHitFloor) floorNormal = VGet(0, 1, 0);
	if (isHitCeiling)
	{
		mvPosition.y = ceilMinY - m_ceilLineMaxY;
		if (mfYVelocity > 0) mfYVelocity = 0;
	}
}