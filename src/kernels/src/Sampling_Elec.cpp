#include "kids/Sampling_Elec.h"

#include "kids/Kernel_Elec_Utils.h"
#include "kids/Kernel_NAForce.h"
#include "kids/Kernel_Random.h"
#include "kids/Kernel_Representation.h"
#include "kids/debug_utils.h"
#include "kids/hash_fnv1a.h"
#include "kids/linalg.h"
#include "kids/macro_utils.h"
#include "kids/vars_list.h"

namespace PROJECT_NS {

inline bool isFileExists(const std::string& name) { return std::ifstream{name.c_str()}.good(); }

const std::string Sampling_Elec::getName() { return "Sampling_Elec"; }

int Sampling_Elec::getType() const { return utils::hash(FUNCTION_NAME); }

void Sampling_Elec::setInputParam_impl(std::shared_ptr<Param> PM) {
    // for c & rho_ele
    sampling_type = ElectronicSamplingPolicy::_from(
        _param->get_string({"solver.sampling_ele_flag", "solver.sampling.ele_flag"}, LOC(), "Constraint"));

    occ0 = _param->get_int({"model.occ", "solver.occ"}, LOC(), -1);
    if (occ0 < 0) throw std::runtime_error("occ < 0");
    if (occ0 >= Dimension::F) throw std::runtime_error("occ >= F");

    gamma1 = _param->get_real({"solver.gamma"}, LOC(), elec_utils::gamma_wigner(Dimension::F));
    if (gamma1 < -1.5) gamma1 = elec_utils::gamma_opt(Dimension::F);
    if (gamma1 < -0.5) gamma1 = elec_utils::gamma_wigner(Dimension::F);
    xi1 = (1 + Dimension::F * gamma1);

    // for rho_nuc
    use_cv   = _param->get_bool({"solver.use_cv"}, LOC(), false);
    use_wmm  = _param->get_bool({"solver.use_wmm"}, LOC(), false);
    use_sum  = _param->get_bool({"solver.use_sum"}, LOC(), false);
    use_fssh = _param->get_bool({"solver.use_fssh"}, LOC(), false);
}

void Sampling_Elec::setInputDataSet_impl(std::shared_ptr<DataSet> DS) {
    c       = DS->def(DATA::integrator::c);
    rho_ele = DS->def(DATA::integrator::rho_ele);
    rho_nuc = DS->def(DATA::integrator::rho_nuc);
    occ_nuc = DS->def(DATA::integrator::occ_nuc);
    w       = DS->def(DATA::integrator::w);
    T       = DS->def(DATA::model::rep::T);
}

Status& Sampling_Elec::initializeKernel_impl(Status& stat) { return stat; }

Status& Sampling_Elec::executeKernel_impl(Status& stat) {
    // if restart, we should get all initial values and current values!
    // not that: all values defined by Kernel_Elec_Functions will also be recoveed later.
    // not that: all values defined by Kernel_Recorder will also be recovered later.
    if (_param->get_bool({"restart"}, LOC(), false)) {  //
        std::string loadfile = _param->get_string({"load"}, LOC(), "NULL");
        if (loadfile == "NULL" || loadfile == "" || loadfile == "null") loadfile = "restart.ds";
        _dataset->def(DATA::init::c, loadfile);
        _dataset->def(DATA::init::rho_ele, loadfile);
        _dataset->def(DATA::init::rho_nuc, loadfile);
        _dataset->def(DATA::init::T, loadfile);
        _dataset->def(DATA::integrator::c, loadfile);
        _dataset->def(DATA::integrator::rho_ele, loadfile);
        _dataset->def(DATA::integrator::rho_nuc, loadfile);
        _dataset->def(DATA::integrator::occ_nuc, loadfile);
        _dataset->def(DATA::integrator::w, loadfile);
        return stat;
    }

    for (int iP = 0; iP < Dimension::P; ++iP) {  // use P insread P_NOW
        auto c       = this->c.subspan(iP * Dimension::F, Dimension::F);
        auto rho_ele = this->rho_ele.subspan(iP * Dimension::FF, Dimension::FF);
        auto rho_nuc = this->rho_nuc.subspan(iP * Dimension::FF, Dimension::FF);
        auto w       = this->w.subspan(iP, 1);
        auto T       = this->T.subspan(iP * Dimension::FF, Dimension::FF);
        auto occ_nuc = this->occ_nuc.subspan(iP, 1);

        /////////////////////////////////////////////////////////////////

        int iocc;
        Kernel_Random::rand_catalog(&iocc, 1, true, 0, Dimension::F - 1);
        iocc = ((use_sum) ? iocc : occ0);
        w[0] = (use_sum) ? kids_complex(Dimension::F) : phys::math::iu;

        switch (sampling_type) {
            case ElectronicSamplingPolicy::Focus: {
                elec_utils::c_focus(c.data(), xi1, gamma1, iocc, Dimension::F);
                elec_utils::ker_from_c(rho_ele.data(), c.data(), 1, 0, Dimension::F);
                elec_utils::ker_from_rho(rho_nuc.data(), rho_ele.data(), xi1, gamma1, Dimension::F, use_cv, iocc);
                break;
            }
            case ElectronicSamplingPolicy::GDTWA: {
                elec_utils::c_focus(c.data(), xi1, gamma1, iocc, Dimension::F);  // @useless

                /// GDTWA sampling step 1: discrete random phase
                for (int j = 0; j < Dimension::F; ++j) {
                    if (j == iocc) {
                        rho_ele[j * Dimension::Fadd1] = 1.0e0;
                    } else {
                        double randu;
                        Kernel_Random::rand_uniform(&randu);
                        randu                            = phys::math::halfpi * (int(randu / 0.25f) + 0.5);
                        rho_ele[iocc * Dimension::F + j] = cos(randu) + phys::math::im * sin(randu);
                        rho_ele[j * Dimension::F + iocc] = std::conj(rho_ele[iocc * Dimension::F + j]);
                    }
                }
                for (int i = 0, ij = 0; i < Dimension::F; ++i) {
                    for (int j = 0; j < Dimension::F; ++j, ++ij) {
                        if (i == iocc || j == iocc) continue;
                        rho_ele[ij] = rho_ele[iocc * Dimension::F + j] / rho_ele[iocc * Dimension::F + i];
                    }
                }
                /// GDTWA sampling step 2: set off-diagonal
                double gamma_ou = phys::math::sqrthalf;
                double gamma_uu = 0.0e0;
                for (int i = 0, ij = 0; i < Dimension::F; ++i) {
                    for (int j = 0; j < Dimension::F; ++j, ++ij) {
                        if (i == j) {
                            rho_ele[ij] = (i == iocc) ? phys::math::iu : phys::math::iz;
                        } else if (i == iocc || j == iocc) {
                            rho_ele[ij] *= gamma_ou;
                        } else {
                            rho_ele[ij] *= gamma_uu;
                        }
                    }
                }
                elec_utils::ker_from_rho(rho_nuc.data(), rho_ele.data(), 1, 0, Dimension::F, false, iocc);
                break;
            }
            case ElectronicSamplingPolicy::SQCtri: {
                elec_utils::c_window(c.data(), iocc, ElectronicSamplingPolicy::SQCtri, Dimension::F);
                elec_utils::ker_from_c(rho_ele.data(), c.data(), 1, 0, Dimension::F);
                elec_utils::ker_from_rho(rho_nuc.data(), rho_ele.data(), 1.0, gamma1, Dimension::F, use_cv, iocc);
                break;
            }
            case ElectronicSamplingPolicy::SQCspx: {
                elec_utils::c_sphere(c.data(), Dimension::F);
                for (int i = 0; i < Dimension::F; ++i) c[i] = std::abs(c[i] * c[i]);
                c[iocc] += 1.0e0;
                for (int i = 0; i < Dimension::F; ++i) {
                    kids_real randu;
                    Kernel_Random::rand_uniform(&randu);
                    randu *= phys::math::twopi;
                    c[i] = sqrt(c[i]) * (cos(randu) + phys::math::im * sin(randu));
                }
                elec_utils::ker_from_c(rho_ele.data(), c.data(), 1, 0, Dimension::F);
                elec_utils::ker_from_rho(rho_nuc.data(), rho_ele.data(), 1.0, gamma1, Dimension::F, use_cv, iocc);
                break;
            }
            case ElectronicSamplingPolicy::SQCtest01: {
                elec_utils::c_window(c.data(), iocc, ElectronicSamplingPolicy::SQCtri, Dimension::F);
                elec_utils::ker_from_c(rho_ele.data(), c.data(), 1, 0, Dimension::F);
                double norm = 0.0;
                for (int i = 0; i < Dimension::F; ++i) norm += std::abs(rho_ele[i * Dimension::Fadd1]);
                elec_utils::ker_from_rho(rho_nuc.data(), rho_ele.data(), xi1 / norm, gamma1, Dimension::F, use_cv,
                                         iocc);
                break;
            }
            case ElectronicSamplingPolicy::SQCtest02: {
                elec_utils::c_window(c.data(), iocc, ElectronicSamplingPolicy::SQCtri, Dimension::F);
                double norm = 0.0e0;
                for (int i = 0; i < Dimension::F; ++i) norm += std::abs(c[i] * c[i]);
                xi1    = norm;
                gamma1 = (xi1 - 1.0e0) / Dimension::F;
                norm   = sqrt(norm);
                for (int i = 0; i < Dimension::F; ++i) c[i] /= norm;
                elec_utils::ker_from_c(rho_ele.data(), c.data(), 1, 0, Dimension::F);
                norm = 0.0;
                for (int i = 0; i < Dimension::F; ++i) norm += std::abs(rho_ele[i * Dimension::Fadd1]);
                double gmeff = (norm - 1.0) / Dimension::F;
                elec_utils::ker_from_rho(rho_nuc.data(), rho_ele.data(), 1.0, gmeff, Dimension::F, use_cv, iocc);
                break;
            }
            case ElectronicSamplingPolicy::Gaussian: {
                // elec_utils::c_gaussian(c, Dimension::F); /// @debug
                elec_utils::ker_from_c(rho_ele.data(), c.data(), 1, 0, Dimension::F);
                elec_utils::ker_from_rho(rho_nuc.data(), rho_ele.data(), xi1, gamma1, Dimension::F, use_cv, iocc);
                w[0] = kids_complex(Dimension::F);
                break;
            }
            case ElectronicSamplingPolicy::Constraint: {
                elec_utils::c_sphere(c.data(), Dimension::F);
                elec_utils::ker_from_c(rho_ele.data(), c.data(), 1, 0, Dimension::F);
                elec_utils::ker_from_rho(rho_nuc.data(), rho_ele.data(), xi1, gamma1, Dimension::F, use_cv, iocc);
                w[0] = kids_complex(Dimension::F);
                break;
            }
            default: {
                std::string open_file = sampling_file;
                if (!isFileExists(sampling_file)) open_file = utils::concat(sampling_file, stat.icalc, ".ds");
                std::string   stmp, eachline;
                std::ifstream ifs(open_file);
                while (getline(ifs, eachline)) {
                    if (eachline.find("init.c") != eachline.npos) {
                        getline(ifs, eachline);
                        for (int i = 0; i < Dimension::F; ++i) ifs >> c[i];
                    }
                    // if (eachline.find("init.rho_ele") != eachline.npos) {
                    //     getline(ifs, eachline);
                    //     for (int i = 0; i < Dimension::FF; ++i) ifs >> rho_ele[i];
                    // }
                    // if (eachline.find("init.rho_nuc") != eachline.npos) {
                    //     getline(ifs, eachline);
                    //     for (int i = 0; i < Dimension::FF; ++i) ifs >> rho_nuc[i];
                    // }
                }
                elec_utils::ker_from_c(rho_ele.data(), c.data(), 1, 0, Dimension::F);  ///< initial rho_ele
                elec_utils::ker_from_rho(rho_nuc.data(), rho_ele.data(), xi1, gamma1, Dimension::F, use_cv, iocc);
                w[0] = phys::math::iu;
            }
        }

        // BO occupation in adiabatic representation
        Kernel_Representation::transform(rho_nuc.data(), T.data(), Dimension::F,  //
                                         Kernel_Representation::inp_repr_type,    //
                                         Kernel_Representation::nuc_repr_type,    //
                                         SpacePolicy::L);
        occ_nuc[0] = elec_utils::max_choose(rho_nuc.data());
        if (use_fssh) occ_nuc[0] = elec_utils::pop_choose(rho_nuc.data());
        Kernel_Representation::transform(rho_nuc.data(), T.data(), Dimension::F,  //
                                         Kernel_Representation::nuc_repr_type,    //
                                         Kernel_Representation::inp_repr_type,    //
                                         SpacePolicy::L);
    }
    _dataset->def(DATA::init::c, c);
    _dataset->def(DATA::init::rho_ele, rho_ele);
    _dataset->def(DATA::init::rho_nuc, rho_nuc);
    _dataset->def(DATA::init::T, T);
    // _dataset->def(VARIABLE<kids_complex>("init.c", &Dimension::shape_PF, "@"), c);
    // _dataset->def(VARIABLE<kids_complex>("init.rho_ele", &Dimension::shape_PFF, "@"), rho_ele);
    // _dataset->def(VARIABLE<kids_complex>("init.rho_nuc", &Dimension::shape_PFF, "@"), rho_nuc);
    // _dataset->def(VARIABLE<kids_real>("init.T", &Dimension::shape_PFF, "@"), T);
    return stat;
}

// Status& Sampling_Elec::executeKernel_impl(Status& stat) { return stat; }

};  // namespace PROJECT_NS
