#include "bath.h"

#include <map>
#include <sstream>

#include "../../utils/definitions.h"
#include "../nad_forcefield/hamiltonian_data.h"

Bath::Bath(const Param& bparm, const int& nbath_in, const int& F_in) : Model(bparm) {
    try {
        // read omegac
        omegac = Param_GetQ(phys::energy_d, bparm, "omegac", 1.0f);
        CHECK_GT(omegac, 0);

        // read lambda (proportional to reorganization energy)
        int lamb_type = BathPolicy::_dict_lamb.at(Param_GetT(std::string, bparm, "lamb_flag", "lambda"));
        switch (lamb_type) {
            case BathPolicy::lambUsed: {
                double strength = Param_GetQ(phys::energy_d, bparm, "strength", 1.0f);
                lambda          = strength;
                break;
            }
            case BathPolicy::alphaUsed: {
                double strength = Param_GetQ(phys::none_d, bparm, "strength", 1.0f);  // dimensionless
                lambda          = 0.5f * omegac * strength;
                break;
            }
            case BathPolicy::etaUsed: {
                double strength = Param_GetQ(phys::energy_d, bparm, "strength", 1.0f);
                lambda          = 0.5f * strength;
                break;
            }
            case BathPolicy::ergUsed: {
                double strength = Param_GetQ(phys::energy_d, bparm, "strength", 1.0f);
                lambda          = 0.25f * strength;
                break;
            }
            default:
                LOG(FATAL) << "Unknown st_flag";
        }
        CHECK_GT(lambda, 0);

        // read temperature
        double temp = Param_GetQ(phys::temperature_d, bparm, "temp", 1.0f);
        beta        = 1.0f / (phys::au::k * temp);  // never ignore k_Boltzman
        CHECK_GT(beta, 0);

        // read coupling flag
        coup_flag = Param_GetT(std::string, bparm, "coup_flag", "#se");
        coup_type = BathPolicy::_dict_coup.at(coup_flag);

        // read spectrum flag
        spec_flag = Param_GetT(std::string, bparm, "spec_flag", "read");
        spec_type = BathPolicy::_dict_spec.at(spec_flag);

    } catch (const std::runtime_error& e) { LOG(FATAL) << "Fail to parse spectrum parameters!"; }

    // allocate Q tensor (the coupling ternsor) & initialization
    nbath = nbath_in, F = F_in;
    ALLOCATE_PTR_TO_VECTOR(Q, nbath * F * F);
    memset(Q, 0, nbath * F * F * sizeof(num_real));

    switch (coup_type) {
        case BathPolicy::SpinBosonCoup: {
            CHECK_EQ(nbath, 1);
            CHECK_EQ(F, 2);                                       // nbath must be 1, F must be 2
            Q[0] = 1.0f, Q[1] = 0.0f, Q[2] = 0.0f, Q[3] = -1.0f;  // sigmaz
            break;
        }
        case BathPolicy::SiteExcitonCoup: {
            // always be careful with nbath and F
            CHECK_LE(nbath, F);  // nbath should equal or less than F. (a ground state can be placed in last)
            for (int i = 0, idx = 0; i < nbath; ++i) {
                for (int j = 0; j < F; ++j) {
                    for (int k = 0; k < F; ++k, ++idx) Q[idx] = (i == j && i == k) ? 1.0f : 0.0f;
                }
            }
            break;
        }
        case BathPolicy::GeneralCoup:
        default: {
            std::ifstream ifs(coup_flag);
            num_real tmp;
            for (int i = 0; i < nbath * F * F; ++i)
                if (ifs >> tmp) Q[i] = tmp;
            ifs.close();
        }
    }
}

Bath::~Bath() { delete[] Q; }

