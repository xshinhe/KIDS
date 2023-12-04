#include "Model_NAD1D.h"

#include "../kernels/Kernel_Declare.h"
#include "../kernels/Kernel_Random.h"

namespace PROJECT_NS {

double mspes_parm[100];

int mspes_SAC(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    const double V0 = 0.01e0, V1 = 0.005e0, E0 = 0.0e0, a = 1.6e0, b = 1.0e0;

    V[0] = (R[0] > 0) ? V0 * (1.0 - exp(-a * R[0])) : -V0 * (1.0 - exp(a * R[0]));
    V[3] = -V[0];
    V[1] = V1 * exp(-b * R[0] * R[0]);
    V[2] = V[1];
    if (flag < 1) return 0;

    dV[0] = (R[0] > 0) ? V0 * a * exp(-a * R[0]) : V0 * a * exp(a * R[0]);
    dV[3] = -dV[0];
    dV[1] = -2 * b * R[0] * V[1];
    dV[2] = dV[1];
    if (flag < 2) return 0;

    // ddV
    // for (int i = 0; i < 4; ++i) ddV[i] = 0;
    return 0;
}

int mspes_SAC2(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    const double V0 = 0.01e0, V1 = 0.005e0, E0 = 0.0e0, a = 1.6e0, b = 1.0e0;

    V[0] = V0 * tanh(a * R[0]);
    V[3] = -V[0];
    V[1] = V1 * exp(-b * R[0] * R[0]);
    V[2] = V[1];
    if (flag < 1) return 0;

    double tmp = cosh(a * R[0]);
    dV[0]      = V0 * a / (tmp * tmp);
    dV[3]      = -dV[0];
    dV[1]      = -2 * b * R[0] * V[1];
    dV[2]      = dV[1];
    if (flag < 2) return 0;

    // ddV
    // for (int i = 0; i < 4; ++i) ddV[i] = 0;
    return 0;
}

int mspes_SAC3(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    const double V1 = 0.04e0, V2 = 0.01e0, Vc = 0.005e0, a = 1.0e0, b = 1.0e0, Rc = 0.7;

    V[0] = V1 * (1.0e0 + tanh(a * R[0]));
    V[3] = V2 * (1.0e0 - tanh(a * R[0]));
    V[1] = Vc * exp(-b * (R[0] + Rc) * (R[0] + Rc));
    V[2] = V[1];
    if (flag < 1) return 0;

    double tmp = cosh(a * R[0]);
    dV[0]      = +V1 * a / (tmp * tmp);
    dV[3]      = -V2 * a / (tmp * tmp);
    dV[1]      = -2 * b * (R[0] + Rc) * V[1];
    dV[2]      = dV[1];
    if (flag < 2) return 0;

    // ddV
    // for (int i = 0; i < 4; ++i) ddV[i] = 0;
    return 0;
}


int mspes_DAC(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    const double V0 = 0.10e0, V1 = 0.015e0, E0 = 0.05e0, a = 0.28e0, b = 0.06e0;

    V[0] = 0.0e0;
    V[3] = -V0 * exp(-a * R[0] * R[0]) + E0;
    V[1] = V1 * exp(-b * R[0] * R[0]);
    V[2] = V[1];
    if (flag < 1) return 0;

    dV[0] = 0.0e0;
    dV[3] = 2 * a * R[0] * V0 * exp(-a * R[0] * R[0]);
    dV[1] = -2 * b * R[0] * V[1];
    dV[2] = dV[1];
    if (flag < 2) return 0;

    // ddV
    // for (int i = 0; i < 4; ++i) ddV[i] = 0;
    return 0;
}

int mspes_ECR(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    const double V0 = 6.0e-4, V1 = 0.1e0, E0 = 0.0e0, a = 0.0e0, b = 0.9e0;

    V[0] = -V0, V[3] = V0;
    V[1] = (R[0] < 0) ? V1 * exp(b * R[0]) : V1 * (2.0 - exp(-b * R[0]));
    V[2] = V[1];
    if (flag < 1) return 0;

    dV[0] = 0.0e0, dV[3] = 0.0e0;
    dV[1] = (R[0] < 0) ? V1 * b * exp(b * R[0]) : V1 * b * exp(-b * R[0]);
    dV[2] = dV[1];
    if (flag < 2) return 0;

    // ddV
    // for (int i = 0; i < 4; ++i) ddV[i] = 0;
    return 0;
}

int mspes_DBG(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    const double V0 = 6.0e-4, V1 = 0.1e0, E0 = 0.0e0, a = 0.0e0, b = 0.9e0, Z = 10.0e0;

    V[0] = -V0, V[3] = V0;
    if (R[0] < -Z) {
        V[1] = V1 * exp(b * (R[0] - Z)) + V1 * (2.0 - exp(b * (R[0] - Z)));
    } else if (R[0] < Z) {
        V[1] = V1 * exp(b * (R[0] - Z)) + V1 * exp(-b * (R[0] + Z));
    } else {
        V[1] = V1 * exp(-b * (R[0] + Z)) + V1 * (2.0 - exp(-b * (R[0] - Z)));
    }
    V[2] = V[1];
    if (flag < 1) return 0;

    dV[0] = 0.0e0, dV[3] = 0.0e0;
    if (R[0] < -Z) {
        dV[1] = V1 * exp(b * (R[0] - Z)) * b + V1 * (-exp(b * (R[0] - Z))) * b;
    } else if (R[0] < Z) {
        dV[1] = V1 * exp(b * (R[0] - Z)) * b + V1 * (exp(-b * (R[0] + Z))) * (-b);
    } else {
        dV[1] = V1 * exp(-b * (R[0] + Z)) * (-b) + V1 * (-exp(-b * (R[0] - Z))) * (-b);
    }
    dV[2] = dV[1];
    if (flag < 2) return 0;

    // ddV
    // for (int i = 0; i < 4; ++i) ddV[i] = 0;
    return 0;
}

int mspes_DAG(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    const double V0 = 6.0e-4, V1 = 0.1e0, E0 = 0.0e0, a = 0.0e0, b = 0.9e0, Z = 10.0e0;

    V[0] = -V0, V[3] = V0;
    if (R[0] < -Z) {
        V[1] = -V1 * exp(b * (R[0] - Z)) + V1 * exp(b * (R[0] + Z));
    } else if (R[0] < Z) {
        V[1] = -V1 * exp(b * (R[0] - Z)) - V1 * exp(-b * (R[0] + Z)) + 2 * V1;
    } else {
        V[1] = V1 * exp(-b * (R[0] - Z)) - V1 * exp(-b * (R[0] + Z));
    }
    V[2] = V[1];
    if (flag < 1) return 0;

    dV[0] = 0.0e0, dV[3] = 0.0e0;
    if (R[0] < -Z) {
        dV[1] = -V1 * exp(b * (R[0] - Z)) * b + V1 * exp(b * (R[0] + Z)) * b;
    } else if (R[0] < Z) {
        dV[1] = -V1 * exp(b * (R[0] - Z)) * b - V1 * exp(-b * (R[0] + Z)) * (-b);
    } else {
        dV[1] = V1 * exp(-b * (R[0] - Z)) * (-b) - V1 * exp(-b * (R[0] + Z)) * (-b);
    }
    dV[2] = dV[1];
    if (flag < 2) return 0;

    // ddV
    // for (int i = 0; i < 4; ++i) ddV[i] = 0;
    return 0;
}

int mspes_DRN(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    const double E0 = 0.01e0, V1 = 0.03e0, b = 3.2e0, Z = 2.0e0;

    V[0] = 0.0e0;
    V[3] = E0;
    V[1] = V1 * (exp(-b * (R[0] - Z) * (R[0] - Z)) + exp(-b * (R[0] + Z) * (R[0] + Z)));
    V[2] = V[1];
    if (flag < 1) return 0;

    dV[0] = 0.0e0;
    dV[3] = 0.0e0;
    dV[1] = V1 * (exp(-b * (R[0] - Z) * (R[0] - Z)) * (-2 * b * (R[0] - Z)) +
                  exp(-b * (R[0] + Z) * (R[0] + Z)) * (-2 * b * (R[0] + Z)));
    dV[2] = dV[1];
    if (flag < 2) return 0;

    // ddV
    // for (int i = 0; i < 4; ++i) ddV[i] = 0;
    return 0;
}


int mspes_MORSE3A(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    // V
    const double De[3]    = {0.003e0, 0.004e0, 0.003e0};
    const double Be[3]    = {0.65e0, 0.60e0, 0.65e0};
    const double Re[3]    = {5.0e0, 4.0e0, 6.0e0};
    const double C[3]     = {0.0e0, 0.01e0, 0.006e0};
    const double Aijx[3]  = {0.002e0, 0.00e0, 0.002e0};
    const double Aeijx[3] = {16.0e0, 0.00e0, 16.0e0};
    const double Rijx[3]  = {4.80e0, 0.00e0, 3.40e0};

    V[0] = De[0] * (1.0e0 - exp(-Be[0] * (R[0] - Re[0]))) * (1.0e0 - exp(-Be[0] * (R[0] - Re[0]))) + C[0];
    V[4] = De[1] * (1.0e0 - exp(-Be[1] * (R[0] - Re[1]))) * (1.0e0 - exp(-Be[1] * (R[0] - Re[1]))) + C[1];
    V[8] = De[2] * (1.0e0 - exp(-Be[2] * (R[0] - Re[2]))) * (1.0e0 - exp(-Be[2] * (R[0] - Re[2]))) + C[2];

    V[1] = Aijx[2] * exp(-Aeijx[2] * (R[0] - Rijx[2]) * (R[0] - Rijx[2]));  // V(0,1)
    V[2] = Aijx[1] * exp(-Aeijx[1] * (R[0] - Rijx[1]) * (R[0] - Rijx[1]));  // V(0,2)
    V[5] = Aijx[0] * exp(-Aeijx[0] * (R[0] - Rijx[0]) * (R[0] - Rijx[0]));  // V(1,2)
    V[3] = V[1];                                                            // V(1,0)
    V[6] = V[2];                                                            // V(2,0)
    V[7] = V[5];                                                            // V(2,1)

    if (flag < 1) return 0;

    dV[0] = De[0] * (1.0e0 - exp(-Be[0] * (R[0] - Re[0]))) * exp(-Be[0] * (R[0] - Re[0])) * 2 * Be[0];
    dV[4] = De[1] * (1.0e0 - exp(-Be[1] * (R[0] - Re[1]))) * exp(-Be[1] * (R[0] - Re[1])) * 2 * Be[1];
    dV[8] = De[2] * (1.0e0 - exp(-Be[2] * (R[0] - Re[2]))) * exp(-Be[2] * (R[0] - Re[2])) * 2 * Be[2];

    dV[1] = V[1] * (-2 * Aeijx[2] * (R[0] - Rijx[2]));  // dV(0,1)
    dV[2] = V[2] * (-2 * Aeijx[1] * (R[0] - Rijx[1]));  // dV(0,2)
    dV[5] = V[5] * (-2 * Aeijx[0] * (R[0] - Rijx[0]));  // dV(1,2)
    dV[3] = dV[1];                                      // dV(1,0)
    dV[6] = dV[2];                                      // dV(2,0)
    dV[7] = dV[5];                                      // dV(2,1)

    if (flag < 2) return 0;
    // for (int i = 0; i < 9; ++i) ddV[i] = 0;
    return 0;
}

int mspes_MORSE3B(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    // V
    const double De[3]    = {0.020e0, 0.010e0, 0.003e0};
    const double Be[3]    = {0.65e0, 0.40e0, 0.65e0};
    const double Re[3]    = {4.5e0, 4.0e0, 4.4e0};
    const double C[3]     = {0.0e0, 0.01e0, 0.02e0};
    const double Aijx[3]  = {0.00e0, 0.005e0, 0.005e0};
    const double Aeijx[3] = {0.00e0, 32.0e0, 32.0e0};
    const double Rijx[3]  = {0.00e0, 3.34e0, 3.66e0};

    V[0] = De[0] * (1.0e0 - exp(-Be[0] * (R[0] - Re[0]))) * (1.0e0 - exp(-Be[0] * (R[0] - Re[0]))) + C[0];
    V[4] = De[1] * (1.0e0 - exp(-Be[1] * (R[0] - Re[1]))) * (1.0e0 - exp(-Be[1] * (R[0] - Re[1]))) + C[1];
    V[8] = De[2] * (1.0e0 - exp(-Be[2] * (R[0] - Re[2]))) * (1.0e0 - exp(-Be[2] * (R[0] - Re[2]))) + C[2];

    V[1] = Aijx[2] * exp(-Aeijx[2] * (R[0] - Rijx[2]) * (R[0] - Rijx[2]));  // V(0,1)
    V[2] = Aijx[1] * exp(-Aeijx[1] * (R[0] - Rijx[1]) * (R[0] - Rijx[1]));  // V(0,2)
    V[5] = Aijx[0] * exp(-Aeijx[0] * (R[0] - Rijx[0]) * (R[0] - Rijx[0]));  // V(1,2)
    V[3] = V[1];                                                            // V(1,0)
    V[6] = V[2];                                                            // V(2,0)
    V[7] = V[5];                                                            // V(2,1)

    if (flag < 1) return 0;

    dV[0] = De[0] * (1.0e0 - exp(-Be[0] * (R[0] - Re[0]))) * exp(-Be[0] * (R[0] - Re[0])) * 2 * Be[0];
    dV[4] = De[1] * (1.0e0 - exp(-Be[1] * (R[0] - Re[1]))) * exp(-Be[1] * (R[0] - Re[1])) * 2 * Be[1];
    dV[8] = De[2] * (1.0e0 - exp(-Be[2] * (R[0] - Re[2]))) * exp(-Be[2] * (R[0] - Re[2])) * 2 * Be[2];

    dV[1] = V[1] * (-2 * Aeijx[2] * (R[0] - Rijx[2]));  // dV(0,1)
    dV[2] = V[2] * (-2 * Aeijx[1] * (R[0] - Rijx[1]));  // dV(0,2)
    dV[5] = V[5] * (-2 * Aeijx[0] * (R[0] - Rijx[0]));  // dV(1,2)
    dV[3] = dV[1];                                      // dV(1,0)
    dV[6] = dV[2];                                      // dV(2,0)
    dV[7] = dV[5];                                      // dV(2,1)

    if (flag < 2) return 0;
    // for (int i = 0; i < 9; ++i) ddV[i] = 0;
    return 0;
}

int mspes_MORSE3C(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    // V
    const double De[3]    = {0.020e0, 0.020e0, 0.003e0};
    const double Be[3]    = {0.40e0, 0.65e0, 0.65e0};
    const double Re[3]    = {4.0e0, 4.5e0, 6.0e0};
    const double C[3]     = {0.02e0, 0.00e0, 0.02e0};
    const double Aijx[3]  = {0.00e0, 0.005e0, 0.005e0};
    const double Aeijx[3] = {0.00e0, 32.0e0, 32.0e0};
    const double Rijx[3]  = {0.00e0, 4.97e0, 3.40e0};

    V[0] = De[0] * (1.0e0 - exp(-Be[0] * (R[0] - Re[0]))) * (1.0e0 - exp(-Be[0] * (R[0] - Re[0]))) + C[0];
    V[4] = De[1] * (1.0e0 - exp(-Be[1] * (R[0] - Re[1]))) * (1.0e0 - exp(-Be[1] * (R[0] - Re[1]))) + C[1];
    V[8] = De[2] * (1.0e0 - exp(-Be[2] * (R[0] - Re[2]))) * (1.0e0 - exp(-Be[2] * (R[0] - Re[2]))) + C[2];

    V[1] = Aijx[2] * exp(-Aeijx[2] * (R[0] - Rijx[2]) * (R[0] - Rijx[2]));  // V(0,1)
    V[2] = Aijx[1] * exp(-Aeijx[1] * (R[0] - Rijx[1]) * (R[0] - Rijx[1]));  // V(0,2)
    V[5] = Aijx[0] * exp(-Aeijx[0] * (R[0] - Rijx[0]) * (R[0] - Rijx[0]));  // V(1,2)
    V[3] = V[1];                                                            // V(1,0)
    V[6] = V[2];                                                            // V(2,0)
    V[7] = V[5];                                                            // V(2,1)

    // if (R[0] < 0) P[0] = abs(P[0]);

    if (flag < 1) return 0;

    dV[0] = De[0] * (1.0e0 - exp(-Be[0] * (R[0] - Re[0]))) * exp(-Be[0] * (R[0] - Re[0])) * 2 * Be[0];
    dV[4] = De[1] * (1.0e0 - exp(-Be[1] * (R[0] - Re[1]))) * exp(-Be[1] * (R[0] - Re[1])) * 2 * Be[1];
    dV[8] = De[2] * (1.0e0 - exp(-Be[2] * (R[0] - Re[2]))) * exp(-Be[2] * (R[0] - Re[2])) * 2 * Be[2];

    dV[1] = V[1] * (-2 * Aeijx[2] * (R[0] - Rijx[2]));  // dV(0,1)
    dV[2] = V[2] * (-2 * Aeijx[1] * (R[0] - Rijx[1]));  // dV(0,2)
    dV[5] = V[5] * (-2 * Aeijx[0] * (R[0] - Rijx[0]));  // dV(1,2)
    dV[3] = dV[1];                                      // dV(1,0)
    dV[6] = dV[2];                                      // dV(2,0)
    dV[7] = dV[5];                                      // dV(2,1)

    if (flag < 2) return 0;
    // for (int i = 0; i < 9; ++i) ddV[i] = 0;
    return 0;
}


int mspes_MORSE15(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    // V
    const double Dg = 0.2e0, De = 0.05e0, Dc = 0.05e0, alpha = 0.4e0, lambda = 1.0e0, eta = 0.004e0;
    const double Re[15] = {3.0000000000000e0,    6.8335793401926175e0, 6.909940519903887e0, 6.988372712202933e0,
                           7.0690037103879675e0, 7.151973244334301e0,  7.237434522736795e0, 7.325556032471846e0,
                           7.416523648311893e0,  7.5105431195440095e0, 7.607843017311827e0, 7.708678249086644e0,
                           7.813334276495011e0,  7.922132212503321e0,  8.035435027584366};
    for (int i = 0; i < 225; ++i) V[i] = 0.0e0;
    V[0] = Dg * (1 - exp(-alpha * (R[0] - Re[0]))) * (1 - exp(-alpha * (R[0] - Re[0])));
    for (int i = 1; i < 15; ++i) {
        V[i * 16] = exp(-alpha * R[0]) + eta * (i + 1) + De;
        V[i]      = Dc * exp(-lambda * (R[0] - Re[i]) * (R[0] - Re[i]));
        V[i * 15] = V[i];
    }
    if (flag < 1) return 0;

    for (int i = 0; i < 225; ++i) dV[i] = 0.0e0;
    dV[0] = Dg * (1 - exp(-alpha * (R[0] - Re[0]))) * exp(-alpha * (R[0] - Re[0])) * 2 * alpha;
    for (int i = 1; i < 15; ++i) {
        dV[i * 16] = -alpha * exp(-alpha * R[0]);
        dV[i]      = V[i] * (-2 * lambda * (R[0] - Re[i]));
        dV[i * 15] = dV[i];
    }

    if (flag < 2) return 0;
    // for (int i = 0; i < 225; ++i) ddV[i] = 0;
    return 0;
}

int mspes_MORSE15C(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    // V
    const double Dg = 0.2e0, De = 0.05e0, Dc = 0.05e0, alpha = 0.4e0, lambda = 1.0e0, eta = 0.004e0;
    const double Re[15] = {3.0000000000000e0,    6.8335793401926175e0, 6.909940519903887e0, 6.988372712202933e0,
                           7.0690037103879675e0, 7.151973244334301e0,  7.237434522736795e0, 7.325556032471846e0,
                           7.416523648311893e0,  7.5105431195440095e0, 7.607843017311827e0, 7.708678249086644e0,
                           7.813334276495011e0,  7.922132212503321e0,  8.035435027584366};
    for (int i = 0; i < 225; ++i) V[i] = 0.0e0;
    V[0] = Dg * (1 - exp(-alpha * (R[0] - Re[0]))) * (1 - exp(-alpha * (R[0] - Re[0])));
    for (int i = 1; i < 15; ++i) {
        V[i * 16] = exp(-alpha * R[0]) + eta * (i + 1) + De;
        V[i]      = 0.25e0 * Dc;
        V[i * 15] = V[i];
    }
    if (flag < 1) return 0;

    for (int i = 0; i < 225; ++i) dV[i] = 0.0e0;
    dV[0] = Dg * (1 - exp(-alpha * (R[0] - Re[0]))) * exp(-alpha * (R[0] - Re[0])) * 2 * alpha;
    for (int i = 1; i < 15; ++i) {
        dV[i * 16] = -alpha * exp(-alpha * R[0]);
        dV[i]      = 0.0e0;
        dV[i * 15] = dV[i];
    }

    if (flag < 2) return 0;
    // for (int i = 0; i < 225; ++i) ddV[i] = 0;
    return 0;
}

int mspes_MORSE15E(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    // V
    const double Dg = 0.2e0, De = 0.05e0, Dc = 0.05e0, alpha = 0.4e0, lambda = 1.0e0, eta = 0.004e0;
    const double Re[15] = {3.0000000000000e0,    6.8335793401926175e0, 6.909940519903887e0, 6.988372712202933e0,
                           7.0690037103879675e0, 7.151973244334301e0,  7.237434522736795e0, 7.325556032471846e0,
                           7.416523648311893e0,  7.5105431195440095e0, 7.607843017311827e0, 7.708678249086644e0,
                           7.813334276495011e0,  7.922132212503321e0,  8.035435027584366};
    for (int i = 0; i < 225; ++i) V[i] = 0.0e0;
    V[0] = Dg * (1 - exp(-alpha * (R[0] - Re[0]))) * (1 - exp(-alpha * (R[0] - Re[0])));
    for (int i = 1; i < 15; ++i) {
        V[i * 16] = exp(-alpha * R[0]) + eta * (i + 1) + De;
        V[i]      = Dc * exp(-0.2 * R[0]);
        V[i * 15] = V[i];
    }
    if (flag < 1) return 0;

    for (int i = 0; i < 225; ++i) dV[i] = 0.0e0;
    dV[0] = Dg * (1 - exp(-alpha * (R[0] - Re[0]))) * exp(-alpha * (R[0] - Re[0])) * 2 * alpha;
    for (int i = 1; i < 15; ++i) {
        dV[i * 16] = -alpha * exp(-alpha * R[0]);
        dV[i]      = -0.2 * V[i];
        dV[i * 15] = dV[i];
    }

    if (flag < 2) return 0;
    // for (int i = 0; i < 225; ++i) ddV[i] = 0;
    return 0;
}

int mspes_CL1D(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    // @deps. on mass and mod_W
    double meanV = 0.5e0 * mspes_parm[3] * mspes_parm[3] * R[0] * R[0];  // mass = 1
    V[0]         = meanV + mspes_parm[0] + mspes_parm[2] * R[0];
    V[3]         = meanV - mspes_parm[0] - mspes_parm[2] * R[0];
    V[1]         = mspes_parm[1];
    V[2]         = mspes_parm[1];
    if (flag < 1) return 0;

    dV[0] = mspes_parm[3] * mspes_parm[3] * R[0] + mspes_parm[2];
    dV[3] = -dV[0];
    dV[1] = 0;
    dV[2] = 0;
    if (flag < 2) return 0;

    // ddV
    for (int i = 0; i < 4; ++i) ddV[i] = 0;
    return 0;
}

int mspes_JC1D(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    // @deps. on mass and mod_W
    double meanV = 0.5e0 * mspes_parm[3] * mspes_parm[3] * R[0] * R[0];  // mass = 1
    V[0]         = meanV + mspes_parm[0];
    V[3]         = meanV - mspes_parm[0];
    V[1]         = mspes_parm[1] + mspes_parm[2] * R[0];
    V[2]         = mspes_parm[1] + mspes_parm[2] * R[0];
    if (flag < 1) return 0;

    dV[0] = mspes_parm[3] * mspes_parm[3] * R[0];
    dV[3] = -dV[0];
    dV[1] = mspes_parm[2];
    dV[2] = mspes_parm[2];
    if (flag < 2) return 0;

    // ddV
    // for (int i = 0; i < 4; ++i) ddV[i] = 0;
    return 0;
}

int mspes_NA_I(double* V, double* dV, double* ddV, double* R, int flag, int rdim, int fdim) {
    const double au_2_ev  = 27.21138602e0;
    const double au_2_ang = 0.529177249e0;
    const double Acov     = 3150.0e0 / au_2_ev;
    const double Bcov12   = pow(2.647e0 / au_2_ang, 12) / au_2_ev;
    const double Ccov     = 1000.0e0 / au_2_ev / pow(au_2_ang, 6);
    const double Rcov     = 0.435e0 / au_2_ang;
    const double Aion     = 2760.0e0 / au_2_ev;
    const double Bion8    = pow(2.398e0 / au_2_ang, 8) / au_2_ev;
    const double Cion     = 11.3e0 / au_2_ev / pow(au_2_ang, 6);
    const double Rion     = 0.3489e0 / au_2_ang;
    const double al_Mp    = 0.408e0 / pow(au_2_ang, 3);
    const double al_Xm    = 6.431e0 / pow(au_2_ang, 3);
    const double al_M     = 27.0e0 / pow(au_2_ang, 3);
    const double al_X     = 7.0e0 / pow(au_2_ang, 3);
    const double Eth      = 2.075e0 / au_2_ev;
    const double Aoff     = 17.08e0 / au_2_ev;
    const double Roff     = 1.239e0 / au_2_ang;
    const double mred     = (22.989769282e0 * 126.904473e0) / (22.989769282e0 + 126.904473e0) / phys::au_2_amu;

    double r   = R[0];
    double r2  = r * r;
    double r4  = r2 * r2;
    double r5  = r4 * r;
    double r6  = r2 * r4;
    double r7  = r6 * r;
    double r8  = r4 * r4;
    double r9  = r8 * r;
    double r12 = r6 * r6;
    double r13 = r12 * r;

    double Vcent = mspes_parm[1] / (2 * mred * r2);

    V[0] = Vcent                                     //
           + (Acov + Bcov12 / r12) * exp(-r / Rcov)  //
           - Ccov / r6;                              //
    V[3] = Vcent                                     //
           + (Aion + Bion8 / r8) * exp(-r / Rion)    //
           - 1 / r                                   //
           - 0.5 * (al_Mp + al_Xm) / r4              //
           - Cion / r6                               //
           - 2 * al_Mp * al_Xm / r7                  //
           + Eth;
    V[1] = Aoff * exp(-r / Roff);
    V[2] = V[1];

    if (flag < 1) return 0;

    double dVcent = -2 * mspes_parm[1] / (2 * mred * r2 * r);

    dV[0] = dVcent                                                  //
            + (-12 * Bcov12 / r13) * exp(-r / Rcov)                 //
            + (Acov + Bcov12 / r12) * (-1 / Rcov) * exp(-r / Rcov)  //
            + 6 * Ccov / r7;                                        //
    dV[3] = dVcent                                                  //
            + (-8 * Bion8 / r9) * exp(-r / Rion)                    //
            + (Aion + Bion8 / r8) * (-1 / Rion) * exp(-r / Rion)    //
            + 1 / r2                                                //
            + 2 * (al_Mp + al_Xm) / r5                              //
            + 6 * Cion / r7                                         //
            + 14 * al_Mp * al_Xm / r8;                              //
    dV[1] = (-1 / Roff) * Aoff * exp(-r / Roff);
    dV[2] = dV[1];

    if (flag < 2) return 0;

    // ddV
    // for (int i = 0; i < 4; ++i) ddV[i] = 0;
    return 0;
}


void Model_NAD1D::read_param_impl(Param* PM) {
    nad1d_type = NAD1DPolicy::_from(_Param->get<std::string>("nad1d_flag", LOC(), "SAC"));

    // revise tend and dt
    switch (nad1d_type) {
        case NAD1DPolicy::SAC:
        case NAD1DPolicy::SAC2:
        case NAD1DPolicy::SAC3:
        case NAD1DPolicy::DAC:
        case NAD1DPolicy::ECR:
        case NAD1DPolicy::DBG:
        case NAD1DPolicy::DAG:
        case NAD1DPolicy::DRN: {
            double x0_read   = _Param->get<double>("x0", LOC(), -10.0e0);
            double p0_read   = _Param->get<double>("p0", LOC(), 10.0e0);
            double mass_read = _Param->get<double>("m0", LOC(), 2000.0e0);
            double tend_read = _Param->get<double>("tend", LOC(), -1);
            double dt_read   = _Param->get<double>("dt", LOC(), -1);

            double tend_rev = std::abs(5 * x0_read * mass_read / p0_read);
            double dt_rev   = std::abs(x0_read * mass_read / p0_read) / 5000;
            if (tend_read < 0) (*(_Param->pjson()))["tend"] = tend_rev;
            if (dt_read < 0) (*(_Param->pjson()))["dt"] = dt_rev;
            break;
        }
        case NAD1DPolicy::NA_I: {
            // CHECK_EQ(F, 2);
            // initializetion in au
            double mass_Na = 22.989769282e0 / phys::au_2_amu;
            double mass_I  = 126.904473e0 / phys::au_2_amu;
            double mred    = (mass_Na * mass_I) / (mass_Na + mass_I);

            double parm_E    = _Param->get<double>("parm_E", LOC(), 1.0e0);
            double tend_read = _Param->get<double>("tend", LOC(), -1);
            double dt_read   = _Param->get<double>("dt", LOC(), -1);

            double x0_max   = 160.0e0 / phys::au_2_ang;
            double vel      = sqrt(2 * parm_E / mred);
            double tend_rev = 2 * x0_max / vel;
            double dt_rev   = tend_rev / 50000;

            if (tend_read < 0) (*(_Param->pjson()))["tend"] = tend_rev;
            if (dt_read < 0) (*(_Param->pjson()))["dt"] = dt_rev;
            break;
        }
    }
};

void Model_NAD1D::init_data_impl(DataSet* DS) {
    Hsys = DS->reg<num_real>("model.Hsys", Dimension::FF);
    memset(Hsys, 0, Dimension::FF * sizeof(num_real));

    // model field
    mass = DS->reg<double>("model.mass", Dimension::N);
    vpes = DS->reg<double>("model.vpes");                 // not used
    grad = DS->reg<double>("model.grad", Dimension::N);   // not used
    hess = DS->reg<double>("model.hess", Dimension::NN);  // not used
    V    = DS->reg<double>("model.V", Dimension::FF);
    dV   = DS->reg<double>("model.dV", Dimension::NFF);
    // ddV  = DS->reg<double>("model.ddV", Dimension::Dimension::NNFF);

    x0      = DS->reg<double>("model.x0", Dimension::N);
    p0      = DS->reg<double>("model.p0", Dimension::N);
    x_sigma = DS->reg<double>("model.x_sigma", Dimension::N);
    p_sigma = DS->reg<double>("model.p_sigma", Dimension::N);

    // init & integrator
    x      = DS->reg<double>("integrator.x", Dimension::N);
    p      = DS->reg<double>("integrator.p", Dimension::N);
    p_sign = DS->reg<num_complex>("integrator.p_sign", 2);

    double x0_read = _Param->get<double>("x0grid", LOC(), -10.0e0);
    int Nxgird     = _Param->get<int>("Nxgrid", LOC(), 101);
    double dx      = (2 * abs(x0_read)) / (Nxgird - 1);
    double* xgrid  = DS->reg<double>("integrator.xgrid", Nxgird);
    for (int i = 0; i < Nxgird; ++i) xgrid[i] = -abs(x0_read) + i * dx;

    DS->reg<double>("init.x", Dimension::N);
    DS->reg<double>("init.p", Dimension::N);

    mass[0]     = _Param->get<double>("m0", LOC(), 2000.0e0);
    x0[0]       = _Param->get<double>("x0", LOC(), 100.0e0);
    p0[0]       = _Param->get<double>("p0", LOC(), 100.0e0);
    double varx = _Param->get<double>("varx", LOC(), 0.5e0);
    double varp = _Param->get<double>("varp", LOC(), 0.5e0);
    x_sigma[0]  = sqrt(varx);
    p_sigma[0]  = sqrt(varp);

    switch (nad1d_type) {
        case NAD1DPolicy::SAC:
        case NAD1DPolicy::SAC2:
        case NAD1DPolicy::DAC:
        case NAD1DPolicy::ECR:
        case NAD1DPolicy::DBG:
        case NAD1DPolicy::DAG:
        case NAD1DPolicy::DRN: {
            mass[0] = 2000.0e0;
            break;
        }
        case NAD1DPolicy::SAC3: {  // asymmetrical SAC
            mass[0]           = 1980.0e0;
            double gammawidth = 0.25e0;
            x_sigma[0]        = sqrt(0.5e0 / gammawidth);
            p_sigma[0]        = sqrt(0.5e0 * gammawidth);
            break;
        }
        case NAD1DPolicy::MORSE3A:
        case NAD1DPolicy::MORSE3B:
        case NAD1DPolicy::MORSE3C: {
            // CHECK_EQ(F, 3);
            mass[0] = 20000.0e0;
            switch (nad1d_type) {
                case NAD1DPolicy::MORSE3A:
                    x0[0] = 2.9e0;
                    break;
                case NAD1DPolicy::MORSE3B:
                    x0[0] = 3.3e0;
                    break;
                case NAD1DPolicy::MORSE3C:
                    x0[0] = 2.1e0;
                    break;
            }
            p0[0]          = 0.0e0;
            double wground = 5.0e-03;
            x_sigma[0]     = sqrt(0.5e0 / (mass[0] * wground));
            p_sigma[0]     = 0.5e0 / x_sigma[0];
            break;
        }
        case NAD1DPolicy::MORSE15:
        case NAD1DPolicy::MORSE15C:
        case NAD1DPolicy::MORSE15E: {
            // CHECK_EQ(F, 15);
            mass[0]   = 2000.0e0;
            x0[0]     = 13.0e0;
            p0[0]     = -30.0e0;
            double Dg = 0.2e0, alpha = 0.4e0;  // 2*alpha^2 * D = m*w^2
            x_sigma[0] = sqrt(0.5e0 / std::sqrt(mass[0] * 2 * alpha * alpha * Dg));
            p_sigma[0] = 0.5e0 / x_sigma[0];
            break;
        }
        case NAD1DPolicy::CL1D:
        case NAD1DPolicy::JC1D: {
            // CHECK_EQ(F, 2);
            mass[0]       = 1.0e0;
            mspes_parm[0] = _Param->get<double>("nad1d_e", LOC(), 1.0e0);
            mspes_parm[1] = _Param->get<double>("nad1d_d", LOC(), 1.0e0);
            mspes_parm[2] = _Param->get<double>("nad1d_c", LOC(), 1.0e0);
            mspes_parm[3] = _Param->get<double>("nad1d_w", LOC(), 1.0e0);
            break;
        }
        case NAD1DPolicy::NA_I: {
            // CHECK_EQ(F, 2);
            // initializetion in au
            double mass_Na = 22.989769282e0 / phys::au_2_amu;
            double mass_I  = 126.904473e0 / phys::au_2_amu;
            mass[0]        = (mass_Na * mass_I) / (mass_Na + mass_I);

            mspes_parm[0] = _Param->get<double>("parm_E", LOC(), 1.0e0);
            x0[0]         = 160.0e0 / phys::au_2_ang;
            p0[0]         = -sqrt(2 * mass[0] * mspes_parm[0]);
            break;
        }
    }
};

void Model_NAD1D::init_calc_impl(int stat) {
    switch (nad1d_type) {
        case NAD1DPolicy::NA_I: {
            double bmax = 9.0e0 / phys::au_2_ang;
            double randu;
            Kernel_Random::rand_uniform(&randu);
            double b      = sqrt(randu) * bmax;
            mspes_parm[1] = 2 * mass[0] * mspes_parm[0] * b * b;  // l^2 = 2 m E b^2
            for (int j = 0; j < Dimension::N; ++j) {
                x[j] = x0[j];
                p[j] = p0[j];
            }
            break;
        }
        default: {
            Kernel_Random::rand_gaussian(x, Dimension::N);
            Kernel_Random::rand_gaussian(p, Dimension::N);
            for (int j = 0; j < Dimension::N; ++j) {
                x[j] = x0[j] + x[j] * x_sigma[j];
                p[j] = p0[j] + p[j] * p_sigma[j];
            }
            break;
        }
    }
    if (p[0] >= 0) {
        p_sign[0] = phys::math::iu, p_sign[1] = phys::math::iz;
    } else {
        p_sign[0] = phys::math::iz, p_sign[1] = phys::math::iu;
    }
    _DataSet->set("init.x", x, Dimension::N);
    _DataSet->set("init.p", p, Dimension::N);

    exec_kernel(stat);
}

int Model_NAD1D::exec_kernel_impl(int stat) {
    switch (nad1d_type) {
        case NAD1DPolicy::SAC:
            mspes_SAC(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
        case NAD1DPolicy::SAC2:
            mspes_SAC2(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
        case NAD1DPolicy::SAC3:
            mspes_SAC3(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
        case NAD1DPolicy::DAC:
            mspes_DAC(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
        case NAD1DPolicy::ECR:
            mspes_ECR(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
        case NAD1DPolicy::DBG:
            mspes_DBG(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
        case NAD1DPolicy::DAG:
            mspes_DAG(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
        case NAD1DPolicy::DRN:
            mspes_DRN(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
        case NAD1DPolicy::MORSE3A:
            mspes_MORSE3A(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
        case NAD1DPolicy::MORSE3B:
            mspes_MORSE3B(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
        case NAD1DPolicy::MORSE3C:
            mspes_MORSE3C(V, dV, ddV, x, 1, 1, Dimension::F);  // @debug
            break;
        case NAD1DPolicy::MORSE15:
            mspes_MORSE15(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
        case NAD1DPolicy::MORSE15C:
            mspes_MORSE15C(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
        case NAD1DPolicy::MORSE15E:
            mspes_MORSE15E(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
        case NAD1DPolicy::JC1D:
            mspes_JC1D(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
        case NAD1DPolicy::CL1D:
            mspes_CL1D(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
        case NAD1DPolicy::NA_I:
            mspes_NA_I(V, dV, ddV, x, 1, 1, Dimension::F);
            break;
    }
    if (p[0] >= 0) {
        p_sign[0] = phys::math::iu, p_sign[1] = phys::math::iz;
    } else {
        p_sign[0] = phys::math::iz, p_sign[1] = phys::math::iu;
    }
    return stat;
}


};  // namespace PROJECT_NS