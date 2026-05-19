#include "Player3D.h"
#include "Model.h"
#include "InputManager.h"
#include "DxLib.h"
#include "Master.h"

// コンストラクタ
Player3D::Player3D(VECTOR initPos, std::string filename)
	: Object3D(initPos)
{
	// Object3D が持っている mpModel にモデルを生成して入れる
	mpModel = new Model(filename, initPos);
	
	// タグを設定（プレイヤーであることを判別できるようにする）
	SetTag(Tag3D::T_Player3D);
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
	// 移動処理
	MoveEx();

	// ジャンプ処理
	Jump();
	
	// 回転処理の呼び出し
	RotationByMove();

	// モデルの更新
	mpModel->SetPosition(mvPosition);
	mpModel->Update();

	// ベースクラスのUpdateを呼んで、mvPositionの座標をモデル(mpModel)に反映させる
	Object3D::Update();
}

// 描画処理
void Player3D::Draw()
{
	// デバック表示用
	DrawCapsule3D(VAdd(mvPosition, VGet(0.0f, 20.0f, 0.0f)), VAdd(mvPosition, VGet(0.0f, 40.0f, 0.0f)),
		23.0f,
		8, GetColor(255, 255, 255),
		GetColor(255, 255, 255), false);


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