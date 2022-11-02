#include "mpack/mpack.h"

#include "ast/desugar/Desugar.h"
#include "common/FileOps.h"
#include "core/Unfreeze.h"
#include "main/autogen/cache.h"
#include "main/autogen/constant_hash.h"
#include "parser/parser.h"

using namespace std;

namespace sorbet::autogen {

const size_t MAX_SKIP_AMOUNT = 100;

bool AutogenCache::canSkipAutogen(core::GlobalState &gs, const string &cachePath, const vector<string> &changedFiles) {
    // this is here as an escape valve: if a _bunch_ of files change
    // all at once, then don't let us skip at all. This should pretty
    // rarely be the case: the autogen runner should only pass us a
    // small number of files at once, and restart us if too many
    // change (since at that point we're vanishingly unlikely to
    // benefit from caching anyway)
    if (changedFiles.size() > MAX_SKIP_AMOUNT) {
        gs.tracer().info("Rerunning autogen: found `{}` changed files, more than the threshold of `{}`",
                         changedFiles.size(), MAX_SKIP_AMOUNT);
        return false;
    }

    if (!FileOps::exists(cachePath)) {
        gs.tracer().info("Rerunning autogen: unable to find constant cache at `{}`", cachePath);
        return false;
    }

    auto changedFileSet = UnorderedSet<string>(changedFiles.begin(), changedFiles.end());
    auto cacheFile = FileOps::read(cachePath);
    auto cache = AutogenCache::unpackForFiles(cacheFile, changedFileSet);

    for (auto &file : changedFiles) {
        if (cache.constantHashMap().count(file) == 0) {
            gs.tracer().info("Rerunning autogen: could not find `{}` in constant cache", file);
            return false;
        }
        if (!FileOps::exists(file)) {
            // this is... confusing, since the runner promised the
            // file existed and apparently it doesn't now, I guess it
            // means we should redo autogen to be safe
            gs.tracer().info("Rerunning autogen: file `{}` does not exist on disk", file);
            return false;
        }
        core::FileRef ref;
        {
            core::UnfreezeFileTable fileTableAccess(gs);
            ref = gs.enterFile(file, FileOps::read(file));
        }

        core::UnfreezeNameTable nameTableAccess(gs);
        auto settings = parser::Parser::Settings{false, false, false};
        auto node = parser::Parser::run(gs, ref, settings);

        core::MutableContext ctx{gs, core::Symbols::root(), ref};
        auto ast = ast::desugar::node2Tree(ctx, move(node));

        ast::ParsedFile pf{std::move(ast), ref};

        if (autogen::constantHashTree(gs, std::move(pf)).constantHash != cache.constantHashMap().at(file)) {
            gs.tracer().info("Rerunning autogen: constant hash for file `{}` does not match stored hash", file);
            return false;
        }
    }

    return true;
}

// this doesn't load the whole cache, since we never need to _consult_
// the whole cache. Instead, we let it know which paths we care about
// and load only those
AutogenCache AutogenCache::unpackForFiles(string_view file_contents, const UnorderedSet<string> &changedFiles) {
    // we'll initialize an empty one and add files as we find them
    AutogenCache cache;

    // Going forward, if anything fails, we'll just return the
    // probably-empty cache. That means it might be incomplete, but if
    // it's incomplete in a way that matters, then we'll lack constant
    // hashes for those files, so `canSkipAutogen` will return `false`
    // and we'll regenerate the whole cache.
    mpack_reader_t reader;
    mpack_reader_init_data(&reader, file_contents.data(), file_contents.size());

    mpack_tag_t tag = mpack_read_tag(&reader);
    if (mpack_reader_error(&reader) != mpack_ok || mpack_tag_type(&tag) != mpack_type_map) {
        mpack_reader_destroy(&reader);
        return cache;
    }

    for (uint32_t i = mpack_tag_map_count(&tag); i > 0; i--) {
        // each key should be a string
        mpack_tag_t key_tag = mpack_read_tag(&reader);
        if (mpack_tag_type(&key_tag) != mpack_type_str) {
            break;
        }
        // extracting the buffer data is a bit of a chore, so here we go
        uint32_t key_len = mpack_tag_str_length(&key_tag);
        auto key_buf = mpack_read_bytes_inplace(&reader, key_len);
        if (mpack_reader_error(&reader) != mpack_ok) {
            break;
        }
        string_view key{key_buf, key_len};
        mpack_done_str(&reader);

        // and the value should be an int
        mpack_tag_t val_tag = mpack_read_tag(&reader);
        if (mpack_tag_type(&val_tag) != mpack_type_uint) {
            break;
        }

        // if this is a file we care about, then add it to the cache we've found
        if (changedFiles.contains(key)) {
            cache._constantHashMap.emplace(key, mpack_tag_uint_value(&val_tag));
        }

        // final loop safety check in case of malformed data
        if (mpack_reader_error(&reader) != mpack_ok) {
            break;
        }
    }

    mpack_done_map(&reader);

    mpack_reader_destroy(&reader);

    return cache;
}

string AutogenCache::pack() const {
    char *data;
    size_t size;
    mpack_writer_t writer;
    mpack_writer_init_growable(&writer, &data, &size);
    mpack_start_map(&writer, _constantHashMap.size());
    for (auto &[path, hash] : _constantHashMap) {
        mpack_write_str(&writer, path.data(), path.size());
        mpack_write_u64(&writer, hash);
    }
    mpack_finish_map(&writer);

    mpack_writer_destroy(&writer);

    auto ret = string(data, size);
    MPACK_FREE(data);
    return ret;
}

} // namespace sorbet::autogen
