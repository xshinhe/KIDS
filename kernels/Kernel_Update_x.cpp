#include "Kernel_Update_x.h"

#include "Kernel_Declare.h"

#define ARRAY_SHOW(_A, _n1, _n2)                                                     \
    ({                                                                               \
        std::cout << "Show Array <" << #_A << ">\n";                                 \
        int _idxA = 0;                                                               \
        for (int _i = 0; _i < (_n1); ++_i) {                                         \
            for (int _j = 0; _j < (_n2); ++_j) std::cout << FMT(4) << (_A)[_idxA++]; \
            std::cout << std::endl;                                                  \
        }                                                                            \
    })

namespace PROJECT_NS {

void Kernel_Update_x::read_param_impl(Param* PM) {
    dt  = PM->get<double>("dt", LOC(), phys::time_d);
    sdt = scale * dt;
}

void Kernel_Update_x::init_data_impl(DataSet* DS) {
    x              = DS->reg<num_real>("integrator.x", Dimension::PN);
    p              = DS->reg<num_real>("integrator.p", Dimension::PN);
    m              = DS->reg<num_real>("integrator.m", Dimension::PN);
    minv           = DS->reg<num_real>("integrator.minv", Dimension::PN);
    num_real* mass = DS->reg<num_real>("model.mass", Dimension::N);
    for (int iP = 0; iP < Dimension::P; ++iP) {
        num_real* m    = this->m + iP * Dimension::N;
        num_real* minv = this->minv + iP * Dimension::N;
        for (int j = 0; j < Dimension::N; ++j) {
            m[j]    = mass[j];
            minv[j] = 1 / m[j];
        }
    }
}

int Kernel_Update_x::exec_kernel_impl(int stat) {
    for (int i = 0; i < Dimension::PN; ++i) x[i] += p[i] * minv[i] * sdt;
    return 0;
}

};  // namespace PROJECT_NS
