#include "PaperSheetProcessor.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui_c.h>

cv::Point2f *PaperSheetProcessor::maintainTransformationPoints(cv::Point2f *srcTransformPoints, cv::Size imageSize)
{
    const int pointsCount = 4;
    cv::Point2f *destTransformPoints = new cv::Point2f[pointsCount];
    for (size_t i = 0; i < pointsCount; i++)
    {
        destTransformPoints[i] = cv::Point2f(srcTransformPoints[i]);
    }

    if (destTransformPoints[0].y < destTransformPoints[1].y &&
        destTransformPoints[1].x < destTransformPoints[2].x &&
        destTransformPoints[2].y > destTransformPoints[3].y &&
        destTransformPoints[3].x > destTransformPoints[0].x)
    {
        destTransformPoints[0] = cv::Point2f(0, 0);
        destTransformPoints[1] = cv::Point2f(0, imageSize.height);
        destTransformPoints[2] = cv::Point2f(imageSize.width, imageSize.height);
        destTransformPoints[3] = cv::Point2f(imageSize.width, 0);
    }
    else
    {
        destTransformPoints[0] = cv::Point2f(imageSize.width, 0);
        destTransformPoints[1] = cv::Point2f(0, 0);
        destTransformPoints[2] = cv::Point2f(0, imageSize.height);
        destTransformPoints[3] = cv::Point2f(imageSize.width, imageSize.height);
    }

    return destTransformPoints;
}


void PaperSheetProcessor::midMatThreshold(const cv::Mat& img, int& threshold1, int& threshold2, float sigma)
{
    int midval = getMatMidVal(img);
    // 计算低阈值
    threshold1 = cv::saturate_cast<uchar>((1.0 - sigma) * midval);
    //计算高阈值
    threshold2 = cv::saturate_cast<uchar>((1.0 + sigma) * midval);
}


//************************************
// Method:    求Mat的中位数
//************************************
int PaperSheetProcessor::getMatMidVal(const cv::Mat& img)
{
    //判断如果不是单通道直接返回128
    if (img.channels() > 1) return 128;
    int rows = img.rows;
    int cols = img.cols;
    //定义数组
    float mathists[256] = { 0 };
    //遍历计算0-255的个数
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int val = img.at<uchar>(row, col);
            mathists[val]++;
        }
    }
    int calcval = rows * cols / 2;
    int tmpsum = 0;
    for (int i = 0; i < 255; ++i) {
        tmpsum += mathists[i];
        if (tmpsum > calcval) {
            return i;
        }
    }
    return 0;
}


void PaperSheetProcessor::_AdaptiveFindThreshold(const cv::Mat* dx, const cv::Mat* dy, double* low, double* high)
{
    CvSize size;
    IplImage* imge = 0;
    int i, j;
    CvHistogram* hist;
    int hist_size = 255;
    float range_0[] = { 0,256 };
    float* ranges[] = { range_0 };
    double PercentOfPixelsNotEdges = 0.7;
    size = cvGetSize(dx);
    imge = cvCreateImage(size, IPL_DEPTH_32F, 1);
    float maxv = 0;
    for (i = 0; i < size.height; i++)
    {
        const short* _dx = (short*)(dx->data + dx->step * i);
        const short* _dy = (short*)(dy->data + dy->step * i);

        float* _image = (float*)(imge->imageData + imge->widthStep * i);
        for (j = 0; j < size.width; j++)
        {
            _image[j] = (float)(abs(_dx[j]) + abs(_dy[j]));
            maxv = maxv < _image[j] ? _image[j] : maxv;
        }
    }

    if (maxv == 0) {
        *high = 0;
        *low = 0;
        cvReleaseImage(&imge);
        return;
    }

    range_0[1] = maxv;
    hist_size = (int)(hist_size > maxv ? maxv : hist_size);
    hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
    cvCalcHist(&imge, hist, 0, nullptr);
    int total = (int)(size.height * size.width * PercentOfPixelsNotEdges);
    float sum = 0;
    int icount = hist->mat.dim[0].size;
    float* h = (float*)cvPtr1D(hist->bins, 0);

    for (i = 0; i < icount; i++)
    {
        sum += h[i];
        if (sum > total)
            break;
    }

    *high = (i + 1) * maxv / hist_size;
    *low = *high * 0.4;
    cvReleaseImage(&imge);
    cvReleaseHist(&hist);
}

