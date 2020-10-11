#pragma once

#include <vector>

#include "base_classifier.h"
#include "focus_point.h"

#include "kdtree.h"


class ConcentrationKNNClassifier : public BaseClassifier
{
public:
    static std::shared_ptr<spdlog::logger> ml_logger;
    static int set_log_level (int log_level);
    static int set_log_file (char *log_file);
    
    ConcentrationKNNClassifier (struct BrainFlowModelParams params) : BaseClassifier (params)
    {
        num_neighbors = 5;
        kdtree = NULL;
    }

    virtual ~ConcentrationKNNClassifier ()
    {
        release ();
    }

    virtual int prepare ();
    virtual int predict (double *data, int data_len, double *output);
    virtual int release ();

private:
    std::vector<FocusPoint> dataset;
    kdt::KDTree<FocusPoint> *kdtree;
    int num_neighbors;
};
