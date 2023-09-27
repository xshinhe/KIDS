#include "solver_heom.h"

#include <omp.h>

#include <complex>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "../solvers/solver.h"
#include "../utils/definitions.h"
#include "../utils/sparse_utils.h"

using namespace ARRAY_EG;

// Commutator: Q2 = A*Q1 - Q1*A
template <typename T>
int Liouvillian_Comm(T* L, T* A, const int& F) {
    int FF   = F * F;
    int FFFF = FF * FF;
    for (int i = 0; i < FFFF; ++i) L[i] = 0;
    for (int i = 0; i < F; ++i) {
        for (int j = 0; j < F; ++j) {
            for (int k = 0; k < F; ++k) {
                int idxA = i * F + j, idxQ1 = j * F + k, idxQ2 = i * F + k;
                L[idxQ2 * FF + idxQ1] += A[idxA];
                L[idxQ2 * FF + idxA] -= A[idxQ1];
            }
        }
    }
    return 0;
}

// Anti-Commutator: Q2 = A*Q1 + Q1*A
template <typename T>
int Liouvillian_Acom(T* L, T* A, const int& F) {
    int FF   = F * F;
    int FFFF = FF * FF;
    for (int i = 0; i < FFFF; ++i) L[i] = 0;
    for (int i = 0; i < F; ++i) {
        for (int j = 0; j < F; ++j) {
            for (int k = 0; k < F; ++k) {
                int idxA = i * F + j, idxQ1 = j * F + k, idxQ2 = i * F + k;
                L[idxQ2 * FF + idxQ1] += A[idxA];
                L[idxQ2 * FF + idxA] += A[idxQ1];
            }
        }
    }
    return 0;
}


