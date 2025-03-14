#ifndef MODEL_HELLO
#define MODEL_HELLO

#include "kids/Model.h"
#include "kids/hash_fnv1a.h"
#include "kids/macro_utils.h"

namespace PROJECT_NS {

class Model_Hello final : public Model {
   public:
    virtual const std::string getName() { return "Model_Hello"; }

    virtual int getType() const { return utils::hash(FUNCTION_NAME); }

   private:
    int             N, F;
    span<kids_real> x, V, dV, ddV;
    span<kids_real> m, w;

    virtual void setInputParam_impl(std::shared_ptr<Param> PM){};

    virtual void setInputDataSet_impl(std::shared_ptr<DataSet> DS){};

    virtual Status& executeKernel_impl(Status& stat) {
        std::cout << "Hello\n";
        return 0;
    }
};

};  // namespace PROJECT_NS

#endif  // MODEL_HELLO
