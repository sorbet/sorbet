
#ifndef SORBET_SERIALIZE_H
#define SORBET_SERIALIZE_H
#include "ast/ast.h"
#include "core/core.h"

namespace sorbet {
namespace core {
namespace serialize {
class Serializer {
public:
    static const u4 VERSION = 4;
    static const u1 GLOBAL_STATE_COMPRESSION_DEGREE =
        10; // >20 introduce decompression slowdown, >10 introduces compression slowdown
    static const u1 FILE_COMPRESSION_DEGREE =
        10; // >20 introduce decompression slowdown, >10 introduces compression slowdown

    // Serialize a global state
    static std::vector<u1> store(GlobalState &gs);

    // Stores a GlobalState, but only includes `File`s with Type ==
    // Payload. This can be used in conjunction with `store(GlobalState,
    // ast::Expression)` to store a global state containing a name table along
    // side a large number of individual cached files, which can be loaded
    // independently.
    static std::vector<u1> storePayloadAndNameTable(GlobalState &gs);
    static std::vector<u1> storeExpression(GlobalState &gs, std::unique_ptr<ast::Expression> &e);

    // Loads an ast::Expression saved by storeExpression. Optionally overrides
    // the saved file ID to the caller-specified ID.
    static std::unique_ptr<ast::Expression> loadExpression(GlobalState &gs, const u1 *const p, u4 forceId = 0);
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

private:
    static Pickler pickle(const GlobalState &gs, bool payloadOnly = false);
    static void pickle(Pickler &p, const File &what);
    static void pickle(Pickler &p, const Name &what);
    static void pickle(Pickler &p, Type *what);
    static void pickle(Pickler &p, const Symbol &what);
    static void pickle(Pickler &p, const std::unique_ptr<ast::Expression> &what);

    template <class T> static void pickleTree(Pickler &p, std::unique_ptr<T> &t);

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
}; // namespace sorbet

#endif
