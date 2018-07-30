
static void HoughLinesModified(const Mat& img, const Mat& dx, const Mat& dy, float rho, float theta,
	int threshold, std::vector<Vec2f>& lines, int linesMax,
	double min_theta, double max_theta)    // theta ´ÜÀ§´Â rad
{


	int i, j;
	float irho = 1 / rho;

	CV_Assert(img.type() == CV_8UC1);

	const uchar* image = img.ptr();
	int step = (int)img.step;
	int width = img.cols;
	int height = img.rows;

	if (max_theta < min_theta) {
		CV_Error(CV_StsBadArg, "max_theta must be greater than min_theta");
	}
	int numangle = cvRound((max_theta - min_theta) / theta);


	//
	int dtheta = (int)(10 * CV_PI / 180) / theta;
	//


	int numrho = cvRound(((width + height) * 2 + 1) / rho);

#if defined HAVE_IPP && !defined(HAVE_IPP_ICV_ONLY) && IPP_VERSION_X100 >= 810 && IPP_DISABLE_BLOCK
	CV_IPP_CHECK()
	{
		IppiSize srcSize = { width, height };
		IppPointPolar delta = { rho, theta };
		IppPointPolar dstRoi[2] = { { (Ipp32f)-(width + height), (Ipp32f)min_theta },{ (Ipp32f)(width + height), (Ipp32f)max_theta } };
		int bufferSize;
		int nz = countNonZero(img);
		int ipp_linesMax = std::min(linesMax, nz*numangle / threshold);
		int linesCount = 0;
		lines.resize(ipp_linesMax);
		IppStatus ok = ippiHoughLineGetSize_8u_C1R(srcSize, delta, ipp_linesMax, &bufferSize);
		Ipp8u* buffer = ippsMalloc_8u(bufferSize);
		if (ok >= 0) ok = ippiHoughLine_Region_8u32f_C1R(image, step, srcSize, (IppPointPolar*)&lines[0], dstRoi, ipp_linesMax, &linesCount, delta, threshold, buffer);
		ippsFree(buffer);
		if (ok >= 0)
		{
			lines.resize(linesCount);
			CV_IMPL_ADD(CV_IMPL_IPP);
			return;
		}
		lines.clear();
		setIppErrorStatus();
	}
#endif

	AutoBuffer<int> _accum((numangle + 2) * (numrho + 2));
	std::vector<int> _sort_buf;
	AutoBuffer<float> _tabSin(numangle);
	AutoBuffer<float> _tabCos(numangle);
	int *accum = _accum;
	float *tabSin = _tabSin, *tabCos = _tabCos;


	memset(accum, 0, sizeof(accum[0]) * (numangle + 2) * (numrho + 2));

	float ang = static_cast<float>(min_theta);
	for (int n = 0; n < numangle; ang += theta, n++)
	{
		tabSin[n] = (float)(sin((double)ang) * irho);
		tabCos[n] = (float)(cos((double)ang) * irho);
	}

	// stage 1. fill accumulator
	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
		{
			if (image[i * step + j] != 0)
			{
				double cent_ang = atan2((double)(dy.at<float>(i, j)), (double)(dx.at<float>(i, j)));
				int cent_cnt = (cent_ang - min_theta) / theta;
				for (int n = cent_cnt - dtheta; n < cent_cnt + dtheta; n++)
				{
					int r = cvRound(j * tabCos[n] + i * tabSin[n]);
					r += (numrho - 1) / 2;
					accum[(n + 1) * (numrho + 2) + r + 1]++;
				}
			}
		}

	// stage 2. find local maximums
	for (int r = 0; r < numrho; r++)
		for (int n = 0; n < numangle; n++)
		{
			int base = (n + 1) * (numrho + 2) + r + 1;
			if (accum[base] > threshold &&
				accum[base] > accum[base - 1] && accum[base] >= accum[base + 1] &&
				accum[base] > accum[base - numrho - 2] && accum[base] >= accum[base + numrho + 2])
				_sort_buf.push_back(base);
		}

	// stage 3. sort the detected lines by accumulator value
	std::sort(_sort_buf.begin(), _sort_buf.end(), hough_cmp_gt(accum));

	// stage 4. store the first min(total,linesMax) lines to the output buffer
	linesMax = std::min(linesMax, (int)_sort_buf.size());
	double scale = 1. / (numrho + 2);
	for (i = 0; i < linesMax; i++)
	{
		LinePolar line;
		int idx = _sort_buf[i];
		int n = cvFloor(idx*scale) - 1;
		int r = idx - (n + 1)*(numrho + 2) - 1;
		line.rho = (r - (numrho - 1)*0.5f) * rho;
		line.angle = static_cast<float>(min_theta) + n * theta;
		lines.push_back(Vec2f(line.rho, line.angle));
	}
}
