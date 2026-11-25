#include <string_view>
#include <vector>

namespace sorbet::core {
class GlobalState;
}
namespace sorbet::rbi {
std::vector<std::pair<std::string_view, std::string_view>> all();
void populateRBIsInto(core::GlobalState &gs);
} // namespace sorbet::rbi
