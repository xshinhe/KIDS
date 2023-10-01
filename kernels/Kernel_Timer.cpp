#include "Kernel_Timer.h"

namespace PROJECT_NS {

void Kernel_Timer::read_param_impl(Param* PM) {
    t0    = PM->get<double>("t0", LOC(), phys::time_d, 0.0f);
    tend  = PM->get<double>("tend", LOC(), phys::time_d, 1.0f);
    dt    = PM->get<double>("dt", LOC(), phys::time_d, 0.1f);
    sstep = PM->get<int>("sstep", LOC(), 1);
    nstep = sstep * (int((tend - t0) / (sstep * dt)) + 1);
    nsamp = nstep / sstep + 1;
}

void Kernel_Timer::init_data_impl(DataSet* DS) {
    t_ptr     = DS->reg<num_real>("integrator.t");
    sstep_ptr = DS->reg<int>("timer.sstep");
    istep_ptr = DS->reg<int>("timer.istep");
    nstep_ptr = DS->reg<int>("timer.nstep");
    isamp_ptr = DS->reg<int>("timer.isamp");
    nsamp_ptr = DS->reg<int>("timer.nsamp");
    // initial
    *sstep_ptr = sstep;
    *nstep_ptr = nstep;
    *nsamp_ptr = nsamp;
}

void Kernel_Timer::init_calc_impl(int stat) {
    (*istep_ptr) = 0;
    (*isamp_ptr) = 0;
    (*t_ptr)     = t0;
}

int Kernel_Timer::exec_kernel_impl(int stat) {
    (*t_ptr) += dt;
    (*istep_ptr)++;
    (*isamp_ptr) = (*istep_ptr) / sstep;
    return 0;
}
};  // namespace PROJECT_NS
