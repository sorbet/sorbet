#include "dsl/custom/CustomReplace.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"
#include "core/errors/internal.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/schema.h"

#include <sstream>
#include <string>
#include <variant>

using namespace std;
using namespace sorbet::ast;

namespace sorbet::dsl::custom {

#define GRAB_MEMBER(document, memberName)                         \
    auto __##memberName##__ = (document).FindMember(#memberName); \
    if (__##memberName##__ == (document).MemberEnd()) {           \
        spdlog::error("member \"" #memberName "\" is missing");   \
        return 0;                                                 \
    }                                                             \
    auto &memberName = __##memberName##__->value;

static unique_ptr<Matcher> buildMatcher(rapidjson::Value &def);

static unique_ptr<NameMatcher> buildNameMatcher(rapidjson::Value &def) {
    if (def.IsString()) {
        return make_unique<NameMatcher>(std::string(def.GetString(), def.GetStringLength()));
    } else {
        GRAB_MEMBER(def, type);
        GRAB_MEMBER(def, expected);
        GRAB_MEMBER(def, capture);

        if (strcmp(type.GetString(), "match_name")) {
            return nullptr;
        }

        auto matcher = make_unique<NameMatcher>(std::string(expected.GetString(), expected.GetStringLength()));
        matcher->captureIndex = capture.GetInt();
        return matcher;
    }
}

static unique_ptr<GroupMatcher> buildGroupMatcher(rapidjson::Value &def) {
    GroupMatcher::MatchGroup_store expectedSequence;
    for (auto &value : def.GetArray()) {
        auto matcher = buildMatcher(value);
        if (!matcher) {
            return nullptr;
        }
        expectedSequence.emplace_back(std::move(matcher));
    }

    return make_unique<GroupMatcher>(std::move(expectedSequence));
}

static unique_ptr<Matcher> buildMatcher(rapidjson::Value &def) {
    GRAB_MEMBER(def, type);

    const char *typeString = type.GetString();
    if (!strcmp(typeString, "match_send")) {
        GRAB_MEMBER(def, callee);
        GRAB_MEMBER(def, recv);
        GRAB_MEMBER(def, args);
        GRAB_MEMBER(def, block);
        auto calleeMatcher = buildNameMatcher(callee);
        auto recvMatcher = buildMatcher(recv);
        auto groupMatcher = buildGroupMatcher(args);
        auto blockMatcher = buildMatcher(block);
        if (!(calleeMatcher && recvMatcher && groupMatcher && blockMatcher)) {
            return nullptr;
        }
        return make_unique<SendMatcher>(std::move(*calleeMatcher), std::move(recvMatcher), std::move(groupMatcher),
                                        std::move(blockMatcher));
    } else if (!strcmp(typeString, "match_wildcard")) {
        return make_unique<WildCardMatcher>();
    } else if (!strcmp(typeString, "match_self")) {
        return make_unique<SelfMatcher>();
    } else if (!strcmp(typeString, "match_literal")) {
        GRAB_MEMBER(def, variant);

        const char *variantString = variant.GetString();
        if (!strcmp(variantString, "symbol")) {
            bool wildcardMatch = false;
            int captureIndex = 0;
            string expectedContent;

            if (auto wildcard = def.FindMember("wildcard"); wildcard != def.MemberEnd()) {
                if (!wildcard->value.IsTrue()) {
                    return nullptr;
                }
                wildcardMatch = true;
            } else {
                GRAB_MEMBER(def, expected);
                expectedContent = string(expected.GetString(), expected.GetStringLength());
            }

            // possible capture
            if (auto capture = def.FindMember("capture"); capture != def.MemberEnd()) {
                if (capture->value.GetInt() < 1) {
                    return nullptr;
                }
                captureIndex = capture->value.GetInt();
            }

            auto matcher = make_unique<LiteralMatcher>(Capture::LiteralType::Symbol, std::move(expectedContent));
            matcher->captureIndex = captureIndex;
            matcher->wildcardContent = wildcardMatch;
            return matcher;
        } else if (!strcmp(variantString, "constant")) {
            bool wildcardMatch = false;
            int captureIndex = 0;
            ConstantMatcher::Components_store components;

            if (auto wildcard = def.FindMember("wildcard"); wildcard != def.MemberEnd()) {
                if (!wildcard->value.IsTrue()) {
                    return nullptr;
                }
                wildcardMatch = true;
            } else {
                GRAB_MEMBER(def, right_to_left);

                for (auto &value : right_to_left.GetArray()) {
                    components.emplace_back(value.GetString(), value.GetStringLength());
                }
            }
            // possible capture
            if (auto capture = def.FindMember("capture"); capture != def.MemberEnd()) {
                if (capture->value.GetInt() < 1) {
                    return nullptr;
                }
                captureIndex = capture->value.GetInt();
            }

            auto matcher = make_unique<ConstantMatcher>(std::move(components));
            matcher->captureIndex = captureIndex;
            matcher->wildcardComponents = wildcardMatch;

            return matcher;
        }
    }

    return nullptr;
}

class CaptureTransformState {
public:
    vector<string> stack;
    vector<Capture> &captures;
    UnorderedMap<string, std::vector<string>> &userMap;
    std::unique_ptr<Expression> result;

    CaptureTransformState(vector<Capture> &captures, UnorderedMap<string, vector<string>> &userMap)
        : stack(), captures(captures), userMap(userMap), result(nullptr) {}
};

enum class OptCode {
    PutCapture,
    PutString,
    MapCapture,
    StringConcat,
    ToCamalCase,
    ReturnAsSymbol,
    ReturnAsConstant,
    DoNothing
};

class TransformInstruction {
public:
    TransformInstruction(const TransformInstruction &) = delete;
    TransformInstruction(TransformInstruction &&) = default;

    static TransformInstruction makePutCapture(int captureIndex) {
        return {OptCode::PutCapture, captureIndex};
    }
    static TransformInstruction makePutString(std::string &&str) {
        return {OptCode::PutString, std::move(str)};
    }
    static TransformInstruction makeMapCapture(int captureIndex) {
        return {OptCode::MapCapture, captureIndex};
    }
    static TransformInstruction makeStringConcat() {
        return {OptCode::StringConcat, 0};
    }
    static TransformInstruction makeToCamalCase() {
        return {OptCode::ToCamalCase, 0};
    }
    static TransformInstruction makeReturnAsSymbol() {
        return {OptCode::ReturnAsSymbol, 0};
    }
    static TransformInstruction makeReturnAsConstant() {
        return {OptCode::ReturnAsConstant, 0};
    }

    // return whether the execution ran without error
    bool execute(CaptureTransformState &trans, core::GlobalState &gs, core::Loc loc) {
        if (opt == OptCode::PutCapture) {
            auto idx = std::get<int>(data);
            if (!(idx >= 0 && idx < trans.captures.size())) {
                return false;
            }
            Capture &capture = trans.captures[idx];

            if (capture.variant == Capture::LiteralType::String || capture.variant == Capture::LiteralType::Symbol) {
                trans.stack.emplace_back(capture.name.data(gs)->shortName(gs));
                return true;
            } else {
                return false;
            }
        } else if (opt == OptCode::PutString) {
            auto str = std::get<string>(data);
            trans.stack.emplace_back(str);
            return true;
        } else if (opt == OptCode::MapCapture) {
            int idx = std::get<int>(data);
            if (!(idx >= 0 && idx < trans.captures.size())) {
                return false;
            }
            Capture &capture = trans.captures[idx];
            if (auto it = trans.userMap.find(capture.name.data(gs)->shortName(gs)); it != trans.userMap.end()) {
                trans.stack.insert(trans.stack.end(), it->second.begin(), it->second.end());
                return true;
            } else {
                return false;
            }
        } else if (opt == OptCode::StringConcat) {
            if (trans.stack.size() < 2) {
                return false;
            }
            string right(std::move(trans.stack.back()));
            trans.stack.pop_back();
            string left(std::move(trans.stack.back()));
            trans.stack.pop_back();
            trans.stack.emplace_back(std::move(left) + std::move(right));
            return true;
        } else if (opt == OptCode::ToCamalCase) {
            if (trans.stack.size() < 1) {
                return false;
            }
            stringstream buf;
            bool capitalize = false;
            for (char c : trans.stack.back()) {
                if (c == '_') {
                    capitalize = true;
                } else if (capitalize) {
                    buf << static_cast<char>(toupper(c));
                    capitalize = false;
                } else {
                    buf << c;
                }
            }
            trans.stack.pop_back();
            trans.stack.emplace_back(buf.str());
            return true;
        } else if (opt == OptCode::ReturnAsSymbol) {
            if (trans.stack.size() < 1) {
                // TODO: show a error instead of failing silently
                return false;
            }
            auto ref = gs.enterNameUTF8(trans.stack.back());
            trans.result = MK::Symbol(loc, ref);
            return true;
        } else if (opt == OptCode::ReturnAsConstant) {
            if (trans.stack.empty()) {
                // TODO: show an error instead of failing silently
                return false;
            }

            // top of the stack is the right most component
            unique_ptr<Expression> constant;
            if (trans.stack.front() == "::") {
                constant = ast::MK::Constant(loc, core::Symbols::root());
            } else {
                auto ref = gs.enterNameConstant(trans.stack.front());
                auto empty = ast::MK::EmptyTree();
                constant = ast::MK::UnresolvedConstant(loc, std::move(empty), ref);
            }

            for (auto it = trans.stack.begin() + 1; it != trans.stack.end(); it++) {
                auto ref = gs.enterNameConstant(*it);
                constant = ast::MK::UnresolvedConstant(loc, std::move(constant), ref);
            }

            trans.result = std::move(constant);
            return true;
        }
        return false;
    }
    OptCode getOpt() {
        return opt;
    }

private:
    OptCode opt;
    variant<int, std::string> data;

    TransformInstruction(OptCode opt, variant<int, std::string> data) : opt(opt), data(data){};
};

class TreeTemplate {
public:
    static unique_ptr<TreeTemplate> build(rapidjson::Document &&rootDocument) {
        auto output = rootDocument.FindMember("output");
        if (output == rootDocument.MemberEnd()) {
            return nullptr;
        }
        auto mappings = rootDocument.FindMember("mappings");
        if (mappings == rootDocument.MemberEnd()) {
            return nullptr;
        }

        auto treeTemplate = make_unique<TreeTemplate>();
        treeTemplate->rootDocument = std::move(rootDocument);
        treeTemplate->parseDefinitionArray(output->value);
        treeTemplate->outputDefinition = std::move(output->value);

        if (!treeTemplate->parseUserMapping(mappings->value)) {
            return nullptr;
        }

        return treeTemplate;
    }

    vector<unique_ptr<ast::Expression>> fill(vector<Capture> captures, core::GlobalState &gs,
                                             ast::Expression *matchee) {
        return makeTreeArray(outputDefinition, gs, captures, matchee->loc);
    }

private:
    UnorderedMap<string, vector<string>> userMap;
    std::vector<std::vector<TransformInstruction>> transformations;
    rapidjson::Value outputDefinition;
    rapidjson::Document rootDocument;
    using makeTreeFn = unique_ptr<Expression> (TreeTemplate::*)(rapidjson::Value &def, core::GlobalState &ctx,
                                                                vector<Capture> &captures, core::Loc &loc);
    static UnorderedMap<string, makeTreeFn> makeTreeDispatch;

    void prepareTransformation(rapidjson::Value &def) {
        vector<TransformInstruction> sequence;
        for (auto &instDef : def["instructions"].GetArray()) {
            auto asArray = instDef.GetArray();
            auto &opt = asArray[0];
            const char *optString = opt.GetString();

            if (!strcmp(optString, "put_capture")) {
                auto &capture_index = asArray[1];
                sequence.emplace_back(TransformInstruction::makePutCapture(capture_index.GetInt() - 1));
            } else if (!strcmp(optString, "map_capture")) {
                auto &capture_index = asArray[1];
                sequence.emplace_back(TransformInstruction::makeMapCapture(capture_index.GetInt() - 1));
            } else if (!strcmp(optString, "put_string")) {
                auto &literal = asArray[1];
                sequence.emplace_back(
                    TransformInstruction::makePutString(string(literal.GetString(), literal.GetStringLength())));
            } else if (!strcmp(optString, "string_concat")) {
                sequence.emplace_back(TransformInstruction::makeStringConcat());
            } else if (!strcmp(optString, "to_camal_case")) {
                sequence.emplace_back(TransformInstruction::makeToCamalCase());
            } else if (!strcmp(optString, "return_as_symbol")) {
                sequence.emplace_back(TransformInstruction::makeReturnAsSymbol());
            } else if (!strcmp(optString, "return_as_constant")) {
                sequence.emplace_back(TransformInstruction::makeReturnAsConstant());
            } else {
                ENFORCE(false, "invalid opt code: ", optString);
            }
        }

        transformations.emplace_back(std::move(sequence));
        def.AddMember("_transformationId", static_cast<int>(transformations.size() - 1), rootDocument.GetAllocator());
    }

    void parseDefinitionArray(rapidjson::Value &def) {
        for (auto &member : def.GetArray()) {
            parseDefinition(member);
        }
    }

    void parseDefinition(rapidjson::Value &def) {
        if (def.IsObject()) {
            if (auto it = def.FindMember("type"); it != def.MemberEnd()) {
                if (!strcmp(it->value.GetString(), "transform")) {
                    prepareTransformation(def);
                }
            }
            for (auto &member : def.GetObject()) {
                parseDefinition(member.value);
            }
        } else if (def.IsArray()) {
            parseDefinitionArray(def);
        }
    }

    bool parseUserMapping(rapidjson::Value &def) {
        for (auto &mapping : def.GetArray()) {
            const auto &key = mapping[0];
            const auto &value = mapping[1];
            const char *variantString = key["variant"].GetString();
            string userMapKey;
            if (!strcmp(variantString, "constant")) {
                const auto &components = key["components"].GetArray();
                stringstream asString;
                bool first = true;
                for (int i = components.Size() - 1; i >= 0; i--) {
                    if (!first) {
                        asString << '|';
                    }
                    first = false;
                    asString << components[i].GetString();
                }
                userMapKey = asString.str();
            }
            vector<string> values;
            for (auto &entry : value.GetArray()) {
                values.emplace_back(entry.GetString());
            }

            userMap[userMapKey] = std::move(values);
        }
        return true;
    }

    vector<std::unique_ptr<Expression>> makeTreeArray(rapidjson::Value &def, core::GlobalState &ctx,
                                                      vector<Capture> &captures, core::Loc loc) {
        vector<std::unique_ptr<Expression>> result;
        for (auto &subDef : def.GetArray()) {
            result.emplace_back(makeTree(subDef, ctx, captures, loc));
        }
        return result;
    }

    core::NameRef enterName(rapidjson::Value &jsonString, core::GlobalState &gs) {
        return gs.enterNameUTF8(std::string(jsonString.GetString(), jsonString.GetStringLength()));
    }

    unique_ptr<Expression> makeTreeTransform(rapidjson::Value &def, core::GlobalState &ctx, vector<Capture> &captures,
                                             core::Loc &loc) {
        int id = def["_transformationId"].GetInt();
        auto &transform = transformations.at(id);

        CaptureTransformState state(captures, userMap);
        for (auto &inst : transform) {
            bool ok = inst.execute(state, ctx, loc);
            if (!ok) {
                return nullptr;
            }
        }
        return std::move(state.result);
    }

    unique_ptr<Expression> makeTreeSelf(rapidjson::Value &def, core::GlobalState &ctx, vector<Capture> &captures,
                                        core::Loc &loc) {
        return ast::MK::Self(loc);
    }

    unique_ptr<Expression> makeTreeMethodDef(rapidjson::Value &def, core::GlobalState &ctx, vector<Capture> &captures,
                                             core::Loc &loc) {
        auto &name = def["name"];
        auto &args = def["args"];
        auto argsArray = makeTreeArray(args, ctx, captures, loc);

        core::NameRef nameRef;
        if (name.IsString()) {
            nameRef = enterName(name, ctx);
        } else {
            auto symbol = makeTree(name, ctx, captures, loc);
            auto symbolTree = cast_tree<Literal>(symbol.get());
            if (symbolTree->isSymbol(ctx)) {
                nameRef = symbolTree->asSymbol(ctx);
            } else {
                return nullptr;
            }
        }
        MethodDef::ARGS_store argsStore(make_move_iterator(argsArray.begin()), make_move_iterator(argsArray.end()));
        return ast::MK::Method(loc, loc, nameRef, std::move(argsStore), ast::MK::EmptyTree(),
                               ast::MethodDef::DSLSynthesized);
    }

    unique_ptr<Expression> makeTreeBlock(rapidjson::Value &def, core::GlobalState &ctx, vector<Capture> &captures,
                                         core::Loc &loc) {
        auto &args = def["args"];
        auto &body = def["body"];
        auto argsArray = makeTreeArray(args, ctx, captures, loc);
        MethodDef::ARGS_store argsStore(make_move_iterator(argsArray.begin()), make_move_iterator(argsArray.end()));

        auto bodyArray = makeTreeArray(body, ctx, captures, loc);
        if (bodyArray.empty()) {
            return nullptr;
        }
        InsSeq::STATS_store stats(make_move_iterator(bodyArray.begin()), make_move_iterator(bodyArray.end() - 1));
        return ast::MK::Block(loc, ast::MK::InsSeq(loc, std::move(stats), std::move(bodyArray.back())),
                              std::move(argsStore));
    }

    unique_ptr<Expression> makeTreeLocal(rapidjson::Value &def, core::GlobalState &ctx, vector<Capture> &captures,
                                         core::Loc &loc) {
        auto &name = def["name"];
        auto nameRef = enterName(name, ctx);
        return ast::MK::Local(core::Loc::none(), nameRef);
    }

    unique_ptr<Expression> makeTreeHash(rapidjson::Value &def, core::GlobalState &ctx, vector<Capture> &captures,
                                        core::Loc &loc) {
        auto &pairs = def["pairs"];
        Hash::ENTRY_store keys;
        Hash::ENTRY_store values;
        for (auto &entry : pairs.GetArray()) {
            const auto &arr = entry.GetArray();
            auto &keyDef = arr[0];
            auto &valueDef = arr[1];
            keys.emplace_back(makeTree(keyDef, ctx, captures, loc));
            values.emplace_back(makeTree(valueDef, ctx, captures, loc));
        }
        return ast::MK::Hash(loc, std::move(keys), std::move(values));
    }

    unique_ptr<Expression> makeTreeLiteral(rapidjson::Value &def, core::GlobalState &ctx, vector<Capture> &captures,
                                           core::Loc &loc) {
        auto &variant = def["variant"];
        auto variantString = variant.GetString();
        if (!strcmp(variantString, "symbol")) {
            auto name = enterName(def["value"], ctx);
            return ast::MK::Symbol(loc, name);
        } else if (!strcmp(variantString, "constant")) {
            // todo
        }
        return nullptr;
    }

    unique_ptr<Expression> makeTreeSend(rapidjson::Value &def, core::GlobalState &ctx, vector<Capture> &captures,
                                        core::Loc &loc) {
        auto &callee = def["callee"];
        auto &block = def["block"];
        auto &recv = def["recv"];
        auto &args = def["args"];
        unique_ptr<ast::Block> blockTree(cast_tree<ast::Block>(makeTree(block, ctx, captures, loc).release()));
        auto recvTree = makeTree(recv, ctx, captures, loc);
        auto argsArray = makeTreeArray(args, ctx, captures, loc);
        auto nameRef = enterName(callee, ctx);
        Send::ARGS_store argsStore(make_move_iterator(argsArray.begin()), make_move_iterator(argsArray.end()));
        return ast::MK::Send(loc, std::move(recvTree), nameRef, std::move(argsStore), 0, std::move(blockTree));
    }

    unique_ptr<Expression> makeTree(rapidjson::Value &def, core::GlobalState &ctx, vector<Capture> &captures,
                                    core::Loc &loc) {
        if (def.IsNull()) {
            return nullptr;
        }
        const char *typeString = def["type"].GetString();
        auto dispatch = makeTreeDispatch.find(typeString);
        ENFORCE(dispatch != makeTreeDispatch.end());
        return invoke(dispatch->second, this, def, ctx, captures, loc);
    }
};

UnorderedMap<string, TreeTemplate::makeTreeFn> TreeTemplate::makeTreeDispatch = {
    {"literal", &TreeTemplate::makeTreeLiteral},
    {"local", &TreeTemplate::makeTreeLocal},
    {"self", &TreeTemplate::makeTreeSelf},
    {"hash", &TreeTemplate::makeTreeHash},
    {"block", &TreeTemplate::makeTreeBlock},
    {"send", &TreeTemplate::makeTreeSend},
    {"method_def", &TreeTemplate::makeTreeMethodDef},
    {"transform", &TreeTemplate::makeTreeTransform},
};

CustomReplace::CustomReplace(CustomReplace &&) = default;
CustomReplace::~CustomReplace() = default;

optional<CustomReplace> CustomReplace::parseDefinition(core::GlobalState &gs, std::string_view specDefinition) {
    using namespace rapidjson;
    Document schemaJson;
    schemaJson.Parse(CustomReplace::specSchema);

    ENFORCE(!schemaJson.HasParseError());

    SchemaDocument schema(schemaJson);

    Document spec;
    if (spec.Parse(specDefinition.data(), specDefinition.size()).HasParseError()) {
        // TODO, should probably use a core::FileRef here so we can get a good loc
        if (auto e = gs.beginError(sorbet::core::Loc::none(), core::errors::Internal::InternalError)) {
            e.setHeader("DSL definition is not valid JSON");
        }
        return nullopt;
    }

    SchemaValidator validator(schema);
    if (!spec.Accept(validator)) {
        StringBuffer buffer;
        PrettyWriter<StringBuffer> writer(buffer);
        validator.GetError().Accept(writer);
        if (auto e = gs.beginError(sorbet::core::Loc::none(), core::errors::Internal::InternalError)) {
            // TODO: nicer error message
            e.setHeader("DSL definition is invalid");
            e.addErrorLine(sorbet::core::Loc::none(), "Schema validation failure JSON: {}", buffer.GetString());
        }
        return nullopt;
    }

    CustomReplace replace;
    if (auto match = spec.FindMember("match"); match != spec.MemberEnd()) {
        replace.matcher = buildMatcher(match->value);
    }
    replace.treeTemplate = TreeTemplate::build(std::move(spec));

    ENFORCE(replace.matcher && replace.treeTemplate);

    return replace;
}

vector<unique_ptr<ast::Expression>> CustomReplace::matchAndReplace(core::GlobalState &gs, ast::Expression *matchee) {
    auto result = matcher->match(matchee, gs);
    if (!result.second) {
        return {};
    }

    return treeTemplate->fill(result.first, gs, matchee);
}

const string CustomReplace::specSchema = R"(
{
  "description": "A definition for matching and replacing AST nodes.",
  "required": ["match", "mappings", "output"],
  "properties": {
    "match": { "$ref": "#/definitions/match_node" },
    "mappings": {
        "type": "array",
        "items": {
            "type": "array",
            "items": [
                { "$ref": "#/definitions/literal" },
                { "type": "array", "items": { "type": "string" }, "minItems": 1 }
            ],
            "additionalItems": false
        }
    },
    "output": {
        "type": "array",
        "items": {
            "$ref": "#/definitions/output_node"
        }
    }
  },
  "definitions": {
    "match_node": {
      "oneOf": [
        {"$ref": "#/definitions/match_send"},
        {"$ref": "#/definitions/match_literal"},
        {"$ref": "#/definitions/match_self"},
        {"$ref": "#/definitions/match_wildcard"},
        {"$ref": "#/definitions/match_name"}
      ]
    },
    "match_send": {
      "type": "object",
      "required": ["type", "callee", "block", "recv", "args"],
      "properties": {
        "type": { "enum": ["match_send"] },
        "callee": { "$ref": "#/definitions/match_node" },
        "block": { "$ref": "#/definitions/match_node" },
        "recv": { "$ref": "#/definitions/match_node" },
        "args": {
            "type": "array",
            "items": {"$ref": "#/definitions/match_node"}
        }
      }
    },
    "match_literal": {
      "type": "object",
      "required": ["type", "variant"],
      "properties": {
        "type": { "enum": ["match_literal"] },
        "variant": { "type": "string", "enum": ["constant", "symbol"] },
        "wildcard": { "type": "boolean" },
        "capture": { "type": "integer", "minimum": 0 }
      }
    },
    "match_self": {
        "type": "object",
        "required": ["type"],
        "properties": { "type": { "enum": ["match_self"] } }
    },
    "match_wildcard": {
        "type": "object",
        "required": ["type"],
        "properties": { "type": { "enum": ["match_wildcard"] } }
    },
    "match_name": {
        "oneOf": [ { "$ref": "#/definitions/capturing_match_name" }, { "type": "string" } ]
    },
    "capturing_match_name": {
        "type": "object",
        "required": ["type", "capture"],
        "properties": {
            "type": { "enum": ["match_name"] },
            "capture": { "type": "integer", "minimum": 0 }
        }
    },
    "literal": {
        "oneOf": [
            {
                "type": "object",
                "required": ["type", "variant", "components"],
                "properties": {
                    "type": { "enum": ["literal"] },
                    "variant": { "enum": ["constant"] },
                    "components": {
                        "type": "array",
                        "items": { "type": "string" },
                        "minItems": 1
                    }
                }
            },
            {
                "type": "object",
                "required": ["type", "variant", "value"],
                "properties": {
                    "type": { "enum": ["literal"] },
                    "variant": { "enum": ["symbol"] },
                    "value": {
                        "type": "string",
                        "minLength": 1
                    }
                }
            }
        ]
    },
    "output_node": {
        "oneOf": [
            { "$ref": "#/definitions/output_method_def" },
            { "$ref": "#/definitions/output_send" }
        ]
    },
    "expression_node": {
        "oneOf": [
            { "$ref": "#/definitions/output_send" },
            { "$ref": "#/definitions/output_hash" },
            { "$ref": "#/definitions/output_self" },
            { "$ref": "#/definitions/output_transform" },
            { "$ref": "#/definitions/literal" }
        ]
    },
    "output_send": {
        "type": "object",
        "required": ["type"],
        "properties": {
            "type": { "enum": ["send"] },
            "callee": { "type": "string" },
            "recv": { "$ref": "#/definitions/expression_node" },
            "args": { "type": "array", "items": { "$ref": "#/definitions/expression_node" } },
            "block": {
                "oneOf": [
                    { "type": "null" },
                    { "$ref": "#/definitions/output_block" }
                ]
            }
        }
    },
    "output_block": {
        "type": "object",
        "required": ["type", "args", "body"],
        "properties": {
            "type": { "enum": ["block"] },
            "args": {
                "type": "array",
                "items": { "$ref": "#/definitions/output_local" }
            },
            "body": {
                "type": "array",
                "items": { "$ref": "#/definitions/expression_node" }
            }
        }
    },
    "output_local": {
        "type": "object",
        "required": ["type", "name"],
        "properties": {
            "type": { "enum": ["local"] },
            "name": { "type": "string" }
        }
    },
    "output_self": {
        "type": "object",
        "required": ["type"],
        "properties": { "type": { "enum": ["self"] } }
    },
    "output_hash": {
        "type": "object",
        "required": ["type", "pairs"],
        "properties": {
            "type": { "enum": ["hash"] },
            "pairs": {
                "type": "array",
                "items": {
                    "type": "array",
                    "items": [
                        {"$ref": "#/definitions/expression_node"},
                        {"$ref": "#/definitions/expression_node"}
                    ],
                    "additionalItems": false
                }
            }
        }
    },
    "output_method_def": {
        "type": "object",
        "required": ["type", "name", "args"],
        "properties": {
            "type": { "enum": ["method_def"] },
            "name": {
                "oneOf": [
                    {
                        "type": "string"
                    },
                    {
                        "$ref": "#/definitions/output_transform"
                    }
                ]
            },
            "args": { "type": "array", "items": { "$ref": "#/definitions/output_local" } }
        }
    },
    "output_transform": {
        "type": "object",
        "required": ["type", "instructions"],
        "properties": {
            "type": { "enum": ["transform"] },
            "instructions": {
                "type": "array",
                "items": {
                    "oneOf": [
                        {"$ref": "#/definitions/transform_opt_only"},
                        {"$ref": "#/definitions/transform_opt_index"},
                        {"$ref": "#/definitions/transform_opt_string"}
                    ]
                }
            }
        }
    },
    "transform_opt_only": {
        "type": "array",
        "items": [{
            "enum": ["string_concat", "to_camal_case", "return_as_symbol", "return_as_constant"]
        }],
        "minItems": 1,
        "maxItems": 1
    },
    "transform_opt_index": {
        "type": "array",
        "items": [
            { "enum": ["map_capture", "put_capture"] },
            { "type": "integer", "minimum": 0 }
        ],
        "additionalItems": false
    },
    "transform_opt_string": {
        "type": "array",
        "items": [
            { "enum": ["put_string"] },
            { "type": "string" }
        ],
        "additionalItems": false
    }
  }
}
)";

} // namespace sorbet::dsl::custom
