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
{
	// Object3D が持っている mpModel にモデルを生成して入れる
	mpModel = new Model(filename, initPos);
	
	// タグを設定（プレイヤーであることを判別できるようにする）
	SetTag(Tag3D::T_Player3D);


	// -------------------------------------------------------------------------
	// 当たり判定パラメータの初期化
	// -------------------------------------------------------------------------
	// ★半径の統一管理
	m_radius = 30.0f;  // 全カプセル共通の半径

	// === 床判定用位置 (緑) ===
	m_floorCapsuleMinY = 3.0f;   // 床判定カプセルの下端の高さ
	m_floorCapsuleMaxY = 40.0f;  // 床判定カプセルの上端の高さ
	m_floorLinePos = 25.0f;  // 床判定用レイキャストのXZ広がり幅
	m_floorLineMinY = 20.0f;  // 床判定用レイキャストの開始高さ
	m_floorLineMaxY = -300.0f; // 床判定用レイキャストの終了高さ

	// === 壁判定用位置 (赤) ===
	m_wallCapsuleMinY = 40.0f;  // 壁判定カプセルの下端の高さ
	m_wallCapsuleMaxY = 60.0f; // 壁判定カプセルの上端の高さ

	// === 天井判定用位置 (青) ===
	m_ceilCapsuleMinY = 60.0f; // 天井判定カプセルの下端の高さ
	m_ceilCapsuleMaxY = 80.0f; // 天井判定カプセルの上端の高さ
	m_ceilLinePos = 15.0f;  // 天井判定用レイキャストのXZ広がり幅
	m_ceilLineMinY = 70.0f; // 天井判定用レイキャストの開始高さ
	m_ceilLineMaxY = 100.0f; // 天井判定用レイキャストの終了高さ


}

// デストラクタ
Player3D::~Player3D()
{
	// モデルの削除
	delete mpModel;
}

// 更新処理
void Player3D::Update()
{
	// --- デバッグ：1キーで原点に戻す ---
	if (CheckHitKey(KEY_INPUT_1))
	{
		mvPosition = VGet(0.0f, 0.0f, 0.0f);
		mfYVelocity = 0.0f;   // 落下速度もリセット
		mbIsGround = false;   // 状態もリセット
	}



	// 移動前の位置を覚えておく（壁のスライドに必須）
	mvOldPosition = mvPosition;

	// 移動処理
	MoveEx();

	// ジャンプ処理
	Jump();
	
	// 回転処理の呼び出し
	RotationByMove();

	// あたり判定の呼び出し
	ResolveCollision3D();


	// モデルの更新
	mpModel->SetPosition(mvPosition);
	mpModel->Update();

	// ベースクラスのUpdateを呼んで、mvPositionの座標をモデル(mpModel)に反映させる
	Object3D::Update();
}

// 描画処理
void Player3D::Draw()
{
	// -------------------------------------------------------------------------
	// デバッグ用：各当たり判定カプセルの可視化 (ワイヤーフレーム描画)
	// -------------------------------------------------------------------------

	// 1. 【床用】カプセル（緑 / Green）
	DrawCapsule3D(
		VAdd(mvPosition, VGet(0.0f, m_floorCapsuleMinY, 0.0f)),
		VAdd(mvPosition, VGet(0.0f, m_floorCapsuleMaxY, 0.0f)),
		m_radius, 8, // ★統一メンバを使用
		GetColor(0, 255, 0), GetColor(0, 255, 0), false
	);

	// 2. 【壁用】カプセル（赤 / Red）
	DrawCapsule3D(
		VAdd(mvPosition, VGet(0.0f, m_wallCapsuleMinY, 0.0f)),
		VAdd(mvPosition, VGet(0.0f, m_wallCapsuleMaxY, 0.0f)),
		m_radius, 8, // ★統一メンバを使用
		GetColor(255, 0, 0), GetColor(255, 0, 0), false
	);

	// 3. 【天井用】カプセル（青 / Blue）
	DrawCapsule3D(
		VAdd(mvPosition, VGet(0.0f, m_ceilCapsuleMinY, 0.0f)),
		VAdd(mvPosition, VGet(0.0f, m_ceilCapsuleMaxY, 0.0f)),
		m_radius, 8, // ★統一メンバを使用
		GetColor(0, 0, 255), GetColor(0, 0, 255), false
	);

	// -------------------------------------------------------------------------

	// デバッグ表示用  白いやつ
	DrawCapsule3D(
		VAdd(mvPosition, VGet(0.0f, 20.0f, 0.0f)),
		VAdd(mvPosition, VGet(0.0f, 40.0f, 0.0f)),
		23.0f, 8,
		GetColor(255, 255, 255), GetColor(255, 255, 255), false
	);


	mpModel->Draw();
	// ベースクラスのDrawを呼んでモデルを描画する
	Object3D::Draw();
}

