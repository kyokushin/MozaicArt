#ifndef ImageSequence_h
#define ImageSequence_h

#define Disable_FlyCapture

#include <string>

#include <opencv/cv.h>
#include <opencv/highgui.h>


#ifndef Disable_FlyCapture
#include <flycapture/FlyCapture2.h>
#include <flycapture/FlyCapture2GUI.h>
#endif

#ifndef Disable_FlyCapture
#ifdef _DEBUG
#pragma comment( lib, "FlyCapture2d_v100.lib" )
#pragma comment( lib, "FlyCapture2GUId_v100.lib")
#else
#pragma comment( lib, "FlyCapture2_v100.lib" )
#pragma comment( lib, "FlyCapture2GUI_v100.lib")
#endif
#endif

/** \brief ImageSequence.h
 * 画像の連続処理を行うためのフレームワークを
 * 担当するクラスがまとまっています。
 */

namespace ys {

	/** キャプチャ情報を保存するための構造体
	 *
	 * 今のところ、FlyCaptureCaptureから取得される情報を
	 * 保持し、提供するためだけに利用されている。
	 */
	struct Info {
		///コンストラクタ
		Info():camera_number(0),serial_number(0){}
		/// カメラのインデックス番号。マルチカメラでの利用が目的。
		unsigned int camera_number;
		/// カメラのシリアルナンバー。マルチカメラでの利用が目的。
		unsigned int serial_number;
		/// 取得した画像の撮影時間の秒
		long long int second; 
		/// 取得した画像の撮影時間のmicrosecond
		unsigned int msecond; 
		/// 取得した画像の幅、高さ
		int width, height; 
		/// 取得した画像が同期されているか。マルチカメラでの同期確認が目的。
		unsigned int droped;
		/// 画像を読み込んだ場合用
		std::string fname;

		/** カメラ情報の簡易取得
		 *
		 * カメラ情報の簡易取得のために実装。
		 * このままは微妙なので、operator<<を実装したほうがいいかも。
		 */
		std::string getCameraInfo(){
			std::stringstream sstr;
			sstr<< "camera " << camera_number << "("<< serial_number <<")" <<std::flush;
			return sstr.str();
		}
	};


	/** 連続画像取得用の抽象クラス
	 *
	 * このクラスを継承したものがフレームワーク内で利用されている。
	 * コンストラクタでは変数の初期化などを行い、カメラへの接続は行わないこと。
	 * カメラへの接続はopenメソッドで行うものとする。
	 * 画像の取得はnextで行い、getで受け渡しとする。
	 */
	class AbstractCapture {
		public:
			/// デストラクタ
			virtual ~AbstractCapture(){}
			/// 画像の受け渡し
			virtual bool get( cv::Mat &image, int channel=0 ) = 0;
			/// 次の画像の取得
			virtual bool next() = 0;
			/// 所得した画像枚数
			virtual int currentNum() = 0;
			/// 汎用パラメータ設定
			virtual bool setOption(const std::string &name, const std::string &val ) = 0;

			/// カメラへの接続やディレクトリのオープン
			virtual bool open(const std::string &filename) = 0;
			/// カメラへの接続が行われているか、いないか
			/// ディレクトリのオープンに成功したか失敗したか
			virtual bool isOpen() = 0;
			/// カメラからの切断、ディレクトリのクローズ
			virtual void release() = 0;

			/// 連続画像の取得先（カメラのシリアルナンバー、ディレクトリ名、etc）を取得するためのもの
			virtual std::string getSourceName() = 0;
			/// オープンしたファイル名。主にディレクトリからの読み込み用
			virtual std::string getFileName() = 0;
			/// カメラからの取得情報を得る。主にFlyCaptureCapture用
			virtual Info getInfo()=0;

			//継承したキャプチャクラスの設定（任意）
			/// カメラごとの特殊なプロパティ設定。主にFlyCaptureCapture用
			virtual void editProperty(){};
			/// カメラの特殊なプロパティを設定する。
			virtual void setProperty( const std::map<std::string,std::string> &options){};
			/// カメラの特殊なプロパティを取得する。
			virtual void getProperty(std::map<std::string,std::string> &options){};
			virtual bool readProperty(cv::FileStorage& fs, std::map<std::string,std::string>& options){return true;}
			
			//マルチカメラ用
			/// マルチカメラ用メソッド。利用するカメラ数を指定する。
			virtual void setMultiCameraNum( int num ){};
			/// マルチカメラ用メソッド。利用しているカメラ数を取得する。
			virtual int getMultiCameraNum(){ return 1; };
			/// マルチカメラ用メソッド。カメラ間の同期処理を有効にする。
			virtual bool enableSyncMultiCamera(){ return false;};
			/// OpenCV以外のライブラリから画像を取得するのに必要なメソッド。
			///
			/// このメソッドで画像情報を保持するためのcv::Matの初期化を行う。
			virtual bool initCvMat(){return false;}
	};

