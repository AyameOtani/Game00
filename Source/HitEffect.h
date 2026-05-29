#pragma once

#include "Object3D.h"

class HitEffect : public Object3D
{
public:

	HitEffect(VECTOR pos);

	~HitEffect();

	void Update() override;
	void Draw() override;

private:

	// 全インスタンスで共有してロード回数を抑える
	static int ms_handle;

	// 生存時間管理用
	int m_timer;

	// 拡大演出用
	float m_scale;

	// フェードアウト用
	int m_alpha;
};