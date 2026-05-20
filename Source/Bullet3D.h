#pragma once
#include "Object3D.h"
#include <string>

class Model;
class Enemy3D;
class Player3D;
class Unit;

class Bullet3D : public Object3D
{
public:
	// 敵の情報も受取るため追加
	Bullet3D(
		VECTOR initPos,             // 位置
		std::string filename,       // モデル名前　弾のやつ
		VECTOR Direction            // 角度　ホーミングのため
	);


	~Bullet3D();

	void Update();
	void Draw();
	void Move();

	void HitStage(); // 壁とのあたり判定

	void HitEnemy(); // 敵とのあたり判定
	void HitPlayer();// プレイヤーとのあたり判定


private:

	VECTOR mvDirection; // 向き
	// 現在の目標角度
	float mfAngle = 0.0f;
	float mfMoveSpeed = 0.0f; // 動いた量で消すため
	float mfSpeed = 40.0f; // 速さ

	Model* mpModel;
};