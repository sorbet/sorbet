#include "core/serialize/serialize.h"
#include "absl/base/casts.h"
#include "absl/types/span.h"
#include "ast/Helpers.h"
#include "core/Error.h"
#include "core/GlobalState.h"
#include "core/Symbols.h"
#include "core/serialize/pickler.h"
#include "lib/lizard_compress.h"
#include "lib/lizard_decompress.h"

template class std::vector<sorbet::u4>;

using namespace std;

namespace sorbet::core::serialize {
const u4 Serializer::VERSION;
const u1 Serializer::GLOBAL_STATE_COMPRESSION_DEGREE;
const u1 Serializer::FILE_COMPRESSION_DEGREE;

// These helper methods are declared in a class inside of an anonymous namespace
// or inline so that `GlobalState` can forward-declare and `friend` the entire
// class.
class SerializerImpl {
public:
    static Pickler pickle(const GlobalState &gs, bool payloadOnly = false);
    static void pickle(Pickler &p, const File &what);
    static void pickle(Pickler &p, const Name &what);
    static void pickle(Pickler &p, Type *what);
    static void pickle(Pickler &p, const Symbol &what);
    static void pickle(Pickler &p, FileRef file, const unique_ptr<ast::Expression> &what);
    static void pickle(Pickler &p, core::Loc loc);

    template <class T> static void pickleTree(Pickler &p, FileRef file, std::unique_ptr<T> &t);

    static shared_ptr<File> unpickleFile(UnPickler &p);
    static Name unpickleName(UnPickler &p, GlobalState &gs);
    static shared_ptr<Type> unpickleType(UnPickler &p, GlobalState *gs);
    static Symbol unpickleSymbol(UnPickler &p, GlobalState *gs);
    static void unpickleGS(UnPickler &p, GlobalState &result);
    static Loc unpickleLoc(UnPickler &p, FileRef file);
    static unique_ptr<ast::Expression> unpickleExpr(UnPickler &p, GlobalState &, FileRef file);
    static NameRef unpickleNameRef(UnPickler &p, GlobalState &);

