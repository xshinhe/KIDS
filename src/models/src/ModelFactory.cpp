#include "kids/ModelFactory.h"

#include "kids/Model_ElectronTransfer.h"
#include "kids/Model_Interf_MNDO.h"
#include "kids/Model_LVCM.h"
#include "kids/Model_NAD1D.h"
#include "kids/Model_QMInterface.h"
#include "kids/Model_QMMMInterface.h"
#include "kids/Model_SystemBath.h"
#include "kids/Model_TDSystemBath.h"

namespace PROJECT_NS {

std::shared_ptr<Model> defaultModelFactory(const std::string& name) {
    if (false) {
    } else if (name == "SystemBath") {
        return std::shared_ptr<Model_SystemBath>(new Model_SystemBath());
    } else if (name == "TDSB") {
        return std::shared_ptr<Model_TDSystemBath>(new Model_TDSystemBath());
    } else if (name == "ET") {
        return std::shared_ptr<Model_ElectronTransfer>(new Model_ElectronTransfer());
    } else if (name == "LVCM") {
        return std::shared_ptr<Model_LVCM>(new Model_LVCM());
    } else if (name == "NAD1D") {
        return std::shared_ptr<Model_NAD1D>(new Model_NAD1D());
    } else if (name == "QM") {
        return std::shared_ptr<Model_QMInterface>(new Model_QMInterface());
    } else if (name == "QMMM") {
        return std::shared_ptr<Model_QMMMInterface>(new Model_QMMMInterface());
    } else if (name == "Interf_MNDO") {
        return std::shared_ptr<Model_Interf_MNDO>(new Model_Interf_MNDO());
    } else {
        throw std::runtime_error("unknown Model name");
    }
    return nullptr;
}

};  // namespace PROJECT_NS
