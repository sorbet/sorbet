# Working with LMDB

Sorbet's `--cache-dir` option is backed by an [LMDB] database. It's basically
just a key/value store, but it has a slightly annoying API because you have to
do all reads and writes through a transaction API implemented in C.

Some docs that might be useful:

- [Getting Started](http://www.lmdb.tech/doc/starting.html)
- [LMDB API](http://www.lmdb.tech/doc/group__mdb.html)
- [command line tools]

For ad hoc investigations into Sorbet's cache, you can use the Python [`lmdb`
package]. It exposes a Python API as well as a command line API.

## Note on named vs unnamed

Sorbet uses "named databases" by which we mean that the `mdb_dbi_open` call is
given a non-NULL `name` argument.

In this mode, the "unnamed database" stores key/value pairs listing the all the
named databases' names.

In the C API, you read the unnamed database by calling `mdb_dbi_open` with a
`nullptr` name argument. In the Python wrapper, you get the unnamed database by
omitting the `<dbname>=<file>` argument to `dump` (and other subcommands).

## Note on using `ls -lh`

The sizes reported using `ls -l` can be misleading.

- If you use `mdb_drop(..., 1)` to delete a database, that just removes the
  entry from the unnamed list. It doesn't actually remove the pages from the
  on-disk file, but it does mean that the space will be available.
- You can request that the database be compacted (using
  `mdb_env_copy2(..., MDB_CP_COMPACT)`). This is never done internally because
  LMDB is trying to optimize for read/write latency not disk usage.
- Even just normal Sorbet write patterns can fragment the database. I have seen
  at least one example where a database was sitting at 4.0 GiB on disk which
  dropped to 2.4 GiB after merely being compacted (not dropping anything).
- The `MDB_MAP_FULL` error is not a function of how many bytes are in the
  database, even though the size is configured in bytes. If you look in the
  code, you see that it's reported when LMDB requests a free page but getting
  one would push the size of the DB over the cap. So things like fragmentation
  and overflow page utilization can mean that there's technically parts of the
  database that are empty, but there's no room (this is similar to the failure
  mode when malloc doesn't have space available in the heap).

## Note on cdbmake

The `dump` subcommand from the `lmdb` Python package references a cdbmake
format. You can read about it here:

- [cdbmake](https://cr.yp.to/cdb/cdbmake.html)


## Assorted recipes

Some commands that have been useful to me in the past.

Be sure to cross reference with the [command line tools] docs for further usage
info.

```bash
# Install the command line tools
pip3 install lmdb

# Reads unnamed database from /tmp/sorbet-cache/data.mdb
# Writes into ./main.cdbmake
# Will list all the "flavors" in the Sorbet cache
python -mlmdb --env /tmp/sorbet-cache dump

# Reads the database (flavor) with the name "default" out of
# `./jez-cache/data.mdb`
# Writes into `./my_dump.cdbmake`
python -mlmdb --env jez-cache dump default=my_dump.cdbmake

# Drops the 'dsl-nromalfastpath' database (flavor) from the cache file at
# `./bad-jenkins-cache/data.mdb`.
# Note: this only updates the unnamed database--it doesn't compact the storage
# so the size reported by `ls -l` will not change.
python -mlmdb --env bad-jenkins-cache drop dsl-normalfastpath

# Copies and compacts what's stored in `./bad-jenkins-cache/data.mdb`.
python -mlmdb copy --compact --env bad-jenkins-cache compacted

# Opens a shell so you can use the Python API to interact with the database
python -mlmdb --env example-cache shell
# Python 3.9.12 (main, Mar 23 2022, 21:36:19)
# [GCC 5.4.0 20160609] on linux
# Type "help", "copyright", "credits" or "license" for more information.
# (InteractiveConsole)
# >>> subdb = ENV.open_db(b'default')
# >>> with ENV.begin() as txn:
# ...     txn.stat(subdb)
# ...
# {'psize': 4096, 'depth': 2, 'branch_pages': 1, 'leaf_pages': 20, 'overflow_pages': 699, 'entries': 90}

# Print a list of key<TAB>len(val) pairs to out.txt:
cd /tmp/sorbet-cache
python -mlmdb --env . shell
# Python 3.9.12 (main, Mar 23 2022, 21:36:19)
# [GCC 5.4.0 20160609] on linux
# Type "help", "copyright", "credits" or "license" for more information.
# (InteractiveConsole)
# >>> with open('/tmp/out.txt', 'wb') as f:
# ...   with ENV.begin(db=ENV.open_db(b'experimentalfastpath')) as txn:
# ...       cursor = txn.cursor()
# ...       for key, val in cursor:
# ...           f.write(key)
# ...           f.write(b'\t')
# ...           f.write(str(len(val)).encode())
# ...           f.write(b'\n')
```


[LMDB]: http://www.lmdb.tech/doc/index.html
[`lmdb` package]: https://pypi.org/project/lmdb/
[command line tools]: https://lmdb.readthedocs.io/en/release/#command-line-tools
