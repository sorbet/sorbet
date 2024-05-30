// have to be included first as they violate our poisons
#include "core/proto/proto.h"
#include "src/google/protobuf/io/zero_copy_stream_impl.h"
#include "src/google/protobuf/util/type_resolver_util.h"

#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "common/Random.h"
#include "common/counters/Counters_impl.h"
#include "common/typecase.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::core {

com::stripe::rubytyper::Name Proto::toProto(const GlobalState &gs, NameRef name) {
    com::stripe::rubytyper::Name protoName;
    protoName.set_name(name.show(gs));
    protoName.set_unique(com::stripe::rubytyper::Name::NOT_UNIQUE);
    switch (name.kind()) {
        case NameKind::UTF8:
            protoName.set_kind(com::stripe::rubytyper::Name::UTF8);
            break;
        case NameKind::UNIQUE:
            protoName.set_kind(com::stripe::rubytyper::Name::UNIQUE);
            switch (name.dataUnique(gs)->uniqueNameKind) {
                case UniqueNameKind::Parser:
                    protoName.set_unique(com::stripe::rubytyper::Name::PARSER);
                    break;
                case UniqueNameKind::Desugar:
                    protoName.set_unique(com::stripe::rubytyper::Name::DESUGAR);
                    break;
                case UniqueNameKind::Namer:
                    protoName.set_unique(com::stripe::rubytyper::Name::NAMER);
                    break;
                case UniqueNameKind::MangleRename:
                    protoName.set_unique(com::stripe::rubytyper::Name::MANGLE_RENAME);
                    break;
                case UniqueNameKind::MangleRenameOverload:
                    protoName.set_unique(com::stripe::rubytyper::Name::MANGLE_RENAME_OVERLOAD);
                    break;
                case UniqueNameKind::Singleton:
                    protoName.set_unique(com::stripe::rubytyper::Name::SINGLETON);
                    break;
                case UniqueNameKind::Overload:
                    protoName.set_unique(com::stripe::rubytyper::Name::OVERLOAD);
                    break;
                case UniqueNameKind::TypeVarName:
                    protoName.set_unique(com::stripe::rubytyper::Name::TYPE_VAR_NAME);
                    break;
                case UniqueNameKind::PositionalArg:
                    protoName.set_unique(com::stripe::rubytyper::Name::POSITIONAL_ARG);
                    break;
                case UniqueNameKind::MangledKeywordArg:
                    protoName.set_unique(com::stripe::rubytyper::Name::MANGLED_KEYWORD_ARG);
                    break;
                case UniqueNameKind::ResolverMissingClass:
                    protoName.set_unique(com::stripe::rubytyper::Name::RESOLVER_MISSING_CLASS);
                    break;
                case UniqueNameKind::TEnum:
                    protoName.set_unique(com::stripe::rubytyper::Name::OPUS_ENUM);
                    break;
                case UniqueNameKind::Struct:
                    protoName.set_unique(com::stripe::rubytyper::Name::STRUCT);
                    break;
                case UniqueNameKind::Packager:
                    protoName.set_unique(com::stripe::rubytyper::Name::PACKAGER);
                    break;
            }
            break;
        case NameKind::CONSTANT:
            protoName.set_kind(com::stripe::rubytyper::Name::CONSTANT);
            break;
    }
    return protoName;
}

