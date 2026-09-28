#include <cstdio>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/core/version.hpp>
// OpenCV 5 keeps the legacy umbrella header: it pulls in the three modules that
// split out of calib3d (geometry, calib, stereo). Including it here verifies
// that the installed include/opencv5 tree still serves the compatibility path.
#include <opencv2/calib3d.hpp>

int main()
{
    std::printf("OpenCV: %s\n", CV_VERSION);
    if (CV_VERSION_MAJOR != 5) {
        std::printf("expected OpenCV 5.x, got %s\n", CV_VERSION);
        return 1;
    }

    // Link check against opencv_core.
    const cv::Mat eye = cv::Mat::eye(3, 3, CV_32F);
    const cv::Scalar total = cv::sum(eye);
    std::printf("cv::Mat::eye(3,3) sum = %.3f\n", total[0]);

    // Link check against the 5.0 calib3d split: findHomography now lives in the
    // geometry module, so this only links if the new module layout installed.
    const std::vector<cv::Point2f> src = {{0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}, {0.f, 1.f}};
    const std::vector<cv::Point2f> dst = {{0.f, 0.f}, {2.f, 0.f}, {2.f, 2.f}, {0.f, 2.f}};
    const cv::Mat homography = cv::findHomography(src, dst);
    if (homography.empty() || homography.rows != 3) {
        std::printf("findHomography failed\n");
        return 2;
    }
    std::printf("findHomography H(0,0) = %.3f\n", homography.at<double>(0, 0));

    std::printf("opencv5-test OK\n");
    return 0;
}
