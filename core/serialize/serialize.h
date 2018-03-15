#include "core/core.h"

namespace ruby_typer {
namespace core {
namespace serialize {
class GlobalStateSerializer {
public:
    static std::vector<u1> store(GlobalState &gs);
    static void load(GlobalState &gs, const u1 *const p);

    class Pickler {
        std::vector<u1> data;
        u1 zeroCounter = 0;

    public:
        void putU4(u4 u);
        void putU1(const u1 v1);
        void putS8(const int64_t i);
        void putStr(const absl::string_view s);
        std::vector<u1> result();
        Pickler() = default;
    };
    class UnPickler {
        int pos;
        u1 zeroCounter = 0;
        std::vector<u1> data;

    public:
        u4 getU4();
        u1 getU1();
        int64_t getS8();
        std::string getStr();
        explicit UnPickler(const u1 *const data);
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
    static void unpickleGS(UnPickler &p, GlobalState &gs);
};
} // namespace serialize
} // namespace core
}; // namespace ruby_typer
