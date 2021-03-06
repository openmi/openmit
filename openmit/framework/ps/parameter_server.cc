#include <stdlib.h>
#include "openmit/framework/ps/parameter_server.h"
#include "ps/ps.h"

namespace mit {

DMLC_REGISTER_PARAMETER(PSParam);

PS::PS(const mit::KWArgs & kwargs) {
  kwargs_ = kwargs;
  param_.InitAllowUnknown(kwargs);
  // feature max feature dimension
  uint64_t max_key = param_.max_dimension > 0 
    ? param_.max_dimension : std::numeric_limits<uint64_t>::max();
  setenv("DMLC_MAX_DIMENSION", std::to_string(max_key).c_str(), 1);
  LOG(INFO) << "param_.max_dimension: " << getenv("DMLC_MAX_DIMENSION");
}

void PS::Run() {
  LOG(INFO) << "PS::Run beginning";
  ps::Start();
  LaunchScheduler();
  LaunchServer();
  LaunchWorker();
  ps::Finalize();
  LOG(INFO) << "PS::Run finalize!";
  LOG(INFO) << "sizeof(ps::Key): " << sizeof(ps::Key);
  // TODO
}

void PS::LaunchScheduler() {
  if (!ps::IsScheduler()) return;
  auto scheduler = new mit::Scheduler(kwargs_);
  ps::RegisterExitCallback([scheduler]() { delete scheduler; });
}

void PS::LaunchServer() {
  if (!ps::IsServer()) return;
  auto server = new mit::Server(kwargs_);
  ps::RegisterExitCallback([server]() { delete server; });
}

void PS::LaunchWorker() {
  if (!ps::IsWorker()) return;
  mit::Worker worker(kwargs_);
  worker.Run();
}

} // namespace mit
