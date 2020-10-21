#include "core/GlobalState.h"
#include <string_view>
#include <vector>

namespace sorbet::rbi {
std::vector<std::pair<std::string_view, std::string_view>> all();
void populateRBIsInto(std::unique_ptr<core::GlobalState> &gs);
} // namespace sorbet::rbi
