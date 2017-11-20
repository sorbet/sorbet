#include "core/core.h"

namespace ruby_typer {
namespace core {
namespace serialize {
class GlobalStateSerializer {
public:
    static std::vector<u4> store(GlobalState &gs);
    static GlobalState load(const u4 *const p, spdlog::logger &logger);

    class Picker {
    public:
        std::vector<ruby_typer::u4> data;
        void putU4(const u4 u);
        void putS8(const int64_t i);
        void putStr(const std::string s);
        Picker() = default;
    };
    class UnPicker {
    public:
        int pos;
        const u4 *const data;
        u4 getU4();
        int64_t getS8();
        std::string getStr();
        explicit UnPicker(const u4 *const data) : pos(0), data(data){};
    };
    static Picker pickle(GlobalState &gs);
    static void pickle(Picker &p, File &what);
    static void pickle(Picker &p, Name &what);
    static void pickle(Picker &p, Type *what);
    static void pickle(Picker &p, Symbol &what);

    static File unpickleFile(UnPicker &p);
    static Name unpickleName(UnPicker &p, GlobalState &gs);
    static std::shared_ptr<Type> unpickleType(UnPicker &p);
    static Symbol unpickleSymbol(UnPicker &p);
    static GlobalState unpickleGS(UnPicker &p, spdlog::logger &logger);
};
} // namespace serialize
} // namespace core
}; // namespace ruby_typer