HEOM_Solver::HEOM_Solver(Param iparm, Model* pM) : Solver(iparm, pM) {
    pForceField = dynamic_cast<SystemBath_ForceField*>(pM);  // casted into child class SystemBath_ForceField

    // get sizes
    F     = pForceField->get_F();
    nbath = pForceField->get_nbath();
    FF    = F * F;
    Param_GetV(H, iparm, 1);

    num_real tmp;
    ALLOCATE_PTR_TO_VECTOR(Hsys, FF);
    ALLOCATE_PTR_TO_VECTOR(eac0, F);
    ALLOCATE_PTR_TO_VECTOR(rho0, FF);
    ALLOCATE_PTR_TO_VECTOR(Hsys_time, FF);
    num_complex* tmpQi      = new num_complex[FF];
    num_complex* tmpL       = new num_complex[FF * FF];
    num_complex* tmpL_1     = new num_complex[FF * FF];
    num_complex* tmpL_2     = new num_complex[FF * FF];
    num_complex* tmpL_saveH = new num_complex[FF * FF];

    ALLOCATE_PTR_TO_VECTOR(Esys, F);
    ALLOCATE_PTR_TO_VECTOR(Tsys, FF);
    ALLOCATE_PTR_TO_VECTOR(T0, FF);

    // get numerical simulation details
    Param_GetV(tend, iparm, 0.0f);
    Param_GetV(dt, iparm, -1.0f);
    Param_GetV(sstep, iparm, 1);
    if (pForceField->Suggest_tend() > 0.0f) tend = pForceField->Suggest_tend();
    if (pForceField->Suggest_dt() > 0.0f) dt = pForceField->Suggest_dt();
    vscale = 1.0f;
    if (dt > 0.1f) {
        LOG(WARNING) << "Large dt will cause unconvergece for RK-4 expansion, so do scaling on dt=0.1";
        vscale = 0.01f / dt;   // remembering [M] ~ [L-1] ~ [T-1]
        dt     = vscale * dt;  // = 0.01
        tend   = vscale * tend;
    }
    nstep = sstep * (int(tend / (sstep * dt)) + 1);  // make to over off
    CHECK_GT(tend, 0);
    CHECK_GT(dt, 0);
    CHECK_GT(sstep, 0);
    CHECK_GT(nstep, 0);

    // read Hsys
    // for(int i=0; i<FF; ++i) Hsys[i] = HFMO_data[i];
    for (int i = 0; i < FF; ++i) Hsys[i] = pForceField->Hsys[i] / vscale;
    Liouvillian_Comm(tmpL, Hsys, F);
    std::cout << "Print Hilbertian for Hsys" << std::endl;
    // ARRAY_SHOW(Hsys, F, F);

    // ask initial rep. transform
    Param_GetV(adia_pure, iparm, 0);
    if (adia_pure == 0) {
        for (int i = 0, idx = 0; i < F; ++i)
            for (int j = 0; j < F; ++j, ++idx) T0[idx] = (i == j) ? 1.0f : 0.0f;
    } else if (adia_pure == 1) {
        EigenSolve(Esys, Tsys, Hsys, F);
        std::cout << "Print Hsys EigenSolve problem" << std::endl;
        // ARRAY_SHOW(Esys, F, 1);
        // ARRAY_SHOW(Tsys, F, F);
        for (int i = 0; i < FF; ++i) T0[i] = Tsys[i];
    } else if (adia_pure == -1) {
        CHECK_EQ(F, 2);
        double rotangle = Param_GetV(rotangle, iparm, 0.0f) * phys::math::twopi;
        T0[0] = std::cos(rotangle), T0[1] = -std::sin(rotangle), T0[2] = std::sin(rotangle), T0[3] = std::cos(rotangle);
    } else {
        std::string T_read_file = Param_GetT(std::string, iparm, "T_read_file", "T0.dat");
        try {
            std::ifstream ifs("T0.dat");
            double tmp;
            for (int i = 0; i < FF; ++i)
                if (ifs >> tmp) T0[i] = tmp;
            ifs.close();
        } catch (std::runtime_error& e) { LOG(FATAL) << e.what(); }
    }

    for (int i = 0; i < FF * FF; ++i) tmpL[i] = -phys::math::im * tmpL[i];  // -i*LHsys
    LHsys = new SparseMat<num_complex>(tmpL, FF, FF);
    std::cout << "Print Liouvillian for Hsys" << std::endl;
    LHsys->Show();
    for (int i = 0; i < FF * FF; ++i) tmpL_saveH[i] = tmpL[i];  // save a copy of -i*LHsys

    // determine the tcf expansion number N
    int sptype = pForceField->mybath->spec_type;
    if (sptype == BathPolicy::DebyeSpec) {
        BasisExpanType = BasisExpan_Matsubara;  // HEOM
        Param_GetV(Nexpan_M, iparm, 0);         // matsubara expansion numbers, default 0
        Nexpan_Nr = Nexpan_M + 1;
        Nexpan_Ni = 0;
        N         = nbath * (Nexpan_M + 1);
    } else {
        BasisExpanType = BasisExpan_Closure;  // eHEOM
        int Nexpan_Nr  = Param_GetT(int, iparm, "Nr");
        int Nexpan_Ni  = Param_GetT(int, iparm, "Ni");
        N              = nbath * (Nexpan_Nr + Nexpan_Ni);
        Param_GetV(basis_file, iparm, "bath");
    }
    std::cout << "Total tcf basis: N=" << N << std::endl;

    ALLOCATE_PTR_TO_VECTOR(Tcftype, N);
    ALLOCATE_PTR_TO_VECTOR(tcf_site, N);
    ALLOCATE_PTR_TO_VECTOR(tcf_coef, N);
    ALLOCATE_PTR_TO_VECTOR(tcf_zero, N);
    ALLOCATE_PTR_TO_VECTOR(tcf_deri, N * N);
    ARRAY_CLEAR(tcf_deri, N * N);
    SQ_list   = new SparseMat<num_complex>*[nbath];
    LQ_list   = new SparseMat<num_complex>*[nbath];
    LQLQ_list = new SparseMat<num_complex>*[nbath];
    for (int ibath = 0, Qidx = 0, cnt = 0; ibath < nbath; ++ibath) {
        int ib = cnt;
        for (int j = 0; j < Nexpan_Nr; ++j, ++cnt) {  // count for real basis (but mixed for matsubara expansion)
            tcf_site[cnt] = ibath;
            Tcftype[cnt]  = (BasisExpanType == BasisExpan_Matsubara) ? TCF_xtype : TCF_rtype;
        }
        for (int j = 0; j < Nexpan_Ni; ++j, ++cnt) {  // count for imag basis
            tcf_site[cnt] = ibath;
            Tcftype[cnt]  = TCF_itype;
        }

        double dphs_coeff = 0.0f;  // coefficients for dephase term for ibath-th bath
        Init_Bath(ib, Nexpan_Nr, Nexpan_Ni, dphs_coeff, tcf_coef, tcf_zero, tcf_deri);

        for (int j = 0; j < FF; ++j, ++Qidx) tmpQi[j] = pForceField->Q[Qidx];  // Qi
        std::cout << "Print Hilbertian for No." << ibath << " Q interaction Matrix" << std::endl;
        // ARRAY_SHOW(tmpQi, F, F);

        Liouvillian_Acom(tmpL, tmpQi, F);  // S_Qi
        SQ_list[ibath] = new SparseMat<num_complex>(tmpL, FF, FF);

        Liouvillian_Comm(tmpL, tmpQi, F);  // -iL_Qi
        for (int i = 0; i < FF * FF; ++i) tmpL[i] = -phys::math::im * tmpL[i];
        LQ_list[ibath] = new SparseMat<num_complex>(tmpL, FF, FF);

        ARRAY_MATMUL(tmpL_1, tmpL, tmpL, FF, FF, FF);  // (-i)L_Qi * (-i)L_Qi = -LQi*LQi
        LQLQ_list[ibath] = new SparseMat<num_complex>(tmpL_1, FF, FF);
        for (int j = 0; j < FF * FF; ++j) tmpL_2[j] += dphs_coeff * tmpL_1[j];

        std::cout << "Print SQ for No." << ibath << " Q interaction Matrix" << std::endl;
        SQ_list[ibath]->Show();
        std::cout << "Print -iLQ for No." << ibath << " Q interaction Matrix" << std::endl;
        LQ_list[ibath]->Show();
        std::cout << "Print -LQLQ for No." << ibath << " Q interaction Matrix" << std::endl;
        LQLQ_list[ibath]->Show();
    }
    // ARRAY_SHOW(tmpL_2,FF,FF);
    ALQLQ = new SparseMat<num_complex>(tmpL_2, FF, FF);  // (-)*sum_i G_i L_Qi L_Qi
    std::cout << "Print total LQLQ for Q interaction Matrix" << std::endl;
    ALQLQ->Show();
    for (int i = 0; i < FF * FF; ++i) tmpL_2[i] += tmpL_saveH[i];
    LHsysDphs = new SparseMat<num_complex>(tmpL_2, FF, FF);  // -iLHsys + (-)*sumi Gi L_Qi L_Qi
    std::cout << "Print total LHsysDphs" << std::endl;
    LHsysDphs->Show();

    std::cout << std::setprecision(9) << std::setiosflags(std::ios::scientific) << "Print coef basis" << std::endl;
    for (int i = 0; i < N; ++i) std::cout << "\t" << tcf_coef[i];
    std::cout << std::endl;
    std::cout << "Print zero basis" << std::endl;
    for (int i = 0; i < N; ++i) std::cout << "\t" << tcf_zero[i];
    std::cout << std::endl;
    std::cout << "Print deri basis" << std::endl;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) std::cout << "\t" << tcf_deri[i * N + j];
        std::cout << std::endl;
    }

    // @TODO OPT
    Nchs = N + H + 10;
    CNK  = new long long int*[Nchs];  // save most useful C(N,k)
    for (int i = 0; i < Nchs; ++i) CNK[i] = new long long int[i + 1];
    std::cout << "\nN choose K: (N,K) structure" << std::endl;
    for (int n = 0; n < Nchs; ++n) {
        for (int k = 0; k <= n; ++k) {
            CNK[n][k] = (k == 0 || k == n) ? 1 : CNK[n - 1][k] + CNK[n - 1][k - 1];
            std::cout << CNK[n][k] << " ";
        }
        std::cout << std::endl;
    }

    std::string suffix = std::to_string(H);
    save               = save + "_" + HEOM_Solver::name() + suffix + "_" + pForceField->tag;

    delete[] tmpQi, tmpL, tmpL_1, tmpL_2, tmpL_saveH;
    Generate_ADOs();
};

