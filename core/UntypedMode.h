#ifndef SORBET_UNTYPED_MODE_H
#define SORBET_UNTYPED_MODE_H

namespace sorbet::core {

enum class UntypedMode {
    AlwaysCompatible = 1,
    AlwaysIncompatible = 2,
};

}
#endif