    SerializerImpl() = delete;

private:
    static void pickleAstHeader(Pickler &p, u1 tag, ast::Expression *tree);
};

void Pickler::putStr(string_view s) {
    putU4(s.size());

    for (char c : s) {
        putU1(absl::bit_cast<u1>(c));
    }
}

constexpr size_t SIZE_BYTES = sizeof(int) / sizeof(u1);

vector<u1> Pickler::result(int compressionDegree) {
    if (zeroCounter != 0) {
        data.emplace_back(zeroCounter);
        zeroCounter = 0;
    }
    const size_t max_dst_size = Lizard_compressBound(data.size());
    vector<u1> compressed_data;
    compressed_data.resize(2048 + max_dst_size); // give extra room for compression
                                                 // Lizard_compressBound returns size of data if compression
                                                 // succeeds. It seems to be written for big inputs
                                                 // and returns too small sizes for small inputs,
                                                 // where compressed size is bigger than original size
    int resultCode = Lizard_compress((const char *)data.data(), (char *)(compressed_data.data() + SIZE_BYTES * 2),
                                     data.size(), (compressed_data.size() - SIZE_BYTES * 2), compressionDegree);
    if (resultCode == 0) {
        // did not compress!
        Exception::raise("incompressible pickler?");
    } else {
        memcpy(compressed_data.data(), &resultCode, SIZE_BYTES); // ~200K of our stdlib
        int uncompressedSize = data.size();
        memcpy(compressed_data.data() + SIZE_BYTES, &uncompressedSize,
               SIZE_BYTES);                                     // 172817 ints(x4), ~675K of our stdlib
        int actualCompressedSize = resultCode + SIZE_BYTES * 2; // SIZE_BYTES * 2 are for sizes
        compressed_data.resize(actualCompressedSize);
    }
    return compressed_data;
}

UnPickler::UnPickler(const u1 *const compressed) : pos(0) {
    int compressedSize;
    memcpy(&compressedSize, compressed, SIZE_BYTES);
    int uncompressedSize;
    memcpy(&uncompressedSize, compressed + SIZE_BYTES, SIZE_BYTES);

    data.resize(uncompressedSize);

    int resultCode = Lizard_decompress_safe((const char *)(compressed + 2 * SIZE_BYTES), (char *)this->data.data(),
                                            compressedSize, uncompressedSize);
    if (resultCode != uncompressedSize) {
        Exception::raise("incomplete decompression");
    }
}

string_view UnPickler::getStr() {
    int sz = getU4();
    string_view result((char *)&data[pos], sz);
    pos += sz;

    return result;
}

void Pickler::putU1(u1 u) {
    if (zeroCounter != 0) {
        data.emplace_back(zeroCounter);
        zeroCounter = 0;
    }
    data.emplace_back(u);
}
void Pickler::putU2(u2 u) {
    putU1(u >> 8);
    putU1(u & 255);
}

u1 UnPickler::getU1() {
    ENFORCE(zeroCounter == 0);
    auto res = data[pos++];
    return res;
}
u2 UnPickler::getU2() {
    auto high = getU1();
    auto low = getU1();
    return (((u2)high) << 8) + low;
}

void Pickler::putU4(u4 u) {
    if (u == 0) {
        if (zeroCounter != 0) {
            if (zeroCounter == UCHAR_MAX) {
                data.emplace_back(UCHAR_MAX);
                zeroCounter = 0;
                putU4(u);
                return;
            }
            zeroCounter++;
            return;
        } else {
            data.emplace_back(0);
            zeroCounter = 1;
        }
    } else {
        if (zeroCounter != 0) {
            data.emplace_back(zeroCounter);
            zeroCounter = 0;
        }
        while (u > 127) {
            data.emplace_back(128 | (u & 127));
            u = u >> 7;
        }
        data.emplace_back(u & 127);
    }
}

u4 UnPickler::getU4() {
    if (zeroCounter != 0) {
        zeroCounter--;
        return 0;
    }
    u1 r = data[pos++];
    if (r == 0) {
        zeroCounter = data[pos++];
        zeroCounter--;
        return r;
    } else {
        u4 res = r & 127;
        u4 vle = r;
        if ((vle & 128) == 0) {
            goto done;
        }

        vle = data[pos++];
        res |= (vle & 127) << 7;
        if ((vle & 128) == 0) {
            goto done;
        }

        vle = data[pos++];
        res |= (vle & 127) << 14;
        if ((vle & 128) == 0) {
            goto done;
        }

        vle = data[pos++];
        res |= (vle & 127) << 21;
        if ((vle & 128) == 0) {
            goto done;
        }

        vle = data[pos++];
        res |= (vle & 127) << 28;
        if ((vle & 128) == 0) {
            goto done;
        }

    done:
        return res;
    }
}

void Pickler::putS8(const int64_t i) {
    auto u = absl::bit_cast<u8>(i);
    while (u > 127) {
        putU1((u & 127) | 128);
        u = u >> 7;
    }
    putU1(u & 127);
}

int64_t UnPickler::getS8() {
    u8 res = 0;
    u8 vle = 128;
    int i = 0;
    while (vle & 128) {
        vle = getU1();
        res |= (vle & 127) << (i * 7);
        i++;
    }
    return absl::bit_cast<int64_t>(res);
}

void SerializerImpl::pickle(Pickler &p, const File &what) {
    p.putU1((u1)what.sourceType);
    p.putStr(what.path());
    p.putStr(what.source());
}

shared_ptr<File> SerializerImpl::unpickleFile(UnPickler &p) {
    auto t = (File::Type)p.getU1();
    auto path = string(p.getStr());
    auto source = string(p.getStr());
    return make_shared<File>(move(path), move(source), t);
}

void SerializerImpl::pickle(Pickler &p, const Name &what) {
    p.putU1(what.kind);
    switch (what.kind) {
        case NameKind::UTF8:
            p.putStr(what.raw.utf8);
            break;
        case NameKind::UNIQUE:
            p.putU1(what.unique.uniqueNameKind);
            p.putU4(what.unique.original._id);
            p.putU4(what.unique.num);
            break;
        case NameKind::CONSTANT:
            p.putU4(what.cnst.original.id());
            break;
    }
}

Name SerializerImpl::unpickleName(UnPickler &p, GlobalState &gs) {
    Name result;
    result.kind = (NameKind)p.getU1();
    switch (result.kind) {
        case NameKind::UTF8:
            result.kind = NameKind::UTF8;
            result.raw.utf8 = gs.enterString(p.getStr());
            break;
        case NameKind::UNIQUE:
            result.unique.uniqueNameKind = (UniqueNameKind)p.getU1();
            result.unique.original = NameRef(gs, p.getU4());
            result.unique.num = p.getU4();
            break;
        case NameKind::CONSTANT:
            result.cnst.original = NameRef(gs, p.getU4());
            break;
    }
    return result;
}

void SerializerImpl::pickle(Pickler &p, Type *what) {
    if (what == nullptr) {
        p.putU4(0);
        return;
    }
    if (auto *c = cast_type<ClassType>(what)) {
        p.putU4(1);
        p.putU4(c->symbol._id);
    } else if (auto *o = cast_type<OrType>(what)) {
        p.putU4(2);
        pickle(p, o->left.get());
        pickle(p, o->right.get());
    } else if (auto *c = cast_type<LiteralType>(what)) {
        p.putU4(3);
        p.putU1((u1)c->literalKind);
        p.putS8(c->value);
    } else if (auto *a = cast_type<AndType>(what)) {
        p.putU4(4);
        pickle(p, a->left.get());
        pickle(p, a->right.get());
    } else if (auto *arr = cast_type<TupleType>(what)) {
        p.putU4(5);
        pickle(p, arr->underlying().get());
        p.putU4(arr->elems.size());
        for (auto &el : arr->elems) {
            pickle(p, el.get());
        }
    } else if (auto *hash = cast_type<ShapeType>(what)) {
        p.putU4(6);
        pickle(p, hash->underlying().get());
        p.putU4(hash->keys.size());
        ENFORCE(hash->keys.size() == hash->values.size());
        for (auto &el : hash->keys) {
            pickle(p, el.get());
        }
        for (auto &el : hash->values) {
            pickle(p, el.get());
        }
    } else if (auto *alias = cast_type<AliasType>(what)) {
        p.putU4(7);
        p.putU4(alias->symbol._id);
    } else if (auto *lp = cast_type<LambdaParam>(what)) {
        p.putU4(8);
        p.putU4(lp->definition._id);
    } else if (auto *at = cast_type<AppliedType>(what)) {
        p.putU4(9);
        p.putU4(at->klass._id);
        p.putU4(at->targs.size());
        for (auto &t : at->targs) {
            pickle(p, t.get());
        }
    } else if (auto *tp = cast_type<TypeVar>(what)) {
        p.putU4(10);
        p.putU4(tp->sym._id);
    } else if (auto *st = cast_type<SelfType>(what)) {
        p.putU4(11);
    } else {
        Exception::notImplemented();
    }
}

shared_ptr<Type> SerializerImpl::unpickleType(UnPickler &p, GlobalState *gs) {
    auto tag = p.getU4(); // though we formally need only u1 here, benchmarks suggest that size difference after
                          // compression is small and u4 is 10% faster
    switch (tag) {
        case 0: {
            shared_ptr<Type> empty;
            return empty;
        }
        case 1:
            return make_shared<ClassType>(SymbolRef(gs, p.getU4()));
        case 2:
            return OrType::make_shared(unpickleType(p, gs), unpickleType(p, gs));
        case 3: {
            auto kind = (core::LiteralType::LiteralTypeKind)p.getU1();
            auto value = p.getS8();
            switch (kind) {
                case LiteralType::LiteralTypeKind::Integer:
                    return make_shared<LiteralType>(value);
                case LiteralType::LiteralTypeKind::Float:
                    return make_shared<LiteralType>(absl::bit_cast<double>(value));
                case LiteralType::LiteralTypeKind::String:
                    return make_shared<LiteralType>(Symbols::String(), core::NameRef(NameRef::WellKnown{}, value));
                case LiteralType::LiteralTypeKind::Symbol:
                    return make_shared<LiteralType>(Symbols::Symbol(), core::NameRef(NameRef::WellKnown{}, value));
                case LiteralType::LiteralTypeKind::True:
                    return make_shared<LiteralType>(true);
                case LiteralType::LiteralTypeKind::False:
                    return make_shared<LiteralType>(false);
            }
            shared_ptr<LiteralType> result = make_shared<LiteralType>(true);

            return result;
        }
        case 4:
            return AndType::make_shared(unpickleType(p, gs), unpickleType(p, gs));
        case 5: {
            auto underlying = unpickleType(p, gs);
            int sz = p.getU4();
            vector<shared_ptr<Type>> elems(sz);
            for (auto &elem : elems) {
                elem = unpickleType(p, gs);
            }
            auto result = make_shared<TupleType>(underlying, move(elems));
            return result;
        }
        case 6: {
            auto underlying = unpickleType(p, gs);
            int sz = p.getU4();
            vector<shared_ptr<LiteralType>> keys(sz);
            vector<shared_ptr<Type>> values(sz);
            for (auto &key : keys) {
                auto tmp = unpickleType(p, gs);
                if (auto *lit = cast_type<LiteralType>(tmp.get())) {
                    key = shared_ptr<LiteralType>(tmp, lit);
                }
            }
            for (auto &value : values) {
                value = unpickleType(p, gs);
            }
            auto result = make_shared<ShapeType>(underlying, keys, values);
            return result;
        }
        case 7:
            return make_shared<AliasType>(SymbolRef(gs, p.getU4()));
        case 8: {
            return make_shared<LambdaParam>(SymbolRef(gs, p.getU4()));
        }
        case 9: {
            SymbolRef klass(gs, p.getU4());
            int sz = p.getU4();
            vector<shared_ptr<Type>> targs(sz);
            for (auto &t : targs) {
                t = unpickleType(p, gs);
            }
            return make_shared<AppliedType>(klass, targs);
        }
        case 10: {
            SymbolRef sym(gs, p.getU4());
            return make_shared<TypeVar>(sym);
        }
        case 11: {
            return make_shared<SelfType>();
        }
        default:
            Exception::raise("Uknown type tag {}", tag);
    }
}

void SerializerImpl::pickle(Pickler &p, const Symbol &what) {
    p.putU4(what.owner._id);
    p.putU4(what.name._id);
    p.putU4(what.superClass._id);
    p.putU4(what.flags);
    p.putU4(what.uniqueCounter);
    p.putU4(what.argumentsOrMixins.size());
    for (SymbolRef s : what.argumentsOrMixins) {
        p.putU4(s._id);
    }
    p.putU4(what.typeParams.size());
    for (SymbolRef s : what.typeParams) {
        p.putU4(s._id);
    }
    p.putU4(what.members.size());
    for (auto member : what.members) {
        p.putU4(member.first.id());
        p.putU4(member.second._id);
    }

    pickle(p, what.resultType.get());
    p.putU4(what.locs().size());
    for (auto &loc : what.locs()) {
        pickle(p, loc);
    }
}

Symbol SerializerImpl::unpickleSymbol(UnPickler &p, GlobalState *gs) {
    Symbol result;
    result.owner = SymbolRef(gs, p.getU4());
    result.name = NameRef(*gs, p.getU4());
    result.superClass = SymbolRef(gs, p.getU4());
    result.flags = p.getU4();
    result.uniqueCounter = p.getU4();
    int argumentsOrMixinsSize = p.getU4();
    result.argumentsOrMixins.reserve(argumentsOrMixinsSize);
    for (int i = 0; i < argumentsOrMixinsSize; i++) {
        result.argumentsOrMixins.emplace_back(SymbolRef(gs, p.getU4()));
    }
    int typeParamsSize = p.getU4();

    result.typeParams.reserve(typeParamsSize);
    for (int i = 0; i < typeParamsSize; i++) {
        result.typeParams.emplace_back(SymbolRef(gs, p.getU4()));
    }

    int membersSize = p.getU4();
    result.members.reserve(membersSize);
    for (int i = 0; i < membersSize; i++) {
        auto name = NameRef(*gs, p.getU4());
        auto sym = SymbolRef(gs, p.getU4());
        result.members[name] = sym;
    }
    result.resultType = unpickleType(p, gs);
    auto locCount = p.getU4();
    for (int i = 0; i < locCount; i++) {
        core::Loc loc;
        auto low = p.getU4();
        auto high = p.getU4();
        loc.setFrom2u4(low, high);
        result.locs_.emplace_back(loc);
    }
    return result;
}

Pickler SerializerImpl::pickle(const GlobalState &gs, bool payloadOnly) {
    Pickler result;
    result.putU4(Serializer::VERSION);

    absl::Span<const shared_ptr<File>> wantFiles;
    if (payloadOnly) {
        auto lastPayload =
            absl::c_find_if(gs.files, [](auto &file) { return file && file->sourceType != File::Payload; });
        ENFORCE(none_of(lastPayload, gs.files.end(), [](auto &file) { return file->sourceType == File::Payload; }));
        wantFiles = absl::Span<const shared_ptr<File>>(gs.files.data(), lastPayload - gs.files.begin());
    } else {
        wantFiles = absl::Span<const shared_ptr<File>>(gs.files.data(), gs.files.size());
    }

    result.putU4(wantFiles.size());
    int i = -1;
    for (auto &f : wantFiles) {
        ++i;
        if (i != 0) {
            pickle(result, *f);
        }
    }

    result.putU4(gs.names.size());
    i = -1;
    for (const Name &n : gs.names) {
        ++i;
        if (i != 0) {
            pickle(result, n);
        }
    }

    result.putU4(gs.symbols.size());
    for (const Symbol &s : gs.symbols) {
        pickle(result, s);
    }

    result.putU4(gs.names_by_hash.size());
    for (const auto &s : gs.names_by_hash) {
        result.putU4(s.first);
        result.putU4(s.second);
    }
    return result;
}

int nearestPowerOf2(int from) {
    int i = 1;
    while (i < from) {
        i = i * 2;
    }
    return i;
}

void SerializerImpl::unpickleGS(UnPickler &p, GlobalState &result) {
    if (p.getU4() != Serializer::VERSION) {
        Exception::raise("Payload version mismatch");
    }

    vector<shared_ptr<File>> files(move(result.files));
    files.clear();
    vector<Name> names(move(result.names));
    names.clear();
    vector<Symbol> symbols(move(result.symbols));
    symbols.clear();
    vector<pair<unsigned int, unsigned int>> names_by_hash(move(result.names_by_hash));
    names_by_hash.clear();

    result.trace("Reading files");

    int filesSize = p.getU4();
    files.reserve(filesSize);
    for (int i = 0; i < filesSize; i++) {
        if (i == 0) {
            files.emplace_back();
        } else {
            files.emplace_back(unpickleFile(p));
        }
    }

    result.trace("Reading names");

    int namesSize = p.getU4();
    ENFORCE(namesSize > 0);
    names.reserve(nearestPowerOf2(namesSize));
    for (int i = 0; i < namesSize; i++) {
        if (i == 0) {
            auto &inserted = names.emplace_back();
            inserted.kind = NameKind::UTF8;
            inserted.raw.utf8 = string_view();
        } else {
            names.emplace_back(unpickleName(p, result));
        }
    }

    result.trace("Reading symbols");

    int symbolSize = p.getU4();
    ENFORCE(symbolSize > 0);
    symbols.reserve(symbolSize);
    for (int i = 0; i < symbolSize; i++) {
        symbols.emplace_back(unpickleSymbol(p, &result));
    }

    result.trace("Reading name table");

    int namesByHashSize = p.getU4();
    names.reserve(namesByHashSize / 2);
    names_by_hash.reserve(names.capacity() * 2);
    for (int i = 0; i < namesByHashSize; i++) {
        auto hash = p.getU4();
        auto value = p.getU4();
        names_by_hash.emplace_back(make_pair(hash, value));
    }

    UnorderedMap<string, FileRef> fileRefByPath;
    result.trace("moving");
    int i = 0;
    for (auto f : files) {
        if (f && !f->path().empty()) {
            fileRefByPath[string(f->path())] = FileRef(i);
        }
        i++;
    }
    result.fileRefByPath = move(fileRefByPath);
    result.files = move(files);
    result.names = move(names);
    result.symbols = move(symbols);
    result.names_by_hash = move(names_by_hash);
    result.sanityCheck();
}

void SerializerImpl::pickle(Pickler &p, Loc loc) {
    auto [low, high] = loc.getAs2u4();
    p.putU4(low);
    p.putU4(high);
}

Loc SerializerImpl::unpickleLoc(UnPickler &p, FileRef file) {
    Loc loc;
    auto low = p.getU4();
    auto high = p.getU4();
    loc.setFrom2u4(low, high);
    loc.setFile(file);
    return loc;
}

vector<u1> Serializer::store(GlobalState &gs) {
    Pickler p = SerializerImpl::pickle(gs);
    return p.result(GLOBAL_STATE_COMPRESSION_DEGREE);
}

vector<u1> Serializer::storePayloadAndNameTable(GlobalState &gs) {
    Pickler p = SerializerImpl::pickle(gs, true);
    return p.result(GLOBAL_STATE_COMPRESSION_DEGREE);
}

void Serializer::loadGlobalState(GlobalState &gs, const u1 *const data) {
    gs.trace("Starting global state deserialization");
    ENFORCE(gs.files.empty() && gs.names.empty() && gs.symbols.empty(), "Can't load into a non-empty state");
    UnPickler p(data);
    gs.trace("Decompression done");
    SerializerImpl::unpickleGS(p, gs);
    gs.installIntrinsics();
    gs.trace("Done reading GS");
}

template <class T> void SerializerImpl::pickleTree(Pickler &p, FileRef file, unique_ptr<T> &t) {
    T *raw = t.get();
    unique_ptr<ast::Expression> tmp(t.release());
    pickle(p, file, tmp);
    t.reset(raw);
    tmp.release();
}

void SerializerImpl::pickleAstHeader(Pickler &p, u1 tag, ast::Expression *tree) {
    p.putU1(tag);
    pickle(p, tree->loc);
}

void SerializerImpl::pickle(Pickler &p, FileRef file, const unique_ptr<ast::Expression> &what) {
    if (what == nullptr) {
        p.putU1(1);
        return;
    }
    ENFORCE(!what->loc.exists() || file == what->loc.file(), "Pickling a tree from file ", what->loc.file().id(),
            " inside a tree from ", file.id());

    typecase(what.get(),
             [&](ast::Send *s) {
                 pickleAstHeader(p, 2, s);
                 p.putU4(s->fun._id);
                 p.putU4(s->flags);
                 p.putU4(s->args.size());
                 pickle(p, file, s->recv);
                 pickleTree(p, file, s->block);
                 for (auto &arg : s->args) {
                     pickle(p, file, arg);
                 }
             },
             [&](ast::Block *a) {
                 pickleAstHeader(p, 3, a);
                 p.putU4(a->symbol._id);
                 p.putU4(a->args.size());
                 pickle(p, file, a->body);
                 for (auto &arg : a->args) {
                     pickle(p, file, arg);
                 };
             },
             [&](ast::Literal *a) {
                 pickleAstHeader(p, 4, a);
                 pickle(p, a->value.get());
             },
             [&](ast::While *a) {
                 pickleAstHeader(p, 5, a);
                 pickle(p, file, a->cond);
                 pickle(p, file, a->body);
             },
             [&](ast::Return *a) {
                 pickleAstHeader(p, 6, a);
                 pickle(p, file, a->expr);
             },
             [&](ast::If *a) {
                 pickleAstHeader(p, 7, a);
                 pickle(p, file, a->cond);
                 pickle(p, file, a->thenp);
                 pickle(p, file, a->elsep);
             },

             [&](ast::UnresolvedConstantLit *a) {
                 pickleAstHeader(p, 8, a);
                 p.putU4(a->cnst._id);
                 pickle(p, file, a->scope);
             },
             [&](ast::Field *a) {
                 pickleAstHeader(p, 9, a);
                 p.putU4(a->symbol._id);
             },
             [&](ast::Local *a) {
                 pickleAstHeader(p, 10, a);
                 p.putU4(a->localVariable._name._id);
                 p.putU4(a->localVariable.unique);
             },
             [&](ast::Self *a) {
                 pickleAstHeader(p, 11, a);
                 p.putU4(a->claz._id);
             },
             [&](ast::Assign *a) {
                 pickleAstHeader(p, 12, a);
                 pickle(p, file, a->lhs);
                 pickle(p, file, a->rhs);
             },
             [&](ast::InsSeq *a) {
                 pickleAstHeader(p, 13, a);
                 p.putU4(a->stats.size());
                 pickle(p, file, a->expr);
                 for (auto &st : a->stats) {
                     pickle(p, file, st);
                 }
             },

             [&](ast::Next *a) {
                 pickleAstHeader(p, 14, a);
                 pickle(p, file, a->expr);
             },

             [&](ast::Break *a) {
                 pickleAstHeader(p, 15, a);
                 pickle(p, file, a->expr);
             },

             [&](ast::Retry *a) { pickleAstHeader(p, 16, a); },

             [&](ast::Hash *h) {
                 pickleAstHeader(p, 17, h);
                 ENFORCE(h->values.size() == h->keys.size());
                 p.putU4(h->values.size());
                 for (auto &v : h->values) {
                     pickle(p, file, v);
                 }
                 for (auto &k : h->keys) {
                     pickle(p, file, k);
                 }
             },

             [&](ast::Array *a) {
                 pickleAstHeader(p, 18, a);
                 p.putU4(a->elems.size());
                 for (auto &e : a->elems) {
                     pickle(p, file, e);
                 }
             },

             [&](ast::Cast *c) {
                 pickleAstHeader(p, 19, c);
                 p.putU4(c->cast._id);
                 pickle(p, c->type.get());
                 pickle(p, file, c->arg);
             },

             [&](ast::EmptyTree *n) { pickleAstHeader(p, 20, n); },
             [&](ast::ClassDef *c) {
                 pickleAstHeader(p, 21, c);
                 pickle(p, c->declLoc);
                 p.putU1(c->kind);
                 p.putU4(c->symbol._id);
                 p.putU4(c->ancestors.size());
                 p.putU4(c->singleton_ancestors.size());
                 p.putU4(c->rhs.size());
                 pickle(p, file, c->name);
                 for (auto &anc : c->ancestors) {
                     pickle(p, file, anc);
                 }
                 for (auto &anc : c->singleton_ancestors) {
                     pickle(p, file, anc);
                 }
                 for (auto &anc : c->rhs) {
                     pickle(p, file, anc);
                 }
             },
             [&](ast::MethodDef *c) {
                 pickleAstHeader(p, 22, c);
                 pickle(p, c->declLoc);
                 p.putU4(c->flags);
                 p.putU4(c->name._id);
                 p.putU4(c->symbol._id);
                 p.putU4(c->args.size());
                 pickle(p, file, c->rhs);
                 for (auto &a : c->args) {
                     pickle(p, file, a);
                 }
             },
             [&](ast::Rescue *a) {
                 pickleAstHeader(p, 23, a);
                 p.putU4(a->rescueCases.size());
                 pickle(p, file, a->ensure);
                 pickle(p, file, a->else_);
                 pickle(p, file, a->body);
                 for (auto &rc : a->rescueCases) {
                     pickleTree(p, file, rc);
                 }
             },
             [&](ast::RescueCase *a) {
                 pickleAstHeader(p, 24, a);
                 p.putU4(a->exceptions.size());
                 pickle(p, file, a->var);
                 pickle(p, file, a->body);
                 for (auto &ex : a->exceptions) {
                     pickle(p, file, ex);
                 }
             },
             [&](ast::RestArg *a) {
                 pickleAstHeader(p, 25, a);
                 pickleTree(p, file, a->expr);
             },
             [&](ast::KeywordArg *a) {
                 pickleAstHeader(p, 26, a);
                 pickleTree(p, file, a->expr);
             },
             [&](ast::ShadowArg *a) {
                 pickleAstHeader(p, 27, a);
                 pickleTree(p, file, a->expr);
             },
             [&](ast::BlockArg *a) {
                 pickleAstHeader(p, 28, a);
                 pickleTree(p, file, a->expr);
             },
             [&](ast::OptionalArg *a) {
                 pickleAstHeader(p, 29, a);
                 pickleTree(p, file, a->expr);
                 pickle(p, file, a->default_);
             },
             [&](ast::Yield *a) {
                 pickleAstHeader(p, 30, a);
                 p.putU4(a->args.size());
                 for (auto &arg : a->args) {
                     pickle(p, file, arg);
                 }
             },
             [&](ast::ZSuperArgs *a) { pickleAstHeader(p, 31, a); },
             [&](ast::UnresolvedIdent *a) {
                 pickleAstHeader(p, 32, a);
                 p.putU1((int)a->kind);
                 p.putU4(a->name._id);
             },
             [&](ast::ConstantLit *a) {
                 pickleAstHeader(p, 33, a);
                 p.putU4(a->symbol._id);
                 pickleTree(p, file, a->original);
                 pickleTree(p, file, a->typeAlias);
             },

             [&](ast::Expression *n) { Exception::raise("Unimplemented AST Node: ", n->nodeName()); });
}

unique_ptr<ast::Expression> SerializerImpl::unpickleExpr(serialize::UnPickler &p, GlobalState &gs, FileRef file) {
    u1 kind = p.getU1();
    if (kind == 1) {
        return nullptr;
    }
    Loc loc = unpickleLoc(p, file);

    switch (kind) {
        case 2: {
            NameRef fun = unpickleNameRef(p, gs);
            auto flags = p.getU4();
            auto argsSize = p.getU4();
            auto recv = unpickleExpr(p, gs, file);
            auto blkt = unpickleExpr(p, gs, file);
            unique_ptr<ast::Block> blk;
            if (blkt) {
                blk.reset(static_cast<ast::Block *>(blkt.release()));
            }
            ast::Send::ARGS_store store(argsSize);
            for (auto &expr : store) {
                expr = unpickleExpr(p, gs, file);
            }
            return ast::MK::Send(loc, move(recv), fun, move(store), flags, move(blk));
        }
        case 3: {
            SymbolRef sym(gs, p.getU4());
            auto argsSize = p.getU4();
            auto body = unpickleExpr(p, gs, file);
            ast::MethodDef::ARGS_store args(argsSize);
            for (auto &arg : args) {
                arg = unpickleExpr(p, gs, file);
            }
            return ast::MK::Block(loc, move(body), move(args), sym);
        }
        case 4: {
            auto tpe = unpickleType(p, &gs);
            return ast::MK::Literal(loc, tpe);
        }
        case 5: {
            auto cond = unpickleExpr(p, gs, file);
            auto body = unpickleExpr(p, gs, file);
            return ast::MK::While(loc, move(cond), move(body));
        }
        case 6: {
            auto expr = unpickleExpr(p, gs, file);
            return ast::MK::Return(loc, move(expr));
        }
        case 7: {
            auto cond = unpickleExpr(p, gs, file);
            auto thenp = unpickleExpr(p, gs, file);
            auto elsep = unpickleExpr(p, gs, file);
            return ast::MK::If(loc, move(cond), move(thenp), move(elsep));
        }
        case 8: {
            NameRef cnst = unpickleNameRef(p, gs);
            auto scope = unpickleExpr(p, gs, file);
            return ast::MK::UnresolvedConstant(loc, move(scope), cnst);
        }
        case 9: {
            SymbolRef sym(gs, p.getU4());
            return make_unique<ast::Field>(loc, sym);
        }
        case 10: {
            NameRef nm = unpickleNameRef(p, gs);
            auto unique = p.getU4();
            LocalVariable lv(nm, unique);
            return make_unique<ast::Local>(loc, lv);
        }
        case 11: {
            SymbolRef clazId(gs, p.getU4());
            return make_unique<ast::Self>(loc, clazId);
        }
        case 12: {
            auto lhs = unpickleExpr(p, gs, file);
            auto rhs = unpickleExpr(p, gs, file);
            return ast::MK::Assign(loc, move(lhs), move(rhs));
        }
        case 13: {
            auto insSize = p.getU4();
            auto expr = unpickleExpr(p, gs, file);
            ast::InsSeq::STATS_store stats(insSize);
            for (auto &stat : stats) {
                stat = unpickleExpr(p, gs, file);
            }
            return ast::MK::InsSeq(loc, move(stats), move(expr));
        }
        case 14: {
            auto expr = unpickleExpr(p, gs, file);
            return ast::MK::Next(loc, move(expr));
        }
        case 15: {
            auto expr = unpickleExpr(p, gs, file);
            return ast::MK::Break(loc, move(expr));
        }
        case 16: {
            return make_unique<ast::Retry>(loc);
        }
        case 17: {
            auto sz = p.getU4();
            ast::Hash::ENTRY_store keys(sz);
            ast::Hash::ENTRY_store values(sz);
            for (auto &value : values) {
                value = unpickleExpr(p, gs, file);
            }
            for (auto &key : keys) {
                key = unpickleExpr(p, gs, file);
            }
            return ast::MK::Hash(loc, move(keys), move(values));
        }
        case 18: {
            auto sz = p.getU4();
            ast::Array::ENTRY_store elems(sz);
            for (auto &elem : elems) {
                elem = unpickleExpr(p, gs, file);
            }
            return ast::MK::Array(loc, move(elems));
        }
        case 19: {
            NameRef kind(gs, p.getU4());
            auto type = unpickleType(p, &gs);
            auto arg = unpickleExpr(p, gs, file);
            return make_unique<ast::Cast>(loc, move(type), move(arg), kind);
        }
        case 20: {
            return ast::MK::EmptyTree(loc);
        }
        case 21: {
            auto declLoc = unpickleLoc(p, file);
            auto kind = p.getU1();
            SymbolRef symbol(gs, p.getU4());
            auto ancestorsSize = p.getU4();
            auto singletonAncestorsSize = p.getU4();
            auto rhsSize = p.getU4();
            auto name = unpickleExpr(p, gs, file);
            ast::ClassDef::ANCESTORS_store ancestors(ancestorsSize);
            for (auto &anc : ancestors) {
                anc = unpickleExpr(p, gs, file);
            }
            ast::ClassDef::ANCESTORS_store singletonAncestors(singletonAncestorsSize);
            for (auto &sanc : singletonAncestors) {
                sanc = unpickleExpr(p, gs, file);
            }
            ast::ClassDef::RHS_store rhs(rhsSize);
            for (auto &r : rhs) {
                r = unpickleExpr(p, gs, file);
            }
            auto ret = ast::MK::Class(loc, declLoc, move(name), move(ancestors), move(rhs), (ast::ClassDefKind)kind);
            ret->singleton_ancestors = move(singletonAncestors);
            ret->symbol = symbol;
            return ret;
        }
        case 22: {
            auto declLoc = unpickleLoc(p, file);
            auto flags = p.getU4();
            NameRef name = unpickleNameRef(p, gs);
            SymbolRef symbol(gs, p.getU4());
            auto argsSize = p.getU4();
            auto rhs = unpickleExpr(p, gs, file);
            ast::MethodDef::ARGS_store args(argsSize);
            for (auto &arg : args) {
                arg = unpickleExpr(p, gs, file);
            }
            auto ret = ast::MK::Method(loc, declLoc, name, move(args), move(rhs));
            ret->flags = flags;
            ret->symbol = symbol;
            return ret;
        }
        case 23: {
            auto rescueCasesSize = p.getU4();
            auto ensure = unpickleExpr(p, gs, file);
            auto else_ = unpickleExpr(p, gs, file);
            auto body_ = unpickleExpr(p, gs, file);
            ast::Rescue::RESCUE_CASE_store cases(rescueCasesSize);
            for (auto &case_ : cases) {
                auto t = unpickleExpr(p, gs, file);
                case_.reset(static_cast<ast::RescueCase *>(t.release()));
            }
            return make_unique<ast::Rescue>(loc, move(body_), move(cases), move(else_), move(ensure));
        }
        case 24: {
            auto exceptionsSize = p.getU4();
            auto var = unpickleExpr(p, gs, file);
            auto body = unpickleExpr(p, gs, file);
            ast::RescueCase::EXCEPTION_store exceptions(exceptionsSize);
            for (auto &ex : exceptions) {
                ex = unpickleExpr(p, gs, file);
            }
            return make_unique<ast::RescueCase>(loc, move(exceptions), move(var), move(body));
        }
        case 25: {
            auto tmp = unpickleExpr(p, gs, file);
            unique_ptr<ast::Reference> ref(static_cast<ast::Reference *>(tmp.release()));
            return make_unique<ast::RestArg>(loc, move(ref));
        }
        case 26: {
            auto tmp = unpickleExpr(p, gs, file);
            unique_ptr<ast::Reference> ref(static_cast<ast::Reference *>(tmp.release()));
            return make_unique<ast::KeywordArg>(loc, move(ref));
        }
        case 27: {
            auto tmp = unpickleExpr(p, gs, file);
            unique_ptr<ast::Reference> ref(static_cast<ast::Reference *>(tmp.release()));
            return make_unique<ast::ShadowArg>(loc, move(ref));
        }
        case 28: {
            auto tmp = unpickleExpr(p, gs, file);
            unique_ptr<ast::Reference> ref(static_cast<ast::Reference *>(tmp.release()));
            return make_unique<ast::BlockArg>(loc, move(ref));
        }
        case 29: {
            auto tmp = unpickleExpr(p, gs, file);
            unique_ptr<ast::Reference> ref(static_cast<ast::Reference *>(tmp.release()));
            auto default_ = unpickleExpr(p, gs, file);
            return make_unique<ast::OptionalArg>(loc, move(ref), move(default_));
        }
        case 30: {
            auto narg = p.getU4();
            ast::Send::ARGS_store args(narg);
            for (auto &arg : args) {
                arg = unpickleExpr(p, gs, file);
            }

            return ast::MK::Yield(loc, move(args));
        }
        case 31: {
            return make_unique<ast::ZSuperArgs>(loc);
        }
        case 32: {
            auto kind = (ast::UnresolvedIdent::VarKind)p.getU1();
            NameRef name = unpickleNameRef(p, gs);
            return make_unique<ast::UnresolvedIdent>(loc, kind, name);
        }
        case 33: {
            SymbolRef sym(gs, p.getU4());
            auto origTmp = unpickleExpr(p, gs, file);
            unique_ptr<ast::UnresolvedConstantLit> orig(static_cast<ast::UnresolvedConstantLit *>(origTmp.release()));
            auto resolved = unpickleExpr(p, gs, file);
            return make_unique<ast::ConstantLit>(loc, sym, move(orig), move(resolved));
        }
    }
    Exception::raise("Not handled {}", kind);
}

unique_ptr<ast::Expression> Serializer::loadExpression(GlobalState &gs, const u1 *const p, u4 forceId) {
    serialize::UnPickler up(p);
    u4 loaded = up.getU4();
    FileRef fileId(forceId > 0 ? forceId : loaded);
    return SerializerImpl::unpickleExpr(up, gs, fileId);
}

vector<u1> Serializer::storeExpression(GlobalState &gs, unique_ptr<ast::Expression> &e) {
    serialize::Pickler pickler;
    pickler.putU4(e->loc.file().id());
    SerializerImpl::pickle(pickler, e->loc.file(), e);
    return pickler.result(FILE_COMPRESSION_DEGREE);
}

NameRef SerializerImpl::unpickleNameRef(UnPickler &p, GlobalState &gs) {
    NameRef name(NameRef::WellKnown{}, p.getU4());
    ENFORCE(name.data(gs)->ref(gs) == name);
    return name;
}

} // namespace sorbet::core::serialize
