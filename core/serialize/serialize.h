#include "core/core.h"

namespace ruby_typer {
namespace core {
namespace serialize {
class GlobalStateSerializer {
public:
    static std::vector<u4> store(GlobalState &gs);
    static GlobalState load(const u4 *const p, spdlog::logger &logger);

    class Pickler {
    public:
        std::vector<ruby_typer::u4> data;
        void putU4(const u4 u);
        void put4U1(u1 v1, u1 v2, u1 v3, u1 v4);
        void putS8(const int64_t i);
        void putStr(const absl::string_view s);
        Pickler() = default;
    };
    class UnPickler {
    public:
        int pos;
        u4 zeroCounter = 0;
        const u4 *const data;
        u4 getU4();
        void get4U1(u1 &v1, u1 &v2, u1 &v3, u1 &v4);
        int64_t getS8();
        std::string getStr();
        explicit UnPickler(const u4 *const data) : pos(0), data(data){};
    };
    static Pickler pickle(GlobalState &gs);
    static void pickle(Pickler &p, File &what);
    static void pickle(Pickler &p, Name &what);
    static void pickle(Pickler &p, Type *what);
    static void pickle(Pickler &p, Symbol &what);

    static std::shared_ptr<File> unpickleFile(UnPickler &p);
    static Name unpickleName(UnPickler &p, GlobalState &gs);
    static std::shared_ptr<Type> unpickleType(UnPickler &p, GlobalState *gs);
    static Symbol unpickleSymbol(UnPickler &p, GlobalState *gs);
    static GlobalState unpickleGS(UnPickler &p, spdlog::logger &logger);
};
} // namespace serialize
} // namespace core
}; // namespace ruby_typer