com::stripe::rubytyper::Symbol::ArgumentInfo Proto::toProto(const GlobalState &gs, const ArgInfo &arg) {
    com::stripe::rubytyper::Symbol::ArgumentInfo argProto;
    *argProto.mutable_name() = toProto(gs, arg.name);
    argProto.set_iskeyword(arg.flags.isKeyword);
    argProto.set_isrepeated(arg.flags.isRepeated);
    argProto.set_isdefault(arg.flags.isDefault);
    argProto.set_isshadow(arg.flags.isShadow);
    argProto.set_isblock(arg.flags.isBlock);

    return argProto;
}
com::stripe::rubytyper::Symbol Proto::toProto(const GlobalState &gs, SymbolRef sym, bool showFull) {
    com::stripe::rubytyper::Symbol symbolProto;

    symbolProto.set_id(sym.rawId());
    *symbolProto.mutable_name() = toProto(gs, sym.name(gs));

    if (sym.isClassOrModule()) {
        symbolProto.set_kind(com::stripe::rubytyper::Symbol::CLASS_OR_MODULE);
    } else if (sym.isStaticField(gs)) {
        symbolProto.set_kind(com::stripe::rubytyper::Symbol::STATIC_FIELD);
    } else if (sym.isField(gs)) {
        symbolProto.set_kind(com::stripe::rubytyper::Symbol::FIELD);
    } else if (sym.isMethod()) {
        symbolProto.set_kind(com::stripe::rubytyper::Symbol::METHOD);
    } else if (sym.isTypeMember()) {
        symbolProto.set_kind(com::stripe::rubytyper::Symbol::TYPE_MEMBER);
    } else if (sym.isTypeArgument()) {
        symbolProto.set_kind(com::stripe::rubytyper::Symbol::TYPE_ARGUMENT);
    }

    if (sym.isClassOrModule() || sym.isMethod()) {
        if (sym.isClassOrModule()) {
            auto klass = sym.asClassOrModuleRef();
            for (auto thing : klass.data(gs)->mixins()) {
                symbolProto.add_mixins(SymbolRef(thing).rawId());
            }

            if (klass.data(gs)->superClass().exists()) {
                symbolProto.set_superclass(SymbolRef(klass.data(gs)->superClass()).rawId());
            }
        } else {
            auto method = sym.asMethodRef();
            for (auto &thing : method.data(gs)->arguments) {
                *symbolProto.add_arguments() = toProto(gs, thing);
            }
        }
    }

    if (sym.isStaticField(gs)) {
        auto field = sym.asFieldRef();
        if (core::isa_type<core::AliasType>(field.data(gs)->resultType)) {
            auto type = core::cast_type_nonnull<AliasType>(field.data(gs)->resultType);
            symbolProto.set_aliasto(type.symbol.rawId());
        }
    }

    if (sym.isClassOrModule()) {
        for (auto pair : sym.asClassOrModuleRef().data(gs)->membersStableOrderSlow(gs)) {
            if (pair.first == Names::singleton() || pair.first == Names::attached() ||
                pair.first == Names::mixedInClassMethods() || pair.first == Names::Constants::AttachedClass()) {
                continue;
            }

            if (!pair.second.exists()) {
                continue;
            }

            if (!showFull && !pair.second.isPrintable(gs)) {
                continue;
            }

            *symbolProto.add_children() = toProto(gs, pair.second, showFull);
        }
    } else if (sym.isMethod()) {
        for (auto typeArg : sym.asMethodRef().data(gs)->typeArguments()) {
            if (!typeArg.exists()) {
                continue;
            }

            if (!showFull && !typeArg.data(gs)->isPrintable(gs)) {
                continue;
            }
            *symbolProto.add_children() = toProto(gs, typeArg, showFull);
        }
    }

    return symbolProto;
}

com::stripe::rubytyper::Type::Literal Proto::toProto(const GlobalState &gs, const NamedLiteralType &lit) {
    com::stripe::rubytyper::Type::Literal proto;

    switch (lit.literalKind) {
        case NamedLiteralType::LiteralTypeKind::String:
            proto.set_kind(com::stripe::rubytyper::Type::Literal::STRING);
            proto.set_string(lit.asName().show(gs));
            break;
        case NamedLiteralType::LiteralTypeKind::Symbol:
            proto.set_kind(com::stripe::rubytyper::Type::Literal::SYMBOL);
            proto.set_symbol(lit.asName().show(gs));
            break;
    }
    return proto;
}

com::stripe::rubytyper::Type::LiteralInteger Proto::toProto(const GlobalState &gs, const IntegerLiteralType &lit) {
    com::stripe::rubytyper::Type::LiteralInteger proto;
    proto.set_integer(lit.value);
    return proto;
}

com::stripe::rubytyper::Type::LiteralFloat Proto::toProto(const GlobalState &gs, const FloatLiteralType &lit) {
    com::stripe::rubytyper::Type::LiteralFloat proto;
    proto.set_float_(lit.value);
    return proto;
}

