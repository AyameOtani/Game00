#include "Character3D.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "ObjectManager.h"
#include "Stage.h"
#include <cfloat>
#include <algorithm>

// コンストラクタ：当たり判定パラメータはデフォルト値を入れておく。
// 派生クラス（Player3D / Enemy3D）はここを上書きして細かく調整する。
Character3D::Character3D(VECTOR initPos, int maxHp, Team team, float radius)
	: Object3D(initPos)
	, m_maxHp(maxHp)
	, m_hp(maxHp)
	, m_team(team)
	, m_radius(radius)
	, m_ceilRadius(radius) // 天井判定用半径はデフォルトで床と同じにしておく
	, m_floorCapsuleMinY(3.0f)
	, m_floorCapsuleMaxY(40.0f)
	, m_floorLinePos(25.0f)
	, m_floorLineMinY(20.0f)
	, m_floorLineMaxY(-300.0f)
	, m_wallCapsuleMinY(10.0f)
	, m_wallCapsuleMaxY(70.0f)
	, m_ceilCapsuleMinY(60.0f)
	, m_ceilCapsuleMaxY(80.0f)
	, m_ceilLinePos(15.0f)
	, m_ceilLineMinY(70.0f)
	, m_ceilLineMaxY(100.0f)
	, mvOldPosition(initPos)
{
	// 初期化は最低限に留める（派生側でモデル等を初期化する）
}

Character3D::~Character3D()
{
}

void Character3D::TakeDamage(int damage)
{
	m_hp -= damage;
	if (m_hp <= 0)
	{
		m_hp = 0;
		SetDeleteFlag(true);
	}
}


// デバッグ表示：床・壁・天井用カプセルと全体カプセルを描画する
void Character3D::DebugDraw()
{
	// 床用（緑）
	DrawCapsule3D(
		VAdd(mvPosition, VGet(0.0f, m_floorCapsuleMinY, 0.0f)),
		VAdd(mvPosition, VGet(0.0f, m_floorCapsuleMaxY, 0.0f)),
		m_radius, 8,
		GetColor(0, 255, 0), GetColor(0, 255, 0), false
	);

	// 壁用（赤）
	DrawCapsule3D(
		VAdd(mvPosition, VGet(0.0f, m_wallCapsuleMinY, 0.0f)),
		VAdd(mvPosition, VGet(0.0f, m_wallCapsuleMaxY, 0.0f)),
		m_radius, 8,
		GetColor(255, 0, 0), GetColor(255, 0, 0), false
	);

	// 天井用（青）
	DrawCapsule3D(
		VAdd(mvPosition, VGet(0.0f, m_ceilCapsuleMinY, 0.0f)),
		VAdd(mvPosition, VGet(0.0f, m_ceilCapsuleMaxY, 0.0f)),
		m_ceilRadius, 8,
		GetColor(0, 0, 255), GetColor(0, 0, 255), false
	);

	// 全体（白）
	DrawCapsule3D(
		VAdd(mvPosition, VGet(0.0f, 0.0f, 0.0f)),
		VAdd(mvPosition, VGet(0.0f, m_ceilCapsuleMaxY, 0.0f)),
		m_radius, 8,
		GetColor(255, 255, 255), GetColor(255, 255, 255), false
	);
}