cv::Mat* PaperSheetProcessor::processImage(std::string filename)
{
    cv::Mat init_image = cv::imread(filename);
    if (init_image.empty())
    {
        return nullptr;
    }

    cv::Mat image;
    double height = init_image.rows;
    //double height = 800;
    cv::Size imageSize(int(height / init_image.rows * init_image.cols), height);
    if (height == 800)
    {
        cv::resize(init_image, image, imageSize);
    }
    else
    {
        image = init_image;
    }

    cv::Mat blurred_image;
    if (height == 800)
    {
        cv::medianBlur(image, blurred_image, 9);
    }
    else {
        cv::GaussianBlur(image, blurred_image, cv::Size(7, 7), 2, 2);
    }

    cv::Mat gray_image;
    cv::cvtColor(blurred_image, gray_image, cv::COLOR_BGR2GRAY);

    int threshold1, threshold2;
    midMatThreshold(init_image, threshold1, threshold2, 0.3);

    cv::Mat canny_image;
    if (height == 800)
    {
        cv::Canny(gray_image, canny_image, 30, 50, 3);
    }
    else {
        //cv::Canny(gray_image, canny_image, threshold1, threshold2, 3);
        //cv::Canny(gray_image, canny_image, 60, 240, 3);
        cv::Canny(gray_image, canny_image, 30, 50, 3);
        cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(20, 20));
        dilate(canny_image, canny_image, element);
        erode(canny_image, canny_image, element);
    }

    std::vector<std::vector<cv::Point>> contours;
    //cv::findContours(canny_image, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
    cv::findContours(canny_image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    double maxFoundPerimeter = 0;
    std::vector<cv::Point> maxContour;
    for (size_t i = 0; i < contours.size(); i++)
    {
        std::vector<cv::Point> contour = contours[i];
        double perimeter = cv::arcLength(contour, true);
        std::vector<cv::Point> contourApprox;
        cv::approxPolyDP(contour, contourApprox, 0.02 * perimeter, true);

        if (perimeter > maxFoundPerimeter && cv::isContourConvex(contourApprox) && contourApprox.size() == 4)
        {
            maxFoundPerimeter = perimeter;
            maxContour = contourApprox;
        }
    }

    cv::polylines(image, maxContour, true, cv::Scalar(0, 255, 0), 1, 8);
    imwrite("z:\\aaa\\findMaxContour.jpg", image);

    cv::Point2f srcTransformationPoints[4];
    for (size_t i = 0; i < 4; i++)
    {
        srcTransformationPoints[i] = cv::Point2f(maxContour.data()[i]);
    }
    cv::Point2f* destTransformationPoints = maintainTransformationPoints(srcTransformationPoints, imageSize);
    cv::Mat transformationMatrix = cv::getPerspectiveTransform(srcTransformationPoints, destTransformationPoints);
    cv::Mat* finalImage = new cv::Mat;
    cv::warpPerspective(image, *finalImage, transformationMatrix, imageSize);
    delete destTransformationPoints;

    return finalImage;
}

void PaperSheetProcessor::AdaptiveFindThreshold(const cv::Mat& src, double* low, double* high, int aperture_size /*= 3*/)
{
    const int cn = src.channels();
    cv::Mat dx(src.rows, src.cols, CV_16SC(cn));
    cv::Mat dy(src.rows, src.cols, CV_16SC(cn));
    cv::Sobel(src, dx, CV_16S, 1, 0, aperture_size, 1, 0, cv::BORDER_REPLICATE);
    cv::Sobel(src, dy, CV_16S, 0, 1, aperture_size, 1, 0, cv::BORDER_REPLICATE);

    cv::Mat _dx = dx, _dy = dy;

    _AdaptiveFindThreshold(&_dx, &_dy, low, high);
}
