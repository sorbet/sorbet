#include "core/serialize/serialize.h"
#include "absl/base/casts.h"
#include "absl/types/span.h"
#include "ast/Helpers.h"
#include "common/Timer.h"
#include "common/sort.h"
#include "core/Error.h"
#include "core/GlobalState.h"
#include "core/NameHash.h"
#include "core/Symbols.h"
#include "core/serialize/pickler.h"
#include "lib/lizard_compress.h"
#include "lib/lizard_decompress.h"

#include <array>
#include <type_traits>
#include <tuple>

#include <nmmintrin.h>

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
    static void pickle(Pickler &p, const UTF8Name &what);
    static void pickle(Pickler &p, const ConstantName &what);
    static void pickle(Pickler &p, const UniqueName &what);
    static void pickle(Pickler &p, const TypePtr &what);
    static void pickle(Pickler &p, const ArgInfo &a);
    static void pickle(Pickler &p, const Symbol &what);
    static void pickle(Pickler &p, const ast::ExpressionPtr &what);
    static void pickle(Pickler &p, core::LocOffsets loc);
    static void pickle(Pickler &p, core::Loc loc);
    static void pickle(Pickler &p, shared_ptr<const FileHash> fh);

    static shared_ptr<File> unpickleFile(UnPickler &p);
    static UTF8Name unpickleUTF8Name(UnPickler &p, GlobalState &gs);
    static ConstantName unpickleConstantName(UnPickler &p, GlobalState &gs);
    static UniqueName unpickleUniqueName(UnPickler &p, GlobalState &gs);
    static TypePtr unpickleType(UnPickler &p, const GlobalState *gs);
    static ArgInfo unpickleArgInfo(UnPickler &p, const GlobalState *gs);
    static Symbol unpickleSymbol(UnPickler &p, const GlobalState *gs);
    static void unpickleGS(UnPickler &p, GlobalState &result);
    static u4 unpickleGSUUID(UnPickler &p);
    static LocOffsets unpickleLocOffsets(UnPickler &p);
    static Loc unpickleLoc(UnPickler &p);
    static ast::ExpressionPtr unpickleExpr(UnPickler &p, const GlobalState &);
    static NameRef unpickleNameRef(UnPickler &p, const GlobalState &);
    static NameRef unpickleNameRef(UnPickler &p, GlobalState &);
    static unique_ptr<const FileHash> unpickleFileHash(UnPickler &p);

    SerializerImpl() = delete;
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
    const size_t maxDstSize = Lizard_compressBound(data.size());
    vector<u1> compressedData;
    compressedData.resize(2048 + maxDstSize); // give extra room for compression
                                              // Lizard_compressBound returns size of data if compression
                                              // succeeds. It seems to be written for big inputs
                                              // and returns too small sizes for small inputs,
                                              // where compressed size is bigger than original size
    int resultCode = Lizard_compress((const char *)data.data(), (char *)(compressedData.data() + SIZE_BYTES * 2),
                                     data.size(), (compressedData.size() - SIZE_BYTES * 2), compressionDegree);
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

