#include "MozaicArt.h"

#include <string>

#include <opencv/highgui.h>

using namespace std;


ys::MozaicArt::MozaicArt( const cv::Mat& orig_image, const cv::Size& split_size, int mozaic_type )
	:_orig_image(orig_image.clone())
	,_split_size(split_size)
	,_proc_mozaic_type_id(mozaic_type)
	,_patches(split_size.area())
	,_patch_size(orig_image.cols/split_size.width,orig_image.rows/split_size.height)
	,_total_calc_times(0)
{
	_init();
}

ys::MozaicArt::MozaicArt( const cv::Mat& orig_image, const int split_num, int mozaic_type)
	:_orig_image(orig_image.clone())
	,_split_size(split_num,split_num)
	,_proc_mozaic_type_id(mozaic_type)
	,_patches(split_num*split_num)
	,_patch_size(orig_image.cols/split_num,orig_image.rows/split_num)
	,_total_calc_times(0)
{
	_init();
}

void ys::MozaicArt::_init()
{
	int patch_counter = 0;
	cv::Rect patch_roi(cv::Point(0,0), _patch_size);
	for( int i=0; i<_split_size.height; i++ ){
		patch_roi.y = i * _patch_size.height;

		for( int h=0; h<_split_size.width; h++ ){
			patch_roi.x = h * _patch_size.width;
			_patches[patch_counter].orig_patch = cv::Mat(_orig_image, patch_roi);
			patch_counter++;
		}
	}
}

void ys::MozaicArt::debug_showAllPatches()
{
	static const string patch_wname("patch");
	static const string orig_wname("original");
	static const cv::Scalar rect_color(0,0,255);

	cv::Mat orig_show_img;
	cv::Rect patch_rect(cv::Point(0,0), _patch_size);

	for( int i=0; i<_split_size.height; i++ ){
		patch_rect.y = i*_patch_size.height;

		for( int h=0; h<_split_size.width; h++ ){
			patch_rect.x = h*_patch_size.width;

			_orig_image.copyTo(orig_show_img);
			cv::rectangle(orig_show_img, patch_rect, rect_color );

			cv::imshow( orig_wname, orig_show_img );
			cv::imshow( patch_wname, _patches[i*_split_size.width+h].orig_patch);
			cv::waitKey();

		}
	}

	cv::destroyWindow( orig_wname );
	cv::destroyWindow( patch_wname );
}

cv::Point ys::MozaicArt::calcPatchPosition( const cv::Mat& src )
{
	cv::Mat resize_src;
	cv::resize( src, resize_src, _patch_size );

	long long int min_val = LLONG_MAX;
	cv::Point min_pos;
	int min_i;

	for( int i=0; i<_patches.size(); i++ ){

		if( _patches[i].calculated > 0 ) continue;

		long long int tmp_val = diffVal( resize_src, _patches[i].orig_patch );

		if( min_val > tmp_val ){
			min_pos.x = i % _split_size.width;
			min_pos.y = i / _split_size.width;
			min_i = i;
			min_val = tmp_val;
		}

	}
	
	MozaicPatch& patch = _patches[min_i];
	patch.calculated = _total_calc_times;
	patch.apply_patch = resize_src;
	patch.calc_val = min_val;

	_total_calc_times++;


	return min_pos;

}

long long int ys::MozaicArt::diffVal( const cv::Mat& src, const cv::Mat& patch )
{
	assert( src.cols == patch.cols );
	assert( src.rows == patch.rows );
	assert( src.channels() == patch.channels() );

	const int width = src.cols;
	const int height = src.rows;
	const int ch = src.channels();

	long long int val = 0;

	for( int i=0; i<height; i++ ){
		const uchar* src_line = src.ptr(i);
		const uchar* patch_line = patch.ptr(i);

		for( int h=0; h<width; h++ ){
			const uchar* src_pix = src_line + ch*h;
			const uchar* patch_pix = patch_line + ch*h;

			for( int g=0; g<ch; g++ ){
				val += abs(src_pix[g] - patch_pix[g]);

			}

		}
	}


	return val;
}