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

	//�\���̂��O�Ŗ��߂�
	memset(&openfile, 0, sizeof(OPENFILENAME));
	//���ʂ��i�[����o�b�t�@�̐擪���k�������ɂ���i���Ȃ�d�v�j
	path[0] = '\0';

	//�\���̂̃T�C�Y�̎w��
	openfile.lStructSize = sizeof(OPENFILENAME);
	//�e�E�C���h�E�̃n���h���iNULL�j
	openfile.hwndOwner = hWnd;
	//�t�B���^�̎w��
	openfile.lpstrFilter = "�摜(jpg,bmp,png)\0*.jpg;*.bmp;*.png\0�S��(*.*)\0*.*\0\0";
	//���ʂ��i�[�����o�b�t�@�̎w��
	openfile.lpstrFile = path;
	//�o�b�t�@�̃T�C�Y
	openfile.nMaxFile = MAX_PATH;
	//�e��ݒ�t���O
	//����͏��ɁA�u�������p�X�̂ݓ��͉v�u�������t�@�C�����̂ݓ��͉v�u�ǂݎ���p�`�F�b�N�������v
	openfile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	//�W���̊g���q�̎w��
	openfile.lpstrDefExt = "exe";
	//�I�����ꂽ�t�@�C�������i�[�����
	openfile.lpstrFileTitle = title;
	//�o�b�t�@�̃T�C�Y���w��
	openfile.nMaxFileTitle = 128;
	//�_�C�A���O�̃^�C�g��
	openfile.lpstrTitle = "�v���O�����̑I��";

	//�_�C�A���O���J��
	if(GetOpenFileName(&openfile)){
		//MessageBox(hWnd, (LPCTSTR)path,(LPCTSTR)"�t���p�X",MB_OK);
		//MessageBox(hWnd, (LPCTSTR)title,(LPCTSTR)"�t�@�C����",MB_OK);
		fname = path;
		return 1;
	}else{
		MessageBox(hWnd, (LPCTSTR)"�L�����Z������܂����B",(LPCTSTR)"�L�����Z��",MB_OK);
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