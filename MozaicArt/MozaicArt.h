#ifndef MozaicArt_h
#define MozaicArt_h

#include <opencv/cv.h>
#include <vector>

/*
*** 実装方針 ***
まずはただの差分方式を実装。
実装方法としては、オリジナル画像の各パッチを保持、
各パッチに適用する各画像の縮小画像も保持。
進捗表示時には、適用済みフラグを参照して進捗画像を毎回生成する。
*/
namespace ys {

	struct MozaicPatch {
		cv::Mat orig_patch;
		cv::Mat apply_patch;
		int calculated;
		long long int calc_val;
		MozaicPatch():calculated(-1),calc_val(0){}
	};

	class MozaicArt {
	public:

		enum {
			MOZAIC_TYPE_PATCH_SUB,//差分方式
			MOZAIC_TYPE_PATCH_AVERAGE,//パッチの平均値を加算する方式
			MOZAIC_TYPE_PATCH_SHAPE,//パッチ、適用画像共に二値化し、TFで判定する方式
		};

		MozaicArt( const cv::Mat& orig_image, const cv::Size& split_size, int mozaic_type = MOZAIC_TYPE_PATCH_SUB );

		MozaicArt( const cv::Mat& orig_image, const int split_num, int mozaic_type = MOZAIC_TYPE_PATCH_SUB );

		//入力画像がどの位置か返す
		//画像中の座標ではなく、分割したパッチの位置を返す
		bool calcPatchPosition( const cv::Mat& src, cv::Point& position );
		bool calcPatchPosition( const cv::Mat& src ){
			cv::Point pos;
			return calcPatchPosition( src, pos );
		}

		//指定した計算済みパッチをクリアする。
		//戻り値はクリアに成功したか否か
		bool clearPatchPosition( const cv::Point& pos );

		//指定した位置のパッチが計算済みか否か
		bool isCalcurated( const cv::Point& pos );
		bool isCalcurated( int x, int y );

		//すべてのパッチに対して計算済みか否か
		bool isComplete();

		//パッチの画像を返す
		const cv::Mat& getPatchImage( const cv::Point& pos );

		//モザイク適用後の画像を返す
		const cv::Mat& getProgressImage();

		long long int diffVal( const cv::Mat& src, const cv::Mat& patch );


		//****************************
		//*** デバッグ用メソッド群 ***
		//****************************

		void debug_showAllPatches();

	private:

		cv::Mat _orig_image; //モザイク画に利用する元画像
		cv::Size _split_size;
		
		cv::Size _patch_size;//パッチ一つのサイズ
		std::vector<MozaicPatch> _patches;

		//モザイク画の表示用
		cv::Mat _progress_image;

		int _total_calc_times;
		int _show_counter;

		int _proc_mozaic_type_id;//モザイクアートの計算方式

		void _init();


		//************************
		//*** 実装するか悩み中 ***
		//************************

		//計算済みのパッチを上書き可能にするか否か
		//これを実装する場合、上書きされる側を
		//どこのパッチに持って行かなければならないかを再計算する必要がある。
		bool enableOverWritePatch( bool flag = true);

		//適用する画像の回転可否
		bool enableRotate( bool flag = true );

		//パッチの形状
		enum {
			PATCH_SHAPE_RECTANGLE,
			PATCH_SHAPE_TRIANGLE,
			PATCH_SHAPE_HEXAGON
		};
		int _patch_shape_id;
		void setPatchShape( int shape_id );
	};

};
#endif
