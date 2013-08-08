/**
 * Author: Rajesh
 * Description: a general purpose binary classifier. Use a text file to provide
 *   it the files used for training and the corresponding labels. Each file that
 *   contains training data should be a .yaml file with two parameters:
 *   descriptors is a cv::Mat and each row is a descriptor, and joint_offsets
 *   is a cv::Mat and each three rows represents the skeleton joints of the
 *   corresponding frame (but it is not used for traning). User is responsible
 *   to make sure that all the descriptors are of the same dimension.
 */

#include "opencv2/core/core.hpp"
#include "opencv2/ml/ml.hpp"

#ifndef BICLASSIFIER_HPP
#define BICLASSIFIER_HPP

class BiClassifier
{
public:
  BiClassifier(const char* fn);
  int predict(const cv::Mat &sample);

private:
  void loadData(const char* fn, cv::Mat &trainData, cv::Mat &labels);

  CvSVM svm;
  //  CvBoost boost;
};

#endif // BICLASSIFIER
