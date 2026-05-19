#include "Player3D.h"
#include "Model.h"
#include "InputManager.h"
#include "DxLib.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Stage.h"
#include "ObjectManager.h"

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
	m_radius = 23.0f;  // 全カプセル共通の半径
	m_wallSlideRadius = 23.0f;  // 壁スライド用の半径

	// === 床判定用位置 (緑) ===
	m_floorCapsuleMinY = 3.0f;   // 床判定カプセルの下端の高さ
	m_floorCapsuleMaxY = 25.0f;  // 床判定カプセルの上端の高さ
	m_floorLinePos = 20.0f;  // 床判定用レイキャストのXZ広がり幅
	m_floorLineMinY = 0.0f;  // 床判定用レイキャストの開始高さ
	m_floorLineMaxY = -100.0f; // 床判定用レイキャストの終了高さ

	// === 壁判定用位置 (赤) ===
	m_wallCapsuleMinY = 35.0f;  // 壁判定カプセルの下端の高さ
	m_wallCapsuleMaxY = 40.0f; // 壁判定カプセルの上端の高さ

	// === 天井判定用位置 (青) ===
	m_ceilCapsuleMinY = 36.0f; // 天井判定カプセルの下端の高さ
	m_ceilCapsuleMaxY = 44.0f; // 天井判定カプセルの上端の高さ
	m_ceilLinePos = 20.0f;  // 天井判定用レイキャストのXZ広がり幅
	m_ceilLineMinY = 40.0f; // 天井判定用レイキャストの開始高さ
	m_ceilLineMaxY = 50.0f; // 天井判定用レイキャストの終了高さ


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
	mvPosition.y += mfYVelocity;

	// ---------------- 地面判定 ----------------
	if (mvPosition.y <= 0.0f)
	{
		mvPosition.y = 0.0f;
		mfYVelocity = 0.0f;
		mbIsGround = true;
	}
}


