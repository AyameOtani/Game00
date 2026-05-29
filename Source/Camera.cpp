#include "Camera.h"
#include "Master.h"
#include "ObjectManager.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Object3D.h"
#include <cmath> // cosなどを使うためのインクルード

Camera::Camera()
	: mfHorizontalAngle(0.0f)
	, mfVerticalAngle(0.0f)
	, mvPosition(VGet(0.0f, 0.0f, 100.0f))     // カメラの座標
	, mvLookAtPosition(VGet(0.0f, 0.0f, 0.0f)) // カメラが見ている方向
	, mpTarget(nullptr)                        // まずは空っぽ
	, mpTargetEnemy(nullptr)                   // まずは空っぽ 敵追加 1117
	// 画面揺れ用変数の初期化
	, mnShakeTime(0)
	, mnShakeTimeCount(0)
	, mfShakeAngle(0.0f)
	, mfShakeTimeCounter(0.0f)
	, mfShakeTime(0.0f)
	, mfShakeWidth(0.0f)
	, mfShakeAngleSpeed(0.0f)
	, mfStepTime(1.0f)
	, mvShakePosition(VGet(0.0f, 0.0f, 0.0f))
	// 制御フラグ・速度の初期化
	, mbTitleMode(false)
	, mbStop(false)
{

}
Camera::~Camera()
{

}

void Camera::Initialize()
{
	// カメラのクリッピング距離の設定----カメラの映す距離（どこからどこまでを映すか）
	SetCameraNearFar(100.0f, 50000.0f); // 100 ～ 50000までオブジェクトを映す

	// 背景色の設定 (今は灰色)
	SetBackgroundColor(128, 128, 128);

	// カメラ設定を反映  上方向はベクトルYだから
	SetCameraPositionAndTarget_UpVecY(mvPosition, mvLookAtPosition);

	// 更新処理を一度行っておく
	Update();
}

void Camera::Update()
{
	if (mbTitleMode)
	{
		mfHorizontalAngle += 0.2f;

		// 見下ろし角度
		mfVerticalAngle = 15.0f;
		// 中心は完全固定
		mvLookAtPosition = VGet(0.0f, 0.0f, 0.0f);

		const float distance = 2800.0f;	   // 距離

		VECTOR temp;
		temp.x = distance * cosf(mfVerticalAngle / 180.0f * DX_PI_F)
			* sinf(mfHorizontalAngle / 180.0f * DX_PI_F);

		temp.y = distance * sinf(mfVerticalAngle / 180.0f * DX_PI_F);

		temp.z = -(distance * cosf(mfVerticalAngle / 180.0f * DX_PI_F)
			* cosf(mfHorizontalAngle / 180.0f * DX_PI_F));

		mvPosition = VAdd(temp, mvLookAtPosition);


		SetCameraPositionAndTarget_UpVecY(mvPosition, mvLookAtPosition);
	}
	else
	{
		// カメラの向く対象の処理
		// ターゲットが削除済みなら一度クリアする
		if (mpTarget != nullptr)
		{
			if (mpTarget->IsDeleteFlag())
			{
				mpTarget = nullptr;
			}
		}

		if (mpTarget == nullptr)
		{
			// ターゲットが存在しない場合はプレイヤーを探して設定する
			mpTarget = Master::mpSceneManager->GetCurrentScene()
				->GetObjectManager()->GetObject3DByTag(Object3D::T_Player3D);
		}

		if (mpTarget != nullptr)
		{
			// ターゲットに追従するように注視位置を更新する
			mvLookAtPosition = mpTarget->GetPosition();
			mvLookAtPosition.y += 110.0f;
		}
		else
		{
			// ターゲットが存在しない場合は何もしない
			// （mvLookAtPositionを更新しないことで、シーン切り替え時などの急なカメラ移動を防ぐ）
		}

		//この中で処理は完結する　分かりやすいように
		{
			VECTOR temp;   // 作業用変数

			// 球面上の座標を求める
			// ★DX_PI_F は 3.14 と同じ意味
			// 250.0f は注視点からどれだけ離れているかという意味
			// 距離は変えないで回転したい
			const float distance = 500.0f;
			temp.x = distance * cosf(mfVerticalAngle / 180.0f * DX_PI_F) * sinf(mfHorizontalAngle / 180.0f * DX_PI_F);    // X座標
			temp.y = distance * sinf(mfVerticalAngle / 180.0f * DX_PI_F);                                                 // Y座標
			temp.z = -(distance * cosf(mfVerticalAngle / 180.0f * DX_PI_F) * cosf(mfHorizontalAngle / 180.0f * DX_PI_F)); // Z座標

			{
				// 求めた座標と注視点の座標を足した位置がカメラ座標になる
				mvPosition = VAdd(temp, mvLookAtPosition);
			}

			// カメラ設定を反映
			SetCameraPositionAndTarget_UpVecY(mvPosition, mvLookAtPosition);
		}

		UpdateRotation();
	}

	//Debug_DrawLockOnFOV(); // 可視化してるやつ
}

