## 5. Should protobuf compiler flags use module.patch instead of per_file_copt?

These serve different purposes:

- `per_file_copt` in the consuming BUILD file applies compiler flags to specific source files during compilation
- `module.patch` adds MODULE.bazel to an upstream archive that lacks one

For protobuf, we're using the BCR version (`bazel_dep(name = "protobuf", version = "27.0")`), which already includes proper MODULE.bazel. The `per_file_copt` is for silencing specific compiler warnings in our build, not for Bzlmod compatibility.
