#pragma once

#include "DxLib.h"
#include "Object3D.h"

class AttachmentModel : public Object3D
{
public:
	// モデルのパス
	// parentModelHandle くっつけたいモデル（親モデル）のハンドル
	// parentFrameIndex  くっつけたいモデル（親モデル）のフレーム番号
	AttachmentModel(std::string filename, int parentModelHandle, int parentFrameIndex);
	~AttachmentModel();

	//void Update() override;
	void Draw() override;
	void LateUpdate() override;

	// モデルハンドルの取得
	int GetHandle() { return mnHandle; }

	// ワールド座標の取得 攻撃判定
	VECTOR GetWorldPosition() const;


	// 持っている武器のframeをとりたい 0130追加
	// 座標取得もconstついてたから一応つける　替えたくないからなかにもつける
	// コピーしたくないから＆
	VECTOR GetFrame(const std::string& frameName) const;


	// 回転を設定する関数
	void SetWeaponRotation(VECTOR rot) { mvRotation = rot; }


private:
	int  mnHandle;          // モデルハンドル
	int mnParentHandle;     // 親モデルのハンドル
	int mnParentFrameIndex; // 親モデルのフレーム番号

	// 新年に追加
	bool mVisible = true;   // 表示フラグ

	VECTOR mvRotation; // 回転変数
};
