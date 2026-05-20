#include "Camera.h"
#include "Master.h"
#include "ObjectManager.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include <cmath> // cosなどを使うためのインクルード

Camera::Camera()
	: mfHorizontalAngle(0.0f)
	, mfVerticalAngle(0.0f)
	, mvPosition(VGet(0.0f, 0.0f, 100.0f))     // カメラの座標
	, mvLookAtPosition(VGet(0.0f, 0.0f, 0.0f)) // カメラが見ている方向
	, mpTarget(nullptr)                        // まずは空っぽ
	, mpTargetEnemy(nullptr)                   // まずは空っぽ 敵追加 1117
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
	// カメラの向く対象の処理
	if (mpTarget == nullptr)
	{
		// プレイヤーのオブジェクトを探し出している
		mpTarget = Master::mpSceneManager->GetCurrentScene()
			->GetObjectManager()->GetObject3DByTag(Object3D::T_Player3D);
	}

	if (mpTarget != nullptr) // ターゲットがいなかったら
	{
		// 基準座標を対象の座標にする
		mvLookAtPosition = mpTarget->GetPosition(); // ターゲットからポジションをとってくる
		mvLookAtPosition.y += 110.0f;
	}
	else
	{
		// 注視点を少し上にずらす
		mvLookAtPosition.y = 110.0f;
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

	//Debug_DrawLockOnFOV(); // 可視化してるやつ
}

void Camera::UpdateRotation()
{
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
		mfHorizontalAngle += mfCurrentSpeedH; // それをここでやっても+ だけど-5とかだから左に行くはず
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

	// カメラを反映
	SetCameraPositionAndTarget_UpVecY(mvPosition, mvLookAtPosition);
}