	/** 動画取得用クラス
	 * 
	 * 動画取得用クラス。OpenCVのVideoCaptureクラスのラッパーになっている。
	 */
	class VideoCapture : public AbstractCapture {
		public:
			/// コンストラクタ
			VideoCapture()
				:_counter(0)
			{}
			/// デストラクタ
			virtual ~VideoCapture(){}
			/// フレームの取得
			virtual bool get(
					cv::Mat &image,/**< [out] 動画から取得された画像データ */
					int channel = 0 /**< [in] 取得する画像のチャンネル数。デフォルトは0で、カラー画像。 */
					);
			/// 次のフレームの読み込み
			/// @param return フレーム読み込みの成否。次のフレームがない場合はfalseが返る
			virtual bool next();
			/// 現在のフレーム取得数
			/// @param return フレーム取得数
			virtual int currentNum();
			/// オプションの指定。設定はないので無効になっている。
			virtual bool setOption(const std::string &name, const std::string &val ){return false;}
			/// ビデオを開く
			/// @param return ビデオファイルオープンの成否
			virtual bool open(
					const std::string &filename /**< [in] ビデオファイル名 */
					);
			/// ビデオファイルオープンの成否
			/// @param return ビデオファイルオープンの成否
			virtual bool isOpen();
			/// ビデオファイルのクローズ
			virtual void release();
			/// 利用しているクラスのソース名を取得する。
			/// @param return "VideoCapture" が返る
			virtual std::string getSourceName();
			/// オープンしたビデオファイル名の取得
			/// @param return ビデオファイル名
			virtual std::string getFileName();
			/// 取得画像の情報を取得する（未実装）
			/// @param return 未実装。
			virtual Info getInfo(){return Info();};
			
		protected:
			cv::VideoCapture _cap; /**< 動画像読み込みの本体。OpenCVに依存 */
			std::string _filename; /**< 動画ファイル名 */
			int _counter; /**< 読み込んだフレーム枚数 */
	};

	/** カメラから画像を取得する
	 *
	 * カメラから画像を取得するためのクラス。
	 * OpenCVのcv::VideoCaptureクラスのラッパーになっており、
	 * カメラからの画像取得のみを行う。
	 */
	class CameraCapture : public VideoCapture {
		public:
			/// カメラへの接続を行うこと
			/// @param return カメラ接続の成否
			virtual bool open(
					const std::string &filename /**< カメラ番号 */
					);
			/// クラス名の取得
			/// @param return "CameraCapture" が返る
			virtual std::string getSourceName();
	};

	/** ディレクトリやリストファイルから画像を取得する。
	 *
	 * 指定されたディレクトリやリストファイルから画像を取得する。
	 * 現在の実装ではWindowsのみに対応。
	 * Pocoのようなクロスプラットフォームなライブラリを利用して
	 * プラットフォーム依存な部分を吸収したいと考えている。
	 */
	class ImageList : public AbstractCapture {
	public:


		/** コンストラクタ
		 *
		 * 読み込み数のカウンタが初期化される。
		 */
		ImageList()
			:_counter(0),_open(false),camera_num(1)
		{}
		/** デストラクタ
		 */
		virtual ~ImageList(){}
		/// 読み込まれた画像を取得する。
		/// @param return 読み込みの成否（next()で読み込めていれば問題ないはず・・・？）
		virtual bool get(
				cv::Mat &image, /**< [out] 読み込まれた画像が返る */
				int channel = 0 /**< [in] 読み込まれた画像のチャンネル数（無効になっているはず） */
				);
		/// ディレクトリ、リストファイルの次の画像を読み込む。
		/// @param return 読み込みの成否
		virtual bool next();
		/// 現在の読み込んだ画像数を返す
		/// @param return 読み込んだ画像数
		virtual int currentNum();
		/// オプションを指定する（無効）
		/// @param return オプション設定の成否
		virtual bool setOption( const std::string &name, const std::string &val ){return false;}
		/// ディレクトリ、リストファイルのオープン
		/// @param return ディレクトリ、リストファイルのオープンの成否
		virtual bool open(const std::string &filename);
		/// ディレクトリ、リストファイルのオープンの成否
		/// @param return ディレクトリ、リストファイルのオープンの成否
		virtual bool isOpen();
		/// ディレクトリ、リストファイルの情報をクリアする。
		virtual void release();
		/// 連続画像の取得先
		/// @param return "ImageList" が返る
		virtual std::string getSourceName();
		/// オープンしたファイル名。主にディレクトリからの読み込み用
		/// @param return 現在読み込んでいる画像ファイル名
		virtual std::string getFileName();
		/// カメラからの取得情報を得る。無効
		virtual Info getInfo(){ return inf;}

