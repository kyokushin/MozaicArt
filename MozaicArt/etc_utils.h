#include <string>
#include <vector>

#include <opencv/cv.h>

namespace ys {

	bool isImageFile( const std::string& fname );
	
	bool fileSelectDialog( std::string& fname, const std::string& def_path="" );
	bool directorySelectDialog( std::string& dirname, const std::string& def_pat="");
	bool colorPicker( cv::Scalar& color );
};
