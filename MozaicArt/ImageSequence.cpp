#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <ctime>

#include <opencv/cv.h>
#include <opencv/highgui.h>


#ifdef _WIN32
#include <windows.h>

#include <shlwapi.h>
#pragma comment( lib, "shlwapi.lib" )

#else
#include <qfile.h>
#include <qdir.h>

#endif

#include "ImageSequence.h"

using namespace std;
using namespace cv;

#ifndef Disable_FlyCapture
unsigned int ys::FlyCaptureCapture::connected_camera_num = 0;
unsigned int ys::FlyCaptureCapture::opened_camera_num = 0;
bool ys::FlyCaptureCapture::enable_synchronized_multicamera = false;
FlyCapture2::BusManager *ys::FlyCaptureCapture::busMgr;
FlyCapture2::Camera** ys::FlyCaptureCapture::multicamera;
unsigned int ys::FlyCaptureCapture::open_camera_num = 0;
int ys::FlyCaptureCapture::busMgr_ref_count = 0;
#endif



bool ys::VideoCapture::get( Mat &image, int channel ){
	return _cap.retrieve(image, channel);
}
bool ys::VideoCapture::next(){
	if( _cap.grab()){
		++_counter;
		return true;
	}

	return false;
}
int ys::VideoCapture::currentNum(){
	return _counter;
}


bool ys::VideoCapture::open(const string &filename ){
	_filename = filename;
	return _cap.open( filename );
}
bool ys::VideoCapture::isOpen(){
	return _cap.isOpened();
}
void ys::VideoCapture::release(){
	_cap.release();
	_counter = 0;
}

string ys::VideoCapture::getSourceName(){
	return "Video";
}

string ys::VideoCapture::getFileName(){
	return _filename;
}

bool ys::CameraCapture::open( const string &filename ){
	stringstream sstr(filename);
	int device;

	sstr>> device;

	return _cap.open(device);
}

string ys::CameraCapture::getSourceName(){
	return "Camera";
}

bool ys::ImageList::get( Mat &image, int channel ){

	if( _counter == _image_list.size() ){
		return false;
	}

	string fname = _image_list[_counter];
	inf.fname = fname.substr( fname.find_last_of("\\") + 1 );
	Mat img = imread( fname);
	inf.height = img.rows;
	inf.width = img.cols;

	if( img.data == NULL ){
		cerr<< "failed to read file > " << _image_list[_counter]  <<endl;
		return false;
	}

	image = img;
	return true;
}
bool ys::ImageList::next(){
	if( _counter + 1 >= _image_list.size() ){
		//cout<< "end image lists" <<endl;
		return false;
	}
	++_counter;
	return true;
	
}
int ys::ImageList::currentNum(){
	return _counter;
}

