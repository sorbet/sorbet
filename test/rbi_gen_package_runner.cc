#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "common/Subprocess.h"
#include "common/common.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

namespace {

using defer = shared_ptr<void>;

vector<string> fromJsonStringArray(const rapidjson::Value &elems) {
    vector<string> result;
    for (const auto &elem : elems.GetArray()) {
        result.emplace_back(elem.GetString());
    }

    return result;
}

rapidjson::Document parseJsonFile(string filename) {
    ifstream ifs(filename);
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document document;
    document.ParseStream(isw);
    if (document.HasParseError()) {
        throw new runtime_error("Failed to parse document: " + filename);
    }

    return document;
}

struct PackageInfo {
    string name;
    vector<string> imports;
    vector<string> testImports;
    vector<string> files;
    vector<string> testFiles;
    string mangledName;

    PackageInfo() = default;
    PackageInfo(string name, vector<string> imports, vector<string> testImports, vector<string> files,
                vector<string> testFiles)
        : name(name), imports(imports), testImports(testImports), files(files), testFiles(testFiles),
          mangledName(absl::StrReplaceAll(name, {{"::", "_"}}) + "_Package"s) {}

    static PackageInfo fromJson(const rapidjson::Value &d) {
        return PackageInfo(d["name"].GetString(), fromJsonStringArray(d["imports"]),
                           fromJsonStringArray(d["testImports"]), fromJsonStringArray(d["files"]),
                           fromJsonStringArray(d["testFiles"]));
    }
};

struct DependencyInfo {
    vector<string> packageRefs;
    vector<string> rbiRefs;

    DependencyInfo() = default;
    DependencyInfo(vector<string> packageRefs, vector<string> rbiRefs) : packageRefs(packageRefs), rbiRefs(rbiRefs) {}

    static DependencyInfo load(string file) {
        auto d = parseJsonFile(file);
        return DependencyInfo(fromJsonStringArray(d["packageRefs"]), fromJsonStringArray(d["rbiRefs"]));
    }
};

absl::flat_hash_map<string, PackageInfo> packageInfoToHash(vector<PackageInfo> packageInfo) {
    absl::flat_hash_map<string, PackageInfo> result;
    for (auto &info : packageInfo) {
        result[info.name] = info;
    }

    return result;
}

struct PackageDB {
    absl::flat_hash_map<string, string> rbis;
    absl::flat_hash_map<string, DependencyInfo> deps;
    absl::flat_hash_map<string, PackageInfo> info;

    PackageDB(string packageDir, vector<PackageInfo> packageInfo)
        : rbis({}), deps({}), info(packageInfoToHash(packageInfo)) {
        for (const auto &fullEntry : filesystem::directory_iterator(filesystem::path(packageDir))) {
            const auto &entry = fullEntry.path().filename();
            if (entry == "." || entry == "..") {
                continue;
            }

            auto filepath = entry.native();
            if (absl::EndsWith(filepath, ".deps.json")) {
                deps[filepath.substr(0, filepath.size() - 10)] = DependencyInfo::load(packageDir + "/" + filepath);
            } else if (absl::EndsWith(filepath, ".package.rbi")) {
                rbis[filepath.substr(0, filepath.size() - 12)] = filepath;
            } else {
                throw new runtime_error("Unknown filename in package directory " + filepath);
            }
        }
    }

    string lookupRbiFor(PackageInfo info) {
        return rbis.at(info.mangledName);
    }

    DependencyInfo lookupDepsFor(PackageInfo info) {
        return deps.at(info.mangledName);
    }

