#pragma once
#include "DxLib.h"
#include <vector>

class Object3D;
class Enemy3D;

class Camera
{
public:
	Camera();
	~Camera();

	void Initialize();      // 初期化
	void Update();          // 更新
	void UpdateRotation(); // 回転処理
	void Reset(); // シーン変えがあったときにカメラ位置もリセットするため


	VECTOR GetPosition() { return mvPosition; } // 座標の取得
	VECTOR GetLookAtPosition() { return mvLookAtPosition; }// 注視点取得
	float GetHorizontalAngle() { return mfHorizontalAngle; } // 水平方向アングルの取得

	void SetPosition(VECTOR pos) { mvPosition = pos; } // 座標の設定
	void SetLookAtPosition(VECTOR lookAt) { mvLookAtPosition = lookAt; } // 注視点の設定
	void SetHorizontalAngle(float angle) { mfHorizontalAngle = angle; } // 水平方向アングルの設定

	// カメラが見ている方向を返す
	float GetHorizonAngle() const { return mfHorizontalAngle; }

	// タイトルモードのゲッターセッター
	bool GetTitleMode() const { return mbTitleMode; }
	void SetTitleMode(bool titleMode) { mbTitleMode = titleMode; }

	// 停止の有無
	bool GetStop() const { return mbStop; }
	void SetStop(bool stop) { mbStop = stop; }


	// ★New★
	// 画面揺れ
	//void Shake();
	//void SetupShake(float time, float width, float angleSpeed, float stepTime = 1.0f);
	VECTOR GetShakePosition() { return mvShakePosition; } // 揺れのゲッター

	//void SetTarget(Object3D* target) { mpTarget = target; } // ターゲット設定


private:
	float mfHorizontalAngle;  // 水平方向アングル  ここでもカメラの座標が変わる（縦座標はそのまま）
	float mfVerticalAngle;    // 垂直方向アングル  ここでもカメラの座標が変わる（横方向はそのまま）

	VECTOR mvPosition;        // カメラ座標
	VECTOR mvLookAtPosition;  // カメラの注視点座標
	Object3D* mpTarget; // カメラを向ける対象
	Object3D* mpTargetEnemy; // カメラを向ける対象 追加

	// 画面振れ
	int mnShakeTime;
	int mnShakeTimeCount;

	float mfShakeAngle;
	float mfShakeTimeCounter;
	float mfShakeTime;
	float mfShakeWidth;
	float mfShakeAngleSpeed;
	float mfStepTime;
	VECTOR mvShakePosition;

private:
	float mfCameraSpeed = 4.0; // カメラが動く速さ

private:
	float mfCurrentSpeedH = 0.0f; // 現在の水平回転速度
	float mfCurrentSpeedV = 0.0f; // 現在の垂直回転速度

	const float MAX_SPEED = 5.0f;     // 最大速度
	const float ADD_SPEED = 0.32f;   // 加速度
	const float SUB_SPEED = 0.32f;  // 減速速度


	bool mbTitleMode; // タイトルモードか

	bool mbStop; // 停止するか;
};


//------------------------------------------//
//
//        
//
//           カメラ
//- - - - ( VECTOR mvPosition ) - - - - - - -水平方向アングル (点線の軌道)
//
//
//                     物体
//        ( VECTOR mvLookAtPosition )
// 
//------------------------------------------//
//
//                    カメラ
//            ( VECTOR mvPosition )
//                      |
//                                    垂直方向アングル (点線の軌道)
//                      |
//
//                     物体
//        ( VECTOR mvLookAtPosition )
//------------------------------------------//
//
//            カメラ
//     ( VECTOR mvPosition )
//
//
//
//
//                     物体
//        ( VECTOR mvLookAtPosition )

