#include "Stage.h"
#include "Model.h"
#include "Master.h"
#include "DxLib.h"

// コンストラクタ（モデルファイル名を指定して生成する場合）
Stage::Stage(std::string stageModelName, std::string stageCollisionModelName, StageType type)
	: Object3D(VGet(0.0f, 0.0f, 0.0f))
	, mnModelHandle(MV1LoadModel(stageModelName.c_str()))
	, mnCollisionHandle(MV1LoadModel(stageCollisionModelName.c_str()))
	, m_type(type)
	, m_ownsModel(true)
	, m_ownsCollision(true)
	, mfMoveTime(0.0f)
	, mfRotation(0.0f)
	, mbRota(false)
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
}

// コンストラクタ（外部からハンドルを受け取って生成する場合）
Stage::Stage(int modelHandle, int collisionHandle, StageType type)
	: Object3D(VGet(0.0f, 0.0f, 0.0f))
	, mnModelHandle(modelHandle)
	, mnCollisionHandle(collisionHandle)
	, m_type(type)
	, m_ownsModel(false)       // 外部管理のため削除しない
	, m_ownsCollision(false)   // 外部管理のため削除しない
	, mfMoveTime(0.0f)
	, mfRotation(0.0f)
	, mbRota(false)
{
	SetTag(Object3D::T_Stage3D);

	if (mnCollisionHandle != -1)
	{
		MV1SetupCollInfo(mnCollisionHandle, -1);
	}
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

// 更新処理
void Stage::Update()
{
	TitleRotate();

	mfMoveTime += 0.016f;

	// ステージのタイプに応じた挙動の切り替え
	switch (m_type)
	{
	case StageType::Static:
		// 静止状態：一切の移動・回転を行わない
		break;

	case StageType::Moving:
		// 上下運動：正弦波（sin）を用いてY軸方向に往復移動させる
		//mvPosition.x = sinf(mfMoveTime) * 300.0f;
		break;

	case StageType::Rotating:
		// 大きな揺れ：Z軸に対して比較的大きく左右に傾きを繰り返す
	{
		float tilt = sinf(mfMoveTime * 0.7f) * 0.8f;
		mvRotation.z = tilt;
	}
	break;

	case StageType::LittleRotation:
		// 小さな揺れ：Z軸に対して緩やかに小さく傾きを繰り返す
	{
		float little = sinf(mfMoveTime * 0.5f) * 0.4f;
		mvRotation.z = little;
	}
	break;

	default:
		// 未定義のタイプが指定された場合は静止状態にリセット
		m_type = StageType::Static;
		break;
	}

	// モデル・コリジョンの位置と回転を更新
	if (mnModelHandle != -1)
	{
		MV1SetPosition(mnModelHandle, mvPosition);
		MV1SetRotationXYZ(mnModelHandle, mvRotation);
	}
	if (mnCollisionHandle != -1)
	{
		MV1SetPosition(mnCollisionHandle, mvPosition);
		MV1SetRotationXYZ(mnCollisionHandle, mvRotation);
		// コリジョン情報を最新の位置・回転に同期
		MV1RefreshCollInfo(mnCollisionHandle);
	}

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
			DrawTriangle3D(result.Dim[i].Position[0], result.Dim[i].Position[1], result.Dim[i].Position[2], GetColor(255, 0, 0), 0);
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