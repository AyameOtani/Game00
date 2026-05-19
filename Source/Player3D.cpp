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

	if (VSize(moveVec) > 0.0f)
	{
		// 移動しているので正規化する（長さ1にする）
		moveVec = VNorm(moveVec);

		// 移動方向から目標の角度を計算する
		mfTargetAngle = atan2f(moveVec.x, moveVec.z);
	}

	// 最後に座標を更新
	mvPosition = VAdd(mvPosition, VScale(moveVec, mfSpeed));
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
	// このフレームでの純粋な「移動しようとした量」を取り出す
	VECTOR moveVec = VSub(mvPosition, mvOldPosition);

	// 一旦、プレイヤーの位置を移動前の安全な位置に戻す
	mvPosition = mvOldPosition;

	bool isHitCeiling = false;
	float ceilMinY = FLT_MAX;
	float ceilMaxY = -FLT_MAX;

	// 床判定用
	float bestFloorY = -FLT_MAX;
	bool isHitFloor = false;

	auto stageList = Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()->GetObject3DListByTag(Object3D::T_Stage3D);

	// いずれかの壁にめり込んでいるかを調べるラムダ式
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

	// =========================================================
	// 1 & 2. 壁判定と押し出し・スライド処理（XZ移動の確定）
	// =========================================================
	// まずはキー入力通りに進んだ仮の座標を作る
	VECTOR targetPos = VAdd(mvOldPosition, moveVec);

	VECTOR avgNormal = VGet(0, 0, 0);
	int hitCount = 0;

	// 壁との接触判定を行い、めり込みを検出する
	for (auto obj : stageList)
	{
		Stage* pStage = dynamic_cast<Stage*>(obj);
		if (!pStage) continue;

		VECTOR hitPosWall, hitNormal;
		if (pStage->CheckHit_Capsule_Wall(
			VAdd(targetPos, VGet(0, m_wallCapsuleMinY, 0)),
			VAdd(targetPos, VGet(0, m_wallCapsuleMaxY, 0)),
			m_radius,
			hitPosWall,
			hitNormal))
		{
			// XZ平面の法線を取り出す
			VECTOR n = VGet(hitNormal.x, 0, hitNormal.z);
			if (VSize(n) > 0.0001f)
			{
				avgNormal = VAdd(avgNormal, VNorm(n));
				hitCount++;

				// 【超重要】めり込んでいる分を直接押し戻す処理
				// カプセルの中心から衝突点までの距離を計算し、半径分外側に押し出す
				VECTOR capsuleCenter = VGet(targetPos.x, 0, targetPos.z);
				VECTOR hitPointXZ = VGet(hitPosWall.x, 0, hitPosWall.z);
				VECTOR vToCenter = VSub(capsuleCenter, hitPointXZ);
				float dist = VSize(vToCenter);

				if (dist < m_radius)
				{
					// 足りない距離分、法線方向に押し戻す
					float pushLen = m_radius - dist;
					targetPos = VAdd(targetPos, VScale(VNorm(n), pushLen + 0.05f));
				}
			}
		}
	}

	// 壁に当たっていた場合、スライドを計算する
	if (hitCount > 0 && VSize(avgNormal) > 0.0001f)
	{
		avgNormal = VNorm(avgNormal);

		// 壁に沿ったスライドベクトルを計算
		float dot = VDot(moveVec, avgNormal);
		VECTOR slide = VSub(moveVec, VScale(avgNormal, dot));

		// 押し戻し済みの位置にスライド移動量を足す
		VECTOR nextPos = VAdd(mvOldPosition, slide);

		// 最終チェック
		if (!CollidesAt(nextPos))
		{
			mvPosition.x = nextPos.x;
			mvPosition.z = nextPos.z;
		}
		else
		{
			// 軸分離テスト
			VECTOR tryX = mvOldPosition;
			tryX.x += slide.x;
			if (!CollidesAt(tryX)) { mvPosition.x = tryX.x; }

			VECTOR tryZ = mvPosition;
			tryZ.z += slide.z;
			if (!CollidesAt(tryZ)) { mvPosition.z = tryZ.z; }
		}
	}
	else
	{
		// 壁に当たっていないならそのままXZを移動
		mvPosition.x = targetPos.x;
		mvPosition.z = targetPos.z;
	}

	// Y軸（ジャンプや落下）の移動を適用
	mvPosition.y = targetPos.y;

	// =========================================================
	// 3. 床・天井判定（位置が確定したあとに上下の調整を行う）
	// =========================================================
	VECTOR floorNormal = VGet(0, 1, 0); // 床の傾き（デフォルトは真上）

	for (auto obj : stageList)
	{
		Stage* pStage = dynamic_cast<Stage*>(obj);
		if (!pStage) continue;

		// 床判定（5本レイ）
		{
			float step = m_floorLinePos;
			VECTOR offsets[5] = {
				VGet(0, 0, 0), VGet(step, 0, 0), VGet(-step, 0, 0), VGet(0, 0, step), VGet(0, 0, -step)
			};

			for (int i = 0; i < 5; ++i)
			{
				VECTOR start = VAdd(mvPosition, VAdd(offsets[i], VGet(0, m_floorLineMinY, 0)));
				VECTOR end = VAdd(mvPosition, VAdd(offsets[i], VGet(0, m_floorLineMaxY, 0)));

				// まずは従来通り、一番高い床のY座標を取るために呼ぶ
				VECTOR hitPos = pStage->CheckHit_Line(start, end);

				DrawLine3D(start, end, GetColor(255, 0, 0));

				if (VSize(hitPos) > 0.0001f)
				{
					isHitFloor = true;
					if (hitPos.y > bestFloorY)
					{
						bestFloorY = hitPos.y;

						// ⭐ここで新しい関数を呼んで、一番高い床の「法線」だけを受け取る
						VECTOR tempNormal;
						if (pStage->CheckHit_Line_Normal(start, end, tempNormal))
						{
							floorNormal = tempNormal;
						}
					}
				}
			}
		}

		// 天井判定（既存のまま触らなくてOK）
		if (pStage->CheckHit_Capsule(
			VAdd(mvPosition, VGet(0, m_ceilCapsuleMinY, 0)),
			VAdd(mvPosition, VGet(0, m_ceilCapsuleMaxY, 0)),
			m_radius))
		{
			float m_ceilLinePos_val = m_ceilLinePos;
			VECTOR LineSet[5] = {
				VGet(0,0,0), VGet(m_ceilLinePos_val,0,0), VGet(-m_ceilLinePos_val,0,0), VGet(0,0,m_ceilLinePos_val), VGet(0,0,-m_ceilLinePos_val)
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

	// =========================================================
	// 4. 床・天井の位置確定 ＆ すべりだい処理
	// =========================================================
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

			// モデルの傾き（床法線に合わせる）
			{
				// 1. まずはワールド空間での傾きを計算
				VECTOR fn = floorNormal;
				float worldTiltX = -asinf(fn.z); // Z軸の傾斜 -> ワールドX軸まわりの回転
				float worldTiltZ = asinf(fn.x);  // X軸の傾斜 -> ワールドZ軸まわりの回転

				// 2. プレイヤーの現在の向き（Y軸回転：mfAngle）を考慮して、ローカルな傾きに変換する
				// プレイヤーの向きに合わせてサイン・コサインで回転を補正するよ
				float sinY = sinf(mfAngle);
				float cosY = cosf(mfAngle);

				// プレイヤーから見た正しい前後の傾き（X回転）と左右の傾き（Z回転）を計算
				float localTiltX = worldTiltX * cosY - worldTiltZ * sinY;
				float localTiltZ = worldTiltX * sinY + worldTiltZ * cosY;

				// 3. スムーズに補間して適用
				mvRotation.x = mvRotation.x * 0.8f + localTiltX * 0.2f;
				mvRotation.z = mvRotation.z * 0.8f + localTiltZ * 0.2f;

				// RotationByMove() で y は更新されているので、そのままモデルにセット
				mpModel->SetRotation(mvRotation);
			}

			// 【すべりだい判定】: より緩やかな傾斜でも滑るようにする
			const float slideThreshold = 0.95f; // cos(≈18deg) これより小さいと滑る
			if (floorNormal.y < slideThreshold)
			{
				VECTOR downDir = VGet(0.0f, -1.0f, 0.0f);
				float dot = VDot(downDir, floorNormal);
				VECTOR slideDir = VSub(downDir, VScale(floorNormal, dot));

				if (VSize(slideDir) > 0.0001f)
				{
					slideDir = VNorm(slideDir);

					// 傾斜に応じて滑り強さを決める（急なら強く）
					float slideStrength = (1.0f - floorNormal.y) * 12.0f; // 調整可能
					// 上限を設ける
					if (slideStrength > 24.0f) slideStrength = 24.0f;

					// プレイヤーが移動入力をしている場合は滑りを少し抑える（抵抗）
					float horSpeed = VSize(VGet(moveVec.x, 0.0f, moveVec.z));
					if (horSpeed > 0.001f)
					{
						// 前方向入力が滑り方向と同じならそのまま、逆なら滑りを減らす
						VECTOR inputDir = VNorm(VGet(moveVec.x, 0.0f, moveVec.z));
						float align = VDot(inputDir, slideDir);
						if (align < 0.0f) slideStrength *= 0.5f; // 抵抗がある場合は弱める
					}

					mvPosition = VAdd(mvPosition, VScale(slideDir, slideStrength));
				}
			}
		}
	}
	else
	{
		mbIsGround = false;
	}

	if (isHitCeiling)
	{
		mvPosition.y = ceilMinY - m_ceilLineMaxY;
		if (mfYVelocity > 0) mfYVelocity = 0;
	}
}