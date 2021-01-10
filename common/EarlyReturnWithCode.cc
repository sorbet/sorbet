#include "EarlyReturnWithCode.h"

using namespace std;

namespace sorbet {

EarlyReturnWithCode::EarlyReturnWithCode(int returnCode)
    : SorbetException("early return with code " + to_string(returnCode)), returnCode(returnCode){};

}
