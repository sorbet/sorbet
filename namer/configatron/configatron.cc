#include "configatron.h"
#include "absl/algorithm/container.h"
#include "yaml-cpp/yaml.h"
#include <cctype>
#include <dirent.h>
#include <sys/types.h>
#include <utility>

using namespace std;
namespace sorbet::namer {
namespace {
bool endsWith(const string &a, const string &b) {
    if (b.size() > a.size()) {
        return false;
    }
    return equal(a.begin() + a.size() - b.size(), a.end(), b.begin());
}

vector<string> listDir(const char *name) {
    vector<string> result;
    DIR *dir;
    struct dirent *entry;
    vector<string> names;

    if ((dir = opendir(name)) == nullptr) {
        return result;
    }

    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            auto nested = listDir(path);
            result.insert(result.end(), nested.begin(), nested.end());
        } else if (endsWith(entry->d_name, ".yaml")) {
            names.emplace_back(entry->d_name);
        }
    }
    absl::c_sort(names);

    closedir(dir);
    return names;
}

enum class StringKind { String, Integer, Float, Symbol };

StringKind classifyString(const string &str) {
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

shared_ptr<core::Type> getType(core::GlobalState &gs, const YAML::Node &node) {
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
    shared_ptr<core::Type> myType;

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
        fmt::format_to(buf, "{}", fmt::map_join(children.begin(), children.end(), "", [&](const auto &child) -> string {
                           return child->show(gs);
                       }));
        return "";
    }

    vector<shared_ptr<Path>> children;

    shared_ptr<Path> getChild(const string &name) {
        if (!name.empty() && name[0] == ':') {
            string withoutColon(name.c_str() + 1, name.size() - 1);
            return getChild(withoutColon);
        }
        for (auto &child : children) {
            if (child->selector == name) {
                return child;
            }
        }
        return children.emplace_back(make_shared<Path>(this, name));
    }

    void setType(core::GlobalState &gs, shared_ptr<core::Type> tp) {
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
                classSym.data(gs)->superClass = core::Symbols::Configatron_RootStore();
            } else {
                classSym.data(gs)->superClass = core::Symbols::Configatron_Store();
            }
            parent.data(gs)->resultType = make_shared<core::ClassType>(classSym);
            // DO NOT ADD METHODS HERE. add them to Configatron::Store shim

            for (auto &child : children) {
                auto method = gs.enterMethodSymbol(core::Loc::none(), classSym, gs.enterNameUTF8(child->selector));
                child->enter(gs, method, owner);
            }
            //            cout << classSym.toString(gs, 1, 1);
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
            shared_ptr<core::Type> elemType;
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
            vector<shared_ptr<core::Type>> elems{elemType};
            prefix->setType(gs, make_shared<core::AppliedType>(core::Symbols::Array(), elems));
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

void handleFile(core::GlobalState &gs, string file, shared_ptr<Path> rootNode) {
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

void configatron::fillInFromFileSystem(core::GlobalState &gs, vector<string> folders, vector<string> files) {
    auto rootNode = make_shared<Path>(nullptr, "");
    for (auto &folder : folders) {
        auto files = listDir(folder.c_str());
        for (const auto &file : files) {
            constexpr int extLen = 5; // strlen(".yaml");
            string fileName(file.c_str(), file.size() - extLen);
            auto innerNode = rootNode->getChild(fileName);
            handleFile(gs, folder + "/" + file, innerNode);
        }
    }
    for (auto &file : files) {
        handleFile(gs, file, rootNode);
    }

    //    cout << rootNode->show(gs) << '\n';
    core::SymbolRef configatron =
        gs.enterMethodSymbol(core::Loc::none(), core::Symbols::Kernel(), gs.enterNameUTF8("configatron"));
    rootNode->enter(gs, configatron, core::Symbols::root());

    //    cout << configatron.toString(gs, 1, 1);
}
} // namespace sorbet::namer