UnPickler::UnPickler(const u1 *const compressed, spdlog::logger &tracer) : pos(0) {
    Timer timeit(tracer, "Unpickler::UnPickler");
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

namespace {
template <typename MakeItem, std::size_t... Index>
constexpr auto make_array_with_(
    MakeItem const& make, std::index_sequence<Index...>) {
  return std::array<decltype(make(0)), sizeof...(Index)>{{make(Index)...}};
}

template <std::size_t Size, typename MakeItem>
constexpr auto make_array_with(MakeItem const& make) {
  return make_array_with_(make, std::make_index_sequence<Size>{});
}

struct group_varint_table_base_make_item {
  constexpr std::size_t get_d(std::size_t index, std::size_t j) const {
    return 1u + ((index >> (2 * j)) & 3u);
  }
  constexpr std::size_t get_offset(std::size_t index, std::size_t j) const {
    // clang-format off
    return
        (j > 0 ? get_d(index, 0) : 0) +
        (j > 1 ? get_d(index, 1) : 0) +
        (j > 2 ? get_d(index, 2) : 0) +
        (j > 3 ? get_d(index, 3) : 0) +
        0;
    // clang-format on
  }
};

struct group_varint_table_length_make_item : group_varint_table_base_make_item {
  constexpr std::uint8_t operator()(std::size_t index) const {
    return 1u + get_offset(index, 4);
  }
};

//  Reference: http://www.stepanovpapers.com/CIKM_2011.pdf
//
//  From 17 encoded bytes, we may use between 5 and 17 bytes to encode 4
//  integers.  The first byte is a key that indicates how many bytes each of
//  the 4 integers takes:
//
//  bit 0..1: length-1 of first integer
//  bit 2..3: length-1 of second integer
//  bit 4..5: length-1 of third integer
//  bit 6..7: length-1 of fourth integer
//
//  The value of the first byte is used as the index in a table which returns
//  a mask value for the SSSE3 PSHUFB instruction, which takes an XMM register
//  (16 bytes) and shuffles bytes from it into a destination XMM register
//  (optionally setting some of them to 0)
//
//  For example, if the key has value 4, that means that the first integer
//  uses 1 byte, the second uses 2 bytes, the third and fourth use 1 byte each,
//  so we set the mask value so that
//
//  r[0] = a[0]
//  r[1] = 0
//  r[2] = 0
//  r[3] = 0
//
//  r[4] = a[1]
//  r[5] = a[2]
//  r[6] = 0
//  r[7] = 0
//
//  r[8] = a[3]
//  r[9] = 0
//  r[10] = 0
//  r[11] = 0
//
//  r[12] = a[4]
//  r[13] = 0
//  r[14] = 0
//  r[15] = 0

struct group_varint_table_sse_mask_make_item
    : group_varint_table_base_make_item {
  constexpr auto partial_item(
      std::size_t d, std::size_t offset, std::size_t k) const {
    // if k < d, the j'th integer uses d bytes, consume them
    // set remaining bytes in result to 0
    // 0xff: set corresponding byte in result to 0
    return std::uint32_t((k < d ? offset + k : std::size_t(0xff)) << (8 * k));
  }

  constexpr auto item_impl(std::size_t d, std::size_t offset) const {
    // clang-format off
    return
        partial_item(d, offset, 0) |
        partial_item(d, offset, 1) |
        partial_item(d, offset, 2) |
        partial_item(d, offset, 3) |
        0;
    // clang-format on
  }

  constexpr auto item(std::size_t index, std::size_t j) const {
    return item_impl(get_d(index, j), get_offset(index, j));
  }

  constexpr auto operator()(std::size_t index) const {
    return std::array<std::uint32_t, 4>{{
        item(index, 0),
        item(index, 1),
        item(index, 2),
        item(index, 3),
    }};
  }
};

alignas(16) constexpr
    std::array<std::array<std::uint32_t, 4>, 256> groupVarintSSEMasks =
        make_array_with<256>(group_varint_table_sse_mask_make_item{});

constexpr std::array<std::uint8_t, 256> groupVarintLengths =
    make_array_with<256>(group_varint_table_length_make_item{});

static void storeUnaligned(void *p, uint32_t x) {
    memcpy(p, &x, sizeof(x));
}

  static uint8_t key(uint32_t x) {
    // __builtin_clz is undefined for the x==0 case
    return uint8_t(3 - (__builtin_clz(x | 1) / 8));
  }
#if 0
  static size_t b0key(size_t x) { return x & 3; }
  static size_t b1key(size_t x) { return (x >> 2) & 3; }
  static size_t b2key(size_t x) { return (x >> 4) & 3; }
  static size_t b3key(size_t x) { return (x >> 6) & 3; }
#endif

}

void UnPickler::getU4Group(uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d) {
    uint8_t key = getU1();
    __m128i val = _mm_loadu_si128((const __m128i*)(&data[pos]));
    __m128i mask =
        _mm_load_si128((const __m128i*)groupVarintSSEMasks[key].data());
    __m128i r = _mm_shuffle_epi8(val, mask);

    // Extracting 32 bits at a time out of an XMM register is a SSE4 feature
    *a = uint32_t(_mm_extract_epi32(r, 0));
    *b = uint32_t(_mm_extract_epi32(r, 1));
    *c = uint32_t(_mm_extract_epi32(r, 2));
    *d = uint32_t(_mm_extract_epi32(r, 3));

    pos += (groupVarintLengths[key] - 1);
}

void Pickler::putU4Group(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    uint8_t b0key = key(a);
    uint8_t b1key = key(b);
    uint8_t b2key = key(c);
    uint8_t b3key = key(d);
    uint8_t key = (b3key << 6) | (b2key << 4) | (b1key << 2) | b0key;
    putU1(key);
    size_t start = data.size();
    data.resize(start + 16);
    storeUnaligned(&data[start], a);
    start += b0key + 1;
    storeUnaligned(&data[start], b);
    start += b1key + 1;
    storeUnaligned(&data[start], c);
    start += b2key + 1;
    storeUnaligned(&data[start], d);
    start += b3key + 1;
    data.resize(start);
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

u1 UnPickler::getU1() {
    ENFORCE(zeroCounter == 0);
    auto res = data[pos++];
    return res;
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

template<typename T>
struct n_args;

template <typename R, typename Arg0>
struct n_args<R(Arg0)> {
    static constexpr const size_t n = 1;
};

template <typename R, typename Arg0, typename Arg1>
struct n_args<R(Arg0, Arg1)> {
    static constexpr const size_t n = 2;
};

namespace {
template <typename Container, typename Transform>
void serializeGroupwise(Pickler &p, const Container &c, Transform&& toU4) {
    using ElementType = typename Container::value_type;
    using Result = std::result_of_t<Transform(const ElementType&)>;

    if constexpr (std::tuple_size<Result>::value == 1) {
        auto b = c.begin(), e = c.end();
        for ( ; std::distance(b, e) >= 4; b += 4) {
            auto [a] = toU4(*(b + 0));
            auto [bx] = toU4(*(b + 1));
            auto [c] = toU4(*(b + 2));
            auto [d] = toU4(*(b + 3));
            p.putU4Group(a, bx, c, d);
        }
        while (b != e) {
            auto [x] = toU4(*b);
            p.putU4(x);
            ++b;
        }
    } else if constexpr (std::tuple_size<Result>::value == 2) {
        auto b = c.begin(), e = c.end();
        for ( ; std::distance(b, e) >= 2; b += 2) {
            auto [a, bx] = toU4(*(b + 0));
            auto [c, d] = toU4(*(b + 1));
            p.putU4Group(a, bx, c, d);
        }
        while (b != e) {
            auto [a, bx] = toU4(*b);
            p.putU4(a);
            p.putU4(bx);
            ++b;
        }
    }
}

template <typename Transform>
void unserializeGroupwise(UnPickler &p, const size_t numElements, Transform&& fromU4) {
    constexpr size_t nargs = n_args<get_signature<Transform>>::n;

    if constexpr (nargs == 1) {
        auto i = 0;
        for ( ; (numElements - i) >= 4; i += 4) {
            u4 a, b, c, d;
            p.getU4Group(&a, &b, &c, &d);
            fromU4(a);
            fromU4(b);
            fromU4(c);
            fromU4(d);
        }
        while (i != numElements) {
            fromU4(p.getU4());
            ++i;
        }
    } else if constexpr (nargs == 2) {
        auto i = 0;
        for ( ; (numElements - i) >= 2; i += 2) {
            u4 a, b, c, d;
            p.getU4Group(&a, &b, &c, &d);
            fromU4(a, b);
            fromU4(c, d);
        }
        while (i != numElements) {
            u4 a = p.getU4();
            u4 b = p.getU4();
            fromU4(a, b);
            ++i;
        }
    }
}

void serializeNameHash(Pickler &p, const std::vector<core::NameHash> &v) {
    p.putU4(v.size());
    serializeGroupwise(p, v, [](const auto& name) -> std::tuple<u4> { return name._hashValue; });
}
void unserializeNameHash(UnPickler &p, std::vector<core::NameHash> &v) {
    auto size = p.getU4();
    v.reserve(size);
    unserializeGroupwise(p, size, [&v](u4 x) {
        NameHash key;
        key._hashValue = x;
        v.emplace_back(key);
        });
}

void serializeLocAndU4(Pickler &p, Loc loc, u4 value) {
    p.putU4Group(loc.storage.offsets.beginLoc,
                 loc.storage.offsets.endLoc,
                 a.loc.file().id(),
                 value);
}

std::tuple<Loc, u4> unserializeLocAndU4(UnPickler &p) {
    u4 begin, end, fileId, value;
    p.getU4Group(&begin, &end, &fileId, &value);
    return {Loc(FileRef(fileId), LocOffsets(begin, end)), value};
}
}

void SerializerImpl::pickle(Pickler &p, shared_ptr<const FileHash> fh) {
    if (fh == nullptr) {
        p.putU1(0);
        return;
    }
    p.putU1(1);
    p.putU4(fh->definitions.hierarchyHash);
    p.putU4(fh->definitions.methodHashes.size());
    serializeGroupwise(p, fh->definitions.methodHashes, [](const auto &p) -> std::tuple<u4, u4> {
            return {p.first._hashValue, p.second};
        });
    serializeNameHash(p, fh->usages.symbols);
    serializeNameHash(p, fh->usages.sends);
}

unique_ptr<const FileHash> SerializerImpl::unpickleFileHash(UnPickler &p) {
    auto hadIt = p.getU1() != 0;
    if (!hadIt) {
        return nullptr;
    }
    FileHash ret;

    ret.definitions.hierarchyHash = p.getU4();
    auto methodHashSize = p.getU4();
    ret.definitions.methodHashes.reserve(methodHashSize);
    unserializeGroupwise(p, methodHashSize, [&ret](u4 keyhash, u4 value) {
        NameHash key;
        key._hashValue = keyhash;
        ret.definitions.methodHashes.emplace_back(key, value);
        });
    unserializeNameHash(p, ret.usages.symbols);
    unserializeNameHash(p, ret.usages.sends);
    return make_unique<const FileHash>(move(ret));
}

void SerializerImpl::pickle(Pickler &p, const File &what) {
    p.putU1((u1)what.sourceType);
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
    p.putU1(static_cast<u1>(what.uniqueNameKind));
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
    p.putU4(static_cast<u4>(what.tag()));
    switch (what.tag()) {
        case TypePtr::Tag::UnresolvedAppliedType:
        case TypePtr::Tag::UnresolvedClassType:
        case TypePtr::Tag::BlamedUntyped:
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
        case TypePtr::Tag::LiteralType: {
            auto c = cast_type_nonnull<LiteralType>(what);
            p.putU1((u1)c.literalKind);
            switch (c.literalKind) {
                case LiteralType::LiteralTypeKind::Float:
                    p.putS8(absl::bit_cast<int64_t>(c.asFloat()));
                    break;
                case LiteralType::LiteralTypeKind::Integer:
                    p.putS8(c.asInteger());
                    break;
                case LiteralType::LiteralTypeKind::Symbol:
                case LiteralType::LiteralTypeKind::String:
                    p.putS8(c.unsafeAsName().rawId());
                    break;
            }
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
            ENFORCE(hash.keys.size() == hash.values.size());
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
            p.putU4(lp.definition.rawId());
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
            p.putU4(tp.sym.rawId());
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
    auto tag = p.getU4(); // though we formally need only u1 here, benchmarks suggest that
                          // size difference after compression is small and u4 is 10% faster
    if (tag == 0) {
        return TypePtr();
    }

    switch (static_cast<TypePtr::Tag>(tag)) {
        case TypePtr::Tag::BlamedUntyped:
        case TypePtr::Tag::UnresolvedClassType:
        case TypePtr::Tag::UnresolvedAppliedType:
        case TypePtr::Tag::ClassType:
            return make_type<ClassType>(ClassOrModuleRef::fromRaw(p.getU4()));
        case TypePtr::Tag::OrType:
            return OrType::make_shared(unpickleType(p, gs), unpickleType(p, gs));
        case TypePtr::Tag::LiteralType: {
            auto kind = (core::LiteralType::LiteralTypeKind)p.getU1();
            auto value = p.getS8();
            switch (kind) {
                case LiteralType::LiteralTypeKind::Integer:
                    return make_type<LiteralType>(value);
                case LiteralType::LiteralTypeKind::Float:
                    return make_type<LiteralType>(absl::bit_cast<double>(value));
                case LiteralType::LiteralTypeKind::String:
                    return make_type<LiteralType>(Symbols::String(), NameRef::fromRawUnchecked(value));
                case LiteralType::LiteralTypeKind::Symbol:
                    return make_type<LiteralType>(Symbols::Symbol(), NameRef::fromRawUnchecked(value));
            }
            Exception::notImplemented();
        }
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
            return make_type<LambdaParam>(SymbolRef::fromRaw(p.getU4()), lower, upper);
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
            auto sym = SymbolRef::fromRaw(p.getU4());
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

void SerializerImpl::pickle(Pickler &p, const ArgInfo &a) {
    p.putU4Group(a.name.rawId(), a.rebind.rawId(), a.loc.storage.offsets.beginLoc,
                 a.loc.storage.offsets.endLoc);
    p.putU4(a.loc.file().id());
    p.putU1(a.flags.toU1());
    pickle(p, a.type);
}

ArgInfo SerializerImpl::unpickleArgInfo(UnPickler &p, const GlobalState *gs) {
    ArgInfo result;
    u4 nameId, symId, locBegin, locEnd;
    p.getU4Group(&nameId, &symId, &locBegin, &locEnd);
    result.name = NameRef::fromRaw(*gs, nameId);
    result.rebind = core::SymbolRef::fromRaw(symId);
    result.loc = Loc(FileRef(p.getU4()), LocOffsets{locBegin, locEnd});
    {
        u1 flags = p.getU1();
        result.flags.setFromU1(flags);
    }
    result.type = unpickleType(p, gs);
    return result;
}

void SerializerImpl::pickle(Pickler &p, const Symbol &what) {
    p.putU4Group(what.owner.rawId(),
                 what.name.rawId(),
                 what.superClassOrRebind.rawId(),
                 what.flags);
    if (!what.isMethod()) {
        p.putU4(what.mixins_.size());
        for (ClassOrModuleRef s : what.mixins_) {
            p.putU4(s.id());
        }
    }
    p.putU4(what.typeParams.size());
    for (SymbolRef s : what.typeParams) {
        p.putU4(s.rawId());
    }
    if (what.isMethod()) {
        p.putU4(what.arguments().size());
        for (const auto &a : what.arguments()) {
            pickle(p, a);
        }
    }
    p.putU4(what.members().size());
    vector<pair<u4, u4>> membersSorted;

    for (const auto &member : what.members()) {
        membersSorted.emplace_back(member.first.rawId(), member.second.rawId());
    }
    fast_sort(membersSorted, [](auto const &lhs, auto const &rhs) -> bool { return lhs.first < rhs.first; });

    serializeGroupwise(p, membersSorted, [](const auto &m) -> std::tuple<u4, u4> { return {m.first, m.second}; });

    pickle(p, what.resultType);
    p.putU4(what.locs().size());
    for (auto &loc : what.locs()) {
        pickle(p, loc);
    }
}

Symbol SerializerImpl::unpickleSymbol(UnPickler &p, const GlobalState *gs) {
    Symbol result;
    u4 owner, name, superClass, flags;
    p.getU4Group(&owner, &name, &superClass, &flags);
    result.owner = SymbolRef::fromRaw(owner);
    result.name = NameRef::fromRaw(*gs, name);
    result.superClassOrRebind = SymbolRef::fromRaw(superClass);
    result.flags = flags;
    if (!result.isMethod()) {
        int mixinsSize = p.getU4();
        result.mixins_.reserve(mixinsSize);
        for (int i = 0; i < mixinsSize; i++) {
            result.mixins_.emplace_back(ClassOrModuleRef::fromRaw(p.getU4()));
        }
    }
    int typeParamsSize = p.getU4();

    result.typeParams.reserve(typeParamsSize);
    for (int i = 0; i < typeParamsSize; i++) {
        result.typeParams.emplace_back(SymbolRef::fromRaw(p.getU4()));
    }

    if (result.isMethod()) {
        int argsSize = p.getU4();
        for (int i = 0; i < argsSize; i++) {
            result.arguments().emplace_back(unpickleArgInfo(p, gs));
        }
    }
    int membersSize = p.getU4();
    result.members().reserve(membersSize);
    unserializeGroupwise(p, membersSize, [&](u4 a, u4 b) {
            auto name = NameRef::fromRaw(*gs, a);
            auto sym = SymbolRef::fromRaw(b);
            if (result.name != core::Names::Constants::Root() && result.name != core::Names::Constants::NoSymbol() &&
                result.name != core::Names::noMethod()) {
                ENFORCE(name.exists());
                ENFORCE(sym.exists());
            }
            result.members()[name] = sym;
        });

    result.resultType = unpickleType(p, gs);
    auto locCount = p.getU4();
    for (int i = 0; i < locCount; i++) {
        result.locs_.emplace_back(unpickleLoc(p));
    }
    return result;
}

Pickler SerializerImpl::pickle(const GlobalState &gs, bool payloadOnly) {
    Timer timeit(gs.tracer(), "pickleGlobalState");
    Pickler result;
    result.putU4(Serializer::VERSION);
    result.putU4(gs.kvstoreUuid);

    absl::Span<const shared_ptr<File>> wantFiles;
    if (payloadOnly) {
        auto lastPayload =
            absl::c_find_if(gs.files, [](auto &file) { return file && file->sourceType != File::Type::Payload; });
        ENFORCE(
            none_of(lastPayload, gs.files.end(), [](auto &file) { return file->sourceType == File::Type::Payload; }));
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

    result.putU4(gs.utf8Names.size());
    for (const auto &n : gs.utf8Names) {
        pickle(result, n);
    }
    result.putU4(gs.constantNames.size());
    serializeGroupwise(result, gs.constantNames, [](const ConstantName &c) -> std::tuple<int> { return c.original.rawId(); });
    result.putU4(gs.uniqueNames.size());
    for (const auto &n : gs.uniqueNames) {
        pickle(result, n);
    }

    result.putU4(gs.classAndModules.size());
    for (const Symbol &s : gs.classAndModules) {
        pickle(result, s);
    }

    result.putU4(gs.methods.size());
    for (const Symbol &s : gs.methods) {
        pickle(result, s);
    }

    result.putU4(gs.fields.size());
    for (const Symbol &s : gs.fields) {
        pickle(result, s);
    }

    result.putU4(gs.typeArguments.size());
    for (const Symbol &s : gs.typeArguments) {
        pickle(result, s);
    }

    result.putU4(gs.typeMembers.size());
    for (const Symbol &s : gs.typeMembers) {
        pickle(result, s);
    }

    result.putU4(gs.namesByHash.size());
    serializeGroupwise(result, gs.namesByHash, [](const auto &s) -> std::tuple<int, int> { return {s.first, s.second}; });
    return result;
}

u4 SerializerImpl::unpickleGSUUID(UnPickler &p) {
    if (p.getU4() != Serializer::VERSION) {
        Exception::raise("Payload version mismatch");
    }
    return p.getU4();
}

void SerializerImpl::unpickleGS(UnPickler &p, GlobalState &result) {
    Timer timeit(result.tracer(), "unpickleGS");
    result.creation = timeit.getFlowEdge();
    if (p.getU4() != Serializer::VERSION) {
        Exception::raise("Payload version mismatch");
    }

    result.kvstoreUuid = p.getU4();

    vector<shared_ptr<File>> files(std::move(result.files));
    files.clear();
    vector<UTF8Name> utf8Names(std::move(result.utf8Names));
    utf8Names.clear();
    vector<ConstantName> constantNames(std::move(result.constantNames));
    constantNames.clear();
    vector<UniqueName> uniqueNames(std::move(result.uniqueNames));
    uniqueNames.clear();
    vector<Symbol> classAndModules(std::move(result.classAndModules));
    classAndModules.clear();
    vector<Symbol> methods(std::move(result.methods));
    methods.clear();
    vector<Symbol> fields(std::move(result.fields));
    fields.clear();
    vector<Symbol> typeArguments(std::move(result.typeArguments));
    typeArguments.clear();
    vector<Symbol> typeMembers(std::move(result.typeMembers));
    typeMembers.clear();
    vector<pair<unsigned int, unsigned int>> namesByHash(std::move(result.namesByHash));
    namesByHash.clear();
    {
        Timer timeit(result.tracer(), "readFiles");

        int filesSize = p.getU4();
        files.reserve(filesSize);
        for (int i = 0; i < filesSize; i++) {
            if (i == 0) {
                files.emplace_back();
            } else {
                files.emplace_back(unpickleFile(p));
            }
        }
    }

    {
        Timer timeit(result.tracer(), "readNames");

        int namesSize = p.getU4();
        ENFORCE(namesSize > 0);
        utf8Names.reserve(nextPowerOfTwo(namesSize));
        for (int i = 0; i < namesSize; i++) {
            utf8Names.emplace_back(unpickleUTF8Name(p, result));
        }
        namesSize = p.getU4();
        ENFORCE(namesSize > 0);
        constantNames.reserve(nextPowerOfTwo(namesSize));
        unserializeGroupwise(p, namesSize, [&result, &constantNames](u4 id) {
                constantNames.emplace_back(ConstantName{NameRef::fromRaw(result, id)});
            });
        namesSize = p.getU4();
        ENFORCE(namesSize > 0);
        uniqueNames.reserve(nextPowerOfTwo(namesSize));
        for (int i = 0; i < namesSize; i++) {
            uniqueNames.emplace_back(unpickleUniqueName(p, result));
        }
    }

    {
        Timer timeit(result.tracer(), "readSymbols");

        int classAndModuleSize = p.getU4();
        ENFORCE(classAndModuleSize > 0);
        classAndModules.reserve(nextPowerOfTwo(classAndModuleSize));
        for (int i = 0; i < classAndModuleSize; i++) {
            classAndModules.emplace_back(unpickleSymbol(p, &result));
        }

        int methodSize = p.getU4();
        ENFORCE(methodSize > 0);
        methods.reserve(nextPowerOfTwo(methodSize));
        for (int i = 0; i < methodSize; i++) {
            methods.emplace_back(unpickleSymbol(p, &result));
        }

        int fieldSize = p.getU4();
        ENFORCE(fieldSize > 0);
        fields.reserve(nextPowerOfTwo(fieldSize));
        for (int i = 0; i < fieldSize; i++) {
            fields.emplace_back(unpickleSymbol(p, &result));
        }

        int typeArgumentSize = p.getU4();
        ENFORCE(typeArgumentSize > 0);
        typeArguments.reserve(nextPowerOfTwo(typeArgumentSize));
        for (int i = 0; i < typeArgumentSize; i++) {
            typeArguments.emplace_back(unpickleSymbol(p, &result));
        }

        int typeMemberSize = p.getU4();
        ENFORCE(typeMemberSize > 0);
        typeMembers.reserve(nextPowerOfTwo(typeMemberSize));
        for (int i = 0; i < typeMemberSize; i++) {
            typeMembers.emplace_back(unpickleSymbol(p, &result));
        }
    }

    {
        Timer timeit(result.tracer(), "readNameTable");
        int namesByHashSize = p.getU4();
        namesByHash.reserve(namesByHashSize);
        unserializeGroupwise(p, namesByHashSize, [&namesByHash](u4 hash, u4 value) {
            namesByHash.emplace_back(make_pair(hash, value));
            });
    }

    UnorderedMap<string, FileRef> fileRefByPath;
    int i = 0;
    for (auto &f : files) {
        if (f && !f->path().empty()) {
            fileRefByPath[string(f->path())] = FileRef(i);
        }
        i++;
    }

    {
        Timer timeit(result.tracer(), "moving");
        result.fileRefByPath = std::move(fileRefByPath);
        result.files = std::move(files);
        result.utf8Names = std::move(utf8Names);
        result.constantNames = std::move(constantNames);
        result.uniqueNames = std::move(uniqueNames);
        result.classAndModules = std::move(classAndModules);
        result.methods = std::move(methods);
        result.fields = std::move(fields);
        result.typeArguments = std::move(typeArguments);
        result.typeMembers = std::move(typeMembers);
        result.namesByHash = std::move(namesByHash);
    }
    result.sanityCheck();
}

void SerializerImpl::pickle(Pickler &p, LocOffsets loc) {
    p.putU4(loc.beginLoc);
    p.putU4(loc.endLoc);
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
    return LocOffsets{p.getU4(), p.getU4()};
}

vector<u1> Serializer::store(GlobalState &gs) {
    Pickler p = SerializerImpl::pickle(gs);
    return p.result(GLOBAL_STATE_COMPRESSION_DEGREE);
}

std::vector<u1> Serializer::storePayloadAndNameTable(GlobalState &gs) {
    Timer timeit(gs.tracer(), "Serializer::storePayloadAndNameTable");
    Pickler p = SerializerImpl::pickle(gs, true);
    return p.result(GLOBAL_STATE_COMPRESSION_DEGREE);
}

void Serializer::loadGlobalState(GlobalState &gs, const u1 *const data) {
    ENFORCE(gs.files.empty() && gs.namesUsedTotal() == 0 && gs.symbolsUsedTotal() == 0,
            "Can't load into a non-empty state");
    UnPickler p(data, gs.tracer());
    SerializerImpl::unpickleGS(p, gs);
    gs.installIntrinsics();
}

u4 Serializer::loadGlobalStateUUID(const GlobalState &gs, const u1 *const data) {
    UnPickler p(data, gs.tracer());
    return SerializerImpl::unpickleGSUUID(p);
}

vector<u1> Serializer::storeFile(const core::File &file, ast::ParsedFile &tree) {
    Pickler p;
    SerializerImpl::pickle(p, file);
    SerializerImpl::pickle(p, tree.tree);
    return p.result(FILE_COMPRESSION_DEGREE);
}

CachedFile Serializer::loadFile(const core::GlobalState &gs, const u1 *const data) {
    UnPickler p(data, gs.tracer());
    auto file = SerializerImpl::unpickleFile(p);
    file->cached = true;
    auto tree = SerializerImpl::unpickleExpr(p, gs);
    return CachedFile{move(file), move(tree)};
}

void SerializerImpl::pickle(Pickler &p, const ast::ExpressionPtr &what) {
    if (what == nullptr) {
        p.putU4(0);
        return;
    }

    auto tag = what.tag();
    p.putU4(static_cast<u4>(tag));

    switch (tag) {
        case ast::Tag::Send: {
            auto &s = ast::cast_tree_nonnull<ast::Send>(what);
            pickle(p, s.loc);
            u1 flags;
            static_assert(sizeof(flags) == sizeof(s.flags));
            // Can replace this with std::bit_cast in C++20
            memcpy(&flags, &s.flags, sizeof(flags));
            p.putU4Group(s.fun.rawId(), flags, s.numPosArgs, s.args.size());
            pickle(p, s.recv);
            pickle(p, s.block);
            for (auto &arg : s.args) {
                pickle(p, arg);
            }
            break;
        }

        case ast::Tag::Block: {
            auto &a = ast::cast_tree_nonnull<ast::Block>(what);
            serializeLocAndU4(p, a.loc, a.args.size());
            pickle(p, a.body);
            for (auto &arg : a.args) {
                pickle(p, arg);
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
            pickle(p, a.loc);
            p.putU4(a.cnst.rawId());
            pickle(p, a.scope);
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
            ENFORCE(h.values.size() == h.keys.size());
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
            break;
        }

        case ast::Tag::EmptyTree: {
            break;
        }

        case ast::Tag::ClassDef: {
            auto &c = ast::cast_tree_nonnull<ast::ClassDef>(what);
            pickle(p, c.loc);
            pickle(p, c.declLoc);
            p.putU1(static_cast<u2>(c.kind));
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
            u1 flags;
            static_assert(sizeof(flags) == sizeof(c.flags));
            // Can replace this with std::bit_cast in C++20
            memcpy(&flags, &c.flags, sizeof(flags));
            p.putU4Group(flags, c.name.rawId(), c.symbol.id(), c.args.size());
            pickle(p, c.rhs);
            for (auto &a : c.args) {
                pickle(p, a);
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

        case ast::Tag::OptionalArg: {
            auto &a = ast::cast_tree_nonnull<ast::OptionalArg>(what);
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
            p.putU1(static_cast<u1>(a.kind));
            p.putU4(a.name.rawId());
            break;
        }

        case ast::Tag::ConstantLit: {
            auto &a = ast::cast_tree_nonnull<ast::ConstantLit>(what);
            pickle(p, a.loc);
            p.putU4(a.symbol.rawId());
            pickle(p, a.original);
            break;
        }

        default:
            Exception::raise("Unimplemented AST Node: {}", what.nodeName());
            break;
    }
}

ast::ExpressionPtr SerializerImpl::unpickleExpr(serialize::UnPickler &p, const GlobalState &gs) {
    auto kind = p.getU4();
    if (kind == 0) {
        return nullptr;
    }

    switch (static_cast<ast::Tag>(kind)) {
        case ast::Tag::Send: {
            auto loc = unpickleLocOffsets(p);
            u4 rawName, flagsU4, numPosArgsU4, argsSize;
            p.getU4Group(&rawName, &flagsU4, &numPosArgsU4, &argsSize);
            NameRef fun = NameRef::fromRawUnchecked(rawName);
            u1 flagsU1 = static_cast<u1>(flagsU4);
            ast::Send::Flags flags;
            static_assert(sizeof(flags) == sizeof(flagsU1));
            // Can replace this with std::bit_cast in C++20
            memcpy(&flags, &flagsU1, sizeof(flags));
            auto numPosArgs = static_cast<u2>(numPosArgsU4);
            auto recv = unpickleExpr(p, gs);
            auto blkt = unpickleExpr(p, gs);
            ast::ExpressionPtr blk;
            if (blkt) {
                blk.reset(static_cast<ast::Block *>(blkt.release()));
            }
            ast::Send::ARGS_store store(argsSize);
            for (auto &expr : store) {
                expr = unpickleExpr(p, gs);
            }
            return ast::MK::Send(loc, std::move(recv), fun, numPosArgs, std::move(store), flags, std::move(blk));
        }
        case ast::Tag::Block: {
            auto [loc, argsSize] = unserializeLocAndU4(p);
            auto body = unpickleExpr(p, gs);
            ast::MethodDef::ARGS_store args(argsSize);
            for (auto &arg : args) {
                arg = unpickleExpr(p, gs);
            }
            return ast::MK::Block(loc, std::move(body), std::move(args));
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
            auto loc = unpickleLocOffsets(p);
            NameRef cnst = unpickleNameRef(p, gs);
            auto scope = unpickleExpr(p, gs);
            return ast::MK::UnresolvedConstant(loc, std::move(scope), cnst);
        }
        case ast::Tag::Local: {
            auto loc = unpickleLocOffsets(p);
            NameRef nm = unpickleNameRef(p, gs);
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
            return ast::make_expression<ast::Cast>(loc, std::move(type), std::move(arg), kind);
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
                auto klass = ast::cast_tree<ast::ClassDef>(ret);
                klass->singletonAncestors = std::move(singletonAncestors);
                klass->symbol = symbol;
            }
            return ret;
        }
        case ast::Tag::MethodDef: {
            auto loc = unpickleLocOffsets(p);
            auto declLoc = unpickleLocOffsets(p);
            u4 flagsU4, nameId, symId, argsSize;
            p.getU4Group(&flagsU4, &nameId, &symId, &argsSize);
            auto flagsU1 = static_cast<u1>(flagsU4);
            ast::MethodDef::Flags flags;
            static_assert(sizeof(flags) == sizeof(flagsU1));
            // Can replace this with std::bit_cast in C++20
            memcpy(&flags, &flagsU1, sizeof(flags));
            NameRef name = NameRef::fromRawUnchecked(nameId);
            auto symbol = MethodRef::fromRaw(symId);
            auto rhs = unpickleExpr(p, gs);
            ast::MethodDef::ARGS_store args(argsSize);
            for (auto &arg : args) {
                arg = unpickleExpr(p, gs);
            }
            auto ret = ast::MK::SyntheticMethod(loc, declLoc, name, std::move(args), std::move(rhs));

            {
                auto *method = ast::cast_tree<ast::MethodDef>(ret);
                method->flags = flags;
                method->symbol = symbol;
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
                auto t = unpickleExpr(p, gs);
                case_.reset(static_cast<ast::RescueCase *>(t.release()));
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
        case ast::Tag::OptionalArg: {
            auto loc = unpickleLocOffsets(p);
            auto ref = unpickleExpr(p, gs);
            auto default_ = unpickleExpr(p, gs);
            return ast::MK::OptionalArg(loc, std::move(ref), std::move(default_));
        }
        case ast::Tag::ZSuperArgs: {
            auto loc = unpickleLocOffsets(p);
            return ast::make_expression<ast::ZSuperArgs>(loc);
        }
        case ast::Tag::UnresolvedIdent: {
            auto loc = unpickleLocOffsets(p);
            auto kind = (ast::UnresolvedIdent::Kind)p.getU1();
            NameRef name = unpickleNameRef(p, gs);
            return ast::make_expression<ast::UnresolvedIdent>(loc, kind, name);
        }
        case ast::Tag::ConstantLit: {
            auto loc = unpickleLocOffsets(p);
            auto sym = SymbolRef::fromRaw(p.getU4());
            auto orig = unpickleExpr(p, gs);
            return ast::make_expression<ast::ConstantLit>(loc, sym, std::move(orig));
        }
    }

    Exception::raise("Not handled {}", kind);
}

NameRef SerializerImpl::unpickleNameRef(UnPickler &p, const GlobalState &gs) {
    NameRef name = NameRef::fromRawUnchecked(p.getU4());
    name.sanityCheck(gs);
    return name;
}

} // namespace sorbet::core::serialize
