# Bison toolchain

Sorbet builds Bison from the GNU release archive, including its bundled gnulib,
with `rules_foreign_cc`. This lets us update Bison independently of the
`rules_bison` dependency that provides the `bison()` rules in `parser/BUILD`.

To build and inspect the tool directly:

```sh
./bazel build --config=dbg //third_party/bison:bison
bazel-bin/third_party/bison/bison --version
```