void Player3D::ResolveCollision3D()
{
	// 1. 事前準備
	VECTOR moveVecWall = VSub(mvPosition, mvOldPosition);
	VECTOR newPos = mvPosition;

	float MaxY = -FLT_MAX; // 一番高い床を探す
	float MinY = FLT_MAX;  // 一番低い天井を探す
	bool isHitFloor = false;
	bool isHitCeiling = false;

	// 壁判定用の法線リスト
	std::vector<VECTOR> hitNormals;

	// 2. 【共通ループ】ステージオブジェクトを1回だけ巡回
	auto stageList = Master::mpSceneManager->GetCurrentScene()->GetObjectManager()->GetObject3DListByTag(Object3D::T_Stage3D);
	for (auto obj : stageList)
	{
		Stage* pStage = dynamic_cast<Stage*>(obj);
		if (pStage == nullptr) continue;

		// --- (A) 壁の当たり判定 ---
		VECTOR hitPosWall, hitNormal;
		if (pStage->CheckHit_Capsule_Wall(
			VAdd(newPos, VGet(0.0f, m_wallCapsuleMinY, 0.0f)),
			VAdd(newPos, VGet(0.0f, m_wallCapsuleMaxY, 0.0f)),
			m_radius, hitPosWall, hitNormal)) // ★統一メンバを使用
		{
			VECTOR mvXZ = VGet(hitNormal.x, 0.0f, hitNormal.z);
			mvXZ = VNorm(mvXZ);
			hitNormals.push_back(mvXZ); // 正しい方向だけを入れる
		}

		// --- (B) 床の当たり判定 ---
		if (pStage->CheckHit_Capsule(
			VAdd(mvPosition, VGet(0.0f, m_floorCapsuleMinY, 0.0f)),
			VAdd(mvPosition, VGet(0.0f, m_floorCapsuleMaxY, 0.0f)),
			m_radius)) // ★統一メンバを使用
		{
			VECTOR LineSet[5]
			{
				VGet(0.0f, 0.0f, 0.0f),
				VGet(m_floorLinePos, 0.0f, 0.0f),
				VGet(-m_floorLinePos, 0.0f, 0.0f),
				VGet(0.0f, 0.0f, m_floorLinePos),
				VGet(0.0f, 0.0f, -m_floorLinePos)
			};

			for (int i = 0; i < 5; i++)
			{
				VECTOR start = VAdd(mvPosition, VAdd(LineSet[i], VGet(0.0f, m_floorLineMinY, 0.0f)));
				VECTOR end = VAdd(mvPosition, VAdd(LineSet[i], VGet(0.0f, m_floorLineMaxY, 0.0f)));
				VECTOR hitPos = pStage->CheckHit_Line(start, end);

				// デバッグ用
				DrawLine3D(start, end, GetColor(255, 0, 0));

				if (hitPos.x != 0.0f || hitPos.y != 0.0f || hitPos.z != 0.0f)
				{
					isHitFloor = true;
					if (hitPos.y > MaxY)
					{
						MaxY = hitPos.y;
					}
				}
			}
		}

		// --- (C) 天井の当たり判定 ---
		if (pStage->CheckHit_Capsule(
			VAdd(mvPosition, VGet(0.0f, m_ceilCapsuleMinY, 0.0f)),
			VAdd(mvPosition, VGet(0.0f, m_ceilCapsuleMaxY, 0.0f)),
			m_radius)) // ★統一メンバを使用
		{
			VECTOR LineSet[5]
			{
				VGet(0.0f, 0.0f, 0.0f),
				VGet(m_ceilLinePos, 0.0f, 0.0f),
				VGet(-m_ceilLinePos, 0.0f, 0.0f),
				VGet(0.0f, 0.0f, m_ceilLinePos),
				VGet(0.0f, 0.0f, -m_ceilLinePos)
			};

			for (int i = 0; i < 5; i++)
			{
				VECTOR start = VAdd(mvPosition, VAdd(LineSet[i], VGet(0.0f, m_ceilLineMinY, 0.0f)));
				VECTOR end = VAdd(mvPosition, VAdd(LineSet[i], VGet(0.0f, m_ceilLineMaxY, 0.0f)));
				VECTOR hitPos = pStage->CheckHit_Line(start, end);

				if (VSquareSize(hitPos) > 0.0f)
				{
					isHitCeiling = true;
					if (hitPos.y < MinY)
					{
						MinY = hitPos.y;
					}
				}
			}
		}
	}

	// 3. ループの後の結果確定処理

	// --- (A') 壁のスライド・位置修正 ---
	if (!hitNormals.empty())
	{
		VECTOR slideVec = moveVecWall;
		for (auto osu : hitNormals)
		{
			float meri = VDot(slideVec, osu);
			if (meri < 0.0f)
			{
				slideVec = VSub(slideVec, VScale(osu, meri));
			}
		}
		VECTOR KPos = VAdd(mvOldPosition, slideVec);

		bool slideHit = false;
		for (auto obj : stageList)
		{
			Stage* wall = dynamic_cast<Stage*>(obj);
			if (wall == nullptr) continue;

			VECTOR hitPos, hitNormal;
			if (wall->CheckHit_Capsule_Wall(
				VAdd(KPos, VGet(0, m_wallCapsuleMinY, 0)),
				VAdd(KPos, VGet(0, m_wallCapsuleMaxY, 0)),
				m_wallSlideRadius, hitPos, hitNormal))
			{
				slideHit = true;
				break;
			}
		}

		if (!slideHit)
		{
			mvPosition.x = KPos.x;
			mvPosition.z = KPos.z;
		}
		else
		{
			VECTOR move = moveVecWall;
			VECTOR oldPos = mvOldPosition;

			// X方向のチェック
			VECTOR nextX = VAdd(oldPos, VGet(move.x, 0, 0));
			bool hitX = false;
			VECTOR normX = VGet(0, 0, 0);
			for (auto obj : stageList)
			{
				Stage* wall = dynamic_cast<Stage*>(obj);
				if (wall == nullptr) continue;

				VECTOR hitPos, hitNormal;
				if (wall->CheckHit_Capsule_Wall(
					VAdd(nextX, VGet(0, m_wallCapsuleMinY, 0)),
					VAdd(nextX, VGet(0, m_wallCapsuleMaxY, 0)),
					m_wallSlideRadius, hitPos, hitNormal))
				{
					hitX = true;
					normX = hitNormal;
					break;
				}
			}

			if (!hitX) { mvPosition.x = nextX.x; }
			else { mvPosition.x = oldPos.x + (normX.x * 0.15f); }

			// Z方向のチェック
			VECTOR basePos = mvPosition;
			VECTOR nextZ = VAdd(basePos, VGet(0, 0, move.z));
			bool hitZ = false;
			VECTOR normZ = VGet(0, 0, 0);
			for (auto obj : stageList)
			{
				Stage* wall = dynamic_cast<Stage*>(obj);
				if (wall == nullptr) continue;

				VECTOR hp, hn;
				if (wall->CheckHit_Capsule_Wall(
					VAdd(nextZ, VGet(0, m_wallCapsuleMinY, 0)),
					VAdd(nextZ, VGet(0, m_wallCapsuleMaxY, 0)),
					m_wallSlideRadius, hp, hn))
				{
					hitZ = true;
					normZ = hn;
					break;
				}
			}

			if (!hitZ) { mvPosition.z = nextZ.z; }
			else { mvPosition.z = oldPos.z + (normZ.z * 0.15f); }
		}
	}

	// --- (B') 床の最終位置確定 ---
	if (isHitFloor)
	{
		mfGroundY = MaxY;
		if (!mbJump)
		{
			mvPosition.y = mfGroundY;
		}
		mbHitUp = false;
	}

	// --- (C') 天井の最終位置確定 ---
	if (isHitCeiling)
	{
		mbHitUp = true;
		mvPosition.y = MinY - m_ceilLineMaxY;
		if (mfJumpPower > 0.0f)
		{
			mfJumpPower = -0.1f;
		}
	}

	// --- (D') 落下状態への移行チェック ---
	if (!isHitFloor && !isHitCeiling)
	{
		mbHitUp = false;

		float closestGroundY = -FLT_MAX;
		for (auto obj : stageList)
		{
			Stage* pStage = dynamic_cast<Stage*>(obj);
			if (pStage == nullptr) continue;

			VECTOR start = VAdd(mvPosition, VGet(0.0f, 60.0f, 0.0f));
			VECTOR end = VAdd(mvPosition, VGet(0.0f, -2000.0f, 0.0f));
			VECTOR hitPos = pStage->CheckHit_Line(start, end);

			if (hitPos.x != 0.0f || hitPos.y != 0.0f || hitPos.z != 0.0f)
			{
				if (hitPos.y > closestGroundY)
				{
					closestGroundY = hitPos.y;
				}
			}
		}

		if (closestGroundY != -FLT_MAX)
		{
			mfGroundY = closestGroundY;
		}

		const float GroundHight = 100.0f;
		if (!mbJump)
		{
			if (mvPosition.y - mfGroundY > GroundHight)
			{
				mbJump = true;
				mbFall = true;
			}
		}
	}
}