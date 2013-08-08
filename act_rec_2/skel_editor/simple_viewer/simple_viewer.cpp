#include <stdio.h>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

int main(int argc, char** argv)
{
  if (argc != 2)
    return -1;

  FILE* fh = fopen(argv[1], "r");

  if (fh == NULL){
    printf("Could not open file %s\n", argv[1]);
    return -1;
  }

  int label;
  char paramFile[256];

  while (fscanf(fh, "%d %s", &label, paramFile) != EOF){
    cv::FileStorage fs(paramFile, cv::FileStorage::READ);
    cv::Mat descriptors;

    fs["descriptors"] >> descriptors;

    for (int i = 0; i < descriptors.rows; ++i){
      cv::Mat d = descriptors.row(i);

      d = d.reshape(0, 64);

      cv::Mat test;
      d.convertTo(test, CV_8UC1);
      cv::imshow("test", test);
      cv::waitKey(0);
    }
  }

  return 0;
}
