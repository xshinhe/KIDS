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

#ifndef Kernel_Elec_MASH_H
#define Kernel_Elec_MASH_H

#include "../core/Kernel.h"
#include "../core/Policy.h"
#include "Kernel_Elec.h"

namespace PROJECT_NS {

/**
 * @brief initialization kernel for electonic DOFs in MASH
 */
class Kernel_Elec_MASH final : public Kernel {
   public:
    virtual const std::string name() { return "Kernel_Elec_MASH"; }

    Kernel_Elec_MASH() {
        push(std::shared_ptr<Kernel_Elec>(new Kernel_Elec()));  //
    }

    // @brief: generate hopping state from iocc (but with change current state)
    static int hopping_choose(num_complex* rho, num_complex* H, int from, num_real dt);

    static void hopping_direction(num_real* direction, num_real* dE, int from, int to);

    static int hopping_impulse(num_real* direction, num_real* np, num_real* nm, num_real* E,  //
                               int from, int to, bool reflect);

   private:
    MASHPolicy::_type sh_type;
    bool reflect;

    double dt;
    num_real* x;
    num_real* p;
    num_real* m;
    num_real* direction;
    num_real *E, *dE;
    num_complex* H;

    virtual void read_param_impl(Param* PM);

    virtual void init_data_impl(DataSet* DS);

    virtual void init_calc_impl(int stat = -1);

    virtual int exec_kernel_impl(int stat = -1);
};

};  // namespace PROJECT_NS

#endif  // Kernel_Elec_MASH_H
