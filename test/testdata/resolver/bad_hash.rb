# typed: true
T.let({}, Hash[Integer, String])
        # ^^^^^^^^^^^^^^^^^^^^^ error: Use `T::Hash[...]`, not `Hash[...]` to declare a typed `Hash`
        # ^^^^^^^^^^^^^^^^^^^^^ error: `T.class_of(Integer)` doesn't match `T::Array[[U$1, V$1]]` for argument `arg0`
        # ^^^^^^^^^^^^^^^^^^^^^ error: `T.class_of(String)` doesn't match `T::Array[[U$1, V$1]]` for argument `arg0`