HEOM_Solver::~HEOM_Solver() {
    delete ALQLQ, delete LHsys, delete LHsysDphs;
    for (int i = 0; i < nbath; ++i) {
        delete LQ_list[i];
        delete SQ_list[i];
        delete LQLQ_list[i];
    }
    delete[] LQ_list, delete[] SQ_list, delete[] LQLQ_list;
    for (int i = 0; i < Nchs; ++i) delete[] CNK[i];
    delete[] CNK;
};

int HEOM_Solver::Init_Bath(const int& ib, const int& Nr, const int& Ni, num_real& gdph_m, num_complex* tcf_coef,
                           num_complex* tcf_zero, num_complex* tcf_deri) {
    if (BasisExpanType == BasisExpan_Matsubara) {  // by default Debye Spectrum
        double omegac = pForceField->mybath->omegac / vscale;
        double lambda = pForceField->mybath->lambda / vscale;
        double beta   = pForceField->mybath->beta * vscale;

        // real part & dephasing by cutoff real part.
        tcf_coef[ib] = 1.0f;
        tcf_zero[ib] = lambda * omegac / tan(0.5f * beta * omegac);
        gdph_m       = 2 * lambda / (beta * omegac) - lambda / tan(0.5f * beta * omegac);
        for (int j = 1; j < Nr; ++j) {  // here Nr = M+1
            tcf_coef[ib + j] = 1.0f;
            tcf_zero[ib + j] = (4.0f * lambda / beta) * omegac * (j * phys::math::twopi / beta) /
                               ((j * phys::math::twopi / beta) * (j * phys::math::twopi / beta) - omegac * omegac);
            gdph_m -= (4.0f * lambda / beta) * omegac /
                      ((j * phys::math::twopi / beta) * (j * phys::math::twopi / beta) - omegac * omegac);
        }
        // real part mixed with imag part
        tcf_zero[ib] += -lambda * omegac * phys::math::im;  // values save in imag part

        // expansion of deriative on basis
        for (int i = 0; i < Nr; ++i) {  // real part
            for (int j = 0; j < Nr; ++j)
                tcf_deri[(ib + i) * N + (ib + j)] =
                    (i == j) ? ((j == 0) ? (-omegac) : (-j * phys::math::twopi / beta)) : 0.0f;
        }
        // imag part (is skipped for Ni=0 that imag part can be merged into real part)
    } else {  // read eHEOM closure
        /*
            Noting Dimension in [E]: coeff is [0], zero is [2], deri is [1]
            please prepare these files correctly as your io_enegyunit
        */
        gdph_m = 0.0f;  // !!! No dephasing approximation
        num_real tmp;
        std::ifstream ifs;
        ifs.open(basis_file + ".cr.in");
        for (int i = 0; i < Nr; ++i) ifs >> tcf_coef[ib + i];
        ifs.close();
        ifs.open(basis_file + ".ci.in");
        for (int i = 0; i < Ni; ++i) ifs >> tcf_coef[ib + Nr + i];
        ifs.close();
        ifs.open(basis_file + ".zr.in");
        for (int i = 0; i < Nr; ++i)
            if (ifs >> tmp) tcf_zero[ib + i] = tmp / (vscale * vscale);  // values save in real part
        ifs.close();
        ifs.open(basis_file + ".zi.in");
        for (int i = 0; i < Ni; ++i)
            if (ifs >> tmp)
                tcf_zero[ib + Nr + i] = phys::math::im * tmp / (vscale * vscale);  // values save in imag part
        ifs.close();
        ifs.open(basis_file + ".dr.in");
        for (int i = 0; i < Nr; ++i) {
            for (int j = 0; j < Nr; ++j)
                if (ifs >> tmp) tcf_deri[(ib + i) * N + (ib + j)] = tmp / vscale;
        }
        ifs.close();
        ifs.open(basis_file + ".di.in");
        for (int i = 0; i < Ni; ++i) {
            for (int j = 0; j < Ni; ++j)
                if (ifs >> tmp) tcf_deri[(ib + Nr + i) * N + (ib + Nr + j)] = tmp / vscale;
        }
        ifs.close();
    }
    return 0;
}

