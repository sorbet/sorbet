#include "common/FileSystem.h"
#include "common/FileOps.h"
#include "common/concurrency/WorkerPool.h"

namespace sorbet {
using namespace std;

string OSFileSystem::readFile(const string &path) const {
    return FileOps::read(path);
}

void OSFileSystem::writeFile(const string &filename, string_view text) {
    return FileOps::write(filename, text);
}

vector<string> FileSystem::listFilesInDir(string_view path, const UnorderedSet<string> &extensions, bool recursive,
                                          const vector<string> &absoluteIgnorePatterns,
                                          const vector<string> &relativeIgnorePatterns) const {
    unique_ptr<WorkerPool> workerPool = WorkerPool::create(0, *spdlog::default_logger());

    return this->listFilesInDir(path, extensions, *workerPool, recursive, absoluteIgnorePatterns,
                                relativeIgnorePatterns);
}

vector<string> OSFileSystem::listFilesInDir(string_view path, const UnorderedSet<string> &extensions,
                                            WorkerPool &workerPool, bool recursive,
                                            const vector<string> &absoluteIgnorePatterns,
                                            const vector<string> &relativeIgnorePatterns) const {
    return FileOps::listFilesInDir(path, extensions, workerPool, recursive, absoluteIgnorePatterns,
                                   relativeIgnorePatterns);
}

} // namespace sorbet
