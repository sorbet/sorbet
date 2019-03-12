#include "yaml-cpp/yaml.h"
// has to go first as it violates our poisions

#include "absl/strings/match.h"
#include "configatron.h"
#include <cctype>
#include <sys/types.h>
#include <utility>

using namespace std;
namespace sorbet::namer {
namespace {

enum class StringKind { String, Integer, Float, Symbol };

StringKind classifyString(string_view str) {
    int dotCount = 0;
    if (!str.empty() && str[0] == ':') {
        return StringKind::Symbol;
    }
    for (const auto &c : str) {
        if (c == '.') {
            dotCount++;
        } else {
            if (c != '_' && !isdigit(c)) {
                return StringKind::String;
            }
        }
    }
    switch (dotCount) {
        case 0:
            return StringKind::Integer;
        case 1:
            return StringKind::Float;
        default:
            return StringKind::String;
    }
}

core::TypePtr getType(core::GlobalState &gs, const YAML::Node &node) {
    ENFORCE(node.IsScalar());
    string value = node.as<string>();
    if (value == "true" || value == "false") {
        return core::Types::Boolean();
    }
    switch (classifyString(value)) {
        case StringKind::Integer:
            return core::Types::Integer();
        case StringKind::Float:
            return core::Types::Float();
        case StringKind::String:
            return core::Types::String();
        case StringKind::Symbol:
            return core::Types::Symbol();
    }
}

struct Path {
    Path *parent;
    string selector;
    core::TypePtr myType;

    Path(Path *parent, string selector) : parent(parent), selector(move(selector)){};

    string toString() {
        if (parent) {
            return parent->toString() + "." + selector;
        };
        return selector;
    }

    string show(core::GlobalState &gs) {
        fmt::memory_buffer buf;
        if (myType) {
            fmt::format_to(buf, "{} -> {}", toString(), myType->toString(gs));
        }
        fmt::format_to(buf, "{}",
                       fmt::map_join(children, "", [&](const auto &child) -> string { return child->show(gs); }));
        return "";
    }

    vector<shared_ptr<Path>> children;

    shared_ptr<Path> getChild(string_view name) {
        if (!name.empty() && name[0] == ':') {
            string_view withoutColon(name.data() + 1, name.size() - 1);
            return getChild(withoutColon);
        }
        for (auto &child : children) {
            if (child->selector == name) {
                return child;
            }
        }
        return children.emplace_back(make_shared<Path>(this, string(name)));
    }

    void setType(core::GlobalState &gs, core::TypePtr tp) {
        if (myType) {
            myType = core::Types::any(core::MutableContext(gs, core::Symbols::root()), myType, tp);
        } else {
            myType = tp;
        }
    }

    void enter(core::GlobalState &gs, core::SymbolRef parent, core::SymbolRef owner) {
        if (children.empty()) {
            parent.data(gs)->resultType = myType;
        } else {
            auto classSym =
                gs.enterClassSymbol(core::Loc::none(), owner, gs.enterNameConstant("configatron" + this->toString()));
            classSym.data(gs)->setIsModule(false);
            if (this->parent == nullptr) {
                classSym.data(gs)->setSuperClass(core::Symbols::Configatron_RootStore());
            } else {
                classSym.data(gs)->setSuperClass(core::Symbols::Configatron_Store());
            }
            parent.data(gs)->resultType = core::make_type<core::ClassType>(classSym);
            // DO NOT ADD METHODS HERE. add them to Configatron::Store shim

            for (auto &child : children) {
                auto method = gs.enterMethodSymbol(core::Loc::none(), classSym, gs.enterNameUTF8(child->selector));
                child->enter(gs, method, owner);

                core::SymbolRef blkArg = gs.enterMethodArgumentSymbol(core::Loc::none(), method, core::Names::blkArg());
                blkArg.data(gs)->setBlockArgument();
                method.data(gs)->arguments().emplace_back(blkArg);
            }
            //            cout << classSym.toStringWithTabs(gs, 1, 1);
        }
    }
};

void recurse(core::GlobalState &gs, const YAML::Node &node, shared_ptr<Path> prefix) {
    switch (node.Type()) {
        case YAML::NodeType::Null:
            prefix->setType(gs, core::Types::nilClass());
            break;
        case YAML::NodeType::Scalar:
            prefix->setType(gs, getType(gs, node));
            break;
        case YAML::NodeType::Sequence: {
            core::TypePtr elemType;
            for (const auto &child : node) {
                auto thisElemType = child.IsScalar() ? getType(gs, child) : core::Types::untypedUntracked();
                if (elemType) {
                    elemType =
                        core::Types::any(core::MutableContext(gs, core::Symbols::root()), elemType, thisElemType);
                } else {
                    elemType = thisElemType;
                }
            }
            if (!elemType) {
                elemType = core::Types::bottom();
            }
            vector<core::TypePtr> elems{elemType};
            prefix->setType(gs, core::make_type<core::AppliedType>(core::Symbols::Array(), elems));
            break;
        }
        case YAML::NodeType::Map:
            for (const auto &child : node) {
                auto key = child.first.as<string>();
                if (key != "<<") {
                    recurse(gs, child.second, prefix->getChild(key));
                } else {
                    recurse(gs, child.second, prefix);
                }
            }

            break;
        case YAML::NodeType::Undefined:
            break;
    }
}

void handleFile(core::GlobalState &gs, const string &file, shared_ptr<Path> rootNode) {
    YAML::Node config = YAML::LoadFile(file);
    switch (config.Type()) {
        case YAML::NodeType::Map:
            for (const auto &child : config) {
                auto key = child.first.as<string>();
                recurse(gs, child.second, rootNode);
            }
            break;
        default:
            break;
    }
}
} // namespace

void configatron::fillInFromFileSystem(core::GlobalState &gs, const vector<string> &folders,
                                       const vector<string> &files) {
    auto rootNode = make_shared<Path>(nullptr, "");
    for (auto &folder : folders) {
        auto files = FileOps::listFilesInDir(folder, {".yaml"}, true);
        const int prefixLen = folder.length() + 1;
        for (const auto &file : files) {
            constexpr int extLen = 5; // strlen(".yaml");
            string_view fileName(file.c_str(), file.size() - extLen);
            // Trim off folder + '/'
            fileName = fileName.substr(prefixLen);
            auto innerNode = rootNode->getChild(fileName);
            handleFile(gs, file, innerNode);
        }
    }
    for (auto &file : files) {
        handleFile(gs, file, rootNode);
    }

    core::SymbolRef configatron =
        gs.enterMethodSymbol(core::Loc::none(), core::Symbols::Kernel(), gs.enterNameUTF8("configatron"));
    rootNode->enter(gs, configatron, core::Symbols::root());

    core::SymbolRef blkArg = gs.enterMethodArgumentSymbol(core::Loc::none(), configatron, core::Names::blkArg());
    blkArg.data(gs)->setBlockArgument();
    configatron.data(gs)->arguments().emplace_back(blkArg);
}
} // namespace sorbet::namer
