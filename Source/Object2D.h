#pragma once
#include "DxLib.h"
#include <string>

// クラスの前方宣言
class Texture;
class TextureAnimation;

// 
// 2D オブジェクトの基底クラス
// 2D オブジェクト（プレイヤーや敵など）を作る際は、
// 必ずこれを継承して作成する　
// 
class Object2D
{
public:    // enum, struct, 定数の定義

	// オブジェクトを見分けるためのタグ
	// Object2D だけだと何か分からないあの出見分けるためのタグ
	enum Tag
	{
		// ---★★★---
		// タグの番号被ったらバグるから要注意 

		// Titleシーンでつかわれるタグ（1000～）

		// Gameシーンでつかわれるタグ（2000～）
		Player2D = 2000,
		Enemy2D = 2001,
		Boss2D = 2002,
		Map2D = 2100,

		Boon2D = 2150,

		// BossEnemy2D  = 2101,  // など、被らないようにに使う
		Bullet2D = 2200,
		HomingBullet2D = 2201,
		EnemyBullet2D = 2202,
		EnemyHomingBullet2D = 2203,
		BossBullet2D = 2204,
		ThreeBullet2D = 2205,
		OffsetBullet2D = 2206,

		// Resultシーンでつかわれるタグ（3000～）

	};


public:
	// コンストラクタ（一枚絵用）
	Object2D(std::string filename, VECTOR initPos);

	// コンストラクタ（アニメーション用）
	Object2D(VECTOR initPos, std::string filename, int allNum, int numX, int numY, int interval);


	virtual ~Object2D(); // デストラクタ

	virtual void Update(); //アップデート
	// ★★  上書きされるかも知れないからvirtualをつける

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


	// ★New!! Zソート用★
public: // ソート関数定義

	// Zオーダーの設定
	// 2Dの場合はよほどのことが無ければZ座標は使っていないはずなので、設定用に利用する。
	void SetZOrder(float z) { mvPosition.z = z; }

	// Zオーダーを基準にソートする
	// Zオーダーの数値が大きいほど手前に来る
	struct CompareZOrder {
		bool operator()(Object2D* a, Object2D* b) const {
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

/*
// 一枚画像用のクラス
class Tecture2D : public Object2D
{

};

// テクスチャアニメーション用のクラス
class TectureAnimation2D : public Object2D
{

};*/