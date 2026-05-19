#pragma once

#include "DxLib.h"
#include <string>
#include <vector>

// 分割画像のデータを保持する構造体
struct DivGraphData
{
	std::string filePath;
	int numX;
	int numY;
	int allNum;
	int* divHandleList; // 分割ハンドルの配列
	
	// コンストラクタ
	DivGraphData(std::string path, int nX, int nY, int aNum)
		: filePath(path), numX(nX), numY(nY), allNum(aNum)
	{
		divHandleList = new int[allNum]; // intの配列で領域確保
	}
	
	~DivGraphData()
	{
		delete[] divHandleList;
	}
};

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	// モデルリソース生成
	int LoadModel(std::string pathName);

	// グラフィックリソース生成
	int LoadGraphics(std::string pathName);

	// 分割されたグラフィックリソース生成
	DivGraphData* LoadDivGraphics(std::string pathName, int allNum, int numX, int numY);

private:
	// std::pair<パス, ハンドル> のリスト
	std::vector<std::pair<std::string, int>> resourceMapList;
	std::vector<std::pair<std::string, int>> graphicResourceMapList;
	std::vector<DivGraphData*> divGraphicResourceMapList;
};






