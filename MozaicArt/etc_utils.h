#include <string>
#include <vector>

#include <opencv/cv.h>

namespace ys {

	bool isImageFile( const std::string& fname );
	
	/** �f�B���N�g��������ɍ���ĉ摜��ۑ����Ă����
	 */
	bool imwrite( const std::string& fname, const cv::Mat& src, std::vector<int>& params );
	
	/** �f�B���N�g���I���_�C�A���O���\�������B
	 *  �����t�H���_���w��ł���@�\��ǉ��B
	 *  CHEN 20121206
	 */
	std::string selectDirectoryDialog(const std::string& path);

	/** �f�B���N�g���I���_�C�A���O���ցB
	 *  �����t�H���_���w��ł���ɂ́A�R�[���o�b�N�֐���ǉ��K�v������B
	 *  CHEN 20121206
	 */
	int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData);
};