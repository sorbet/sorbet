# typed: true

# disable-fast-path: true
Foo::A_PUBLIC_CONST
Foo::A_PRIVATE_CONST # error: Non-private reference to private constant `Foo::A_PRIVATE_CONST` referenced
Foo::PrivateClass # error: Non-private reference to private constant `Foo::PrivateClass` referenced
Foo::PrivateModule # error: Non-private reference to private constant `Foo::PrivateModule` referenced
Foo::PrivateModule::ClassInsidePrivateModule.also_ok_private_usage # error: Non-private reference to private constant `Foo::PrivateModule` referenced

Foo.using_private_ints(1, 2)
Foo.using_private_class(1)
#                       ^ error: Expected `Foo::PrivateClass` but found `Integer(1)` for argument `x`
Foo.using_private_module(1)
#                        ^  error: Expected `T.class_of(Foo::PrivateModule)` but found `Integer(1)` for argument `x`