		//マルチカメラ用
		virtual void setMultiCameraNum( int num );
		virtual int getMultiCameraNum();
		virtual bool enableSyncMultiCamera(){ return false;};
		virtual bool initCvMat(){return true;}
	private:
		/** 画像ファイルのリスト
		 *
		 * open()の実行時に読み込まれる画像ファイルのリスト。
		 * 名前順にソートされる。
		 */
		std::vector<std::string> _image_list;
		/// ディレクトリ名、またはリストファイル名
		std::string _filename;
		/// 読み込んだファイル数の数
		int _counter;
		/// ディレクトリ、またはリストファイルのオープン成否
		bool _open;

		/// 複数のImageList利用時にいくつを同時に利用するか
		int camera_num;
		/// 読み込んだ画像情報
		Info inf;
		/// ディレクトリの読み込み
		size_t _readDir2List(const std::string &name );
		/// リストファイルの読み込み
		size_t _readFile2List( const std::string &filename );
	};

#ifndef Disable_FlyCapture
	/** FlyCapture2のラッパークラス
	 *
	 * PointGreyResearchのカメラを利用するためのクラス。
	 * マルチカメラ環境下における同期もサポートしている。
	 *
	 * 画像データはcv::Matで保持しており、他のCaptureクラスと
	 * データの互換性は保っている。
	 */
	class FlyCaptureCapture : public AbstractCapture {
		public:
			/** コンストラクタ
			 *
			 * カメラの設定などを行う。
			 */
			FlyCaptureCapture();
			/// デストラクタ
			virtual ~FlyCaptureCapture();
			/// 画像を取得する
			virtual bool get(
					cv::Mat &image,/**< [out] カメラから取得された画像 */
					int channel=0 /**< [in] 取得する画像のチャンネル数（無効） */
					);
			/** 次の画像の読み込み
			 *
			 * カメラからフレームを取得し、保持する。
			 * この時画像データはFlyCapture2::Image から cv::Mat へと変換され、
			 * タイムスタンプなどの画像情報も保持される。
			 */
			virtual bool next();
			/** フレーム取得数
			 *
			 * カメラに接続してからのフレーム取得数を取得する。
			 *
			 * @param return フレーム取得数
			 */
			virtual int currentNum();
			/** カメラのプロパティを設定する
			 * 実装がかなり中途半端。今のところ利用していない。
			 * プロパティの設定はsePropertyを参照
			 * @see setProperty
			 * @param return オプションの設定の成否
			 */
			virtual bool setOption( const std::string &name, const std::string &val );
			/** カメラのオープン（接続）を行う
			 *
			 * カメラへの接続を行うと同時に、マルチカメラ用の設定も行なっている。
			 * なので同期を取ったマルチカメラを有効にする時は、
			 * open が実行される前にenableSyncMultiCamera を実行する必要がある。
			 *
			 * @param return カメラへの接続の成否
			 */
			virtual bool open(const std::string &filename);
			/** カメラに接続されているかいないか
			 * @param return カメラに接続されているかいないか
			 */
			virtual bool isOpen();
			/** カメラのリリース（開放）を行う。 
			 * このメソッドにより、カメラが開放される。
			 * 関連するパラメータなどもリセットされる。
			 */
			virtual void release();

			virtual std::string getSourceName();
			virtual std::string getFileName();
			/** 画像情報の取得
			 * カメラのインデックス番号、シリアルナンバー、画像取得時の
			 * タイムスタンプを保持したInfoを取得する。
			 * カメラやFlyCapture2::Imageからの情報取得はnext()で行われる。
			 * @param return 画像情報
			 */
			virtual Info getInfo();
			/** カメラのプロパティを編集する
			 * FlyCapture2で用意されているプロパティ設定GUIを起動する。
			 * 起動時に同じスレッドで動作しているプログラムは一時的に停止する。
			 * GUIでの設定は動的に反映される。
			 */
			virtual void editProperty();
			/** カメラのプロパティを設定する
			 * getPropertyから取得したoptionsを使って、複数のカメラのプロパティを
			 * 同じにする目的で実装した。
			 * 一応オプション名を指定して個別に設定も可能だが、editPropertyを
			 * 実行することで起動するプロパティ設定GUIを利用することをおすすめする。
			 */
			virtual void setProperty(
					const std::map<std::string,std::string> &options /**< [in] カメラのプロパティ */
					);
			/** カメラのプロパティを取得する
			 * このメソッドで取得したoptionsを使って、複数カメラのプロパティを
			 * 同じに設定する目的で実装した。
			 * オプションは指定はできず、一括で取得される。
			 */
			virtual void getProperty(
					std::map<std::string,std::string> &options /**< [out] カメラのプロパティ */
					);
			
