#ifndef Model_Bath_H
#define Model_Bath_H

#include "kids/Model.h"
#include "kids/Policy.h"


namespace PROJECT_NS {


DEFINE_POLICY(BathPolicy,   //
              Debye,        //
              Ohmic,        //
              Closure,      //
              HuangRhys,    //
              GFactor,      //
              ReadFormula,  //
              ReadFile,     //
              Read);        //

DEFINE_POLICY(StrengthPolicy,  //
              Lambda,          //
              Alpha,           //
              Eta,             //
              Erg);            //

class Model_Bath final : public Model {
   public:
    virtual const std::string getName();

    virtual int getType() const;

    bool classical_bath = false;

    int                   Nb;
    BathPolicy::_type     bath_type;
    StrengthPolicy::_type strength_type;

    double omegac, lambda, beta;

    double J_Debye(double w);

    double J_Ohmic(double w);

    static double J(double w, double* w_arr = nullptr, double* c_arr = nullptr, int Nb = 0);

    static int fun_Cw(kids_complex* Cw_arr, double* w, int Nw, double* w_arr, double* c_arr, double beta, int Nb);

   private:
    kids_real* coeffs;
    kids_real* omegas;
    kids_real* x_sigma;
    kids_real* p_sigma;

    virtual void setInputParam_impl(std::shared_ptr<Param> PM);
    virtual void setInputDataSet_impl(std::shared_ptr<DataSet> DS);
};


};  // namespace PROJECT_NS

#endif  // Model_Bath_H