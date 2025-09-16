#include "core/serialize/serialize.h"
#include "absl/base/casts.h"
#include "absl/strings/escaping.h" // BytesToHexString
#include "absl/types/span.h"
#include "ast/Helpers.h"
#include "common/crypto_hashing/crypto_hashing.h"
#include "common/sort/sort.h"
#include "common/timers/Timer.h"
#include "core/Error.h"
#include "core/FileHash.h"
#include "core/GlobalState.h"
#include "core/Symbols.h"
#include "core/serialize/pickler.h"
#include "lib/lz4.h"

template class std::vector<uint32_t>;

using namespace std;

namespace sorbet::core::serialize {
const uint32_t Serializer::VERSION;

// These helper methods are declared in a class inside of an anonymous namespace
// or inline so that `GlobalState` can forward-declare and `friend` the entire
// class.
class SerializerImpl {
public:
    static Pickler pickleNameTable(const GlobalState &gs);
    static void pickleNameTable(Pickler &p, const GlobalState &gs);
    static Pickler pickleSymbolTable(const GlobalState &gs);
    static void pickleSymbolTable(Pickler &p, const GlobalState &gs);
    static Pickler pickleFileTable(const GlobalState &gs, bool payloadOnly);
    static void pickleFileTable(Pickler &p, const GlobalState &gs, bool payloadOnly);
    static void pickle(Pickler &p, const File &what);
    static void pickle(Pickler &p, const UTF8Name &what);
    static void pickle(Pickler &p, const ConstantName &what);
    static void pickle(Pickler &p, const UniqueName &what);
    static void pickle(Pickler &p, const TypePtr &what);
    static void pickle(Pickler &p, const ParamInfo &what);
    static void pickle(Pickler &p, const ClassOrModule &what);
    static void pickle(Pickler &p, const Method &what);
    static void pickle(Pickler &p, const Field &what);
    static void pickle(Pickler &p, const TypeParameter &what);
    static void pickle(Pickler &p, const ast::ExpressionPtr &what);
    static void pickle(Pickler &p, core::LocOffsets loc);
    static void pickle(Pickler &p, core::Loc loc);
    static void pickle(Pickler &p, shared_ptr<const FileHash> fh);
    static void pickle(Pickler &p, const ast::UnresolvedConstantLit &lit);

    static shared_ptr<File> unpickleFile(UnPickler &p);
    static UTF8Name unpickleUTF8Name(UnPickler &p, GlobalState &gs);
    static ConstantName unpickleConstantName(UnPickler &p, GlobalState &gs);
    static UniqueName unpickleUniqueName(UnPickler &p, GlobalState &gs);
    static TypePtr unpickleType(UnPickler &p, const GlobalState *gs);
    static ParamInfo unpickleArgInfo(UnPickler &p, const GlobalState *gs);
    static ClassOrModule unpickleClassOrModule(UnPickler &p, const GlobalState *gs);
    static Method unpickleMethod(UnPickler &p, const GlobalState *gs);
    static Field unpickleField(UnPickler &p, const GlobalState *gs);
    static TypeParameter unpickleTypeParameter(UnPickler &p, const GlobalState *gs);
    static void unpickleSymbolTable(UnPickler &g, GlobalState &result);
    static void unpickleNameTable(UnPickler &g, GlobalState &result);
    static void unpickleFileTable(UnPickler &g, GlobalState &result);
    static uint32_t unpickleGSUUID(UnPickler &p);
    static LocOffsets unpickleLocOffsets(UnPickler &p);
    static Loc unpickleLoc(UnPickler &p);
    static ast::ExpressionPtr unpickleExpr(UnPickler &p, const GlobalState &);
    static NameRef unpickleNameRef(UnPickler &p);
    static unique_ptr<const FileHash> unpickleFileHash(UnPickler &p);
    static unique_ptr<ast::UnresolvedConstantLit> unpickleUnresolvedConstantLit(UnPickler &p, const GlobalState &gs);

