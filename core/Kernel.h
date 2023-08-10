/**
 * @file Kernel.h
 * @author xshinhe
 * @version 1.1
 * @date 2023-03
 * @brief Kernel template
 * @details
 *  Kernal is one of the most important classes in OpenDF, and is responsible
 *  for the implementment of algorithms and functionalities.
 *
 *  1) Dependencies: Param & DataSet. The parameters are passed by Param class.
 *      It is also free of storage of data, which is mapped to DataSet class.
 *  2) It's a (hierarchical) tree structure, each kernal also consists of a
 *      list of other kernels.
 *  3) The detailed algorithms can be realized in its child. In priciple, we
 *      don't recomment multilevel inheritance, except Kernel_Model. (refer to
 *      `final` keyword).
 *  4) There should be no `new` or `delete` in Kernel and its derivatives!
 *      Be clear that there are three types attributes for members in Kernel:
 *
 *      a) external: means only need kernel's pointer points to correct object.
 *      b) shared: means kernel's pointer points to correct object and takes
 *              ownership.
 *      c) internal: means a non-pointer data type which is not exposed to
 *              public domain.
 *
 */

#ifndef Kernel_H
#define Kernel_H

#include <chrono>
#include <ctime>
#include <memory>

#include "DataSet.h"
#include "Param.h"

namespace PROJECT_NS {

/**
 * @brief Kernel class is the container and implementation of algorithms
 */
class Kernel {
   public:
    static int TOTAL;
    static int TOTAL_ROOT;

    inline virtual const std::string name() { return utils::concat("Kernel__", customized_name); }

    /**
     * @brief constructors
     */
    Kernel(const std::string& iname = "");

    /**
     * @brief deconstructor.
     */
    virtual ~Kernel();

    /**
     * @brief build tree structure of the kernel
     */
    Kernel& push(std::shared_ptr<Kernel> ker, const bool& take_ownership = true);

    /**
     * @brief read_param() fetch parameters
     */
    inline void read_param(Param* PM, int count = -1) {
        if (count == -1) count = count_read;
        if (count != count_read) return;
        // std::cout << "read_param:" << name() << "\n";
        _Param = PM;

        is_timing = PM->get<bool>("is_timing", LOC(), false);

        read_param_impl(PM);
        for (auto& pkernel : _kernel_vector) pkernel->read_param(PM, count);
        count_read++;
    };

    /**
     * @brief init_data() reference storage
     */
    inline void init_data(DataSet* DS, int count = -1) {
        if (count == -1) count = count_init;
        if (count != count_init) return;
        // std::cout << "init_data:" << name() << "\n";
        _DataSet = DS;

        init_data_impl(DS);
        for (auto& pkernel : _kernel_vector) pkernel->init_data(DS, count);
        count_init++;
    };

    /**
     * @brief init_calc() prepare initial condition
     */
    inline void init_calc(int stat = -1, int count = -1) {
        if (count == -1) count = count_calc;
        if (count != count_calc) return;
        // std::cout << "init_calc:" << name() << "\n";

        init_calc_impl(stat);
        for (auto& pkernel : _kernel_vector) pkernel->init_calc(stat, count);
        count_calc++;
    };

    /**
     * @brief exec_kernel() realize algorithms
     */
    inline int exec_kernel(int stat = -1, int count = -1) {
        assert(count_init > 0);
        std::chrono::time_point<std::chrono::steady_clock> begin, end;
        if (is_timing) begin = std::chrono::steady_clock::now();
        {
            exec_kernel_impl(stat);
            for (auto& pkernel : _kernel_vector) pkernel->exec_kernel(stat);
        }
        if (is_timing) end = std::chrono::steady_clock::now();
        if (is_timing) exec_time += static_cast<std::chrono::duration<double>>(end - begin).count();
        count_exec++;
        return stat;
    };

    inline int id() { return kernel_id; }

    /**
     * @brief  print information of the kernel
     */
    std::string scheme(double total_time = -1.0f, int current_layer = 0, int total_depth = 0, int total_align_size = 0);

   protected:
    bool is_root     = true;
    bool is_timing   = false;
    int count_read   = 0;
    int count_init   = 0;
    int count_calc   = 0;
    int count_exec   = 0;
    int kernel_id    = 0;
    double exec_time = 0.0f;
    std::string customized_name;
    int depth          = 0;
    int max_align_size = 0;

    Param* _Param;  // store the parameter dict
    DataSet* _DataSet;

    std::vector<std::shared_ptr<Kernel>> _kernel_vector;

    virtual void read_param_impl(Param* P);

    virtual void init_data_impl(DataSet* DS);

    virtual void init_calc_impl(int stat = -1);

    virtual int exec_kernel_impl(int stat = -1);

    void destory();
};

};  // namespace PROJECT_NS

#endif  // Kernel_H
