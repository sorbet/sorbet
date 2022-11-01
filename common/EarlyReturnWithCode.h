#ifndef SORBET_COMMON_EARLYRETURNWITHCODE_H
#define SORBET_COMMON_EARLYRETURNWITHCODE_H

#include "exception/Exception.h"

namespace sorbet {

// Terminate execution of sorbet with specific return code
class EarlyReturnWithCode : public SorbetException {
public:
    EarlyReturnWithCode(int returnCode);
    const int returnCode;
};

} // namespace sorbet

#endif
