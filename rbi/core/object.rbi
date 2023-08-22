# typed: __STDLIB_INTERNAL

# [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) is the default
# root of all Ruby objects.
# [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) inherits from
# [`BasicObject`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html) which
# allows creating alternate object hierarchies. Methods on
# [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) are available to
# all classes unless explicitly overridden.
#
# [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) mixes in the
# [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html) module, making the
# built-in kernel functions globally accessible. Although the instance methods
# of [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) are defined by
# the [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html) module, we
# have chosen to document them here for clarity.
#
# When referencing constants in classes inheriting from
# [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) you do not need to
# use the full namespace. For example, referencing `File` inside `YourClass`
# will find the top-level
# [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) class.
#
# In the descriptions of Object's methods, the parameter *symbol* refers to a
# symbol, which is either a quoted string or a
# [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html) (such as `:name`).
class Object < BasicObject
  include Kernel

end
