#ifndef PAPERSHEETPROCESSOR_HPP
#define PAPERSHEETPROCESSOR_HPP

#include <algorithm>
#include <opencv2/opencv.hpp>

class PaperSheetProcessor
{
private:
    cv::Point2f *maintainTransformationPoints(cv::Point2f *srcTransformPoints, cv::Size imageSize);

    int getMatMidVal(const cv::Mat& img);

    void _AdaptiveFindThreshold(const cv::Mat* dx, const cv::Mat* dy, double* low, double* high);
public:

    void midMatThreshold(const cv::Mat& img, int& threshold1, int& threshold2, float sigma);

    cv::Mat *processImage(std::string filename);

    void AdaptiveFindThreshold(const cv::Mat& src, double* low, double* high, int aperture_size = 3);
};

#endif