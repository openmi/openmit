#include "openmit/framework/mpi/server.h"
#include "openmit/tools/monitor/transaction.h"

namespace mit {

MPIServer::MPIServer(const mit::KWArgs & kwargs, const size_t max_dim) {
  Init(kwargs, max_dim);
}

MPIServer::~MPIServer() {
  // TODO
}

void MPIServer::Init(const mit::KWArgs & kwargs, const size_t max_dim) {
  admm_param_.InitAllowUnknown(kwargs);
  theta_.resize(max_dim, 0.0f);
  // TODO
}


void MPIServer::Run(mit_float * local,
                       mit_float * dual,
                       const size_t max_dim) {
  MiddleAggr(local, dual, max_dim);
  AdmmGlobal();
}

// TODO optimized to vector computation
void MPIServer::MiddleAggr(mit_float * local, 
                           mit_float * dual,
                           const size_t max_dim) {
  for (auto j = 0u; j < max_dim; ++j) {
    theta_[j] = *(dual + j) + admm_param_.rho * *(local + j);
  }
}

void MPIServer::ThetaUpdate() {
  auto scale = rabit::GetWorldSize() * admm_param_.rho + 1e-10;
  auto max_dim = Size();
  for (auto j = 0u; j < max_dim; ++j) {
    if (theta_[j] > admm_param_.lambda_obj) {
      theta_[j] = (theta_[j] - admm_param_.lambda_obj) / scale;
    } else if (theta_[j] < - admm_param_.lambda_obj) {
      theta_[j] = (theta_[j] + admm_param_.lambda_obj) / scale;
    } else {
      theta_[j] = 0;
    }
  }
}

void MPIServer::AdmmGlobal() {
  // AllReduce Sum
  std::unique_ptr<Transaction> trans_allr(
    new Transaction(2, "communication", "allreduce", true));
  rabit::Allreduce<rabit::op::Sum>(Data(), Size());
  Transaction::End(trans_allr.get());

  if (rabit::GetRank() == 0) {
    ThetaUpdate();
  }

  // BroadCast
  std::unique_ptr<Transaction> trans_bc(
    new Transaction(2, "communication", "broadcast", true));
  rabit::Broadcast(Data(), sizeof(mit_float) * Size(), 0);
  Transaction::End(trans_bc.get());
}

void MPIServer::SaveModel(dmlc::Stream * fo) {
  std::unique_ptr<Transaction> trans(
    new Transaction(1, "server", "model_dump"));
  dmlc::ostream os(fo);
  for (auto i = 0u; i < Size(); ++i) {
    os << i << "\t" << theta_[i] << "\n";
  }
  // force flush before fo destruct
  os.set_stream(nullptr);
  Transaction::End(trans.get());
}

void MPIServer::DebugTheta() {
  LOG(INFO) << mit::DebugStr(theta_.data(), theta_.size());
}

} // namespace mit
