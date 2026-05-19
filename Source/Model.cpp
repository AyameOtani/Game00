#include "Model.h"
#include "AttachmentModel.h"
#include "Master.h"

// ハンドルを直接受け取るコンストラクタ
Model::Model(int modelHandle, VECTOR initPos)
	: mnHandle(modelHandle)
	, mvPosition(initPos)
	, mbIsSharedHandle(true)      // 共有ハンドルなので true
	, mpAttachment(nullptr)
	, mvRotation(VGet(0.0f, 0.0f, 0.0f))
	, mfScale(1.0f)
	, mnChangeTextureHandle(-1)
	, mnRootFrameIndex(-1)
	, mmInitializeMatrix(MGetIdent())
{
}

// コンストラクタ
Model::Model(std::string filename, VECTOR initPos)
	: mvPosition(initPos)
	, mpAttachment(nullptr)
	, mfScale(1.0f)
	, mvRotation(VGet(0.0f, 0.0f, 0.0f))
	, mnChangeTextureHandle(-1)
	, mnRootFrameIndex(-1)              // ★New★
	, mmInitializeMatrix(MGetIdent())   // ★New★
{
	// モデルの読み込み
	//mnHandle = MV1LoadModel(filename.c_str()); ★New★
	mnHandle = Master::mpResourceManager->LoadModel(filename.c_str()); // 変更した

}

// デストラクタ
Model::~Model()
{
	// アタッチモデルのクラス破棄
	if (mpAttachment != nullptr)
	{
		mpAttachment->SetDeleteFlag(true);
	}

	// テクスチャを切り替えている場合はそのテクスチャの破棄
	if (mnChangeTextureHandle != -1)
	{
		DeleteGraph(mnChangeTextureHandle);
	}

	// 共有ハンドルGameManagerで管理していない場合だけ削除する
	if (mbIsSharedHandle == false)
	{
		MV1DeleteModel(mnHandle);
	}

	// 読み込んだモデルの削除
	// note: 読み込んだモデルは勝手に破棄してくれないので、
	// 必要なくなったら手動で破棄する。
	// MV1DeleteModel(mnHandle);
}


// ★New★
// 初期行列の設定
void Model::SetupInitializeMatrix(std::string rootFrameName)
{
	// 指定されたフレーム名が存在するか探す
	mnRootFrameIndex = MV1SearchFrame(mnHandle, rootFrameName.c_str());

	// フレームが有効なインデックスだった場合
	if (ValidRootFrameIndex())
	{
		// MV1GetFrameLocalMatrix でも出来るが、
		// 用途的には MV1GetFrameBaseLocalMatrix が合ってそうなので、そっちを使ってみる
		//mmInitializeMatrix = MV1GetFrameLocalMatrix(mnHandle, mnRootFrameIndex);
		mmInitializeMatrix = MV1GetFrameBaseLocalMatrix(mnHandle, mnRootFrameIndex);
	}
}

// ★New★
// 有効なフレームかどうかの判定
bool Model::ValidRootFrameIndex()
{
	// MV1SearchFrame では -1 か -2 がエラーで帰ってくるので、その判定用
	return (mnRootFrameIndex != -1 && mnRootFrameIndex != -2);
}




// アタッチモデルの座標取得 敵で使ってるから消さない
VECTOR Model::GetAttachmentPosition()
{
	if (mpAttachment != nullptr)
	{
		VECTOR vec = VGet(0.0f, -50.0f, 0.0f);
		// 行列の取得  剣のモデルのマトリックスの取得
		MATRIX matrix = MV1GetFrameLocalWorldMatrix(mpAttachment->GetHandle(), 0);
		// 行列情報をもとに座標変換する
		vec = VTransform(vec, matrix);
		return vec;
	}
	// アタッチメントがない場合は原点を返しておく これはどっちにも必要
	return VGet(0.0f, 0.0f, 0.0f);
}

void Model::Update()
{
	// ★New★
	// 固定した行列を一旦解除する
	if (ValidRootFrameIndex())
	{
		MV1ResetFrameUserLocalMatrix(mnHandle, mnRootFrameIndex);
	}

	// ★New★
	// アニメーションで移動している成分だけを初期値に戻すことで、アニメーションでの移動を無効化しているようにみせる。
	// note: 実験的な実装なので、上手く行かないアニメーションもあるかも。
	if (ValidRootFrameIndex())
	{
		auto Matrix = MV1GetFrameLocalMatrix(mnHandle, mnRootFrameIndex);
		MATRIX result = Matrix;
		result.m[3][0] = mmInitializeMatrix.m[3][0];
		//result.m[3][1] = mmInitializeMatrix.m[3][1]; // Y成分だけは一旦反映しないようにしておく（反映してもよいが、見た目が少しおかしくなることが多い）
		result.m[3][2] = mmInitializeMatrix.m[3][2];
		MV1SetFrameUserLocalMatrix(mnHandle, mnRootFrameIndex, result);
	}


	// 座標設定
	MV1SetPosition(mnHandle, mvPosition);

	// 回転設定
	MV1SetRotationXYZ(mnHandle, mvRotation);
}

void Model::Draw()
{
	// モデルの描画
	MV1DrawModel(mnHandle);
}


// アタッチメントを追加
void Model::AddAttachment(std::string filename, std::string attachFrameName)
{
	// アタッチ先のフレーム番号を取得
	int frameIndex = MV1SearchFrame(mnHandle, attachFrameName.c_str());

	// アタッチメントモデルの作成
	// frameIndexは武器を持つところを探してくれたもの
	mpAttachment = new AttachmentModel(filename, mnHandle, frameIndex);
}


void Model::SetScale(float scale)
{
	MV1SetScale(mnHandle, VGet(scale, scale, scale));
}

void Model::SetTexture(std::string filename, int index)
{
	// テクスチャをすでに切り替えている場合はそのテクスチャの破棄
	if (mnChangeTextureHandle != -1)
	{
		DeleteGraph(mnChangeTextureHandle);
	}

	// テクスチャの読み込み
	//mnChangeTextureHandle = LoadGraph(filename.c_str());
	mnChangeTextureHandle = Master::mpResourceManager->LoadGraphics(filename);

	// 読み込んだテクスチャの反映
	MV1SetTextureGraphHandle(mnHandle, index, mnChangeTextureHandle, FALSE);
}