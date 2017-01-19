#pragma once
#ifndef _STEREOVISION_H_
#define _STEREOVISION_H_

#include <opencv2/opencv.hpp>
#include <opencv2/cudastereo.hpp>
//#include <opencv2/cuda

using namespace std;
using namespace cv;

class StereoVision {
private:
    Size imageSize;                    // ??????????????

    Rect validROIL, validROIR;        // ??????????????????????????validROI????????????????

    Mat Rl, Rr, Pl, Pr, Q;            // ??????????R????????P ????????Q\
    cuda::GpuMat gpu_Rl,gpu_Rr,gpu_Pl,gpu_Pr,gpu_Q;

    Mat disp, disp8;

    Mat XYZ;                        // ???????


    cuda::GpuMat gpu_disp, gpu_disp8;
    cuda::GpuMat gpu_l, gpu_r;
    cuda::GpuMat gpu_XYZ;

// 	Mat cameraMatrixL, distCoeffL;	// ?????????????????????
// 	Mat cameraMatrixR, distCoeffR;	// ?????????????????????
//	Mat T, R;						// T-???????, R-???????

    bool first = true;                // ??????????

    Ptr<cuda::StereoBM> bm;                // ???????????BM?????StereoBM::create()????????bm->compute()???????
//    Ptr<StereoBM> bm;
public:
    Mat mapLx, mapLy, mapRx, mapRy;    // ????

//    cuda::GpuMat gpu_mapLx,gpu_mapLy,gpu_mapRx,gpu_mapRy;

    /* @brief ????????
     */
    StereoVision();

    /* @brief ??????
     * @param cameraMatrixL_, cameraMatrixR_ ??????????????
     * @param distCoeffL_, distCoeffR_ ?????????????
     * @param T_ ??????????????
     * @param R_ ??????????????
     * @param imageWidth_, imageHeight_ ?????
     */
    StereoVision(Mat cameraMatrixL_, Mat distCoeffL_, Mat cameraMatrixR_, Mat distCoeffR_, Mat T_, Mat R_,
                 int imageWidth_, int imageHeight_);

    /* @brief ????????
     */
    ~StereoVision();

    /* @brief ????????????????????get?????????????
     * @param grayImageL_, grayImageR_ ????????CV_8UC1
     * @param blockSize_ SAD????????5~21??????
     * @param uniquenessRatio_ ??????????????????????????????16
     * @param numDisparities_ uniquenessRatio?????????????????50
     */
    void stereoMatch(Mat grayImageL_, Mat grayImageR_);

    //void stereoMatch(Mat grayImageL_, Mat grayImageR_);

    /* @brief ????????p_??????????????XYZ_
     */
    void getXYZ(Point p_, Vec3f &XYZ_);

    /* @brief ????????????
     */
    //void getRectifyImage(Mat &rectifyImageL_, Mat &rectifyImageR_);

    /* @brief ????????????
     */
    void getDisparityImage(Mat &disp_);
};

StereoVision::StereoVision() {

}

StereoVision::StereoVision(Mat cameraMatrixL_, Mat distCoeffL_,
                           Mat cameraMatrixR_, Mat distCoeffR_,
                           Mat T_, Mat R_, int imageWidth_, int imageHeight_) {
    imageSize = Size(imageWidth_, imageHeight_);
// 	cameraMatrixL = cameraMatrixL_;
// 	cameraMatrixR = cameraMatrixR_;
// 	distCoeffL = distCoeffL_;
// 	distCoeffR = distCoeffR_;
// 	T = T_;
// 	R = R_;
//    cuda::GpuMat gpu_cameraMatrixL_,gpu_distCoeffL_,gpu_cameraMatriXR_,gpu_distCoeffR_,

    gpu_disp = cuda::GpuMat(imageSize, CV_16U);

    bm = cuda::createStereoBM();
//	bm = cv::StereoBM::create();

    stereoRectify(cameraMatrixL_, distCoeffL_, cameraMatrixR_, distCoeffR_, imageSize, R_, T_, Rl, Rr, Pl, Pr, Q,
                  CALIB_ZERO_DISPARITY, 0, imageSize, &validROIL, &validROIR);
    initUndistortRectifyMap(cameraMatrixL_, distCoeffL_, Rl, Pl, imageSize, CV_32FC1, mapLx, mapLy);
    initUndistortRectifyMap(cameraMatrixR_, distCoeffR_, Rr, Pr, imageSize, CV_32FC1, mapRx, mapRy);

    bm->setBlockSize(3);    // ?????5~21????
    bm->setROI1(validROIL);
    bm->setROI2(validROIR);
    bm->setPreFilterCap(31);
    bm->setMinDisparity(0);                    // ?????????0, ??????int?
    bm->setNumDisparities(32);    // ???????????????????????????16????
    bm->setTextureThreshold(10);
    bm->setUniquenessRatio(0);// uniquenessRatio?????????
    bm->setSpeckleWindowSize(100);
    bm->setSpeckleRange(32);
    bm->setDisp12MaxDiff(-1);
}

StereoVision::~StereoVision() {

}

void StereoVision::stereoMatch(Mat grayImageL_, Mat grayImageR_) {

    gpu_l.upload(grayImageL_);
    gpu_r.upload(grayImageR_);
    bm->compute(gpu_l, gpu_r, gpu_disp);    // ??????????????????CV_16S??
    gpu_disp.download(disp);
    disp8 = disp;
    reprojectImageTo3D(disp, XYZ, Q, true);
    XYZ = XYZ * 16;
}

void StereoVision::getXYZ(Point p_, Vec3f &XYZ_) {
    XYZ_ = XYZ.at<Vec3f>(p_);
}

/*
void StereoVision::getRectifyImage(Mat &rectifyImageL_, Mat &rectifyImageR_) {
	rectifyImageL_ = rectifyImageL;
	rectifyImageR_ = rectifyImageR;
}
*/
void StereoVision::getDisparityImage(Mat &disp_) {
    disp_ = disp8;
}

#endif