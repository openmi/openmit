/*!
 *  Copyright (c) 2017 by Contributors
 *  \file parameter_server.h
 *  \brief parameter server distributed computation framework
 *  \author ZhouYong
 */
#ifndef OPENMIT_FRAMEWORK_PS_PARAMETER_SERVER_H_
#define OPENMIT_FRAMEWORK_PS_PARAMETER_SERVER_H_

#include "openmit/learner/mi_learner.h"
#include "openmit/framework/ps/scheduler.h"
#include "openmit/framework/ps/server.h"
#include "openmit/framework/ps/worker.h"

namespace mit {
/*! 
 * \brief parameter server computational platform parameter
 */
class PSParam : public dmlc::Parameter<PSParam> {
  public:
    /*! \brief framework */
    std::string framework;
    /*! \brief max feature dimension (default uint64::max) */
    uint64_t max_dimension;

    /*! declare parameter field */
    DMLC_DECLARE_PARAMETER(PSParam) {
      DMLC_DECLARE_FIELD(framework).set_default("ps");
      DMLC_DECLARE_FIELD(max_dimension).set_default(0l);
    }
}; // class PSParam

/*!
 * \brief parameter server framework for
 *        distributed machine learning tasks.
 */
class PS : public MILearner {
  public:
    /*! \brief constructor */
    PS(const mit::KWArgs & kwargs);

    /*! \brief destructor */
    ~PS() {}

    /*! \brief get parameter server object */
    static PS * Get(const mit::KWArgs & kwargs) {
      return new PS(kwargs);
    }
    /*! \brief running */
    void Run() override;
  private:
    /*! \brief scheduler launcher */
    void LaunchScheduler();
    /*! \brief server launcher */
    void LaunchServer();
    /*! \brief worker launcher */
    void LaunchWorker();
  private:
    /*! \brief */
    mit::KWArgs kwargs_;
    /*! \brief */
    PSParam param_;
}; // class PS
} // namespace mit

#endif // OPENMIT_FRAMEWORK_PS_PARAMETER_SERVER_H_