int HEOM_Solver::Generate_ADOs() {  //@TODO
    // geuss the maximum size
    long long int Nmax_ADOs = nchoosek(N + H, H);  // Max ADOs including zeroth layer before filtration
    long long int Nmax_conn = 50 * Nmax_ADOs;
    csr_iADOs               = new int[Nmax_ADOs];
    csr_connec_LD           = new int[Nmax_ADOs];
    csr_connec              = new int[Nmax_conn];
    csr_type                = new int[Nmax_conn];
    csr_ivalue              = new int[Nmax_conn];
    csr_cvalue              = new num_complex[Nmax_conn];
    int* arr                = new int[N];
    int* arrp               = new int[N];

    std::cout << "\nGenerating ADOs ..." << std::endl;
    clock_t start, end;
    start     = clock();
    int iconn = 0, iADOs = 0;
    int sstep = (Nmax_ADOs / 100 > 1) ? Nmax_ADOs / 100 : 1;
    for (long long int i = 0; i < Nmax_ADOs; ++i) {
        if (csr_iADOs[i] == -1) continue;  // which has been filter by (h+1) finding
        // otherwise, it should be included in ADOs

        csr_iADOs[i] = iADOs;  // through i (index before filter) to find iADOs (index after filter)
        findarr(arr, i, N);    // find array like (0,0,0,0,...,0)
        int h = 0;
        for (int k = 0; k < N; ++k) h += arr[k];  // determine the layer order (h order = h index + 1)

        // find connections in (h-1) layers
        if (h > 0) {
            for (int k = N - 1; k >= 0; --k) {  // in this order
                if (arr[k] == 0) continue;
                // if(arr[k] == 0 || NORM_OF(tcf_zero[k]) < phys::math::eps8) continue;

                for (int j = 0; j < N; ++j) arrp[j] = arr[j];
                arrp[k] -= 1;
                int y             = findpos(arrp, N);
                csr_connec[iconn] = y;
                csr_type[iconn]   = -1;
                csr_ivalue[iconn] = k;
                csr_cvalue[iconn] = (num_real) arr[k] * tcf_zero[k];
                iconn++;
            }
        }

        // same order (h) layers but before i-th ADO, (k2 > k1 case)
        for (int k1 = 0; k1 < N; ++k1) {           // creation of k1
            for (int k2 = N - 1; k2 > k1; --k2) {  // annihalitio of k2
                // if(arr[k2] == 0) continue;
                if (arr[k2] == 0 || NORM_OF(tcf_deri[k2 * N + k1]) < phys::math::eps8) continue;

                for (int j = 0; j < N; ++j) arrp[j] = arr[j];
                arrp[k1] += 1;
                arrp[k2] -= 1;
                int y             = findpos(arrp, N);
                csr_connec[iconn] = y;
                csr_type[iconn]   = 0;
                csr_ivalue[iconn] = -1;  // pre-same order
                csr_cvalue[iconn] = (num_real) arr[k2] * tcf_deri[k2 * N + k1];
                iconn++;
            }
        }

        // connection to itself
        csr_connec[iconn] = i;
        csr_type[iconn]   = 0;
        csr_ivalue[iconn] = 0;  // itself
        num_complex kw    = 0;
        for (int j = 0; j < N; ++j) kw += (num_real) arr[j] * tcf_deri[j * (N + 1)];
        csr_cvalue[iconn] = kw;
        iconn++;

        // same order (h) layers but after i-th ADO, (k2 < k1 case)
        for (int k2 = N - 1; k2 >= 0; --k2) {      // annihalitio of k2
            for (int k1 = k2 + 1; k1 < N; ++k1) {  // creation of k1
                // if(arr[k2] == 0) continue;
                if (arr[k2] == 0 || NORM_OF(tcf_deri[k2 * N + k1]) < phys::math::eps8) continue;

                for (int j = 0; j < N; ++j) arrp[j] = arr[j];
                arrp[k1] += 1;
                arrp[k2] -= 1;
                int y             = findpos(arrp, N);
                csr_connec[iconn] = y;
                csr_type[iconn]   = 0;
                csr_ivalue[iconn] = +1;  // post-same order
                csr_cvalue[iconn] = (num_real) arr[k2] * tcf_deri[k2 * N + k1];
                iconn++;
            }
        }

        // find connections in (h+1) layers
        if (h < H) {
            for (int k = 0; k < N; ++k) {
                for (int j = 0; j < N; ++j) arrp[j] = arr[j];
                arrp[k] += 1;
                int y = findpos(arrp, N);
                if (if_filteration(arrp)) {
                    csr_iADOs[y] = -1;
                    continue;
                }

                csr_connec[iconn] = y;
                csr_type[iconn]   = 1;
                csr_ivalue[iconn] = k;
                csr_cvalue[iconn] = tcf_coef[k];
                iconn++;
            }
        }

        csr_connec_LD[iADOs] = iconn;  // record the pointer to end of connections for iADOs
        iADOs++;
        if (i % sstep == 0) {
            std::cout << i << " [x] ";
            for (int k = 0; k < N; ++k) std::cout << arr[k] << " ";
            std::cout << std::endl;
        }
    }
    NADOs = iADOs;
    Nconn = iconn;

    // re-point connection from i-th ADOs to iADOs
    for (int iconn = 0; iconn < Nconn; ++iconn) csr_connec[iconn] = csr_iADOs[csr_connec[iconn]];

    int* new_csr_connec_LD      = new int[NADOs];
    int* new_csr_connec         = new int[Nconn];
    int* new_csr_type           = new int[Nconn];
    num_complex* new_csr_cvalue = new num_complex[Nconn];

    for (iADOs = 0; iADOs < NADOs; ++iADOs) {
        int ib_conn              = (iADOs == 0) ? 0 : csr_connec_LD[iADOs - 1];
        int ie_conn              = csr_connec_LD[iADOs];
        new_csr_connec_LD[iADOs] = ie_conn;
        std::cout << iADOs << " [iADOs]: ";
        for (iconn = ib_conn; iconn < ie_conn; ++iconn) {
            if (csr_type[iconn] < 0)
                std::cout << "\033[0;1;32m" << csr_connec[iconn] << "\t";
            else if (csr_type[iconn] == 0)
                std::cout << "\033[0;1;33m" << csr_connec[iconn] << "\t";
            else
                std::cout << "\033[0;1;31m" << csr_connec[iconn] << "\t";
            new_csr_connec[iconn] = csr_connec[iconn];
            new_csr_type[iconn]   = csr_type[iconn];
            new_csr_cvalue[iconn] = csr_cvalue[iconn];
        }
        std::cout << "\033[0m" << std::endl;
    }
    delete[] csr_connec_LD, csr_connec, csr_type, csr_cvalue, arr, arrp;
    csr_connec_LD = new_csr_connec_LD;
    csr_connec    = new_csr_connec;
    csr_type      = new_csr_type;
    csr_cvalue    = new_csr_cvalue;

    std::cout << "NmaxADOs: " << Nmax_ADOs << std::endl;
    std::cout << "NADOs: " << NADOs << std::endl;
    std::cout << "NConn: " << Nconn << std::endl;
    std::cout << "Nfiler: " << Nmax_ADOs - NADOs << std::endl;

    end = clock();
    std::cout << "Generating ADOs using time = " << num_real(end - start) / CLOCKS_PER_SEC << "s" << std::endl;
    return 0;
}