void Character3D::ResolveCollision3D()
{
	// ステージオブジェクトの取得
	auto stageList = Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()->GetObject3DListByTag(Object3D::T_Stage3D);

	// 前フレーム位置との差分（移動量）を計算して、一旦巻き戻す
	VECTOR moveVec = VSub(mvPosition, mvOldPosition);
	mvPosition = mvOldPosition;

	// 判定用フラグ・補助変数の初期化
	bool isHitCeiling = false;
	bool isHitFloor = false;

	float ceilMinY = FLT_MAX;
	float ceilMaxY = -FLT_MAX;
	float bestFloorY = -FLT_MAX;

	// 接地したステージの移動量を保持する変数
	VECTOR totalMoveDelta = VGet(0, 0, 0);
	bool isGroundedStageFound = false;

	// 足元・床検出用のサンプルオフセット（5点チェック）
	const float step = m_floorLinePos;
	const VECTOR kOffsets[5] =
	{
		VGet(0,0,0),
		VGet(step,0,0),
		VGet(-step,0,0),
		VGet(0,0,step),
		VGet(0,0,-step)
	};

	// 未来位置（移動後想定位置）
	VECTOR targetPos = VAdd(mvOldPosition, moveVec);



	// ==============================
	// 壁との衝突判定（XZ方向補正）
	// ==============================
	VECTOR wallPos = targetPos; // 判定の開始位置を未来位置（tarrrgetPos）に設定

	// 角に挟まった場合などを考慮し、複数回繰り返してめり込みを解決する（イテレーション）
	for (int iter = 0; iter < 3; ++iter)
	{
		bool hitWall = false; // 今回のループで壁に当たったかどうかのフラグ

		for (auto obj : stageList)
		{
			Stage* pStage = dynamic_cast<Stage*>(obj);
			if (!pStage) continue;

			VECTOR hitPosWall, hitNormal;

			// 現在のwallPosに基づいたカプセルの位置を計算
			VECTOR wallCapMin = VAdd(wallPos, VGet(0, m_wallCapsuleMinY, 0));
			VECTOR wallCapMax = VAdd(wallPos, VGet(0, m_wallCapsuleMaxY, 0));

			// 壁との当たり判定を実行
			if (pStage->CheckHit_Capsule_Wall(
				wallCapMin,
				wallCapMax,
				m_radius,
				hitPosWall,
				hitNormal))
			{
				// 水平方向（XZ平面）の法線のみを抽出して正規化
				VECTOR n = VGet(hitNormal.x, 0, hitNormal.z);
				if (VSize(n) < 0.0001f)
					continue;

				n = VNorm(n);

				// 【滑り処理】壁方向への移動成分を打ち消す（壁に沿ってスライドさせる）
				VECTOR moveXZ = VGet(moveVec.x, 0, moveVec.z);
				float dot = VDot(moveXZ, n);
				if (dot < 0.0f) // 壁に向かっている場合のみ補正
				{
					moveXZ = VSub(moveXZ, VScale(n, dot)); // 壁に垂直な成分を削除
					moveVec.x = moveXZ.x;
					moveVec.z = moveXZ.z;
				}

				// 【追従処理】補正した移動量を反映した位置を一時的に計算
				wallPos.x = mvOldPosition.x + moveVec.x;
				wallPos.z = mvOldPosition.z + moveVec.z;

				// 【めり込み解消】カプセル中心と衝突地点の差分から距離を計算
				VECTOR centerXZ = VGet(wallPos.x, 0, wallPos.z);
				VECTOR hitXZ = VGet(hitPosWall.x, 0, hitPosWall.z);
				VECTOR diff = VSub(centerXZ, hitXZ);

				float dist = VSize(diff);

				// 距離が近すぎる場合は強引に法線方向に押し出す
				if (dist < 0.0001f)
				{
					diff = n;
					dist = 1.0f;
				}

				// 半径よりもめり込んでいる分だけ、法線方向に押し出す
				if (dist < m_radius)
				{
					float push = (m_radius - dist) + 0.01f; // わずかな余裕(0.1f)を持たせて押し出し
					wallPos = VAdd(wallPos, VScale(VNorm(diff), push));
				}

				hitWall = true; // 壁に衝突したのでフラグを立てる
			}
		}

		// これ以上壁に衝突しなければループを抜ける（効率化）
		if (!hitWall)
			break;
	}

	// まず壁処理後の位置を反映
	mvPosition = wallPos;
	mvPosition.y = targetPos.y;

	VECTOR floorNormal = VGet(0, 1, 0);




	// ==============================
	// 床・天井判定（縦方向処理）
	// ==============================
	for (auto obj : stageList)
	{
		Stage* pStage = dynamic_cast<Stage*>(obj);
		if (!pStage) continue;

		// ------------------------------
		// 床判定
		// ------------------------------
		for (const auto& offset : kOffsets)
		{
			VECTOR start = VAdd(mvPosition, VAdd(offset, VGet(0, m_floorLineMinY, 0)));
			VECTOR end = VAdd(mvPosition, VAdd(offset, VGet(0, m_floorLineMaxY, 0)));

			VECTOR hitPos = pStage->CheckHit_Line(start, end);

			DrawLine3D(start, end, GetColor(255, 0, 0));

			if (VSize(hitPos) > 0.0001f)
			{
				isHitFloor = true;

				if (hitPos.y > bestFloorY)
				{
					bestFloorY = hitPos.y;
					totalMoveDelta = pStage->GetPositionDelta();
					isGroundedStageFound = true;

					VECTOR tmpNormal;
					if (pStage->CheckHit_Line_Normal(start, end, tmpNormal))
					{
						floorNormal = tmpNormal;
					}
				}
			}
		}

		// ------------------------------
		// 天井判定
		// ------------------------------
		if (pStage->CheckHit_Capsule(
			VAdd(mvPosition, VGet(0, m_ceilCapsuleMinY, 0)),
			VAdd(mvPosition, VGet(0, m_ceilCapsuleMaxY, 0)),
			m_ceilRadius))
		{
			for (const auto& offset : kOffsets)
			{
				VECTOR start = VAdd(mvPosition, VAdd(offset, VGet(0, m_ceilLineMinY, 0)));
				VECTOR end = VAdd(mvPosition, VAdd(offset, VGet(0, m_ceilLineMaxY, 0)));

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

	// ==============================
	// 床確定処理（接地・滑り・姿勢）
	// ==============================
	if (isHitFloor && bestFloorY > -FLT_MAX)
	{
		float footY = mvPosition.y + m_floorCapsuleMinY - m_radius;

		bool isGrounded = false;
		if (m_team == Team::Player)
		{
			if (mfYVelocity <= 0 && footY <= bestFloorY + 10.0f) isGrounded = true;
		}
		else
		{
			if (footY <= bestFloorY + 5.0f) isGrounded = true;
		}

		if (isGrounded)
		{
			if (isGroundedStageFound)
			{
				mvPosition = VAdd(mvPosition, totalMoveDelta);
			}

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

				SyncModel();
			}

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
		else
		{
			mbIsGround = false;
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

	// ==============================
	// 天井補正（頭上押し戻し）
	// ==============================
	if (isHitCeiling)
	{
		mvPosition.y = ceilMinY - m_ceilLineMaxY;

		if (mfYVelocity > 0)
		{
			mfYVelocity = 0;
		}
	}

	SyncModel();
}