#include "openmit/engine/trainer.h"

namespace mit {

DMLC_REGISTER_PARAMETER(TrainerParam);

Trainer::Trainer(const mit::KWArgs & kwargs) {
  Init(kwargs);
}

void Trainer::Init(const mit::KWArgs & kwargs) {
  param_.InitAllowUnknown(kwargs);
  model_ = mit::Model::Create(kwargs);
  metric_ = mit::Metric::Create(param_.metric);
  loss_  = mit::Loss::Create(param_.loss_type);
}

void Trainer::Run(
    const dmlc::RowBlock<mit_uint> & batch, 
    std::vector<ps::Key> & keys,
    std::vector<mit_float> & rets,
    std::vector<mit_float> * vals) {

  auto nfeature = keys.size();
  auto unit_size = 
    model_->Param().field_num * model_->Param().k + 1;
  // map weight
  std::unordered_map<mit_uint, mit::Unit * > map_weight;

  for (auto i = 0u; i < nfeature; ++i) {
    mit::Unit * unit = new mit::Unit(unit_size);
    unit->CopyFrom(
        rets.begin() + i*unit_size, rets.begin() + (i+1)*unit_size);
    map_weight.insert(std::make_pair(keys[i], unit));
  }
  
  // predict
  std::vector<mit_float> preds(batch.size);
  model_->Predict(batch, map_weight, &preds);
  
  // gradient
  std::unordered_map<mit_uint, mit::Unit * > map_grad;
  for (auto i = 0u; i < nfeature; ++i) {
    mit::Unit * unit = new mit::Unit(unit_size, 0.0);
    map_grad.insert(std::make_pair(keys[i], unit));
  }
  model_->Gradient(batch, preds, map_weight, &map_grad);

  // map_grad_ --> vals
  vals->clear();
  for (auto i = 0u; i < nfeature; i++) {
    mit::Unit * unit = map_grad[keys[i]];
    vals->insert(vals->end(), 
        unit->Data(), unit->Data() + unit->Size());
    delete unit;
  }
}

float Trainer::Eval(mit::DMatrix * data, 
                    std::vector<ps::Key> & keys, 
                    std::vector<mit_float> & rets) {
  // 1. predict
  auto nfeature = keys.size();
  auto unit_size = 
    model_->Param().field_num * model_->Param().k + 1;
  // map weight
  std::unordered_map<mit_uint, mit::Unit * > map_weight;

  for (auto i = 0u; i < nfeature; ++i) {
    mit::Unit * unit = new mit::Unit(unit_size);
    unit->CopyFrom(
        rets.begin() + i*unit_size, rets.begin() + (i+1)*unit_size);
    map_weight.insert(std::make_pair(keys[i], unit));
  }
  
  std::vector<mit_float> preds;
  std::vector<mit_float> labels;
  std::vector<mit_float> tmp_preds;;
  data->BeforeFirst();

  while (data->Next()) {
    const auto & block = data->Value();
    labels.insert(labels.end(), block.label, block.label + block.size);

    tmp_preds.clear(); tmp_preds.resize(block.size);
    model_->Predict(block, map_weight, &tmp_preds);
    preds.insert(preds.end(), tmp_preds.begin(), tmp_preds.end());
  }

  CHECK_EQ(preds.size(), labels.size());

  // 2. metric
  float metric = metric_->Eval(preds, labels);
  return metric;
}


void Trainer::Loss(
    const dmlc::RowBlock<mit_uint> & batch, 
    const std::vector<mit_float> * predict, 
    std::vector<mit_float> * loss) {
  // TODO loss_->Loss()
}


} // namespace mit