void Camera::UpdateRotation()
{
	if (mbStop) return;


	{
		int StickX, StickY;
		GetJoypadAnalogInputRight(&StickX, &StickY, DX_INPUT_PAD1);
		const int stickDeadZone = 50;

		// 水平方向
		if (CheckHitKey(KEY_INPUT_LEFT) || StickX < -stickDeadZone)
		{
			mfCurrentSpeedH -= ADD_SPEED;
			if (mfCurrentSpeedH < -MAX_SPEED)	 // ここで-なんかになって
			{
				mfCurrentSpeedH = -MAX_SPEED;
			}
		}
		else if (CheckHitKey(KEY_INPUT_RIGHT) || StickX > stickDeadZone)
		{
			mfCurrentSpeedH += ADD_SPEED;
			if (mfCurrentSpeedH > MAX_SPEED)
			{
				mfCurrentSpeedH = MAX_SPEED;
			}
		}
		else
		{
			// 徐々に遅く
			if (mfCurrentSpeedH > 0)
			{
				mfCurrentSpeedH -= SUB_SPEED;
				if (mfCurrentSpeedH < 0)
				{
					mfCurrentSpeedH = 0;
				}
			}
			else if (mfCurrentSpeedH < 0)
			{
				mfCurrentSpeedH += SUB_SPEED;
				if (mfCurrentSpeedH > 0)
				{
					mfCurrentSpeedH = 0;
				}
			}
		}


		// 垂直方向
		if (CheckHitKey(KEY_INPUT_UP) || StickY < -stickDeadZone)
		{
			mfCurrentSpeedV += ADD_SPEED * 1.7f;
			if (mfCurrentSpeedV > MAX_SPEED) mfCurrentSpeedV = MAX_SPEED;
		}
		else if (CheckHitKey(KEY_INPUT_DOWN) || StickY > stickDeadZone)
		{
			mfCurrentSpeedV -= ADD_SPEED * 1.7f;
			if (mfCurrentSpeedV < -MAX_SPEED) mfCurrentSpeedV = -MAX_SPEED;
		}
		else
		{
			// 徐々に遅く
			if (mfCurrentSpeedV > 0)
			{
				mfCurrentSpeedV -= SUB_SPEED * 1.2f;
				if (mfCurrentSpeedV < 0)
				{
					mfCurrentSpeedV = 0;
				}
			}
			else if (mfCurrentSpeedV < 0)
			{
				mfCurrentSpeedV += SUB_SPEED * 1.2f;
				if (mfCurrentSpeedV > 0)
				{
					mfCurrentSpeedV = 0;
				}
			}
		}

		//  計算した速度を角度に反映
		mfHorizontalAngle -= mfCurrentSpeedH; // それをここでやっても+ だけど-5とかだから左に行くはず
		mfVerticalAngle += mfCurrentSpeedV;

		// 角度の正規化clamp
		if (mfHorizontalAngle >= 180.0f) mfHorizontalAngle -= 360.0f;
		if (mfHorizontalAngle < -180.0f) mfHorizontalAngle += 360.0f;

		if (mfVerticalAngle >= 80.0f) mfVerticalAngle = 80.0f;
		if (mfVerticalAngle <= 11.0f)  mfVerticalAngle = 11.0f;
	}
}



// ゲームを何集荷したときにカメラの位置が前のままになるのを防ぐため
// Scene3D.cppの初期化で呼んでいる
void Camera::Reset()
{
	// 座標と角度を初期化
	mvPosition = VGet(0.0f, 0.0f, 100.0f);
	mvLookAtPosition = VGet(0.0f, 0.0f, 0.0f);
	mfHorizontalAngle = 0.0f;
	mfVerticalAngle = 0.0f;
	mpTarget = nullptr;
	mpTargetEnemy = nullptr;

	mfHorizontalAngle = -90.0f; // 角度設定　これはステージのせいだ

	// カメラを反映
	SetCameraPositionAndTarget_UpVecY(mvPosition, mvLookAtPosition);
}