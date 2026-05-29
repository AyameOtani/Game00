#include "DxLib.h"
#include "ResourceManager.h"

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
    // メモリリーク防止のため、キャッシュしていた全てのモデルデータを解放
    for (int i = 0; i < resourceMapList.size(); i++)
    {
        MV1DeleteModel(resourceMapList.at(i).second);
    }

    // DxLibの全リソースを初期化し、メモリをクリーンにする
    MV1InitModel();
    InitGraph();
}

// モデルリソース生成
int ResourceManager::LoadModel(std::string pathName)
{
    // 既に読み込まれたモデルか確認（キャッシュ利用によるメモリ節約）
    for (int i = 0; i < resourceMapList.size(); i++)
    {
        if (resourceMapList.at(i).first == pathName)
        {
            // 複製して返すことで、モデル本体はResourceManagerで保持しつつ
            // 個別の描画・操作が可能になる
            return MV1DuplicateModel(resourceMapList.at(i).second);
        }
    }

    // 未読み込みの場合は新規ロード
    int handle = MV1LoadModel(pathName.c_str());
    if (handle == -1)
    {
        return -1;
    }

    // オリジナルのハンドルをキャッシュとして保存
    resourceMapList.push_back(std::pair<std::string, int>(pathName, handle));

    // 呼び出し元には複製したハンドルを渡す
    return MV1DuplicateModel(handle);
}

// グラフィックリソース生成
int ResourceManager::LoadGraphics(std::string pathName)
{
    // 同一画像は複数回読み込まず、既存のハンドルを再利用
    for (int i = 0; i < graphicResourceMapList.size(); i++)
    {
        if (graphicResourceMapList.at(i).first == pathName)
        {
            return graphicResourceMapList.at(i).second;
        }
    }

    int handle = LoadGraph(pathName.c_str());
    if (handle == -1)
    {
        return -1;
    }

    graphicResourceMapList.push_back(std::pair<std::string, int>(pathName, handle));
    return handle;
}

// 分割グラフィックリソース生成
DivGraphData* ResourceManager::LoadDivGraphics(std::string pathName, int allNum, int numX, int numY)
{
    // キャッシュ済みの分割データを確認
    for (int i = 0; i < divGraphicResourceMapList.size(); i++)
    {
        if (divGraphicResourceMapList.at(i)->filePath == pathName)
        {
            return divGraphicResourceMapList.at(i);
        }
    }

    // サイズ取得用に一旦ロード
    int handle = LoadGraph(pathName.c_str());
    if (handle == -1)
    {
        return nullptr;
    }

    // 管理用データクラスの作成
    DivGraphData* data = new DivGraphData(pathName, numX, numY, allNum);

    // 画像サイズから分割サイズを算出
    int sizeX, sizeY;
    GetGraphSize(handle, &sizeX, &sizeY);

    // 分割読み込み実行
    int result = LoadDivGraph(pathName.c_str(), allNum, numX, numY, sizeX / numX, sizeY / numY, data->divHandleList);
    if (result == -1)
    {
        delete data; // 失敗時はメモリ解放
        return nullptr;
    }

    // キャッシュに保存
    divGraphicResourceMapList.push_back(data);

    return data;
}