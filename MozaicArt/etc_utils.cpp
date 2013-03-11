#include "etc_utils.h"

#include <algorithm>
#include <string>
#include <cctype>
#include <cstdlib>

#include <qfiledialog.h>
#include <qcolordialog.h>


#include <opencv/highgui.h>


using namespace std;

bool ys::isImageFile( const std::string& fname ){

	string::size_type pos = fname.find_last_of(".");
	if( pos == string::npos ){
		return false;
	}

	string ext = fname.substr(pos);
	std::transform( ext.begin(), ext.end(), ext.begin(), (int(*)(int))tolower );
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

bool ys::fileSelectDialog( std::string& fname, const std::string& def_path ){
	QString qfname = QFileDialog::getOpenFileName(
				0, QString("select open file"),
				QString(def_path.c_str()),
				//QString("Images (*.png *.xpm *.jpg)")
				QString()
				);
	fname = qfname.toUtf8().constData();
	if( fname == "" ) return false;
	return true;
}

bool ys::directorySelectDialog( std::string& fname, const std::string& def_path  ){
		QString qfname = QFileDialog::getExistingDirectory(
						0, QString("select open file"),
						QString(def_path.c_str())
					);
	fname = qfname.toUtf8().constData();
	if( fname == "" ) return false;
	return true;
}

bool ys::colorPicker( cv::Scalar& color ){
	QColorDialog color_dialog;
	color_dialog.open();
	const QColor qcolor = color_dialog.selectedColor();
	color[0] = qcolor.blue();
	color[1] = qcolor.green();
	color[2] = qcolor.red();
}
