## 4. Must MODULE.bazel stay at the project root?

Yes, this is a Bazel requirement. Per the Bzlmod specification, the MODULE.bazel file must be at the workspace root, adjacent to the (legacy) WORKSPACE file. Bazel discovers the module by looking for this file at the root.
