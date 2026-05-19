/*
キー入力に関する補助関数
DXLibに存在しないので自作
*/

class InputManager
{
public:
	// コンストラクタ
	InputManager();
	// デストラクタ
	~InputManager();

	// 一回しか反応しないのをやってくれている処理
	// 指定されたキーが押された瞬間だけ 1 を返す関数
	static int CheckDownKey(int KeyCode);

	// 指定されたキーが離された瞬間だけ 1 を返す関数
	static int CheckUpKey(int KeyCode);

	// 指定されたキーを押し続けている間１を返す関数
	static int CheckPressKey(int KeyCode);



	// --- ゲームパッド ---
	//// 指定されたキーが押された瞬間だけ 1 を返す関数
	//static int CheckDownPad(int Button);
	//// 指定されたキーが離された瞬間だけ 1 を返す関数
	//static int CheckUpPad(int Button);
	//// 指定されたキーを押し続けている間１を返す関数
	//static int CheckPressPad(int Button); // これが CheckHitKeyみたいなもの

	static bool CheckDownPad(int button);
	static bool CheckPressPad(int button);
	static bool CheckUpPad(int button);

	//// スティックの入力値を取得する関数（-1.0f ～ 1.0f）
	//static VECTOR GetLeftStick();
	//static VECTOR GetRightStick();



private:
	static int mDownBuffer[256];	// CheckDownKey用のキーバッファ
	static int mUpBuffer[256];		// CheckUpKey用のキーバッファ


	static int mPadDownBuffer; // ゲームパッドのCheckDownKey用のキーバッファ
	static int mPadUpBuffer;   // ゲームパッドのCheckUpKey用のキーバッファ

};
