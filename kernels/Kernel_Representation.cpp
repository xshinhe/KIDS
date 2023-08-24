#include "Kernel_Representation.h"

#include "Kernel_Declare.h"
#include "Kernel_NADForce.h"

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

void Kernel_Representation::read_param_impl(Param* PM) {
    std::string rep_string = PM->get<std::string>("representation_flag", LOC(), "Diabatic");
    representation_type    = RepresentationPolicy::_from(rep_string);
    inp_repr_type          = RepresentationPolicy::_from(PM->get<std::string>("inp_repr_flag", LOC(), rep_string));
    ele_repr_type          = RepresentationPolicy::_from(PM->get<std::string>("ele_repr_flag", LOC(), rep_string));
    nuc_repr_type          = RepresentationPolicy::_from(PM->get<std::string>("nuc_repr_flag", LOC(), rep_string));
    tcf_repr_type          = RepresentationPolicy::_from(PM->get<std::string>("tcf_repr_flag", LOC(), rep_string));
    phase_correction       = PM->get<bool>("phase_correction", LOC(), false);
}

void Kernel_Representation::init_data_impl(DataSet* DS) {
    V  = DS->reg<num_real>("model.V", Dimension::PFF);
    dV = DS->reg<num_real>("model.dV", Dimension::PNFF);
    // ddV = DS->reg<num_real>("model.ddV", Dimension::NNFF);
    E    = DS->reg<num_real>("model.rep.E", Dimension::PF);
    T    = DS->reg<num_real>("model.rep.T", Dimension::PFF);
    Told = DS->reg<num_real>("model.rep.Told", Dimension::PFF);
    dE   = DS->reg<num_real>("model.rep.dE", Dimension::PNFF);
    // ddE = DS->reg<num_real>("model.rep.ddE", Dimension::NNFF);
    L = DS->reg<num_real>("model.rep.L", Dimension::PF);
    R = DS->reg<num_complex>("model.rep.R", Dimension::PFF);
    H = DS->reg<num_complex>("model.rep.H", Dimension::PFF);

    m       = DS->reg<num_real>("integrator.m", Dimension::PN);
    p       = DS->reg<num_real>("integrator.p", Dimension::PN);
    occ_nuc = DS->reg<int>("integrator.occ_nuc", Dimension::P);
    rho_ele = DS->reg<num_complex>("integrator.rho_ele", Dimension::PFF);

    TtTold = DS->reg<num_real>("integrator.tmp.TtTold", Dimension::FF);
    ve     = DS->reg<num_real>("integrator.tmp.ve", Dimension::N);
    vedE   = DS->reg<num_real>("integrator.tmp.vedE", Dimension::FF);
}

void Kernel_Representation::init_calc_impl(int stat) {
    do_refer = false;
    exec_kernel(stat);
    do_refer = true;
    _DataSet->set("init.T", T, Dimension::PFF);
}