int HEOM_Solver::run_impl() {
    num_real h1 = dt, h2 = h1 * dt / 2.0f, h3 = h2 * dt / 3.0f, h4 = h3 * dt / 4.0f;  //  for RK4
    int Nsigma = FF * NADOs;
    std::ofstream ofs(save);
    num_real timeunit = iou.time / vscale;  // vscale is for energy, so under division

    rdm_arr                   = new num_complex[FF * nstep];
    sigma_tot                 = new num_complex[Nsigma];
    num_complex* sigma_tot_t1 = new num_complex[Nsigma];
    num_complex* sigma_tot_t2 = new num_complex[Nsigma];
    num_complex* sigma_tot_t3 = new num_complex[Nsigma];
    num_complex* sigma_tot_t4 = new num_complex[Nsigma];
    num_complex* tmpL         = new num_complex[FF * FF];
    num_complex* workc        = new num_complex[FF];

    // initial condition
    pForceField->ForceField_init_elec(rho0, eac0, occ0, F, 0);
    if (occ0 >= 0 && occ0 < F) {
        for (int i = 0, idx = 0; i < F; ++i)
            for (int j = 0; j < F; ++j, ++idx)
                sigma_tot[idx] = (i == occ0 && j == occ0) ? phys::math::iu : phys::math::iz;
    } else if (occ0 = -1) {
        for (int i = 0, idx = 0; i < F; ++i)
            for (int j = 0; j < F; ++j, ++idx) sigma_tot[idx] = eac0[i] * CONJ_OF(eac0[j]);
    } else if (occ0 = -2) {
        for (int i = 0; i < FF; ++i) sigma_tot[i] = rho0[i];
    }
    if (adia_pure != 0) {  // from adiabatic initialization
        ARRAY_MATMUL(workc, T0, sigma_tot, F, F, F);
        ARRAY_MATMUL_TRANS2(sigma_tot, workc, T0, F, F, F);
    }

    int irho = 0;
    for (int istep = 0; istep < nstep; ++istep) {
        // @if Hamiltonian is time-dependent
        // for(int j=0;j<FF;++j) Hsys_time[j] = (j%(F+1) == 0) ? Hsys[j]: Hsys[j]*cos(istep*dt);
        // Liouvillian_Comm(tmpL, Hsys_time, F);
        // for(int i=0; i<FF*FF; ++i) tmpL[i] = -phys::math::im * tmpL[i];     // -i*LHsys
        // delete LHsysDphs; LHsysDphs = new SparseMat<num_complex>(tmpL, FF, FF);

        for (int i = 0; i < FF; ++i) rdm_arr[irho++] = sigma_tot[i];  // record rdm matrix, in sequence

        if (istep == 0) {
            ofs << FMT(0) << "stat" << FMT(8) << "time";
            for (int i1 = 0; i1 < F; ++i1)
                for (int i2 = 0; i2 < F; ++i2) {
                    ofs << FMT(8) << utils::concat("Re(", i1, ",", i2, ")");
                    ofs << FMT(8) << utils::concat("Im(", i1, ",", i2, ")");
                }
            ofs << std::endl;
        }
        if (istep % sstep == 0) {
            ofs << FMT(0) << 0 << FMT(8) << istep * dt * timeunit;
            for (int i = 0; i < FF; ++i) ofs << FMT(8) << REAL_OF(sigma_tot[i]) << FMT(8) << IMAG_OF(sigma_tot[i]);
            ofs << std::endl;
        }

        Evolve(sigma_tot, sigma_tot_t1);
        Evolve(sigma_tot_t1, sigma_tot_t2);
        Evolve(sigma_tot_t2, sigma_tot_t3);
        Evolve(sigma_tot_t3, sigma_tot_t4);

        // when d_t y = f(t,y) where f(t,y) is time-independent, the RK4 is equivalent to following approach
        for (int i = 0; i < Nsigma; ++i) {
            sigma_tot[i] += h1 * sigma_tot_t1[i] + h2 * sigma_tot_t2[i] + h3 * sigma_tot_t3[i] + h4 * sigma_tot_t4[i];
        }
    }
    ofs.close();
    delete[] sigma_tot, delete[] rdm_arr, delete[] sigma_tot_t1, delete[] sigma_tot_t2, delete[] sigma_tot_t3,
        delete[] sigma_tot_t4;
    delete[] tmpL;
    return 0;
}

