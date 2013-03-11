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
	,_show_counter(0)
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
	,_show_counter(0)
{
	_init();
}

void ys::MozaicArt::_init()
{
	int patch_counter = 0;
	cv::Rect patch_roi(cv::Point(0,0), _patch_size);

	int new_width = _patch_size.width * _split_size.width;
	int new_height = _patch_size.height * _split_size.height;
	int start_x = (_orig_image.cols - new_width )/2;
	int start_y = (_orig_image.rows - new_height )/2;
	cv::Rect clip_orig_rect( start_x, start_y, new_width, new_height);
	_orig_image = cv::Mat( _orig_image, clip_orig_rect).clone();

	for( int i=0; i<_split_size.height; i++ ){
		patch_roi.y = i * _patch_size.height;

		for( int h=0; h<_split_size.width; h++ ){
			patch_roi.x = h * _patch_size.width;
			_patches[patch_counter].orig_patch = cv::Mat(_orig_image, patch_roi).clone();
			patch_counter++;
		}
	}
	_progress_image = _orig_image.clone();
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
			if( 0x1b == cv::waitKey()){
				goto end_loop;
			}

		}
	}
	end_loop:

	cv::destroyWindow( orig_wname );
	cv::destroyWindow( patch_wname );
}

bool ys::MozaicArt::calcPatchPosition( const cv::Mat& src, cv::Point& position )
{
	cerr<< "+++++ calc patch osition +++++" <<endl;

	if( _total_calc_times >= _split_size.area() ) return false;

	cv::Mat resize_src;
	cv::resize( src, resize_src, _patch_size );

	long long int min_val = LLONG_MAX;
	int min_i = -1;

	for( int i=0; i<_patches.size(); i++ ){

		if( _patches[i].calculated > 0 ) continue;

		long long int tmp_val = diffVal( resize_src, _patches[i].orig_patch );

		if( min_val > tmp_val ){
			position.x = i % _split_size.width;
			position.y = i / _split_size.width;
			min_i = i;
			min_val = tmp_val;
		}

	}

	if( min_i == -1 ) return false;
	
	MozaicPatch& patch = _patches[min_i];
	patch.calculated = _total_calc_times;
	patch.apply_patch = resize_src;
	patch.calc_val = min_val;
	cout<< "min_val:" << min_val <<endl;

	_total_calc_times++;

	cerr<< "====== calc patch osition =====" <<endl;
	return true;

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

#define DEBUG_getProgressImage
const cv::Mat& ys::MozaicArt::getProgressImage()
{

	int patch_counter = 0;

	if( _total_calc_times < _show_counter )
		return _progress_image;
	
	for( int i=0; i<_split_size.height; i++ ){

		for( int h=0; h<_split_size.width; h++ ){
#ifndef DEBUG_getProgressImage
			cout<< "[" << h << "," << i << "]" <<flush;
#endif
			const MozaicPatch &patch_param = _patches[patch_counter++];

			if( patch_param.calculated >= _show_counter){
#ifndef DEBUG_getProgressImage
				cout<< " calculated" <<flush;
#endif
				cv::Rect patch_rect(_patch_size.width*h,_patch_size.height*i,
								_patch_size.width,_patch_size.height);
#ifndef DEBUG_getProgressImage
				cout<< ":" << patch_rect <<flush;
#endif
				cv::Mat prog_patch(_progress_image, patch_rect);
				cv::imshow( "apply patch", patch_param.apply_patch );
				cv::waitKey(10);
				patch_param.apply_patch.copyTo( prog_patch );
			}
#ifndef DEBUG_getProgressImage
			cout<<endl;
#endif
		}
	}

	_show_counter ++;

	return _progress_image;

}
