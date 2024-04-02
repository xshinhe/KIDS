#include "../kernels/Kernel_Conserve.h"
#include "../kernels/Kernel_Dump_DataSet.h"
#include "../kernels/Kernel_Elec_NAD.h"
#include "../kernels/Kernel_Elec_Switch.h"
#include "../kernels/Kernel_GWP.h"
#include "../kernels/Kernel_Load_DataSet.h"
#include "../kernels/Kernel_NADForce.h"
#include "../kernels/Kernel_Prioritization.h"
#include "../kernels/Kernel_Random.h"
#include "../kernels/Kernel_Read_Dimensions.h"
#include "../kernels/Kernel_Record.h"
#include "../kernels/Kernel_Representation.h"
#include "../kernels/Kernel_Update.h"

namespace PROJECT_NS {

std::shared_ptr<Kernel> NAD_AdaptM_Kernel(std::shared_ptr<Kernel> kmodel, std::string NAD_Kernel_name) {
    bool take_ownership_false = false;

    int split = 4;

    // Root Kernel
    std::shared_ptr<Kernel> ker(new Kernel(NAD_Kernel_name));

    /// Integrator Kernel
    std::shared_ptr<Kernel> kinte(new Kernel("MULTI_Integrator"));

    std::shared_ptr<Kernel_Representation> krepr(new Kernel_Representation());
    std::shared_ptr<Kernel_NADForce>       kforc(new Kernel_NADForce());

    std::shared_ptr<Kernel_Update_p> ku_p(new Kernel_Update_p(0.5e0 / (double) split));
    std::shared_ptr<Kernel_Update_x> ku_x(new Kernel_Update_x(0.5e0));
    std::shared_ptr<Kernel_Update_c> ku_c(new Kernel_Update_c(0.5e0 / (double) split));
    std::shared_ptr<Kernel>          kele;
    if (false) {
        // } else if (NAD_Kernel_name == "CMM") {
        //     kele = std::shared_ptr<Kernel_Elec_CMM>(new Kernel_Elec_CMM());
        // } else if (NAD_Kernel_name == "SQC") {
        //     kele = std::shared_ptr<Kernel_Elec_SQC>(new Kernel_Elec_SQC());
        // } else if (NAD_Kernel_name == "MMD") {
        //     kele = std::shared_ptr<Kernel_Elec_MMD>(new Kernel_Elec_MMD());
        // } else if (NAD_Kernel_name == "SH") {
        //     kele = std::shared_ptr<Kernel_Hopping>(new Kernel_Hopping());
        // } else if (NAD_Kernel_name == "MMSH") {
        //     kele = std::shared_ptr<Kernel_Elec_MMSH>(new Kernel_Elec_MMSH());
    } else if (NAD_Kernel_name == "NAD") {
        kele = std::shared_ptr<Kernel_Elec_NAD>(new Kernel_Elec_NAD(0.5e0 / (double) split));
        // } else if (NAD_Kernel_name == "MCE") {
        //     kele = std::shared_ptr<Kernel_GWP>(new Kernel_GWP(kmodel, krepr, kforc));
    } else {
        throw std::runtime_error("unknown Elec Kernel");
    }

    /// Result & Sampling & TCF
    std::shared_ptr<Kernel_Record> krecd(new Kernel_Record());

    for (int i = 0; i < split; ++i) {
        kinte->push(ku_p);
        kinte->push(krepr);
        kinte->push(ku_c);
        kinte->push(kele);
        kinte->push(kforc);
    }

    kinte->push(ku_x);
    kinte->push(ku_x);
    kinte->push(kmodel);
    kinte->push(krepr);

    for (int i = 0; i < split; ++i) {
        kinte->push(ku_c);
        kinte->push(kele);
        kinte->push(kforc);
        kinte->push(ku_p);
        kinte->push(krepr);
    }
    kinte->push(std::shared_ptr<Kernel_Elec_Switch>(new Kernel_Elec_Switch()));
    kinte->push(kforc);

    kinte->push(std::shared_ptr<Kernel_Conserve>(new Kernel_Conserve()));

    std::shared_ptr<Kernel_Iter_Adapt> kiter(new Kernel_Iter_Adapt());
    kiter->push(krecd);  // stacked in iteration
    kiter->push(kinte);  // stacked in iteration

    // /// CMM kernel
    ker->push(std::shared_ptr<Kernel_Load_DataSet>(new Kernel_Load_DataSet()))
        .push(std::shared_ptr<Kernel_Random>(new Kernel_Random()))
        .push(std::shared_ptr<Kernel_Read_Dimensions>(new Kernel_Read_Dimensions()))
        .push(std::shared_ptr<Kernel_Prioritization>(new Kernel_Prioritization({kmodel, kinte}, 1)))
        .push(std::shared_ptr<Kernel_Prioritization>(new Kernel_Prioritization({kmodel, krepr, kele, krecd}, 2)))
        .push(kiter)
        .push(std::shared_ptr<Kernel_Dump_DataSet>(new Kernel_Dump_DataSet()));
    return ker;
}

};  // namespace PROJECT_NS