			virtual bool readProperty(cv::FileStorage& fs, std::map<std::string,std::string>& options);


			//マルチカメラ用
			/** マルチカメラに利用するカメラ数を指定する
			 * 指定するカメラ数は認識されているカメラ数と一致している必要がある。
			 */
			virtual void setMultiCameraNum(
					int num /**< [in] カメラ数 */
					);
			/** 設定されたマルチカメラの数
			 * @param return 設定されたカメラ数
			 */
			virtual int getMultiCameraNum();
			/** マルチカメラの同期を有効にする
			 * マルチカメラ利用時に各カメラの撮影タイミングの同期を有効にする
			 * @param return 同期設定の成否
			 */
			virtual bool enableSyncMultiCamera();
			/** クラス内で画像を保持するためのcv::Matを初期化する
			 * openの実行後にこれを実行することで画像保持用のcv::Matを
			 * 初期化する。
			 * @param return 初期化の成否
			 */
			virtual bool initCvMat();

			//カメラそれぞれのパラメータ
			FlyCapture2::Camera *camera; /**< カメラ */
			//FlyCapture2::Image orig_frame, buffer_frame, bgr_frame;
			cv::Mat cv_frame; /**< カメラから取得した画像が保持される */
			
			FlyCapture2::Image orig_frame; /**< カメラから取得された画像をそのまま保持する */
			FlyCapture2::Image bgr_frame; /**< カメラから取得された画像をRGB形式からBGR形式に変換したものを保持する */
			unsigned int data_size; /**< orig_frameのデータサイズ */
			Info inf; /**< カメラや画像から取得された情報を保持する */
			int counter; /**< カメラ接続時からの画像取得数を保持する */

			//カメラの設定用パラメータ
			std::string video_mode_str; /**< 未使用 */
			std::string resolution_str; /**< 未使用 */
			FlyCapture2::VideoMode video_mode; /**< 未使用 */
			FlyCapture2::FrameRate frame_rate; /**< 未使用 */

			struct PropertyName{
				const std::string abs;
				const std::string auto_manual;
				const std::string value;
				const std::string red;
				const std::string blue;
				PropertyName( const std::string& abs, const std::string& auto_manual,
					const std::string& value,
					const std::string& red = (std::string)"",
					const std::string& blue = (std::string)"" )
					:abs(abs), auto_manual(auto_manual), value(value), red(red), blue(blue)
				{}
			};

			//カメラ設定用のパラメータ
			//明るさ
			static const PropertyName prop_brightness;
			//露出
			static const PropertyName prop_exposure;
			//シャープネス
			static const PropertyName prop_sharpness;
			//色合い
			static const PropertyName prop_hue;
			//彩度
			static const PropertyName prop_saturation;
			//ガンマ
			static const PropertyName prop_gamma;
			//シャッタースピード
			static const PropertyName prop_shutter;
			//ゲイン
			static const PropertyName prop_gain;
			//ホワイトバランス
			static const PropertyName prop_white;

			///パラメータの値
			static const std::string prop_parameter_on;
			static const std::string prop_parameter_off;

			//マルチカメラ用クラスパラメータ＆クラスメソッド
			//クラス間で共有する必要がある
			static unsigned int open_camera_num; /**< マルチカメラ数 */
			static unsigned int connected_camera_num; /**< FlyCapture2で検出されたカメラ数 */
			static unsigned int opened_camera_num; /**< 接続されたカメラ数 */
			static FlyCapture2::BusManager *busMgr; /**< カメラ接続に利用する。インスタンス間で共有されている必要がある。 */
			static FlyCapture2::Camera **multicamera; /**< インスタンス間でCameraを共有するために利用。マルチカメラ用 */
			static bool enable_synchronized_multicamera; /**< マルチカメラ用 */

			static int busMgr_ref_count; /**< busMgrの参照カウンタ。 */
	};
#endif

	AbstractCapture* getCapture(std::string &type);