    PackageInfo lookupPackageFor(string name) {
        return info.at(name);
    }
};

string copyFromOwnedCharStar(char *str) {
    string result(str);
    free(str);
    return result;
}

string mktempTemplate(string basename) {
    string packageInfoFilename = basename + ".XXXXXXXXXX";
    if (const char *envTmpdir = getenv("TMPDIR")) {
        packageInfoFilename = string(envTmpdir) + "/" + packageInfoFilename;
    } else {
        packageInfoFilename = "/tmp/" + packageInfoFilename;
    }

    return packageInfoFilename;
}

void chDirAndSpawn(string executable, vector<string> arguments, optional<string_view> stdinContents,
                   optional<string> chdir) {
    auto pwd = filesystem::current_path();
    defer _(nullptr, [&](...) { filesystem::current_path(pwd); });
    if (chdir.has_value()) {
        filesystem::path chdirPath = chdir.value();
        filesystem::current_path(chdirPath);
    }
    auto result = sorbet::Subprocess::spawn(move(executable), move(arguments), nullopt);

    if (!result.has_value() || result->status != 0) {
        throw runtime_error("Subprocess failed");
    }
}

vector<PackageInfo> loadPackageInfo(string packageInfoFilename) {
    auto document = parseJsonFile(packageInfoFilename);

    vector<PackageInfo> result;
    for (const auto &package : document.GetArray()) {
        result.emplace_back(PackageInfo::fromJson(package));
    }

    return result;
}

void verifySinglePackageTypechecking(string sorbet, string root, string testDirectory,
                                     absl::flat_hash_set<string> requiredFiles, string rbiPackageDir,
                                     absl::flat_hash_set<string> rbiFiles) {
    string tmpdirFilename = mktempTemplate("tmp");
    auto tmpdir = mkdtemp(tmpdirFilename.data());
    filesystem::path tmpdirPath = tmpdir;

    absl::flat_hash_set<string> dirnames;
    for (const auto &f : requiredFiles) {
        dirnames.emplace(filesystem::path(f).parent_path());
    }
    for (const auto &dirname : dirnames) {
        filesystem::path target = tmpdirPath / filesystem::path(dirname);
        filesystem::create_directories(target);
    }

    auto pwd = filesystem::current_path();
    filesystem::current_path(filesystem::path(root));
    for (const auto &f : requiredFiles) {
        auto fPath = filesystem::path(f);
        auto dirname = fPath.parent_path();
        // For some reason, I couldn't get the filesystem::copy_file call to work, and I think it
        // has something to do with bazel sandboxing. I didn't look into it further.
        chDirAndSpawn("cp",
                      {
                          fPath.native(),
                          (tmpdirPath / dirname).native(),
                      },
                      nullopt, nullopt);
    }
    filesystem::current_path(pwd);

    for (const auto &rbi : rbiFiles) {
        auto source = filesystem::path(rbiPackageDir) / filesystem::path(rbi);
        chDirAndSpawn("cp",
                      {
                          source.native(),
                          (filesystem::path(tmpdirPath) / filesystem::path(testDirectory)).native(),
                      },
                      nullopt, nullopt);
    }

    chDirAndSpawn(sorbet,
                  {
                      "--silence-dev-message",
                      "--stripe-mode",
                      "--ignore",
                      "__package.rb",
                      testDirectory,
                  },
                  nullopt, tmpdir);
}

} // namespace

int main(int argc, char **argv) {
    if (argc != 4) {
        return 1;
    }

    auto sorbet = copyFromOwnedCharStar(realpath(argv[1], nullptr));
    auto root = copyFromOwnedCharStar(realpath(argv[2], nullptr));
    auto testDirectory = string(argv[3]);

    vector<PackageInfo> packageInfo;
    vector<string> packageDirectories;
    {
        string packageInfoFilename = mktempTemplate("package_info");
        auto packageInfoFd = mkstemp(packageInfoFilename.data());
        if (packageInfoFd == -1) {
            return 1;
        }
        defer _(nullptr, [&](...) { unlink(packageInfoFilename.c_str()); });
        close(packageInfoFd);

        chDirAndSpawn(sorbet,
                      {
                          "--silence-dev-message"s,
                          "--stripe-mode"s,
                          "--stripe-packages"s,
                          "--dump-package-info"s,
                          packageInfoFilename,
                          testDirectory,
                      },
                      nullopt, root);

        packageInfo = loadPackageInfo(packageInfoFilename);
        for (const auto &package : packageInfo) {
            absl::flat_hash_set<string> dirs;
            for (const auto &f : package.files) {
                dirs.emplace(filesystem::path(f).parent_path());
            }

            if (dirs.size() != 1) {
                throw runtime_error("Package must be limited to a single directory");
            }

            packageDirectories.emplace_back(*dirs.begin());
        }
    }

    {
        string rbiPackageDirFilename = mktempTemplate("tmp");
        auto rbiPackageDir = mkdtemp(rbiPackageDirFilename.data());
        chDirAndSpawn(sorbet,
                      {
                          "--silence-dev-message",
                          "--stripe-mode",
                          "--package-rbi-generation",
                          "--package-rbi-dir",
                          rbiPackageDir,
                          "--ignore",
                          "__package.rb",
                          testDirectory,
                      },
                      nullopt, root);

        PackageDB db(rbiPackageDir, packageInfo);

        for (const auto &info : packageInfo) {
            absl::flat_hash_set<string> requiredFiles;
            absl::flat_hash_set<string> rbiFiles;
            absl::flat_hash_set<string> visited;
            deque<string> worklist;

            for (const auto &file : info.files) {
                requiredFiles.emplace(file);
            }
            for (const auto &file : info.testFiles) {
                requiredFiles.emplace(file);
            }
            visited.emplace(info.name);
            for (const auto &import_ : info.imports) {
                worklist.emplace_back(import_);
            }
            for (const auto &import_ : info.testImports) {
                worklist.emplace_back(import_);
            }

            while (!worklist.empty()) {
                auto pkg = worklist.front();
                worklist.pop_front();
                if (visited.count(pkg) == 1) {
                    continue;
                }

                visited.emplace(pkg);
                auto infoForDependency = db.lookupPackageFor(pkg);
                rbiFiles.emplace(db.lookupRbiFor(infoForDependency));
                for (const auto &import_ : infoForDependency.imports) {
                    worklist.emplace_back(import_);
                }
                for (const auto &import_ : infoForDependency.testImports) {
                    worklist.emplace_back(import_);
                }
            }

            verifySinglePackageTypechecking(sorbet, root, testDirectory, requiredFiles, rbiPackageDir, rbiFiles);
        }
    }

    return 0;
}
