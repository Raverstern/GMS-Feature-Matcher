// GridMatch.cpp : Defines the entry point for the console application.

//#define USE_GPU 

#include "Header.h"
#include "gms_matcher.h"

// implementation of alg.1 in the paper
void GmsMatch(Mat &img1, Mat &img2);

void runImagePair(){
	Mat img1 = imread("../data/nn_left.jpg");
	Mat img2 = imread("../data/nn_right.jpg");

	imresize(img1, 480);
	imresize(img2, 480);

	GmsMatch(img1, img2);
}


int main()
{
#ifdef USE_GPU
	int flag = cuda::getCudaEnabledDeviceCount();
	if (flag != 0){ cuda::setDevice(0); }
#endif // USE_GPU

	runImagePair();

	return 0;
}


void GmsMatch(Mat &img1, Mat &img2){
	vector<KeyPoint> kp1, kp2;
	Mat d1, d2;
	vector<DMatch> matches_all, matches_gms;

	Ptr<ORB> orb = ORB::create(10000);
	orb->setFastThreshold(0);
	
	if(img1.rows * img1.cols > 480 * 640 ){
		orb->setMaxFeatures(100000);
		orb->setFastThreshold(5);
	}
	
    // 1. Detect feature points and calculate their descriptors
	orb->detectAndCompute(img1, Mat(), kp1, d1);
	orb->detectAndCompute(img2, Mat(), kp2, d2);

    // 2. Find nearest neighbour between features in two images
#ifdef USE_GPU
    std::cout << "Using GPU for matching." << std::endl;
	GpuMat gd1(d1), gd2(d2);
	Ptr<cuda::DescriptorMatcher> matcher = cv::cuda::DescriptorMatcher::createBFMatcher(NORM_HAMMING);
	matcher->match(gd1, gd2, matches_all);
#else
	BFMatcher matcher(NORM_HAMMING);
	matcher.match(d1, d2, matches_all);
#endif

	// GMS filter
	int num_inliers = 0;
	std::vector<bool> vbInliers;
    // 3. Divide images by G grids respectively
	gms_matcher gms(kp1,img1.size(), kp2,img2.size(), matches_all);
    // From line 4.
    num_inliers = gms.GetInlierMask(vbInliers, false, false);

	cout << "Get total " << num_inliers << " matches." << endl;

	// draw matches
	for (size_t i = 0; i < vbInliers.size(); ++i)
	{
		if (vbInliers[i] == true)
		{
			matches_gms.push_back(matches_all[i]);
		}
	}

	Mat show = DrawInlier(img1, img2, kp1, kp2, matches_gms, 2);
	imshow("show", show);
	waitKey();
}