// 移動処理（ステージとのあたり判定用）
void Player3D::MoveEx()
{
	// 移動方向をきめる -> 速度をかける
	VECTOR moveVec = VGet(0.0f, 0.0f, 0.0f); // 移動方向
	VECTOR upMoveVector = VGet(0.0f, 0.0f, 0.0f); // カメラの上方向（奥方向）ベクトル
	VECTOR leftMoveVector = VGet(0.0f, 0.0f, 0.0f); // カメラの左方向ベクトル

	// カメラの向きから移動ベクトルを求める
	{
		// 上方向への移動ベクトルは、カメラの視点方向からY成分を抜いたもの
		// 注視点からカメラ自身の座標を引いているだけ
		upMoveVector = VSub(Master::mpCamera->GetLookAtPosition(), Master::mpCamera->GetPosition());
		upMoveVector.y = 0.0f;

		// 左方向への移動ベクトルは、上方向への移動ベクトルと、ｙ軸のプラス方向へのベクトルに垂直な方向 (外積)
		// VCross: ベクトル同士の外積を計算してくれるもの ★★★
		leftMoveVector = VCross(upMoveVector, VGet(0.0f, 1.0f, 0.0f));
		leftMoveVector.y = 0.0f;

		// 移動ベクトルは移動量を加味しないので、正規化しておく（ベクトルの長さを１にすること）
		// VNorm: 正規化してくれる便利なもの
		upMoveVector = VNorm(upMoveVector);
		leftMoveVector = VNorm(leftMoveVector);
	}

	if (CheckHitKey(KEY_INPUT_A)) // 左方向
	{
		moveVec = VAdd(moveVec, leftMoveVector);
	}
	if (CheckHitKey(KEY_INPUT_D)) // 右方向
	{
		// moveVec = VSub(moveVec, leftMoveVector);
		// VScaleはVECTORに対して指定した数をかけてあげるもの
		moveVec = VAdd(moveVec, VScale(leftMoveVector, -1.0f));
	}
	if (CheckHitKey(KEY_INPUT_W)) // 奥方向
	{
		moveVec = VAdd(moveVec, upMoveVector);
	}
	if (CheckHitKey(KEY_INPUT_S)) // 手前方向
	{
		// moveVec = VSub(moveVec, upMoveVector);
		// VScaleはVECTORに対して指定した数をかけてあげるもの
		moveVec = VAdd(moveVec, VScale(upMoveVector, -1.0f));
	}

	// 上限突破したら、初期位置に戻す　11/11追加
	if (mvPosition.y >= 4500)
	{
		mvPosition.x = 0;
		mvPosition.y = 0;
		mvPosition.z = 0;
	}

	// -------------------- ゲームパッドのスティック入力 -------------------- ★★        
	int StickX, StickY; // XとYを入れる変数
	VECTOR moveVecPad = VGet(0.0f, 0.0f, 0.0f);  // スティック用

	bool DushFlag = false;

	// 傾きを-1000 から1000で返してくれる関数                                                                    -1000 上
	GetJoypadAnalogInput(&StickX, &StickY, DX_INPUT_PAD1); // 左スティック                                         |
	const int stickDeadZone = 50; // ここで少し触れただけで動くのを制御する             -1000 左                   |0             右 1000
	//const int dushZone = 700; // ここでダッシュしたときの基準                       //---------------------------|---------------------
	// スティックの傾きが一定以上なら移動ベクトルを加算                                                            |
	if (StickX < -stickDeadZone) // 左                                                                            1000　下
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

    // target velocity based on input
	VECTOR targetDir = VGet(0,0,0);
	float inputMag = VSize(moveVec);
	if (inputMag > 0.0001f)
	{
		targetDir = VNorm(moveVec);
		mfTargetAngle = atan2f(targetDir.x, targetDir.z);
	}

	// use fixed deltaTime for now (assume ~60fps)
	const float dt = 1.0f / 60.0f;

	// target horizontal velocity
	VECTOR targetVel = VScale(targetDir, mfSpeed * (inputMag > 0.0001f ? 1.0f : 0.0f));

	// compute acceleration used
	float accel = mbIsGround ? mfAccel : mfAirAccel;
	// if braking (target slower than current), use decel
	if (VSize(targetVel) < VSize(mvVelocity)) accel = mfDecel;

	// smooth velocity toward target
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

	// apply movement
	mvPosition = VAdd(mvPosition, VScale(mvVelocity, 1.0f));


}