int Bath::Discretization(double* W_arr, double* C_arr, const int& Nb) {
    switch (spec_type) {
        case BathPolicy::DebyeSpec: {  // from continuous spectrum
            for (int j = 0; j < Nb; ++j) {
                W_arr[j] = omegac * std::tan(phys::math::halfpi * ((j + 1.0f) / (Nb + 1.0f)));
                C_arr[j] = sqrt(2 * lambda / (Nb + 1.0f)) * W_arr[j];
            }
            break;
        }
        case BathPolicy::OhmicSpec: {  // from continuous spectrum
            for (int j = 0; j < Nb; ++j) {
                W_arr[j] = -omegac * std::log(1.0f - (j + 1.0f) / (Nb + 1.0f));
                C_arr[j] = sqrt(2 * lambda / (Nb + 1.0f)) * W_arr[j];
            }
            break;
        }
        case BathPolicy::ClosureSpec: {  // from continuous spectrum
            // read from "spec.dat", but not here, but in HEOM_Solver; so do nothing
            break;
        }
        case BathPolicy::HuangRhysSpec: {
            if (spec_flag == "#b850") {
                int innerNb = sizeof(B850_spectrum_data) / sizeof(B850_spectrum_data[0]) / 2;
                CHECK_EQ(innerNb, Nb);
                for (int j = 0, idx = 0; j < Nb; ++j) {
                    double w_unit = phys::au_2_wn;
                    W_arr[j]      = B850_spectrum_data[idx++] / w_unit;  // cm-1
                    double SHR    = B850_spectrum_data[idx++];           // dimensionless (Huang-Rhys factor)
                    C_arr[j]      = 2.0f * sqrt(0.5f * W_arr[j] * W_arr[j] * W_arr[j] * SHR);
                }
            } else if (spec_flag == "#pbi") {
                int innerNb = sizeof(PBI_spectrum_data) / sizeof(PBI_spectrum_data[0]) / 2;
                CHECK_EQ(innerNb, Nb);
                for (int j = 0, idx = 0; j < Nb; ++j) {
                    double w_unit = phys::au_2_wn;
                    W_arr[j]      = PBI_spectrum_data[idx++] / w_unit;  // cm-1
                    double SHR    = PBI_spectrum_data[idx++];           // dimensionless (Huang-Rhys factor)
                    C_arr[j]      = 2.0f * sqrt(0.5f * W_arr[j] * W_arr[j] * W_arr[j] * SHR);
                }
            } else {  // read from "spec.dat"
                try {
                    std::ifstream ifs("spec.dat");
                    std::string firstline, str;

                    // get unit of frequency
                    getline(ifs, firstline);
                    std::stringstream sstr(firstline);
                    sstr >> str;
                    phys::uval uw = phys::us::parse(str);
                    if (uw.dim != phys::energy_d) LOG(FATAL) << "need dimension of energy";
                    double w_unit = phys::us::conv(phys::au::unit, uw);

                    double val;
                    for (int j = 0; j < Nb; ++j) {
                        if (ifs >> val) W_arr[j] = val / w_unit;
                        if (ifs >> val) C_arr[j] = 2.0f * sqrt(0.5f * W_arr[j] * W_arr[j] * W_arr[j] * val);
                    }
                } catch (std::runtime_error& e) { LOG(FATAL) << "read spec.dat error"; }
            }
            break;
        }
        case BathPolicy::GFactorSpec: {
            if (spec_flag == "#rub") {
                int innerNb = sizeof(Rubrene_spectrum_data) / sizeof(Rubrene_spectrum_data[0]) / 2;
                CHECK_EQ(innerNb, Nb);
                for (int j = 0, idx = 0; j < Nb; ++j) {
                    double w_unit  = phys::au_2_ev;
                    W_arr[j]       = Rubrene_spectrum_data[idx++] / w_unit;  // eV
                    double gfactor = Rubrene_spectrum_data[idx++];           // dimensionless (gfactor)
                    C_arr[j]       = 2.0f * sqrt(0.5f * W_arr[j] * W_arr[j] * W_arr[j]) * gfactor;
                }
            } else {  // read from spec.dat
                try {
                    std::ifstream ifs("spec.dat");
                    std::string firstline, str;

                    // get unit of frequency
                    getline(ifs, firstline);
                    std::stringstream sstr(firstline);
                    sstr >> str;
                    phys::uval uw = phys::us::parse(str);
                    if (uw.dim != phys::energy_d) LOG(FATAL) << "need dimension of energy";
                    double w_unit = phys::us::conv(phys::au::unit, uw);

                    double val;
                    for (int j = 0; j < Nb; ++j) {
                        if (ifs >> val) W_arr[j] = val / w_unit;  ///< omegac ~ [energy_d]
                        if (ifs >> val) C_arr[j] = 2.0f * sqrt(0.5f * W_arr[j] * W_arr[j] * W_arr[j]) * val;
                    }
                } catch (std::runtime_error& e) { LOG(FATAL) << "read spec.dat error"; }
            }
            break;
        }
        case BathPolicy::ReadSpec:
        default: {
            try {
                std::ifstream ifs("spec.dat");
                std::string firstline, str;

                // get unit of frequency & coefficients
                getline(ifs, firstline);
                std::stringstream sstr(firstline);
                sstr >> str;
                phys::uval uw = phys::us::parse(str);
                if (uw.dim != phys::energy_d) LOG(FATAL) << "need dimension of energy";
                double w_unit = phys::us::conv(phys::au::unit, uw);
                sstr >> str;
                phys::uval uc = phys::us::parse(str);
                if (uc.dim != phys::energy_d) LOG(FATAL) << "need dimension of energy";
                double c_unit = phys::us::conv(phys::au::unit, uc);
                // read rest
                double val;
                for (int j = 0; j < Nb; ++j) {
                    if (ifs >> val) W_arr[j] = val / w_unit;              ///< omegac ~ [energy_d]
                    if (ifs >> val) C_arr[j] = val / pow(c_unit, 1.5e0);  ///< coeffs ~ [energy_d]**1.5
                }
            } catch (std::runtime_error& e) { LOG(FATAL) << "read spec.dat error"; }
        }
    }
    return 0;
}

