#pragma once
#include "Object3D.h"
#include "Team.h" // šæ‚Ù‚Çì‚Á‚½‹¤’Êƒwƒbƒ_[‚ğ“Ç‚İ‚Ş

class Character3D : public Object3D
{
public:
    // Hp‚Æƒ`[ƒ€‚Æ‚ ‚½‚è”»’è‚Ì”¼Œa
    Character3D(VECTOR initPos, int maxHp, Team team, float radius);
    virtual ~Character3D();

    virtual void TakeDamage(int damage);

    // HPŠÖŒW
    int GetHp() const { return m_hp; }
    void SetHp(int hp) { m_hp = hp; }

    //@ƒ`[ƒ€ŠÖŒW
    Team GetTeam() const { return m_team; }
    float GetRadius() const { return m_radius; }

protected:
    int m_maxHp;
    int m_hp;
    Team m_team;    // w‰c
    float m_radius; // “–‚½‚è”»’è‚Ì”¼Œa
};