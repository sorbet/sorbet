
#ifndef SORBET_SERIALIZE_H
#define SORBET_SERIALIZE_H
#include "ast/ast.h"
#include "core/core.h"

namespace ruby_typer {
namespace core {
namespace serialize {
class Serializer {
public:
    static const u4 VERSION = 4;
    static const u1 GLOBAL_STATE_COMPRESSION_DEGREE =
        10; // >20 introduce decompression slowdown, >10 introduces compression slowdown
    static const u1 FILE_COMPRESSION_DEGREE =
        10; // >20 introduce decompression slowdown, >10 introduces compression slowdown
    static std::vector<u1> store(GlobalState &gs);
    static std::vector<u1> store(GlobalState &gs, std::unique_ptr<ast::Expression> &e);
    static std::unique_ptr<ast::Expression> loadExpression(GlobalState &gs, const u1 *const p);
    static void loadGlobalState(GlobalState &gs, const u1 *const data);

    class Pickler {
        std::vector<u1> data;
        u1 zeroCounter = 0;

    public:
        void putU4(u4 u);
        void putU1(const u1 u);
        void putS8(const int64_t i);
        void putStr(const absl::string_view s);
        std::vector<u1> result(int compressionDegree);
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
        absl::string_view getStr();
        explicit UnPickler(const u1 *const compressed);
    };
    static Pickler pickle(const GlobalState &gs);
    static void pickle(Pickler &p, const File &what);
    static void pickle(Pickler &p, const Name &what);
    static void pickle(Pickler &p, Type *what);
    static void pickle(Pickler &p, const Symbol &what);
    static void pickle(Pickler &p, const std::unique_ptr<ast::Expression> &what);

private:
    static std::shared_ptr<File> unpickleFile(UnPickler &p);
    static Name unpickleName(UnPickler &p, GlobalState &gs);
    static std::shared_ptr<Type> unpickleType(UnPickler &p, GlobalState *gs);
    static Symbol unpickleSymbol(UnPickler &p, GlobalState *gs);
    static void unpickleGS(UnPickler &p, GlobalState &result);
    static std::unique_ptr<ast::Expression> unpickleExpr(UnPickler &p, GlobalState &, FileRef file);
    static NameRef unpickleNameRef(UnPickler &p, GlobalState &);
};
} // namespace serialize
} // namespace core
}; // namespace ruby_typer

#endif
