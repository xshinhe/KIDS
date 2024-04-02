/**@file        Kernel_Record.h
 * @brief       this file provides Kernel_Record class for trace data in dataset
 *              during the dynamics.
 *
 * @author      Xin He
 * @date        2024-03
 * @version     1.0
 * @copyright   GNU Lesser General Public License (LGPL)
 *
 *              Copyright (c) 2024 Xin He, Liu-Group
 *
 *  This software is a product of Xin's PhD research conducted by Professor Liu's
 *  Group at the College of Chemistry and Molecular Engineering, Peking University.
 *  All rights are reserved by Peking University.
 *  You should have received a copy of the GNU Lesser General Public License along
 *  with this software. If not, see <https://www.gnu.org/licenses/lgpl-3.0.en.html>
 **********************************************************************************
 * @par revision:
 * <table>
 * <tr><th> Date        <th> Description
 * <tr><td> 2024-04-02  <td> Initial version.
 * </table>
 **********************************************************************************
 */

#ifndef Kernel_Record_H
#define Kernel_Record_H

#include "../core/Kernel.h"
#include "OFSManager.h"

namespace PROJECT_NS {

struct Record_Item {
    std::string v0;
    std::string vt;
    std::string name;
    std::string save;

    int rec_type;
    int ofs_ID;

    int fml_ID0;
    int fml_IDt;
};

struct Result {
   public:
    double                      t0, dt;
    int                         size;
    int                         frame;
    std::vector<std::string>    header;
    std::vector<int>            stat;
    std::vector<double>         data;
    std::ofstream               ofs;
    std::vector<std::ofstream*> ofses;

    Result(){};

    Result(Result& res) {
        t0     = res.t0;
        dt     = res.dt;
        size   = res.size;
        frame  = res.frame;
        header = res.header;
        stat.resize(frame);
        data.resize(frame * size);
        memset(stat.data(), 0, frame * sizeof(int));
        memset(data.data(), 0, frame * size * sizeof(double));
    }

    void stack(kids_dtype itype, std::string str);

    void save(const std::string& fname, int ibegin, int length = -1, bool with_header = false);
    virtual ~Result();
};

class Kernel_Record final : public Kernel {
   public:
    inline virtual const std::string name() { return "Kernel_Record"; }

    Kernel_Record(){};

    virtual ~Kernel_Record();

    inline static Result& get_correlation() { return _correlation; }

   private:
    static Result _correlation;
    std::ofstream ofs_corr;

    std::vector<Record_Item> Record_List;
    OFSManager               ofsm;

    int*  istep_ptr;
    int*  sstep_ptr;
    int*  isamp_ptr;
    int*  nsamp_ptr;
    bool* at_samplingstep_initially_ptr;

    double t0, dt, time_unit;

    std::string directory;

    virtual void token(Result& res, Param::JSON& j);

    virtual void token_array(Result& res, Param::JSON& j);

    virtual void token_object(Result& res, Param::JSON& j);

    virtual void read_param_impl(Param* PM);

    virtual void init_data_impl(DataSet* DS);

    virtual void init_calc_impl(int stat = -1);

    virtual int exec_kernel_impl(int stat = -1);
};

};  // namespace PROJECT_NS


#endif  // Kernel_Record_H
