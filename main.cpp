#include <cstdio>

#ifdef TESTCAP_WITH_OPENCV
#include <opencv2/core.hpp>
#include <opencv2/core/version.hpp>
#endif

int main(int, char**)
{
#ifdef TESTCAP_WITH_OPENCV
    std::printf("OpenCV: %s\n", CV_VERSION);

    // Touch real symbols so this is a linkage check and not only a header check.
    cv::Mat eye = cv::Mat::eye(3, 3, CV_32F);
    const cv::Scalar total = cv::sum(eye);
    std::printf("cv::Mat::eye(3,3) sum = %.3f\n", total[0]);
#else
    std::printf("Hello, from testcap! (built without OpenCV)\n");
#endif
    return 0;
}
