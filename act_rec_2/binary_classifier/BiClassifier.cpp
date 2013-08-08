/**
 * Author: Rajesh
 * Description: an implementation of BiClassifier.hpp.
 */

#include <stdio.h>
#include <stdlib.h>

#include "BiClassifier.hpp"

BiClassifier::BiClassifier(const char* fn)
{
  cv::Mat trainData, labels;

  loadData(fn, trainData, labels);

  CvSVMParams params(CvSVM::C_SVC, CvSVM::LINEAR, 0, 1, 0, 1, 0, 0, 0,
		     cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 1000,
				    FLT_EPSILON));

  svm.train(trainData, labels, cv::Mat(), cv::Mat(), params);
  //  boost.train(trainData, CV_ROW_SAMPLE, labels);
}

int BiClassifier::predict(const cv::Mat &sample)
{
  return (int)svm.predict(sample);
}

void BiClassifier::loadData(const char* fn, cv::Mat &trainData,
			    cv::Mat &labels)
{
  FILE* fh = fopen(fn, "r");

  if (fh == NULL){
    printf("Could not open file %s\n", fn);
    exit(EXIT_FAILURE);
  }

  int label;
  char paramFile[256];

  while (fscanf(fh, "%d %s", &label, paramFile) != EOF){
    cv::FileStorage fs(paramFile, cv::FileStorage::READ);
    cv::Mat descriptors;

    fs["descriptors"] >> descriptors;

    //    for (int i = 0; i < descriptors.rows; ++i){
    //      for (int j = 0; j < descriptors.cols; ++j){
    //	if (descriptors.at<float>(i, j) != 0.0f)
    //	  descriptors.at<float>(i, j) = 255.0f;
    //      }
    //    }

    //    printf("%d %d\n", descriptors.rows, descriptors.cols);
    //    if (descriptors.type() == CV_32FC1)
    //      printf("OK\n");

    static bool onceThrough = false;

    if (onceThrough && descriptors.cols != trainData.cols){
      printf("Inconsistent training vector dimensions\n");
      exit(EXIT_FAILURE);
    }
    else if (!onceThrough)
      onceThrough = true;

    trainData.push_back(descriptors);

    cv::Mat tmp;

    if (label == 0)
      tmp = cv::Mat::zeros(descriptors.rows, 1, CV_32SC1);
    else
      tmp = cv::Mat::ones(descriptors.rows, 1, CV_32SC1);

    labels.push_back(tmp);

    fs.release();
  }

  fclose(fh);
}