	/** AbstractCaptureクラスを継承したクラスを隠蔽するために作成したクラス
	 * 将来的にはなくしたほうが良い。このクラスの性でプログラムに柔軟性が
	 * 失われてしまっている。
	 */
	class Capture {
		public:
			Capture():_cap(0){}
			Capture( std::string &type );
			Capture( int type );
			~Capture();
			void setType( std::string& type );
			bool get( cv::Mat &image, int channel = 0);
			bool next();
			int currentNum();
			bool setOption(const std::string &name, const std::string &val);

			bool open(const std::string &filename);

			bool isOpen();
			//開いたファイルやカメラを開放する。
			void release();

			std::string getSourceName();
			Info getInfo();

			//利用するキャプチャに依存した設定
			void editProperty();
			void getProperty( std::map<std::string, std::string> &options );
			void setProperty( const std::map<std::string, std::string> &options );

			
			/** キャプチャクラスの設定書き出し
			 */
			static bool saveProperty( const std::string& fname, const std::map<std::string,std::string> &options );
			///キャプチャクラスの設定読み込み＆反映
			bool readProperty( const std::string& fname, std::map<std::string,std::string> &options );

			//マルチカメラ用
			void setMultiCameraNum( int num );
			int getMultiCameraNum();
			bool enableSyncMultiCamera();
			bool initCvMat();

		private:
			AbstractCapture *_cap;
			AbstractCapture* _getSource(std::string &type);
	};

	/** 処理内容を記述するための抽象クラス
	 * このクラスを継承して実装することでImageSequenceクラスでの処理を
	 * 行う。
	 */
	class ImageProcessorInterface {
		public:
			virtual ~ImageProcessorInterface(){};
			/** 処理の初期化
			 */
			virtual void onInit( cv::Mat &frame) = 0;
			//処理内容を記述
			//引数より与えられた画像データは書き換え不可
			virtual bool onProcess(
					const cv::Mat &image,/**< カメラから取得された画像 */
					const int count /**< 取得された画像数 */
					) = 0;
			/** 終了時の処理
			 */
			virtual void onFinish() = 0;

			/** マルチカメラ時の処理の初期化
			 */
			virtual void onInitMulti(
					Capture *captures, /**< キャプチャー */
					cv::Mat *image, /**< キャプチャーから取得された画像 */
					Info *infs, /**< 情報 */
					int n /**< 取得された画像枚数 */
					){}
			/** マルチカメラ時の処理
			 */
			virtual bool onProcessMulti(
					Capture *captures, /**< キャプチャー */
					cv::Mat *image, /**< キャプチャーから取得された画像 */
					Info *infs, /**< 情報 */
					int n /**< 取得された画像枚数 */
					){return false;}
			/** マルチカメラ時の終了時の処理
			 */
			virtual void onFinishMulti(){}

			/** クラス名を指定する
			 * @param return クラス名
			 */
			virtual std::string getClassName() = 0;
			//Captureごとの特殊な設定を行う
			virtual void getOptions( std::map<std::string, std::string> &options ) = 0;

	};

	class ImageSequence {
		public:
			ImageSequence();
			~ImageSequence();
			void setCaptureType(std::string type);

			enum {
				ROTATE_LEFT,
				ROTATE_RIGHT,
				ROTATE_NONE
			};
			void setRotate( int rotate );

			void setFrameSize( cv::Size &size );
			void setFrameSize( int width, int height );
			void setInterval( int interval );
			void setWindowName( std::string &name );
			void showProgress( bool show );
			//処理内容を記述したImageProcessorInterfaceをぶち込む。
			//複数追加可。
			//外部で確保したImageProcessorInterfaceは外部で開放すること
			void setImageProcessor( ImageProcessorInterface &processor );

			//シングルカメラ用
			bool run();

			//マルチカメラ用
			void setFrameMatrixSize( int rows, int cols );
			void enableSyncMultiCamera();
			void setMultiCameraNum( int num );
			bool runMulti();
			Info getCaptureInfo(int i);

			int getOriginalWidth(){ return orig_image_width; }
			int getOriginalHeight(){ return orig_image_height; }
		private:
			std::string _cap_type;
			ImageProcessorInterface* _processor;
			cv::Size _size;
			int _interval;
			bool _show_progress;
			bool _use_gui;
			std::string _window_name;

			Capture *captures;
			int captures_num;

			//カメラから取得した画像の回転
			int _rotate;

			//マルチカメラ用
			bool enable_synchronized;

			int orig_image_width;
			int orig_image_height;

			static void rotateLeft( cv::Mat* srcs, cv::Mat*dsts, int n );
			static void rotateRight( cv::Mat* srcs, cv::Mat*dsts, int n );
			static void rotateNone( cv::Mat* srcs, cv::Mat*dsts, int n );
	};
}



#endif