com::stripe::rubytyper::Type Proto::toProto(const GlobalState &gs, const TypePtr &typ) {
    com::stripe::rubytyper::Type proto;
    typecase(
        typ,
        [&](const ClassType &t) {
            proto.set_kind(com::stripe::rubytyper::Type::CLASS);
            proto.set_class_full_name(t.symbol.show(gs));
        },
        [&](const AndType &t) {
            proto.set_kind(com::stripe::rubytyper::Type::AND);
            *proto.mutable_and_()->mutable_left() = toProto(gs, t.left);
            *proto.mutable_and_()->mutable_right() = toProto(gs, t.right);
        },
        [&](const OrType &t) {
            proto.set_kind(com::stripe::rubytyper::Type::OR);
            *proto.mutable_or_()->mutable_left() = toProto(gs, t.left);
            *proto.mutable_or_()->mutable_right() = toProto(gs, t.right);
        },
        [&](const AppliedType &t) {
            proto.set_kind(com::stripe::rubytyper::Type::APPLIED);
            proto.mutable_applied()->set_symbol_full_name(t.klass.show(gs));
            for (auto &a : t.targs) {
                *proto.mutable_applied()->add_type_args() = toProto(gs, a);
            }
        },
        [&](const ShapeType &t) {
            proto.set_kind(com::stripe::rubytyper::Type::SHAPE);
            for (auto &k : t.keys) {
                *proto.mutable_shape()->add_keys() = toProto(gs, k);
            }
            for (auto &v : t.values) {
                *proto.mutable_shape()->add_values() = toProto(gs, v);
            }
        },
        [&](const NamedLiteralType &t) {
            proto.set_kind(com::stripe::rubytyper::Type::LITERAL);
            *proto.mutable_literal() = toProto(gs, t);
        },
        [&](const IntegerLiteralType &t) {
            proto.set_kind(com::stripe::rubytyper::Type::LITERALINTEGER);
            *proto.mutable_literalinteger() = toProto(gs, t);
        },
        [&](const FloatLiteralType &t) {
            proto.set_kind(com::stripe::rubytyper::Type::LITERALINTEGER);
            *proto.mutable_literalfloat() = toProto(gs, t);
        },
        [&](const TupleType &t) {
            proto.set_kind(com::stripe::rubytyper::Type::TUPLE);
            for (auto &e : t.elems) {
                *proto.mutable_tuple()->add_elems() = toProto(gs, e);
            }
        },
        // TODO later: add more types
        [&](const TypePtr &t) { proto.set_kind(com::stripe::rubytyper::Type::UNKNOWN); });
    return proto;
}

com::stripe::rubytyper::Loc Proto::toProto(const GlobalState &gs, Loc loc) {
    com::stripe::rubytyper::Loc protoLoc;
    auto position = protoLoc.mutable_position();
    auto start = position->mutable_start();
    auto end = position->mutable_end();

    if (!loc.exists()) {
        protoLoc.set_path("???");
    } else {
        auto path = loc.file().data(gs).path();
        protoLoc.set_path(string(path));

        auto pos = loc.position(gs);
        start->set_line(pos.first.line);
        start->set_column(pos.first.column);
        end->set_line(pos.second.line);
        end->set_column(pos.second.column);
    }

    return protoLoc;
}

com::stripe::payserver::events::cibot::SourceMetrics Proto::toProto(const CounterState &counters, string_view prefix) {
    com::stripe::payserver::events::cibot::SourceMetrics metrics;
    auto unix_timestamp = chrono::seconds(time(nullptr));
    metrics.set_timestamp(unix_timestamp.count());

    // UUID version 1 as specified in RFC 4122
    string uuid = fmt::format(
        "{:#08x}-{:#04x}-{:#04x}-{:#04x}-{:#08x}{:#04x}",
        (unsigned long long)Random::uniformU8(), // Generates a 64-bit Hex number
        Random::uniformU4(),                     // Generates a 32-bit Hex number
        Random::uniformU4(0, 0x0fff) |
            0x4000, // Generates a 32-bit Hex number of the form 4xxx (4 indicates the UUID version)
        Random::uniformU4(0, 0x3fff) | 0x8000,            // Generates a 32-bit Hex number in the range [0x8000, 0xbfff]
        (unsigned long long)Random::uniformU8(), rand()); // Generates a 96-bit Hex number

    metrics.set_uuid(uuid);

    counters.counters->canonicalize();
    for (auto &cat : counters.counters->countersByCategory) {
        CounterImpl::CounterType sum = 0;
        for (auto &e : cat.second) {
            sum += e.second;
            com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
            metric->set_name(absl::StrCat(prefix, ".", cat.first, ".", e.first));
            metric->set_value(e.second);
        }

        com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", cat.first, ".total"));
        metric->set_value(sum);
    }

    for (auto &hist : counters.counters->histograms) {
        CounterImpl::CounterType sum = 0;
        int histMin = hist.second.begin()->first;
        int histMax = hist.second.begin()->first;
        for (auto &e : hist.second) {
            sum += e.second;
            histMin = min(histMin, e.first);
            histMax = max(histMin, e.first);
        }
        CounterImpl::CounterType running = 0;
        vector<pair<int, bool>> percentiles = {{25, false}, {50, false}, {75, false}, {90, false}};
        for (auto &e : hist.second) {
            running += e.second;
            for (auto &pct : percentiles) {
                if (pct.second) {
                    continue;
                }
                if (running >= sum * pct.first / 100) {
                    pct.second = true;
                    com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric =
                        metrics.add_metrics();
                    metric->set_name(absl::StrCat(prefix, ".", hist.first, ".p", pct.first));
                    metric->set_value(e.first);
                }
            }
        }
        com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", hist.first, ".min"));
        metric->set_value(histMin);

        metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", hist.first, ".max"));
        metric->set_value(histMax);

        metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", hist.first, ".total"));
        metric->set_value(sum);
    }

    for (auto &e : counters.counters->counters) {
        com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", e.first));
        metric->set_value(e.second);
    }

    return metrics;
}