double Bath::J_Ohmic(const double& w) { return phys::math::pi * lambda / omegac * w * std::exp(-w / omegac); }

double Bath::J_Debye(const double& w) { return 2 * lambda * omegac * w / (w * w + omegac * omegac); }

double Bath::J_Refit(const double& w, double* W_arr, double* C_arr, const int& Nb) {
    if (W_arr[Nb - 1] / w < 10e0) LOG(FATAL) << "inaccurate fitting! please increase Nb";

    double a  = 10e0 * Nb * Nb / (W_arr[Nb - 1] * W_arr[Nb - 1]);
    double Jw = 0.0;
    for (int j = 0; j < Nb; ++j) {
        Jw += C_arr[j] * C_arr[j] / W_arr[j] * std::exp(-a * (w - W_arr[j]) * (w - W_arr[j]));
    }
    return 0.5e0 * phys::math::halfpi * std::sqrt(a / phys::math::pi) * Jw;
}

double Bath::J(const double& w, double* W_arr, double* C_arr, const int& Nb) {
    if (w < 0) return -J(-w, W_arr, C_arr, Nb);
    double Jw;
    switch (spec_type) {
        case BathPolicy::DebyeSpec: {  // from continuous spectrum
            Jw = J_Debye(w);
            break;
        }
        case BathPolicy::OhmicSpec: {  // from continuous spectrum
            Jw = J_Ohmic(w);
            break;
        }
        default: {
            if (W_arr == nullptr || C_arr == nullptr || Nb <= 0) LOG(FATAL);
            Jw = J_Refit(w, W_arr, C_arr, Nb);
        }
    }
    return Jw;
}

double Bath::fun_Cw_Re(const double& w, double* W_arr, double* C_arr, const int& Nb) {
    constexpr double dw = 1.0e-5;
    if (w == 0) return (J(dw, W_arr, C_arr, Nb) - J(-dw, W_arr, C_arr, Nb)) / (2 * beta * dw);
    return J(w, W_arr, C_arr, Nb) * (1 + 1 / (exp(beta * w) - 1.0f));
}

int Bath::fun_Cw(num_complex* Cw, double* w, const int& Nw, double* W_arr, double* C_arr, const int& Nb) {
    double dws = omegac / 1000;

    for (int i = 0; i < Nw; ++i) {
        Cw[i] = fun_Cw_Re(w[i], W_arr, C_arr, Nb);
        if (w[i] != 0) {  // imaginary part is non-zero
            num_complex sum = 0.0e0, tmp = 1.0e0;
            int k = 1;
            while (std::abs(tmp) > 1.0e-6 || std::abs(tmp / sum) > 1.0e-8) {
                num_complex Cplus  = fun_Cw_Re(w[i] + k * dws, W_arr, C_arr, Nb);
                num_complex Cminus = fun_Cw_Re(w[i] - k * dws, W_arr, C_arr, Nb);
                tmp                = (Cplus - Cminus) / (double) k;
                sum                = sum + tmp;
                k++;
            }
            Cw[i] += phys::math::im / phys::math::pi * (-sum);
        }
    }
    return 0;
}
