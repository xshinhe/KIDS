/**
 * @file Kernel_Elec.h
 * @author xshinhe
 * @version 1.1
 * @date 2023-03
 * @brief initialization kernels for electonic DOFs
 * @details
 *  The initialization of electonic DOFs are tightly related to Solver.
 *  Use it in Solver's Kernel_Builder();
 */

#ifndef Kernel_Elec_SQC_H
#define Kernel_Elec_SQC_H

#include "../core/Kernel.h"
#include "../core/Policy.h"
#include "Kernel_Elec.h"


namespace PROJECT_NS {

DEFINE_POLICY(SQCPolicy,
              SQR,  // square window
              TRI,  // triangle window
              SPX,  // simplex window
              BIG   // simplex window
);

/**
 * @brief initialization kernel for electonic DOFs in SQC
 */
class Kernel_Elec_SQC final : public Kernel {
   public:
    virtual const std::string name() { return "Kernel_Elec_SQC"; }

    Kernel_Elec_SQC() {
        push(std::shared_ptr<Kernel_Elec>(new Kernel_Elec()));  //
    }

    /**
     * @brief sampling mapping variables from uniform sphere distribution (i.e. uniform simplex for action)
     */
    static int c_window(num_complex *c, int iocc, int type, int fdim);

    static int ker_binning(num_complex *ker, num_complex *rho, int sqc_type);

   private:
    SQCPolicy::_type sqc_type;
    num_real gamma;
    bool use_cv;

    num_real *sqcw;
    num_real *sqcw0;
    num_real *sqcwh;

    virtual void read_param_impl(Param *PM);

    virtual void init_data_impl(DataSet *DS);

    virtual void init_calc_impl(int stat = -1);

    virtual int exec_kernel_impl(int stat = -1);
};


};  // namespace PROJECT_NS

#endif  // Kernel_Elec_SQC_H