com::stripe::rubytyper::File::StrictLevel strictToProto(core::StrictLevel strict) {
    switch (strict) {
        case core::StrictLevel::None:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_None;
        case core::StrictLevel::Internal:
            // we should never attempt to serialize any state that had internal errors
            Exception::raise("Should never attempt to serialize StrictLevel::Internal to a proto");
        case core::StrictLevel::Ignore:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_Ignore;
        case core::StrictLevel::False:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_False;
        case core::StrictLevel::True:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_True;
        case core::StrictLevel::Strict:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_Strict;
        case core::StrictLevel::Strong:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_Strong;
        case core::StrictLevel::Max:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_Max;
        case core::StrictLevel::Autogenerated:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_Autogenerated;
        case core::StrictLevel::Stdlib:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_Stdlib;
    }
}

com::stripe::rubytyper::File::CompiledLevel compiledToProto(core::CompiledLevel compiled) {
    switch (compiled) {
        case core::CompiledLevel::None:
            return com::stripe::rubytyper::File::CompiledLevel::File_CompiledLevel_CompiledNone;
        case core::CompiledLevel::False:
            return com::stripe::rubytyper::File::CompiledLevel::File_CompiledLevel_CompiledFalse;
        case core::CompiledLevel::True:
            return com::stripe::rubytyper::File::CompiledLevel::File_CompiledLevel_CompiledTrue;
    }
}

com::stripe::rubytyper::FileTable Proto::filesToProto(const GlobalState &gs,
                                                      const UnorderedMap<long, long> &untypedUsages, bool showFull) {
    com::stripe::rubytyper::FileTable files;
    const auto &packageDB = gs.packageDB();
    auto stripePackages = packageDB.enabled();
    for (int i = 1; i < gs.filesUsed(); ++i) {
        core::FileRef file(i);
        if (file.data(gs).isPayload()) {
            if (!showFull) {
                continue;
            } else if (gs.censorForSnapshotTests && i > 10) {
                // Only show the first 10 payload files for the sake of snapshot tests, so that
                // adding a new RBI file to the payload doesn't require updating snapshot tests.
                continue;
            }
        }
        auto *entry = files.add_files();
        auto path_view = file.data(gs).path();
        string path(path_view.begin(), path_view.end());
        entry->set_path(path);
        entry->set_sigil(strictToProto(file.data(gs).originalSigil));
        entry->set_strict(strictToProto(file.data(gs).strictLevel));
        entry->set_min_error_level(strictToProto(file.data(gs).minErrorLevel()));
        entry->set_compiled(compiledToProto(file.data(gs).compiledLevel));

        auto frefIdIt = untypedUsages.find(i);
        if (frefIdIt != untypedUsages.end()) {
            entry->set_untyped_usages(frefIdIt->second);
        }

        if (stripePackages) {
            const auto &packageInfo = packageDB.getPackageForFile(gs, file);
            if (packageInfo.exists()) {
                entry->set_pkg(packageInfo.show(gs));
            }
        }
    }
    return files;
}

string Proto::toJSON(const google::protobuf::Message &message) {
    string jsonString;
    google::protobuf::json::PrintOptions options;
    options.add_whitespace = true;
    options.preserve_proto_field_names = true;
    auto status = google::protobuf::json::MessageToJsonString(message, &jsonString, options);
    if (!status.ok()) {
        cerr << "error converting to proto json: " << status.message() << '\n';
        abort();
    }
    return jsonString;
}

const char *kTypeUrlPrefix = "type.googleapis.com";

void Proto::toJSON(const google::protobuf::Message &message, ostream &out) {
    string binaryProto;
    message.SerializeToString(&binaryProto);

    google::protobuf::io::ArrayInputStream istream(binaryProto.data(), binaryProto.size());
    google::protobuf::io::OstreamOutputStream ostream(&out);

    google::protobuf::json::PrintOptions options;
    options.add_whitespace = true;
    options.preserve_proto_field_names = true;

    const google::protobuf::DescriptorPool *pool = message.GetDescriptor()->file()->pool();
    unique_ptr<google::protobuf::util::TypeResolver> resolver(
        google::protobuf::util::NewTypeResolverForDescriptorPool(kTypeUrlPrefix, pool));

    string url = absl::StrCat(kTypeUrlPrefix, "/", message.GetDescriptor()->full_name());
    auto status = google::protobuf::json::BinaryToJsonStream(resolver.get(), url, &istream, &ostream, options);
    if (!status.ok()) {
        cerr << "error converting to proto json: " << status.message() << '\n';
        abort();
    }
}

} // namespace sorbet::core
