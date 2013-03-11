#include <string>
#include <iostream>

#ifdef _WIN32
#include "opencv_windows_lib.h"
#else
#include <qapplication.h>
#include <qtextcodec.h>
#endif

#include "etc_utils.h"

#include "ImageSequence.h"

#include "MozaicArt.h"


using namespace std;
using namespace cv;



int main( int argc, char** argv )
{
#ifndef _WIN32
	QApplication qapp(argc, argv);
	QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
#endif


	string fname, dirname;
//#define DEBUG
#ifdef DEBUG	
	fname = "/home/kyokushin/画像/(成年コミック) [鰻丸] 良い子のご褒美/002.jpg";
	dirname = "/home/kyokushin/jpg_list.txt";
	bool selected = true;
#else
	bool selected = ys::fileSelectDialog( fname );
	cout<< "selected file >" << fname <<endl;
	if( !selected ) {
		cout<< "not select file " <<endl;
		return -1;
	}
	cout<< "1 : file select" <<endl
		<< "2 : directory select" <<endl
		<< ">" <<flush;
	int type;
	cin>> type;
	
	if( type == 1 )
		selected = ys::fileSelectDialog(dirname);
	else if( type == 2 )
		selected = ys::directorySelectDialog(dirname);
	else selected = false;
#endif
	cout<< "selected file or directory >" << dirname <<endl;
	if( !selected ){
		cout<< "not select directory." <<endl;
		return -1;
	}

	cout<< "read images" <<endl;
	ys::ImageList imglist;
	imglist.open(dirname);

	cout<< "Select Calculate type:" <<endl
		<< "\t1:Color" <<endl
		<< "\t2:Monochrome" <<endl
		<<endl;

	cv::Mat src_image;
	cv::Size reduction_size(1200,600);
	{
		cv::Mat orig_image = cv::imread( fname );
		cv::Size orig_size = orig_image.size();
		double width_rate = (double)reduction_size.width/orig_image.cols;
		double height_rate = (double)reduction_size.height/orig_image.rows;

		if( width_rate < height_rate ){
			reduction_size.height = orig_image.rows * width_rate;
		}
		else {
			reduction_size.width = orig_image.cols * height_rate;
		}
		cout<< "resize:" << reduction_size <<endl;
		cv::resize( orig_image, src_image, reduction_size );
	}

	cv::Size split_size = cv::Size(50,100);
	//cv::Size split_size = cv::Size(9,10);
	ys::MozaicArt mozaicart( src_image, split_size );
	mozaicart.debug_showAllPatches();

	
	cv::Mat read_image; 
	int counter = 0;
	while( imglist.next() ){
		imglist.get( read_image );	
		bool updated = mozaicart.calcPatchPosition( read_image );
		if( !updated ){
			cout<< "finish." <<endl;
			break;
		}

		const cv::Mat& progress_image = mozaicart.getProgressImage();

		cout<< ++counter << "/" << split_size.area() <<endl;
		cv::imshow( "progress image", progress_image );
		int ikey = cv::waitKey(30);

		if( ikey == 0x1b ) break;
	}
	
	cout<< "save result image" <<endl;
	cv::imwrite( "mozaic_result.png", mozaicart.getProgressImage());
	cv::waitKey();
}