int HEOM_Solver::Evolve(num_complex* sigma_tot_t1, num_complex* sigma_tot_t2) {
#pragma omp parallel for default(none) shared(sigma_tot_t1, sigma_tot_t2)
    for (int iADOs = 0; iADOs < NADOs; ++iADOs) {
        int ib_conn = (iADOs == 0) ? 0 : csr_connec_LD[iADOs - 1];
        int ie_conn = csr_connec_LD[iADOs];

        // num_complex* rho_tmp1 = sigma_tmp1 + FF*iADOs;
        // num_complex* rho_tmp2 = sigma_tmp2 + FF*iADOs;
        num_complex* rho_tmp1 = new num_complex[FF];
        num_complex* rho_tmp2 = new num_complex[FF];

        num_complex *ADOith = sigma_tot_t2 + FF * iADOs, *ADOithp = sigma_tot_t1 + FF * iADOs;

        ARRAY_CLEAR(ADOith, FF);  // set zero
        SparseMat<num_complex>* LH = LHsysDphs;
        LH->MatMul(ADOith, ADOithp);

        for (int iconn = ib_conn; iconn < ie_conn; ++iconn) {
            int jADOs           = csr_connec[iconn];
            num_complex* ADOjth = sigma_tot_t1 + FF * jADOs;

            int itype = csr_type[iconn];
            if (itype == -1) {
                int k = csr_ivalue[iconn], it = Tcftype[k], iQ = tcf_site[k];
                num_complex coeff         = csr_cvalue[iconn];
                SparseMat<num_complex>*L1 = LQ_list[iQ], *L2 = SQ_list[iQ];
                switch (it) {
                    case (TCF_rtype):
                        L1->MatMul(rho_tmp1, ADOjth);
                        for (int i = 0; i < FF; ++i) rho_tmp1[i] = REAL_OF(coeff) * rho_tmp1[i];
                        break;
                    case (TCF_itype):
                        L2->MatMul(rho_tmp1, ADOjth);
                        for (int i = 0; i < FF; ++i) rho_tmp1[i] = IMAG_OF(coeff) * rho_tmp1[i];
                        break;
                    case (TCF_xtype):
                        L1->MatMul(rho_tmp1, ADOjth);
                        L2->MatMul(rho_tmp2, ADOjth);
                        for (int i = 0; i < FF; ++i)
                            rho_tmp1[i] = REAL_OF(coeff) * rho_tmp1[i] + IMAG_OF(coeff) * rho_tmp2[i];
                        break;
                }
            } else if (itype == 0) {
                num_complex coeff = csr_cvalue[iconn];
                for (int i = 0; i < FF; ++i) rho_tmp1[i] = coeff * ADOjth[i];
            } else if (itype == 1) {
                int k = csr_ivalue[iconn], iQ = tcf_site[k];
                num_complex coeff         = csr_cvalue[iconn];
                SparseMat<num_complex>* L = LQ_list[iQ];
                L->MatMul(rho_tmp1, ADOjth);
                for (int i = 0; i < FF; ++i) rho_tmp1[i] = coeff * rho_tmp1[i];
            }
            for (int i = 0; i < FF; ++i) ADOith[i] += rho_tmp1[i];
        }
        delete[] rho_tmp1, delete[] rho_tmp2;
    }
    return 0;
}

