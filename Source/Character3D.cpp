#include "Character3D.h"

Character3D::Character3D(VECTOR initPos, int maxHp, Team team, float radius)
    : Object3D(initPos)
    , m_maxHp(maxHp)
    , m_hp(maxHp)
    , m_team(team)
    , m_radius(radius)
{
}

// デストラクタの実装
Character3D::~Character3D()
{

}

// ダメージ処理の実装
void Character3D::TakeDamage(int damage)
{
    m_hp -= damage;
    if (m_hp <= 0)
    {
		m_hp = 0; // HPが0以下にならないようにする

        // HPが0になったら削除フラグを立てる
        SetDeleteFlag(true);
    }
}