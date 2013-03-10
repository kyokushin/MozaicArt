#include <string>
#include <vector>

#include <opencv/cv.h>

namespace ys {

	bool isImageFile( const std::string& fname );
	
	/** ディレクトリを勝手に作って画像を保存してくれる
	 */
	bool imwrite( const std::string& fname, const cv::Mat& src, std::vector<int>& params );
	
	/** ディレクトリ選択ダイアログが表示される。
	 *  初期フォルダが指定できる機能を追加。
	 *  CHEN 20121206
	 */
	std::string selectDirectoryDialog(const std::string& path);

	/** ディレクトリ選択ダイアログ相関。
	 *  初期フォルダが指定できるには、コールバック関数を追加必要がある。
	 *  CHEN 20121206
	 */
	int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData);
};