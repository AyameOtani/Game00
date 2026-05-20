#pragma once

#include "DxLib.h"
#include <string>

class Model;
class Object3D
{
public:

	// オブジェクトを見分けるためのタグ設定
	enum Tag3D
	{
		None3D = 0, //設定なし

		T_Player3D = 2000, // プレイヤーのタグ
		T_Wall3D = 2100,   // 壁のタグ  画像のやつ
		T_Floor3D = 2150,   // 床のタグ 画像のやつ
		T_Enemy3D = 2200,  // 敵のタグ
		T_Stage3D = 2250,  // ステージのタグ  ３Dモデル床だけ

		T_Bullet3D = 3000, // 弾のタグ
	};


	// 透明化不透明化を判別させる 1218
	// これをいれないとバリアあの透明が原点付近で反映されない
	enum DrawType
	{
		ToumeiModel,
		HutoumeiModel,
	};
	virtual DrawType GetDrawType() const { return DrawType::HutoumeiModel; }



public:
	Object3D(VECTOR initPos); // コンストラクタ
	virtual ~Object3D();      //デストラクタ  継承されるから virtual がつく

	virtual void Update(); // 更新
	virtual void Draw();   // 描画


	// ★New!! Zソート用★
public: // ソート関数定義

	// Zソート用の設定
	void SetCameraDistance(float distance) { mfCurrentCameraDistance = distance; }

	// カメラ距離を基準にソートするためのオペレーター
	// note: カメラとの距離が離れるほどリストの手前に来るようにする（先に描画したい）
	struct CompareZOrder
	{
		bool operator()(Object3D* a, Object3D* b) const
		{
			return a->mfCurrentCameraDistance > b->mfCurrentCameraDistance;
		}
	};


	// Zソート順にすると、mpModelがバグるため、いれる
	virtual void LateUpdate();



public: // ゲッター・セッター用

	// 座標
	void SetPosition(VECTOR pos) { mvPosition = pos; } // 座標設定
	VECTOR GetPosition() { return mvPosition; }        // 座標取得

	// 回転
	void SetRotation(VECTOR rot) { mvRotation = rot; } // 回転設定
	VECTOR GetRotation() { return mvRotation; }        // 回転取得

	// 削除フラグ
	void SetDeleteFlag(bool flag) { mbDeleteFlag = flag; }  // 削除フラグ設定
	bool IsDeleteFlag() { return mbDeleteFlag; }            // 削除フラグ取得

	//タグ
	void SetTag(Tag3D tag) { mnTag = tag; }  // タグ設定
	Tag3D GetTag() { return mnTag; }         // タグ取得

	// 攻撃で使うタグ
	void SetAttackFlag(bool at) { mbAttack = at; } // 攻撃設定
	bool IsAttackFlag() { return mbAttack; }       // 攻撃取得


protected:
	VECTOR mvPosition; // 座標
	VECTOR mvRotation; // 回転 3Dでは必要


private:
	bool mbDeleteFlag; // 削除フラグ
	Tag3D mnTag;       // オブジェクトを見分ける用のタグ


private:
	bool mbAttack; // 攻撃のフラグ
	bool mb_EAttack; // 攻撃のフラグ


	// 追加
	float mfCurrentCameraDistance;     // 現在のカメラとの距離

	Model* mpModel;    // モデルクラスのポインタ
};