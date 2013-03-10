#include "etc_utils.h"

#include <algorithm>

#include <opencv/highgui.h>

#include <Windows.h>
#include <ShlObj.h>
#include <ImageHlp.h>
#include <Shlwapi.h>
#pragma comment( lib, "imagehlp.lib")
#pragma comment( lib, "shlwapi.lib")

using namespace std;

bool ys::isImageFile( const std::string& fname ){

	string::size_type pos = fname.find_last_of(".");
	if( pos == string::npos ){
		return false;
	}

	string ext = fname.substr(pos);
	transform( ext.begin(), ext.end(), ext.begin(), tolower );
	if( ext == ".bmp" 
		|| ext == ".jpg" || ext == ".jpeg" || ext == ".jpe" 
		|| ext == ".jp2"
		|| ext == ".png"
		|| ext == ".pbm" || ext == ".pgm" || ext == ".ppm"
		|| ext == ".sr" || ext == ".ras"
		|| ext == ".tiff" || ext == ".tif" 
		){
			return true;
	}

	return false;
}

bool ys::imwrite( 
	const std::string& fname, const cv::Mat& src,
	std::vector<int>& params )
{

	string::size_type pos = fname.find_last_of("\\");
	string dirname = fname.substr(0,pos);

	if( !PathIsDirectory( dirname.c_str() ) ){
		BOOL success = MakeSureDirectoryPathExists( fname.c_str() );
	}

	return cv::imwrite( fname, src, params );
}

int CALLBACK ys::BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData)
{
	if(uMsg==BFFM_INITIALIZED)
	{
        SendMessage(hwnd,BFFM_SETSELECTION,(WPARAM)TRUE,lpData);
    }
    return 0;
}

//CHEN 20121206
std::string ys::selectDirectoryDialog(const std::string& path)
{
	BROWSEINFO bi;
	ITEMIDLIST *idl;
	LPMALLOC g_pMalloc;
	char szTmp[MAX_PATH];

	int size = path.size();
	for(int i = 0; i <= size ; i++)
    {
		szTmp[i] = path[i];
    }

	SHGetMalloc(&g_pMalloc);

	bi.hwndOwner = NULL;
	bi.pidlRoot  = NULL;
	bi.pszDisplayName =szTmp;
	bi.lpszTitle = TEXT("フォルダを選択してください");
	bi.ulFlags	 = BIF_RETURNONLYFSDIRS;
	bi.lpfn		 = &BrowseCallbackProc;
	bi.lParam	 = (LPARAM)szTmp;
	bi.iImage	 = 0;

	//ダイアログを表示
	idl=SHBrowseForFolder(&bi);
	if(idl != NULL)
	{
		if(SHGetPathFromIDList(idl,szTmp) != FALSE){
			MessageBox(NULL,szTmp,TEXT("tips5"),MB_OK);
		}
		//PIDLを解放する
		//g_pMalloc->lpVtbl->Free(g_pMalloc,idl); //for c
		g_pMalloc->Free(idl); //for c++
	}


	return szTmp;
}