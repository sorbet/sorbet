# GNU M4 toolchain

Sorbet builds GNU M4 from the GNU release archive, including its bundled
gnulib, with `rules_foreign_cc`. This lets us update M4 independently of the
`rules_m4` dependency that provides the toolchain API used by `rules_bison`.
The configure step detects the host's libc and floating-point layout.

The adapter retains the rules_m4 toolchain API used by rules_bison. Bison's
foreign build depends directly on the launcher because the toolchain alias
exports both the launcher and its installation, not a single executable.

To build and inspect the tool directly:

```sh
./bazel build --config=dbg //third_party/m4:m4
bazel-bin/third_party/m4/m4 --version
```
