#pragma once
#include "Object3D.h"
#include "Team.h" // ★先ほど作った共通ヘッダーを読み込む

class Character3D : public Object3D
{
public:
    // Hpとチームとあたり判定の半径
    Character3D(VECTOR initPos, int maxHp, Team team, float radius);
    virtual ~Character3D();

    virtual void TakeDamage(int damage);

    // HP関係
    int GetHp() const { return m_hp; }
    void SetHp(int hp) { m_hp = hp; }

    //　チーム関係
    Team GetTeam() const { return m_team; }
   

	// 子クラスでオーバーライドして、あたり判定の中心を調整できるようにする
public: 
    // あたり判定について
    virtual VECTOR GetHitCenter() const
    {
        // デフォはそのまま
		return VAdd(mvPosition, VGet(0.0f, 0.0f, 0.0f)); // キャラクターの中心より少し上を当たり判定の中心にする
    }


    virtual VECTOR GetCapsuleTop() const
    {
        return VAdd(mvPosition, VGet(0.0f, 60.0f, 0.0f));
    }

    virtual VECTOR GetCapsuleBottom() const
    {
        return VAdd(mvPosition, VGet(0.0f, 0.0f, 0.0f));
    }

    virtual float GetRadius() const { return m_radius; }

protected:
    int m_maxHp;
    int m_hp;
    Team m_team;    // 陣営
    float m_radius; // 当たり判定の半径
};