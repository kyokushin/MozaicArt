#include <string>
#include <iostream>

#include "opencv_windows_lib.h"
#include "etc_utils.h"

#include "ImageSequence.h"

#include "MozaicArt.h"

#include <Windows.h>

using namespace std;
using namespace cv;

int selctFileDialog( std::string& fname ){
	HWND hWnd = NULL;
	char path[MAX_PATH+1];
	char title[129];
	OPENFILENAME openfile;

	//構造体を０で埋める
	memset(&openfile, 0, sizeof(OPENFILENAME));
	//結果を格納するバッファの先頭をヌル文字にする（かなり重要）
	path[0] = '\0';

	//構造体のサイズの指定
	openfile.lStructSize = sizeof(OPENFILENAME);
	//親ウインドウのハンドル（NULL可）
	openfile.hwndOwner = hWnd;
	//フィルタの指定
	openfile.lpstrFilter = "画像(jpg,bmp,png)\0*.jpg;*.bmp;*.png\0全て(*.*)\0*.*\0\0";
	//結果が格納されるバッファの指定
	openfile.lpstrFile = path;
	//バッファのサイズ
	openfile.nMaxFile = MAX_PATH;
	//各種設定フラグ
	//今回は順に、「正しいパスのみ入力可」「正しいファイル名のみ入力可」「読み取り専用チェックを消す」
	openfile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	//標準の拡張子の指定
	openfile.lpstrDefExt = "exe";
	//選択されたファイル名が格納される
	openfile.lpstrFileTitle = title;
	//バッファのサイズを指定
	openfile.nMaxFileTitle = 128;
	//ダイアログのタイトル
	openfile.lpstrTitle = "プログラムの選択";

	//ダイアログを開く
	if(GetOpenFileName(&openfile)){
		//MessageBox(hWnd, (LPCTSTR)path,(LPCTSTR)"フルパス",MB_OK);
		//MessageBox(hWnd, (LPCTSTR)title,(LPCTSTR)"ファイル名",MB_OK);
		fname = path;
		return 1;
	}else{
		MessageBox(hWnd, (LPCTSTR)"キャンセルされました。",(LPCTSTR)"キャンセル",MB_OK);
		return 0;
	}
}


int main( int argc, char** argv )
{
	string fname;
	selctFileDialog( fname );
	string dirname = ys::selectDirectoryDialog("C:\\Users\\u-ta\\Pictures");

	ys::ImageList imglist;
	imglist.open(dirname);

	cout<< "Select Calculate type:" <<endl
		<< "\t1:Color" <<endl
		<< "\t2:Monochrome" <<endl
		<<endl;

	cv::Mat orig_image = cv::imread( fname );
	ys::MozaicArt mozaicart( orig_image, 10 );
	mozaicart.debug_showAllPatches();

}