// 移動による回転処理
void Player3D::RotationByMove()
{
	// 現在の回転値から目標の回転値の差分を求める
	float  subAngle = mfTargetAngle - mfAngle;

	// ある方向からある方向の差が180以上（以下）にならないはずなので
	// 差の値が180以上（以下）になっていたら矯正する
	if (subAngle < -DX_PI_F)
	{
		subAngle += DX_TWO_PI_F;
	}
	else if (subAngle > DX_PI_F)
	{
		subAngle -= DX_TWO_PI_F;
	}

	// 角度の差分を徐々に０に近づける
	if (subAngle > 0.0f)
	{
		subAngle -= ROTATE_SPEED;
		if (subAngle < 0.0f)
		{
			subAngle = 0.0f;
		}
	}
	// マイナス値だった場合
	else if (subAngle < 0.0f)
	{
		subAngle += ROTATE_SPEED;
		if (subAngle > 0.0f)
		{
			subAngle = 0.0f;
		}
	}

	// 今向いてほしい角度を産出
	mfAngle = mfTargetAngle - subAngle;
	// 回転値を設定
	// 行きたい方向＋円周率
	mvRotation.y = mfAngle + DX_PI_F;
	// モデルに伝える
	// これがないと変らない
	mpModel->SetRotation(mvRotation);
}


void Player3D::Jump()
{
	if (mbIsGround && CheckHitKey(KEY_INPUT_SPACE))
	{
		mfYVelocity = mfJumpPower;
		mbIsGround = false;
	}

	// パッドAボタンでもジャンプ
	if (mbIsGround && (GetJoypadInputState(DX_INPUT_PAD1) & PAD_INPUT_1))
	{
		mfYVelocity = mfJumpPower;
		mbIsGround = false;
	}

    // ---------------- 重力 ----------------
	mfYVelocity += mfGravity;

	// 落下速度の上限を適用（負の方向）
	if (mfYVelocity < -mfMaxFallSpeed) mfYVelocity = -mfMaxFallSpeed;

	mvPosition.y += mfYVelocity;

	//// ---------------- 地面判定 ----------------
	//if (mvPosition.y <= 0.0f)
	//{
	//	mvPosition.y = 0.0f;
	//	mfYVelocity = 0.0f;
	//	mbIsGround = true;
	//}
}


