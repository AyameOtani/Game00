#include "HitEffect.h"

#include "DxLib.h"

int HitEffect::ms_handle = -1;

HitEffect::HitEffect(VECTOR pos)
	: Object3D(pos)
	, m_timer(0)
	, m_scale(200.0f)
	, m_alpha(255)
{
	// 同じ画像を毎回ロードしない
	if (ms_handle == -1)
	{
		ms_handle = LoadGraph("Resource/2D/Effect.png");
	}
}

HitEffect::~HitEffect()
{
}

void HitEffect::Update()
{
	m_timer++;

	// 衝撃感を出すため徐々に拡大
	m_scale += 2.5f;

	// 消滅までを自然に見せる
	m_alpha -= 18;

	if (m_alpha < 0)
	{
		m_alpha = 0;
	}

	// 不要になったエフェクトを残さない
	if (m_timer > 15)
	{
		SetDeleteFlag(true);
	}
}

void HitEffect::Draw()
{
	// 発光感を出すため加算合成を使用
	SetDrawBlendMode(DX_BLENDMODE_ADD, m_alpha);

	DrawBillboard3D(
		mvPosition,
		0.5f,
		0.5f,
		m_scale,
		0.0f,
		ms_handle,
		TRUE
	);

	// 後続描画へ影響を残さない
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}