//リストファイル又はディレクトリのオープン
bool ys::ImageList::open(const string &filename){
	_filename = filename;

#ifdef _WIN32
	const int result = PathFileExists( filename.c_str());
#else
	const int result = QFile( filename.c_str() ).exists();
#endif
	if( !result ){
		cerr << "not exist file " <<endl;
		return false;
	}

	size_t read_num;
#ifdef _WIN32
	if( PathIsDirectory( filename.c_str() )){
#else
	if( QDir( filename.c_str() ).exists() ){
#endif
		read_num = _readDir2List( filename );
	}
	else {
		read_num = _readFile2List( filename );
	}

	return read_num > 0;
}

bool ys::ImageList::isOpen(){
	return _open;
}
void ys::ImageList::release(){
	_image_list.clear();
	_counter = 0;
	_open = false;
}

string ys::ImageList::getSourceName(){
	return "ImageList";
}

string ys::ImageList::getFileName(){
	return _filename;
}

size_t ys::ImageList::_readDir2List(const string &name ){

	string dirname = name;
	if( dirname[dirname.size()-1] != '\\'){
		dirname += "\\";
	}
	string sub = dirname.substr( 0, dirname.size() -1 );
	string::size_type pos = sub.find_last_of( "\\" );
	inf.serial_number = pos + 1;


#ifdef _WIN32
	WIN32_FIND_DATA fd;
	//ファイル名をすべて_image_listにpush_backする
	HANDLE hSearch = FindFirstFile( (dirname + "*.*").c_str(), &fd );
	if( hSearch == INVALID_HANDLE_VALUE ){
		cerr<< "failed to find files in " << name << "in" << __FILE__ << " at " << __LINE__ <<endl;
		return 0;
	}
	for(;;){
		string fname = fd.cFileName;
		if( fname != "." && fname != ".." ){
			_image_list.push_back( dirname + fname);
		}

		BOOL hasNextFile = FindNextFile(hSearch, &fd); 
		if( !hasNextFile ){
			if( GetLastError() == ERROR_NO_MORE_FILES ){
				break;
			}
			else {
				cerr<< "illegal exception in " << __FILE__ << " at " << __LINE__ <<endl;
				break;
			}
		}
	}

	FindClose( hSearch);
#else

	/*
	for( fs::directory_iterator itr(read_path); itr != end; ++itr ){
		if( fs::is_directory(*itr) ) continue;

		_image_list.push_back(itr->path().filename().string());
	}
	*/
	
	QDir qdir( dirname.c_str() );
	QStringList qfilters;
	qfilters<< "*.jpg" << "*.png" << "*.bmp";
	QStringList qflist =qdir.entryList(qfilters, QDir::Files|QDir::NoDotAndDotDot);

	for( int i=0; i<qflist.size(); i++ ){
		_image_list.push_back( qflist.at(i).toLocal8Bit().constData() );
	}

#endif

	sort( _image_list.begin(), _image_list.end() );

	return _image_list.size();
}


size_t ys::ImageList::_readFile2List( const string &filename ){

	stringstream sstr;
#ifdef _WIN32
	sstr<< filename.substr( filename.find_last_of("\\") );
#else
	sstr<< filename.substr( filename.find_last_of("/") );
#endif
	sstr>> inf.serial_number;

	ifstream ifs(filename.c_str());
	string line;
	while( ifs ){
		char fpath[PATH_MAX];
		ifs.getline(fpath, PATH_MAX);
		line = fpath;
		//cout<< line <<endl;

		//ファイルの存在確認
		//存在しなければcontinue
#ifdef _WIN32
		const BOOL result = PathFileExists( line.c_str() );
#else
		const bool result = QFile::exists(QObject::tr(line.c_str()));
#endif
		if( !result ){
			cerr<< "not exists \"" << line << "\". skip" <<endl;
			continue;
		}

		_image_list.push_back( line );
	}

	return _image_list.size();
}

void ys::ImageList::setMultiCameraNum( int num ){
	camera_num = num;
}

int ys::ImageList::getMultiCameraNum(){
	return camera_num;
}


//****************************************
//*** FlyCapture用のラッパークラス実装 ***
//****************************************

#ifndef Disable_FlyCapture
using namespace FlyCapture2;
//明るさ
const ys::FlyCaptureCapture::PropertyName ys::FlyCaptureCapture::prop_brightness("brightness_abs","brightness_auto","brightness_val");
//露出
const ys::FlyCaptureCapture::PropertyName ys::FlyCaptureCapture::prop_exposure("exposure_abs","exposure_auto","exposure_val");
//シャープネス
const ys::FlyCaptureCapture::PropertyName ys::FlyCaptureCapture::prop_sharpness("sharpness_abs","sharpness_auto","sharpness_val");
//色合い
const ys::FlyCaptureCapture::PropertyName ys::FlyCaptureCapture::prop_hue("hue_abs","hue_auto","hue_val");
//彩度
const ys::FlyCaptureCapture::PropertyName ys::FlyCaptureCapture::prop_saturation("saturation_abs","saturation_auto","saturation_val");
//ガンマ
const ys::FlyCaptureCapture::PropertyName ys::FlyCaptureCapture::prop_gamma("gamma_abs","gamma_auto","gamma_val");
const ys::FlyCaptureCapture::PropertyName ys::FlyCaptureCapture::prop_shutter("shutter_abs","shutter_auto","shutter_val");
const ys::FlyCaptureCapture::PropertyName ys::FlyCaptureCapture::prop_gain("gain_abs","gain_auto","gain_val");
const ys::FlyCaptureCapture::PropertyName ys::FlyCaptureCapture::prop_white("white_abs","white_auto","white_val", "white_red", "white_blue");

const std::string ys::FlyCaptureCapture::prop_parameter_on("on");
const std::string ys::FlyCaptureCapture::prop_parameter_off("off");


ys::FlyCaptureCapture::FlyCaptureCapture():counter(0),camera(0){
	busMgr_ref_count++;
	if( busMgr_ref_count == 1 ){
		busMgr = new BusManager();
	}
}

ys::FlyCaptureCapture::~FlyCaptureCapture(){
	busMgr_ref_count--;
	if( busMgr_ref_count == 0 ){
		delete busMgr;
		delete[] multicamera;
	}

	if( camera != 0 ){
		delete camera;
	}
}

bool ys::FlyCaptureCapture::get( cv::Mat &image, int channel ){

	image = cv_frame;
	
	return true;
}

bool ys::FlyCaptureCapture::next(){

	//Image orig_frame;
	Error error = camera->RetrieveBuffer( &orig_frame );

	if( error != PGRERROR_OK ){
		cerr<< "retrieve frame error at "<< inf.getCameraInfo() <<" in " << __FILE__ << " at " << __LINE__ <<endl;
		error.PrintErrorTrace();
		return false;
	}	
	orig_frame.Convert( PIXEL_FORMAT_BGR, &bgr_frame );
	if( error != PGRERROR_OK ){
		cerr<< "convert frame error at "<< inf.getCameraInfo() <<" in " << __FILE__ << " at " << __LINE__ <<endl;
		error.PrintErrorTrace();
		return false;
	}

	
	if( cv_frame.data == NULL ){
		return false;
	}
	
	memcpy( cv_frame.data, bgr_frame.GetData(), bgr_frame.GetDataSize() );
	//memcpy( cv_frame.data, orig_frame.GetData(), orig_frame.GetDataSize() );
	
	/*
	//IplImageを介したデータコピー
	IplImage *ipl = cvCreateImage( cvSize( inf.width, inf.height ), IPL_DEPTH_8U, 3);
	memcpy( ipl->imageData, bgr_frame.GetData(), bgr_frame.GetDataSize());
	cv::Mat tmp(ipl);
	tmp.copyTo( cv_frame );
	
	sstr.str("");
	sstr<< inf.camera_number << "_" << counter << "cv.jpg" <<flush;
	imwrite( sstr.str(), cv_frame );
	
	sstr.str("");
	sstr<< inf.camera_number << "_" << counter << "orig2.jpg" <<flush;
	orig_frame.Save( sstr.str().c_str() );
	*/

	TimeStamp time = orig_frame.GetTimeStamp();
	inf.second = time.seconds;
	inf.msecond = time.microSeconds;

	counter++;

	return true;
}

int ys::FlyCaptureCapture::currentNum(){
	return counter;
}

bool ys::FlyCaptureCapture::open( const std::string &filename ){

	Error error;
	
	if( connected_camera_num == 0 ){
		error = busMgr->GetNumOfCameras(&connected_camera_num);
		if( error != PGRERROR_OK ){
			cerr<< "failed to initial camera open. " <<endl;
			error.PrintErrorTrace();
			return false;
		}
		
		cout<< "connected camera num : " << connected_camera_num <<endl;

		if( connected_camera_num == 0 ){
			cerr<< "no camera connected" <<endl;
			return false;
		}

		if( enable_synchronized_multicamera ){
			multicamera = new Camera*[connected_camera_num];
		}

	}

	if( opened_camera_num >= connected_camera_num ){
		cerr<< "Error : too many camera opened" <<endl;
		cerr<< "\tnum of connected camera is " << connected_camera_num <<endl;
		cerr<< "\tnum of opening camera is " << opened_camera_num <<endl;
		return false;
	}
	
	//カメラの取得
	PGRGuid guid;
	error = busMgr->GetCameraFromIndex(opened_camera_num, &guid);
	unsigned int camera_number = opened_camera_num;
	opened_camera_num++;

	if( error != PGRERROR_OK ){
		cerr<< "failed to get camera" <<endl;
		error.PrintErrorTrace();
		return false;
	}

	//カメラへの接続
	camera = new Camera();
	error = camera->Connect( &guid );
	if( error != PGRERROR_OK ){
		cerr<< "failed to connect camera at " << inf.getCameraInfo() <<endl;
		error.PrintErrorTrace();
		return false;
	}

	//************************
	//** 一つ一つ設定する用 **
	//************************
	//FlyCapture2::CameraControlDlg dlg;
	//dlg.Connect( camera );
	//dlg.Show();


	//カメラの設定確認
	
	FC2Config camera_conf;
	camera->GetConfiguration( &camera_conf );

	if( camera_conf.grabMode == DROP_FRAMES ){
		cout<< "\tgrab mode : DROP_FRAME" <<endl;
	}
	cout<< "\tnum of buffers" << camera_conf.numBuffers <<endl;

	camera_conf.grabMode = BUFFER_FRAMES;

	error = camera->SetConfiguration( &camera_conf );
	if( error != PGRERROR_OK ){
		cerr<< "Error : failed to set camera configuration." <<endl;
		error.PrintErrorTrace();
		return false;
	}

	camera->GetConfiguration( &camera_conf );
	if( camera_conf.grabMode == BUFFER_FRAMES ){
		cout<< "\tgrab mode : BUFFER_FRAMES" <<endl;
	}
	

	//カメラのパラメータ設定
	error = camera->SetVideoModeAndFrameRate(
		VIDEOMODE_640x480RGB,
		FRAMERATE_30 );
	if( error != PGRERROR_OK ){
		error.PrintErrorTrace();
		cerr<< "Error starting cameras. " <<endl
			<< "This example requires cameras to be able to set to 640x480 Y8 at 30fps. " <<endl
			<< "If your camera does not support this mode, please edit the source code and recompile the application. " <<endl
			<< "Press Enter to exit. " <<endl
			<<endl;
		getchar();
		return false;
	}

	CameraInfo camInfo;
	error = camera->GetCameraInfo( &camInfo );
	inf.camera_number = camera_number;
	inf.serial_number = camInfo.serialNumber;

	//マルチカメラ時の制御
	if( enable_synchronized_multicamera ){

		multicamera[camera_number] = camera;
		if( opened_camera_num == connected_camera_num ){
			int error_counter = 0;
			const int error_max = 20;
			do{
				if( error_counter >= error_max ) break;
				error = Camera::StartSyncCapture( connected_camera_num, (const Camera**)multicamera );
				if( error == PGRERROR_OK ) break;
				error_counter++;

				cerr<< "failed to synchronize camera, " << error_counter << " times." <<endl;

				//Sleep(500);
			}
			while( error != PGRERROR_OK );

			if( error_counter == error_max ){
				cerr<< "failed to start synchronized capture "<< error_counter <<" times in " << __FILE__ << " at " << __LINE__ <<endl;
				cerr<< "+++++ stack trace +++++" <<endl;
				error.PrintErrorTrace();
				cerr<< "+++++++++++++++++++++++" <<endl;
				return false;
			}

			cout<< "start synchronized multi camera " << error_counter << "times." <<endl;
		}
		else {
			/*
			//画像保持用のcv::Matの初期化を行う
			error = camera->StartCapture();
			if( error != PGRERROR_OK ){
				cerr<< "failed to start capture from FlyCapture" <<endl;
				error.PrintErrorTrace();
				return false;
			}
			initCvMat();
			error = camera->StopCapture();
			if( error != PGRERROR_OK ){
				cerr<< "failed to stop capture from FlyCapture" <<endl;
				error.PrintErrorTrace();
				return false;
			}
			*/
		}
	}
	else {
		error = camera->StartCapture();
		if( error != PGRERROR_OK ){
			cerr<< "failed to start capture at" << inf.getCameraInfo() << endl;
			error.PrintErrorTrace();
			return false;
		}
		//initCvMat();
	}

	return true;
}

bool ys::FlyCaptureCapture::setOption( const std::string &name, const std::string &val ){


	return false;
}

bool ys::FlyCaptureCapture::isOpen(){
	return camera->IsConnected();
}

void ys::FlyCaptureCapture::release(){
	camera->StopCapture();
	camera->Disconnect();
}

string ys::FlyCaptureCapture::getSourceName(){
	return "FlyCaptureCapture";
}

string ys::FlyCaptureCapture::getFileName(){

	stringstream sstr;
	sstr<< "camera number:" << inf.camera_number << ", serial number:" << inf.serial_number <<flush;
	return sstr.str();
}

ys::Info ys::FlyCaptureCapture::getInfo(){

	return inf;
}

void ys::FlyCaptureCapture::setMultiCameraNum( int num ){
	open_camera_num = num;
}

int ys::FlyCaptureCapture::getMultiCameraNum(){
	Error error = busMgr->GetNumOfCameras( &connected_camera_num);
	return connected_camera_num;
}

bool ys::FlyCaptureCapture::enableSyncMultiCamera(){
	enable_synchronized_multicamera = true;
	return true;
}

bool ys::FlyCaptureCapture::initCvMat(){
	
	int error_counter = 0;
	const int init_max = 10;
	Error error;
	//Image orig_frame;
	do{
		if( error_counter >= init_max ) break;

		//cv::Matの初期化のために一度カメラから画像を取得
		error = camera->RetrieveBuffer( &orig_frame );
		if( error == PGRERROR_OK ){
			break;
		}

		error_counter++;
	}while( error != PGRERROR_OK );

	if( error_counter >= init_max ){
		cerr<< "Error initial Retrieve Image at camera " << inf.camera_number << "("<< inf.serial_number <<") in "
			<< __FILE__ << " at " << __LINE__ <<endl;
		error.PrintErrorTrace();
		cerr<<endl
			<< "***********************************" <<endl
			<< "connected camera num = " << connected_camera_num <<endl
			<< "open_camera_num = " << open_camera_num <<endl
			<< "must be equal these parameter" <<endl
			<< "***********************************" <<endl
			<<endl;
		return false;
	}
	cout<< "error num : " << error_counter <<endl;

	assert( orig_frame.GetRows() > 0 );
	assert( orig_frame.GetCols() > 0 );

	orig_frame.Convert(PIXEL_FORMAT_BGR, &bgr_frame );

	cv_frame = Mat( bgr_frame.GetRows(), bgr_frame.GetCols(), CV_8UC3 );
	memcpy( cv_frame.data, bgr_frame.GetData(), bgr_frame.GetDataSize());
	
	data_size = bgr_frame.GetDataSize();
	
	inf.width = cv_frame.cols;
	inf.height = cv_frame.rows;
	
	TimeStamp time = orig_frame.GetTimeStamp();
	inf.second = time.seconds;
	inf.msecond = time.microSeconds;

	return true;
	
}

void ys::FlyCaptureCapture::editProperty(){
	
	CameraControlDlg camDlg;
	camDlg.Connect( camera );
	camDlg.Show();

}

//#define DEBUG_getProperty
//Property→Option
void setAbsControl2Option(const Property& prop, std::map<std::string,std::string>&options,
	const ys::FlyCaptureCapture::PropertyName& opt_name ){
	if( prop.absControl ){
		options[opt_name.abs] = "on";
	}
	else {
		options[opt_name.abs] = "off";
	}
}
void setAutoManual2Option(const Property& prop, std::map<std::string,std::string>&options,
	const ys::FlyCaptureCapture::PropertyName& opt_name ){
	
		if( prop.autoManualMode ){
			options[opt_name.auto_manual] = "on";
		}
		else {
			options[opt_name.auto_manual] = "off";
		}
}
void setAbsValue2Option(const Property& prop, std::map<std::string,std::string>&options,
	const ys::FlyCaptureCapture::PropertyName& opt_name){
	
		stringstream sstr;
		sstr<< prop.absValue <<flush;
		options[opt_name.value] = sstr.str();
}
//Option→Property
void setAbsControl2Property( const std::map<std::string,std::string>&options,
	const ys::FlyCaptureCapture::PropertyName& opt_name, Property& prop ){

		map<string,string>::const_iterator itr = options.find(opt_name.abs);
	if( itr == options.end() ){
		return;
	}

	if( itr->second == "on" ){
		prop.absControl = true;
	}
	else if( itr->second == "off") {
		prop.absControl = false;
	}
}
void setAutoManual2Property( const std::map<std::string,std::string>&options,
	const ys::FlyCaptureCapture::PropertyName& opt_name, Property& prop ){

		map<string,string>::const_iterator itr = options.find(opt_name.auto_manual);
	if( itr == options.end() ){
		return;
	}

	if( itr->second == "on" ){
		prop.autoManualMode = true;
	}
	else if( itr->second == "off") {
		prop.autoManualMode = false;
	}
	else {
		cerr<< "illegal option for " << opt_name.auto_manual << " in " << __FILE__ << " at " << __LINE__ <<endl;
	}
}
void setAbsValue2Property( const std::map<std::string,std::string>&options,
	const ys::FlyCaptureCapture::PropertyName& opt_name, Property& prop ){

		map<string,string>::const_iterator itr = options.find(opt_name.value);
	if( itr == options.end() ){
		return;
	}

	stringstream sstr;
	sstr<< itr->second;
	sstr>> prop.absValue;
}


void ys::FlyCaptureCapture::getProperty( std::map<std::string,std::string> &options ){

	options.clear();
	stringstream sstr;

	//**************
	//*** 明るさ ***
	//**************
	{
		Property prop( BRIGHTNESS );
		camera->GetProperty( &prop );
		setAbsControl2Option( prop, options, prop_brightness);
		setAbsValue2Option( prop, options, prop_brightness);
		setAutoManual2Option( prop, options, prop_brightness );
	}

	//************
	//*** 露出 ***
	//************
	{
		Property prop( AUTO_EXPOSURE  );
		camera->GetProperty( &prop );
		setAbsControl2Option( prop, options, prop_exposure);
		setAbsValue2Option( prop, options, prop_exposure);
		setAutoManual2Option( prop, options, prop_exposure );
	}
	
	//********************
	//*** シャープネス ***
	//********************
	{
		Property prop( SHARPNESS );
		camera->GetProperty( &prop );
		setAbsControl2Option( prop, options, prop_sharpness);
		setAbsValue2Option( prop, options, prop_sharpness);
		setAutoManual2Option( prop, options, prop_sharpness );
	}
	//*******************
	//*** 色合い(HUE) ***
	//*******************
	{
		Property prop( HUE  );
		camera->GetProperty( &prop );
		setAbsControl2Option( prop, options, prop_hue);
		setAbsValue2Option( prop, options, prop_hue);
		setAutoManual2Option( prop, options, prop_hue );
	}
	//************************
	//*** 彩度(saturation) ***
	//************************
	{
		Property prop( SATURATION  );
		camera->GetProperty( &prop );
		setAbsControl2Option( prop, options, prop_saturation);
		setAbsValue2Option( prop, options, prop_saturation);
		setAutoManual2Option( prop, options, prop_saturation );
	}
	//**************
	//*** ガンマ ***
	//**************
	{
		Property prop( GAMMA );
		camera->GetProperty( &prop );
		setAbsControl2Option( prop, options, prop_gamma);
		setAbsValue2Option( prop, options, prop_gamma);
		setAutoManual2Option( prop, options, prop_gamma );
	}
	//******************
	//*** シャッター ***
	//******************
	{
		Property shutterProp(SHUTTER);
		camera->GetProperty( &shutterProp );
		setAbsControl2Option( shutterProp, options, prop_shutter);
#ifdef DEBUG_getProperty
		cerr<< "getProperty"<<inf.serial_number<<":shutter abs = \"" << options["shutter_abs"] <<"\""<<endl;
#endif
		//シャッタースピード
		setAbsValue2Option( shutterProp, options, prop_shutter);
#ifdef DEBUG_getProperty
		cerr<< "getProperty"<<inf.serial_number<<":shutter speed = \"" << options["shutter_speed"] <<"\""<<endl;
#endif
		//シャッターの自動調整
		setAutoManual2Option( shutterProp, options, prop_shutter );
#ifdef DEBUG_getProperty
		cerr<< "getProperty"<<inf.serial_number<<":shutter auto = \"" << options["shutter_auto"] <<"\""<<endl;
#endif
	}

	//**************
	//*** ゲイン ***
	//**************
	{
		Property gainProp( GAIN );
		camera->GetProperty( & gainProp );
		//abs
		setAbsControl2Option(gainProp, options, prop_gain );
#ifdef DEBUG_getProperty
		cerr<< "getProperty"<<inf.serial_number<<":gain abs = \"" << options["gain_abs"] <<"\""<<endl;
#endif
		//ゲインの値
		setAbsValue2Option(gainProp, options, prop_gain);
#ifdef DEBUG_getProperty
		cerr<< "getProperty"<<inf.serial_number<<":gain value = \"" << options["gain_val"] <<"\""<<endl;
#endif
		//ゲインの自動調整
		setAutoManual2Option( gainProp, options, prop_gain );
#ifdef DEBUG_getProperty
		cerr<< "getProperty"<<inf.serial_number<<":gain auto = \"" << options["gain_auto"] <<"\""<<endl;
#endif
	}

	//************************
	//*** ホワイトバランス ***
	//************************
	{
		Property whiteProp( WHITE_BALANCE );
		camera->GetProperty( &whiteProp );

		//ホワイトバランスの値は特別なので別指定
		stringstream sstr;
		unsigned int red_val = whiteProp.valueA;
		sstr<< red_val;
		options[prop_white.red] = sstr.str();
		sstr.str("");
		unsigned int blue_val = whiteProp.valueB;
		sstr<< blue_val;
		options[prop_white.blue] = sstr.str();

		setAutoManual2Option( whiteProp, options, prop_white);
#ifdef DEBUG_getProperty
		cerr<< "getProperty" << inf.serial_number << ":white red = \"" << options["white_red"] << "\"" <<endl;
		cerr<< "getProperty" << inf.serial_number << ":white blue = \"" << options["white_blue"] << "\"" <<endl;
		cerr<< "getProperty" << inf.serial_number << ":white auto = \"" << options["white_auto"] << "\"" <<endl;
#endif

	}
}

//#define DEBUG_setProperty
void ys::FlyCaptureCapture::setProperty( const std::map<std::string,std::string>&options ){
	stringstream sstr;

	//明るさ
	{
		Property prop(BRIGHTNESS );
		camera->GetProperty(&prop);
		setAbsControl2Property( options, prop_brightness, prop );
		setAbsValue2Property( options, prop_brightness, prop );
		setAutoManual2Property( options, prop_brightness, prop );
		Error error = camera->SetProperty( &prop );
		if( error != PGRERROR_OK ){
			cerr<< "Error : Failed to set brightness Property to Camera "
				<< inf.serial_number 
				<<endl
				<< "++++ FlyCapture2 Error Trace ++++"
				<<endl;
			error.PrintErrorTrace();
			cerr<< "+++++++++++++++++++++++++++++++++" 
				<<endl;
		}
	}
	//露出
	{
		Property prop(AUTO_EXPOSURE );
		camera->GetProperty(&prop);
		setAbsControl2Property( options, prop_exposure, prop );
		setAbsValue2Property( options, prop_exposure, prop );
		setAutoManual2Property( options, prop_exposure, prop );
		Error error = camera->SetProperty( &prop );
		if( error != PGRERROR_OK ){
			cerr<< "Error : Failed to set exposure Property to Camera "
				<< inf.serial_number 
				<<endl
				<< "++++ FlyCapture2 Error Trace ++++"
				<<endl;
			error.PrintErrorTrace();
			cerr<< "+++++++++++++++++++++++++++++++++" 
				<<endl;
		}
	}
	//シャープネス
	{
		Property prop(SHARPNESS );
		camera->GetProperty(&prop);
		setAbsControl2Property( options, prop_sharpness, prop );
		setAbsValue2Property( options, prop_sharpness, prop );
		setAutoManual2Property( options, prop_sharpness, prop );
		Error error = camera->SetProperty( &prop );
		if( error != PGRERROR_OK ){
			cerr<< "Error : Failed to set sharpness Property to Camera "
				<< inf.serial_number 
				<<endl
				<< "++++ FlyCapture2 Error Trace ++++"
				<<endl;
			error.PrintErrorTrace();
			cerr<< "+++++++++++++++++++++++++++++++++" 
				<<endl;
		}
	}
	//色合い
	{
		Property prop( HUE );
		camera->GetProperty(&prop);
		setAbsControl2Property( options, prop_hue, prop );
		setAbsValue2Property( options, prop_hue, prop );
		setAutoManual2Property( options, prop_hue, prop );
		Error error = camera->SetProperty( &prop );
		if( error != PGRERROR_OK ){
			cerr<< "Error : Failed to set hue Property to Camera "
				<< inf.serial_number 
				<<endl
				<< "++++ FlyCapture2 Error Trace ++++"
				<<endl;
			error.PrintErrorTrace();
			cerr<< "+++++++++++++++++++++++++++++++++" 
				<<endl;
		}
	}
	//彩度
	{
		Property prop( SATURATION );
		camera->GetProperty(&prop);
		setAbsControl2Property( options, prop_saturation, prop );
		setAbsValue2Property( options, prop_saturation, prop );
		setAutoManual2Property( options, prop_saturation, prop );
		Error error = camera->SetProperty( &prop );
		if( error != PGRERROR_OK ){
			cerr<< "Error : Failed to set saturation Property to Camera "
				<< inf.serial_number 
				<<endl
				<< "++++ FlyCapture2 Error Trace ++++"
				<<endl;
			error.PrintErrorTrace();
			cerr<< "+++++++++++++++++++++++++++++++++" 
				<<endl;
		}
	}
	//ガンマ
	{
		Property prop( GAMMA );
		camera->GetProperty(&prop);
		setAbsControl2Property( options, prop_gamma, prop );
		setAbsValue2Property( options, prop_gamma, prop );
		setAutoManual2Property( options, prop_gamma, prop );
		Error error = camera->SetProperty( &prop );
		if( error != PGRERROR_OK ){
			cerr<< "Error : Failed to set gamma Property to Camera "
				<< inf.serial_number 
				<<endl
				<< "++++ FlyCapture2 Error Trace ++++"
				<<endl;
			error.PrintErrorTrace();
			cerr<< "+++++++++++++++++++++++++++++++++" 
				<<endl;
		}
	}
	//シャッター
	Property shutterProp(SHUTTER);
	camera->GetProperty( &shutterProp );
	//abs
	setAbsControl2Property( options, prop_shutter,shutterProp);
#ifdef DEBUG_setProperty
	cerr<< "setProperty"<<inf.serial_number<<":shutter abs = \"" << shutterProp.absControl <<"\""<<endl;
#endif
	//シャッタースピード
	setAbsValue2Property( options, prop_shutter, shutterProp );
#ifdef DEBUG_setProperty
	cerr<< "setProperty"<<inf.serial_number<<":shutter speed = \"" << shutterProp.absValue <<"\""<<endl;
#endif
	//シャッターの自動調整
	setAutoManual2Property( options, prop_shutter, shutterProp );

#ifdef DEBUG_setProperty
	cerr<< "setProperty"<<inf.serial_number<<":shutter auto = \"" << shutterProp.autoManualMode <<"\""<<endl;
#endif

	Error error = camera->SetProperty( &shutterProp );
	if( error != PGRERROR_OK ){
		cerr<< "Error : Failed to set shutter Property to Camera "
			<< inf.serial_number 
			<<endl
			<< "++++ FlyCapture2 Error Trace ++++"
			<<endl;
		error.PrintErrorTrace();
		cerr<< "+++++++++++++++++++++++++++++++++" 
			<<endl;
	}


	//Gain
	Property gainProp( GAIN );
	camera->GetProperty( & gainProp );
	//abs
	setAbsControl2Property( options, prop_gain,gainProp);
#ifdef DEBUG_setProperty
	cerr<< "setProperty"<<inf.serial_number<<":gain abs = \"" << gainProp.absControl <<"\""<<endl;
#endif
	//ゲインの値
	setAbsValue2Property( options, prop_gain, gainProp );
#ifdef DEBUG_setProperty
	cerr<< "setProperty"<<inf.serial_number<<":gain value = \"" <<  gain <<  gainProp.absValue <<"\""<<endl;
#endif
	setAutoManual2Property( options, prop_gain, gainProp );
#ifdef DEBUG_setProperty
	cerr<< "setProperty"<<inf.serial_number<<":gain auto = \"" << gainProp.autoManualMode <<"\""<<endl;
#endif
	error = camera->SetProperty( &gainProp );
	if( error != PGRERROR_OK ){
		cerr<< "Error : Failed to set gain Property to Camera "
			<< inf.serial_number
			<<endl
			<< "++++ FlyCapture2 Error Trace ++++"
			<<endl;
		error.PrintErrorTrace();
		cerr<< "+++++++++++++++++++++++++++++++++" 
			<<endl;
	}

	//ホワイトバランス
	Property whiteProp( WHITE_BALANCE );
	camera->GetProperty( &whiteProp );

	map<string,string>::const_iterator itr = options.find(prop_white.red);
	if( itr != options.end() ){
		stringstream sstr;
		sstr<< itr->second;
		sstr>> whiteProp.valueA;
	}
	itr = options.find(prop_white.blue);
	if( itr != options.end() ){
		stringstream sstr;
		sstr<< itr->second;
		sstr>> whiteProp.valueB;
	}
	setAutoManual2Property( options, prop_white, whiteProp );
#ifdef DEBUG_setProperty
	cerr<< "setProperty" << inf.serial_number << ":white red = \"" << whiteProp.valueA << "\"" <<endl;
	cerr<< "setProperty" << inf.serial_number << ":white blue = \"" << whiteProp.valueB << "\"" <<endl;
	cerr<< "setProperty" << inf.serial_number << ":white auto = \"" << whiteProp.autoManualMode << "\"" <<endl;
#endif
	error = camera->SetProperty( &whiteProp );
	if( error != PGRERROR_OK ){
		cerr<< "Error : Failed to set white balance Property to Camera "
			<< inf.serial_number
			<<endl
			<< "++++ FlyCapture2 Error Trace ++++"
			<<endl;
		error.PrintErrorTrace();
		cerr<< "+++++++++++++++++++++++++++++++++" 
			<<endl;
	}


}

void fileStorage2Options( FileStorage& fs, map<string,string>& options,
	const ys::FlyCaptureCapture::PropertyName& prop_name ){
	options[prop_name.abs] = (string)fs[prop_name.abs];
	options[prop_name.value] = (string)fs[prop_name.value];
	options[prop_name.auto_manual] = (string)fs[prop_name.auto_manual];
}

bool ys::FlyCaptureCapture::readProperty( FileStorage& fs, map<string,string>& options ){

	fileStorage2Options( fs, options, prop_brightness );

	fileStorage2Options( fs, options, prop_exposure );

	fileStorage2Options( fs, options, prop_sharpness );

	fileStorage2Options( fs, options, prop_hue );

	fileStorage2Options( fs, options, prop_saturation );

	fileStorage2Options( fs, options, prop_gamma );

	fileStorage2Options( fs, options, prop_shutter );

	fileStorage2Options( fs, options, prop_gain );

	options[prop_white.abs] = (string)fs[prop_white.abs];
	options[prop_white.auto_manual] = (string)fs[prop_white.auto_manual];
	options[prop_white.red] = (string)fs[prop_white.red];
	options[prop_white.blue] = (string)fs[prop_white.blue];

	return true;
}

#endif

ys::AbstractCapture* ys::getCapture( std::string &type ){
	if( type == "video" ){
		cout<< "open video" <<endl;
		return new ys::VideoCapture();
	}
	else if( type == "camera" ){
		cout<< "open camera" <<endl;
		return new ys::CameraCapture();
	}
	else if( type == "list" ){
		cout<< "open image list" <<endl;
		return new ys::ImageList();
	}
#ifndef Disable_FlyCapture
	else if( type == "flycapture" ){
		cout<< "open FlyCapture" <<endl;
		return new ys::FlyCaptureCapture();
	}
#endif
	else {
		cerr<< "illegal error: invalid source > " << type <<endl;
		cerr<< "exit program" <<endl;
		return 0;
	}
}

ys::Capture::Capture( std::string &type ){
	_cap = getCapture(type);
}

ys::Capture::Capture( int type ){
	string str_type;
	if( type == 0 )
		str_type = "video";
	else if( type == 1 )
		str_type = "camera";
	else if( type == 2 )
		str_type = "images";
#ifndef Disable_FlyCapture
	else if( type == 3 )
		str_type = "flycapture";
#endif
	_cap = getCapture( str_type );
}

ys::Capture::~Capture(){
	if( _cap != NULL )
		delete _cap;
}

ys::AbstractCapture* ys::Capture::_getSource( std::string &type ){

	if( type == "video" ){
		cout<< "open video" <<endl;
		return new ys::VideoCapture();
	}
	else if( type == "camera" ){
		cout<< "open camera" <<endl;
		return new ys::CameraCapture();
	}
	else if( type == "list" ){
		cout<< "open image list" <<endl;
		return new ys::ImageList();
	}
#ifndef Disable_FlyCapture
	else if( type == "flycapture" ){
		cout<< "open FlyCapture" <<endl;
		return new ys::FlyCaptureCapture();
	}
#endif
	else {
		cerr<< "illegal error: invalid source > " << type <<endl;
		cerr<< "exit program" <<endl;
		return 0;
	}
}

void ys::Capture::setType( std::string& type ){
	_cap = getCapture(type);
}

bool ys::Capture::get( cv::Mat &image, int channel ){
	return _cap->get( image, channel );
}

bool ys::Capture::next(){
	return _cap->next();
}

int ys::Capture::currentNum(){
	return _cap->currentNum();
}

bool ys::Capture::setOption( const std::string &name, const std::string &val ){
	return _cap->setOption( name, val );
}

bool ys::Capture::open( const std::string &filename ){
	return _cap->open( filename );
}

bool ys::Capture::isOpen(){
	return _cap->isOpen();
}

void ys::Capture::release(){
	_cap->release();
}

ys::Info ys::Capture::getInfo(){
	return _cap->getInfo();
}

string ys::Capture::getSourceName(){
	return _cap->getSourceName();
}

void ys::Capture::setMultiCameraNum( int num ){
	_cap->setMultiCameraNum( num );
}

int ys::Capture::getMultiCameraNum(){
	return _cap->getMultiCameraNum();
}

bool ys::Capture::enableSyncMultiCamera(){
	return _cap->enableSyncMultiCamera();
}

bool ys::Capture::initCvMat(){
	return _cap->initCvMat();
}

void ys::Capture::editProperty(){
	_cap->editProperty();
}

void ys::Capture::getProperty( std::map<std::string, std::string> &options ){
	_cap->getProperty(options);
}

void ys::Capture::setProperty( const std::map<std::string, std::string> &options ){
	_cap->setProperty( options );
}

bool ys::Capture::saveProperty( const std::string& fname, const std::map<std::string,std::string>& options ){
	
	//OpenCVのXML/YAMLのファイル入出力クラスを使用
	//XML、YAMLで出力するかは拡張子で判定される
	FileStorage fs( fname, FileStorage::WRITE );
	if( !fs.isOpened() ){
		return false;
	}
	for( map<string,string>::const_iterator itr=options.begin();
		itr != options.end(); itr++ ){
			fs<< itr->first << itr->second;
	}

	return true;
}



bool ys::Capture::readProperty( const std::string& fname, std::map<std::string,std::string>& options ){
	//OpenCVのXML/YAMLのファイル入出力クラスを使用
	//読み込んだファイルは内容からXMLかYAMLに判別される。
	FileStorage fs( fname, FileStorage::READ );
	if(!fs.isOpened()){
		return false;
	}

	return _cap->readProperty(fs, options);

}

ys::ImageSequence::ImageSequence()
	:_cap_type("video"), _processor(NULL), _interval(1)
	 ,_show_progress(false),captures(0)
	 ,_window_name("ImageSequence") ,_use_gui(false)
	 ,_size(240,320)
	 ,enable_synchronized(false)
	 ,_rotate(ROTATE_NONE)
{}

ys::ImageSequence::~ImageSequence(){
	if( captures != 0 )
		delete[] captures;
}

void ys::ImageSequence::setCaptureType( string type ){
	_cap_type = type;
}

void ys::ImageSequence::setRotate( int rotate ){
	_rotate = rotate;
}

void ys::ImageSequence::setFrameSize( cv::Size &size ){
	_size = size;
}

void ys::ImageSequence::setFrameSize( int width, int height ){
	_size = Size(width, height);
}

void ys::ImageSequence::setInterval( int interval ){
	_interval = interval;
}

void ys::ImageSequence::setWindowName( string &name ){
	_window_name = name;
}

void ys::ImageSequence::showProgress( bool show ){
	_show_progress = show;
}
void ys::ImageSequence::setImageProcessor( ImageProcessorInterface &processor ){
	_processor = &processor;
}

void ys::ImageSequence::rotateLeft( cv::Mat* srcs, cv::Mat* dsts, int n )
{
	const int& src_width = srcs[0].cols;
	const int& src_height = srcs[0].rows;
	const int& src_step = srcs[0].step;
	const int& src_ch = srcs[0].channels();
	
	const int& dst_step = dsts[0].step;


	for( int j=0; j<n; j++ ){
		uchar* src_data = srcs[j].data;
		uchar* dst_data = dsts[j].data;

		for( int i=0; i<src_height; i++ ){

			for( int h=0; h<src_width; h++){
				memcpy(dst_data + src_ch * ((src_width - h - 1) * src_height + i),
					src_data + src_ch * (i * src_width + h), src_ch );
			}
		}
	}
}

void ys::ImageSequence::rotateRight( cv::Mat* srcs, cv::Mat* dsts, int n )
{
	const int& src_width = srcs[0].cols;
	const int& src_height = srcs[0].rows;
	const int& src_step = srcs[0].step;
	const int& src_ch = srcs[0].channels();
	
	const int& dst_step = dsts[0].step;

	for( int j=0; j<n; j++ ){
		uchar* src_data = srcs[j].data;
		uchar* dst_data = dsts[j].data;

		for( int i=0; i<src_height; i++ ){

			for( int h=0; h<src_width; h++){
				memcpy(dst_data + src_ch * ( h* src_height + (src_height - i - 1)),
					src_data + src_ch * (i * src_width + h), src_ch );
			}
		}
	}
}

void ys::ImageSequence::rotateNone( cv::Mat* srcs, cv::Mat* dsts, int n )
{
	//何もしない
}

bool ys::ImageSequence::run(){
	
	captures = new Capture[1];
	
	Capture &cap = captures[0];

	cap.setType(_cap_type);

	if( _processor == NULL ){
		cerr<< "error: empty ImageProcessorInterface in "
			<< __FILE__ << " at " << __LINE__ <<endl;
		return false;
	}

	map<string,string> options;
	_processor->getOptions( options );

	for( map<string,string>::iterator itr = options.begin();
		itr != options.end(); itr++ ){
			cap.setOption( itr->first, itr->second );
	}

	cap.open("0");
	if( cap.initCvMat()){
		cerr<< "error: failed to initialize cv::Mat for Capture in " << __FILE__ << " at " << __LINE__ <<endl;
		return false;
	}

	if( !cap.isOpen() ){
		cerr<< "error: failed to open Capture in "
			<< __FILE__ << " at " << __LINE__ <<endl;
		return false;
	}

	if( !cap.next() ){
		cerr<< "error: failed to first image in "
			<< __FILE__ << " at " << __LINE__ <<endl;
		return false;
	}

	cv::Mat orig_frame;

	_processor->onInit( orig_frame );
	
	bool now_processing_image = false;
	int counter = 0;

	do{
		//フレームの取得
		cap.get(orig_frame);
		bool res = _processor->onProcess( orig_frame, counter );
		if( !res ){
			cout<< "break" <<endl;
			break;
		}

	} while( cap.next() );

	_processor->onFinish();

	cout<< "finish program";

	return true;
}
void ys::ImageSequence::enableSyncMultiCamera(){
	enable_synchronized = true;
}


void ys::ImageSequence::setMultiCameraNum( int num ){
	captures_num = num;
}

ys::Info ys::ImageSequence::getCaptureInfo( int i ){
	return captures[i].getInfo();
}

bool ys::ImageSequence::runMulti(){

	cout<< "Initialize Multi Camera mode" <<endl;

	captures = new Capture[captures_num];

	for( int i=0; i<captures_num; i++ ){
		cout<< "Initializing " << i << " camera..." <<endl;

		Capture &cap = captures[i];

		cap.setType(_cap_type);
		cap.setMultiCameraNum( captures_num ); //ImageSequence.h で Disable_FlyCapture が有効になってない？

		if( enable_synchronized ){
			cap.enableSyncMultiCamera();
		}

		if( _processor == NULL ){
			cerr<< "error: empty ImageProcessorInterface in "
				<< __FILE__ << " at " << __LINE__ <<endl;
			return false;
		}

		map<string,string> options;
		_processor->getOptions( options );

		for( map<string,string>::iterator itr = options.begin();
			itr != options.end(); itr++ ){
				cap.setOption( itr->first, itr->second );
		}

		bool result = cap.open("0");

		if( !result ){
			cerr<< "error: failed to open Capture in "
				<< __FILE__ << " at " << __LINE__ <<endl;
			return false;
		}
		cout<< "finish initialize " << i << " camera." <<endl;
	}
	for( int i=0; i<captures_num; i++ ){
		if( !captures[i].initCvMat()) return false;
	}

	cout<< "Finish Initialize Cameras" <<endl;

	cout<< "\tcapture type : " << captures[0].getSourceName() <<endl;
	cout<< "\tnumber of connected camera : " << captures[0].getMultiCameraNum() <<endl;
	cout<< "\tnumber of opened camera : " << captures_num <<endl;
	cout<< "\tsynchronized mode : ";
		if( enable_synchronized ) cout<< "true" <<endl;
		else cout<< "false" <<endl;
	
	cv::Mat *orig_frame = new cv::Mat[captures_num];
	Info *infs = new Info[captures_num];
	
	cv::Mat *rotate_frame;
	void (*rotate_func)(cv::Mat* srcs, cv::Mat*dsts, int n);
	if( _rotate == ROTATE_NONE ){
		rotate_frame = orig_frame;
		rotate_func = rotateNone;
	}
	else if( _rotate == ROTATE_LEFT ){
		rotate_frame = new cv::Mat[captures_num];
		rotate_func = rotateLeft;
	}
	else if( _rotate == ROTATE_RIGHT ){
		rotate_frame = new cv::Mat[captures_num];
		rotate_func = rotateRight;
	}

	for( int i=0; i<captures_num; i++){
		//フレームの取得
		captures[i].get(orig_frame[i]);
		infs[i] = captures[i].getInfo();
		
		rotate_frame[i].create( orig_frame[i].cols, orig_frame[i].rows, orig_frame[i].type() );
		//rotate_frame[i].setTo( Scalar(0,0,0));
	}
	rotate_func( orig_frame, rotate_frame, captures_num );

	_processor->onInitMulti( captures, rotate_frame, infs, captures_num );

	bool continue_capture = true;

	do{

		for( int i=0; i<captures_num; i++){
			//フレームの取得
			captures[i].get(orig_frame[i]);
			infs[i] = captures[i].getInfo();
		}
		
		rotate_func( orig_frame, rotate_frame, captures_num );

		bool continue_processor = _processor->onProcessMulti(
				captures, rotate_frame, infs, captures_num
			);
		if( !continue_processor ){
			cout<< "break process." <<endl;
			break;
		}
		
		for( int i=0; i<captures_num; i++){
			if( !captures[i].next()) continue_capture = false;
		}

	}while( continue_capture );
	
	_processor->onFinishMulti();

	delete[] orig_frame;
	delete[] infs;
	if( _rotate != ROTATE_NONE ){
		delete[] rotate_frame;
	}

	for( int i=0; i<captures_num; i++ ){
		captures[i].release();
	}

	cout<< "finish capture" <<endl;

	return true;
}