bool HEOM_Solver::if_filteration(int* arr) { return false; }

long long int HEOM_Solver::nchoosek(unsigned int n, unsigned int k) {
    if (k > n) return 0;
    if (k == 0) return 1;
    if (n < Nchs) return CNK[n][k];  // saved C(N,K)
    if (k * 2 > n) k = n - k;
    long long int result = n;
    for (int i = 2; i <= k; ++i) {
        result *= (n - i + 1);
        result /= i;
    }
    return result;
}

long long int HEOM_Solver::findpos(int* arr, const int& N) {  // order N
    long long int X = 0;
    int Hk          = 0;
    for (int k = 1; k <= N; ++k) {
        Hk += arr[N - k];
        X += nchoosek(k - 1 + Hk, k);
    }
    return X;
}

int HEOM_Solver::findarr(int* arr, const long long int& iX, const int& N) {  // order(H*N)
    if (iX == 0) {
        ARRAY_CLEAR(arr, N);
        return 0;
    }
    int k = N, h = 0;
    while (nchoosek(N + h, h) <= iX) h++;  // compare C(N+h,h) - 1 with iX
    long long int X = iX - nchoosek(N + h - 1, h - 1) + 1;
    for (int i = 0; i < N; ++i) {
        int find = h;
        for (int hk = 0, base = k + h - 2; hk < h; ++hk, --base) {
            if (X > nchoosek(base, k - 1)) {
                find = hk;
                break;
            }
        }
        arr[i] = find;  // RDS, cannot be further improved ???
        h -= find;
        k -= 1;
        X -= nchoosek(k + h - 1, k);
    }
    return 0;
}
