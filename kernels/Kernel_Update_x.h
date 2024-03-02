#ifndef Kernel_Update_x_H
#define Kernel_Update_x_H

#include "../core/Kernel.h"

namespace kids {

class Kernel_Update_x final : public Kernel {
   public:
    Kernel_Update_x(double scale) : Kernel(), scale{scale} {};

    inline virtual const std::string name() { return "Kernel_Update_x"; }

   private:
    double *x, *p, *m, *minv;

    double scale, *dt_ptr;

    virtual void init_data_impl(DataSet* DS);

    virtual int exec_kernel_impl(int stat = -1);
};

};  // namespace kids


#endif  // Kernel_Update_x_H
