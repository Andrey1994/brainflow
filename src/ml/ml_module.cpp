#include <map>
#include <memory>
#include <mutex>
#include <utility>

#include "base_classifier.h"
#include "brainflow_constants.h"
#include "brainflow_model_params.h"
#include "concentration_regression_classifier.h"
#include "ml_module.h"
#include "relaxation_regression_classifier.h"

#include "json.hpp"

using json = nlohmann::json;

int string_to_brainflow_model_params (const char *json_params, struct BrainFlowModelParams *params);

std::map<struct BrainFlowModelParams, std::shared_ptr<BaseClassifier>> ml_models;
std::mutex models_mutex;


int prepare (char *json_params)
{
    std::lock_guard<std::mutex> lock (models_mutex);

    std::shared_ptr<BaseClassifier> model = NULL;
    struct BrainFlowModelParams key (
        (int)BrainFlowMetrics::CONCENTRATION, (int)BrainFlowClassifiers::REGRESSION);
    int res = string_to_brainflow_model_params (json_params, &key);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    if (ml_models.find (key) != ml_models.end ())
    {
        return (int)BrainFlowExitCodes::ANOTHER_CLASSIFIER_IS_PREPARED_ERROR;
    }

    if ((key.metric == (int)BrainFlowMetrics::RELAXATION) &&
        (key.classifier == (int)BrainFlowClassifiers::REGRESSION))
    {
        model = std::shared_ptr<BaseClassifier> (new RelaxationRegressionClassifier (key));
    }
    else if ((key.metric == (int)BrainFlowMetrics::CONCENTRATION) &&
        (key.classifier == (int)BrainFlowClassifiers::REGRESSION))
    {
        model = std::shared_ptr<BaseClassifier> (new ConcentrationRegressionClassifier (key));
    }
    else
    {
        return (int)BrainFlowExitCodes::UNSUPPORTED_CLASSIFIER_AND_METRIC_COMBINATION_ERROR;
    }

    res = model->prepare ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        model = NULL;
    }
    else
    {
        ml_models[key] = model;
    }
    return res;
}

int predict (double *data, int data_len, double *output, char *json_params)
{
    std::lock_guard<std::mutex> lock (models_mutex);

    struct BrainFlowModelParams key (
        (int)BrainFlowMetrics::CONCENTRATION, (int)BrainFlowClassifiers::REGRESSION);
    int res = string_to_brainflow_model_params (json_params, &key);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    auto model = ml_models.find (key);
    if (model == ml_models.end ())
    {
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    return model->second->predict (data, data_len, output);
}

int release (char *json_params)
{
    std::lock_guard<std::mutex> lock (models_mutex);

    struct BrainFlowModelParams key (
        (int)BrainFlowMetrics::CONCENTRATION, (int)BrainFlowClassifiers::REGRESSION);
    int res = string_to_brainflow_model_params (json_params, &key);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    auto model = ml_models.find (key);
    if (model == ml_models.end ())
    {
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    res = model->second->release ();
    ml_models.erase (model);
    return res;
}

int string_to_brainflow_model_params (const char *json_params, struct BrainFlowModelParams *params)
{
    // input string -> json -> struct BrainFlowModelParams
    try
    {
        json config = json::parse (std::string (json_params));
        params->metric = config["metric"];
        params->classifier = config["classifier"];
        params->file = config["file"];
        params->other_info = config["other_info"];
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (json::exception &e)
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
}
