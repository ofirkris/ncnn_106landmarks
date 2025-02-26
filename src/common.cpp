#include "common.h"
#include <iostream>

int RatioAnchors(const cv::Rect & anchor,
	const std::vector<float>& ratios, 
	std::vector<cv::Rect>* anchors) {
	anchors->clear();
	cv::Point center = cv::Point(anchor.x + (anchor.width - 1) * 0.5f,
		anchor.y + (anchor.height - 1) * 0.5f);
	float anchor_size = anchor.width * anchor.height;
	for (int i = 0; i < static_cast<int>(ratios.size()); ++i) {
		float ratio = ratios.at(i);
		float anchor_size_ratio = anchor_size / ratio;
		float curr_anchor_width = std::sqrt(anchor_size_ratio);
		float curr_anchor_height = curr_anchor_width * ratio;
		float curr_x = center.x - (curr_anchor_width - 1)* 0.5f;
		float curr_y = center.y - (curr_anchor_height - 1)* 0.5f;

		cv::Rect curr_anchor = cv::Rect(curr_x, curr_y,
			curr_anchor_width - 1, curr_anchor_height - 1);
		anchors->push_back(curr_anchor);
	}
	return 0;
}

int ScaleAnchors(const std::vector<cv::Rect>& ratio_anchors,
	const std::vector<float>& scales, std::vector<cv::Rect>* anchors) {
	anchors->clear();
	for (int i = 0; i < static_cast<int>(ratio_anchors.size()); ++i) {
		cv::Rect anchor = ratio_anchors.at(i);
		cv::Point2f center = cv::Point2f(anchor.x + anchor.width * 0.5f,
			anchor.y + anchor.height * 0.5f);
		std::cout << "center.x: " << center.x << std::endl;
		std::cout << "center.y: " << center.y << std::endl;
		for (int j = 0; j < static_cast<int>(scales.size()); ++j) {
			float scale = scales.at(j);
			float curr_width = scale * (anchor.width + 1);
			float curr_height = scale * (anchor.height + 1);
			float curr_x = center.x - curr_width * 0.5f;
			float curr_y = center.y - curr_height * 0.5f;
			cv::Rect curr_anchor = cv::Rect(curr_x, curr_y,
				curr_width, curr_height);
			anchors->push_back(curr_anchor);
		}
	}

	return 0;
}

int GenerateAnchors(const int & base_size,
	const std::vector<float>& ratios, 
	const std::vector<float> scales,
	std::vector<cv::Rect>* anchors) {
	anchors->clear();
	cv::Rect anchor = cv::Rect(0, 0, base_size, base_size);
	std::cout << "base size: " << anchor.width << std::endl;
	std::vector<cv::Rect> ratio_anchors;
	RatioAnchors(anchor, ratios, &ratio_anchors);
	for (int i = 0; i < static_cast<int>(ratio_anchors.size()); ++i) {
		std::cout << "ratio anchor: " << ratio_anchors[i] << std::endl;
	}
	ScaleAnchors(ratio_anchors, scales, anchors);
	std::cout << "anchor size: " << anchors->size() << std::endl;
	for (int i = 0; i < static_cast<int>(anchors->size()); ++i) {
		std::cout << "rect: " << anchors->at(i) << std::endl;
	}

	return 0;
}

float InterRectArea(const cv::Rect & a, const cv::Rect & b) {
	cv::Point left_top = cv::Point(MAX(a.x, b.x), MAX(a.y, b.y));
	cv::Point right_bottom = cv::Point(MIN(a.br().x, b.br().x), MIN(a.br().y, b.br().y));
	cv::Point diff = right_bottom - left_top;
	return (MAX(diff.x + 1, 0) * MAX(diff.y + 1, 0));
}

int ComputeIOU(const cv::Rect & rect1,
	const cv::Rect & rect2, float * iou) {

	float inter_area = InterRectArea(rect1, rect2);
	*iou = inter_area /(rect1.area() + rect2.area() - inter_area);

	return 0;
}

int NMS(const std::vector<FaceInfo>& faces, std::vector<FaceInfo>* result) {
	result->clear();
	if (faces.size() == 0)
		return -1;

	std::vector<size_t> idx(faces.size());

	for (unsigned i = 0; i < idx.size(); i++) {
		idx[i] = i;
	}

	while (idx.size() > 0) {
		int good_idx = idx[0];
		result->push_back(faces[good_idx]);
		std::vector<size_t> tmp = idx;
		idx.clear();
		for (unsigned i = 1; i < tmp.size(); i++) {
			int tmp_i = tmp[i];
			float iou = 0.0f;
			ComputeIOU(faces[good_idx].face_, faces[tmp_i].face_, &iou);
			if (iou <= 0.4)
				idx.push_back(tmp_i);
		}
	}

	return 0;
}