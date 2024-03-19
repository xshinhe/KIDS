#ifndef Kernel_Dump_DataSet_H
#define Kernel_Dump_DataSet_H

#include "../core/Kernel.h"

namespace PROJECT_NS {


/**
 * @brief Kernel_Dump_DataSet dump current data with stuctured format
 * @It should be added to the last end (as well as middle) of every solver's
 *  builder
 */
class Kernel_Dump_DataSet : public Kernel {
   public:
    inline virtual const std::string name() { return "Kernel_Dump_DataSet"; }

   private:
    std::string directory;
    std::string fn;  ///< filename

    virtual void read_param_impl(Param* PM);

    virtual void init_calc_impl(int stat = -1);

    virtual int exec_kernel_impl(int stat = -1);
};

};  // namespace PROJECT_NS


#endif  // Kernel_Dump_DataSet_H
