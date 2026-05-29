#include "Stage.h"
#include "Model.h"
#include "Master.h"
#include "DxLib.h"

// コンストラクタ（モデルファイル名を指定して生成する場合）
Stage::Stage(std::string stageModelName, std::string stageCollisionModelName, VECTOR iniPos, StageType type)
	: Object3D(iniPos)
	, m_iniPos(iniPos)
	, mnModelHandle(MV1LoadModel(stageModelName.c_str()))
	, mnCollisionHandle(MV1LoadModel(stageCollisionModelName.c_str()))
	, m_type(type)
	, m_ownsModel(true)
	, m_ownsCollision(true)
	, mfMoveTime(0.0f)
	, mfRotation(0.0f)
	, mbRota(false)
	, m_originPos(VGet(0.0f, 0.0f, 0.0f))
	, m_rotatePivot(iniPos) // デフォルト値
	, m_rotateSpeed(1.1f)
	, m_rotateRange(0.15f)
{
	SetTag(Object3D::T_Stage3D);

	// 当たり判定情報の構築
	if (mnCollisionHandle != -1)
	{
		MV1SetupCollInfo(mnCollisionHandle, -1);
	}

	m_prevPosition = VGet(0, 0, 0);
	m_prevRotation = VGet(0, 0, 0);
	m_posDelta = VGet(0, 0, 0);
	m_rotDelta = VGet(0, 0, 0);

	mvPosition = iniPos;
	m_originPos = iniPos;   // 回転用の基準
}

// コンストラクタ（外部からハンドルを受け取って生成する場合）
Stage::Stage(int modelHandle, int collisionHandle, VECTOR iniPos, StageType type)
	: Object3D(iniPos)
	, m_iniPos(iniPos)
	, mnModelHandle(modelHandle)
	, mnCollisionHandle(collisionHandle)
	, m_type(type)
	, m_ownsModel(false)       // 外部管理のため削除しない
	, m_ownsCollision(false)   // 外部管理のため削除しない
	, mfMoveTime(0.0f)
	, mfRotation(0.0f)
	, mbRota(false)
	, m_originPos(VGet(0.0f, 0.0f, 0.0f))
	, m_rotatePivot(iniPos) // デフォルト値
	, m_rotateSpeed(1.1f)
	, m_rotateRange(0.15f)
	, m_prevPosition(VGet(0.0f, 0.0f, 0.0f))
	, m_prevRotation(VGet(0.0f, 0.0f, 0.0f))
	, m_posDelta(VGet(0.0f, 0.0f, 0.0f))
	, m_rotDelta(VGet(0.0f, 0.0f, 0.0f))
{


	SetTag(Object3D::T_Stage3D);

	if (mnCollisionHandle != -1)
	{
		MV1SetupCollInfo(mnCollisionHandle, -1);
	}

	mvPosition = iniPos;   // ★これ絶対入れる
	m_originPos = iniPos;   // ★回転用の基準もここ
}

// デストラクタ
Stage::~Stage()
{
	// 所有権があるハンドルのみ削除する
	if (m_ownsModel && mnModelHandle != -1)
	{
		MV1DeleteModel(mnModelHandle);
	}
	if (m_ownsCollision && mnCollisionHandle != -1)
	{
		MV1DeleteModel(mnCollisionHandle);
	}
}

// 毎フレーム更新
void Stage::Update()
{
	TitleRotate();

	// 時間を進めてアニメーション系の基準にする
	// 固定増加なのでフレーム依存だが軽量処理前提
	mfMoveTime += 0.016f;

	switch (m_type)
	{
	case StageType::Static:
		// 動かないステージは処理なし
		break;

	case StageType::MoveSide:
		// サイン波で左右移動（周期運動）
		mvPosition.x = sinf(mfMoveTime) * 300.0f;
		break;

	case StageType::MoveUpDown:
		// 上下移動は少し速くしてテンポを変える
		mvPosition.y = sinf(mfMoveTime * 1.5f) * 200.0f;
		break;

	case StageType::Rotate:
	{
		// 回転量はsinで往復させて自然な揺れを作る
		float angle = sinf(mfMoveTime * m_rotateSpeed) * m_rotateRange;
		float s = sinf(angle);
		float c = cosf(angle);

		// 軸回転は「基準点からの相対位置」を回す必要がある
		// これをやらないと位置ごと回転してしまう
		if (m_rotateAxis == RotateAxis::Z)
		{
			float dx = m_iniPos.x - m_rotatePivot.x;
			float dy = m_iniPos.y - m_rotatePivot.y;

			mvPosition.x = m_rotatePivot.x + dx * c - dy * s;
			mvPosition.y = m_rotatePivot.y + dx * s + dy * c;
			mvRotation.z = angle;
		}
		else
		{
			float dy = m_iniPos.y - m_rotatePivot.y;
			float dz = m_iniPos.z - m_rotatePivot.z;

			mvPosition.y = m_rotatePivot.y + dy * c - dz * s;
			mvPosition.z = m_rotatePivot.z + dy * s + dz * c;
			mvRotation.x = angle;
		}
	}
	break;

	default:
		// 想定外値でも落ちないよう安全側に戻す
		m_type = StageType::Static;
		break;
	}

	// モデルとコリジョンを同じ位置に同期
	// ずれていると見た目と判定が乖離するため必須
	if (mnModelHandle != -1)
	{
		MV1SetPosition(mnModelHandle, mvPosition);
		MV1SetRotationXYZ(mnModelHandle, mvRotation);
	}
	if (mnCollisionHandle != -1)
	{
		MV1SetPosition(mnCollisionHandle, mvPosition);
		MV1SetRotationXYZ(mnCollisionHandle, mvRotation);

		// 回転・移動後はコリジョンの内部BVH更新が必要
		MV1RefreshCollInfo(mnCollisionHandle);
	}

	// 差分を保持（プレイヤーや敵をステージと一緒に動かす用途）
	m_posDelta = VSub(mvPosition, m_prevPosition);
	m_rotDelta = VSub(mvRotation, m_prevRotation);

	m_prevPosition = mvPosition;
	m_prevRotation = mvRotation;
}

