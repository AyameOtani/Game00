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
	, m_wallCapsuleMinY(40.0f)
	, m_wallCapsuleMaxY(60.0f)
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

// あたり判定の実装（ステージとの衝突解決）
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

	// 壁衝突用の平均法線（スライド計算用）
	VECTOR avgNormal = VGet(0, 0, 0);
	int hitCount = 0;

	// ==============================
	// 壁との衝突判定（XZ方向補正）
	// ==============================
	for (auto obj : stageList)
	{
		Stage* pStage = dynamic_cast<Stage*>(obj);
		if (!pStage) continue;

		VECTOR hitPosWall, hitNormal;

		// カプセル（縦方向）で壁判定
		if (pStage->CheckHit_Capsule_Wall(
			VAdd(targetPos, VGet(0, m_wallCapsuleMinY, 0)),
			VAdd(targetPos, VGet(0, m_wallCapsuleMaxY, 0)),
			m_radius,
			hitPosWall,
			hitNormal))
		{
			// 水平方向の法線だけ抽出（XZ平面）
			VECTOR n = VGet(hitNormal.x, 0, hitNormal.z);

			if (VSize(n) > 0.0001f)
			{
				n = VNorm(n);
				avgNormal = VAdd(avgNormal, n);
				hitCount++;

				// カプセル中心と壁とのめり込み補正
				VECTOR capsuleCenter = VGet(targetPos.x, 0, targetPos.z);
				VECTOR hitPointXZ = VGet(hitPosWall.x, 0, hitPosWall.z);

				VECTOR toCenter = VSub(capsuleCenter, hitPointXZ);
				float dist = VSize(toCenter);

				if (dist < m_radius)
				{
					float push = m_radius - dist;
					targetPos = VAdd(targetPos, VScale(n, push + 0.05f));
				}
			}
		}
	}

	// 壁法線を使って移動ベクトルをスライドさせる
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
		mvPosition.x = targetPos.x;
		mvPosition.z = targetPos.z;
	}

	// Y方向は一旦そのまま適用
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
		// 足元の複数点（kOffsets）に対して、床判定用の垂直レイ（線分）を順次チェック
		for (const auto& offset : kOffsets)
		{
			// レイの始点と終点を算出（キャラ位置 + オフセット + 上下オフセット）
			VECTOR start = VAdd(mvPosition, VAdd(offset, VGet(0, m_floorLineMinY, 0)));
			VECTOR end = VAdd(mvPosition, VAdd(offset, VGet(0, m_floorLineMaxY, 0)));

			// ステージの当たり判定を実行し、ヒット位置（地面のY座標など）を取得
			VECTOR hitPos = pStage->CheckHit_Line(start, end);

			// デバッグ用に、現在判定を行っている線分を画面に描画
			DrawLine3D(start, end, GetColor(255, 0, 0));

			// ヒット位置が(0,0,0)以外（つまり床に衝突した）か確認
			if (VSize(hitPos) > 0.0001f)
			{
				// 少なくとも一つ床に当たったのでフラグを立てる
				isHitFloor = true;

				// 複数のチェックポイントの中で、一番高いY座標（一番高い床）を採用
				// （段差や坂道で誤った低い位置にめり込むのを防ぐため）
				if (hitPos.y > bestFloorY)
				{
					bestFloorY = hitPos.y;

					// どのステージに接地しているかを特定し、そのステージの移動量（delta）を保存
					totalMoveDelta = pStage->GetPositionDelta();
					isGroundedStageFound = true;

					// 採用した床地点の法線を取得（坂道での姿勢制御用）
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
		// カプセル判定で頭上に何らかのステージが存在するかを事前にチェック（負荷軽減のため）
		if (pStage->CheckHit_Capsule(
			VAdd(mvPosition, VGet(0, m_ceilCapsuleMinY, 0)),
			VAdd(mvPosition, VGet(0, m_ceilCapsuleMaxY, 0)),
			m_ceilRadius))
		{
			// ステージが存在する場合、さらに詳細な位置を特定するためにレイ（線分）判定を行う
			for (const auto& offset : kOffsets)
			{
				// レイの始点と終点を算出（キャラ位置 + オフセット + 頭上方向の高さ）
				VECTOR start = VAdd(mvPosition, VAdd(offset, VGet(0, m_ceilLineMinY, 0)));
				VECTOR end = VAdd(mvPosition, VAdd(offset, VGet(0, m_ceilLineMaxY, 0)));

				// デバッグ用に、現在判定を行っているレイを青色で描画
				DrawLine3D(start, end, GetColor(0, 0, 255));

				// レイが天井にヒットしたか確認
				VECTOR hit = pStage->CheckHit_Line(start, end);
				if (VSize(hit) > 0.0001f)
				{
					// 天井に当たったのでフラグを立てる
					isHitCeiling = true;

					// ヒットした中で一番低いY座標を保持（頭がぶつかる高さを特定するため）
					if (hit.y < ceilMinY) ceilMinY = hit.y;

					// （補足：ceilMaxYについては、この後の押し戻し処理で必要になる場合に備えて保持）
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
			// 【追加】接地しているならステージの移動量を反映
			if (isGroundedStageFound)
			{
				mvPosition = VAdd(mvPosition, totalMoveDelta);
			}

			// 床にスナップ
			mvPosition.y = bestFloorY - m_floorCapsuleMinY + m_radius;

			// 速度リセット
			mfYVelocity = 0;
			mbIsGround = true;
			mbJump = false;
			mbFall = false;


			// 床の傾きに合わせてキャラ回転を補正
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

				// モデル同期は派生クラスに任せる
				SyncModel();
			}

			// 急斜面なら滑らせる
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
			// ここで接地していない時の処理
			mbIsGround = false;
		}
	}
	else
	{
		mbIsGround = false;

		// 空中では回転を徐々に戻す
		mvRotation.x *= 0.8f;
		mvRotation.z *= 0.8f;
	}

	// 床に何も当たってない場合はデフォルト法線
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

	// 最終的なモデル同期（描画反映）
	SyncModel();
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