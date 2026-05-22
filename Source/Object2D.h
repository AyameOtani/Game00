#pragma once
#include "DxLib.h"
#include <string>

// クラスの前方宣言
class Texture;
class TextureAnimation;

class Object2D
{
public:    // enum, struct, 定数の定義

	// オブジェクトを見分けるためのタグ
	// Object2D だけだと何か分からないあの出見分けるためのタグ
	enum Tag
	{
		Player2D = 2000,
		Enemy2D = 2001,
		Boss2D = 2002,
		Map2D = 2100,

	};


public:
	// コンストラクタ（一枚絵用）
	Object2D(std::string filename, VECTOR initPos);

	// コンストラクタ（アニメーション用）
	Object2D(VECTOR initPos, std::string filename, int allNum, int numX, int numY, int interval);


	virtual ~Object2D(); // デストラクタ

	virtual void Update(); //アップデート

	virtual void Draw(); // 描画

public:   // ゲッター・セッター
	void SetPosition(VECTOR pos) { mvPosition = pos; }  // 座標設定
	VECTOR GetPosition() { return mvPosition; }         // 座標取得

	void SetDeleteFlag(bool flag) { mbDeleteFlag = flag; }  // 削除フラグ設定
	bool IsDeleteFlag() { return mbDeleteFlag; }           // 削除フラグ取得

	void SetTag(Tag tag) { mnTag = tag; }  // タグ設定
	Tag GetTag() { return mnTag; }         // タグ取得


	float GetRadius(); // 半径の取得

	// プレイヤーで使うため
	int GetSizeX();  // 幅
	int GetSizeY();  // 高さ


public: // ソート関数定義

	// Zオーダーの設定
	// 2Dの場合はよほどのことが無ければZ座標は使っていないはずなので、設定用に利用する。
	void SetZOrder(float z) { mvPosition.z = z; }

	// Zオーダーを基準にソートする
	// Zオーダーの数値が大きいほど手前に来る
	struct CompareZOrder {
		bool operator()(Object2D* a, Object2D* b) const
		{
			return a->GetPosition().z < b->GetPosition().z;
		}
	};



protected:
	//2Dの要素は何が必要か考えて書く
	Texture* mpTexture;   // 画像
	VECTOR mvPosition;    // 座標
	TextureAnimation* mpTextureAnimation; // アニメーション画像

private:
	bool mbDeleteFlag;    // 削除フラグ(これがtrue になっていると自動的に削除される（ように作る）)
	Tag mnTag;            // オブジェクトを見分ける用のタグ
};