#pragma once

#include "DxLib.h"
#include <string>
#include <map>


// 前方宣言
class AttachmentModel;

class Model
{
public:
	//Model(std::string filename, VECTOR initPos);  // コンストラクタ

	// ★New★
	// コンストラクタ
	// note: 分割アニメーションを使うかどうかの設定を追加。
	// コンストラクタ仲で分岐しているからひとつで良い
	Model(std::string filename, VECTOR initPos);


	// すでにロード済みのハンドルをそのまま使う時に使う
	Model(int modelHandle, VECTOR initPos);


	// 読み込むモデルのファイル名 , 位置
	~Model(); // デストラクタ
	void Update(); // 更新
	void Draw();   // 描画


	// -------------アタッチ関係---------------------------
	// アタッチモデル関連
	// アタッチメントを追加
	void AddAttachment(std::string filename, std::string attachFrameName);
	// アタッチモデルの座標取得　こっちは敵で使ってる
	VECTOR GetAttachmentPosition();


	// -------------ポジションto回転関係---------------------------
	VECTOR GetPosition() { return mvPosition; }        // 座標取得
	void SetPosition(VECTOR pos) { mvPosition = pos; } // 座標設定
	VECTOR GetRotation() { return mvRotation; }        // 回転取得
	void SetRotation(VECTOR rot) { mvRotation = rot; } // 回転設定

	// -------------SkyBox用---------------------------
	void SetScale(float scale);
	void SetTexture(std::string filename, int index = 0);

	// メインモデルのハンドルを返す
	int GetHandle() const { return mnHandle; }


	// ★New★
	// 初期行列の設定
	void SetupInitializeMatrix(std::string rootFrameName);

	// ★New★
	// 有効なフレームかどうかの判定
	bool ValidRootFrameIndex();

private:

	int mnHandle;      // 読み込んだモデルのハンドル
	VECTOR mvPosition; // 座標
	VECTOR mvRotation; // 回転
	// sukai
	float mfScale;
	int mnChangeTextureHandle;
	AttachmentModel* mpAttachment; // アタッチモデルクラスのポインタ
	// (複数持たせたいときは std::vector や配列で管理すると良い)


	// あたらいいやつ
	MATRIX mmInitializeMatrix;  // ★New★ // モデル内部で移動をしている（であろう）フレームの初期行列
	int mnRootFrameIndex;       // ★New★ // モデル内部で移動をしている（であろう）フレームのインデックス（初期値 -1）


	// これが true の時はデストラクタで MV1DeleteModel を呼ばない
	bool mbIsSharedHandle = false;
};

// メモ
// std::map...IDと値をセットで保存できるもの
// 例えば数字とポインタとか、文字列とかもできるから神