int Kernel_Representation::exec_kernel_impl(int stat) {
    if (Dimension::F <= 1) return 0;

    for (int iP = 0; iP < Dimension::P; ++iP) {
        num_real* V    = this->V + iP * Dimension::FF;
        num_real* E    = this->E + iP * Dimension::F;
        num_real* T    = this->T + iP * Dimension::FF;
        num_real* Told = this->Told + iP * Dimension::FF;
        num_real* dV   = this->dV + iP * Dimension::NFF;
        num_real* dE   = this->dE + iP * Dimension::NFF;
        num_real* L    = this->L + iP * Dimension::F;
        num_complex* R = this->R + iP * Dimension::FF;
        num_complex* H = this->H + iP * Dimension::FF;

        num_real* p          = this->p + iP * Dimension::N;
        num_real* m          = this->m + iP * Dimension::N;
        int* occ_nuc         = this->occ_nuc + iP;
        num_complex* rho_ele = this->rho_ele + iP * Dimension::FF;

        switch (representation_type) {
            case RepresentationPolicy::Diabatic: {
                EigenSolve(E, T, V, Dimension::F);
                break;
            }
            case RepresentationPolicy::Adiabatic: {
                if (!onthefly) {  // solve adiabatic from diabatic (otherwise E/dE should be provided from Ab Initio

                    for (int i = 0; i < Dimension::FF; ++i) Told[i] = T[i];  // backup old T matrix
                    EigenSolve(E, T, V, Dimension::F);                       // solve new eigen problem

                    if (do_refer) {  ///< refer the sign and order of the previous step
                        // calculate permutation matrix = rountint(T^ * Told)
                        ARRAY_MATMUL_TRANS1(TtTold, T, Told, Dimension::F, Dimension::F, Dimension::F);

                        // ARRAY_SHOW(TtTold, Dimension::F, Dimension::F);

                        double vset = 0.1 * std::sqrt(1.0e0 / Dimension::F);
                        for (int i = 0; i < Dimension::F; ++i) {
                            double maxnorm = 0;
                            int csr1 = 0, csr2 = 0, csr12 = 0;
                            for (int k1 = 0, k1k2 = 0; k1 < Dimension::F; ++k1) {
                                for (int k2 = 0; k2 < Dimension::F; ++k2, ++k1k2) {
                                    // vmax must be larger than sqrt(1/fdim)
                                    if (std::abs(TtTold[k1k2]) > maxnorm) {
                                        maxnorm = std::abs(TtTold[k1k2]);
                                        csr1 = k1, csr2 = k2, csr12 = k1k2;
                                    }
                                }
                            }
                            double vsign = copysign(1.0f, TtTold[csr12]);
                            for (int k2 = 0, k1k2 = csr1 * Dimension::F;  //
                                 k2 < Dimension::F;                       //
                                 ++k2, ++k1k2) {
                                TtTold[k1k2] = 0;
                            }
                            for (int k1 = 0, k1k2 = csr2; k1 < Dimension::F; ++k1, k1k2 += Dimension::F) {
                                TtTold[k1k2] = 0;
                            }
                            TtTold[csr12] = vsign * vset;
                        }
                        for (int i = 0; i < Dimension::FF; ++i) TtTold[i] = round(TtTold[i] / vset);

                        // adjust order of eigenvectors & eigenvalues
                        ARRAY_MATMUL(T, T, TtTold, Dimension::F, Dimension::F, Dimension::F);
                        for (int i = 0; i < Dimension::FF; ++i) TtTold[i] = std::abs(TtTold[i]);
                        ARRAY_MATMUL(E, E, TtTold, 1, Dimension::F, Dimension::F);
                    }

                    if (FORCE_OPT::BATH_FORCE_BILINEAR) {
                        int& B       = FORCE_OPT::nbath;
                        int& J       = FORCE_OPT::Nb;
                        int JFF      = J * Dimension::FF;
                        double* dVb0 = dV;
                        double* dEb0 = dE;
                        for (int b = 0, bb = 0; b < B; ++b, bb += Dimension::Fadd1, dVb0 += JFF, dEb0 += JFF) {
                            ARRAY_MATMUL3_TRANS1(dEb0, T, dVb0, T, Dimension::F, Dimension::F, Dimension::F,
                                                 Dimension::F);
                            for (int j = 0, jik = 0, jbb = bb; j < J; ++j, jbb += Dimension::FF) {
                                double scale = dVb0[jbb] / dVb0[bb];
                                for (int ik = 0; ik < Dimension::FF; ++ik, ++jik) { dEb0[jik] = dEb0[ik] * scale; }
                            }
                        }
                    } else {
                        Eigen::Map<EigMX<double>> Map_T(T, Dimension::F, Dimension::F);
                        Eigen::Map<EigMX<double>> Map_dV(dV, Dimension::NF, Dimension::F);
                        Eigen::Map<EigMX<double>> Map_dE1(dE, Dimension::NF, Dimension::F);
                        Eigen::Map<EigMX<double>> Map_dE2(dE, Dimension::F, Dimension::NF);

                        Map_dE2 = Map_dV.transpose();
                        Map_dE2 = (Map_T.adjoint() * Map_dE2).eval();
                        Map_dE1 = (Map_dE1 * Map_T).eval();
                        Map_dE1 = Map_dE2.transpose().eval();
                    }
                }

                // calc H = E - im * nacv * p / m, and note here nacv_{ij} = dEij / (Ej - Ei)
                for (int i = 0; i < Dimension::N; ++i) ve[i] = p[i] / m[i];
                ARRAY_MATMUL(vedE, ve, dE, 1, Dimension::N, Dimension::FF);
                for (int i = 0, ij = 0; i < Dimension::F; ++i) {
                    for (int j = 0; j < Dimension::F; ++j, ++ij) {  //
                        H[ij] = ((i == j) ? E[i] : -phys::math::im * vedE[ij] / (E[j] - E[i]));
                    }
                }

                if (phase_correction) {
                    num_real Ekin = 0;
                    for (int j = 0; j < Dimension::N; ++j) Ekin += 0.5f * p[j] * p[j] / m[j];
                    double Epes = 0.0f;
                    if (Kernel_NADForce::NADForce_type == NADForcePolicy::BO) {
                        Epes = E[*occ_nuc];
                    } else {
                        for (int i = 0, ii = 0; i < Dimension::F; ++i, ii += Dimension::Fadd1) {
                            Epes += std::real(rho_ele[ii]) * E[i];
                        }
                    }
                    for (int i = 0, ii = 0; i < Dimension::F; ++i, ii += Dimension::Fadd1) {
                        H[ii] = -2 * Ekin * sqrt(std::max<double>(1.0 + (Epes - E[i]) / Ekin, 0.0f));
                    }
                }

                ARRAY_SHOW(H, Dimension::F, Dimension::F);

                EigenSolve(L, R, H, Dimension::F);  // R*L*R^ = H
                break;
            }
            case RepresentationPolicy::Force:
            case RepresentationPolicy::Density:
            default:
                break;
        }
    }

    return 0;
}

RepresentationPolicy::_type Kernel_Representation::representation_type;
RepresentationPolicy::_type Kernel_Representation::inp_repr_type;
RepresentationPolicy::_type Kernel_Representation::ele_repr_type;
RepresentationPolicy::_type Kernel_Representation::nuc_repr_type;
RepresentationPolicy::_type Kernel_Representation::tcf_repr_type;
bool Kernel_Representation::onthefly;

};  // namespace PROJECT_NS
