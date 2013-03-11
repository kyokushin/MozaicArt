#ifndef MozaicArt_h
#define MozaicArt_h

#include <opencv/cv.h>
#include <vector>

/*
*** �������j ***
�܂��͂����̍��������������B
�������@�Ƃ��ẮA�I���W�i���摜�̊e�p�b�`��ێ��A
�e�p�b�`�ɓK�p����e�摜�̏k���摜���ێ��B
�i���\�����ɂ́A�K�p�ς݃t���O���Q�Ƃ��Đi���摜�𖈉񐶐�����B
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
			MOZAIC_TYPE_PATCH_SUB,//��������
			MOZAIC_TYPE_PATCH_AVERAGE,//�p�b�`�̕��ϒl�����Z�������
			MOZAIC_TYPE_PATCH_SHAPE,//�p�b�`�A�K�p�摜���ɓ�l�����ATF�Ŕ��肷�����
		};

		MozaicArt( const cv::Mat& orig_image, const cv::Size& split_size, int mozaic_type = MOZAIC_TYPE_PATCH_SUB );

		MozaicArt( const cv::Mat& orig_image, const int split_num, int mozaic_type = MOZAIC_TYPE_PATCH_SUB );

		//���͉摜���ǂ̈ʒu���Ԃ�
		//�摜���̍��W�ł͂Ȃ��A���������p�b�`�̈ʒu��Ԃ�
		bool calcPatchPosition( const cv::Mat& src, cv::Point& position );
		bool calcPatchPosition( const cv::Mat& src ){
			cv::Point pos;
			return calcPatchPosition( src, pos );
		}

		//�w�肵���v�Z�ς݃p�b�`���N���A����B
		//�߂�l�̓N���A�ɐ����������ۂ�
		bool clearPatchPosition( const cv::Point& pos );

		//�w�肵���ʒu�̃p�b�`���v�Z�ς݂��ۂ�
		bool isCalcurated( const cv::Point& pos );
		bool isCalcurated( int x, int y );

		//���ׂẴp�b�`�ɑ΂��Čv�Z�ς݂��ۂ�
		bool isComplete();

		//�p�b�`�̉摜��Ԃ�
		const cv::Mat& getPatchImage( const cv::Point& pos );

		//���U�C�N�K�p��̉摜��Ԃ�
		const cv::Mat& getProgressImage();

		long long int diffVal( const cv::Mat& src, const cv::Mat& patch );


		//****************************
		//*** �f�o�b�O�p���\�b�h�Q ***
		//****************************

		void debug_showAllPatches();

	private:

		cv::Mat _orig_image; //���U�C�N��ɗ��p���錳�摜
		cv::Size _split_size;
		
		cv::Size _patch_size;//�p�b�`��̃T�C�Y
		std::vector<MozaicPatch> _patches;

		//���U�C�N��̕\���p
		cv::Mat _progress_image;

		int _total_calc_times;
		int _show_counter;

		int _proc_mozaic_type_id;//���U�C�N�A�[�g�̌v�Z����

		void _init();


		//************************
		//*** �������邩�Y�ݒ� ***
		//************************

		//�v�Z�ς݂̃p�b�`���㏑���\�ɂ��邩�ۂ�
		//�������������ꍇ�A�㏑������鑤��
		//�ǂ��̃p�b�`�Ɏ����čs���Ȃ���΂Ȃ�Ȃ������Čv�Z����K�v������B
		bool enableOverWritePatch( bool flag = true);

		//�K�p����摜�̉�]��
		bool enableRotate( bool flag = true );

		//�p�b�`�̌`��
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