    SerializerImpl() = delete;
};

void Pickler::putStr(string_view s) {
    putU4(s.size());

    for (char c : s) {
        putU1(absl::bit_cast<uint8_t>(c));
    }
}

constexpr size_t SIZE_BYTES = sizeof(int) / sizeof(uint8_t);
constexpr int LZ4_COMPRESSION_SETTING = 1;

vector<uint8_t> Pickler::result() {
    if (zeroCounter != 0) {
        data.emplace_back(zeroCounter);
        zeroCounter = 0;
    }
    const size_t maxDstSize = LZ4_compressBound(data.size());
    vector<uint8_t> compressedData;
    compressedData.resize(2048 + maxDstSize); // give extra room for compression
                                              // Lizard_compressBound returns size of data if compression
                                              // succeeds. It seems to be written for big inputs
                                              // and returns too small sizes for small inputs,
                                              // where compressed size is bigger than original size
    int resultCode = LZ4_compress_fast((const char *)data.data(), (char *)(compressedData.data() + SIZE_BYTES * 2),
                                       data.size(), (compressedData.size() - SIZE_BYTES * 2), LZ4_COMPRESSION_SETTING);
    if (resultCode == 0) {
        // did not compress!
        Exception::raise("incompressible pickler?");
    } else {
        memcpy(compressedData.data(), &resultCode, SIZE_BYTES); // ~200K of our stdlib
        int uncompressedSize = data.size();
        memcpy(compressedData.data() + SIZE_BYTES, &uncompressedSize,
               SIZE_BYTES);                                     // 172817 ints(x4), ~675K of our stdlib
        int actualCompressedSize = resultCode + SIZE_BYTES * 2; // SIZE_BYTES * 2 are for sizes
        compressedData.resize(actualCompressedSize);
    }
    return compressedData;
}

UnPickler::UnPickler(const uint8_t *const compressed, spdlog::logger &tracer) : pos(0) {
    Timer timeit(tracer, "Unpickler::UnPickler");
    int compressedSize;
    memcpy(&compressedSize, compressed, SIZE_BYTES);
    int uncompressedSize;
    memcpy(&uncompressedSize, compressed + SIZE_BYTES, SIZE_BYTES);

    data.resize(uncompressedSize);

    int resultCode = LZ4_decompress_safe((const char *)(compressed + 2 * SIZE_BYTES), (char *)this->data.data(),
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

void Pickler::putU1(uint8_t u) {
    if (zeroCounter != 0) {
        data.emplace_back(zeroCounter);
        zeroCounter = 0;
    }
    data.emplace_back(u);
}

uint8_t UnPickler::getU1() {
    ENFORCE_NO_TIMER(zeroCounter == 0);
    auto res = data[pos++];
    return res;
}

// Note that this does not necessarily write 4 bytes:
// smaller numbers may take fewer bytes.
//
// NOTE(froydnj): SQLite has a different encoding for varints:
//     https://sqlite.org/src4/doc/trunk/www/varint.wiki
// Given that varint decoding is a signification fraction of cache reading time,
// it may make sense to experiment using this in Sorbet.
void Pickler::putU4(uint32_t u) {
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

uint32_t UnPickler::getU4() {
    if (zeroCounter != 0) {
        zeroCounter--;
        return 0;
    }
    uint8_t r = data[pos++];
    if (r == 0) {
        zeroCounter = data[pos++];
        zeroCounter--;
        return r;
    } else {
        uint32_t res = r & 127;
        uint32_t vle = r;
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
    auto u = absl::bit_cast<uint64_t>(i);
    while (u > 127) {
        putU1((u & 127) | 128);
        u = u >> 7;
    }
    putU1(u & 127);
}

int64_t UnPickler::getS8() {
    uint64_t res = 0;
    uint64_t vle = 128;
    int i = 0;
    while (vle & 128) {
        vle = getU1();
        res |= (vle & 127) << (i * 7);
        i++;
    }
    return absl::bit_cast<int64_t>(res);
}

void SerializerImpl::pickle(Pickler &p, shared_ptr<const FileHash> fh) {
    if (fh == nullptr) {
        p.putU1(0);
        return;
    }
    p.putU1(1);
    p.putU4(fh->localSymbolTableHashes.hierarchyHash);
    p.putU4(fh->localSymbolTableHashes.classModuleHash);
    p.putU4(fh->localSymbolTableHashes.typeMemberHash);
    p.putU4(fh->localSymbolTableHashes.fieldHash);
    p.putU4(fh->localSymbolTableHashes.staticFieldHash);
    p.putU4(fh->localSymbolTableHashes.classAliasHash);
    p.putU4(fh->localSymbolTableHashes.methodHash);
    p.putU4(fh->localSymbolTableHashes.retypecheckableSymbolHashes.size());
    for (const auto &[key, value] : fh->localSymbolTableHashes.retypecheckableSymbolHashes) {
        p.putU4(key._hashValue);
        p.putU4(value);
    }
    p.putU4(fh->usages.nameHashes.size());
    for (const auto &e : fh->usages.nameHashes) {
        p.putU4(e._hashValue);
    }
    p.putU4(fh->foundHashes.staticFieldHashes.size());
    for (const auto &ffh : fh->foundHashes.staticFieldHashes) {
        p.putU4(ffh.ownerIdx);
        p.putU1(ffh.ownerIsSymbol);
        p.putU4(ffh.nameHash._hashValue);
    }
    p.putU4(fh->foundHashes.typeMemberHashes.size());
    for (const auto &ffh : fh->foundHashes.typeMemberHashes) {
        p.putU4(ffh.ownerIdx);
        p.putU1(ffh.ownerIsSymbol);
        p.putU1(ffh.isTypeTemplate);
        p.putU4(ffh.nameHash._hashValue);
    }
    p.putU4(fh->foundHashes.methodHashes.size());
    for (const auto &fdh : fh->foundHashes.methodHashes) {
        p.putU4(fdh.ownerIdx);
        p.putU1(fdh.ownerIsSymbol);
        p.putU1(fdh.useSingletonClass);
        p.putU4(fdh.nameHash._hashValue);
        p.putU4(fdh.arityHash._hashValue);
    }
    p.putU4(fh->foundHashes.fieldHashes.size());
    for (const auto &ffh : fh->foundHashes.fieldHashes) {
        p.putU4(ffh.ownerIdx);
        p.putU1(ffh.ownerIsSymbol);
        p.putU1(ffh.onSingletonClass);
        p.putU1(ffh.isInstanceVariable);
        p.putU1(ffh.fromWithinMethod);
        p.putU4(ffh.nameHash._hashValue);
    }
}

unique_ptr<const FileHash> SerializerImpl::unpickleFileHash(UnPickler &p) {
    auto hadIt = p.getU1() != 0;
    if (!hadIt) {
        return nullptr;
    }
    FileHash ret;

    ret.localSymbolTableHashes.hierarchyHash = p.getU4();
    ret.localSymbolTableHashes.classModuleHash = p.getU4();
    ret.localSymbolTableHashes.typeMemberHash = p.getU4();
    ret.localSymbolTableHashes.fieldHash = p.getU4();
    ret.localSymbolTableHashes.staticFieldHash = p.getU4();
    ret.localSymbolTableHashes.classAliasHash = p.getU4();
    ret.localSymbolTableHashes.methodHash = p.getU4();
    auto retypecheckableSymbolHashSize = p.getU4();
    ret.localSymbolTableHashes.retypecheckableSymbolHashes.reserve(retypecheckableSymbolHashSize);
    for (int it = 0; it < retypecheckableSymbolHashSize; it++) {
        WithoutUniqueNameHash key;
        key._hashValue = p.getU4();
        ret.localSymbolTableHashes.retypecheckableSymbolHashes.emplace_back(key, p.getU4());
    }
    auto constantsSize = p.getU4();
    ret.usages.nameHashes.reserve(constantsSize);
    for (int it = 0; it < constantsSize; it++) {
        WithoutUniqueNameHash key;
        key._hashValue = p.getU4();
        ret.usages.nameHashes.emplace_back(key);
    }
    auto foundStaticFieldHashesSize = p.getU4();
    ret.foundHashes.staticFieldHashes.reserve(foundStaticFieldHashesSize);
    for (int it = 0; it < foundStaticFieldHashesSize; it++) {
        auto ownerIdx = p.getU4();
        auto ownerIsSymbol = p.getU1();
        FullNameHash fullNameHash;
        fullNameHash._hashValue = p.getU4();
        ret.foundHashes.staticFieldHashes.emplace_back(ownerIdx, ownerIsSymbol, fullNameHash);
    }
    auto foundTypeMemberHashesSize = p.getU4();
    ret.foundHashes.typeMemberHashes.reserve(foundTypeMemberHashesSize);
    for (int it = 0; it < foundTypeMemberHashesSize; it++) {
        auto ownerIdx = p.getU4();
        auto ownerIsSymbol = p.getU1();
        auto isTypeTemplate = p.getU1();
        FullNameHash fullNameHash;
        fullNameHash._hashValue = p.getU4();
        ret.foundHashes.typeMemberHashes.emplace_back(ownerIdx, ownerIsSymbol, isTypeTemplate, fullNameHash);
    }
    auto foundMethodHashesSize = p.getU4();
    ret.foundHashes.methodHashes.reserve(foundMethodHashesSize);
    for (int it = 0; it < foundMethodHashesSize; it++) {
        auto ownerIdx = p.getU4();
        auto ownerIsSymbol = p.getU1();
        auto useSingletonClass = p.getU1();
        FullNameHash fullNameHash;
        fullNameHash._hashValue = p.getU4();
        ArityHash arityHash;
        arityHash._hashValue = p.getU4();
        ret.foundHashes.methodHashes.emplace_back(ownerIdx, ownerIsSymbol, useSingletonClass, fullNameHash, arityHash);
    }
    auto foundFieldHashesSize = p.getU4();
    ret.foundHashes.fieldHashes.reserve(foundFieldHashesSize);
    for (int it = 0; it < foundFieldHashesSize; it++) {
        auto ownerIdx = p.getU4();
        auto ownerIsSymbol = p.getU1();
        auto onSingletonClass = p.getU1();
        auto isInstanceVariable = p.getU1();
        auto fromWithinMethod = p.getU1();
        FullNameHash fullNameHash;
        fullNameHash._hashValue = p.getU4();
        ret.foundHashes.fieldHashes.emplace_back(ownerIdx, ownerIsSymbol, onSingletonClass, isInstanceVariable,
                                                 fromWithinMethod, fullNameHash);
    }
    return make_unique<const FileHash>(move(ret));
}

void SerializerImpl::pickle(Pickler &p, const File &what) {
    p.putU1((uint8_t)what.sourceType);
    p.putStr(what.path());
    p.putStr(what.source());
    pickle(p, what.getFileHash());
}

shared_ptr<File> SerializerImpl::unpickleFile(UnPickler &p) {
    auto t = (File::Type)p.getU1();
    auto path = string(p.getStr());
    auto source = string(p.getStr());
    auto globalStateHash = SerializerImpl::unpickleFileHash(p);
    auto ret = make_shared<File>(std::move(path), std::move(source), t);
    ret->setFileHash(move(globalStateHash));
    return ret;
}

void SerializerImpl::pickle(Pickler &p, const UTF8Name &what) {
    p.putStr(what.utf8);
}

void SerializerImpl::pickle(Pickler &p, const ConstantName &what) {
    p.putU4(what.original.rawId());
}

void SerializerImpl::pickle(Pickler &p, const UniqueName &what) {
    p.putU4(what.original.rawId());
    p.putU4(what.num);
    p.putU1(static_cast<uint8_t>(what.uniqueNameKind));
}

UTF8Name SerializerImpl::unpickleUTF8Name(UnPickler &p, GlobalState &gs) {
    return UTF8Name{gs.enterString(p.getStr())};
}

ConstantName SerializerImpl::unpickleConstantName(UnPickler &p, GlobalState &gs) {
    return ConstantName{NameRef::fromRaw(gs, p.getU4())};
}

UniqueName SerializerImpl::unpickleUniqueName(UnPickler &p, GlobalState &gs) {
    return UniqueName{
        NameRef::fromRaw(gs, p.getU4()),
        p.getU4(),
        (UniqueNameKind)p.getU1(),
    };
}

void SerializerImpl::pickle(Pickler &p, const TypePtr &what) {
    if (what == nullptr) {
        p.putU4(0);
        return;
    }
    p.putU4(static_cast<uint32_t>(what.tag()));
    switch (what.tag()) {
        case TypePtr::Tag::UnresolvedAppliedType:
        case TypePtr::Tag::UnresolvedClassType:
        case TypePtr::Tag::BlamedUntyped: {
            p.putU4(what.untypedBlame().rawId());
            break;
        }
        case TypePtr::Tag::ClassType: {
            auto c = cast_type_nonnull<ClassType>(what);
            p.putU4(c.symbol.id());
            break;
        }
        case TypePtr::Tag::OrType: {
            auto &o = cast_type_nonnull<OrType>(what);
            pickle(p, o.left);
            pickle(p, o.right);
            break;
        }
        case TypePtr::Tag::NamedLiteralType: {
            auto c = cast_type_nonnull<NamedLiteralType>(what);
            p.putU1((uint8_t)c.literalKind);
            switch (c.literalKind) {
                case NamedLiteralType::LiteralTypeKind::Symbol:
                case NamedLiteralType::LiteralTypeKind::String:
                    p.putU4(c.unsafeAsName().rawId());
                    break;
            }
            break;
        }
        case TypePtr::Tag::IntegerLiteralType: {
            auto &i = cast_type_nonnull<IntegerLiteralType>(what);
            p.putS8(i.value);
            break;
        }
        case TypePtr::Tag::FloatLiteralType: {
            auto &f = cast_type_nonnull<FloatLiteralType>(what);
            p.putS8(absl::bit_cast<int64_t>(f.value));
            break;
        }
        case TypePtr::Tag::AndType: {
            auto &a = cast_type_nonnull<AndType>(what);
            pickle(p, a.left);
            pickle(p, a.right);
            break;
        }
        case TypePtr::Tag::TupleType: {
            auto &arr = cast_type_nonnull<TupleType>(what);
            p.putU4(arr.elems.size());
            for (auto &el : arr.elems) {
                pickle(p, el);
            }
            break;
        }
        case TypePtr::Tag::ShapeType: {
            auto &hash = cast_type_nonnull<ShapeType>(what);
            p.putU4(hash.keys.size());
            ENFORCE_NO_TIMER(hash.keys.size() == hash.values.size());
            for (auto &el : hash.keys) {
                pickle(p, el);
            }
            for (auto &el : hash.values) {
                pickle(p, el);
            }
            break;
        }
        case TypePtr::Tag::AliasType: {
            auto alias = cast_type_nonnull<AliasType>(what);
            p.putU4(alias.symbol.rawId());
            break;
        }
        case TypePtr::Tag::LambdaParam: {
            auto &lp = cast_type_nonnull<LambdaParam>(what);
            pickle(p, lp.lowerBound);
            pickle(p, lp.upperBound);
            p.putU4(lp.definition.id());
            break;
        }
        case TypePtr::Tag::AppliedType: {
            auto &at = cast_type_nonnull<AppliedType>(what);
            p.putU4(at.klass.id());
            p.putU4(at.targs.size());
            for (auto &t : at.targs) {
                pickle(p, t);
            }
            break;
        }
        case TypePtr::Tag::TypeVar: {
            auto &tp = cast_type_nonnull<TypeVar>(what);
            p.putU4(tp.sym.id());
            break;
        }
        case TypePtr::Tag::SelfType: {
            break;
        }
        case TypePtr::Tag::MetaType:
        case TypePtr::Tag::SelfTypeParam: {
            Exception::notImplemented();
        }
    }
}

TypePtr SerializerImpl::unpickleType(UnPickler &p, const GlobalState *gs) {
    auto tag = p.getU4(); // though we formally need only uint8_t here, benchmarks suggest that
                          // size difference after compression is small and uint32_t is 10% faster
    if (tag == 0) {
        return TypePtr();
    }

    switch (static_cast<TypePtr::Tag>(tag)) {
        case TypePtr::Tag::BlamedUntyped: {
            SymbolRef blame = SymbolRef::fromRaw(p.getU4());
            return make_type<BlamedUntyped>(blame);
        }
        case TypePtr::Tag::UnresolvedClassType:
        case TypePtr::Tag::UnresolvedAppliedType:
        case TypePtr::Tag::ClassType:
            return make_type<ClassType>(ClassOrModuleRef::fromRaw(p.getU4()));
        case TypePtr::Tag::OrType:
            return OrType::make_shared(unpickleType(p, gs), unpickleType(p, gs));
        case TypePtr::Tag::NamedLiteralType: {
            auto kind = (core::NamedLiteralType::LiteralTypeKind)p.getU1();
            auto name = NameRef::fromRawUnchecked(p.getU4());
            switch (kind) {
                case NamedLiteralType::LiteralTypeKind::String:
                    return make_type<NamedLiteralType>(Symbols::String(), name);
                case NamedLiteralType::LiteralTypeKind::Symbol:
                    return make_type<NamedLiteralType>(Symbols::Symbol(), name);
            }
            Exception::notImplemented();
        }
        case TypePtr::Tag::IntegerLiteralType:
            return make_type<IntegerLiteralType>(p.getS8());
        case TypePtr::Tag::FloatLiteralType:
            return make_type<FloatLiteralType>(absl::bit_cast<double>(p.getS8()));
        case TypePtr::Tag::AndType:
            return AndType::make_shared(unpickleType(p, gs), unpickleType(p, gs));
        case TypePtr::Tag::TupleType: {
            int sz = p.getU4();
            vector<TypePtr> elems(sz);
            for (auto &elem : elems) {
                elem = unpickleType(p, gs);
            }
            auto result = make_type<TupleType>(std::move(elems));
            return result;
        }
        case TypePtr::Tag::ShapeType: {
            int sz = p.getU4();
            vector<TypePtr> keys(sz);
            vector<TypePtr> values(sz);
            for (auto &key : keys) {
                key = unpickleType(p, gs);
            }
            for (auto &value : values) {
                value = unpickleType(p, gs);
            }
            auto result = make_type<ShapeType>(move(keys), move(values));
            return result;
        }
        case TypePtr::Tag::AliasType:
            return make_type<AliasType>(SymbolRef::fromRaw(p.getU4()));
        case TypePtr::Tag::LambdaParam: {
            auto lower = unpickleType(p, gs);
            auto upper = unpickleType(p, gs);
            return make_type<LambdaParam>(TypeMemberRef::fromRaw(p.getU4()), lower, upper);
        }
        case TypePtr::Tag::AppliedType: {
            auto klass = ClassOrModuleRef::fromRaw(p.getU4());
            int sz = p.getU4();
            vector<TypePtr> targs(sz);
            for (auto &t : targs) {
                t = unpickleType(p, gs);
            }
            return make_type<AppliedType>(klass, move(targs));
        }
        case TypePtr::Tag::TypeVar: {
            auto sym = TypeArgumentRef::fromRaw(p.getU4());
            return make_type<TypeVar>(sym);
        }
        case TypePtr::Tag::SelfType: {
            return make_type<SelfType>();
        }
        case TypePtr::Tag::MetaType:
        case TypePtr::Tag::SelfTypeParam:
            Exception::raise("Unknown type tag {}", tag);
    }
}

void SerializerImpl::pickle(Pickler &p, const ParamInfo &what) {
    p.putU4(what.name.rawId());
    p.putU4(what.rebind.id());
    pickle(p, what.loc);
    p.putU1(what.flags.toU1());
    pickle(p, what.type);
}

ParamInfo SerializerImpl::unpickleArgInfo(UnPickler &p, const GlobalState *gs) {
    ParamInfo result;
    result.name = NameRef::fromRaw(*gs, p.getU4());
    result.rebind = core::ClassOrModuleRef::fromRaw(p.getU4());
    result.loc = unpickleLoc(p);
    {
        uint8_t flags = p.getU1();
        result.flags.setFromU1(flags);
    }
    result.type = unpickleType(p, gs);
    return result;
}

void SerializerImpl::pickle(Pickler &p, const Method &what) {
    p.putU4(what.owner.id());
    p.putU4(what.name.rawId());
    p.putU4(what.rebind.id());
    p.putU4(what.flags.serialize());
    p.putU4(what.typeArguments().size());
    for (auto s : what.typeArguments()) {
        p.putU4(s.id());
    }
    p.putU4(what.parameters.size());
    for (const auto &param : what.parameters) {
        pickle(p, param);
    }
    pickle(p, what.resultType);
    p.putU4(what.intrinsicOffset);
    p.putU4(what.locs().size());
    for (auto &loc : what.locs()) {
        pickle(p, loc);
    }
}

Method SerializerImpl::unpickleMethod(UnPickler &p, const GlobalState *gs) {
    Method result;
    result.owner = ClassOrModuleRef::fromRaw(p.getU4());
    result.name = NameRef::fromRaw(*gs, p.getU4());
    result.rebind = ClassOrModuleRef::fromRaw(p.getU4());
    auto flagsU2 = static_cast<uint16_t>(p.getU4());
    Method::Flags flags;
    static_assert(sizeof(flags) == sizeof(flagsU2));
    // Can replace this with std::bit_cast in C++20
    memcpy(&flags, &flagsU2, sizeof(flags));
    result.flags = flags;

    int typeParamsSize = p.getU4();
    if (typeParamsSize != 0) {
        auto &vec = result.getOrCreateTypeArguments();
        for (int i = 0; i < typeParamsSize; i++) {
            vec.emplace_back(TypeArgumentRef::fromRaw(p.getU4()));
        }
    }

    int parametersSize = p.getU4();
    for (int i = 0; i < parametersSize; i++) {
        result.parameters.emplace_back(unpickleArgInfo(p, gs));
    }

    result.resultType = unpickleType(p, gs);
    result.intrinsicOffset = p.getU4();
    auto locCount = p.getU4();
    for (int i = 0; i < locCount; i++) {
        result.locs_.emplace_back(unpickleLoc(p));
    }
    return result;
}

void SerializerImpl::pickle(Pickler &p, const ClassOrModule &what) {
    p.putU4(what.packageRegistryOwner.id());
    p.putU4(what.package.owner.id());
    p.putU4(what.owner.id());
    p.putU4(what.name.rawId());
    p.putU4(what.superClass_.id());
    p.putU4(what.flags.serialize());
    p.putU4(what.mixins_.size());
    for (ClassOrModuleRef s : what.mixins_) {
        p.putU4(s.id());
    }

    p.putU4(what.typeMembers().size());
    for (auto s : what.typeMembers()) {
        p.putU4(s.id());
    }

    p.putU4(what.members().size());
    vector<pair<uint32_t, uint32_t>> membersSorted;

    for (const auto &member : what.members()) {
        membersSorted.emplace_back(member.first.rawId(), member.second.rawId());
    }
    fast_sort(membersSorted, [](auto const &lhs, auto const &rhs) -> bool { return lhs.first < rhs.first; });

    for (const auto &member : membersSorted) {
        p.putU4(member.first);
        p.putU4(member.second);
    }

    pickle(p, what.resultType);
    p.putU4(what.locs().size());
    for (auto &loc : what.locs()) {
        pickle(p, loc);
    }
}

ClassOrModule SerializerImpl::unpickleClassOrModule(UnPickler &p, const GlobalState *gs) {
    ClassOrModule result;
    result.packageRegistryOwner = ClassOrModuleRef::fromRaw(p.getU4());
    result.package = packages::MangledName(ClassOrModuleRef::fromRaw(p.getU4()));
    result.owner = ClassOrModuleRef::fromRaw(p.getU4());
    result.name = NameRef::fromRaw(*gs, p.getU4());
    result.superClass_ = ClassOrModuleRef::fromRaw(p.getU4());
    ClassOrModule::Flags flags;
    uint16_t flagsRaw = p.getU4();
    static_assert(sizeof(flagsRaw) == sizeof(flags));
    // Can replace this with std::bit_cast in C++20
    memcpy(&flags, &flagsRaw, sizeof(flags));
    result.flags = flags;
    int mixinsSize = p.getU4();
    result.mixins_.reserve(mixinsSize);
    for (int i = 0; i < mixinsSize; i++) {
        result.mixins_.emplace_back(ClassOrModuleRef::fromRaw(p.getU4()));
    }

    int typeParamsSize = p.getU4();

    if (typeParamsSize != 0) {
        auto &vec = result.getOrCreateTypeMembers();
        vec.reserve(typeParamsSize);
        for (int i = 0; i < typeParamsSize; i++) {
            vec.emplace_back(TypeMemberRef::fromRaw(p.getU4()));
        }
    }

    int membersSize = p.getU4();
    result.members().reserve(membersSize);
    for (int i = 0; i < membersSize; i++) {
        auto name = NameRef::fromRaw(*gs, p.getU4());
        auto sym = SymbolRef::fromRaw(p.getU4());
        if (result.name != core::Names::Constants::Root() && result.name != core::Names::Constants::NoSymbol() &&
            result.name != core::Names::noMethod()) {
            ENFORCE_NO_TIMER(name.exists());
            ENFORCE_NO_TIMER(sym.exists());
        }
        result.members()[name] = sym;
    }
    result.resultType = unpickleType(p, gs);
    auto locCount = p.getU4();
    for (int i = 0; i < locCount; i++) {
        result.locs_.emplace_back(unpickleLoc(p));
    }
    return result;
}

void SerializerImpl::pickle(Pickler &p, const Field &what) {
    p.putU4(what.owner.id());
    p.putU4(what.name.rawId());
    p.putU1(what.flags.serialize());
    pickle(p, what.resultType);
    p.putU4(what.locs().size());
    for (auto &loc : what.locs()) {
        pickle(p, loc);
    }
}

Field SerializerImpl::unpickleField(UnPickler &p, const GlobalState *gs) {
    Field result;
    result.owner = ClassOrModuleRef::fromRaw(p.getU4());
    result.name = NameRef::fromRaw(*gs, p.getU4());
    auto flagsU1 = p.getU1();
    Field::Flags flags;
    static_assert(sizeof(flags) == sizeof(flagsU1));
    // Can replace this with std::bit_cast in C++20
    memcpy(&flags, &flagsU1, sizeof(flags));
    result.flags = flags;
    result.resultType = unpickleType(p, gs);
    auto locCount = p.getU4();
    for (int i = 0; i < locCount; i++) {
        result.locs_.emplace_back(unpickleLoc(p));
    }
    return result;
}

void SerializerImpl::pickle(Pickler &p, const TypeParameter &what) {
    p.putU4(what.owner.rawId());
    p.putU4(what.name.rawId());
    p.putU1(what.flags.serialize());
    pickle(p, what.resultType);
    p.putU4(what.locs().size());
    for (auto &loc : what.locs()) {
        pickle(p, loc);
    }
}

TypeParameter SerializerImpl::unpickleTypeParameter(UnPickler &p, const GlobalState *gs) {
    TypeParameter result;
    result.owner = SymbolRef::fromRaw(p.getU4());
    result.name = NameRef::fromRaw(*gs, p.getU4());

    auto flagsU1 = p.getU1();
    TypeParameter::Flags flags;
    static_assert(sizeof(flags) == sizeof(flagsU1));
    // Can replace this with std::bit_cast in C++20
    memcpy(&flags, &flagsU1, sizeof(flags));
    result.flags = flags;
    result.resultType = unpickleType(p, gs);
    auto locCount = p.getU4();
    for (int i = 0; i < locCount; i++) {
        result.locs_.emplace_back(unpickleLoc(p));
    }
    return result;
}

uint32_t SerializerImpl::unpickleGSUUID(UnPickler &p) {
    if (p.getU4() != Serializer::VERSION) {
        Exception::raise("Payload version mismatch");
    }
    return p.getU4();
}

Pickler SerializerImpl::pickleFileTable(const GlobalState &gs, bool payloadOnly) {
    Pickler p;
    SerializerImpl::pickleFileTable(p, gs, payloadOnly);
    return p;
}

void SerializerImpl::pickleFileTable(Pickler &p, const GlobalState &gs, bool payloadOnly) {
    p.putU4(gs.files.size());
    int i = -1;
    for (const auto &f : gs.files) {
        ++i;
        if (i != 0) {
            pickle(p, *f);
        }
    }
}

void SerializerImpl::unpickleFileTable(UnPickler &p, GlobalState &result) {
    result.files.clear();
    result.fileRefByPath.clear();

    {
        Timer timeit(result.tracer(), "readFiles");

        int filesSize = p.getU4();
        result.files.reserve(filesSize);
        for (int i = 0; i < filesSize; i++) {
            if (i == 0) {
                result.files.emplace_back();
            } else {
                result.files.emplace_back(unpickleFile(p));
            }
        }
    }

    int i = 0;
    for (auto &f : result.files) {
        if (f && !f->path().empty()) {
            result.fileRefByPath[string(f->path())] = FileRef(i);
        }
        i++;
    }
}

Pickler SerializerImpl::pickleSymbolTable(const GlobalState &gs) {
    Pickler p;
    SerializerImpl::pickleSymbolTable(p, gs);
    return p;
}

void SerializerImpl::pickleSymbolTable(Pickler &p, const GlobalState &gs) {
    p.putU4(gs.classAndModules.size());
    for (const ClassOrModule &s : gs.classAndModules) {
        pickle(p, s);
    }

    p.putU4(gs.methods.size());
    for (const Method &s : gs.methods) {
        pickle(p, s);
    }

    p.putU4(gs.fields.size());
    for (const Field &s : gs.fields) {
        pickle(p, s);
    }

    p.putU4(gs.typeArguments.size());
    for (const TypeParameter &s : gs.typeArguments) {
        pickle(p, s);
    }

    p.putU4(gs.typeMembers.size());
    for (const TypeParameter &s : gs.typeMembers) {
        pickle(p, s);
    }
}

void SerializerImpl::unpickleSymbolTable(UnPickler &p, GlobalState &result) {
    result.classAndModules.clear();
    result.methods.clear();
    result.fields.clear();
    result.typeArguments.clear();
    result.typeMembers.clear();

    {
        Timer timeit(result.tracer(), "readSymbols");

        int classAndModuleSize = p.getU4();
        ENFORCE_NO_TIMER(classAndModuleSize > 0);
        result.classAndModules.reserve(nextPowerOfTwo(classAndModuleSize));
        for (int i = 0; i < classAndModuleSize; i++) {
            result.classAndModules.emplace_back(unpickleClassOrModule(p, &result));
        }

        int methodSize = p.getU4();
        ENFORCE_NO_TIMER(methodSize > 0);
        result.methods.reserve(nextPowerOfTwo(methodSize));
        for (int i = 0; i < methodSize; i++) {
            result.methods.emplace_back(unpickleMethod(p, &result));
        }

        int fieldSize = p.getU4();
        ENFORCE_NO_TIMER(fieldSize > 0);
        result.fields.reserve(nextPowerOfTwo(fieldSize));
        for (int i = 0; i < fieldSize; i++) {
            result.fields.emplace_back(unpickleField(p, &result));
        }

        int typeArgumentSize = p.getU4();
        ENFORCE_NO_TIMER(typeArgumentSize > 0);
        result.typeArguments.reserve(nextPowerOfTwo(typeArgumentSize));
        for (int i = 0; i < typeArgumentSize; i++) {
            result.typeArguments.emplace_back(unpickleTypeParameter(p, &result));
        }

        int typeMemberSize = p.getU4();
        ENFORCE_NO_TIMER(typeMemberSize > 0);
        result.typeMembers.reserve(nextPowerOfTwo(typeMemberSize));
        for (int i = 0; i < typeMemberSize; i++) {
            result.typeMembers.emplace_back(unpickleTypeParameter(p, &result));
        }
    }
}

Pickler SerializerImpl::pickleNameTable(const GlobalState &gs) {
    Pickler p;
    SerializerImpl::pickleNameTable(p, gs);
    return p;
}

void SerializerImpl::pickleNameTable(Pickler &p, const GlobalState &gs) {
    p.putU4(gs.utf8Names.size());
    for (const auto &n : gs.utf8Names) {
        pickle(p, n);
    }
    p.putU4(gs.constantNames.size());
    for (const auto &n : gs.constantNames) {
        pickle(p, n);
    }
    p.putU4(gs.uniqueNames.size());
    for (const auto &n : gs.uniqueNames) {
        pickle(p, n);
    }

    p.putU4(gs.namesByHash.size());
    for (const auto &s : gs.namesByHash) {
        p.putU4(s.hash);
        p.putU4(s.rawId);
    }
}

void SerializerImpl::unpickleNameTable(UnPickler &p, GlobalState &result) {
    result.utf8Names.clear();
    result.constantNames.clear();
    result.uniqueNames.clear();
    result.namesByHash.clear();

    {
        Timer timeit(result.tracer(), "readNames");

        int namesSize = p.getU4();
        ENFORCE_NO_TIMER(namesSize > 0);
        result.utf8Names.reserve(nextPowerOfTwo(namesSize));
        for (int i = 0; i < namesSize; i++) {
            result.utf8Names.emplace_back(unpickleUTF8Name(p, result));
        }
        namesSize = p.getU4();
        ENFORCE_NO_TIMER(namesSize > 0);
        result.constantNames.reserve(nextPowerOfTwo(namesSize));
        for (int i = 0; i < namesSize; i++) {
            result.constantNames.emplace_back(unpickleConstantName(p, result));
        }
        namesSize = p.getU4();
        ENFORCE_NO_TIMER(namesSize > 0);
        result.uniqueNames.reserve(nextPowerOfTwo(namesSize));
        for (int i = 0; i < namesSize; i++) {
            result.uniqueNames.emplace_back(unpickleUniqueName(p, result));
        }
    }

    {
        Timer timeit(result.tracer(), "readNameTable");
        int namesByHashSize = p.getU4();
        result.namesByHash.reserve(namesByHashSize);
        for (int i = 0; i < namesByHashSize; i++) {
            auto hash = p.getU4();
            auto value = p.getU4();
            result.namesByHash.emplace_back(GlobalState::Bucket{hash, value});
        }
    }
}

void SerializerImpl::pickle(Pickler &p, LocOffsets loc) {
    p.putU4(loc.beginLoc);
    // The length of the loc likely encodes as a smaller varint.
    p.putU4(loc.endLoc - loc.beginLoc);
}

void SerializerImpl::pickle(Pickler &p, Loc loc) {
    p.putU4(loc.file().id());
    pickle(p, loc.storage.offsets);
}

Loc SerializerImpl::unpickleLoc(UnPickler &p) {
    FileRef file(p.getU4());
    auto offsets = unpickleLocOffsets(p);
    return Loc(file, offsets);
}

LocOffsets SerializerImpl::unpickleLocOffsets(UnPickler &p) {
    uint32_t start = p.getU4();
    uint32_t length = p.getU4();
    return LocOffsets{start, start + length};
}

vector<uint8_t> Serializer::storeNameTable(const GlobalState &gs) {
    Pickler p;
    p.putU4(Serializer::VERSION);
    p.putU4(gs.kvstoreUuid);
    SerializerImpl::pickleNameTable(p, gs);
    return p.result();
}

Serializer::SerializedGlobalState Serializer::store(const GlobalState &gs) {
    SerializedGlobalState result;
    result.symbolTableData = SerializerImpl::pickleSymbolTable(gs).result();
    result.nameTableData = SerializerImpl::pickleNameTable(gs).result();
    result.fileTableData = SerializerImpl::pickleFileTable(gs, false).result();
    return result;
}

void Serializer::loadAndOverwriteNameTable(GlobalState &gs, const uint8_t *const data) {
    UnPickler p(data, gs.tracer());

    if (p.getU4() != Serializer::VERSION) {
        Exception::raise("Payload version mismatch");
    }

    ENFORCE(gs.kvstoreUuid == 0, "The name table may only be loaded into a fresh GlobalState");
    gs.kvstoreUuid = p.getU4();

    SerializerImpl::unpickleNameTable(p, gs);

    // Check that well-known names are consistent with what the payload would expect.
    if constexpr (debug_mode) {
        // The name of the Sorbet class is well-known, and the name is generated by core/tools/generate_names.cc. If
        // this doesn't match up, something has gone seriously wrong with name table serialization.
        ENFORCE(core::Symbols::Sorbet().data(gs)->name == core::Names::Constants::Sorbet());
        ENFORCE(core::Names::Constants::Sorbet().kind() == core::NameKind::CONSTANT);
        auto name = core::Symbols::Sorbet().data(gs)->name.dataCnst(gs)->original;
        ENFORCE(name.kind() == core::NameKind::UTF8);
        ENFORCE(name.dataUtf8(gs)->utf8 == "Sorbet");

        // Array#bsearch is defined in the payload rbis with no overloads. If that changes, pick a new method that's
        // defined in an rbi, but on a well-known class.
        ENFORCE(core::Symbols::T_Array().data(gs)->name.kind() == core::NameKind::CONSTANT);
        auto bsearch = gs.lookupMethodSymbol(core::Symbols::Array(), gs.lookupNameUTF8("bsearch"));
        ENFORCE(bsearch.exists());
        name = bsearch.data(gs)->name;
        ENFORCE(name.kind() == core::NameKind::UTF8);
        ENFORCE(name.dataUtf8(gs)->utf8 == "bsearch");
    }
}

void Serializer::loadGlobalState(GlobalState &gs, const uint8_t *const symbolTableData,
                                 const uint8_t *const nameTableData, const uint8_t *const fileTableData) {
    ENFORCE_NO_TIMER(gs.files.empty() && gs.namesUsedTotal() == 0 && gs.symbolsUsedTotal() == 0,
                     "Can't load into a non-empty state");
    {
        UnPickler p(symbolTableData, gs.tracer());
        SerializerImpl::unpickleSymbolTable(p, gs);
    }
    {
        UnPickler p(nameTableData, gs.tracer());
        SerializerImpl::unpickleNameTable(p, gs);
    }
    {
        UnPickler p(fileTableData, gs.tracer());
        SerializerImpl::unpickleFileTable(p, gs);
    }
}

void Serializer::loadSymbolTable(GlobalState &gs, const uint8_t *const symbolTableData) {
    ENFORCE_NO_TIMER(gs.symbolsUsedTotal() == 0, "Can't load into a non-empty symbol table");
    UnPickler p(symbolTableData, gs.tracer());
    SerializerImpl::unpickleSymbolTable(p, gs);
}

uint32_t Serializer::loadGlobalStateUUID(const GlobalState &gs, const uint8_t *const data) {
    UnPickler p(data, gs.tracer());
    return SerializerImpl::unpickleGSUUID(p);
}

string Serializer::fileKey(const core::File &file) {
    auto path = file.path();
    string key(path.begin(), path.end());
    key += "//";
    auto hashBytes = crypto_hashing::hash64(file.source());
    key += absl::BytesToHexString(string_view{(char *)hashBytes.data(), size(hashBytes)});
    return key;
}

vector<uint8_t> Serializer::storeTree(const core::File &file, const ast::ParsedFile &tree) {
    Pickler p;
    // See comment in `serialize.h` above `loadTree`.
    p.putU4(file.source().size());
    SerializerImpl::pickle(p, file.getFileHash());
    SerializerImpl::pickle(p, tree.tree);
    return p.result();
}

ast::ExpressionPtr Serializer::loadTree(const core::GlobalState &gs, core::File &file, const uint8_t *const data) {
    UnPickler p(data, gs.tracer());
    uint32_t fileSrcLen = p.getU4();
    if (file.source().size() != fileSrcLen) {
        // See comment in `serialize.h` above `loadTree`.
        // File does not have expected size; bail.
        return nullptr;
    }
    file.setFileHash(SerializerImpl::unpickleFileHash(p));
    return SerializerImpl::unpickleExpr(p, gs);
}

void SerializerImpl::pickle(Pickler &p, const ast::UnresolvedConstantLit &lit) {
    pickle(p, lit.loc);
    p.putU4(lit.cnst.rawId());
    pickle(p, lit.scope);
}

void SerializerImpl::pickle(Pickler &p, const ast::ExpressionPtr &what) {
    if (what == nullptr) {
        p.putU4(0);
        return;
    }

    auto tag = what.tag();
    p.putU4(static_cast<uint32_t>(tag));

    switch (tag) {
        case ast::Tag::Send: {
            auto &s = ast::cast_tree_nonnull<ast::Send>(what);
            pickle(p, s.loc);
            p.putU4(s.fun.rawId());
            pickle(p, s.funLoc);
            uint8_t flags;
            static_assert(sizeof(flags) == sizeof(s.flags));
            // Can replace this with std::bit_cast in C++20
            memcpy(&flags, &s.flags, sizeof(flags));
            p.putU1(flags);
            p.putU4(s.numPosArgs());

            const auto hasBlock = s.hasBlock();

            uint32_t size = s.numNonBlockArgs() + (hasBlock ? 1 : 0);
            p.putU4(size);
            pickle(p, s.recv);

            for (auto &arg : s.nonBlockArgs()) {
                pickle(p, arg);
            }
            if (hasBlock) {
                pickle(p, *s.rawBlock());
            }

            break;
        }

        case ast::Tag::Block: {
            auto &a = ast::cast_tree_nonnull<ast::Block>(what);
            pickle(p, a.loc);
            p.putU4(a.params.size());
            pickle(p, a.body);
            for (auto &param : a.params) {
                pickle(p, param);
            }
            break;
        }

        case ast::Tag::Literal: {
            auto &a = ast::cast_tree_nonnull<ast::Literal>(what);
            pickle(p, a.loc);
            pickle(p, a.value);
            break;
        }

        case ast::Tag::While: {
            auto &a = ast::cast_tree_nonnull<ast::While>(what);
            pickle(p, a.loc);
            pickle(p, a.cond);
            pickle(p, a.body);
            break;
        }

        case ast::Tag::Return: {
            auto &a = ast::cast_tree_nonnull<ast::Return>(what);
            pickle(p, a.loc);
            pickle(p, a.expr);
            break;
        }

        case ast::Tag::If: {
            auto &a = ast::cast_tree_nonnull<ast::If>(what);
            pickle(p, a.loc);
            pickle(p, a.cond);
            pickle(p, a.thenp);
            pickle(p, a.elsep);
            break;
        }

        case ast::Tag::UnresolvedConstantLit: {
            auto &a = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(what);
            pickle(p, a);
            break;
        }

        case ast::Tag::Local: {
            auto &a = ast::cast_tree_nonnull<ast::Local>(what);
            pickle(p, a.loc);
            p.putU4(a.localVariable._name.rawId());
            p.putU4(a.localVariable.unique);
            break;
        }

        case ast::Tag::Assign: {
            auto &a = ast::cast_tree_nonnull<ast::Assign>(what);
            pickle(p, a.loc);
            pickle(p, a.lhs);
            pickle(p, a.rhs);
            break;
        }

        case ast::Tag::InsSeq: {
            auto &a = ast::cast_tree_nonnull<ast::InsSeq>(what);
            pickle(p, a.loc);
            p.putU4(a.stats.size());
            pickle(p, a.expr);
            for (auto &st : a.stats) {
                pickle(p, st);
            }
            break;
        }

        case ast::Tag::Next: {
            auto &a = ast::cast_tree_nonnull<ast::Next>(what);
            pickle(p, a.loc);
            pickle(p, a.expr);
            break;
        }

        case ast::Tag::Break: {
            auto &a = ast::cast_tree_nonnull<ast::Break>(what);
            pickle(p, a.loc);
            pickle(p, a.expr);
            break;
        }

        case ast::Tag::Retry: {
            auto &a = ast::cast_tree_nonnull<ast::Retry>(what);
            pickle(p, a.loc);
            break;
        }

        case ast::Tag::Hash: {
            auto &h = ast::cast_tree_nonnull<ast::Hash>(what);
            pickle(p, h.loc);
            ENFORCE_NO_TIMER(h.values.size() == h.keys.size());
            p.putU4(h.values.size());
            for (auto &v : h.values) {
                pickle(p, v);
            }
            for (auto &k : h.keys) {
                pickle(p, k);
            }
            break;
        }

        case ast::Tag::Array: {
            auto &a = ast::cast_tree_nonnull<ast::Array>(what);
            pickle(p, a.loc);
            p.putU4(a.elems.size());
            for (auto &e : a.elems) {
                pickle(p, e);
            }
            break;
        }

        case ast::Tag::Cast: {
            auto &c = ast::cast_tree_nonnull<ast::Cast>(what);
            pickle(p, c.loc);
            p.putU4(c.cast.rawId());
            pickle(p, c.type);
            pickle(p, c.arg);
            pickle(p, c.typeExpr);
            break;
        }

        case ast::Tag::EmptyTree: {
            break;
        }

        case ast::Tag::ClassDef: {
            auto &c = ast::cast_tree_nonnull<ast::ClassDef>(what);
            pickle(p, c.loc);
            pickle(p, c.declLoc);
            p.putU1(static_cast<uint16_t>(c.kind));
            p.putU4(c.symbol.id());
            p.putU4(c.ancestors.size());
            p.putU4(c.singletonAncestors.size());
            p.putU4(c.rhs.size());
            pickle(p, c.name);
            for (auto &anc : c.ancestors) {
                pickle(p, anc);
            }
            for (auto &anc : c.singletonAncestors) {
                pickle(p, anc);
            }
            for (auto &anc : c.rhs) {
                pickle(p, anc);
            }
            break;
        }

        case ast::Tag::MethodDef: {
            auto &c = ast::cast_tree_nonnull<ast::MethodDef>(what);
            pickle(p, c.loc);
            pickle(p, c.declLoc);
            uint8_t flags;
            static_assert(sizeof(flags) == sizeof(c.flags));
            // Can replace this with std::bit_cast in C++20
            memcpy(&flags, &c.flags, sizeof(flags));
            p.putU1(flags);
            p.putU4(c.name.rawId());
            p.putU4(c.symbol.id());
            p.putU4(c.params.size());
            pickle(p, c.rhs);
            for (auto &param : c.params) {
                pickle(p, param);
            }
            break;
        }

        case ast::Tag::Rescue: {
            auto &a = ast::cast_tree_nonnull<ast::Rescue>(what);
            pickle(p, a.loc);
            p.putU4(a.rescueCases.size());
            pickle(p, a.ensure);
            pickle(p, a.else_);
            pickle(p, a.body);
            for (auto &rc : a.rescueCases) {
                pickle(p, rc);
            }
            break;
        }
        case ast::Tag::RescueCase: {
            auto &a = ast::cast_tree_nonnull<ast::RescueCase>(what);
            pickle(p, a.loc);
            p.putU4(a.exceptions.size());
            pickle(p, a.var);
            pickle(p, a.body);
            for (auto &ex : a.exceptions) {
                pickle(p, ex);
            }
            break;
        }

        case ast::Tag::RestArg: {
            auto &a = ast::cast_tree_nonnull<ast::RestArg>(what);
            pickle(p, a.loc);
            pickle(p, a.expr);
            break;
        }

        case ast::Tag::KeywordArg: {
            auto &a = ast::cast_tree_nonnull<ast::KeywordArg>(what);
            pickle(p, a.loc);
            pickle(p, a.expr);
            break;
        }

        case ast::Tag::ShadowArg: {
            auto &a = ast::cast_tree_nonnull<ast::ShadowArg>(what);
            pickle(p, a.loc);
            pickle(p, a.expr);
            break;
        }

        case ast::Tag::BlockArg: {
            auto &a = ast::cast_tree_nonnull<ast::BlockArg>(what);
            pickle(p, a.loc);
            pickle(p, a.expr);
            break;
        }

        case ast::Tag::OptionalParam: {
            auto &a = ast::cast_tree_nonnull<ast::OptionalParam>(what);
            pickle(p, a.loc);
            pickle(p, a.expr);
            pickle(p, a.default_);
            break;
        }

        case ast::Tag::ZSuperArgs: {
            auto &a = ast::cast_tree_nonnull<ast::ZSuperArgs>(what);
            pickle(p, a.loc);
            break;
        }

        case ast::Tag::UnresolvedIdent: {
            auto &a = ast::cast_tree_nonnull<ast::UnresolvedIdent>(what);
            pickle(p, a.loc);
            p.putU1(static_cast<uint8_t>(a.kind));
            p.putU4(a.name.rawId());
            break;
        }

        case ast::Tag::ConstantLit: {
            auto &a = ast::cast_tree_nonnull<ast::ConstantLit>(what);
            pickle(p, a.loc());
            p.putU4(a.symbol().rawId());
            // This encoding is the same encoding that would be used if we were
            // serializing an UnresolvedConstantLit as an ExpressionPtr, nullptr
            // and all.
            auto *original = a.original();
            if (original == nullptr) {
                p.putU4(0);
            } else {
                p.putU4(uint32_t(ast::Tag::UnresolvedConstantLit));
                pickle(p, *original);
            }
            break;
        }

        case ast::Tag::RuntimeMethodDefinition: {
            auto &a = ast::cast_tree_nonnull<ast::RuntimeMethodDefinition>(what);
            pickle(p, a.loc);
            p.putU4(a.name.rawId());
            p.putU1(a.isSelfMethod);
            break;
        }

        case ast::Tag::Self: {
            auto &a = ast::cast_tree_nonnull<ast::Self>(what);
            pickle(p, a.loc);
            break;
        }

        default:
            Exception::raise("Unimplemented AST Node: {}", what.nodeName());
            break;
    }
}

unique_ptr<ast::UnresolvedConstantLit> SerializerImpl::unpickleUnresolvedConstantLit(UnPickler &p,
                                                                                     const GlobalState &gs) {
    auto loc = unpickleLocOffsets(p);
    NameRef cnst = unpickleNameRef(p);
    auto scope = unpickleExpr(p, gs);
    return make_unique<ast::UnresolvedConstantLit>(loc, std::move(scope), cnst);
}

ast::ExpressionPtr SerializerImpl::unpickleExpr(serialize::UnPickler &p, const GlobalState &gs) {
    auto kind = p.getU4();
    if (kind == 0) {
        return nullptr;
    }

    switch (static_cast<ast::Tag>(kind)) {
        case ast::Tag::Send: {
            auto loc = unpickleLocOffsets(p);
            NameRef fun = unpickleNameRef(p);
            auto funLoc = unpickleLocOffsets(p);
            auto flagsU1 = p.getU1();
            ast::Send::Flags flags;
            static_assert(sizeof(flags) == sizeof(flagsU1));
            // Can replace this with std::bit_cast in C++20
            memcpy(&flags, &flagsU1, sizeof(flags));
            auto numPosArgs = static_cast<uint16_t>(p.getU4());
            auto argsSize = p.getU4();
            auto recv = unpickleExpr(p, gs);
            ast::Send::ARGS_store store(argsSize);
            for (auto &expr : store) {
                expr = unpickleExpr(p, gs);
            }
            return ast::MK::Send(loc, std::move(recv), fun, funLoc, numPosArgs, std::move(store), flags);
        }
        case ast::Tag::Block: {
            auto loc = unpickleLocOffsets(p);
            auto paramsSize = p.getU4();
            auto body = unpickleExpr(p, gs);
            ast::MethodDef::PARAMS_store params(paramsSize);
            for (auto &param : params) {
                param = unpickleExpr(p, gs);
            }
            return ast::MK::Block(loc, std::move(body), std::move(params));
        }
        case ast::Tag::Literal: {
            auto loc = unpickleLocOffsets(p);
            auto tpe = unpickleType(p, &gs);
            return ast::MK::Literal(loc, tpe);
        }
        case ast::Tag::While: {
            auto loc = unpickleLocOffsets(p);
            auto cond = unpickleExpr(p, gs);
            auto body = unpickleExpr(p, gs);
            return ast::MK::While(loc, std::move(cond), std::move(body));
        }
        case ast::Tag::Return: {
            auto loc = unpickleLocOffsets(p);
            auto expr = unpickleExpr(p, gs);
            return ast::MK::Return(loc, std::move(expr));
        }
        case ast::Tag::If: {
            auto loc = unpickleLocOffsets(p);
            auto cond = unpickleExpr(p, gs);
            auto thenp = unpickleExpr(p, gs);
            auto elsep = unpickleExpr(p, gs);
            return ast::MK::If(loc, std::move(cond), std::move(thenp), std::move(elsep));
        }
        case ast::Tag::UnresolvedConstantLit: {
            auto ptr = unpickleUnresolvedConstantLit(p, gs);
            return ast::ExpressionPtr::fromUnique(std::move(ptr));
        }
        case ast::Tag::Local: {
            auto loc = unpickleLocOffsets(p);
            NameRef nm = unpickleNameRef(p);
            auto unique = p.getU4();
            LocalVariable lv(nm, unique);
            return ast::make_expression<ast::Local>(loc, lv);
        }
        case ast::Tag::Assign: {
            auto loc = unpickleLocOffsets(p);
            auto lhs = unpickleExpr(p, gs);
            auto rhs = unpickleExpr(p, gs);
            return ast::MK::Assign(loc, std::move(lhs), std::move(rhs));
        }
        case ast::Tag::InsSeq: {
            auto loc = unpickleLocOffsets(p);
            auto insSize = p.getU4();
            auto expr = unpickleExpr(p, gs);
            ast::InsSeq::STATS_store stats(insSize);
            for (auto &stat : stats) {
                stat = unpickleExpr(p, gs);
            }
            return ast::MK::InsSeq(loc, std::move(stats), std::move(expr));
        }
        case ast::Tag::Next: {
            auto loc = unpickleLocOffsets(p);
            auto expr = unpickleExpr(p, gs);
            return ast::MK::Next(loc, std::move(expr));
        }
        case ast::Tag::Break: {
            auto loc = unpickleLocOffsets(p);
            auto expr = unpickleExpr(p, gs);
            return ast::MK::Break(loc, std::move(expr));
        }
        case ast::Tag::Retry: {
            auto loc = unpickleLocOffsets(p);
            return ast::make_expression<ast::Retry>(loc);
        }
        case ast::Tag::Hash: {
            auto loc = unpickleLocOffsets(p);
            auto sz = p.getU4();
            ast::Hash::ENTRY_store keys(sz);
            ast::Hash::ENTRY_store values(sz);
            for (auto &value : values) {
                value = unpickleExpr(p, gs);
            }
            for (auto &key : keys) {
                key = unpickleExpr(p, gs);
            }
            return ast::MK::Hash(loc, std::move(keys), std::move(values));
        }
        case ast::Tag::Array: {
            auto loc = unpickleLocOffsets(p);
            auto sz = p.getU4();
            ast::Array::ENTRY_store elems(sz);
            for (auto &elem : elems) {
                elem = unpickleExpr(p, gs);
            }
            return ast::MK::Array(loc, std::move(elems));
        }
        case ast::Tag::Cast: {
            auto loc = unpickleLocOffsets(p);
            NameRef kind = NameRef::fromRaw(gs, p.getU4());
            auto type = unpickleType(p, &gs);
            auto arg = unpickleExpr(p, gs);
            auto typeExpr = unpickleExpr(p, gs);
            return ast::make_expression<ast::Cast>(loc, std::move(type), std::move(arg), kind, std::move(typeExpr));
        }
        case ast::Tag::EmptyTree:
            return ast::MK::EmptyTree();

        case ast::Tag::ClassDef: {
            auto loc = unpickleLocOffsets(p);
            auto declLoc = unpickleLocOffsets(p);
            auto kind = p.getU1();
            auto symbol = ClassOrModuleRef::fromRaw(p.getU4());
            auto ancestorsSize = p.getU4();
            auto singletonAncestorsSize = p.getU4();
            auto rhsSize = p.getU4();
            auto name = unpickleExpr(p, gs);
            ast::ClassDef::ANCESTORS_store ancestors(ancestorsSize);
            for (auto &anc : ancestors) {
                anc = unpickleExpr(p, gs);
            }
            ast::ClassDef::ANCESTORS_store singletonAncestors(singletonAncestorsSize);
            for (auto &sanc : singletonAncestors) {
                sanc = unpickleExpr(p, gs);
            }
            ast::ClassDef::RHS_store rhs(rhsSize);
            for (auto &r : rhs) {
                r = unpickleExpr(p, gs);
            }
            auto ret = ast::MK::ClassOrModule(loc, declLoc, std::move(name), std::move(ancestors), std::move(rhs),
                                              (ast::ClassDef::Kind)kind);
            {
                auto &klass = ast::cast_tree_nonnull<ast::ClassDef>(ret);
                klass.singletonAncestors = std::move(singletonAncestors);
                klass.symbol = symbol;
            }
            return ret;
        }
        case ast::Tag::MethodDef: {
            auto loc = unpickleLocOffsets(p);
            auto declLoc = unpickleLocOffsets(p);
            auto flagsU1 = p.getU1();
            ast::MethodDef::Flags flags;
            static_assert(sizeof(flags) == sizeof(flagsU1));
            // Can replace this with std::bit_cast in C++20
            memcpy(&flags, &flagsU1, sizeof(flags));
            NameRef name = unpickleNameRef(p);
            auto symbol = MethodRef::fromRaw(p.getU4());
            auto argsSize = p.getU4();
            auto rhs = unpickleExpr(p, gs);
            ast::MethodDef::PARAMS_store params(argsSize);
            for (auto &param : params) {
                param = unpickleExpr(p, gs);
            }
            auto ret = ast::MK::Method(loc, declLoc, name, std::move(params), std::move(rhs));

            {
                auto &method = ast::cast_tree_nonnull<ast::MethodDef>(ret);
                method.flags = flags;
                method.symbol = symbol;
            }
            return ret;
        }
        case ast::Tag::Rescue: {
            auto loc = unpickleLocOffsets(p);
            auto rescueCasesSize = p.getU4();
            auto ensure = unpickleExpr(p, gs);
            auto else_ = unpickleExpr(p, gs);
            auto body_ = unpickleExpr(p, gs);
            ast::Rescue::RESCUE_CASE_store cases(rescueCasesSize);
            for (auto &case_ : cases) {
                case_ = unpickleExpr(p, gs);
            }
            return ast::make_expression<ast::Rescue>(loc, std::move(body_), std::move(cases), std::move(else_),
                                                     std::move(ensure));
        }
        case ast::Tag::RescueCase: {
            auto loc = unpickleLocOffsets(p);
            auto exceptionsSize = p.getU4();
            auto var = unpickleExpr(p, gs);
            auto body = unpickleExpr(p, gs);
            ast::RescueCase::EXCEPTION_store exceptions(exceptionsSize);
            for (auto &ex : exceptions) {
                ex = unpickleExpr(p, gs);
            }
            return ast::make_expression<ast::RescueCase>(loc, std::move(exceptions), std::move(var), std::move(body));
        }
        case ast::Tag::RestArg: {
            auto loc = unpickleLocOffsets(p);
            auto ref = unpickleExpr(p, gs);
            return ast::make_expression<ast::RestArg>(loc, std::move(ref));
        }
        case ast::Tag::KeywordArg: {
            auto loc = unpickleLocOffsets(p);
            auto ref = unpickleExpr(p, gs);
            return ast::make_expression<ast::KeywordArg>(loc, std::move(ref));
        }
        case ast::Tag::ShadowArg: {
            auto loc = unpickleLocOffsets(p);
            auto ref = unpickleExpr(p, gs);
            return ast::make_expression<ast::ShadowArg>(loc, std::move(ref));
        }
        case ast::Tag::BlockArg: {
            auto loc = unpickleLocOffsets(p);
            auto ref = unpickleExpr(p, gs);
            return ast::make_expression<ast::BlockArg>(loc, std::move(ref));
        }
        case ast::Tag::OptionalParam: {
            auto loc = unpickleLocOffsets(p);
            auto ref = unpickleExpr(p, gs);
            auto default_ = unpickleExpr(p, gs);
            return ast::MK::OptionalParam(loc, std::move(ref), std::move(default_));
        }
        case ast::Tag::ZSuperArgs: {
            auto loc = unpickleLocOffsets(p);
            return ast::make_expression<ast::ZSuperArgs>(loc);
        }
        case ast::Tag::UnresolvedIdent: {
            auto loc = unpickleLocOffsets(p);
            auto kind = (ast::UnresolvedIdent::Kind)p.getU1();
            NameRef name = unpickleNameRef(p);
            return ast::make_expression<ast::UnresolvedIdent>(loc, kind, name);
        }
        case ast::Tag::ConstantLit: {
            auto loc = unpickleLocOffsets(p);
            auto sym = SymbolRef::fromRaw(p.getU4());
            auto litTag = p.getU4();
            unique_ptr<ast::UnresolvedConstantLit> orig;
            if (litTag == uint32_t(ast::Tag::UnresolvedConstantLit)) {
                orig = unpickleUnresolvedConstantLit(p, gs);
                return ast::make_expression<ast::ConstantLit>(sym, std::move(orig));
            } else if (litTag != 0) {
                Exception::raise("Unknown tag for `ConstantLit::original`: {}", litTag);
            }
            ENFORCE(orig == nullptr);
            return ast::make_expression<ast::ConstantLit>(loc, sym);
        }
        case ast::Tag::RuntimeMethodDefinition: {
            auto loc = unpickleLocOffsets(p);
            NameRef name = unpickleNameRef(p);
            auto isSelfMethod = p.getU1();
            return ast::make_expression<ast::RuntimeMethodDefinition>(loc, name, isSelfMethod);
        }
        case ast::Tag::Self: {
            auto loc = unpickleLocOffsets(p);
            return ast::make_expression<ast::Self>(loc);
        }
    }

    Exception::raise("Not handled {}", kind);
}

NameRef SerializerImpl::unpickleNameRef(UnPickler &p) {
    NameRef name = NameRef::fromRawUnchecked(p.getU4());

    // We don't call NameRef::sanityCheck here because basically the only thing that method is going
    // to do is attempt to const_cast (!!) the GlobalState and re-enter the NameRef, in an attempt
    // to ENFORCE that re-entering the thing with the given name produces the same result.
    //
    // For unpickled NameRefs, that might not be the case: the GlobalState we're unpickling a tree
    // that contains NameRef with might be a tree created by `GlobalState::copyForIndex`, which
    // doesn't have a complete name table (it will only have the names created by generate_names.cc)
    // which normally isn't important because mergeIndexResults skips ast::Substitute::run for trees
    // which came out of the cache (e.g., it assumes those trees are getting merged into a
    // GlobalState which was already fetched from the cache and thus already had the name table
    // loaded via Serializer::loadGlobalState).

    return name;
}

} // namespace sorbet::core::serialize