// 描画処理
void Stage::Draw()
{
	if (mnModelHandle != -1)
	{
		MV1DrawModel(mnModelHandle);
	}
}

// スケール（拡大率）の設定
void Stage::SetScale(float scale)
{
	if (mnModelHandle != -1)
	{
		MV1SetScale(mnModelHandle, VGet(scale, scale, scale));
	}
	if (mnCollisionHandle != -1)
	{
		MV1SetScale(mnCollisionHandle, VGet(scale, scale, scale));
		// スケール変更時は当たり判定情報を再構築する必要がある
		MV1SetupCollInfo(mnCollisionHandle, -1);
	}
}

// カプセル当たり判定
bool Stage::CheckHit_Capsule(VECTOR pos1, VECTOR pos2, float r)
{
	if (mnCollisionHandle == -1) return false;

	MV1_COLL_RESULT_POLY_DIM result = MV1CollCheck_Capsule(mnCollisionHandle, -1, pos1, pos2, r);
	bool hit = result.HitNum >= 1;

	// デバッグ用描画（ヒットした三角形を表示）
	if (hit)
	{
		for (int i = 0; i < result.HitNum; ++i)
		{
			//DrawTriangle3D(result.Dim[i].Position[0], result.Dim[i].Position[1], result.Dim[i].Position[2], GetColor(255, 0, 0), 0);
		}
	}
	MV1CollResultPolyDimTerminate(result);
	return hit;
}

// 線分当たり判定
VECTOR Stage::CheckHit_Line(VECTOR pos1, VECTOR pos2)
{
	VECTOR ret = VGet(0.0f, 0.0f, 0.0f);
	if (mnCollisionHandle == -1) return ret;

	auto result = MV1CollCheck_Line(mnCollisionHandle, -1, pos1, pos2);
	if (result.HitFlag >= 1) ret = result.HitPosition;

	return ret;
}

// 線分の法線付き当たり判定
bool Stage::CheckHit_Line_Normal(VECTOR pos1, VECTOR pos2, VECTOR& hitNormal)
{
	if (mnCollisionHandle == -1) return false;

	auto result = MV1CollCheck_Line(mnCollisionHandle, -1, pos1, pos2);
	if (result.HitFlag >= 1)
	{
		hitNormal = result.Normal;
		return true;
	}
	return false;
}

// 壁沿い移動用カプセル当たり判定
bool Stage::CheckHit_Capsule_Wall(VECTOR pos1, VECTOR pos2, float r, VECTOR& hitPos, VECTOR& hitNormal)
{
	if (mnCollisionHandle == -1) return false;

	MV1_COLL_RESULT_POLY_DIM result = MV1CollCheck_Capsule(mnCollisionHandle, -1, pos1, pos2, r);
	bool isHit = false;

	if (result.HitNum > 0)
	{
		VECTOR avgHitPos = VGet(0, 0, 0);
		VECTOR avgNormal = VGet(0, 0, 0);

		// ヒットした全ポリゴンの情報を平均化する
		for (int i = 0; i < result.HitNum; ++i)
		{
			avgHitPos = VAdd(avgHitPos, result.Dim[i].HitPosition);
			avgNormal = VAdd(avgNormal, result.Dim[i].Normal);
		}
		avgHitPos = VScale(avgHitPos, 1.0f / (float)result.HitNum);
		avgNormal = VNorm(avgNormal);

		hitPos = avgHitPos;
		hitNormal = avgNormal;
		isHit = true;
	}

	MV1CollResultPolyDimTerminate(result);
	return isHit;
}

void Stage::TitleRotate()
{
	// 必要に応じて実装してください
}

int Stage::GetModelHandle() const
{
	return mnCollisionHandle;
}

// Stage.cpp

void Stage::SetRotateParam(float speed, float range, RotateAxis axis)
{
	m_rotateSpeed = speed;
	m_rotateRange = range;
	m_rotateAxis = axis; // 軸を保存
}