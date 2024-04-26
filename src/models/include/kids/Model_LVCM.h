#ifndef Model_LVCM_H
#define Model_LVCM_H

#include "kids/Kernel.h"
#include "kids/Policy.h"

namespace PROJECT_NS {

DEFINE_POLICY(LVCMPolicy,  //
              PYR3,        //
              PYR3_SPEC,   //
              PYR4,        //
              PYR4_SPEC,   //
              PYR24,       //
              CRC2,        //
              CRC5,        //
              BEN5,        //
              BUTA5,       //
              PENTA5,      //
              CED2,        //
              CED3,        //
              PYR2CED,     //
              Read);       //

class Model_LVCM final : public Kernel {
   public:
    virtual const std::string getName();

    virtual int getType() const;

    Model_LVCM(){};

   private:
    LVCMPolicy::_type lvcm_type;
    int               N_coup;
    int               N_mode;
    bool              classical_bath;


    kids_real* Hsys;
    kids_real *kcoeff, *lcoeff;

    kids_real* x_sigma;
    kids_real* p_sigma;
    kids_real* x0;
    kids_real* p0;

    // integrator
    kids_real *x, *p, *m, *w;

    // model
    kids_real* mass;
    kids_real *vpes, *grad, *hess;
    kids_real *V, *dV, *ddV;

    // int N_ligh;
    // N = N_mode + N_coup + N_ligh

    void setInputParam_impl(std::shared_ptr<Param>& PM);

    void setInputDataSet_impl(std::shared_ptr<DataSet>& DS);

    Status& initializeKernel_impl(Status& stat);

    Status& executeKernel_impl(Status& stat);
};

};  // namespace PROJECT_NS

#endif  // Model_LVCM_H
