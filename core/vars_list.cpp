#include "vars_list.h"

namespace PROJECT_NS {

std::vector<VARIABLE_BASE*> VARIABLE_BASE::_LIST;

namespace DATA {

std::size_t _M;      ///< Number of Monte Carlo calculations.
std::size_t _P;      ///< Number of parallel trajectories in each run, used in PIMD or swarms.
std::size_t _N;      ///< Number of nuclear degrees of freedom.
std::size_t _F;      ///< Number of electronic degrees of freedom.
std::size_t _Fadd1;  ///< Number of electronic degrees of freedom + 1.

Shape shape_1(1);
Shape shape_2(2);
Shape shape_X(1);

Shape shape_Nxgrid(1);
Shape shape_Nb(1);

using sh_ref = std::vector<std::size_t*>;

Shape shape_M(sh_ref{&_M});
Shape shape_P(sh_ref{&_P});
Shape shape_N(sh_ref{&_N});
Shape shape_F(sh_ref{&_F});
Shape shape_Fadd1(sh_ref{&_Fadd1});

Shape shape_MP(sh_ref{&_M, &_P});
Shape shape_PP(sh_ref{&_P, &_P});
Shape shape_PN(sh_ref{&_P, &_N});
Shape shape_PNN(sh_ref{&_P, &_N, &_N});
Shape shape_PF(sh_ref{&_P, &_F});
Shape shape_PFF(sh_ref{&_P, &_F, &_F});
Shape shape_PNFF(sh_ref{&_P, &_N, &_F, &_F});
Shape shape_NF(sh_ref{&_N, &_F});
Shape shape_NN(sh_ref{&_N, &_N});
Shape shape_FF(sh_ref{&_F, &_F});
Shape shape_NFF(sh_ref{&_N, &_F, &_F});
Shape shape_NNFF(sh_ref{&_N, &_N, &_F, &_F});

#define NAME_WRAPPER(name, shape, doc_string) name(#name, shape, doc_string)

VARIABLE<kids_real>    NAME_WRAPPER(init::Etot, &shape_1, "total energy");
VARIABLE<kids_real>    NAME_WRAPPER(init::p, &shape_N, "nuclear momentum");
VARIABLE<kids_real>    NAME_WRAPPER(init::x, &shape_N, "nuclear coordinate");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::Acoeff, &shape_P, "configuration coefficients");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::E, &shape_PF, "adiabatic energies");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::Ekin, &shape_P, "kinematic energy");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::Epot, &shape_P, "potential energy");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::Etot, &shape_P, "total energy");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::GWP::L, &shape_P, "ES used for GWP");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::GWP::L1, &shape_P, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::GWP::L2, &shape_P, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::GWP::R, &shape_PP, "EV used for GWP");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::GWP::R1, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::GWP::R2, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::GWP::S1, &shape_PP, "overlap used for GWP");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::GWP::S1h, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::GWP::S2, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::GWP::S2h, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::GWP::Sx, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::GWP::invS1h, &shape_PP, "inversed overlap used for GWP");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::GWP::invS2h, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::Hbasis, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::Hcoeff, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::K0, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::K1, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::K1DA, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::K1DD, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::K1QA, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::K1QD, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::K2, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::K2DA, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::K2DD, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::K2QA, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::K2QD, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::OpA, &shape_FF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::OpB, &shape_FF, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::P_used, &shape_1, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::S, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::Sele, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::Snuc, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::U, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::UXdt, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::UYdt, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::Ubranch, &shape_FF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::Udt, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::Xcoeff, &shape_P, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::alpha, &shape_P, "");
// VARIABLE<kids_real>    NAME_WRAPPER(integrator::alpha, &shape_N, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::c, &shape_PF, "electronic amplitude");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::c1, &shape_PN, "langevin coefficients 1");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::c2p, &shape_PN, "langevin coefficients 2");
VARIABLE<kids_int>     NAME_WRAPPER(integrator::clone_account, &shape_P, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::dtAcoeff, &shape_P, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::dtSele, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::dtlnSnuc, &shape_PP, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::f, &shape_PN, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::fadd, &shape_PN, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::g, &shape_P, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::invS, &shape_PP, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::m, &shape_PN, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::minv, &shape_PN, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::nhc::G, &shape_X, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::nhc::Q, &shape_X, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::nhc::p, &shape_X, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::nhc::x, &shape_X, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::norm, &shape_1, "");
VARIABLE<kids_int>     NAME_WRAPPER(integrator::occ_nuc, &shape_P, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::p, &shape_PN, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::p_sign, &shape_2, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::param::c1, &shape_X, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::param::c2p, &shape_X, "");
VARIABLE<kids_bool>    NAME_WRAPPER(integrator::pf_cross, &shape_PF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::rho_dual, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::rho_ele, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::rho_nuc, &shape_PFF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::rhored, &shape_FF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::rhored2, &shape_FF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::rhored3, &shape_FF, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::sqcIA, &shape_1, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::sqcID, &shape_1, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::sqcw, &shape_F, "");
// VARIABLE<kids_real>    NAME_WRAPPER(integrator::sqcw, &shape_FF, "");
// VARIABLE<kids_real>    NAME_WRAPPER(integrator::sqcw, &shape_F, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::sqcw0, &shape_FF, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::sqcwh, &shape_FF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::tmp::I_PP, &shape_PP, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::tmp::MatC_PP, &shape_PP, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::tmp::MatR_PP, &shape_PP, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::tmp::TtTold, &shape_FF, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::tmp::direction, &shape_N, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::tmp::fporj, &shape_N, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::tmp::ftmp, &shape_N, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::tmp::fun_diag_F, &shape_F, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::tmp::fun_diag_P, &shape_P, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::tmp::invexpidiagdt, &shape_F, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::tmp::ve, &shape_N, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::tmp::vedE, &shape_FF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::tmp::wrho, &shape_FF, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::veF, &shape_FF, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::w, &shape_P, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::w_AA, &shape_P, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::w_AD, &shape_P, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::w_CC, &shape_P, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::w_CP, &shape_P, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::w_DD, &shape_P, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::w_PP, &shape_P, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::ww_A, &shape_P, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::ww_D, &shape_P, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::wz_A, &shape_P, "");
VARIABLE<kids_complex> NAME_WRAPPER(integrator::wz_D, &shape_P, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::x, &shape_PN, "");
VARIABLE<kids_real>    NAME_WRAPPER(integrator::xgrid, &shape_Nxgrid, "");
VARIABLE<kids_bool>    NAME_WRAPPER(iter::at_samplingstep_finally, &shape_1, "");
VARIABLE<kids_bool>    NAME_WRAPPER(iter::at_samplingstep_initially, &shape_1, "");
VARIABLE<kids_real>    NAME_WRAPPER(iter::dt, &shape_1, "");
VARIABLE<kids_int>     NAME_WRAPPER(iter::dtsize, &shape_1, "");
VARIABLE<kids_int>     NAME_WRAPPER(iter::fail_type, &shape_1, "");
VARIABLE<kids_bool>    NAME_WRAPPER(iter::frez, &shape_1, "");
VARIABLE<kids_int>     NAME_WRAPPER(iter::isamp, &shape_1, "");
VARIABLE<kids_int>     NAME_WRAPPER(iter::istep, &shape_1, "");
VARIABLE<kids_bool>    NAME_WRAPPER(iter::last_attempt, &shape_1, "");
VARIABLE<kids_int>     NAME_WRAPPER(iter::msize, &shape_1, "");
VARIABLE<kids_int>     NAME_WRAPPER(iter::nsamp, &shape_1, "");
VARIABLE<kids_int>     NAME_WRAPPER(iter::nstep, &shape_1, "");
VARIABLE<kids_bool>    NAME_WRAPPER(iter::succ, &shape_1, "");
VARIABLE<kids_real>    NAME_WRAPPER(iter::t, &shape_1, "");
VARIABLE<kids_int>     NAME_WRAPPER(iter::tsize, &shape_1, "");
VARIABLE<kids_real>    NAME_WRAPPER(last::Etot, &shape_P, "total energy in last step");
VARIABLE<kids_complex> NAME_WRAPPER(last::c, &shape_PF, "");
VARIABLE<kids_real>    NAME_WRAPPER(last::dV, &shape_PNFF, "");
VARIABLE<kids_real>    NAME_WRAPPER(last::g, &shape_P, "");
VARIABLE<kids_real>    NAME_WRAPPER(last::grad, &shape_PN, "");
VARIABLE<kids_real>    NAME_WRAPPER(last::p, &shape_PN, "");
VARIABLE<kids_real>    NAME_WRAPPER(last::x, &shape_PN, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::Hsys, &shape_FF, "system part of Hamiltonian");
VARIABLE<kids_real>    NAME_WRAPPER(model::Tmod, &shape_NN, "normalmode transformation");
VARIABLE<kids_real>    NAME_WRAPPER(model::V, &shape_PFF, "diabatic potential of energy matrix");
VARIABLE<kids_int>     NAME_WRAPPER(model::atoms, &shape_N, "atom indices");
VARIABLE<kids_real>    NAME_WRAPPER(model::bath::coeffs, &shape_Nb, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::bath::omegas, &shape_Nb, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::bath::p_sigma, &shape_Nb, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::bath::x_sigma, &shape_Nb, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::coupling::CL, &shape_X, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::coupling::Q, &shape_X, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::coupling::QL, &shape_X, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::coupling::Xnj, &shape_NFF, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::dV, &shape_PNFF, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::ddV, &shape_NNFF, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::f_p, &shape_X, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::f_r, &shape_X, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::f_rp, &shape_X, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::grad, &shape_PN, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::hess, &shape_PNN, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::kcoeff, &shape_X, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::lcoeff, &shape_X, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::mass, &shape_N, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::p0, &shape_N, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::p_sigma, &shape_N, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::rep::E, &shape_PF, "");
VARIABLE<kids_complex> NAME_WRAPPER(model::rep::H, &shape_PFF, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::rep::L, &shape_PF, "");
VARIABLE<kids_complex> NAME_WRAPPER(model::rep::R, &shape_PFF, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::rep::T, &shape_PFF, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::rep::Told, &shape_PFF, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::rep::dE, &shape_PNFF, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::rep::ddE, &shape_NNFF, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::rep::nac, &shape_NFF, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::rep::nac_prev, &shape_NFF, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::vpes, &shape_P, "scalar potential of energy");
VARIABLE<kids_real>    NAME_WRAPPER(model::w, &shape_N, "");
VARIABLE<kids_real>    NAME_WRAPPER(model::x0, &shape_N, "initial cencter of coordinate");
VARIABLE<kids_real>    NAME_WRAPPER(model::x_sigma, &shape_N, "initial width of coordinate");
VARIABLE<kids_int>     NAME_WRAPPER(random::seed, &shape_X, "");
};  // namespace DATA

};  // namespace PROJECT_NS