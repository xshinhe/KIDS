#include "kids/Model_LVCM.h"

#include "kids/Kernel_Random.h"
#include "kids/debug_utils.h"
#include "kids/hash_fnv1a.h"
#include "kids/macro_utils.h"
#include "kids/vars_list.h"

namespace PROJECT_NS {

const std::string Model_LVCM::getName() { return "Model_LVCM"; }

int Model_LVCM::getType() const { return utils::hash(FUNCTION_NAME); }

void Model_LVCM::setInputParam_impl(std::shared_ptr<Param> PM) {
    lvcm_type = LVCMPolicy::_from(_param->get_string({"model.lvcm_flag"}, LOC(), "PYR3"));
}

void Model_LVCM::setInputDataSet_impl(std::shared_ptr<DataSet> DS) {
    Hsys = DS->def(DATA::model::Hsys);
    w    = DS->def(DATA::model::w);
    Kmat = DS->def(DATA::model::Kmat);
    Qmat = DS->def(DATA::model::Qmat);
    Tmod = DS->def(DATA::model::Tmod);
    memset(Hsys.data(), 0, Dimension::FF * sizeof(kids_real));
    switch (lvcm_type) {
        case LVCMPolicy::PYR3: {
            kids_assert(Dimension::N == 3, "Dimension Error");
            double H_unit = phys::au_2_ev;

            N_mode = 2;
            // parameter for PYR3
            double  E_data_PYR3[2]      = {3.94f, 4.84f};
            double  w_data_PYR3[3]      = {0.074f, 0.126f, 0.118f};
            double  kcoeff_data_PYR3[4] = {-0.105f, 0.149f, 0.037f, -0.254f};
            double  lcoeff_data_PYR3[4] = {0.000f, 0.262f, 0.262f, 0.000f};
            double *E_data              = E_data_PYR3;
            double *w_data              = w_data_PYR3;
            double *kcoeff_data         = kcoeff_data_PYR3;
            double *lcoeff_data         = lcoeff_data_PYR3;
            for (int i = 0, ii = 0; i < Dimension::F; ++i, ii += Dimension::Fadd1) {  //
                Hsys[ii] = E_data[i] / H_unit;
            }
            for (int j = 0; j < Dimension::N; ++j) w[j] = w_data[j] / H_unit;
            for (int j = 0, idxkcoeff = 0, idxlcoeff = 0; j < Dimension::N; ++j) {
                for (int i = 0, ik = 0; i < Dimension::F; ++i) {
                    for (int k = 0; k < Dimension::F; ++k, ++ik) {
                        Qmat[j * Dimension::FF + ik] =                                      //
                            (j < N_mode) ? ((i == k) ? (kcoeff_data[idxkcoeff++]) : 0.0e0)  //
                                         : lcoeff_data[idxlcoeff++];
                        Qmat[j * Dimension::FF + ik] /= H_unit;
                        Qmat[j * Dimension::FF + ik] *= std::sqrt(w[j]);
                    }
                }
            }
            break;
        }
        case LVCMPolicy::PYR24: {
            kids_assert(Dimension::N == 24, "Dimension Error");
            double H_unit = phys::au_2_ev;

            N_mode = 23;
            // parameter for PYR24
            double E_data_PYR24[2]       = {-0.4617f, 0.4617f};
            double w_data_PYR24[24]      = {0.0740f, 0.1273f, 0.1568f, 0.1347f, 0.3431f, 0.1157f, 0.3242f, 0.3621f,
                                       0.2673f, 0.3052f, 0.0968f, 0.0589f, 0.0400f, 0.1726f, 0.2863f, 0.2484f,
                                       0.1536f, 0.2105f, 0.0778f, 0.2294f, 0.1915f, 0.4000f, 0.3810f, 0.0936f};
            double kcoeff_data_PYR24[46] = {-0.0964f, 0.1194f,  0.0470f, 0.2012f,  0.1594f, 0.0484f,  0.0308f, -0.0308f,
                                            0.0782f,  -0.0782f, 0.0261f, -0.0261f, 0.0717f, -0.0717f, 0.0780f, -0.0780f,
                                            0.0560f,  -0.0560f, 0.0625f, -0.0625f, 0.0188f, -0.0188f, 0.0112f, -0.0112f,
                                            0.0069f,  -0.0069f, 0.0265f, -0.0265f, 0.0433f, -0.0433f, 0.0361f, -0.0361f,
                                            0.0210f,  -0.0210f, 0.0281f, -0.0281f, 0.0102f, -0.0102f, 0.0284f, -0.0284f,
                                            0.0196f,  -0.0196f, 0.0306f, -0.0306f, 0.0269f, -0.0269f};
            double lcoeff_data_PYR24[4]  = {0.000f, 0.1825f, 0.1825f, 0.000f};
            double *E_data               = E_data_PYR24;
            double *w_data               = w_data_PYR24;
            double *kcoeff_data          = kcoeff_data_PYR24;
            double *lcoeff_data          = lcoeff_data_PYR24;

            for (int i = 0, ii = 0; i < Dimension::F; ++i, ii += Dimension::Fadd1) {  //
                Hsys[ii] = E_data[i] / H_unit;
            }
            for (int j = 0; j < Dimension::N; ++j) w[j] = w_data[j] / H_unit;
            for (int j = 0, idxkcoeff = 0, idxlcoeff = 0; j < Dimension::N; ++j) {
                for (int i = 0, ik = 0; i < Dimension::F; ++i) {
                    for (int k = 0; k < Dimension::F; ++k, ++ik) {
                        Qmat[j * Dimension::FF + ik] =                                      //
                            (j < N_mode) ? ((i == k) ? (kcoeff_data[idxkcoeff++]) : 0.0e0)  //
                                         : lcoeff_data[idxlcoeff++];
                        Qmat[j * Dimension::FF + ik] /= H_unit;
                        Qmat[j * Dimension::FF + ik] *= std::sqrt(w[j]);
                    }
                }
            }
            break;
        }
        case LVCMPolicy::BUTA5: {
            kids_assert(Dimension::N == 5, "Dimension Error");
            double H_unit = phys::au_2_ev;

            N_mode = 4;
            // parameter for BUTA5
            double  E_data_BUTA5[2]      = {9.41165f, 9.95575f};
            double  w_data_BUTA5[5]      = {0.1089f, 0.1773f, 0.2578f, 0.3713f, 0.0912f};
            double  kcoeff_data_BUTA5[8] = {-0.0531f, -0.0594f, 0.0115f,  0.0100f,  //
                                           -0.1628f, 0.3422f,  -0.0403f, 0.0321f};
            double  lcoeff_data_BUTA5[4] = {0.000f, 0.2880f, 0.2880f, 0.000f};
            double *E_data               = E_data_BUTA5;
            double *w_data               = w_data_BUTA5;
            double *kcoeff_data          = kcoeff_data_BUTA5;
            double *lcoeff_data          = lcoeff_data_BUTA5;

            for (int i = 0, ii = 0; i < Dimension::F; ++i, ii += Dimension::Fadd1) {  //
                Hsys[ii] = E_data[i] / H_unit;
            }
            for (int j = 0; j < Dimension::N; ++j) w[j] = w_data[j] / H_unit;
            for (int j = 0, idxkcoeff = 0, idxlcoeff = 0; j < Dimension::N; ++j) {
                for (int i = 0, ik = 0; i < Dimension::F; ++i) {
                    for (int k = 0; k < Dimension::F; ++k, ++ik) {
                        Qmat[j * Dimension::FF + ik] =                                      //
                            (j < N_mode) ? ((i == k) ? (kcoeff_data[idxkcoeff++]) : 0.0e0)  //
                                         : lcoeff_data[idxlcoeff++];
                        Qmat[j * Dimension::FF + ik] /= H_unit;
                        Qmat[j * Dimension::FF + ik] *= std::sqrt(w[j]);
                    }
                }
            }
            break;
        }
        case LVCMPolicy::CRC2:
        case LVCMPolicy::CRC5: {
            kids_assert(Dimension::N <= 5, "Dimension Error");
            double H_unit = phys::au_2_ev;

            N_mode = 0;

            const double kappa1   = -0.0169f;
            const double kappa3   = -0.0272f;
            const double lambda1a = 0.0328f;
            const double lambda1b = -0.0978f;
            const double lambda2a = 0.0095f;
            const double lambda2b = 0.1014f;

            double E_data_CRC5[3]       = {0.0424f, 0.0424f, 0.4344f};
            double w_data_CRC5[5]       = {0.0129f, 0.0129f, 0.0342f, 0.0561f, 0.0561f};
            double kcoeff_data_CRC5[1]  = {0.0f};
            double lcoeff_data_CRC5[45] = {
                // data
                0.0f,      lambda1a, 0.0f,      // x0
                lambda1a,  0.0f,     lambda1b,  //
                0.0f,      lambda1b, 0.0f,      //
                -lambda1a, 0.0f,     lambda1b,  // x1
                0.0f,      lambda1a, 0.0f,      //
                lambda1b,  0.0f,     0.0f,      //
                kappa1,    0.0f,     0.0f,      // x2
                0.0f,      kappa1,   0.0f,      //
                0.0f,      0.0f,     kappa3,    //
                -lambda2a, 0.0f,     lambda2b,  // x3
                0.0f,      lambda2a, 0.0f,      //
                lambda2b,  0.0f,     0.0f,      //
                0.0f,      lambda2a, 0.0f,      // x4
                lambda2a,  0.0f,     lambda2b,  //
                0.0f,      lambda2b, 0.0f       //
            };

            double *E_data      = E_data_CRC5;
            double *w_data      = w_data_CRC5;
            double *kcoeff_data = kcoeff_data_CRC5;
            double *lcoeff_data = lcoeff_data_CRC5;

            for (int i = 0, ii = 0; i < Dimension::F; ++i, ii += Dimension::Fadd1) {  //
                Hsys[ii] = E_data[i] / H_unit;
            }
            for (int j = 0; j < Dimension::N; ++j) w[j] = w_data[j] / H_unit;
            for (int j = 0, idxkcoeff = 0, idxlcoeff = 0; j < Dimension::N; ++j) {
                for (int i = 0, ik = 0; i < Dimension::F; ++i) {
                    for (int k = 0; k < Dimension::F; ++k, ++ik) {
                        Qmat[j * Dimension::FF + ik] =                                      //
                            (j < N_mode) ? ((i == k) ? (kcoeff_data[idxkcoeff++]) : 0.0e0)  //
                                         : lcoeff_data[idxlcoeff++];
                        Qmat[j * Dimension::FF + ik] /= H_unit;
                        Qmat[j * Dimension::FF + ik] *= std::sqrt(w[j]);
                    }
                }
            }
            break;
        }
        case LVCMPolicy::CED2:
        case LVCMPolicy::CED3: {
            N_mode                  = 0;
            const double lightspeed = 137.03599907444f;
            const double epsilon0   = 0.25f / phys::math::pi;

            double E_data_CED2[2]  = {-0.6738f, -0.2798f};
            double mu_data_CED2[4] = {0.000f, +1.034f,  //
                                      +1.034f, 0.000f};

            double E_data_CED3[3]  = {-0.6738f, -0.2798f, -0.1547f};
            double mu_data_CED3[9] = {0.000f,  +1.034f, 0.000f,   //
                                      +1.034f, 0.000f,  -2.536f,  //
                                      0.000f,  -2.536f, 0.000f};

            double Lcav = 2.362e5;
            double Rcav = Lcav / 2;

            double *E_data;
            double *mu_data;
            switch (lvcm_type) {
                case LVCMPolicy::CED2:
                    kids_assert(Dimension::F == 2, "Dimension Error");

                    E_data  = E_data_CED2;
                    mu_data = mu_data_CED2;
                    break;
                case LVCMPolicy::CED3:
                    kids_assert(Dimension::F == 3, "Dimension Error");

                    E_data  = E_data_CED3;
                    mu_data = mu_data_CED3;
                    break;
            }

            for (int i = 0, ii = 0; i < Dimension::F; ++i, ii += Dimension::Fadd1) { Hsys[ii] = E_data[i]; }
            for (int j = 0, jik = 0; j < Dimension::N; ++j) {
                w[j] = (2 * j + 1) * lightspeed * phys::math::pi / Lcav;
                for (int ik = 0; ik < Dimension::FF; ++ik, ++jik) {
                    Qmat[jik] = sqrt(2.0f / (epsilon0 * Lcav)) * sin((2 * j + 1) * phys::math::pi * Rcav / Lcav) *
                                w[j] * mu_data[ik];
                }
            }
            break;
        }
        case LVCMPolicy::PYR2CED: {
            N_mode        = 1;
            double H_unit = phys::au_2_ev;

            double gcoup = _param->get_real({"model.gcoup"}, LOC(), phys::energy_d, 0.24f / H_unit);
            double wcav  = _param->get_real({"model.wcav"}, LOC(), phys::energy_d, 0.62f / H_unit);

            // parameter for PYR2
            double  E_data_PYR2[2]      = {3.94f, 4.84f};
            double  w_data_PYR2[3]      = {0.074f, 0.118f};
            double  kcoeff_data_PYR2[4] = {-0.105f, 0.149f};
            double  lcoeff_data_PYR2[4] = {0.000f, 0.262f, 0.262f, 0.000f};
            double *E_data              = E_data_PYR2;
            double *w_data              = w_data_PYR2;
            double *kcoeff_data         = kcoeff_data_PYR2;
            double *lcoeff_data         = lcoeff_data_PYR2;

            for (int i = 0, ii = 0; i < Dimension::F; ++i, ii += Dimension::Fadd1) {
                Hsys[ii] = i / (Dimension::F / 2) * wcav + E_data[i % 2] / H_unit;
            }
            Hsys[0 * 4 + 3] = gcoup;
            Hsys[1 * 4 + 2] = gcoup;
            Hsys[2 * 4 + 1] = gcoup;
            Hsys[3 * 4 + 0] = gcoup;
            for (int j = 0; j < Dimension::N; ++j) w[j] = w_data[j] / H_unit;
            for (int j = 0, ji = 0; j < N_mode; ++j) {
                for (int i = 0; i < Dimension::F; ++i, ++ji) {
                    kcoeff[ji] = (kcoeff_data[j * 2 + i % 2] / H_unit) * sqrt(w[j]);
                }
            }
            for (int j = 0, jik = 0; j < Dimension::N; ++j) {
                for (int i = 0, ik = 0; i < Dimension::F; ++i) {
                    for (int k = 0; k < Dimension::F; ++k, ++ik, ++jik) {
                        if (j < N_mode) {
                            Qmat[jik] = (i == k) ? (kcoeff_data[j * 2 + i % 2] / H_unit) * sqrt(w[j]) : 0.0e0;
                        } else {
                            Qmat[jik] =
                                (i / (Dimension::F / 2) == k / (Dimension::F / 2))  //
                                    ? (lcoeff_data[(j - N_mode) * 4 + (i % 2) * 2 + k % 2] / H_unit) * sqrt(w[j])
                                    : 0.0e0;
                        }
                    }
                }
            }
            break;
        }
        case LVCMPolicy::BEN5: {
            break;
        }
        case LVCMPolicy::Read: {
            std::string   lvcm_file = _param->get_string({"model.lvcm_file"}, LOC(), "lvcm.dat");
            std::ifstream ifs(lvcm_file);
            std::string   H_unit_str;
            std::string   firstline;
            getline(ifs, firstline);
            std::stringstream sstr(firstline);
            sstr >> H_unit_str;  ///< the firstline stores H's unit
            double H_unit = phys::us::conv(phys::au::unit, phys::us::parse(H_unit_str));

            // read E
            int         dsize;
            std::string flag;
            double      val;
            ifs >> flag >> dsize;
            kids_assert(dsize == Dimension::F, "Dimension Error");
            for (int i = 0, ii = 0; i < Dimension::F; ++i, ii += Dimension::Fadd1)
                if (ifs >> val) Hsys[ii] = val / H_unit;

            // read w
            ifs >> flag >> dsize;
            kids_assert(dsize == Dimension::N, "Dimension Error");
            for (int i = 0, ii = 0; i < Dimension::N; ++i)
                if (ifs >> val) w[i] = val / H_unit;

            N_mode = 0;
            // read kcoeff & lcoeff
            for (int j = 0; j < Dimension::N; ++j) {
                ifs >> flag;
                if (flag == "K") {
                    for (int i = 0, ii = 0; i < Dimension::F; ++i, ii += Dimension::Fadd1) {
                        if (ifs >> val) {
                            Qmat[j * Dimension::FF + ii] = val / H_unit;
                            Qmat[j * Dimension::FF + ii] *= std::sqrt(w[j]);
                        }
                    }
                    N_mode++;
                } else if (flag == "L") {
                    for (int ik = 0; ik < Dimension::FF; ++ik) {
                        if (ifs >> val) {
                            Qmat[j * Dimension::FF + ik] = val / H_unit;
                            Qmat[j * Dimension::FF + ik] *= std::sqrt(w[j]);
                        }
                    }
                } else {  // not modified with sqrt(w)
                    for (int ik = 0; ik < Dimension::FF; ++ik) {
                        if (ifs >> val) { Qmat[j * Dimension::FF + ik] = val / H_unit; }
                    }
                }
            }
            ifs.close();
        }
    }

    /// 2) init Bath sub-kernel (declaration & call)
    x0      = DS->def(DATA::model::x0);
    p0      = DS->def(DATA::model::p0);
    x_sigma = DS->def(DATA::model::x_sigma);
    p_sigma = DS->def(DATA::model::p_sigma);
    switch (lvcm_type) {
        case LVCMPolicy::CRC2:
        case LVCMPolicy::CRC5: {
            double reqb[5] = {0.0f, 14.3514f, -9.9699f, -7.0189f, 0.0f};
            double alpw[5] = {0.4501f, 0.4286f, 0.6204f, 0.4535f, 0.5539f};
            for (int j = 0; j < Dimension::N; ++j) {
                x0[j]      = reqb[j] / sqrt(w[j]);
                p0[j]      = 0.0f;
                x_sigma[j] = alpw[j] / sqrt(w[j]);
                p_sigma[j] = 0.5f * sqrt(w[j]) / alpw[j];
            }
            break;
        }
        default: {
            for (int j = 0; j < Dimension::N; ++j) {
                x0[j]      = 0.0f;
                p0[j]      = 0.0f;
                x_sigma[j] = sqrt(0.5f / w[j]);
                p_sigma[j] = sqrt(0.5f * w[j]);
            }
            break;
        }
    }

    for (int j = 0, jk = 0; j < Dimension::N; ++j) {
        for (int k = 0; k < Dimension::N; ++k, ++jk) {
            Kmat[jk] = (j == k) ? w[j] * w[j] : 0.0e0;
            Tmod[jk] = (j == k) ? 1.0e0 : 0.0e0;
        }
    }

    // model field
    mass = DS->def(DATA::model::mass);
    for (int j = 0; j < Dimension::N; ++j) mass[j] = 1.0f;
    vpes = DS->def(DATA::model::vpes);
    grad = DS->def(DATA::model::grad);
    hess = DS->def(DATA::model::hess);
    V    = DS->def(DATA::model::V);
    dV   = DS->def(DATA::model::dV);
    // ddV  = DS->def(DATA::model::ddV);
    // init & integrator
    x = DS->def(DATA::integrator::x);
    p = DS->def(DATA::integrator::p);
}

Status &Model_LVCM::initializeKernel_impl(Status &stat) { return stat; }

Status &Model_LVCM::executeKernel_impl(Status &stat) {
    for (int iP = 0; iP < Dimension::P; ++iP) {
        auto x    = this->x.subspan(iP * Dimension::N, Dimension::N);
        auto vpes = this->vpes.subspan(iP, 1);
        auto grad = this->grad.subspan(iP * Dimension::N, Dimension::N);
        auto hess = this->hess.subspan(iP * Dimension::NN, Dimension::NN);
        auto V    = this->V.subspan(iP * Dimension::FF, Dimension::FF);
        auto dV   = this->dV.subspan(iP * Dimension::NFF, Dimension::NFF);

        // calculate nuclear vpes and grad
        double term = 0.0f;
        for (int j = 0; j < Dimension::N; ++j) {
            term += w[j] * w[j] * x[j] * x[j];
            grad[j] = w[j] * w[j] * x[j];
        }
        vpes[0] = 0.5 * term;

        // electronic pes
        memset(V.data(), 0, Dimension::FF * sizeof(kids_real));
        for (int ik = 0; ik < Dimension::FF; ++ik) V[ik] = Hsys[ik];
        // ARRAY_SHOW(V, Dimension::F, Dimension::F);

        for (int j = 0, jFF = 0; j < N_mode; ++j, jFF += Dimension::FF) {
            for (int i = 0, ii = 0; i < Dimension::F; ++i, ii += Dimension::Fadd1) {
                V[ii] += Qmat[jFF + ii] * x[j];  // fast
            };
        }
        for (int j = N_mode, jik = N_mode * Dimension::FF; j < Dimension::N; ++j) {
            for (int ik = 0; ik < Dimension::FF; ++ik, ++jik) {
                V[ik] += Qmat[jik] * x[j];  // slow
            };
        }

        // ARRAY_SHOW(V, Dimension::F, Dimension::F);

        if (count_exec == 0) {
            for (int i = 0; i < Dimension::NFF; ++i) dV[i] = Qmat[i];
            // ddV = 0;
        }
    }
    return stat;
}


};  // namespace PROJECT_NS