void Player3D::ResolveCollision3D()
{
	// このフレームでプレイヤーが実際に動こうとした差分を計算
	VECTOR moveVec = VSub(mvPosition, mvOldPosition);

	// いったん安全側として「前フレーム位置」に戻す（ここから再構築する）
	mvPosition = mvOldPosition;

	// 天井に当たったかどうかのフラグ
	bool isHitCeiling = false;

	// 天井判定用の最小Y
	float ceilMinY = FLT_MAX;

	// 天井判定用の最大Y
	float ceilMaxY = -FLT_MAX;

	// 床に当たったかどうかのフラグ
	float bestFloorY = -FLT_MAX;
	bool isHitFloor = false;

	// ステージ上の全3Dオブジェクトを取得
	auto stageList = Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()->GetObject3DListByTag(Object3D::T_Stage3D);

	// 指定位置で壁に当たっているかだけを見る簡易チェック用ラムダ
	auto CollidesAt = [&](VECTOR pos) -> bool
		{
			for (auto obj : stageList)
			{
				// Stage以外は無視
				Stage* wall = dynamic_cast<Stage*>(obj);
				if (!wall) continue;

				// カプセルの上下端を作って壁判定
				VECTOR hp, hn;

				// 壁とのカプセル衝突判定（XZ移動チェック用）
				if (wall->CheckHit_Capsule_Wall(
					VAdd(pos, VGet(0, m_wallCapsuleMinY, 0)),
					VAdd(pos, VGet(0, m_wallCapsuleMaxY, 0)),
					m_radius, hp, hn))
				{
					return true;
				}
			}
			// どこにも当たっていない
			return false;
		};

	// =========================================================
	// 1. 入力移動の仮位置を作る（まだ確定ではない）
	// =========================================================

	// プレイヤーが本来行きたかった位置
	VECTOR targetPos = VAdd(mvOldPosition, moveVec);

	// 壁法線の平均（スライド方向計算用）
	VECTOR avgNormal = VGet(0, 0, 0);

	// 衝突した壁の数
	int hitCount = 0;

	// =========================================================
	// 2. 壁との衝突チェック＆押し戻し処理
	// =========================================================
	for (auto obj : stageList)
	{
		Stage* pStage = dynamic_cast<Stage*>(obj);
		if (!pStage) continue;

		// 壁とのカプセル衝突判定
		VECTOR hitPosWall, hitNormal;

		if (pStage->CheckHit_Capsule_Wall(
			VAdd(targetPos, VGet(0, m_wallCapsuleMinY, 0)),
			VAdd(targetPos, VGet(0, m_wallCapsuleMaxY, 0)),
			m_radius,
			hitPosWall,
			hitNormal))
		{
			// XZ平面だけ使う（縦方向影響を除去）
			VECTOR n = VGet(hitNormal.x, 0, hitNormal.z);

			// ノイズ除去（ゼロベクトル対策）
			if (VSize(n) > 0.0001f)
			{
				// 法線を蓄積して後で平均化
				avgNormal = VAdd(avgNormal, VNorm(n));
				hitCount++;

				// =====================================================
				// めり込み補正（カプセル半径外に押し出す）
				// =====================================================

				VECTOR capsuleCenter = VGet(targetPos.x, 0, targetPos.z);
				VECTOR hitPointXZ = VGet(hitPosWall.x, 0, hitPosWall.z);

				// 衝突点から中心方向ベクトル
				VECTOR vToCenter = VSub(capsuleCenter, hitPointXZ);

				// 距離計算
				float dist = VSize(vToCenter);

				// 半径以内ならめり込み
				if (dist < m_radius)
				{
					// 足りない分だけ外へ押し戻す
					float pushLen = m_radius - dist;

					// 少しバッファを加えてめり込み防止
					targetPos = VAdd(targetPos, VScale(VNorm(n), pushLen + 0.05f));
				}
			}
		}
	}

	// =========================================================
	// 3. 壁スライド処理（移動方向を壁に沿わせる）
	// =========================================================
	if (hitCount > 0 && VSize(avgNormal) > 0.0001f)
	{
		// 法線を平均化
		avgNormal = VNorm(avgNormal);

		// 移動ベクトルを法線方向に分解
		float dot = VDot(moveVec, avgNormal);

		// 法線成分を除去＝壁に沿った移動
		VECTOR slide = VSub(moveVec, VScale(avgNormal, dot));

		// スライド先候補
		VECTOR nextPos = VAdd(mvOldPosition, slide);

		// 最終的に安全か確認
		if (!CollidesAt(nextPos))
		{
			mvPosition.x = nextPos.x;
			mvPosition.z = nextPos.z;
		}
		else
		{
			// ダメならXとZを分離してチェック
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
		// 壁に当たってないならそのまま移動
		mvPosition.x = targetPos.x;
		mvPosition.z = targetPos.z;
	}

	// Y移動（ジャンプ・落下）はそのまま適用
	mvPosition.y = targetPos.y;

	// =========================================================
	// 4. 床・天井判定
	// =========================================================
	VECTOR floorNormal = VGet(0, 1, 0);

	for (auto obj : stageList)
	{
		Stage* pStage = dynamic_cast<Stage*>(obj);
		if (!pStage) continue;

		// 5本レイで床を広くチェック
		float step = m_floorLinePos;

		VECTOR offsets[5] =
		{
			VGet(0, 0, 0),
			VGet(step, 0, 0),
			VGet(-step, 0, 0),
			VGet(0, 0, step),
			VGet(0, 0, -step)
		};

		// 各レイで床判定
		for (int i = 0; i < 5; ++i)
		{
			VECTOR start = VAdd(mvPosition, VAdd(offsets[i], VGet(0, m_floorLineMinY, 0)));
			VECTOR end = VAdd(mvPosition, VAdd(offsets[i], VGet(0, m_floorLineMaxY, 0)));

			// レイ衝突で床位置取得
			VECTOR hitPos = pStage->CheckHit_Line(start, end);

			// デバッグ描画（レイ）
			DrawLine3D(start, end, GetColor(255, 0, 0));

			if (VSize(hitPos) > 0.0001f)
			{
				isHitFloor = true;

				// 最も高い床を採用
				if (hitPos.y > bestFloorY)
				{
					bestFloorY = hitPos.y;

					// 法線も取得（傾き計算用）
					VECTOR tempNormal;
					if (pStage->CheckHit_Line_Normal(start, end, tempNormal))
					{
						floorNormal = tempNormal;
					}
				}
			}
		}

		// =====================================================
		// 天井判定（カプセル上方向チェック）
		// =====================================================
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

			// 天井レイチェック
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

	// =========================================================
	// 5. 床確定処理（接地・傾き・滑り）
	// =========================================================
	if (isHitFloor && bestFloorY > -FLT_MAX)
	{
		// 足元位置
		float footY = mvPosition.y + m_floorCapsuleMinY - m_radius;

		// 接地条件
		if (mfYVelocity <= 0 && footY <= bestFloorY + 1.0f)
		{
			// 床にスナップ
			mvPosition.y = bestFloorY - m_floorCapsuleMinY + m_radius;

			// 速度リセット
			mfYVelocity = 0;

			// 状態更新
			mbIsGround = true;
			mbJump = false;
			mbFall = false;

			// =================================================
			// 床の傾きにモデルを合わせる
			// =================================================
			{
				VECTOR fn = floorNormal;

				// ワールド傾き
				float worldTiltX = -asinf(fn.z);
				float worldTiltZ = asinf(fn.x);

				// プレイヤー向き補正
				float sinY = sinf(mfAngle);
				float cosY = cosf(mfAngle);

				float localTiltX = worldTiltX * cosY - worldTiltZ * sinY;
				float localTiltZ = worldTiltX * sinY + worldTiltZ * cosY;

				// スムージング
				mvRotation.x = mvRotation.x * 0.8f + localTiltX * 0.2f;
				mvRotation.z = mvRotation.z * 0.8f + localTiltZ * 0.2f;

				mpModel->SetRotation(mvRotation);
			}

			// =================================================
			// すべり判定（急斜面で滑る）
			// =================================================
			const float slideThreshold = 0.95f;

			// 床の法線がほぼ真上(1.0)より傾いている場合のみ滑り対象
			if (floorNormal.y < slideThreshold)
			{
				// 重力方向ベクトル（下向き）
				VECTOR downDir = VGet(0.0f, -1.0f, 0.0f);

				// -------------------------------------------------
				// 斜面に沿った「落下方向」を作る処理
				// -------------------------------------------------
				// downDirをfloorNormalに投影して、垂直成分を除去する
				// → これにより「斜面に沿う方向」だけが残る
				float dot = VDot(downDir, floorNormal);
				VECTOR slideDir = VSub(downDir, VScale(floorNormal, dot));

				// 有効なベクトルかチェック（ほぼゼロならスキップ）
				if (VSize(slideDir) > 0.0001f)
				{
					// 単位ベクトル化（方向だけを使うため正規化）
					slideDir = VNorm(slideDir);

					// -------------------------------------------------
					// 傾斜の強さを数値化（どれくらい滑るか）
					// -------------------------------------------------
					// floorNormal.yが小さいほど急斜面 → 強く滑る
					float slideStrength = (1.0f - floorNormal.y) * 30.0f;

					// 安全上限（暴走防止）
					if (slideStrength > 50.0f)
						slideStrength = 50.0f;

					// -------------------------------------------------
					// プレイヤー入力による抵抗処理
					// -------------------------------------------------
					// 水平方向の移動量（入力があるかチェック）
					float horSpeed = VSize(VGet(moveVec.x, 0.0f, moveVec.z));

					if (horSpeed > 0.001f)
					{
						// 入力方向を正規化（XZ平面のみ）
						VECTOR inputDir = VNorm(VGet(moveVec.x, 0.0f, moveVec.z));

						// 入力方向と滑り方向の一致度を計算
						// 正:同じ方向 / 負:逆方向
						float align = VDot(inputDir, slideDir);

						// 逆方向に入力している場合は滑りを弱める（踏ん張り）
						if (align < 0.0f)
							slideStrength *= 0.5f;
					}

					// -------------------------------------------------
					// 最終的な滑り移動を適用
					// -------------------------------------------------
					// 斜面方向(slideDir) × 強さ(slideStrength) で移動量決定
					mvPosition = VAdd(mvPosition, VScale(slideDir, slideStrength));
				}
			}
		}
	}
	else
	{
		// 地面にいない
		mbIsGround = false;

		// 角度を戻す
		mvRotation.x *= 0.8f;
		mvRotation.z *= 0.8f;

	}

	if (!isHitFloor)
	{
		floorNormal = VGet(0, 1, 0);
	}



	// =========================================================
	// 6. 天井衝突処理（上方向の押し戻し）
	// =========================================================
	if (isHitCeiling)
	{
		mvPosition.y = ceilMinY - m_ceilLineMaxY;

		// 上昇中なら速度を殺す
		if (mfYVelocity > 0)
			mfYVelocity = 0;
	}
}