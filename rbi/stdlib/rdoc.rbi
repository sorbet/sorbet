# typed: __STDLIB_INTERNAL

# Gem::RDoc provides methods to generate
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) and ri data for
# installed gems upon gem installation.
#
# This file is automatically required by RubyGems 1.9 and newer.
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) produces documentation
# for Ruby source files by parsing the source and extracting the definition for
# classes, modules, methods, includes and requires. It associates these with
# optional documentation contained in an immediately preceding comment block
# then renders the result using an output formatter.
#
# For a simple introduction to writing or generating documentation using
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) see the
# [README](https://docs.ruby-lang.org/en/2.7.0/README_md.html).
#
# ## Roadmap
#
# If you think you found a bug in
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) see [Bugs at
# `CONTRIBUTING`](https://docs.ruby-lang.org/en/2.7.0/CONTRIBUTING_md.html#label-Bugs)
#
# If you want to use [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) to
# create documentation for your Ruby source files, see
# [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html) and
# refer to `rdoc --help` for command line usage.
#
# If you want to set the default markup format see [Supported Formats at
# `RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html#class-RDoc::Markup-label-Supported+Formats)
#
# If you want to store rdoc configuration in your gem (such as the default
# markup format) see [Saved Options at
# `RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#class-RDoc::Options-label-Saved+Options)
#
# If you want to write documentation for Ruby files see
# [`RDoc::Parser::Ruby`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/Ruby.html)
#
# If you want to write documentation for extensions written in C see
# [`RDoc::Parser::C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html)
#
# If you want to generate documentation using `rake` see
# [`RDoc::Task`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Task.html).
#
# If you want to drive [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
# programmatically, see
# [`RDoc::RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RDoc.html).
#
# If you want to use the library to format text blocks into HTML or other
# formats, look at
# [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html).
#
# If you want to make an [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
# plugin such as a generator or directive handler see
# [`RDoc::RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RDoc.html).
#
# If you want to write your own output generator see
# [`RDoc::Generator`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Generator.html).
#
# If you want an overview of how
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) works see
# [CONTRIBUTING](https://docs.ruby-lang.org/en/2.7.0/CONTRIBUTING_md.html)
#
# ## Credits
#
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) is currently being
# maintained by Eric Hodel <drbrain@segment7.net>.
#
# Dave Thomas <dave@pragmaticprogrammer.com> is the original author of
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html).
#
# *   The Ruby parser in rdoc/parse.rb is based heavily on the outstanding work
#     of Keiju ISHITSUKA of Nippon
#     [`Rational`](https://docs.ruby-lang.org/en/2.7.0/Rational.html) Inc, who
#     produced the Ruby parser for irb and the rtags package.
module RDoc
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) modifiers for
  # attributes
  ATTR_MODIFIERS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) modifiers for
  # classes
  CLASS_MODIFIERS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) modifiers for
  # constants
  CONSTANT_MODIFIERS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Name of the dotfile that contains the description of files to be processed
  # in the current directory
  DOT_DOC_FILENAME = T.let(T.unsafe(nil), String)

  # General [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) modifiers
  GENERAL_MODIFIERS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Ruby's built-in classes, modules and exceptions
  KNOWN_CLASSES = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) modifiers for
  # methods
  METHOD_MODIFIERS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) version you are
  # using
  VERSION = T.let(T.unsafe(nil), String)

  # [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) visibilities
  VISIBILITIES = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Loads the best available
  # [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) library.
  def self.load_yaml; end
end

# Represent an alias, which is an old\_name/new\_name pair associated with a
# particular context
class RDoc::Alias < ::RDoc::CodeObject
  # Creates a new [`Alias`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Alias.html)
  # with a token stream of `text` that aliases `old_name` to `new_name`, has
  # `comment` and is a `singleton` context.
  def self.new(text, old_name, new_name, comment, singleton = _); end

  # Order by
  # [`singleton`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Alias.html#attribute-i-singleton)
  # then
  # [`new_name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Alias.html#attribute-i-new_name)
  def <=>(other); end

  # HTML fragment reference for this alias
  def aref; end

  # Full old name including namespace
  def full_old_name; end

  # HTML id-friendly version of `#new_name`.
  def html_name; end

  def inspect; end

  # Aliased method's name
  def name; end

  # '::' for the alias of a singleton method/attribute, '#' for instance-level.
  def name_prefix; end

  # Aliased method's name
  def new_name; end

  # Aliasee method's name
  def old_name; end

  # Alias for:
  # [`pretty_new_name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Alias.html#method-i-pretty_new_name)
  def pretty_name; end

  # New name with prefix '::' or '#'.
  #
  # Also aliased as:
  # [`pretty_name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Alias.html#method-i-pretty_name)
  def pretty_new_name; end

  # Old name with prefix '::' or '#'.
  def pretty_old_name; end

  # Is this an alias declared in a singleton context?
  def singleton; end

  # Is this an alias declared in a singleton context?
  def singleton=(_); end

  # Source file token stream
  def text; end

  def to_s; end
end

# An anonymous class like:
#
# ```ruby
# c = Class.new do end
# ```
#
# [`AnonClass`](https://docs.ruby-lang.org/en/2.7.0/RDoc/AnonClass.html) is
# currently not used.
class RDoc::AnonClass < ::RDoc::ClassModule; end

# [`AnyMethod`](https://docs.ruby-lang.org/en/2.7.0/RDoc/AnyMethod.html) is the
# base class for objects representing methods
class RDoc::AnyMethod < ::RDoc::MethodAttr
  include(::RDoc::TokenStream)

  MARSHAL_VERSION = T.let(T.unsafe(nil), Integer)

  # Creates a new
  # [`AnyMethod`](https://docs.ruby-lang.org/en/2.7.0/RDoc/AnyMethod.html) with
  # a token stream `text` and `name`
  def self.new(text, name); end

  # Adds `an_alias` as an alias for this method in `context`.
  def add_alias(an_alias, context = _); end

  # Prefix for `aref` is 'method'.
  def aref_prefix; end

  # The
  # [`call_seq`](https://docs.ruby-lang.org/en/2.7.0/RDoc/AnyMethod.html#attribute-i-call_seq)
  # or the
  # [`param_seq`](https://docs.ruby-lang.org/en/2.7.0/RDoc/AnyMethod.html#method-i-param_seq)
  # with method name, if there is no call\_seq.
  #
  # Use this for displaying a method's argument lists.
  def arglists; end

  # The C function that implements this method (if it was defined in a C file)
  def c_function; end

  # The C function that implements this method (if it was defined in a C file)
  def c_function=(_); end

  # Different ways to call this method
  def call_seq; end

  # Sets the different ways you can call this method. If an empty `call_seq` is
  # given nil is assumed.
  #
  # See also
  # [`param_seq`](https://docs.ruby-lang.org/en/2.7.0/RDoc/AnyMethod.html#method-i-param_seq)
  def call_seq=(call_seq); end

  # If true this method uses `super` to call a superclass version
  def calls_super; end

  # If true this method uses `super` to call a superclass version
  def calls_super=(_); end

  # Don't rename initialize to ::new
  def dont_rename_initialize; end

  # Don't rename initialize to ::new
  def dont_rename_initialize=(_); end

  def is_alias_for; end

  # Dumps this
  # [`AnyMethod`](https://docs.ruby-lang.org/en/2.7.0/RDoc/AnyMethod.html) for
  # use by ri. See also
  # [`marshal_load`](https://docs.ruby-lang.org/en/2.7.0/RDoc/AnyMethod.html#method-i-marshal_load)
  def marshal_dump; end

  # Loads this
  # [`AnyMethod`](https://docs.ruby-lang.org/en/2.7.0/RDoc/AnyMethod.html) from
  # `array`. For a loaded
  # [`AnyMethod`](https://docs.ruby-lang.org/en/2.7.0/RDoc/AnyMethod.html) the
  # following methods will return cached values:
  #
  # *   [`full_name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/MethodAttr.html#method-i-full_name)
  # *   [`parent_name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/MethodAttr.html#method-i-parent_name)
  def marshal_load(array); end

  # [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) name
  #
  # If the method has no assigned name, it extracts it from
  # [`call_seq`](https://docs.ruby-lang.org/en/2.7.0/RDoc/AnyMethod.html#attribute-i-call_seq).
  def name; end

  # A list of this method's method and yield parameters. `call-seq` params are
  # preferred over parsed method and block params.
  def param_list; end

  # Pretty parameter list for this method. If the method's parameters were given
  # by `call-seq` it is preferred over the parsed values.
  def param_seq; end

  # Parameters for this method
  def params; end

  # Parameters for this method
  def params=(_); end

  # Sets the store for this method and its referenced code objects.
  def store=(store); end

  # For methods that `super`, find the superclass method that would be called.
  def superclass_method; end
end

# An attribute created by attr, attr\_reader, attr\_writer or attr\_accessor
class RDoc::Attr < ::RDoc::MethodAttr
  MARSHAL_VERSION = T.let(T.unsafe(nil), Integer)

  # Creates a new [`Attr`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Attr.html)
  # with body `text`, `name`, read/write status `rw` and `comment`. `singleton`
  # marks this as a class attribute.
  def self.new(text, name, rw, comment, singleton = _); end

  # Attributes are equal when their names, singleton and rw are identical
  def ==(other); end

  # Add `an_alias` as an attribute in `context`.
  def add_alias(an_alias, context); end

  # The
  # [`aref`](https://docs.ruby-lang.org/en/2.7.0/RDoc/MethodAttr.html#method-i-aref)
  # prefix for attributes
  def aref_prefix; end

  def calls_super; end

  # Returns attr\_reader, attr\_writer or attr\_accessor as appropriate.
  def definition; end

  def inspect; end

  # Dumps this [`Attr`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Attr.html) for
  # use by ri. See also
  # [`marshal_load`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Attr.html#method-i-marshal_load)
  def marshal_dump; end

  # Loads this [`Attr`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Attr.html) from
  # `array`. For a loaded
  # [`Attr`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Attr.html) the following
  # methods will return cached values:
  #
  # *   [`full_name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/MethodAttr.html#method-i-full_name)
  # *   [`parent_name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/MethodAttr.html#method-i-parent_name)
  def marshal_load(array); end

  def pretty_print(q); end

  # Is the attribute readable ('R'), writable ('W') or both ('RW')?
  def rw; end

  # Is the attribute readable ('R'), writable ('W') or both ('RW')?
  def rw=(_); end

  def to_s; end

  def token_stream; end
end

# [`ClassModule`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html) is
# the base class for objects representing either a class or a module.
class RDoc::ClassModule < ::RDoc::Context
  MARSHAL_VERSION = T.let(T.unsafe(nil), Integer)

  # Creates a new
  # [`ClassModule`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html)
  # with `name` with optional `superclass`
  #
  # This is a constructor for subclasses, and must never be called directly.
  def self.new(name, superclass = _); end

  # Adds `comment` to this ClassModule's list of comments at `location`. This
  # method is preferred over
  # [`comment=`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#method-i-comment-3D)
  # since it allows ri data to be updated across multiple runs.
  def add_comment(comment, location); end

  def add_things(my_things, other_things); end

  # Ancestors list for this ClassModule: the list of included modules (classes
  # will add their superclass if any).
  #
  # Returns the included classes or modules, not the includes themselves. The
  # returned values are either
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) or
  # [`RDoc::NormalModule`](https://docs.ruby-lang.org/en/2.7.0/RDoc/NormalModule.html)
  # instances (see
  # [`RDoc::Include#module`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Mixin.html#method-i-module)).
  #
  # The values are returned in reverse order of their inclusion, which is the
  # order suitable for searching methods/attributes in the ancestors. The
  # superclass, if any, comes last.
  #
  # Also aliased as:
  # [`direct_ancestors`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html#method-i-direct_ancestors)
  def ancestors; end

  # HTML fragment reference for this module or class. See
  # [`RDoc::NormalClass#aref`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html#method-i-aref)
  # and
  # [`RDoc::NormalModule#aref`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html#method-i-aref)
  def aref; end

  def aref_prefix; end

  # Clears the comment. Used by the Ruby parser.
  def clear_comment; end

  def comment=(comment); end

  # Comment and the location it came from. Use
  # [`add_comment`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html#method-i-add_comment)
  # to add comments
  def comment_location; end

  # Comment and the location it came from. Use
  # [`add_comment`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html#method-i-add_comment)
  # to add comments
  def comment_location=(_); end

  # Prepares this
  # [`ClassModule`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html)
  # for use by a generator.
  #
  # See
  # [`RDoc::Store#complete`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html#method-i-complete)
  def complete(min_visibility); end

  # Constants that are aliases for this class or module
  def constant_aliases; end

  # Constants that are aliases for this class or module
  def constant_aliases=(_); end

  # Handy wrapper for marking up this class or module's comment
  def description; end

  def diagram; end

  def diagram=(_); end

  # Ancestors of this class or module only
  #
  # Alias for:
  # [`ancestors`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html#method-i-ancestors)
  def direct_ancestors; end

  # Does this
  # [`ClassModule`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html)
  # or any of its methods have document\_self set?
  def document_self_or_methods; end

  # Does this class or module have a comment with content or is
  # [`received_nodoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#attribute-i-received_nodoc)
  # true?
  def documented?; end

  # Iterates the ancestors of this class or module for which an
  # [`RDoc::ClassModule`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html)
  # exists.
  def each_ancestor; end

  # Looks for a symbol in the
  # [`ancestors`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html#method-i-ancestors).
  # See Context#find\_local\_symbol.
  def find_ancestor_local_symbol(symbol); end

  # Finds a class or module with `name` in this namespace or its descendants
  def find_class_named(name); end

  # Return the fully qualified name of this class or module
  def full_name; end

  # [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) or module this
  # constant is an alias for
  def is_alias_for; end

  # [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) or module this
  # constant is an alias for
  def is_alias_for=(_); end

  def marshal_dump; end

  def marshal_load(array); end

  # Merges `class_module` into this
  # [`ClassModule`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html).
  #
  # The data in `class_module` is preferred over the receiver.
  def merge(class_module); end

  def merge_collections(mine, other, other_files, &block); end

  def merge_sections(cm); end

  # Does this object represent a module?
  def module?; end

  # Allows overriding the initial name.
  #
  # Used for modules and classes that are constant aliases.
  def name=(new_name); end

  # Name to use to generate the url: modules and classes that are aliases for
  # another module or class return the name of the latter.
  def name_for_path; end

  # Returns the classes and modules that are not constants aliasing another
  # class or module. For use by formatters only (caches its result).
  def non_aliases; end

  # Parses `comment_location` into an RDoc::Markup::Document composed of
  # multiple RDoc::Markup::Documents with their file set.
  def parse(comment_location); end

  # Path to this class or module for use with HTML generator output.
  def path; end

  # Updates the child modules or classes of class/module `parent` by deleting
  # the ones that have been removed from the documentation.
  #
  # `parent_hash` is either `parent.modules_hash` or `parent.classes_hash` and
  # `all_hash` is ::all\_modules\_hash or ::all\_classes\_hash.
  def remove_nodoc_children; end

  def remove_things(my_things, other_files); end

  # Search record used by RDoc::Generator::JsonIndex
  def search_record; end

  # Sets the store for this class or module and its contained code objects.
  def store=(store); end

  # Get the superclass of this class. Attempts to retrieve the superclass
  # object, returns the name if it is not known.
  def superclass; end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the superclass of this
  # class to `superclass`
  def superclass=(superclass); end

  def to_s; end

  # 'module' or 'class'
  def type; end

  # Updates the child modules & classes by replacing the ones that are aliases
  # through a constant.
  #
  # The aliased module/class is replaced in the children and in
  # [`RDoc::Store#modules_hash`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html#method-i-modules_hash)
  # or
  # [`RDoc::Store#classes_hash`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html#method-i-classes_hash)
  # by a copy that has `RDoc::ClassModule#is_alias_for` set to the aliased
  # module/class, and this copy is added to `#aliases` of the aliased
  # module/class.
  #
  # Formatters can use the
  # [`non_aliases`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html#method-i-non_aliases)
  # method to retrieve children that are not aliases, for instance to list the
  # namespace content, since the aliased modules are included in the constants
  # of the class/module, that are listed separately.
  def update_aliases; end

  # Deletes from
  # [`extends`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html#attribute-i-extends)
  # those whose module has been removed from the documentation.
  def update_extends; end

  # Deletes from
  # [`includes`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html#attribute-i-includes)
  # those whose module has been removed from the documentation.
  def update_includes; end

  # Return a
  # [`RDoc::ClassModule`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html)
  # of class `class_type` that is a copy of module `module`. Used to promote
  # modules to classes.
  def self.from_module(class_type, mod); end
end

# Base class for the [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
# code tree.
#
# We contain the common stuff for contexts (which are containers) and other
# elements (methods, attributes and so on)
#
# Here's the tree of the
# [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html)
# subclasses:
#
# *   [`RDoc::Context`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html)
#     *   [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
#     *   [`RDoc::ClassModule`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html)
#         *   [`RDoc::AnonClass`](https://docs.ruby-lang.org/en/2.7.0/RDoc/AnonClass.html)
#             (never used so far)
#         *   [`RDoc::NormalClass`](https://docs.ruby-lang.org/en/2.7.0/RDoc/NormalClass.html)
#         *   [`RDoc::NormalModule`](https://docs.ruby-lang.org/en/2.7.0/RDoc/NormalModule.html)
#         *   [`RDoc::SingleClass`](https://docs.ruby-lang.org/en/2.7.0/RDoc/SingleClass.html)
#
#
# *   [`RDoc::MethodAttr`](https://docs.ruby-lang.org/en/2.7.0/RDoc/MethodAttr.html)
#     *   [`RDoc::Attr`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Attr.html)
#     *   [`RDoc::AnyMethod`](https://docs.ruby-lang.org/en/2.7.0/RDoc/AnyMethod.html)
#         *   [`RDoc::GhostMethod`](https://docs.ruby-lang.org/en/2.7.0/RDoc/GhostMethod.html)
#         *   [`RDoc::MetaMethod`](https://docs.ruby-lang.org/en/2.7.0/RDoc/MetaMethod.html)
#
#
# *   [`RDoc::Alias`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Alias.html)
# *   [`RDoc::Constant`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Constant.html)
# *   [`RDoc::Mixin`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Mixin.html)
#     *   [`RDoc::Require`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Require.html)
#     *   [`RDoc::Include`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Include.html)
class RDoc::CodeObject
  include(::RDoc::Generator::Markup)
  include(::RDoc::Text)

  # Creates a new
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html)
  # that will document itself and its children
  def self.new; end

  # Our comment
  def comment; end

  # Replaces our comment with `comment`, unless it is empty.
  def comment=(comment); end

  # Should this
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html) be
  # displayed in output?
  #
  # A code object should be displayed if:
  #
  # *   The item didn't have a nodoc or wasn't in a container that had nodoc
  # *   The item wasn't ignored
  # *   The item has documentation and was not suppressed
  def display?; end

  # Do we document our children?
  def document_children; end

  # Enables or disables documentation of this CodeObject's children unless it
  # has been turned off by :enddoc:
  def document_children=(document_children); end

  # Do we document ourselves?
  def document_self; end

  # Enables or disables documentation of this
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html)
  # unless it has been turned off by :enddoc:. If the argument is `nil` it means
  # the documentation is turned off by `:nodoc:`.
  def document_self=(document_self); end

  # Does this object have a comment with content or is
  # [`received_nodoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#attribute-i-received_nodoc)
  # true?
  def documented?; end

  # Are we done documenting (ie, did we come across a :enddoc:)?
  def done_documenting; end

  # Turns documentation on/off, and turns on/off
  # [`document_self`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#attribute-i-document_self)
  # and
  # [`document_children`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#attribute-i-document_children).
  #
  # Once documentation has been turned off (by `:enddoc:`), the object will
  # refuse to turn
  # [`document_self`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#attribute-i-document_self)
  # or
  # [`document_children`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#attribute-i-document_children)
  # on, so `:doc:` and `:start_doc:` directives will have no effect in the
  # current file.
  def done_documenting=(value); end

  # Yields each parent of this
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html).
  # See also
  # [`RDoc::ClassModule#each_ancestor`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html#method-i-each_ancestor)
  def each_parent; end

  # Which file this code object was defined in
  def file; end

  # [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) name where this
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html) was
  # found.
  #
  # See also
  # [`RDoc::Context#in_files`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html#attribute-i-in_files)
  def file_name; end

  # Force documentation of this
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html)
  def force_documentation; end

  # Force the documentation of this object unless documentation has been turned
  # off by :enddoc:
  def force_documentation=(value); end

  # Sets the full\_name overriding any computed full name.
  #
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) to `nil` to clear
  # RDoc's cached value
  def full_name=(full_name); end

  # Use this to ignore a
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html) and
  # all its children until found again
  # ([`record_location`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#method-i-record_location)
  # is called). An ignored item will not be displayed in documentation.
  #
  # See github issue #55
  #
  # The ignored status is temporary in order to allow implementation details to
  # be hidden. At the end of processing a file
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) allows all classes
  # and modules to add new documentation to previously created classes.
  #
  # If a class was ignored (via stopdoc) then reopened later with additional
  # documentation it should be displayed. If a class was ignored and never
  # reopened it should not be displayed. The ignore flag allows this to occur.
  def ignore; end

  # Has this class been ignored?
  #
  # See also
  # [`ignore`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#method-i-ignore)
  def ignored?; end

  def self.new_visibility; end

  # Line in
  # [`file`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#attribute-i-file)
  # where this
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html) was
  # defined
  def line; end

  # Line in
  # [`file`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#attribute-i-file)
  # where this
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html) was
  # defined
  def line=(_); end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of arbitrary
  # metadata for this
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html)
  def metadata; end

  # The options instance from the store this
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html) is
  # attached to, or a default options instance if the
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html) is
  # not attached.
  #
  # This is used by Text#snippet
  def options; end

  # Sets the parent
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html)
  def parent; end

  # Sets the parent
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.6.0/RDoc/CodeObject.html)
  def parent=(_); end

  # [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) name of our parent
  def parent_file_name; end

  # Name of our parent
  def parent_name; end

  # Did we ever receive a `:nodoc:` directive?
  def received_nodoc; end

  # Records the
  # [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
  # (file) where this code object was defined
  def record_location(top_level); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the section this
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html) is
  # in
  def section; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the section this
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.6.0/RDoc/CodeObject.html) is
  # in
  def section=(_); end

  # Enable capture of documentation unless documentation has been turned off by
  # :enddoc:
  def start_doc; end

  # Disable capture of documentation
  def stop_doc; end

  # The [`RDoc::Store`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html) for
  # this object.
  def store; end

  # Sets the `store` that contains this
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html)
  def store=(store); end

  # Use this to suppress a
  # [`CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html) and
  # all its children until the next file it is seen in or documentation is
  # discovered. A suppressed item with documentation will be displayed while an
  # ignored item with documentation may not be displayed.
  def suppress; end

  # Has this class been suppressed?
  #
  # See also
  # [`suppress`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#method-i-suppress)
  def suppressed?; end

  # We are the model of the code, but we know that at some point we will be
  # worked on by viewers. By implementing the Viewable protocol, viewers can
  # associated themselves with these objects.
  def viewer; end

  # We are the model of the code, but we know that at some point we will be
  # worked on by viewers. By implementing the Viewable protocol, viewers can
  # associated themselves with these objects.
  def viewer=(_); end
end

# A comment holds the text comment for a
# [`RDoc::CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html)
# and provides a unified way of cleaning it up and parsing it into an
# RDoc::Markup::Document.
#
# Each comment may have a different markup format set by
# [`format=`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Comment.html#method-i-format-3D).
# By default 'rdoc' is used. The :markup: directive tells
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) which format to use.
#
# See [Other directives at
# `RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html#class-RDoc::Markup-label-Other+directives)
# for instructions on adding an alternate format.
class RDoc::Comment
  include(::RDoc::Text)

  # Creates a new comment with `text` that is found in the
  # [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
  # `location`.
  def self.new(text = _, location = _); end

  def ==(other); end

  # Overrides the content returned by
  # [`parse`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Comment.html#method-i-parse).
  # Use when there is no
  # [`text`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Comment.html#attribute-i-text)
  # source for this comment
  def document=(_); end

  # A comment is empty if its text
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) is empty.
  def empty?; end

  # HACK dubious
  def encode!(encoding); end

  # Look for a 'call-seq' in the comment to override the normal parameter
  # handling. The :call-seq: is indented from the baseline. All lines of the
  # same indentation level and prefix are consumed.
  #
  # For example, all of the following will be used as the :call-seq:
  #
  # ```ruby
  # # :call-seq:
  # #   ARGF.readlines(sep=$/)     -> array
  # #   ARGF.readlines(limit)      -> array
  # #   ARGF.readlines(sep, limit) -> array
  # #
  # #   ARGF.to_a(sep=$/)     -> array
  # #   ARGF.to_a(limit)      -> array
  # #   ARGF.to_a(sep, limit) -> array
  # ```
  def extract_call_seq(method); end

  # The
  # [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
  # this comment was found in
  def file; end

  # The format of this comment. Defaults to
  # [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html)
  def format; end

  # Sets the format of this comment and resets any parsed document
  def format=(format); end

  def inspect; end

  # The
  # [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
  # this comment was found in
  def location; end

  # The
  # [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
  # this comment was found in
  def location=(_); end

  # Normalizes the text. See
  # [`RDoc::Text#normalize_comment`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Text.html#method-i-normalize_comment)
  # for details
  def normalize; end

  def normalized?; end

  # Parses the comment into an RDoc::Markup::Document. The parsed document is
  # cached until the text is changed.
  def parse; end

  # Removes private sections from this comment. Private sections are flush to
  # the comment marker and start with `--` and end with `++`. For C-style
  # comments, a private marker may not start at the opening of the comment.
  #
  # ```
  # /*
  #  *--
  #  * private
  #  *++
  #  * public
  #  */
  # ```
  def remove_private; end

  # The text for this comment
  def text; end

  # Replaces this comment's text with `text` and resets the parsed document.
  #
  # An error is raised if the comment contains a document but no text.
  def text=(text); end

  # Returns true if this comment is in TomDoc format.
  def tomdoc?; end
end

# A constant
class RDoc::Constant < ::RDoc::CodeObject
  ::RDoc::Constant::MARSHAL_VERSION = T.let(T.unsafe(nil), Integer)

  # Creates a new constant with `name`, `value` and `comment`
  def self.new(name, value, comment); end

  # Constants are ordered by name
  def <=>(other); end

  # Constants are equal when their
  # [`parent`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#attribute-i-parent)
  # and
  # [`name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Constant.html#attribute-i-name)
  # is the same
  def ==(other); end

  # A constant is documented if it has a comment, or is an alias for a
  # documented class or module.
  def documented?; end

  # Full constant name including namespace
  def full_name; end

  def inspect; end

  # Sets the module or class this is constant is an alias for.
  def is_alias_for; end

  # Sets the module or class this is constant is an alias for.
  def is_alias_for=(_); end

  # Dumps this
  # [`Constant`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Constant.html) for use
  # by ri. See also
  # [`marshal_load`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Constant.html#method-i-marshal_load)
  def marshal_dump; end

  # Loads this
  # [`Constant`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Constant.html) from
  # `array`. For a loaded
  # [`Constant`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Constant.html) the
  # following methods will return cached values:
  #
  # *   [`full_name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Constant.html#method-i-full_name)
  # *   [`parent_name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#method-i-parent_name)
  def marshal_load(array); end

  # The constant's name
  def name; end

  # The constant's name
  def name=(_); end

  # Path to this constant for use with HTML generator output.
  def path; end

  def pretty_print(q); end

  # Sets the store for this class or module and its contained code objects.
  def store=(store); end

  def to_s; end

  # The constant's value
  def value; end

  # The constant's value
  def value=(_); end

  # The constant's visibility
  def visibility; end

  # The constant's visibility
  def visibility=(_); end
end

# A [`Context`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html) is
# something that can hold modules, classes, methods, attributes, aliases,
# requires, and includes. Classes, modules, and files are all Contexts.
class RDoc::Context < ::RDoc::CodeObject
  include(::Comparable)

  TOMDOC_TITLES = T.let(T.unsafe(nil), T::Array[T.untyped])

  TOMDOC_TITLES_SORT = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Types of methods
  TYPES = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Creates an unnamed empty context with public current visibility
  def self.new; end

  # Contexts are sorted by
  # [`full_name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html#method-i-full_name)
  def <=>(other); end

  # Adds an item of type `klass` with the given `name` and `comment` to the
  # context.
  #
  # Currently only
  # [`RDoc::Extend`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Extend.html) and
  # [`RDoc::Include`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Include.html) are
  # supported.
  def add(klass, name, comment); end

  # Adds `an_alias` that is automatically resolved
  def add_alias(an_alias); end

  # Adds `attribute` if not already there. If it is (as method(s) or attribute),
  # updates the comment if it was empty.
  #
  # The attribute is registered only if it defines a new method. For instance,
  # `attr_reader :foo` will not be registered if method `foo` exists, but
  # `attr_accessor :foo` will be registered if method `foo` exists, but `foo=`
  # does not.
  def add_attribute(attribute); end

  # Adds a class named `given_name` with `superclass`.
  #
  # Both `given_name` and `superclass` may contain '::', and are interpreted
  # relative to the `self` context. This allows handling correctly examples like
  # these:
  #
  # ```
  # class RDoc::Gauntlet < Gauntlet
  # module Mod
  #   class Object   # implies < ::Object
  #   class SubObject < Object  # this is _not_ ::Object
  # ```
  #
  # Given `class Container::Item`
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) assumes `Container`
  # is a module unless it later sees `class Container`. `add_class`
  # automatically upgrades `given_name` to a class in this case.
  def add_class(class_type, given_name, superclass = _); end

  # Adds the class or module `mod` to the modules or classes
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) `self_hash`, and to
  # `all_hash` (either `TopLevel::modules_hash` or `TopLevel::classes_hash`),
  # unless
  # [`done_documenting`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#attribute-i-done_documenting)
  # is `true`. Sets the
  # [`parent`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#attribute-i-parent)
  # of `mod` to `self`, and its
  # [`section`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#attribute-i-section)
  # to
  # [`current_section`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html#attribute-i-current_section).
  # Returns `mod`.
  def add_class_or_module(mod, self_hash, all_hash); end

  # Adds `constant` if not already there. If it is, updates the comment, value
  # and/or is\_alias\_for of the known constant if they were empty/nil.
  def add_constant(constant); end

  # Adds extension module `ext` which should be an
  # [`RDoc::Extend`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Extend.html)
  def add_extend(ext); end

  # Adds included module `include` which should be an
  # [`RDoc::Include`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Include.html)
  def add_include(include); end

  # Adds `method` if not already there. If it is (as method or attribute),
  # updates the comment if it was empty.
  def add_method(method); end

  # Adds a module named `name`. If
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) already knows `name`
  # is a class then that class is returned instead. See also
  # [`add_class`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html#method-i-add_class).
  def add_module(class_type, name); end

  # Adds an alias from `from` (a class or module) to `name` which was defined in
  # `file`.
  def add_module_alias(from, name, file); end

  # Adds `require` to this context's top level
  def add_require(require); end

  # Returns a section with `title`, creating it if it doesn't already exist.
  # `comment` will be appended to the section's comment.
  #
  # A section with a `title` of `nil` will return the default section.
  #
  # See also RDoc::Context::Section
  def add_section(title, comment = _); end

  # Adds `thing` to the collection `array`
  def add_to(array, thing); end

  # Class/module aliases
  def aliases; end

  # Is there any content?
  #
  # This means any of: comment, aliases, methods, attributes, external aliases,
  # require, constant.
  #
  # Includes and extends are also checked unless `includes == false`.
  def any_content(includes = _); end

  # All attr\* methods
  def attributes; end

  # Block params to be used in the next MethodAttr parsed under this context
  def block_params; end

  # Block params to be used in the next MethodAttr parsed under this context
  def block_params=(_); end

  # Creates the full name for a child with `name`
  def child_name(name); end

  # [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) attributes
  def class_attributes; end

  # [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) methods
  def class_method_list; end

  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of classes in this
  # context
  def classes; end

  # All classes and modules in this namespace
  def classes_and_modules; end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of classes keyed by
  # class name
  def classes_hash; end

  # Constants defined
  def constants; end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of registered
  # constants.
  def constants_hash; end

  # Current visibility of this line
  def current_line_visibility=(_); end

  # Sets the current documentation section of documentation
  def current_section; end

  # Sets the current documentation section of documentation
  def current_section=(_); end

  # Is part of this thing was defined in `file`?
  def defined_in?(file); end

  def display(method_attr); end

  def each_ancestor; end

  # Iterator for attributes
  def each_attribute; end

  # Iterator for classes and modules
  def each_classmodule(&block); end

  # Iterator for constants
  def each_constant; end

  # Iterator for extension modules
  def each_extend; end

  # Iterator for included modules
  def each_include; end

  # Iterator for methods
  def each_method; end

  # Iterator for each section's contents sorted by title. The `section`, the
  # section's `constants` and the sections `attributes` are yielded. The
  # `constants` and `attributes` collections are sorted.
  #
  # To retrieve methods in a section use
  # [`methods_by_type`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html#method-i-methods_by_type)
  # with the optional `section` parameter.
  #
  # NOTE: Do not edit collections yielded by this method
  def each_section; end

  # Modules this context is extended with
  def extends; end

  # Aliases that could not be resolved.
  def external_aliases; end

  # Finds an attribute `name` with singleton value `singleton`.
  def find_attribute(name, singleton); end

  # Finds an attribute with `name` in this context
  def find_attribute_named(name); end

  # Finds a class method with `name` in this context
  def find_class_method_named(name); end

  # Finds a constant with `name` in this context
  def find_constant_named(name); end

  # [`Find`](https://docs.ruby-lang.org/en/2.7.0/Find.html) a module at a higher
  # scope
  def find_enclosing_module_named(name); end

  # Finds an external alias `name` with singleton value `singleton`.
  def find_external_alias(name, singleton); end

  # Finds an external alias with `name` in this context
  def find_external_alias_named(name); end

  # Finds a file with `name` in this context
  def find_file_named(name); end

  # Finds an instance method with `name` in this context
  def find_instance_method_named(name); end

  # Finds a method, constant, attribute, external alias, module or file named
  # `symbol` in this context.
  def find_local_symbol(symbol); end

  # Finds a method named `name` with singleton value `singleton`.
  def find_method(name, singleton); end

  # Finds a instance or module method with `name` in this context
  def find_method_named(name); end

  # [`Find`](https://docs.ruby-lang.org/en/2.7.0/Find.html) a module with `name`
  # using ruby's scoping rules
  def find_module_named(name); end

  # Look up `symbol`, first as a module, then as a local symbol.
  def find_symbol(symbol); end

  # Look up a module named `symbol`.
  def find_symbol_module(symbol); end

  # The full name for this context. This method is overridden by subclasses.
  def full_name; end

  # Does this context and its methods and constants all have documentation?
  #
  # (Yes, fully documented doesn't mean everything.)
  def fully_documented?; end

  # URL for this with a `prefix`
  def http_url(prefix); end

  # Files this context is found in
  def in_files; end

  # Modules this context includes
  def includes; end

  def self.new_methods_etc; end

  # Instance attributes
  def instance_attributes; end

  # Instance methods
  def instance_method_list; end

  # Methods defined in this context
  def method_list; end

  # Breaks
  # [`method_list`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html#attribute-i-method_list)
  # into a nested hash by type (`'class'` or `'instance'`) and visibility
  # (`:public`, `:protected`, `:private`).
  #
  # If `section` is provided only methods in that RDoc::Context::Section will be
  # returned.
  def methods_by_type(section = _); end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of registered
  # methods. Attributes are also registered here, twice if they are RW.
  def methods_hash; end

  # Yields AnyMethod and Attr entries matching the list of names in `methods`.
  def methods_matching(methods, singleton = _, &block); end

  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of modules in this
  # context
  def modules; end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of modules keyed by
  # module name
  def modules_hash; end

  # Name of this class excluding namespace. See also
  # [`full_name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html#method-i-full_name)
  def name; end

  # Name to use to generate the url. `#full_name` by default.
  def name_for_path; end

  # Changes the visibility for new methods to `visibility`
  def ongoing_visibility=(visibility); end

  # Params to be used in the next MethodAttr parsed under this context
  def params; end

  # Params to be used in the next MethodAttr parsed under this context
  def params=(_); end

  # Record `top_level` as a file `self` is in.
  def record_location(top_level); end

  # Should we remove this context from the documentation?
  #
  # The answer is yes if:
  # *   [`received_nodoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#attribute-i-received_nodoc)
  #     is `true`
  # *   [`any_content`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html#method-i-any_content)
  #     is `false` (not counting includes)
  # *   All
  #     [`includes`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html#attribute-i-includes)
  #     are modules (not a string), and their module has
  #     `#remove_from_documentation? == true`
  # *   All classes and modules have `#remove_from_documentation? == true`
  def remove_from_documentation?; end

  # Removes methods and attributes with a visibility less than `min_visibility`.
  def remove_invisible(min_visibility); end

  def remove_invisible_in(array, min_visibility); end

  # Files this context requires
  def requires; end

  # Tries to resolve unmatched aliases when a method or attribute has just been
  # added.
  def resolve_aliases(added); end

  # Returns RDoc::Context::Section objects referenced in this context for use in
  # a table of contents.
  def section_contents; end

  # Sections in this context
  def sections; end

  def sections_hash; end

  # Given an array `names` of constants, set the visibility of each constant to
  # `visibility`
  def set_constant_visibility_for(names, visibility); end

  # Sets the current section to a section with `title`. See also
  # [`add_section`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html#method-i-add_section)
  def set_current_section(title, comment); end

  # Given an array `methods` of method names, set the visibility of each to
  # `visibility`
  def set_visibility_for(methods, visibility, singleton = _); end

  # Sorts sections alphabetically (default) or in TomDoc fashion (none, Public,
  # Internal, Deprecated)
  def sort_sections; end

  # Use this section for the next method, attribute or constant added.
  def temporary_section; end

  # Use this section for the next method, attribute or constant added.
  def temporary_section=(_); end

  def to_s; end

  # Return the TopLevel that owns us
  def top_level; end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) `old_name =>
  # [aliases]`, for aliases that haven't (yet) been resolved to a
  # method/attribute. (Not to be confused with the aliases of the context.)
  def unmatched_alias_lists; end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) `old_name =>
  # [aliases]`, for aliases that haven't (yet) been resolved to a
  # method/attribute. (Not to be confused with the aliases of the context.)
  def unmatched_alias_lists=(_); end

  # Upgrades NormalModule `mod` in `enclosing` to a `class_type`
  def upgrade_to_class(mod, class_type, enclosing); end

  # Current visibility of this context
  def visibility; end

  # Current visibility of this context
  def visibility=(_); end
end

# A section of documentation like:
#
# ```ruby
# # :section: The title
# # The body
# ```
#
# Sections can be referenced multiple times and will be collapsed into a single
# section.
class RDoc::Context::Section
  include(::RDoc::Generator::Markup)
  include(::RDoc::Text)

  MARSHAL_VERSION = T.let(T.unsafe(nil), Integer)

  # Creates a new section with `title` and `comment`
  def self.new(parent, title, comment); end

  # Sections are equal when they have the same
  # [`title`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Context/Section.html#attribute-i-title)
  #
  # Also aliased as:
  # [`eql?`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Context/Section.html#method-i-eql-3F)
  def ==(other); end

  # Adds `comment` to this section
  def add_comment(comment); end

  # Anchor reference for linking to this section
  def aref; end

  # [`Section`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Context/Section.html)
  # comment
  def comment; end

  # [`Section`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Context/Section.html)
  # comments
  def comments; end

  # Alias for:
  # [`==`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Context/Section.html#method-i-3D-3D)
  def eql?(other); end

  # Extracts the comment for this section from the original comment block. If
  # the first line contains :section:, strip it and use the rest. Otherwise
  # remove lines up to the line containing :section:, and look for those lines
  # again at the end and remove them. This lets us write
  #
  # ```ruby
  # # :section: The title
  # # The body
  # ```
  def extract_comment(comment); end

  def hash; end

  # The files comments in this section come from
  def in_files; end

  def inspect; end

  # Serializes this
  # [`Section`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Context/Section.html).
  # The title and parsed comment are saved, but not the section parent which
  # must be restored manually.
  def marshal_dump; end

  # De-serializes this
  # [`Section`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Context/Section.html).
  # The section parent must be restored manually.
  def marshal_load(array); end

  # Context this
  # [`Section`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Context/Section.html)
  # lives in
  def parent; end

  # Parses `comment_location` into an
  # [`RDoc::Markup::Document`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Document.html)
  # composed of multiple RDoc::Markup::Documents with their file set.
  def parse; end

  # The section's title, or 'Top Section' if the title is nil.
  #
  # This is used by the table of contents template so the name is silly.
  def plain_html; end

  # Removes a comment from this section if it is from the same file as `comment`
  def remove_comment(comment); end

  # [`Section`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Context/Section.html)
  # sequence number (deprecated)
  def sequence; end

  # [`Section`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Context/Section.html)
  # title
  def title; end
end

# [`RDoc::CrossReference`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CrossReference.html)
# is a reusable way to create cross references for names.
class RDoc::CrossReference
  # Version of
  # [`CROSSREF_REGEXP`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CrossReference.html#CROSSREF_REGEXP)
  # used when `--hyperlink-all` is specified.
  ALL_CROSSREF_REGEXP = T.let(T.unsafe(nil), Regexp)

  # Regular expression to match class references
  #
  # 1.  There can be a '\' in front of text to suppress the cross-reference
  # 2.  There can be a '::' in front of class names to reference from the
  #     top-level namespace.
  # 3.  The method can be followed by parenthesis (not recommended)
  CLASS_REGEXP_STR = T.let(T.unsafe(nil), String)

  # Regular expressions matching text that should potentially have
  # cross-reference links generated are passed to add\_regexp\_handling. Note
  # that these expressions are meant to pick up text for which cross-references
  # have been suppressed, since the suppression characters are removed by the
  # code that is triggered.
  CROSSREF_REGEXP = T.let(T.unsafe(nil), Regexp)

  # Regular expression to match method references.
  #
  # See
  # [`CLASS_REGEXP_STR`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CrossReference.html#CLASS_REGEXP_STR)
  METHOD_REGEXP_STR = T.let(T.unsafe(nil), String)

  # Allows cross-references to be created based on the given `context`
  # ([`RDoc::Context`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html)).
  def self.new(context); end

  # Returns a reference to `name`.
  #
  # If the reference is found and `name` is not documented `text` will be
  # returned. If `name` is escaped `name` is returned. If `name` is not found
  # `text` is returned.
  def resolve(name, text); end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of references that
  # have been looked-up to their replacements
  def seen; end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of references that
  # have been looked-up to their replacements
  def seen=(_); end
end

# A subclass of [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html) that
# writes directly to an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
# Credit to Aaron Patterson and Masatoshi SEKI.
#
# To use:
#
# ```ruby
# erbio = RDoc::ERBIO.new '<%= "hello world" %>', nil, nil
#
# File.open 'hello.txt', 'w' do |io|
#   erbio.result binding
# end
# ```
#
# Note that binding must enclose the io you wish to output on.
class RDoc::ERBIO < ::ERB
  # Defaults `eoutvar` to 'io', otherwise is identical to ERB's initialize
  def self.new(str, safe_level = _, trim_mode = _, eoutvar = _); end

  # Instructs `compiler` how to write to `io_variable`
  def set_eoutvar(compiler, io_variable); end
end

# Allows an [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html) template to be
# rendered in the context (binding) of an existing
# [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html) template evaluation.
class RDoc::ERBPartial < ::ERB
  # Overrides `compiler` startup to set the `eoutvar` to an empty string only if
  # it isn't already set.
  def set_eoutvar(compiler, eoutvar = _); end
end

# This class is a wrapper around
# [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html)
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) and
# [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Encoding.html) that
# helps [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) load files and
# convert them to the correct encoding.
module RDoc::Encoding
  # Changes encoding based on `encoding` without converting and returns new
  # string
  def self.change_encoding(text, encoding); end

  # Reads the contents of `filename` and handles any encoding directives in the
  # file.
  #
  # The content will be converted to the `encoding`. If the file cannot be
  # converted a warning will be printed and nil will be returned.
  #
  # If `force_transcode` is true the document will be transcoded and any unknown
  # character in the target encoding will be replaced with '?'
  def self.read_file(filename, encoding, force_transcode = _); end

  def self.remove_frozen_string_literal(string); end

  def self.set_encoding(string); end
end

# [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) thrown by
# any rdoc error.
class RDoc::Error < ::RuntimeError; end

# A [`Module`](https://docs.ruby-lang.org/en/2.7.0/Module.html) extension to a
# class with
# [`extend`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-extend)
#
# ```ruby
# RDoc::Extend.new 'Enumerable', 'comment ...'
# ```
class RDoc::Extend < ::RDoc::Mixin; end

# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) uses generators to
# turn parsed source code in the form of an
# [`RDoc::CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html)
# tree into some form of output.
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) comes with the HTML
# generator RDoc::Generator::Darkfish and an ri data generator
# RDoc::Generator::RI.
#
# ## Registering a [`Generator`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Generator.html)
#
# Generators are registered by calling
# [`RDoc::RDoc.add_generator`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html#method-c-add_generator)
# with the class of the generator:
#
# ```ruby
# class My::Awesome::Generator
#   RDoc::RDoc.add_generator self
# end
# ```
#
# ## Adding Options to `rdoc`
#
# Before option processing in `rdoc`,
# [`RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html) will
# call ::setup\_options on the generator class with an
# [`RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html)
# instance. The generator can use
# [`RDoc::Options#option_parser`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#attribute-i-option_parser)
# to add command-line options to the `rdoc` tool. See [Custom Options at
# `RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#class-RDoc::Options-label-Custom+Options)
# for an example and see
# [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html) for
# details on how to add options.
#
# You can extend the
# [`RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html)
# instance with additional accessors for your generator.
#
# ## [`Generator`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Generator.html) Instantiation
#
# After parsing,
# [`RDoc::RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RDoc.html) will
# instantiate a generator by calling initialize with an
# [`RDoc::Store`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html) instance
# and an
# [`RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html)
# instance.
#
# The [`RDoc::Store`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html)
# instance holds documentation for parsed source code. In
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) 3 and earlier the
# [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
# class held this data. When upgrading a generator from
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) 3 and earlier you
# should only need to replace
# [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
# with the store instance.
#
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) will then call
# generate on the generator instance. You can use the various methods on
# [`RDoc::Store`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html) and in
# the
# [`RDoc::CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html)
# tree to create your desired output format.
module RDoc::Generator; end

# [`Darkfish`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/Darkfish.html)
# [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) HTML Generator
#
# $Id: darkfish.rb 52 2009-01-07 02:08:11Z deveiant $
#
# ## Author/s
# *   Michael Granger (ged@FaerieMUD.org)
#
#
# ## Contributors
# *   Mahlon E. Smith (mahlon@martini.nu)
# *   Eric Hodel (drbrain@segment7.net)
#
#
# ## License
#
# Copyright (c) 2007, 2008, Michael Granger. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# *   Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#
# *   Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#
# *   Neither the name of the author/s, nor the names of the project's
#     contributors may be used to endorse or promote products derived from this
#     software without specific prior written permission.
#
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ## Attributions
#
# [`Darkfish`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/Darkfish.html)
# uses the [Silk Icons](http://www.famfamfam.com/lab/icons/silk/) set by Mark
# James.
class RDoc::Generator::Darkfish
  include(::ERB::Util)

  BUILTIN_STYLE_ITEMS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Description of this generator
  DESCRIPTION = T.let(T.unsafe(nil), String)

  # Path to this file's parent directory. Used to find templates and other
  # resources.
  GENERATOR_DIR = T.let(T.unsafe(nil), String)

  # %q$Id: darkfish.rb 52 2009-01-07 02:08:11Z deveiant $"
  SVNID_PATTERN = T.let(T.unsafe(nil), Regexp)

  # Release Version
  VERSION = T.let(T.unsafe(nil), String)

  # Initialize a few instance variables before we start
  def self.new(store, options); end

  # Creates a template from its components and the `body_file`.
  #
  # For backwards compatibility, if `body_file` contains "<html" the body is
  # used directly.
  def assemble_template(body_file); end

  # The relative path to style sheets and javascript. By default this is set the
  # same as the rel\_prefix.
  def asset_rel_path; end

  # The relative path to style sheets and javascript. By default this is set the
  # same as the rel\_prefix.
  def asset_rel_path=(_); end

  # The path to generate files into, combined with `--op` from the options for a
  # full path.
  def base_dir; end

  # Directory where generated class HTML files live relative to the output dir.
  def class_dir; end

  # Classes and modules to be used by this generator, not necessarily displayed.
  # See also
  # [`modsort`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/Darkfish.html#attribute-i-modsort)
  def classes; end

  # Copies static files from the static\_path into the output directory
  def copy_static; end

  # Output progress information if debugging is enabled
  def debug_msg(*msg); end

  # No files will be written when
  # [`dry_run`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/Darkfish.html#attribute-i-dry_run)
  # is true.
  def dry_run; end

  # No files will be written when
  # [`dry_run`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/Darkfish.html#attribute-i-dry_run)
  # is true.
  def dry_run=(_); end

  # Directory where generated class HTML files live relative to the output dir.
  def file_dir; end

  # When false the generate methods return a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) instead of
  # writing to a file. The default is true.
  def file_output; end

  # When false the generate methods return a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) instead of
  # writing to a file. The default is true.
  def file_output=(_); end

  # Files to be displayed by this generator
  def files; end

  # Create the directories the generated docs will live in if they don't already
  # exist.
  def gen_sub_directories; end

  # Build the initial indices and output objects based on an array of TopLevel
  # objects containing the extracted information.
  def generate; end

  # Generates a class file for `klass`
  def generate_class(klass, template_file = _); end

  # Generate a documentation file for each class and module
  def generate_class_files; end

  # Generate a documentation file for each file
  def generate_file_files; end

  # Generate an index page which lists all the classes which are documented.
  def generate_index; end

  # Generate a page file for `file`
  def generate_page(file); end

  # Generates the 404 page for the
  # [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) servlet
  def generate_servlet_not_found(message); end

  # Generates the servlet root page for the
  # [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) servlet
  def generate_servlet_root(installed); end

  # Generate an index page which lists all the classes which are documented.
  def generate_table_of_contents; end

  # Return a list of the documented modules sorted by salience first, then by
  # name.
  def get_sorted_module_list(classes); end

  # Try to extract Subversion information out of the first constant whose value
  # looks like a subversion Id tag. If no matching constant is found, and empty
  # hash is returned.
  def get_svninfo(klass); end

  def install_rdoc_static_file(source, destination, options); end

  # The [`JSON`](https://docs.ruby-lang.org/en/2.6.0/JSON.html) index generator
  # for this
  # [`Darkfish`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/Darkfish.html)
  # generator
  def json_index; end

  # Methods to be displayed by this generator
  def methods; end

  # Sorted list of classes and modules to be displayed by this generator
  def modsort; end

  # The output directory
  def outputdir; end

  # Renders the ERb contained in `file_name` relative to the template directory
  # and returns the result based on the current context.
  def render(file_name); end

  # Load and render the erb template in the given `template_file` and write it
  # out to `out_file`.
  #
  # Both `template_file` and `out_file` should be Pathname-like objects.
  #
  # An io will be yielded which must be captured by binding in the caller.
  def render_template(template_file, out_file = _); end

  # Prepares for generation of output from the current directory
  def setup; end

  # The [`RDoc::Store`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Store.html)
  # that is the source of the generated content
  def store; end

  def template_dir; end

  # Retrieves a cache template for `file`, if present, or fills the cache.
  def template_for(file, page = _, klass = _); end

  # Creates the result for `template` with `context`. If an error is raised a
  # [`Pathname`](https://docs.ruby-lang.org/en/2.6.0/Pathname.html)
  # `template_file` will indicate the file where the error occurred.
  def template_result(template, context, template_file); end

  # Return a string describing the amount of time in the given number of seconds
  # in terms a human can understand easily.
  def time_delta_string(seconds); end

  # Copy over the stylesheet into the appropriate place in the output directory.
  def write_style_sheet; end
end

# The
# [`JsonIndex`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/JsonIndex.html)
# generator is designed to complement an HTML generator and produces a
# [`JSON`](https://docs.ruby-lang.org/en/2.6.0/JSON.html) search index. This
# generator is derived from sdoc by Vladimir Kolesnikov and contains verbatim
# code written by him.
#
# This generator is designed to be used with a regular HTML generator:
#
# ```ruby
# class RDoc::Generator::Darkfish
#   def initialize options
#     # ...
#     @base_dir = Pathname.pwd.expand_path
#
#     @json_index = RDoc::Generator::JsonIndex.new self, options
#   end
#
#   def generate
#     # ...
#     @json_index.generate
#   end
# end
# ```
#
# ## Index Format
#
# The index is output as a
# [`JSON`](https://docs.ruby-lang.org/en/2.6.0/JSON.html) file assigned to the
# global variable `search_data`. The structure is:
#
# ```
# var search_data = {
#   "index": {
#     "searchIndex":
#       ["a", "b", ...],
#     "longSearchIndex":
#       ["a", "a::b", ...],
#     "info": [
#       ["A", "A", "A.html", "", ""],
#       ["B", "A::B", "A::B.html", "", ""],
#       ...
#     ]
#   }
# }
# ```
#
# The same item is described across the `searchIndex`, `longSearchIndex` and
# `info` fields. The `searchIndex` field contains the item's short name, the
# `longSearchIndex` field contains the full\_name (when appropriate) and the
# `info` field contains the item's name, full\_name, path, parameters and a
# snippet of the item's comment.
#
# ## [LICENSE](https://docs.ruby-lang.org/en/2.6.0/gems/2_6_0/gems/CFPropertyList-2_3_6/LICENSE.html)
#
# Copyright (c) 2009 Vladimir Kolesnikov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
class RDoc::Generator::JsonIndex
  include(::RDoc::Text)

  # Where the search index lives in the generated output
  SEARCH_INDEX_FILE = T.let(T.unsafe(nil), String)

  # Creates a new generator. `parent_generator` is used to determine the
  # [`class_dir`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/JsonIndex.html#method-i-class_dir)
  # and
  # [`file_dir`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/JsonIndex.html#method-i-file_dir)
  # of links in the output index.
  #
  # `options` are the same options passed to the parent generator.
  def self.new(parent_generator, options); end

  # Builds the [`JSON`](https://docs.ruby-lang.org/en/2.6.0/JSON.html) index as
  # a [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html).
  def build_index; end

  # The directory classes are written to
  def class_dir; end

  # Output progress information if debugging is enabled
  def debug_msg(*msg); end

  # The directory files are written to
  def file_dir; end

  # Writes the [`JSON`](https://docs.ruby-lang.org/en/2.6.0/JSON.html) index to
  # disk
  def generate; end

  # Compress the search\_index.js file using gzip
  def generate_gzipped; end

  def index; end

  # Adds classes and modules to the index
  def index_classes; end

  # Adds methods to the index
  def index_methods; end

  # Adds pages to the index
  def index_pages; end

  def reset(files, classes); end

  # Removes whitespace and downcases `string`
  def search_string(string); end
end

# Handle common
# [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup.html) tasks
# for various CodeObjects
#
# This module is loaded by generators. It allows RDoc's CodeObject tree to avoid
# loading generator code to improve startup time for `ri`.
module RDoc::Generator::Markup
  # Generates a relative URL from this object's path to `target_path`
  def aref_to(target_path); end

  # Generates a relative URL from `from_path` to this object's path
  def as_href(from_path); end

  # Build a webcvs URL starting for the given `url` with `full_path` appended as
  # the destination path. If `url` contains '%s' `full_path` will be will
  # replace the %s using sprintf on the `url`.
  def cvs_url(url, full_path); end

  # Handy wrapper for marking up this object's comment
  def description; end

  # Creates an
  # [`RDoc::Markup::ToHtmlCrossref`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToHtmlCrossref.html)
  # formatter
  def formatter; end
end

# Generates a
# [`POT`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/POT.html) file.
#
# Here is a translator work flow with the generator.
#
# ## Create .pot
#
# You create .pot file by pot formatter:
#
# ```
# % rdoc --format pot
# ```
#
# It generates doc/rdoc.pot.
#
# ## Create .po
#
# You create .po file from doc/rdoc.pot. This operation is needed only the first
# time. This work flow assumes that you are a translator for Japanese.
#
# You create locale/ja/rdoc.po from doc/rdoc.pot. You can use msginit provided
# by GNU gettext or rmsginit provided by gettext gem. This work flow uses
# gettext gem because it is more portable than GNU gettext for Rubyists. Gettext
# gem is implemented by pure Ruby.
#
# ```
# % gem install gettext
# % mkdir -p locale/ja
# % rmsginit --input doc/rdoc.pot --output locale/ja/rdoc.po --locale ja
# ```
#
# Translate messages in .po
#
# You translate messages in .po by a PO file editor. po-mode.el exists for Emacs
# users. There are some GUI tools such as GTranslator. There are some Web
# services such as POEditor and Tansifex. You can edit by your favorite text
# editor because .po is a text file. Generate localized documentation
#
# You can generate localized documentation with locale/ja/rdoc.po:
#
# ```
# % rdoc --locale ja
# ```
#
# You can find documentation in Japanese in doc/. Yay!
#
# ## Update translation
#
# You need to update translation when your application is added or modified
# messages.
#
# You can update .po by the following command lines:
#
# ```
# % rdoc --format pot
# % rmsgmerge --update locale/ja/rdoc.po doc/rdoc.pot
# ```
#
# You edit locale/ja/rdoc.po to translate new messages.
class RDoc::Generator::POT
  # Description of this generator
  DESCRIPTION = T.let(T.unsafe(nil), String)

  def self.new(store, options); end

  def class_dir; end

  # Writes .pot to disk.
  def generate; end
end

# Extracts message from
# [`RDoc::Store`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Store.html)
class RDoc::Generator::POT::MessageExtractor
  # Creates a message extractor for `store`.
  def self.new(store); end

  # Extracts messages from `store`, stores them into
  # [`RDoc::Generator::POT::PO`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/POT/PO.html)
  # and returns it.
  def extract; end
end

# Generates a
# [`PO`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/POT/PO.html) format
# text
class RDoc::Generator::POT::PO
  # Creates an object that represents
  # [`PO`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/POT/PO.html)
  # format.
  def self.new; end

  # Adds a
  # [`PO`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/POT/PO.html) entry
  # to the
  # [`PO`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/POT/PO.html).
  def add(entry); end

  # Returns
  # [`PO`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/POT/PO.html)
  # format text for the
  # [`PO`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Generator/POT/PO.html).
  def to_s; end
end

# A PO entry in PO
class RDoc::Generator::POT::POEntry
  # Creates a PO entry for `msgid`. Other valus can be specified by `options`.
  def self.new(msgid, options = _); end

  # The comment content extracted from source file
  def extracted_comment; end

  # The flags of the PO entry
  def flags; end

  # Merges the PO entry with `other_entry`.
  def merge(other_entry); end

  # The msgid content
  def msgid; end

  # The msgstr content
  def msgstr; end

  # The locations where the PO entry is extracted
  def references; end

  # Returns the PO entry in PO format.
  def to_s; end

  # The comment content created by translator (PO editor)
  def translator_comment; end
end

# Generates ri data files
class RDoc::Generator::RI
  # Description of this generator
  DESCRIPTION = T.let(T.unsafe(nil), String)

  def self.new(store, options); end

  # Writes the parsed data store to disk for use by ri.
  def generate; end
end

# [`GhostMethod`](https://docs.ruby-lang.org/en/2.7.0/RDoc/GhostMethod.html)
# represents a method referenced only by a comment
class RDoc::GhostMethod < ::RDoc::AnyMethod; end

# This module provides i18n related features.
module RDoc::I18n; end

# A message container for a locale.
#
# This object provides the following two features:
#
# ```
# * Loads translated messages from .po file.
# * Translates a message into the locale.
# ```
class RDoc::I18n::Locale
  # Creates a new locale object for `name` locale. `name` must follow IETF
  # language tag format.
  def self.new(name); end

  # Loads translation messages from `locale_directory`/+@name+/rdoc.po or
  # `locale_directory`/+@name+.po. The former has high priority.
  #
  # This method requires gettext gem for parsing .po file. If you don't have
  # gettext gem, this method doesn't load .po file. This method warns and
  # returns `false`.
  #
  # Returns `true` if succeeded, `false` otherwise.
  def load(locale_directory); end

  # The name of the locale. It uses IETF language tag format
  # +[[language](_territory)[.[codeset]](@modifier)]+.
  #
  # See also [BCP 47 - Tags for Identifying
  # Languages](http://tools.ietf.org/rfc/bcp/bcp47.txt).
  def name; end

  # Translates the `message` into locale. If there is no translation messages
  # for `message` in locale, `message` itself is returned.
  def translate(message); end

  # Returns the locale object for `locale_name`.
  def self.[](locale_name); end

  # Sets the locale object for `locale_name`.
  #
  # Normally, this method is not used. This method is useful for testing.
  def self.[]=(locale_name, locale); end
end

# An i18n supported text.
#
# This object provides the following two features:
#
# ```
# * Extracts translation messages from wrapped raw text.
# * Translates wrapped raw text in specified locale.
# ```
#
# Wrapped raw text is one of
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html),
# [`RDoc::Comment`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Comment.html) or
# [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of them.
class RDoc::I18n::Text
  # Creates a new i18n supported text for `raw` text.
  def self.new(raw); end

  # Extracts translation target messages and yields each message.
  #
  # Each yielded message is a
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html). It consists of the
  # followings:
  #
  # :type
  # :   :paragraph
  # :paragraph
  # :   [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) (The
  #     translation target message itself.)
  # :line\_no
  # :   [`Integer`](https://docs.ruby-lang.org/en/2.6.0/Integer.html) (The line
  #     number of the :paragraph is started.)
  #
  #
  # The above content may be added in the future.
  def extract_messages; end

  # Translates raw text into `locale`.
  def translate(locale); end
end

# A [`Module`](https://docs.ruby-lang.org/en/2.7.0/Module.html) included in a
# class with include
#
# ```ruby
# RDoc::Include.new 'Enumerable', 'comment ...'
# ```
class RDoc::Include < ::RDoc::Mixin; end

# [`RDoc::Markdown`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markdown.html) as
# described by the [markdown
# syntax](http://daringfireball.net/projects/markdown/syntax).
# To choose [`Markdown`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markdown.html)
# as your only default format
# see
# [Saved
# Options at
# `RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#class-RDoc::Options-label-Saved+Options)
# for instructions on setting up a
# `.doc_options`
# file
# to store your project default.
# ## Usage
# Here is a brief example of using this parse to read a markdown file by hand.
#
# ```ruby
# data = File.read("README.md")
# formatter = RDoc::Markup::ToHtml.new(RDoc::Options.new, nil)
# html = RDoc::Markdown.parse(data).accept(formatter)
#
# # do something with html
# ```
#
# ## Extensions
# The following markdown extensions are supported by the parser, but not
# all
# are
# used in [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) output by
# default.
# ### [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
# The [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
# [`Markdown`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markdown.html) parser
# has the following built-in behaviors that cannot
# be
# disabled.
# Underscores embedded in words are never interpreted as emphasis. (While
# the
# [markdown
# dingus](http://daringfireball.net/projects/markdown/dingus) emphasizes in-word
# underscores, neither
# the
# [`Markdown`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markdown.html)
# syntax nor MarkdownTest mention this behavior.)
# For HTML output, [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
# always auto-links bare URLs.
# ### Break on Newline
# The
# [`break_on_newline`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markdown.html#method-i-break_on_newline)
# extension converts all newlines into hard line
# breaks
# as
# in [Github Flavored
# Markdown](http://github.github.com/github-flavored-markdown/). This extension
# is disabled
# by
# default.
# ### CSS
# The
# [`css`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markdown.html#method-i-css)
# extension enables CSS blocks to be included in the output, but
# they
# are
# not used for any built-in
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) output format. This
# extension is
# disabled
# by
# default.
# Example:
#
# ```
# <style type="text/css">
# h1 { font-size: 3em }
# </style>
# ```
#
# ### Definition Lists
# The
# [`definition_lists`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markdown.html#method-i-definition_lists)
# extension allows definition lists using the [PHP Markdown Extra
# syntax](http://michelf.com/projects/php-markdown/extra/#def-list), but only
# one label and definition are
# supported
# at
# this time. This extension is enabled by default.
# Example:
#
# ```
# cat
# :   A small furry mammal
# that seems to sleep a lot
#
# ant
# :   A little insect that is known
# to enjoy picnics
# ```
#
# Produces:
# cat
# :   A small furry
#     mammal
# that
#     seems to sleep a lot
# ant
# :   A little insect that is
#     known
# to
#     enjoy picnics
#
# ### Strike
# Example:
#
# ```
# This is ~~striked~~.
# ```
#
# Produces:
# This is ~striked~.
# ### Github
# The
# [`github`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markdown.html#method-i-github)
# extension enables a partial set of [Github Flavored
# Markdown](http://github.github.com/github-flavored-markdown/). This extension
# is enabled by default.
# Supported github extensions include:
# #### Fenced code blocks
# Use ````` around a block of code instead of indenting it four spaces.
# #### Syntax highlighting
# Use ```` ruby` as the start of a code fence to add syntax
# highlighting.
# (Currently
# only `ruby` syntax is supported).
# ### HTML
# Enables raw HTML to be included in the output. This extension is enabled
# by
# default.
# Example:
#
# ```
# <table>
# ...
# </table>
# ```
#
# ### Notes
# The
# [`notes`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markdown.html#method-i-notes)
# extension enables footnote support. This extension is enabled
# by
# default.
# Example:
#
# ```
# Here is some text[^1] including an inline footnote ^[for short footnotes]
#
# ...
#
# [^1]: With the footnote text down at the bottom
# ```
#
# Produces:
# Here is some text[^1] including an inline footnote [^2]
# ## Limitations
# *   Link titles are not used
# *   Footnotes are collapsed into a single paragraph
#
# ## Author
# This markdown parser is a port to kpeg from
# [peg-markdown](https://github.com/jgm/peg-markdown)
# by
# John
# MacFarlane.
# It is used under the MIT license:
# Permission is hereby granted, free of charge, to any person obtaining a
# copy
# of
# this software and associated documentation files (the "Software"), to
# deal
# in
# the Software without restriction, including without limitation the
# rights
# to
# use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell
# copies
# of the Software, and to permit persons to whom the Software
# is
# furnished
# to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included
# in
# all
# copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR
# IMPLIED,
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY,
# FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE
# AUTHORS
# OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER
# LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM,
# OUT
# OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN
# THE
# SOFTWARE.
# The port to kpeg was performed by Eric Hodel and Evan Phoenix
# ---
# [^1]: With the footnote text down at the bottom
# [^2]: for short footnotes
class RDoc::Markdown
  # Extensions enabled by default
  DEFAULT_EXTENSIONS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Supported extensions
  EXTENSIONS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # HTML entity name map for
  # [`RDoc::Markdown`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markdown.html)
  HTML_ENTITIES = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  Rules = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # Creates a new markdown parser that enables the given `extensions`.
  #
  # Also aliased as: orig\_initialize
  def self.new(extensions = _, debug = _); end

  def _Alphanumeric; end

  def _AlphanumericAscii; end

  def _AtxHeading; end

  def _AtxInline; end

  def _AtxStart; end

  def _AutoLink; end

  def _AutoLinkEmail; end

  def _AutoLinkUrl; end

  def _BOM; end

  def _BlankLine; end

  def _Block; end

  def _BlockQuote; end

  def _BlockQuoteRaw; end

  def _Bullet; end

  def _BulletList; end

  def _CharEntity; end

  def _Code; end

  def _CodeFence; end

  def _DecEntity; end

  def _DefinitionList; end

  def _DefinitionListDefinition; end

  def _DefinitionListItem; end

  def _DefinitionListLabel; end

  def _Digit; end

  def _Doc; end

  def _Emph; end

  def _EmphStar; end

  def _EmphUl; end

  def _EmptyTitle; end

  def _Endline; end

  def _Entity; end

  def _Enumerator; end

  def _Eof; end

  def _EscapedChar; end

  def _ExplicitLink; end

  def _ExtendedSpecialChar; end

  def _Heading; end

  def _HexEntity; end

  def _HorizontalRule; end

  def _HtmlAnchor; end

  def _HtmlAttribute; end

  def _HtmlBlock; end

  def _HtmlBlockAddress; end

  def _HtmlBlockBlockquote; end

  def _HtmlBlockCenter; end

  def _HtmlBlockCloseAddress; end

  def _HtmlBlockCloseBlockquote; end

  def _HtmlBlockCloseCenter; end

  def _HtmlBlockCloseDd; end

  def _HtmlBlockCloseDir; end

  def _HtmlBlockCloseDiv; end

  def _HtmlBlockCloseDl; end

  def _HtmlBlockCloseDt; end

  def _HtmlBlockCloseFieldset; end

  def _HtmlBlockCloseForm; end

  def _HtmlBlockCloseFrameset; end

  def _HtmlBlockCloseH1; end

  def _HtmlBlockCloseH2; end

  def _HtmlBlockCloseH3; end

  def _HtmlBlockCloseH4; end

  def _HtmlBlockCloseH5; end

  def _HtmlBlockCloseH6; end

  def _HtmlBlockCloseHead; end

  def _HtmlBlockCloseLi; end

  def _HtmlBlockCloseMenu; end

  def _HtmlBlockCloseNoframes; end

  def _HtmlBlockCloseNoscript; end

  def _HtmlBlockCloseOl; end

  def _HtmlBlockCloseP; end

  def _HtmlBlockClosePre; end

  def _HtmlBlockCloseScript; end

  def _HtmlBlockCloseTable; end

  def _HtmlBlockCloseTbody; end

  def _HtmlBlockCloseTd; end

  def _HtmlBlockCloseTfoot; end

  def _HtmlBlockCloseTh; end

  def _HtmlBlockCloseThead; end

  def _HtmlBlockCloseTr; end

  def _HtmlBlockCloseUl; end

  def _HtmlBlockDd; end

  def _HtmlBlockDir; end

  def _HtmlBlockDiv; end

  def _HtmlBlockDl; end

  def _HtmlBlockDt; end

  def _HtmlBlockFieldset; end

  def _HtmlBlockForm; end

  def _HtmlBlockFrameset; end

  def _HtmlBlockH1; end

  def _HtmlBlockH2; end

  def _HtmlBlockH3; end

  def _HtmlBlockH4; end

  def _HtmlBlockH5; end

  def _HtmlBlockH6; end

  def _HtmlBlockHead; end

  def _HtmlBlockInTags; end

  def _HtmlBlockLi; end

  def _HtmlBlockMenu; end

  def _HtmlBlockNoframes; end

  def _HtmlBlockNoscript; end

  def _HtmlBlockOl; end

  def _HtmlBlockOpenAddress; end

  def _HtmlBlockOpenBlockquote; end

  def _HtmlBlockOpenCenter; end

  def _HtmlBlockOpenDd; end

  def _HtmlBlockOpenDir; end

  def _HtmlBlockOpenDiv; end

  def _HtmlBlockOpenDl; end

  def _HtmlBlockOpenDt; end

  def _HtmlBlockOpenFieldset; end

  def _HtmlBlockOpenForm; end

  def _HtmlBlockOpenFrameset; end

  def _HtmlBlockOpenH1; end

  def _HtmlBlockOpenH2; end

  def _HtmlBlockOpenH3; end

  def _HtmlBlockOpenH4; end

  def _HtmlBlockOpenH5; end

  def _HtmlBlockOpenH6; end

  def _HtmlBlockOpenHead; end

  def _HtmlBlockOpenLi; end

  def _HtmlBlockOpenMenu; end

  def _HtmlBlockOpenNoframes; end

  def _HtmlBlockOpenNoscript; end

  def _HtmlBlockOpenOl; end

  def _HtmlBlockOpenP; end

  def _HtmlBlockOpenPre; end

  def _HtmlBlockOpenScript; end

  def _HtmlBlockOpenTable; end

  def _HtmlBlockOpenTbody; end

  def _HtmlBlockOpenTd; end

  def _HtmlBlockOpenTfoot; end

  def _HtmlBlockOpenTh; end

  def _HtmlBlockOpenThead; end

  def _HtmlBlockOpenTr; end

  def _HtmlBlockOpenUl; end

  def _HtmlBlockP; end

  def _HtmlBlockPre; end

  def _HtmlBlockScript; end

  def _HtmlBlockSelfClosing; end

  def _HtmlBlockTable; end

  def _HtmlBlockTbody; end

  def _HtmlBlockTd; end

  def _HtmlBlockTfoot; end

  def _HtmlBlockTh; end

  def _HtmlBlockThead; end

  def _HtmlBlockTr; end

  def _HtmlBlockType; end

  def _HtmlBlockUl; end

  def _HtmlCloseAnchor; end

  def _HtmlComment; end

  def _HtmlOpenAnchor; end

  def _HtmlTag; end

  def _HtmlUnclosed; end

  def _HtmlUnclosedType; end

  def _Image; end

  def _InStyleTags; end

  def _Indent; end

  def _IndentedLine; end

  def _Inline; end

  def _InlineNote; end

  def _Inlines; end

  def _Label; end

  def _Line; end

  def _LineBreak; end

  def _Link; end

  def _ListBlock; end

  def _ListBlockLine; end

  def _ListContinuationBlock; end

  def _ListItem; end

  def _ListItemTight; end

  def _ListLoose; end

  def _ListTight; end

  def _Newline; end

  def _NonblankIndentedLine; end

  def _NonindentSpace; end

  def _Nonspacechar; end

  def _NormalChar; end

  def _NormalEndline; end

  def _Note; end

  def _NoteReference; end

  def _Notes; end

  def _OptionallyIndentedLine; end

  def _OrderedList; end

  def _Para; end

  def _Plain; end

  def _Quoted; end

  def _RawHtml; end

  def _RawLine; end

  def _RawNoteBlock; end

  def _RawNoteReference; end

  def _RefSrc; end

  def _RefTitle; end

  def _RefTitleDouble; end

  def _RefTitleParens; end

  def _RefTitleSingle; end

  def _Reference; end

  def _ReferenceLink; end

  def _ReferenceLinkDouble; end

  def _ReferenceLinkSingle; end

  def _References; end

  def _SetextBottom1; end

  def _SetextBottom2; end

  def _SetextHeading; end

  def _SetextHeading1; end

  def _SetextHeading2; end

  def _SkipBlock; end

  def _Source; end

  def _SourceContents; end

  def _Sp; end

  def _Space; end

  def _Spacechar; end

  def _SpecialChar; end

  def _Spnl; end

  def _StarLine; end

  def _StartList; end

  def _Str; end

  def _StrChunk; end

  def _Strike; end

  def _Strong; end

  def _StrongStar; end

  def _StrongUl; end

  def _StyleBlock; end

  def _StyleClose; end

  def _StyleOpen; end

  def _Symbol; end

  def _TerminalEndline; end

  def _Ticks1; end

  def _Ticks2; end

  def _Ticks3; end

  def _Ticks4; end

  def _Ticks5; end

  def _Title; end

  def _TitleDouble; end

  def _TitleSingle; end

  def _UlLine; end

  def _UlOrStarLine; end

  def _Verbatim; end

  def _VerbatimChunk; end

  def _Whitespace; end

  def _root; end

  def apply(rule); end

  def apply_with_args(rule, *args); end

  def break_on_newline=(enable); end

  def break_on_newline?; end

  def css=(enable); end

  def css?; end

  def current_column(target = _); end

  def current_line(target = _); end

  def definition_lists=(enable); end

  def definition_lists?; end

  # Wraps `text` in emphasis for rdoc inline formatting
  def emphasis(text); end

  # Enables or disables the extension with `name`
  def extension(name, enable); end

  # Is the extension `name` enabled?
  def extension?(name); end

  def external_invoke(other, rule, *args); end

  def failed_rule; end

  def failing_rule_offset; end

  def failure_caret; end

  def failure_character; end

  def failure_info; end

  def failure_oneline; end

  def get_byte; end

  def get_text(start); end

  def github=(enable); end

  def github?; end

  def grow_lr(rule, args, start_pos, m); end

  def html=(enable); end

  def html?; end

  def inner_parse(text); end

  def lines; end

  # Finds a link reference for `label` and creates a new link to it
  # with
  # `content`
  # as the link text. If `label` was not encountered in
  # the
  # reference-gathering
  # parser pass the label and content are
  # reconstructed
  # with
  # the linking `text` (usually whitespace).
  def link_to(content, label = _, text = _); end

  # Creates an RDoc::Markup::ListItem by parsing the `unparsed` content
  # from
  # the
  # first parsing pass.
  def list_item_from(unparsed); end

  def match_string(str); end

  # Stores `label` as a note and fills in previously unknown note references.
  def note(label); end

  # Creates a new link for the footnote `reference` and adds the reference
  # to
  # the
  # note order list for proper display at the end of the document.
  def note_for(ref); end

  def notes=(enable); end

  def notes?; end

  # Creates an RDoc::Markup::Paragraph from `parts` and
  # including
  # extension-specific
  # behavior
  def paragraph(parts); end

  # Parses `markdown` into an RDoc::Document
  #
  # Also aliased as:
  # [`peg_parse`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markdown.html#method-i-peg_parse)
  def parse(markdown); end

  # The internal kpeg parse method
  #
  # Alias for:
  # [`parse`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markdown.html#method-i-parse)
  def peg_parse(rule = _); end

  def pos; end

  def pos=(_); end

  def raise_error; end

  # Stores `label` as a reference to `link` and fills in previously
  # unknown
  # link
  # references.
  def reference(label, link); end

  def result; end

  def result=(_); end

  def scan(reg); end

  def set_failed_rule(name); end

  def set_string(string, pos); end

  def setup_foreign_grammar; end

  def setup_parser(str, debug = _); end

  def show_error(io = _); end

  def show_pos; end

  # Enables the strike extension
  def strike(text); end

  def strike=(enable); end

  def strike?; end

  def string; end

  # Wraps `text` in strong markup for rdoc inline formatting
  def strong(text); end

  # Creates extension methods for the `name` extension to enable and
  # disable
  # the
  # extension and to query if they are active.
  def self.extension(name); end

  # Parses the `markdown` document into an RDoc::Document using the
  # default
  # extensions.
  def self.parse(markdown); end

  def self.rule_info(name, rendered); end
end

class RDoc::Markdown::Literals
  def self.new(str, debug = _); end

  def _Alphanumeric; end

  def _AlphanumericAscii; end

  def _BOM; end

  def _Newline; end

  def _NonAlphanumeric; end

  def _Spacechar; end

  def apply(rule); end

  def apply_with_args(rule, *args); end

  def current_column(target = _); end

  def current_line(target = _); end

  def external_invoke(other, rule, *args); end

  def failed_rule; end

  def failing_rule_offset; end

  def failure_caret; end

  def failure_character; end

  def failure_info; end

  def failure_oneline; end

  def get_byte; end

  def get_text(start); end

  def grow_lr(rule, args, start_pos, m); end

  def lines; end

  def match_string(str); end

  def parse(rule = _); end

  def pos; end

  def pos=(_); end

  def raise_error; end

  def result; end

  def result=(_); end

  def scan(reg); end

  def set_failed_rule(name); end

  def set_string(string, pos); end

  def setup_foreign_grammar; end

  def setup_parser(str, debug = _); end

  def show_error(io = _); end

  def show_pos; end

  def string; end

  def self.rule_info(name, rendered); end
end

class RDoc::Markdown::Literals::MemoEntry
  def self.new(ans, pos); end

  def ans; end

  def left_rec; end

  def left_rec=(_); end

  def move!(ans, pos, result); end

  def pos; end

  def result; end

  def set; end
end

class RDoc::Markdown::Literals::ParseError < ::RuntimeError; end

class RDoc::Markdown::Literals::RuleInfo
  def self.new(name, rendered); end

  def name; end

  def rendered; end
end

RDoc::Markdown::Literals::Rules = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

class RDoc::Markdown::MemoEntry
  def self.new(ans, pos); end

  def ans; end

  def left_rec; end

  def left_rec=(_); end

  def move!(ans, pos, result); end

  def pos; end

  def result; end

  def set; end
end

class RDoc::Markdown::ParseError < ::RuntimeError; end

class RDoc::Markdown::RuleInfo
  def self.new(name, rendered); end

  def name; end

  def rendered; end
end

# [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html) parses
# plain text documents and attempts to decompose them into their constituent
# parts. Some of these parts are high-level: paragraphs, chunks of verbatim
# text, list entries and the like. Other parts happen at the character level: a
# piece of bold text, a word in code font. This markup is similar in spirit to
# that used on WikiWiki webs, where folks create web pages using a simple set of
# formatting rules.
#
# [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html) and
# other markup formats do no output formatting, this is handled by the
# RDoc::Markup::Formatter subclasses.
#
# # Supported Formats
#
# Besides the
# [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html) format,
# the following formats are built in to RDoc:
#
# markdown
# :   The markdown format as described by
#     http://daringfireball.net/projects/markdown/. See
#     [`RDoc::Markdown`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markdown.html)
#     for details on the parser and supported extensions.
# rd
# :   The rdtool format. See
#     [`RDoc::RD`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RD.html) for details
#     on the parser and format.
# tomdoc
# :   The TomDoc format as described by http://tomdoc.org/. See
#     [`RDoc::TomDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TomDoc.html) for
#     details on the parser and supported extensions.
#
#
# You can choose a markup format using the following methods:
#
# per project
# :   If you build your documentation with rake use
#     [`RDoc::Task#markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Task.html#attribute-i-markup).
#
#     If you build your documentation by hand run:
#
# ```
# rdoc --markup your_favorite_format --write-options
# ```
#
#     and commit `.rdoc_options` and ship it with your packaged gem.
# per file
# :   At the top of the file use the `:markup:` directive to set the default
#     format for the rest of the file.
# per comment
# :   Use the `:markup:` directive at the top of a comment you want to write in
#     a different format.
#
#
# # [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html)
#
# [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html) is
# extensible at runtime: you can add new markup elements to be recognized in the
# documents that
# [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html) parses.
#
# [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html) is
# intended to be the basis for a family of tools which share the common
# requirement that simple, plain-text should be rendered in a variety of
# different output formats and media. It is envisaged that
# [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html) could
# be the basis for formatting
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) style comment blocks,
# Wiki entries, and online FAQs.
#
# ## Synopsis
#
# This code converts `input_string` to HTML. The conversion takes place in the
# `convert` method, so you can use the same
# [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html)
# converter to convert multiple input strings.
#
# ```ruby
# require 'rdoc'
#
# h = RDoc::Markup::ToHtml.new(RDoc::Options.new)
#
# puts h.convert(input_string)
# ```
#
# You can extend the
# [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html) parser
# to recognize new markup sequences, and to add regexp handling. Here we make
# WikiWords significant to the parser, and also make the sequences {word} and
# <no>text...</no> signify strike-through text. We then subclass the HTML output
# class to deal with these:
#
# ```ruby
# require 'rdoc'
#
# class WikiHtml < RDoc::Markup::ToHtml
#   def handle_regexp_WIKIWORD(target)
#     "<font color=red>" + target.text + "</font>"
#   end
# end
#
# markup = RDoc::Markup.new
# markup.add_word_pair("{", "}", :STRIKE)
# markup.add_html("no", :STRIKE)
#
# markup.add_regexp_handling(/\b([A-Z][a-z]+[A-Z]\w+)/, :WIKIWORD)
#
# wh = WikiHtml.new RDoc::Options.new, markup
# wh.add_tag(:STRIKE, "<strike>", "</strike>")
#
# puts "<body>#{wh.convert ARGF.read}</body>"
# ```
#
# ## [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html)
#
# Where [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) support
# is available, [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) will
# automatically convert all documents to the same output encoding. The output
# encoding can be set via
# [`RDoc::Options#encoding`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#attribute-i-encoding)
# and defaults to
# [`Encoding.default_external`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html#method-c-default_external).
#
# # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) [`Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html) Reference
#
# ## Block [`Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html)
#
# ### Paragraphs and Verbatim
#
# The markup engine looks for a document's natural left margin. This is used as
# the initial margin for the document.
#
# Consecutive lines starting at this margin are considered to be a paragraph.
# Empty lines separate paragraphs.
#
# Any line that starts to the right of the current margin is treated as verbatim
# text. This is useful for code listings:
#
# ```ruby
# 3.times { puts "Ruby" }
# ```
#
# In verbatim text, two or more blank lines are collapsed into one, and trailing
# blank lines are removed:
#
# ```
# This is the first line
#
# This is the second non-blank line,
# after 2 blank lines in the source markup.
# ```
#
# There were two trailing blank lines right above this paragraph, that have been
# removed. In addition, the verbatim text has been shifted left, so the amount
# of indentation of verbatim text is unimportant.
#
# For HTML output [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) makes
# a small effort to determine if a verbatim section contains Ruby source code.
# If so, the verbatim block will be marked up as HTML. Triggers include "def",
# "class", "module", "require", the "hash rocket"# (=>) or a block call with a
# parameter.
#
# ### Headers
#
# A line starting with an equal sign (=) is treated as a heading. Level one
# headings have one equals sign, level two headings have two, and so on until
# level six, which is the maximum (seven hyphens or more result in a level six
# heading).
#
# For example, the above header was obtained with:
#
# ```
# === Headers
# ```
#
# In HTML output headers have an id matching their name. The above example's
# HTML is:
#
# ```
# <h3 id="label-Headers">Headers</h3>
# ```
#
# If a heading is inside a method body the id will be prefixed with the method's
# id. If the above header where in the documentation for a method such as:
#
# ```ruby
# ##
# # This method does fun things
# #
# # = Example
# #
# #   Example of fun things goes here ...
#
# def do_fun_things
# end
# ```
#
# The header's id would be:
#
# ```
# <h1 id="method-i-do_fun_things-label-Example">Example</h1>
# ```
#
# The label can be linked-to using `SomeClass@Headers`. See
# [Links](RDoc::Markup@Links) for further details.
#
# ### Rules
#
# A line starting with three or more hyphens (at the current indent) generates a
# horizontal rule.
#
# ```
# ---
# ```
#
# produces:
#
# ---
#
# ### Simple Lists
#
# If a paragraph starts with a "\*", "-", "<digit>." or "<letter>.", then it is
# taken to be the start of a list. The margin is increased to be the first
# non-space following the list start flag. Subsequent lines should be indented
# to this new margin until the list ends. For example:
#
# ```
# * this is a list with three paragraphs in
#   the first item.  This is the first paragraph.
#
#   And this is the second paragraph.
#
#   1. This is an indented, numbered list.
#   2. This is the second item in that list
#
#   This is the third conventional paragraph in the
#   first list item.
#
# * This is the second item in the original list
# ```
#
# produces:
#
# *   this is a list with three paragraphs in the first item. This is the first
#     paragraph.
#
#     And this is the second paragraph.
#
#     1.  This is an indented, numbered list.
#     2.  This is the second item in that list
#
#
#     This is the third conventional paragraph in the first list item.
#
# *   This is the second item in the original list
#
#
# ### Labeled Lists
#
# You can also construct labeled lists, sometimes called description or
# definition lists. Do this by putting the label in square brackets and
# indenting the list body:
#
# ```
# [cat]  a small furry mammal
#        that seems to sleep a lot
#
# [ant]  a little insect that is known
#        to enjoy picnics
# ```
#
# produces:
#
# cat
# :   a small furry mammal that seems to sleep a lot
#
# ant
# :   a little insect that is known to enjoy picnics
#
#
# If you want the list bodies to line up to the left of the labels, use two
# colons:
#
# ```ruby
# cat::  a small furry mammal
#        that seems to sleep a lot
#
# ant::  a little insect that is known
#        to enjoy picnics
# ```
#
# produces:
#
# cat
# :   a small furry mammal that seems to sleep a lot
#
# ant
# :   a little insect that is known to enjoy picnics
#
#
# Notice that blank lines right after the label are ignored in labeled lists:
#
# ```ruby
# [one]
#
#     definition 1
#
# [two]
#
#     definition 2
# ```
#
# produces the same output as
#
# ```
# [one]  definition 1
# [two]  definition 2
# ```
#
# ### Lists and Verbatim
#
# If you want to introduce a verbatim section right after a list, it has to be
# less indented than the list item bodies, but more indented than the list
# label, letter, digit or bullet. For instance:
#
# ```
# *   point 1
#
# *   point 2, first paragraph
#
#     point 2, second paragraph
#       verbatim text inside point 2
#     point 2, third paragraph
#   verbatim text outside of the list (the list is therefore closed)
# regular paragraph after the list
# ```
#
# produces:
#
# *   point 1
#
# *   point 2, first paragraph
#
#     point 2, second paragraph
#
# ```ruby
# verbatim text inside point 2
# ```
#
#     point 2, third paragraph
#
# ```ruby
# verbatim text outside of the list (the list is therefore closed)
# ```
#
# regular paragraph after the list
#
# ## Text [`Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html)
#
# ### Bold, Italic, Typewriter Text
#
# You can use markup within text (except verbatim) to change the appearance of
# parts of that text. Out of the box,
# [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html)
# supports word-based and general markup.
#
# Word-based markup uses flag characters around individual words:
#
# `*word*`
# :   displays *word* in a **bold** font
# `_word_`
# :   displays *word* in an *emphasized* font
# `+word+`
# :   displays *word* in a `code` font
#
#
# General markup affects text between a start delimiter and an end delimiter.
# Not surprisingly, these delimiters look like HTML markup.
#
# `<b>text</b>`
# :   displays *text* in a **bold** font
# `<em>text</em>`
# :   displays *text* in an *emphasized* font (alternate tag: `<i>`)
# `<tt>text</tt>`
# :   displays *text* in a `code` font (alternate tag: `<code>`)
#
#
# Unlike conventional Wiki markup, general markup can cross line boundaries. You
# can turn off the interpretation of markup by preceding the first character
# with a backslash (see *Escaping Text
# [`Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html)*, below).
#
# ### Links
#
# Links to starting with `http:`, `https:`, `mailto:`, `ftp:` or `www.` are
# recognized. An HTTP url that references an external image is converted into an
# inline image element.
#
# Classes and methods will be automatically linked to their definition. For
# example, `RDoc::Markup` will link to this documentation. By default methods
# will only be automatically linked if they contain an `_` (all methods can be
# automatically linked through the `--hyperlink-all` command line option).
#
# Single-word methods can be linked by using the `#` character for instance
# methods or `::` for class methods. For example, `#convert` links to
# [`convert`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html#method-i-convert).
# A class or method may be combined like `RDoc::Markup#convert`.
#
# A heading inside the documentation can be linked by following the class or
# method by an `@` then the heading name. `RDoc::Markup@Links` will link to this
# section like this: [Links at
# `RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html#class-RDoc::Markup-label-Links).
# Spaces in headings with multiple words must be escaped with `+` like
# `RDoc::Markup@Escaping+Text+Markup`. Punctuation and other special characters
# must be escaped like
# [`CGI.escape`](https://docs.ruby-lang.org/en/2.7.0/CGI/Util.html#method-i-escape).
#
# The `@` can also be used to link to sections. If a section and a heading share
# the same name the section is preferred for the link.
#
# Links can also be of the form `label[url]`, in which case `label` is used in
# the displayed text, and `url` is used as the target. If `label` contains
# multiple words, put it in braces: `{multi word label}[url]`. The `url` may be
# an `http:`-type link or a cross-reference to a class, module or method with a
# label.
#
# Links with the `rdoc-image:` scheme will create an image tag for HTML output.
# Only fully-qualified URLs are supported.
#
# Links with the `rdoc-ref:` scheme will link to the referenced class, module,
# method, file, etc. If the referenced item is does not exist no link will be
# generated and `rdoc-ref:` will be removed from the resulting text.
#
# Links starting with `rdoc-label:label_name` will link to the `label_name`. You
# can create a label for the current link (for bidirectional links) by supplying
# a name for the current link like `rdoc-label:label-other:label-mine.`
#
# Links starting with `link:` refer to local files whose path is relative to the
# `--op` directory. Use `rdoc-ref:` instead of `link:` to link to files
# generated by [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) as the
# link target may be different across
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) generators.
#
# Example links:
#
# ```
# https://github.com/ruby/rdoc
# mailto:user@example.com
# {RDoc Documentation}[http://rdoc.rubyforge.org]
# {RDoc Markup}[rdoc-ref:RDoc::Markup]
# ```
#
# ### Escaping Text [`Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html)
#
# Text markup can be escaped with a backslash, as in <tt>, which was obtained
# with `\<tt>`. Except in verbatim sections and between <tt> tags, to produce a
# backslash you have to double it unless it is followed by a space, tab or
# newline. Otherwise, the HTML formatter will discard it, as it is used to
# escape potential links:
#
# ```
# * The \ must be doubled if not followed by white space: \\.
# * But not in \<tt> tags: in a Regexp, <tt>\S</tt> matches non-space.
# * This is a link to {ruby-lang}[www.ruby-lang.org].
# * This is not a link, however: \{ruby-lang.org}[www.ruby-lang.org].
# * This will not be linked to \RDoc::RDoc#document
# ```
#
# generates:
#
# *   The \\ must be doubled if not followed by white space: \\.
# *   But not in <tt> tags: in a
#     [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html), `\S` matches
#     non-space.
# *   This is a link to [ruby-lang](www.ruby-lang.org)
# *   This is not a link, however: {ruby-lang.org}[www.ruby-lang.org]
# *   This will not be linked to
#     [`RDoc::RDoc#document`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html#method-i-document)
#
#
# Inside <tt> tags, more precisely, leading backslashes are removed only if
# followed by a markup character (`<*_+`), a backslash, or a known link
# reference (a known class or method). So in the example above, the backslash of
# `\S` would be removed if there was a class or module named `S` in the current
# context.
#
# This behavior is inherited from
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) version 1, and has
# been kept for compatibility with existing
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) documentation.
#
# ### Conversion of characters
#
# HTML will convert two/three dashes to an em-dash. Other common characters are
# converted as well:
#
# ```
# em-dash::  -- or ---
# ellipsis:: ...
#
# single quotes:: 'text' or `text'
# double quotes:: "text" or ``text''
#
# copyright:: (c)
# registered trademark:: (r)
# ```
#
# produces:
#
# em-dash
# :   -- or ---
# ellipsis
# :   ...
#
# single quotes
# :   'text' or 'text'
# double quotes
# :   "text" or "text"
#
# copyright
# :   (c)
# registered trademark
# :   (r)
#
#
# ## Documenting Source Code
#
# Comment blocks can be written fairly naturally, either using `#` on successive
# lines of the comment, or by including the comment in a `=begin`/`=end` block.
# If you use the latter form, the `=begin` line *must* be flagged with an `rdoc`
# tag:
#
# ```ruby
# =begin rdoc
# Documentation to be processed by RDoc.
#
# ...
# =end
# ```
#
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) stops processing
# comments if it finds a comment line starting with `--` right after the `#`
# character (otherwise, it will be treated as a rule if it has three dashes or
# more). This can be used to separate external from internal comments, or to
# stop a comment being associated with a method, class, or module. Commenting
# can be turned back on with a line that starts with `++`.
#
# ```ruby
# ##
# # Extract the age and calculate the date-of-birth.
# #--
# # FIXME: fails if the birthday falls on February 29th
# #++
# # The DOB is returned as a Time object.
#
# def get_dob(person)
#   # ...
# end
# ```
#
# Names of classes, files, and any method names containing an underscore or
# preceded by a hash character are automatically linked from comment text to
# their description. This linking works inside the current class or module, and
# with ancestor methods (in included modules or in the superclass).
#
# [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) parameter lists
# are extracted and displayed with the method description. If a method calls
# `yield`, then the parameters passed to yield will also be displayed:
#
# ```
# def fred
#   ...
#   yield line, address
# ```
#
# This will get documented as:
#
# ```
# fred() { |line, address| ... }
# ```
#
# You can override this using a comment containing ':yields: ...' immediately
# after the method definition
#
# ```
# def fred # :yields: index, position
#   # ...
#
#   yield line, address
# ```
#
# which will get documented as
#
# ```
# fred() { |index, position| ... }
# ```
#
# `:yields:` is an example of a documentation directive. These appear
# immediately after the start of the document element they are modifying.
#
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) automatically
# cross-references words with underscores or camel-case. To suppress
# cross-references, prefix the word with a \\ character. To include special
# characters like "`\n`", you'll need to use two \\ characters in normal text,
# but only one in <tt> text:
#
# ```ruby
# "\\n" or "<tt>\n</tt>"
# ```
#
# produces:
#
# "\n" or "`\n`"
#
# ## Directives
#
# Directives are keywords surrounded by ":" characters.
#
# ### Controlling what is documented
#
# `:nodoc:` / `:nodoc: all`
# :   This directive prevents documentation for the element from being
#     generated. For classes and modules, methods, aliases, constants, and
#     attributes directly within the affected class or module also will be
#     omitted. By default, though, modules and classes within that class or
#     module *will* be documented. This is turned off by adding the `all`
#     modifier.
#
# ```ruby
# module MyModule # :nodoc:
#   class Input
#   end
# end
#
# module OtherModule # :nodoc: all
#   class Output
#   end
# end
# ```
#
#     In the above code, only class `MyModule::Input` will be documented.
#
#     The `:nodoc:` directive, like `:enddoc:`, `:stopdoc:` and `:startdoc:`
#     presented below, is local to the current file: if you do not want to
#     document a module that appears in several files, specify `:nodoc:` on each
#     appearance, at least once per file.
#
# `:stopdoc:` / `:startdoc:`
# :   Stop and start adding new documentation elements to the current container.
#     For example, if a class has a number of constants that you don't want to
#     document, put a `:stopdoc:` before the first, and a `:startdoc:` after the
#     last. If you don't specify a `:startdoc:` by the end of the container,
#     disables documentation for the rest of the current file.
#
# `:doc:`
# :   Forces a method or attribute to be documented even if it wouldn't be
#     otherwise. Useful if, for example, you want to include documentation of a
#     particular private method.
#
# `:enddoc:`
# :   Document nothing further at the current level: directives `:startdoc:` and
#     `:doc:` that appear after this will not be honored for the current
#     container (file, class or module), in the current file.
#
# `:notnew:` / `:not_new:` / `:not-new:`
# :   Only applicable to the `initialize` instance method. Normally
#     [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) assumes that the
#     documentation and parameters for `initialize` are actually for the `new`
#     method, and so fakes out a `new` for the class. The `:notnew:` directive
#     stops this. Remember that `initialize` is private, so you won't see the
#     documentation unless you use the `-a` command line option.
#
#
# ### [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) arguments
#
# `:arg:` or `:args:` *parameters*
# :   Overrides the default argument handling with exactly these parameters.
#
# ```ruby
# ##
# #  :args: a, b
#
# def some_method(*a)
# end
# ```
#
# `:yield:` or `:yields:` *parameters*
# :   Overrides the default yield discovery with these parameters.
#
# ```ruby
# ##
# # :yields: key, value
#
# def each_thing &block
#   @things.each(&block)
# end
# ```
#
# `:call-seq:`
# :   Lines up to the next blank line or lines with a common prefix in the
#     comment are treated as the method's calling sequence, overriding the
#     default parsing of method parameters and yield arguments.
#
#     Multiple lines may be used.
#
# ```ruby
# # :call-seq:
# #   ARGF.readlines(sep=$/)     -> array
# #   ARGF.readlines(limit)      -> array
# #   ARGF.readlines(sep, limit) -> array
# #
# #   ARGF.to_a(sep=$/)     -> array
# #   ARGF.to_a(limit)      -> array
# #   ARGF.to_a(sep, limit) -> array
# #
# # The remaining lines are documentation ...
# ```
#
#
# ### Sections
#
# Sections allow you to group methods in a class into sensible containers. If
# you use the sections 'Public', 'Internal' and 'Deprecated' (the three allowed
# method statuses from TomDoc) the sections will be displayed in that order
# placing the most useful methods at the top. Otherwise, sections will be
# displayed in alphabetical order.
#
# `:category:` *section*
# :   Adds this item to the named `section` overriding the current section. Use
#     this to group methods by section in
#     [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) output while
#     maintaining a sensible ordering (like alphabetical).
#
# ```ruby
# # :category: Utility Methods
# #
# # CGI escapes +text+
#
# def convert_string text
#   CGI.escapeHTML text
# end
# ```
#
#     An empty category will place the item in the default category:
#
# ```ruby
# # :category:
# #
# # This method is in the default category
#
# def some_method
#   # ...
# end
# ```
#
#     Unlike the :section: directive, :category: is not sticky. The category
#     only applies to the item immediately following the comment.
#
#     Use the :section: directive to provide introductory text for a section of
#     documentation.
#
# `:section:` *title*
# :   Provides section introductory text in
#     [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) output. The title
#     following `:section:` is used as the section name and the remainder of the
#     comment containing the section is used as introductory text. A section's
#     comment block must be separated from following comment blocks. Use an
#     empty title to switch to the default section.
#
#     The :section: directive is sticky, so subsequent methods, aliases,
#     attributes, and classes will be contained in this section until the
#     section is changed. The :category: directive will override the :section:
#     directive.
#
#     A :section: comment block may have one or more lines before the :section:
#     directive. These will be removed, and any identical lines at the end of
#     the block are also removed. This allows you to add visual cues to the
#     section.
#
#     Example:
#
# ```ruby
# # ----------------------------------------
# # :section: My Section
# # This is the section that I wrote.
# # See it glisten in the noon-day sun.
# # ----------------------------------------
#
# ##
# # Comment for some_method
#
# def some_method
#   # ...
# end
# ```
#
#
# ### Other directives
#
# `:markup:` *type*
# :   Overrides the default markup type for this comment with the specified
#     markup type. For Ruby files, if the first comment contains this directive
#     it is applied automatically to all comments in the file.
#
#     Unless you are converting between markup formats you should use a
#     `.rdoc_options` file to specify the default documentation format for your
#     entire project. See [Saved Options at
#     `RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#class-RDoc::Options-label-Saved+Options)
#     for instructions.
#
#     At the top of a file the `:markup:` directive applies to the entire file:
#
# ```
# # coding: UTF-8
# # :markup: TomDoc
#
# # TomDoc comment here ...
#
# class MyClass
#   # ...
# ```
#
#     For just one comment:
#
# ```
#   # ...
# end
#
# # :markup: RDoc
# #
# # This is a comment in RDoc markup format ...
#
# def some_method
#   # ...
# ```
#
#     See [CONTRIBUTING at
#     `Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html#class-RDoc::Markup-label-CONTRIBUTING)
#     for instructions on adding a new markup format.
#
# `:include:` *filename*
# :   Include the contents of the named file at this point. This directive must
#     appear alone on one line, possibly preceded by spaces. In this position,
#     it can be escaped with a \\ in front of the first colon.
#
#     The file will be searched for in the directories listed by the `--include`
#     option, or in the current directory by default. The contents of the file
#     will be shifted to have the same indentation as the ':' at the start of
#     the `:include:` directive.
#
# `:title:` *text*
# :   Sets the title for the document. Equivalent to the `--title` command line
#     parameter. (The command line parameter overrides any :title: directive in
#     the source).
#
# `:main:` *name*
# :   Equivalent to the `--main` command line parameter.
class RDoc::Markup
  # Take a block of text and use various heuristics to determine its structure
  # (paragraphs, lists, and so on). Invoke an event handler as we identify
  # significant chunks.
  def self.new(attribute_manager = _); end

  # Add to the sequences recognized as general markup.
  def add_html(tag, name); end

  def add_special(pattern, name); end

  # Add to the sequences used to add formatting to an individual word (such as
  # **bold**). Matching entries will generate attributes that the output
  # formatters can recognize by their `name`.
  def add_word_pair(start, stop, name); end

  # An AttributeManager which handles inline markup.
  def attribute_manager; end

  # We take `input`, parse it if necessary, then invoke the output `formatter`
  # using a Visitor to render the result.
  def convert(input, formatter); end

  # Parses `str` into an RDoc::Markup::Document.
  def self.parse(str); end
end

# An
# [`AttrChanger`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/AttrChanger.html)
# records a change in attributes. It contains a bitmap of the attributes to turn
# on, and a bitmap of those to turn off.
class RDoc::Markup::AttrChanger < ::Struct
  Elem = type_member(:out)

  def inspect; end

  def to_s; end

  def turn_off; end

  def turn_off=(_); end

  def turn_on; end

  def turn_on=(_); end

  def self.[](*_); end

  def self.inspect; end

  def self.members; end

  def self.new(*_); end
end

# An array of attributes which parallels the characters in a string.
class RDoc::Markup::AttrSpan
  # Creates a new
  # [`AttrSpan`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/AttrSpan.html)
  # for `length` characters
  def self.new(length); end

  # Accesses flags for character `n`
  def [](n); end

  # Toggles `bits` from `start` to `length`
  def set_attrs(start, length, bits); end
end

# Manages changes of attributes in a block of text
class RDoc::Markup::AttributeManager
  A_PROTECT = T.let(T.unsafe(nil), Integer)

  # The NUL character
  NULL = T.let(T.unsafe(nil), String)

  PROTECT_ATTR = T.let(T.unsafe(nil), String)

  # Creates a new attribute manager that understands bold, emphasized and
  # teletype text.
  def self.new; end

  # Adds a markup class with `name` for words surrounded by HTML tag `tag`. To
  # process emphasis tags:
  #
  # ```ruby
  # am.add_html 'em', :EM
  # ```
  def add_html(tag, name); end

  def add_special(pattern, name); end

  # Adds a markup class with `name` for words wrapped in the `start` and `stop`
  # character. To make words wrapped with "\*" bold:
  #
  # ```ruby
  # am.add_word_pair '*', '*', :BOLD
  # ```
  def add_word_pair(start, stop, name); end

  # Return an attribute object with the given turn\_on and turn\_off bits set
  def attribute(turn_on, turn_off); end

  # The attributes enabled for this markup object.
  def attributes; end

  # Changes the current attribute from `current` to `new`
  def change_attribute(current, new); end

  # Used by the tests to change attributes by name from `current_set` to
  # `new_set`
  def changed_attribute_by_name(current_set, new_set); end

  # Map attributes like **text**to the sequence 001002<char>001003<char>, where
  # <char> is a per-attribute specific character
  def convert_attrs(str, attrs); end

  # Converts HTML tags to
  # [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) attributes
  def convert_html(str, attrs); end

  def convert_specials(str, attrs); end

  # Copies `start_pos` to `end_pos` from the current string
  def copy_string(start_pos, end_pos); end

  # Debug method that prints a string along with its attributes
  def display_attributes; end

  # Processes `str` converting attributes, HTML and regexp handlings
  def flow(str); end

  # This maps HTML tags to the corresponding attribute char
  def html_tags; end

  # Escapes regexp handling sequences of text to prevent conversion to
  # [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html)
  def mask_protected_sequences; end

  # This maps delimiters that occur around words (such as **bold** or `tt`)
  # where the start and end delimiters and the same. This lets us optimize the
  # regexp
  def matching_word_pairs; end

  # A \\ in front of a character that would normally be processed turns off
  # processing. We do this by turning < into <#{PROTECT}
  def protectable; end

  def special; end

  # Splits the string into chunks by attribute change
  def split_into_flow; end

  # Unescapes regexp handling sequences of text
  def unmask_protected_sequences; end

  # And this is used when the delimiters aren't the same. In this case the hash
  # maps a pattern to the attribute character
  def word_pair_map; end
end

# We manage a set of attributes. Each attribute has a symbol name and a bit
# value.
class RDoc::Markup::Attributes
  # Creates a new attributes set.
  def self.new; end

  # Returns a string representation of `bitmap`
  def as_string(bitmap); end

  # Returns a unique bit for `name`
  def bitmap_for(name); end

  # yields each attribute name in `bitmap`
  def each_name_of(bitmap); end

  def special; end
end

# An empty line. This class is a singleton.
class RDoc::Markup::BlankLine
  # Calls accept\_blank\_line on `visitor`
  def accept(visitor); end

  def pretty_print(q); end

  # [`RDoc::Markup::BlankLine`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/BlankLine.html)
  # is a singleton
  def self.new; end
end

# A quoted section which contains markup items.
class RDoc::Markup::BlockQuote < ::RDoc::Markup::Raw
  # Calls accept\_block\_quote on `visitor`
  def accept(visitor); end
end

# A [`Document`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Document.html)
# containing lists, headings, paragraphs, etc.
class RDoc::Markup::Document
  include(::Enumerable)

  Elem = type_member(:out) {{fixed: T.untyped}}

  # Creates a new
  # [`Document`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Document.html)
  # with `parts`
  def self.new(*parts); end

  # Appends `part` to the document
  def <<(part); end

  def ==(other); end

  # Runs this document and all its items through `visitor`
  def accept(visitor); end

  # Concatenates the given `parts` onto the document
  def concat(parts); end

  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) for the
  # parts of this document
  def each(&block); end

  # Does this document have no parts?
  def empty?; end

  # The file this document was created from. See also
  # [`RDoc::ClassModule#add_comment`](https://docs.ruby-lang.org/en/2.6.0/RDoc/ClassModule.html#method-i-add_comment)
  def file; end

  # The file this
  # [`Document`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Document.html)
  # was created from.
  def file=(location); end

  # When this is a collection of documents (#file is not set and this document
  # contains only other documents as its direct children)
  # [`merge`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Document.html#method-i-merge)
  # replaces documents in this class with documents from `other` when the file
  # matches and adds documents from `other` when the files do not.
  #
  # The information in `other` is preferred over the receiver
  def merge(other); end

  # Does this
  # [`Document`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Document.html)
  # contain other Documents?
  def merged?; end

  # If a heading is below the given level it will be omitted from the
  # [`table_of_contents`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Document.html#method-i-table_of_contents)
  def omit_headings_below; end

  # If a heading is below the given level it will be omitted from the
  # [`table_of_contents`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Document.html#method-i-table_of_contents)
  def omit_headings_below=(_); end

  # The parts of the
  # [`Document`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Document.html)
  def parts; end

  def pretty_print(q); end

  # Appends `parts` to the document
  def push(*parts); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  # headings in the document.
  #
  # Require 'rdoc/markup/formatter' before calling this method.
  def table_of_contents; end
end

# Base class for [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) markup
# formatters
#
# Formatters are a visitor that converts an
# [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup.html) tree
# (from a comment) into some kind of output.
# [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) ships with formatters
# for converting back to rdoc, ANSI text, HTML, a Table of Contents and other
# formats.
#
# If you'd like to write your own
# [`Formatter`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Formatter.html)
# use
# [`RDoc::Markup::FormatterTestCase`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/FormatterTestCase.html).
# If you're writing a text-output formatter use
# [`RDoc::Markup::TextFormatterTestCase`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/TextFormatterTestCase.html)
# which provides extra test cases.
class RDoc::Markup::Formatter
  # Creates a new
  # [`Formatter`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Formatter.html)
  def self.new(options, markup = _); end

  # Adds `document` to the output
  def accept_document(document); end

  def add_special_RDOCLINK; end

  def add_special_TIDYLINK; end

  # Add a new set of tags for an attribute. We allow separate start and end tags
  # for flexibility
  def add_tag(name, start, stop); end

  # Allows `tag` to be decorated with additional information.
  def annotate(tag); end

  # Marks up `content`
  def convert(content); end

  # Converts flow items `flow`
  def convert_flow(flow); end

  def convert_special(special); end

  # Converts a string to be fancier if desired
  def convert_string(string); end

  # Use ignore in your subclass to ignore the content of a node.
  #
  # ```ruby
  # ##
  # # We don't support raw nodes in ToNoRaw
  #
  # alias accept_raw ignore
  # ```
  def ignore(*node); end

  # Are we currently inside tt tags?
  def in_tt?; end

  # Turns off tags for `item` on `res`
  def off_tags(res, item); end

  # Turns on tags for `item` on `res`
  def on_tags(res, item); end

  # Extracts and a scheme, url and an anchor id from `url` and returns them.
  def parse_url(url); end

  # Is `tag` a tt tag?
  def tt?(tag); end

  # Converts a target url to one that is relative to a given path
  def self.gen_relative_url(path, target); end
end

# Tag for inline markup containing a `bit` for the bitmask and the `on` and
# `off` triggers.
class RDoc::Markup::Formatter::InlineTag < ::Struct
  Elem = type_member(:out)

  def bit; end

  def bit=(_); end

  def off; end

  def off=(_); end

  def on; end

  def on=(_); end

  def self.[](*_); end

  def self.inspect; end

  def self.members; end

  def self.new(*_); end
end

# A hard-break in the middle of a paragraph.
class RDoc::Markup::HardBreak
  def ==(other); end

  # Calls accept\_hard\_break on `visitor`
  def accept(visitor); end

  def pretty_print(q); end

  # [`RDoc::Markup::HardBreak`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/HardBreak.html)
  # is a singleton
  def self.new; end
end

class RDoc::Markup::Heading < ::Struct
  Elem = type_member(:out)

  def accept(visitor); end

  def aref; end

  def label(context = _); end

  def level; end

  def level=(_); end

  def plain_html; end

  def pretty_print(q); end

  def text; end

  def text=(_); end

  def self.[](*_); end

  def self.inspect; end

  def self.members; end

  def self.new(*_); end

  def self.to_html; end

  def self.to_label; end
end

# A file included at generation time. Objects of this class are created by
# [`RDoc::RD`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RD.html) for an
# extension-less include.
#
# This implementation in incomplete.
class RDoc::Markup::Include
  # Creates a new include that will import `file` from `include_path`
  def self.new(file, include_path); end

  def ==(other); end

  # The filename to be included, without extension
  def file; end

  # Directories to search for
  # [`file`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Include.html#attribute-i-file)
  def include_path; end

  def pretty_print(q); end
end

# An Indented Paragraph of text
class RDoc::Markup::IndentedParagraph < ::RDoc::Markup::Raw
  # Creates a new
  # [`IndentedParagraph`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/IndentedParagraph.html)
  # containing `parts` indented with `indent` spaces
  def self.new(indent, *parts); end

  def ==(other); end

  # Calls accept\_indented\_paragraph on `visitor`
  def accept(visitor); end

  # The indent in number of spaces
  def indent; end

  # Joins the raw paragraph text and converts inline HardBreaks to the
  # `hard_break` text followed by the indent.
  def text(hard_break = _); end
end

# A [`List`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/List.html) is a
# homogeneous set of ListItems.
#
# The supported list types include:
#
# :BULLET
# :   An unordered list
# :LABEL
# :   An unordered definition list, but using an alternate
#     [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup.html)
#     syntax
# :LALPHA
# :   An ordered list using increasing lowercase
#     [`English`](https://docs.ruby-lang.org/en/2.6.0/English.html) letters
# :NOTE
# :   An unordered definition list
# :NUMBER
# :   An ordered list using increasing Arabic numerals
# :UALPHA
# :   An ordered list using increasing uppercase
#     [`English`](https://docs.ruby-lang.org/en/2.6.0/English.html) letters
#
#
# Definition lists behave like HTML definition lists. Each list item can
# describe multiple terms. See
# [`RDoc::Markup::ListItem`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ListItem.html)
# for how labels and definition are stored as list items.
class RDoc::Markup::List
  # Creates a new list of `type` with `items`. Valid list types are: `:BULLET`,
  # `:LABEL`, `:LALPHA`, `:NOTE`, `:NUMBER`, `:UALPHA`
  def self.new(type = _, *items); end

  # Appends `item` to the list
  def <<(item); end

  def ==(other); end

  # Runs this list and all its
  # [`items`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/List.html#attribute-i-items)
  # through `visitor`
  def accept(visitor); end

  # Is the list empty?
  def empty?; end

  # Items in the list
  def items; end

  # Returns the last item in the list
  def last; end

  def pretty_print(q); end

  # Appends `items` to the list
  def push(*items); end

  # The list's type
  def type; end

  # The list's type
  def type=(_); end
end

# An item within a List that contains paragraphs, headings, etc.
#
# For BULLET, NUMBER, LALPHA and UALPHA lists, the label will always be nil. For
# NOTE and LABEL lists, the list label may contain:
#
# *   a single [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) for a
#     single label
# *   an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of Strings
#     for a list item with multiple terms
# *   nil for an extra description attached to a previously labeled list item
class RDoc::Markup::ListItem
  # Creates a new
  # [`ListItem`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ListItem.html)
  # with an optional `label` containing `parts`
  def self.new(label = _, *parts); end

  # Appends `part` to the
  # [`ListItem`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ListItem.html)
  def <<(part); end

  def ==(other); end

  # Runs this list item and all its
  # [`parts`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ListItem.html#attribute-i-parts)
  # through `visitor`
  def accept(visitor); end

  # Is the
  # [`ListItem`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ListItem.html)
  # empty?
  def empty?; end

  # The label for the
  # [`ListItem`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ListItem.html)
  def label; end

  # The label for the
  # [`ListItem`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ListItem.html)
  def label=(_); end

  # Length of parts in the
  # [`ListItem`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ListItem.html)
  def length; end

  # Parts of the
  # [`ListItem`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ListItem.html)
  def parts; end

  def pretty_print(q); end

  # Adds `parts` to the
  # [`ListItem`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ListItem.html)
  def push(*parts); end
end

# A
# [`Paragraph`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Paragraph.html)
# of text
class RDoc::Markup::Paragraph < ::RDoc::Markup::Raw
  # Calls accept\_paragraph on `visitor`
  def accept(visitor); end

  # Joins the raw paragraph text and converts inline HardBreaks to the
  # `hard_break` text.
  def text(hard_break = _); end
end

# A recursive-descent parser for
# [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) markup.
#
# The parser tokenizes an input string then parses the tokens into a Document.
# Documents can be converted into output formats by writing a visitor like
# RDoc::Markup::ToHTML.
#
# The parser only handles the block-level constructs Paragraph, List, ListItem,
# Heading, Verbatim, BlankLine, Rule and BlockQuote. Inline markup such as
# `+blah+` is handled separately by
# [`RDoc::Markup::AttributeManager`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/AttributeManager.html).
#
# To see what markup the
# [`Parser`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Parser.html)
# implements read [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html). To
# see how to use [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) markup
# to format text in your program read
# [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup.html).
class RDoc::Markup::Parser
  include(::RDoc::Text)

  # List token types
  LIST_TOKENS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Creates a new
  # [`Parser`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Parser.html). See
  # also
  # [`::parse`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Parser.html#method-c-parse)
  def self.new; end

  # Builds a Heading of `level`
  def build_heading(level); end

  # Builds a List flush to `margin`
  def build_list(margin); end

  # Builds a Paragraph that is flush to `margin`
  def build_paragraph(margin); end

  # Builds a Verbatim that is indented from `margin`.
  #
  # The verbatim block is shifted left (the least indented lines start in column
  # 0). Each part of the verbatim is one line of text, always terminated by a
  # newline. Blank lines always consist of a single newline character, and there
  # is never a single newline at the end of the verbatim.
  def build_verbatim(margin); end

  # The character offset for the input string at the given `byte_offset`
  def char_pos(byte_offset); end

  # Enables display of debugging information
  def debug; end

  # Enables display of debugging information
  def debug=(_); end

  # Pulls the next token from the stream.
  def get; end

  # Parses the tokens into an array of RDoc::Markup::XXX objects, and appends
  # them to the passed `parent` RDoc::Markup::YYY object.
  #
  # Exits at the end of the token stream, or when it encounters a token in a
  # column less than `indent` (unless it is a NEWLINE).
  #
  # Returns `parent`.
  def parse(parent, indent = _); end

  def parse_text(parent, indent); end

  # Returns the next token on the stream without modifying the stream
  def peek_token; end

  # Creates the
  # [`StringScanner`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html)
  def setup_scanner(input); end

  # Skips the next token if its type is `token_type`.
  #
  # Optionally raises an error if the next token is not of the expected type.
  def skip(token_type, error = _); end

  # Calculates the column (by character) and line of the current token based on
  # `byte_offset`.
  def token_pos(byte_offset); end

  # Turns text `input` into a stream of tokens
  def tokenize(input); end

  # Token accessor
  def tokens; end

  # Returns the current token to the token stream
  def unget; end

  # Parses `str` into a Document.
  #
  # Use RDoc::Markup#parse instead of this method.
  def self.parse(str); end

  # Returns a token stream for `str`, for testing
  def self.tokenize(str); end
end

# [`Parser`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Parser.html) error
# subclass
class RDoc::Markup::Parser::Error < ::RuntimeError; end

# Raised when the parser is unable to handle the given markup
class RDoc::Markup::Parser::ParseError < ::RDoc::Markup::Parser::Error; end

# Handle common directives that can occur in a block of text:
#
# ```
# :include: filename
# ```
#
# Directives can be escaped by preceding them with a backslash.
#
# [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) plugin authors can
# register additional directives to be handled by using
# [`RDoc::Markup::PreProcess::register`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/PreProcess.html#method-c-register).
#
# Any directive that is not built-in to
# [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) (including those
# registered via plugins) will be stored in the metadata hash on the CodeObject
# the comment is attached to. See [Directives at
# `RDoc::Markup`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup.html#label-Directives)
# for the list of built-in directives.
class RDoc::Markup::PreProcess
  # Creates a new pre-processor for `input_file_name` that will look for
  # included files in `include_path`
  def self.new(input_file_name, include_path); end

  # Look for the given file in the directory containing the current file, and
  # then in each of the directories specified in the RDOC\_INCLUDE path
  def find_include_file(name); end

  # Look for directives in the given `text`.
  #
  # Options that we don't handle are yielded. If the block returns false the
  # directive is restored to the text. If the block returns nil or no block was
  # given the directive is handled according to the registered directives. If a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) was returned the
  # directive is replaced with the string.
  #
  # If no matching directive was registered the directive is restored to the
  # text.
  #
  # If `code_object` is given and the directive is unknown then the directive's
  # parameter is set as metadata on the `code_object`. See
  # [`RDoc::CodeObject#metadata`](https://docs.ruby-lang.org/en/2.6.0/RDoc/CodeObject.html#attribute-i-metadata)
  # for details.
  def handle(text, code_object = _, &block); end

  # Performs the actions described by `directive` and its parameter `param`.
  #
  # `code_object` is used for directives that operate on a class or module.
  # `prefix` is used to ensure the replacement for handled directives is
  # correct. `encoding` is used for the `include` directive.
  #
  # For a list of directives in
  # [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) see
  # [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup.html).
  def handle_directive(prefix, directive, param, code_object = _, encoding = _); end

  # Handles the `:include: filename` directive.
  #
  # If the first line of the included file starts with '#', and contains an
  # encoding information in the form 'coding:' or 'coding=', it is removed.
  #
  # If all lines in the included file start with a '#', this leading '#' is
  # removed before inclusion. The included content is indented like the
  # `:include:` directive.
  def include_file(name, indent, encoding); end

  # An [`RDoc::Options`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Options.html)
  # instance that will be filled in with overrides from directives
  def options; end

  # An [`RDoc::Options`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Options.html)
  # instance that will be filled in with overrides from directives
  def options=(_); end

  # Adds a post-process handler for directives. The handler will be called with
  # the result
  # [`RDoc::Comment`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Comment.html) (or
  # text [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html)) and the
  # code object for the comment (if any).
  def self.post_process(&block); end

  # Registered post-processors
  def self.post_processors; end

  # Registers `directive` as one handled by
  # [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html). If a block is given
  # the directive will be replaced by the result of the block, otherwise the
  # directive will be removed from the processed text.
  #
  # The block will be called with the directive name and the directive
  # parameter:
  #
  # ```ruby
  # RDoc::Markup::PreProcess.register 'my-directive' do |directive, param|
  #   # replace text, etc.
  # end
  # ```
  def self.register(directive, &block); end

  # Registered directives
  def self.registered; end

  # Clears all registered directives and post-processors
  def self.reset; end
end

# A section of text that is added to the output document as-is
class RDoc::Markup::Raw
  # Creates a new
  # [`Raw`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Raw.html) containing
  # `parts`
  def self.new(*parts); end

  # Appends `text`
  def <<(text); end

  def ==(other); end

  # Calls accept\_raw+ on `visitor`
  def accept(visitor); end

  # Appends `other`'s parts
  def merge(other); end

  # The component parts of the list
  def parts; end

  def pretty_print(q); end

  # Appends `texts` onto this Paragraph
  def push(*texts); end

  # The raw text
  def text; end
end

# A horizontal rule with a weight
class RDoc::Markup::Rule < ::Struct
  Elem = type_member(:out)

  # Calls accept\_rule on `visitor`
  def accept(visitor); end

  def pretty_print(q); end
end

class RDoc::Markup::Special
  def self.new(type, text); end

  def ==(o); end

  def inspect; end

  def text; end

  def text=(_); end

  def to_s; end

  def type; end
end

# Outputs [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) markup with
# vibrant ANSI color!
class RDoc::Markup::ToAnsi < ::RDoc::Markup::ToRdoc
  # Creates a new
  # [`ToAnsi`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToAnsi.html)
  # visitor that is ready to output vibrant ANSI color!
  def self.new(markup = _); end

  # Overrides indent width to ensure output lines up correctly.
  def accept_list_item_end(list_item); end

  # Adds coloring to note and label list items
  def accept_list_item_start(list_item); end

  # Maps attributes to ANSI sequences
  def init_tags; end

  # Starts accepting with a reset screen
  def start_accepting; end
end

# Outputs [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) markup with
# hot backspace action!  You will probably need a pager to use this output
# format.
#
# This formatter won't work on 1.8.6 because it lacks
# [`String#chars`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-chars).
class RDoc::Markup::ToBs < ::RDoc::Markup::ToRdoc
  # Returns a new
  # [`ToBs`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToBs.html) that is
  # ready for hot backspace action!
  def self.new(markup = _); end

  # Makes heading text bold.
  def accept_heading(heading); end

  # Turns on or off regexp handling for `convert_string`
  def annotate(tag); end

  def convert_special(special); end

  # Adds bold or underline mixed with backspaces
  def convert_string(string); end

  # Sets a flag that is picked up by
  # [`annotate`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToBs.html#method-i-annotate)
  # to do the right thing in
  # [`convert_string`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToBs.html#method-i-convert_string)
  def init_tags; end
end

# Outputs [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) markup as
# HTML.
class RDoc::Markup::ToHtml < ::RDoc::Markup::Formatter
  include(::RDoc::Text)

  # Maps RDoc::Markup::Parser::LIST\_TOKENS types to HTML tags
  LIST_TYPE_TO_HTML = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # Creates a new formatter that will output HTML
  def self.new(options, markup = _); end

  # Adds `blank_line` to the output
  def accept_blank_line(blank_line); end

  # Adds `block_quote` to the output
  def accept_block_quote(block_quote); end

  # Adds `heading` to the output. The headings greater than 6 are trimmed to
  # level 6.
  def accept_heading(heading); end

  # Finishes consumption of `list`
  def accept_list_end(list); end

  # Finishes consumption of `list_item`
  def accept_list_item_end(list_item); end

  # Prepares the visitor for consuming `list_item`
  def accept_list_item_start(list_item); end

  # Prepares the visitor for consuming `list`
  def accept_list_start(list); end

  # Adds `paragraph` to the output
  def accept_paragraph(paragraph); end

  # Adds `raw` to the output
  def accept_raw(raw); end

  # Adds `rule` to the output
  def accept_rule(rule); end

  # Adds `verbatim` to the output
  def accept_verbatim(verbatim); end

  # The
  # [`RDoc::CodeObject`](https://docs.ruby-lang.org/en/2.6.0/RDoc/CodeObject.html)
  # HTML is being generated for. This is used to generate namespaced
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) fragments
  def code_object; end

  # The
  # [`RDoc::CodeObject`](https://docs.ruby-lang.org/en/2.6.0/RDoc/CodeObject.html)
  # HTML is being generated for. This is used to generate namespaced
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) fragments
  def code_object=(_); end

  # CGI-escapes `text`
  def convert_string(text); end

  # Returns the generated output
  def end_accepting; end

  # Path to this document for relative links
  def from_path; end

  # Path to this document for relative links
  def from_path=(_); end

  # Generate a link to `url` with content `text`. Handles the special cases for
  # img: and link: described under
  # [`handle_regexp_HYPERLINK`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToHtml.html#method-i-handle_regexp_HYPERLINK)
  def gen_url(url, text); end

  def handle_RDOCLINK(url); end

  def handle_special_HARD_BREAK(special); end

  def handle_special_HYPERLINK(special); end

  def handle_special_RDOCLINK(special); end

  def handle_special_TIDYLINK(special); end

  # Determines the HTML list element for `list_type` and `open_tag`
  def html_list_name(list_type, open_tag); end

  def in_list_entry; end

  # Maps attributes to HTML tags
  def init_tags; end

  def list; end

  # Returns the HTML end-tag for `list_type`
  def list_end_for(list_type); end

  # Returns the HTML tag for `list_type`, possible using a label from
  # `list_item`
  def list_item_start(list_item, list_type); end

  # Returns true if text is valid ruby syntax
  def parseable?(text); end

  def res; end

  # Prepares the visitor for HTML generation
  def start_accepting; end

  # Converts `item` to HTML using
  # [`RDoc::Text#to_html`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Text.html#method-i-to_html)
  def to_html(item); end
end

# Subclass of the
# [`RDoc::Markup::ToHtml`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToHtml.html)
# class that supports looking up method names, classes, etc to create links.
# [`RDoc::CrossReference`](https://docs.ruby-lang.org/en/2.6.0/RDoc/CrossReference.html)
# is used to generate those links based on the current context.
class RDoc::Markup::ToHtmlCrossref < ::RDoc::Markup::ToHtml
  ALL_CROSSREF_REGEXP = T.let(T.unsafe(nil), Regexp)

  CLASS_REGEXP_STR = T.let(T.unsafe(nil), String)

  CROSSREF_REGEXP = T.let(T.unsafe(nil), Regexp)

  METHOD_REGEXP_STR = T.let(T.unsafe(nil), String)

  # Creates a new crossref resolver that generates links relative to `context`
  # which lives at `from_path` in the generated files. '#' characters on
  # references are removed unless `show_hash` is true. Only method names
  # preceded by '#' or '::' are linked, unless `hyperlink_all` is true.
  def self.new(options, from_path, context, markup = _); end

  # [`RDoc::CodeObject`](https://docs.ruby-lang.org/en/2.6.0/RDoc/CodeObject.html)
  # for generating references
  def context; end

  # [`RDoc::CodeObject`](https://docs.ruby-lang.org/en/2.6.0/RDoc/CodeObject.html)
  # for generating references
  def context=(_); end

  # Creates a link to the reference `name` if the name exists. If `text` is
  # given it is used as the link text, otherwise `name` is used.
  def cross_reference(name, text = _); end

  # Generates links for `rdoc-ref:` scheme URLs and allows
  # [`RDoc::Markup::ToHtml`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToHtml.html)
  # to handle other schemes.
  def gen_url(url, text); end

  def handle_special_CROSSREF(special); end

  def handle_special_HYPERLINK(special); end

  def handle_special_RDOCLINK(special); end

  # Creates an HTML link to `name` with the given `text`.
  def link(name, text); end

  # Should we show '#' characters on method references?
  def show_hash; end

  # Should we show '#' characters on method references?
  def show_hash=(_); end
end

# Outputs [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) markup as
# paragraphs with inline markup only.
class RDoc::Markup::ToHtmlSnippet < ::RDoc::Markup::ToHtml
  # Creates a new
  # [`ToHtmlSnippet`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToHtmlSnippet.html)
  # formatter that will cut off the input on the next word boundary after the
  # given number of `characters` or `paragraphs` of text have been encountered.
  def self.new(options, characters = _, paragraphs = _, markup = _); end

  # Adds `heading` to the output as a paragraph
  def accept_heading(heading); end

  # Finishes consumption of `list_item`
  def accept_list_item_end(list_item); end

  # Prepares the visitor for consuming `list_item`
  def accept_list_item_start(list_item); end

  # Prepares the visitor for consuming `list`
  def accept_list_start(list); end

  def accept_paragraph(paragraph); end

  def accept_raw(*node); end

  def accept_rule(*node); end

  # Adds `verbatim` to the output
  def accept_verbatim(verbatim); end

  # Throws `:done` when
  # [`paragraph_limit`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToHtmlSnippet.html#attribute-i-paragraph_limit)
  # paragraphs have been encountered
  def add_paragraph; end

  # After this many characters the input will be cut off.
  def character_limit; end

  def characters; end

  # Marks up `content`
  def convert(content); end

  # Converts flow items `flow`
  def convert_flow(flow); end

  # Returns just the text of `link`, `url` is only used to determine the link
  # type.
  def gen_url(url, text); end

  def handle_special_CROSSREF(special); end

  def handle_special_HARD_BREAK(special); end

  # In snippets, there are no lists
  def html_list_name(list_type, open_tag); end

  # Lists are paragraphs, but notes and labels have a separator
  def list_item_start(list_item, list_type); end

  # The attribute bitmask
  def mask; end

  # Maintains a bitmask to allow HTML elements to be closed properly. See
  # [`RDoc::Markup::Formatter`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Formatter.html).
  def off_tags(res, item); end

  # Maintains a bitmask to allow HTML elements to be closed properly. See
  # [`RDoc::Markup::Formatter`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Formatter.html).
  def on_tags(res, item); end

  # After this many paragraphs the input will be cut off.
  def paragraph_limit; end

  # Count of paragraphs found
  def paragraphs; end

  # Prepares the visitor for HTML snippet generation
  def start_accepting; end

  # Truncates `text` at the end of the first word after the character\_limit.
  def truncate(text); end
end

# Joins the parts of an
# [`RDoc::Markup::Paragraph`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Paragraph.html)
# into a single [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html).
#
# This allows for easier maintenance and testing of Markdown support.
#
# This formatter only works on Paragraph instances. Attempting to process other
# markup syntax items will not work.
class RDoc::Markup::ToJoinedParagraph < ::RDoc::Markup::Formatter
  def self.new; end

  def accept_block_quote(*node); end

  def accept_heading(*node); end

  def accept_list_end(*node); end

  def accept_list_item_end(*node); end

  def accept_list_item_start(*node); end

  def accept_list_start(*node); end

  # Converts the parts of `paragraph` to a single entry.
  def accept_paragraph(paragraph); end

  def accept_raw(*node); end

  def accept_rule(*node); end

  def accept_verbatim(*node); end

  def end_accepting; end

  def start_accepting; end
end

# Creates HTML-safe labels suitable for use in id attributes. Tidylinks are
# converted to their link part and cross-reference links have the suppression
# marks removed (\SomeClass is converted to SomeClass).
class RDoc::Markup::ToLabel < ::RDoc::Markup::Formatter
  # Creates a new formatter that will output HTML-safe labels
  def self.new(markup = _); end

  def accept_blank_line(*node); end

  def accept_block_quote(*node); end

  def accept_heading(*node); end

  def accept_list_end(*node); end

  def accept_list_item_end(*node); end

  def accept_list_item_start(*node); end

  def accept_list_start(*node); end

  def accept_paragraph(*node); end

  def accept_raw(*node); end

  def accept_rule(*node); end

  def accept_verbatim(*node); end

  # Converts `text` to an HTML-safe label
  def convert(text); end

  def end_accepting(*node); end

  def handle_special_CROSSREF(special); end

  def handle_special_HARD_BREAK(*node); end

  def handle_special_TIDYLINK(special); end

  def res; end

  def start_accepting(*node); end
end

# Outputs parsed markup as Markdown
class RDoc::Markup::ToMarkdown < ::RDoc::Markup::ToRdoc
  # Creates a new formatter that will output Markdown format text
  def self.new(markup = _); end

  # Finishes consumption of `list`
  def accept_list_end(list); end

  # Finishes consumption of `list_item`
  def accept_list_item_end(list_item); end

  # Prepares the visitor for consuming `list_item`
  def accept_list_item_start(list_item); end

  # Prepares the visitor for consuming `list`
  def accept_list_start(list); end

  # Adds `rule` to the output
  def accept_rule(rule); end

  # Outputs `verbatim` indented 4 columns
  def accept_verbatim(verbatim); end

  # Creates a Markdown-style URL from `url` with `text`.
  def gen_url(url, text); end

  # Handles `rdoc-` type links for footnotes.
  def handle_rdoc_link(url); end

  def handle_special_HARD_BREAK(special); end

  def handle_special_RDOCLINK(special); end

  def handle_special_TIDYLINK(special); end

  # Maps attributes to HTML sequences
  def init_tags; end
end

# Outputs [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) markup as
# [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) markup! (mostly)
class RDoc::Markup::ToRdoc < ::RDoc::Markup::Formatter
  # Creates a new formatter that will output (mostly)
  # [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) markup
  def self.new(markup = _); end

  # Adds `blank_line` to the output
  def accept_blank_line(blank_line); end

  # Adds `paragraph` to the output
  def accept_block_quote(block_quote); end

  # Adds `heading` to the output
  def accept_heading(heading); end

  # Adds `paragraph` to the output
  def accept_indented_paragraph(paragraph); end

  # Finishes consumption of `list`
  def accept_list_end(list); end

  # Finishes consumption of `list_item`
  def accept_list_item_end(list_item); end

  # Prepares the visitor for consuming `list_item`
  def accept_list_item_start(list_item); end

  # Prepares the visitor for consuming `list`
  def accept_list_start(list); end

  # Adds `paragraph` to the output
  def accept_paragraph(paragraph); end

  # Adds `raw` to the output
  def accept_raw(raw); end

  # Adds `rule` to the output
  def accept_rule(rule); end

  # Outputs `verbatim` indented 2 columns
  def accept_verbatim(verbatim); end

  # Applies attribute-specific markup to `text` using RDoc::AttributeManager
  def attributes(text); end

  # Returns the generated output
  def end_accepting; end

  def handle_special_HARD_BREAK(special); end

  def handle_special_SUPPRESSED_CROSSREF(special); end

  # Current indent amount for output in characters
  def indent; end

  # Current indent amount for output in characters
  def indent=(_); end

  # Maps attributes to HTML sequences
  def init_tags; end

  # Stack of current list indexes for alphabetic and numeric lists
  def list_index; end

  # Stack of list types
  def list_type; end

  # Stack of list widths for indentation
  def list_width; end

  # Prefix for the next list item. See
  # [`use_prefix`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToRdoc.html#method-i-use_prefix)
  def prefix; end

  # Output accumulator
  def res; end

  # Prepares the visitor for text generation
  def start_accepting; end

  # Adds the stored
  # [`prefix`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToRdoc.html#attribute-i-prefix)
  # to the output and clears it. Lists generate a prefix for later consumption.
  def use_prefix; end

  # Output width in characters
  def width; end

  # Output width in characters
  def width=(_); end

  # Wraps `text` to
  # [`width`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToRdoc.html#attribute-i-width)
  def wrap(text); end
end

# Extracts just the RDoc::Markup::Heading elements from a
# [`RDoc::Markup::Document`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Document.html)
# to help build a table of contents
class RDoc::Markup::ToTableOfContents < ::RDoc::Markup::Formatter
  def self.new; end

  def accept_blank_line(*node); end

  def accept_block_quote(*node); end

  # Adds `document` to the output, using its heading cutoff if present
  def accept_document(document); end

  # Adds `heading` to the table of contents
  def accept_heading(heading); end

  def accept_list_end(*node); end

  def accept_list_end_bullet(*node); end

  def accept_list_item_end(*node); end

  def accept_list_item_start(*node); end

  def accept_list_start(*node); end

  def accept_paragraph(*node); end

  def accept_raw(*node); end

  def accept_rule(*node); end

  def accept_verbatim(*node); end

  # Returns the table of contents
  def end_accepting; end

  # Omits headings with a level less than the given level.
  def omit_headings_below; end

  # Omits headings with a level less than the given level.
  def omit_headings_below=(_); end

  # Output accumulator
  def res; end

  # Prepares the visitor for text generation
  def start_accepting; end

  # Returns true if `heading` is below the display threshold
  def suppressed?(heading); end

  # [`Singleton`](https://docs.ruby-lang.org/en/2.6.0/Singleton.html) for
  # table-of-contents generation
  def self.to_toc; end
end

# This Markup outputter is used for testing purposes.
class RDoc::Markup::ToTest < ::RDoc::Markup::Formatter
  def accept_blank_line(blank_line); end

  def accept_heading(heading); end

  def accept_list_end(list); end

  def accept_list_item_end(list_item); end

  def accept_list_item_start(list_item); end

  def accept_list_start(list); end

  def accept_paragraph(paragraph); end

  def accept_raw(raw); end

  def accept_rule(rule); end

  def accept_verbatim(verbatim); end

  def end_accepting; end

  def start_accepting; end
end

# Extracts sections of text enclosed in plus, tt or code. Used to discover
# undocumented parameters.
class RDoc::Markup::ToTtOnly < ::RDoc::Markup::Formatter
  # Creates a new tt-only formatter.
  def self.new(markup = _); end

  # Alias for:
  # [`do_nothing`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToTtOnly.html#method-i-do_nothing)
  def accept_blank_line(markup_item); end

  # Adds tts from `block_quote` to the output
  def accept_block_quote(block_quote); end

  # Alias for:
  # [`do_nothing`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToTtOnly.html#method-i-do_nothing)
  def accept_heading(markup_item); end

  # Pops the list type for `list` from
  # [`list_type`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToTtOnly.html#attribute-i-list_type)
  def accept_list_end(list); end

  # Alias for:
  # [`do_nothing`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToTtOnly.html#method-i-do_nothing)
  def accept_list_item_end(markup_item); end

  # Prepares the visitor for consuming `list_item`
  def accept_list_item_start(list_item); end

  # Pushes the list type for `list` onto
  # [`list_type`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToTtOnly.html#attribute-i-list_type)
  def accept_list_start(list); end

  # Adds `paragraph` to the output
  def accept_paragraph(paragraph); end

  # Alias for:
  # [`do_nothing`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToTtOnly.html#method-i-do_nothing)
  def accept_raw(markup_item); end

  # Alias for:
  # [`do_nothing`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToTtOnly.html#method-i-do_nothing)
  def accept_rule(markup_item); end

  # Alias for:
  # [`do_nothing`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToTtOnly.html#method-i-do_nothing)
  def accept_verbatim(markup_item); end

  # Does nothing to `markup_item` because it doesn't have any user-built content
  #
  # Also aliased as:
  # [`accept_blank_line`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToTtOnly.html#method-i-accept_blank_line),
  # [`accept_heading`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToTtOnly.html#method-i-accept_heading),
  # [`accept_list_item_end`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToTtOnly.html#method-i-accept_list_item_end),
  # [`accept_raw`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToTtOnly.html#method-i-accept_raw),
  # [`accept_rule`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToTtOnly.html#method-i-accept_rule),
  # [`accept_verbatim`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/ToTtOnly.html#method-i-accept_verbatim)
  def do_nothing(markup_item); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  # items that were wrapped in plus, tt or code.
  def end_accepting; end

  # Stack of list types
  def list_type; end

  # Output accumulator
  def res; end

  # Prepares the visitor for gathering tt sections
  def start_accepting; end

  # Extracts tt sections from `text`
  def tt_sections(text); end
end

# A section of verbatim text
class RDoc::Markup::Verbatim < ::RDoc::Markup::Raw
  def self.new(*parts); end

  def ==(other); end

  # Calls accept\_verbatim on `visitor`
  def accept(visitor); end

  # Format of this verbatim section
  def format; end

  # Format of this verbatim section
  def format=(_); end

  # Collapses 3+ newlines into two newlines
  def normalize; end

  def pretty_print(q); end

  # Is this verbatim section Ruby code?
  def ruby?; end

  # The text of the section
  def text; end
end

# [`MetaMethod`](https://docs.ruby-lang.org/en/2.7.0/RDoc/MetaMethod.html)
# represents a meta-programmed method
class RDoc::MetaMethod < ::RDoc::AnyMethod; end

# Abstract class representing either a method or an attribute.
class RDoc::MethodAttr < ::RDoc::CodeObject
  include(::Comparable)

  # Creates a new
  # [`MethodAttr`](https://docs.ruby-lang.org/en/2.7.0/RDoc/MethodAttr.html)
  # from token stream `text` and method or attribute name `name`.
  #
  # Usually this is called by super from a subclass.
  def self.new(text, name); end

  # Order by
  # [`singleton`](https://docs.ruby-lang.org/en/2.7.0/RDoc/MethodAttr.html#attribute-i-singleton)
  # then
  # [`name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/MethodAttr.html#attribute-i-name)
  def <=>(other); end

  def ==(other); end

  # Abstract method. Contexts in their building phase call this to register a
  # new alias for this known method/attribute.
  #
  # *   creates a new AnyMethod/Attribute named `an_alias.new_name`;
  # *   adds `self` as an alias for the new method or attribute
  # *   adds the method or attribute to
  #     [`aliases`](https://docs.ruby-lang.org/en/2.7.0/RDoc/MethodAttr.html#attribute-i-aliases)
  # *   adds the method or attribute to `context`.
  def add_alias(an_alias, context); end

  # Prepend `src` with line numbers. Relies on the first line of a source code
  # listing having:
  #
  # ```ruby
  # # File xxxxx, line dddd
  # ```
  #
  # If it has this comment then line numbers are added to `src` and the `, line
  # dddd` portion of the comment is removed.
  def add_line_numbers(src); end

  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of other names for
  # this method/attribute
  def aliases; end

  # HTML fragment reference for this method
  def aref; end

  # Prefix for `aref`, defined by subclasses.
  def aref_prefix; end

  # The
  # [`call_seq`](https://docs.ruby-lang.org/en/2.7.0/RDoc/MethodAttr.html#attribute-i-call_seq)
  # or the
  # [`param_seq`](https://docs.ruby-lang.org/en/2.7.0/RDoc/MethodAttr.html#attribute-i-param_seq)
  # with method name, if there is no call\_seq.
  def arglists; end

  # Parameters yielded by the called block
  def block_params; end

  # Attempts to sanitize the content passed by the Ruby parser: remove outer
  # parentheses, etc.
  def block_params=(value); end

  # Different ways to call this method
  def call_seq; end

  # Different ways to call this method
  def call_seq=(_); end

  # A method/attribute is documented if any of the following is true:
  # *   it was marked with :nodoc:;
  # *   it has a comment;
  # *   it is an alias for a documented method;
  # *   it has a `#see` method that is documented.
  def documented?; end

  def find_method_or_attribute(name); end

  def find_see; end

  # Full method/attribute name including namespace
  def full_name; end

  # HTML id-friendly method/attribute name
  def html_name; end

  def self.new_visibility; end

  def inspect; end

  # The method/attribute we're aliasing
  def is_alias_for; end

  # The method/attribute we're aliasing
  def is_alias_for=(_); end

  # Turns the method's token stream into HTML.
  #
  # Prepends line numbers if `options.line_numbers` is true.
  def markup_code; end

  # Name of this method/attribute.
  def name; end

  # Name of this method/attribute.
  def name=(_); end

  # '::' for a class method/attribute, '#' for an instance method.
  def name_prefix; end

  # Name for output to HTML. For class methods the full name with a "." is used
  # like `SomeClass.method_name`. For instance methods the class name is used if
  # `context` does not match the parent.
  #
  # This is to help prevent people from using
  # :   to call class methods.
  def output_name(context); end

  # Pretty parameter list for this method
  def param_seq; end

  # Parameters for this method
  def params; end

  # Parameters for this method
  def params=(_); end

  # Name of our parent with special handling for un-marshaled methods
  def parent_name; end

  # Path to this method for use with HTML generator output.
  def path; end

  # Method/attribute name with class/instance indicator
  def pretty_name; end

  def pretty_print(q); end

  # Used by RDoc::Generator::JsonIndex to create a record for the search engine.
  def search_record; end

  # A method/attribute to look at, in particular if this method/attribute has no
  # documentation.
  #
  # It can be a method/attribute of the superclass or of an included module,
  # including the [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html)
  # module, which is always appended to the included modules.
  #
  # Returns `nil` if there is no such method/attribute. The `#is_alias_for`
  # method/attribute, if any, is not included.
  #
  # Templates may generate a "see also ..." if this method/attribute has
  # documentation, and "see ..." if it does not.
  def see; end

  # Is this a singleton method/attribute?
  def singleton; end

  # Is this a singleton method/attribute?
  def singleton=(_); end

  # Sets the store for this class or module and its contained code objects.
  def store=(store); end

  # Source file token stream
  def text; end

  def to_s; end

  # Type of method/attribute (class or instance)
  def type; end

  # public, protected, private
  def visibility; end

  # public, protected, private
  def visibility=(_); end

  def self.add_line_numbers; end

  def self.add_line_numbers=(_); end
end

# A [`Mixin`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Mixin.html) adds features
# from a module into another context.
# [`RDoc::Include`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Include.html) and
# [`RDoc::Extend`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Extend.html) are
# both mixins.
class RDoc::Mixin < ::RDoc::CodeObject
  # Creates a new [`Mixin`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Mixin.html)
  # for `name` with `comment`
  def self.new(name, comment); end

  # Mixins are sorted by name
  def <=>(other); end

  def ==(other); end

  def eql?(other); end

  # Full name based on
  # [`module`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Mixin.html#method-i-module)
  def full_name; end

  def hash; end

  def inspect; end

  # Attempts to locate the included module object. Returns the name if not
  # known.
  #
  # The scoping rules of Ruby to resolve the name of an included module are:
  # *   first look into the children of the current context;
  # *   if not found, look into the children of included modules, in reverse
  #     inclusion order;
  # *   if still not found, go up the hierarchy of names.
  #
  #
  # This method has `O(n!)` behavior when the module calling include is
  # referencing nonexistent modules. Avoid calling
  # [`module`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Mixin.html#method-i-module)
  # until after all the files are parsed. This behavior is due to ruby's
  # constant lookup behavior.
  #
  # As of the beginning of October, 2011, no gem includes nonexistent modules.
  def module; end

  # Name of included module
  def name; end

  # Name of included module
  def name=(_); end

  # Sets the store for this class or module and its contained code objects.
  def store=(store); end

  def to_s; end
end

# A normal class, neither singleton nor anonymous
class RDoc::NormalClass < ::RDoc::ClassModule
  # The ancestors of this class including modules. Unlike
  # [`Module#ancestors`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-ancestors),
  # this class is not included in the result. The result will contain both
  # RDoc::ClassModules and Strings.
  def ancestors; end

  def aref_prefix; end

  # The definition of this class, `class MyClassName`
  def definition; end

  def direct_ancestors; end

  def inspect; end

  def pretty_print(q); end

  def to_s; end
end

# A normal module, like NormalClass
class RDoc::NormalModule < ::RDoc::ClassModule
  def aref_prefix; end

  # The definition of this module, `module MyModuleName`
  def definition; end

  def inspect; end

  # This is a module, returns true
  def module?; end

  def pretty_print(q); end

  # Modules don't have one, raises
  # [`NoMethodError`](https://docs.ruby-lang.org/en/2.7.0/NoMethodError.html)
  def superclass; end
end

# [`RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html)
# handles the parsing and storage of options
#
# ## Saved [`Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html)
#
# You can save some options like the markup format in the `.rdoc_options` file
# in your gem. The easiest way to do this is:
#
# ```
# rdoc --markup tomdoc --write-options
# ```
#
# Which will automatically create the file and fill it with the options you
# specified.
#
# The following options will not be saved since they interfere with the user's
# preferences or with the normal operation of RDoc:
#
# *   `--coverage-report`
# *   `--dry-run`
# *   `--encoding`
# *   `--force-update`
# *   `--format`
# *   `--pipe`
# *   `--quiet`
# *   `--template`
# *   `--verbose`
#
#
# ## Custom [`Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html)
#
# Generators can hook into
# [`RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html) to
# add generator-specific command line options.
#
# When `--format` is encountered in ARGV,
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) calls ::setup\_options
# on the generator class to add extra options to the option parser.
# [`Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html) for custom
# generators must occur after `--format`. `rdoc --help` will list options for
# all installed generators.
#
# Example:
#
# ```ruby
# class RDoc::Generator::Spellcheck
#   RDoc::RDoc.add_generator self
#
#   def self.setup_options rdoc_options
#     op = rdoc_options.option_parser
#
#     op.on('--spell-dictionary DICTIONARY',
#           RDoc::Options::Path) do |dictionary|
#       rdoc_options.spell_dictionary = dictionary
#     end
#   end
# end
# ```
#
# Of course,
# [`RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html) does
# not respond to `spell_dictionary` by default so you will need to add it:
#
# ```ruby
# class RDoc::Options
#
#   ##
#   # The spell dictionary used by the spell-checking plugin.
#
#   attr_accessor :spell_dictionary
#
# end
# ```
#
# ## Option Validators
#
# [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html)
# validators will validate and cast user input values. In addition to the
# validators that ship with
# [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html)
# ([`String`](https://docs.ruby-lang.org/en/2.7.0/String.html),
# [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html),
# [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html),
# [`TrueClass`](https://docs.ruby-lang.org/en/2.7.0/TrueClass.html),
# [`FalseClass`](https://docs.ruby-lang.org/en/2.7.0/FalseClass.html),
# [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html),
# [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html),
# [`Date`](https://docs.ruby-lang.org/en/2.7.0/Date.html),
# [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html),
# [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html), etc.),
# [`RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html) adds
# [`Path`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#Path),
# [`PathArray`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#PathArray)
# and
# [`Template`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#Template).
class RDoc::Options
  # The deprecated options.
  DEPRECATED = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # Option validator for
  # [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html) that
  # matches a directory that exists on the filesystem.
  Directory = T.let(T.unsafe(nil), Object)

  # Option validator for
  # [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html) that
  # matches a file or directory that exists on the filesystem.
  Path = T.let(T.unsafe(nil), Object)

  # Option validator for
  # [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html) that
  # matches a comma-separated list of files or directories that exist on the
  # filesystem.
  PathArray = T.let(T.unsafe(nil), Object)

  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) options ignored (or
  # handled specially) by --write-options
  SPECIAL = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Option validator for
  # [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html) that
  # matches a template directory for an installed generator that lives in
  # `"rdoc/generator/template/#{template_name}"`
  Template = T.let(T.unsafe(nil), Object)

  def self.new; end

  def ==(other); end

  # Character-set for HTML output.
  # [`encoding`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#attribute-i-encoding)
  # is preferred over
  # [`charset`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#attribute-i-charset)
  def charset; end

  # Character-set for HTML output.
  # [`encoding`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#attribute-i-encoding)
  # is preferred over
  # [`charset`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#attribute-i-charset)
  def charset=(_); end

  # Check that the files on the command line exist
  def check_files; end

  # Ensure only one generator is loaded
  def check_generator; end

  # If true, only report on undocumented files
  def coverage_report; end

  # If true, only report on undocumented files
  def coverage_report=(_); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the title, but only if
  # not already set. Used to set the title from a source file, so that a title
  # set from the command line will have the priority.
  def default_title=(string); end

  # If true, [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) will not
  # write any files.
  def dry_run; end

  # If true, [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) will not
  # write any files.
  def dry_run=(_); end

  def encode_with(coder); end

  # The output encoding. All input files will be transcoded to this encoding.
  #
  # The default encoding is UTF-8. This is set via --encoding.
  def encoding; end

  # The output encoding. All input files will be transcoded to this encoding.
  #
  # The default encoding is UTF-8. This is set via --encoding.
  def encoding=(_); end

  # Files matching this pattern will be excluded
  def exclude; end

  # Files matching this pattern will be excluded
  def exclude=(_); end

  # The list of files to be processed
  def files; end

  # The list of files to be processed
  def files=(_); end

  # Completes any unfinished option setup business such as filtering for
  # existent files, creating a regexp for
  # [`exclude`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#attribute-i-exclude)
  # and setting a default
  # [`template`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#attribute-i-template).
  def finish; end

  # Fixes the
  # [`page_dir`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#attribute-i-page_dir)
  # to be relative to the root\_dir and adds the
  # [`page_dir`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#attribute-i-page_dir)
  # to the files list.
  def finish_page_dir; end

  # Create the output even if the output directory does not look like an rdoc
  # output directory
  def force_output; end

  # Create the output even if the output directory does not look like an rdoc
  # output directory
  def force_output=(_); end

  # Scan newer sources than the flag file if true.
  def force_update; end

  # Scan newer sources than the flag file if true.
  def force_update=(_); end

  # Formatter to mark up text with
  def formatter; end

  # Formatter to mark up text with
  def formatter=(_); end

  # Description of the output generator (set with the `--format` option)
  def generator; end

  # Description of the output generator (set with the `--format` option)
  def generator=(_); end

  # Returns a properly-space list of generators and their descriptions.
  def generator_descriptions; end

  def generator_name; end

  # Loaded generator options. Used to prevent --help from loading the same
  # options multiple times.
  def generator_options; end

  # Loaded generator options. Used to prevent --help from loading the same
  # options multiple times.
  def generator_options=(_); end

  # Old rdoc behavior: hyperlink all words that match a method name, even if not
  # preceded by '#' or '::'
  def hyperlink_all; end

  # Old rdoc behavior: hyperlink all words that match a method name, even if not
  # preceded by '#' or '::'
  def hyperlink_all=(_); end

  def init_ivars; end

  def init_with(map); end

  # Include line numbers in the source code
  def line_numbers; end

  # Include line numbers in the source code
  def line_numbers=(_); end

  # The output locale.
  def locale; end

  # The output locale.
  def locale=(_); end

  # The directory where locale data live.
  def locale_dir; end

  # The directory where locale data live.
  def locale_dir=(_); end

  # Name of the file, class or module to display in the initial index page (if
  # not specified the first file we encounter is used)
  def main_page; end

  # Name of the file, class or module to display in the initial index page (if
  # not specified the first file we encounter is used)
  def main_page=(_); end

  # The default markup format. The default is 'rdoc'. 'markdown', 'tomdoc' and
  # 'rd' are also built-in.
  def markup; end

  # The default markup format. The default is 'rdoc'. 'markdown', 'tomdoc' and
  # 'rd' are also built-in.
  def markup=(_); end

  # The name of the output directory
  def op_dir; end

  # The name of the output directory
  def op_dir=(_); end

  # The [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html)
  # for this instance
  def option_parser; end

  # The [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html)
  # for this instance
  def option_parser=(_); end

  # Output heading decorations?
  def output_decoration; end

  # Output heading decorations?
  def output_decoration=(_); end

  # [`Directory`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#Directory)
  # where guides, FAQ, and other pages not associated with a class live. You may
  # leave this unset if these are at the root of your project.
  def page_dir; end

  # [`Directory`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#Directory)
  # where guides, FAQ, and other pages not associated with a class live. You may
  # leave this unset if these are at the root of your project.
  def page_dir=(_); end

  # Parses command line options.
  def parse(argv); end

  # Is [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) in pipe mode?
  def pipe; end

  # Is [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) in pipe mode?
  def pipe=(_); end

  # Don't display progress as we process the files
  def quiet; end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) quietness to `bool`
  def quiet=(bool); end

  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of directories to
  # search for files to satisfy an :include:
  def rdoc_include; end

  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of directories to
  # search for files to satisfy an :include:
  def rdoc_include=(_); end

  # Root of the source documentation will be generated for.
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) this when building
  # documentation outside the source directory. Defaults to the current
  # directory.
  def root; end

  # Root of the source documentation will be generated for.
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) this when building
  # documentation outside the source directory. Defaults to the current
  # directory.
  def root=(_); end

  # Removes directories from `path` that are outside the current directory
  def sanitize_path(path); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) up an output generator
  # for the named `generator_name`.
  #
  # If the found generator responds to :setup\_options it will be called with
  # the options instance. This allows generators to add custom options or set
  # default options.
  def setup_generator(generator_name = _); end

  # Include the '#' at the front of hyperlinked instance method names
  def show_hash; end

  # Include the '#' at the front of hyperlinked instance method names
  def show_hash=(_); end

  # [`Directory`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#Directory)
  # to copy static files from
  def static_path; end

  # [`Directory`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#Directory)
  # to copy static files from
  def static_path=(_); end

  # The number of columns in a tab
  def tab_width; end

  # The number of columns in a tab
  def tab_width=(_); end

  # [`Template`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#Template)
  # to be used when generating output
  def template; end

  # [`Template`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#Template)
  # to be used when generating output
  def template=(_); end

  # [`Directory`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#Directory)
  # the template lives in
  def template_dir; end

  # [`Directory`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#Directory)
  # the template lives in
  def template_dir=(_); end

  # Finds the template dir for `template`
  def template_dir_for(template); end

  # Additional template stylesheets
  def template_stylesheets; end

  # Additional template stylesheets
  def template_stylesheets=(_); end

  # Documentation title
  def title; end

  # Documentation title
  def title=(_); end

  # Should [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) update the
  # timestamps in the output dir?
  def update_output_dir; end

  # Should [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) update the
  # timestamps in the output dir?
  def update_output_dir=(_); end

  # Verbosity, zero means quiet
  def verbosity; end

  # Verbosity, zero means quiet
  def verbosity=(_); end

  # Minimum visibility of a documented method. One of `:public`, `:protected`,
  # `:private` or `:nodoc`.
  #
  # The `:nodoc` visibility ignores all directives related to visibility. The
  # other visibilities may be overridden on a per-method basis with the :doc:
  # directive.
  def visibility; end

  # Sets the minimum visibility of a documented method.
  #
  # Accepts `:public`, `:protected`, `:private`, `:nodoc`, or `:all`.
  #
  # When `:all` is passed, visibility is set to `:private`, similarly to
  # RDOCOPT="--all", see
  # [`visibility`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#attribute-i-visibility)
  # for more information.
  def visibility=(visibility); end

  # Displays a warning using
  # [`Kernel#warn`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-warn)
  # if we're being verbose
  def warn(message); end

  # URL of web cvs frontend
  def webcvs; end

  # URL of web cvs frontend
  def webcvs=(_); end

  # Writes the [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) file
  # .rdoc\_options to the current directory containing the parsed options.
  def write_options; end

  def yaml_initialize(tag, map); end
end

# A parser is simple a class that subclasses
# [`RDoc::Parser`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser.html) and
# implements scan to fill in an
# [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
# with parsed data.
#
# The initialize method takes an
# [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html) to
# fill with parsed content, the name of the file to be parsed, the content of
# the file, an
# [`RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html)
# object and an
# [`RDoc::Stats`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Stats.html) object to
# inform the user of parsed items. The scan method is then called to parse the
# file and must return the
# [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
# object. By calling super these items will be set for you.
#
# In order to be used by [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
# the parser needs to register the file extensions it can parse. Use
# [`::parse_files_matching`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser.html#method-c-parse_files_matching)
# to register extensions.
#
# ```ruby
# require 'rdoc'
#
# class RDoc::Parser::Xyz < RDoc::Parser
#   parse_files_matching /\.xyz$/
#
#   def initialize top_level, file_name, content, options, stats
#     super
#
#     # extra initialization if needed
#   end
#
#   def scan
#     # parse file and fill in @top_level
#   end
# end
# ```
class RDoc::Parser
  # Creates a new
  # [`Parser`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser.html) storing
  # `top_level`, `file_name`, `content`, `options` and `stats` in instance
  # variables. In +@preprocess+ an RDoc::Markup::PreProcess object is created
  # which allows processing of directives.
  def self.new(top_level, file_name, content, options, stats); end

  # The name of the file being parsed
  def file_name; end

  # Alias an extension to another extension. After this call, files ending
  # "new\_ext" will be parsed using the same parser as "old\_ext"
  def self.alias_extension(old_ext, new_ext); end

  # Determines if the file is a "binary" file which basically means it has
  # content that an [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
  # parser shouldn't try to consume.
  def self.binary?(file); end

  # Return a parser that can handle a particular extension
  def self.can_parse(file_name); end

  # Returns a parser that can handle the extension for `file_name`. This does
  # not depend upon the file being readable.
  def self.can_parse_by_name(file_name); end

  # Returns the file type from the modeline in `file_name`
  def self.check_modeline(file_name); end

  # Finds and instantiates the correct parser for the given `file_name` and
  # `content`.
  def self.for(top_level, file_name, content, options, stats); end

  # Record which file types this parser can understand.
  #
  # It is ok to call this multiple times.
  def self.parse_files_matching(regexp); end

  # An [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of arrays that
  # maps file extension (or name) regular expressions to parser classes that
  # will parse matching filenames.
  #
  # Use
  # [`parse_files_matching`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser.html#method-c-parse_files_matching)
  # to register a parser's file extensions.
  def self.parsers; end

  # Removes an emacs-style modeline from the first line of the document
  def self.remove_modeline(content); end

  # If there is a `markup: parser_name` comment at the front of the file, use it
  # to determine the parser. For example:
  #
  # ```ruby
  # # markup: rdoc
  # # Class comment can go here
  #
  # class C
  # end
  # ```
  #
  # The comment should appear as the first line of the `content`.
  #
  # If the content contains a shebang or editor modeline the comment may appear
  # on the second or third line.
  #
  # Any comment style may be used to hide the markup comment.
  def self.use_markup(content); end

  # Checks if `file` is a zip file in disguise. Signatures from
  # http://www.garykessler.net/library/file\_sigs.html
  def self.zip?(file); end
end

# [`RDoc::Parser::C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html)
# attempts to parse
# [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html) extension files.
# It looks for the standard patterns that you find in extensions:
# `rb_define_class`, `rb_define_method` and so on. It tries to find the
# corresponding [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html)
# source for the methods and extract comments, but if we fail we don't worry too
# much.
#
# The comments associated with a Ruby method are extracted from the
# [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html) comment block
# associated with the routine that *implements* that method, that is to say the
# method whose name is given in the `rb_define_method` call. For example, you
# might write:
#
# ```
# /*
#  * Returns a new array that is a one-dimensional flattening of this
#  * array (recursively). That is, for every element that is an array,
#  * extract its elements into the new array.
#  *
#  *    s = [ 1, 2, 3 ]           #=> [1, 2, 3]
#  *    t = [ 4, 5, 6, [7, 8] ]   #=> [4, 5, 6, [7, 8]]
#  *    a = [ s, t, 9, 10 ]       #=> [[1, 2, 3], [4, 5, 6, [7, 8]], 9, 10]
#  *    a.flatten                 #=> [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
#  */
#  static VALUE
#  rb_ary_flatten(VALUE ary)
#  {
#      ary = rb_obj_dup(ary);
#      rb_ary_flatten_bang(ary);
#      return ary;
#  }
#
#  ...
#
#  void
#  Init_Array(void)
#  {
#    ...
#    rb_define_method(rb_cArray, "flatten", rb_ary_flatten, 0);
# ```
#
# Here [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) will determine
# from the `rb_define_method` line that there's a method called "flatten" in
# class [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html), and will look
# for the implementation in the method `rb_ary_flatten`. It will then use the
# comment from that method in the HTML output. This method must be in the same
# source file as the `rb_define_method`.
#
# The comment blocks may include special directives:
#
# Document-class: `name`
# :   Documentation for the named class.
#
# Document-module: `name`
# :   Documentation for the named module.
#
# Document-const: `name`
# :   Documentation for the named `rb_define_const`.
#
#     Constant values can be supplied on the first line of the comment like so:
#
# ```
# /* 300: The highest possible score in bowling */
# rb_define_const(cFoo, "PERFECT", INT2FIX(300));
# ```
#
#     The value can contain internal colons so long as they are escaped with a
#     \\
#
# Document-global: `name`
# :   Documentation for the named `rb_define_global_const`
#
# Document-variable: `name`
# :   Documentation for the named `rb_define_variable`
#
# Document-method: `method_name`
# :   Documentation for the named method. Use this when the method name is
#     unambiguous.
#
# Document-method: `ClassName::method_name`
# :   Documentation for a singleton method in the given class. Use this when the
#     method name alone is ambiguous.
#
# Document-method: `ClassName#method_name`
# :   Documentation for a instance method in the given class. Use this when the
#     method name alone is ambiguous.
#
# Document-attr: `name`
# :   Documentation for the named attribute.
#
# call-seq:  *text up to an empty line*
# :   Because [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html)
#     source doesn't give descriptive names to Ruby-level parameters, you need
#     to document the calling sequence explicitly
#
#
# In addition, [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) assumes
# by default that the
# [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html) method
# implementing a Ruby function is in the same source file as the
# rb\_define\_method call. If this isn't the case, add the comment:
#
# ```
# rb_define_method(....);  // in filename
# ```
#
# As an example, we might have an extension that defines multiple classes in its
# Init\_xxx method. We could document them using
#
# ```
# /*
#  * Document-class:  MyClass
#  *
#  * Encapsulate the writing and reading of the configuration
#  * file. ...
#  */
#
# /*
#  * Document-method: read_value
#  *
#  * call-seq:
#  *   cfg.read_value(key)            -> value
#  *   cfg.read_value(key} { |key| }  -> value
#  *
#  * Return the value corresponding to +key+ from the configuration.
#  * In the second form, if the key isn't found, invoke the
#  * block and return its value.
#  */
# ```
class RDoc::Parser::C < ::RDoc::Parser
  include(::RDoc::Text)

  # Prepares for parsing a
  # [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html) file. See
  # RDoc::Parser#initialize for details on the arguments.
  def self.new(top_level, file_name, content, options, stats); end

  # Maps [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html) variable
  # names to names of Ruby classes or modules
  def classes; end

  # [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html) file the
  # parser is parsing
  def content; end

  # [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html) file the
  # parser is parsing
  def content=(_); end

  # Removes duplicate call-seq entries for methods using the same
  # implementation.
  def deduplicate_call_seq; end

  def deduplicate_method_name(class_obj, method_name); end

  # Scans
  # [`content`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html#attribute-i-content)
  # for rb\_define\_alias
  def do_aliases; end

  # Scans
  # [`content`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html#attribute-i-content)
  # for rb\_attr and rb\_define\_attr
  def do_attrs; end

  # Scans
  # [`content`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html#attribute-i-content)
  # for boot\_defclass
  def do_boot_defclass; end

  # Scans
  # [`content`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Parser/C.html#attribute-i-content)
  # for rb\_define\_class, boot\_defclass, rb\_define\_class\_under and
  # rb\_singleton\_class
  def do_classes; end

  # Scans
  # [`content`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html#attribute-i-content)
  # for rb\_define\_variable, rb\_define\_readonly\_variable, rb\_define\_const
  # and rb\_define\_global\_const
  def do_constants; end

  # Scans
  # [`content`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Parser/C.html#attribute-i-content)
  # for rb\_define\_class
  def do_define_class; end

  # Scans
  # [`content`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Parser/C.html#attribute-i-content)
  # for rb\_define\_class\_under
  def do_define_class_under; end

  # Scans
  # [`content`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Parser/C.html#attribute-i-content)
  # for rb\_define\_module
  def do_define_module; end

  # Scans
  # [`content`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Parser/C.html#attribute-i-content)
  # for rb\_define\_module\_under
  def do_define_module_under; end

  # Scans
  # [`content`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html#attribute-i-content)
  # for rb\_include\_module
  def do_includes; end

  # Scans
  # [`content`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html#attribute-i-content)
  # for rb\_define\_method, rb\_define\_singleton\_method,
  # rb\_define\_module\_function, rb\_define\_private\_method,
  # rb\_define\_global\_function and define\_filetest\_function
  def do_methods; end

  # Creates classes and module that were missing were defined due to the file
  # order being different than the declaration order.
  def do_missing; end

  # Scans
  # [`content`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Parser/C.html#attribute-i-content)
  # for rb\_define\_module and rb\_define\_module\_under
  def do_modules; end

  # Scans
  # [`content`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Parser/C.html#attribute-i-content)
  # for rb\_singleton\_class
  def do_singleton_class; end

  # Scans
  # [`content`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Parser/C.html#attribute-i-content)
  # for struct\_define\_without\_accessor
  def do_struct_define_without_accessor; end

  # Dependencies from a missing enclosing class to the classes in
  # [`missing_dependencies`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html#attribute-i-missing_dependencies)
  # that depend upon it.
  def enclosure_dependencies; end

  # Finds the comment for an alias on `class_name` from `new_name` to `old_name`
  def find_alias_comment(class_name, new_name, old_name); end

  # Finds a comment for rb\_define\_attr, rb\_attr or Document-attr.
  #
  # `var_name` is the
  # [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html) class variable
  # the attribute is defined on. `attr_name` is the attribute's name.
  #
  # `read` and `write` are the read/write flags ('1' or '0'). Either both or
  # neither must be provided.
  def find_attr_comment(var_name, attr_name, read = _, write = _); end

  # [`Find`](https://docs.ruby-lang.org/en/2.7.0/Find.html) the
  # [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html) code
  # corresponding to a Ruby method
  def find_body(class_name, meth_name, meth_obj, file_content, quiet = _); end

  # Finds a
  # [`RDoc::NormalClass`](https://docs.ruby-lang.org/en/2.7.0/RDoc/NormalClass.html)
  # or
  # [`RDoc::NormalModule`](https://docs.ruby-lang.org/en/2.7.0/RDoc/NormalModule.html)
  # for `raw_name`
  def find_class(raw_name, name); end

  # Look for class or module documentation above Init\_+class\_name+(void), in a
  # Document-class `class_name` (or module) comment or above an
  # rb\_define\_class (or module). If a comment is supplied above a matching
  # Init\_ and a rb\_define\_class the Init\_ comment is used.
  #
  # ```
  # /*
  #  * This is a comment for Foo
  #  */
  # Init_Foo(void) {
  #     VALUE cFoo = rb_define_class("Foo", rb_cObject);
  # }
  #
  # /*
  #  * Document-class: Foo
  #  * This is a comment for Foo
  #  */
  # Init_foo(void) {
  #     VALUE cFoo = rb_define_class("Foo", rb_cObject);
  # }
  #
  # /*
  #  * This is a comment for Foo
  #  */
  # VALUE cFoo = rb_define_class("Foo", rb_cObject);
  # ```
  def find_class_comment(class_name, class_mod); end

  # Finds a comment matching `type` and `const_name` either above the comment or
  # in the matching Document- section.
  def find_const_comment(type, const_name, class_name = _); end

  # Handles modifiers in `comment` and updates `meth_obj` as appropriate.
  def find_modifiers(comment, meth_obj); end

  # Finds a `Document-method` override for `meth_obj` on `class_name`
  def find_override_comment(class_name, meth_obj); end

  # Generate a Ruby-method table
  def gen_body_table(file_content); end

  # Generate a const table
  def gen_const_table(file_content); end

  # Creates a new
  # [`RDoc::Attr`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Attr.html)
  # `attr_name` on class `var_name` that is either `read`, `write` or both
  def handle_attr(var_name, attr_name, read, write); end

  # Creates a new
  # [`RDoc::NormalClass`](https://docs.ruby-lang.org/en/2.7.0/RDoc/NormalClass.html)
  # or
  # [`RDoc::NormalModule`](https://docs.ruby-lang.org/en/2.7.0/RDoc/NormalModule.html)
  # based on `type` named `class_name` in `parent` which was assigned to the
  # [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html) `var_name`.
  def handle_class_module(var_name, type, class_name, parent, in_module); end

  # Adds constants. By providing some\_value: at the start of the comment you
  # can override the
  # [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html) value of the
  # comment to give a friendly definition.
  #
  # ```
  # /* 300: The perfect score in bowling */
  # rb_define_const(cFoo, "PERFECT", INT2FIX(300));
  # ```
  #
  # Will override `INT2FIX(300)` with the value `300` in the output
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html). Values may include
  # quotes and escaped colons (:).
  def handle_constants(type, var_name, const_name, definition); end

  # Removes ifdefs that would otherwise confuse us
  def handle_ifdefs_in(body); end

  # Adds an
  # [`RDoc::AnyMethod`](https://docs.ruby-lang.org/en/2.7.0/RDoc/AnyMethod.html)
  # `meth_name` defined on a class or module assigned to `var_name`. `type` is
  # the type of method definition function used. `singleton_method` and
  # `module_function` create a singleton method.
  def handle_method(type, var_name, meth_name, function, param_count, source_file = _); end

  # Registers a singleton class `sclass_var` as a singleton of `class_var`
  def handle_singleton(sclass_var, class_var); end

  # Normalizes tabs in `body`
  def handle_tab_width(body); end

  # Maps [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html) variable
  # names to names of Ruby classes (and singleton classes)
  def known_classes; end

  # Loads the variable map with the given `name` from the
  # [`RDoc::Store`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html), if
  # present.
  def load_variable_map(map_name); end

  # Look for directives in a normal comment block:
  #
  # ```
  # /*
  #  * :title: My Awesome Project
  #  */
  # ```
  #
  # This method modifies the `comment`
  def look_for_directives_in(context, comment); end

  # Classes found while parsing the
  # [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html) file that were
  # not yet registered due to a missing enclosing class. These are processed by
  # [`do_missing`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html#method-i-do_missing)
  def missing_dependencies; end

  # Extracts parameters from the `method_body` and returns a method parameter
  # string. Follows 1.9.3dev's scan-arg-spec, see
  # [README](https://docs.ruby-lang.org/en/2.7.0/README_md.html).EXT
  def rb_scan_args(method_body); end

  # Removes lines that are commented out that might otherwise get picked up when
  # scanning for classes and methods
  def remove_commented_out_lines; end

  # Extracts the classes, modules, methods, attributes, constants and aliases
  # from a [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html) file
  # and returns an
  # [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
  # for this file
  def scan; end

  # Maps [`C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html) variable
  # names to names of Ruby singleton classes
  def singleton_classes; end

  # The TopLevel items in the parsed file belong to
  def top_level; end
end

# A
# [`ChangeLog`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/ChangeLog.html)
# file parser.
#
# This parser converts a
# [`ChangeLog`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/ChangeLog.html)
# into an RDoc::Markup::Document. When viewed as HTML a
# [`ChangeLog`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/ChangeLog.html)
# page will have an entry for each day's entries in the sidebar table of
# contents.
#
# This parser is meant to parse the MRI
# [`ChangeLog`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/ChangeLog.html),
# but can be used to parse any [GNU style Change
# Log](http://www.gnu.org/prep/standards/html_node/Style-of-Change-Logs.html).
class RDoc::Parser::ChangeLog < ::RDoc::Parser
  include(::RDoc::Parser::Text)

  # Attaches the `continuation` of the previous line to the `entry_body`.
  #
  # Continued function listings are joined together as a single entry. Continued
  # descriptions are joined to make a single paragraph.
  def continue_entry_body(entry_body, continuation); end

  # Creates an RDoc::Markup::Document given the `groups` of
  # [`ChangeLog`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/ChangeLog.html)
  # entries.
  def create_document(groups); end

  # Returns a list of
  # [`ChangeLog`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/ChangeLog.html)
  # entries an
  # [`RDoc::Markup`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html) nodes
  # for the given `entries`.
  def create_entries(entries); end

  # Returns an RDoc::Markup::List containing the given `items` in the
  # [`ChangeLog`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/ChangeLog.html)
  def create_items(items); end

  # Groups `entries` by date.
  def group_entries(entries); end

  # Parses the entries in the
  # [`ChangeLog`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/ChangeLog.html).
  #
  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of each
  # [`ChangeLog`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/ChangeLog.html)
  # entry in order of parsing.
  #
  # A
  # [`ChangeLog`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/ChangeLog.html)
  # entry is an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
  # containing the
  # [`ChangeLog`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/ChangeLog.html)
  # title (date and committer) and an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # [`ChangeLog`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/ChangeLog.html)
  # items (file and function changed with description).
  #
  # An example result would be:
  #
  # ```ruby
  # [ 'Tue Dec  4 08:33:46 2012  Eric Hodel  <drbrain@segment7.net>',
  #   [ 'README.EXT:  Converted to RDoc format',
  #     'README.EXT.ja:  ditto']]
  # ```
  def parse_entries; end

  # Converts the
  # [`ChangeLog`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/ChangeLog.html)
  # into an RDoc::Markup::Document
  def scan; end
end

# Parse a
# [`Markdown`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/Markdown.html)
# format file. The parsed RDoc::Markup::Document is attached as a file comment.
class RDoc::Parser::Markdown < ::RDoc::Parser
  include(::RDoc::Parser::Text)

  # Creates an Markdown-format TopLevel for the given file.
  def scan; end
end

# Parse a [`RD`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/RD.html) format
# file. The parsed RDoc::Markup::Document is attached as a file comment.
class RDoc::Parser::RD < ::RDoc::Parser
  include(::RDoc::Parser::Text)

  # Creates an rd-format TopLevel for the given file.
  def scan; end
end

class RDoc::Parser::Ruby < ::RDoc::Parser
  include(::RDoc::Parser::RubyTools)
  include(::RDoc::TokenStream)

  # [`RDoc::NormalClass`](https://docs.ruby-lang.org/en/2.7.0/RDoc/NormalClass.html)
  # type
  NORMAL = T.let(T.unsafe(nil), String)

  # [`RDoc::SingleClass`](https://docs.ruby-lang.org/en/2.7.0/RDoc/SingleClass.html)
  # type
  SINGLE = T.let(T.unsafe(nil), String)

  # Creates a new
  # [`Ruby`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/Ruby.html) parser.
  def self.new(top_level, file_name, content, options, stats); end

  # Look for the first comment in a file that isn't a shebang line.
  def collect_first_comment; end

  def consume_trailing_spaces; end

  def create_attr(container, single, name, rw, comment); end

  def create_module_alias(container, constant, rhs_name); end

  # Aborts with `msg`
  def error(msg); end

  # Looks for a true or false token.
  def get_bool; end

  # Look for the name of a class of module (optionally with a leading
  # :   or
  # with
  # :   separated named) and return the ultimate name, the associated
  #
  # container, and the given name (with the ::).
  def get_class_or_module(container, ignore_constants = _); end

  # Return a superclass, which can be either a constant of an expression
  def get_class_specification; end

  # Parse a constant, which might be qualified by one or more class or module
  # names
  def get_constant; end

  def get_constant_with_optional_parens; end

  def get_end_token(tk); end

  def get_method_container(container, name_t); end

  # Extracts a name or symbol from the token stream.
  def get_symbol_or_name; end

  def get_tkread_clean(pattern, replacement); end

  def get_visibility_information(tk, single); end

  # Look for directives in a normal comment block:
  #
  # ```ruby
  # # :stopdoc:
  # # Don't display comment from this point forward
  # ```
  #
  # This routine modifies its `comment` parameter.
  def look_for_directives_in(container, comment); end

  # Adds useful info about the parser to `message`
  def make_message(message); end

  # Creates a comment with the correct format
  def new_comment(comment); end

  # Parses an `alias` in `context` with `comment`
  def parse_alias(context, single, tk, comment); end

  # Creates an
  # [`RDoc::Attr`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Attr.html) for the
  # name following `tk`, setting the comment to `comment`.
  def parse_attr(context, single, tk, comment); end

  # Creates an
  # [`RDoc::Attr`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Attr.html) for each
  # attribute listed after `tk`, setting the comment for each to `comment`.
  def parse_attr_accessor(context, single, tk, comment); end

  # Extracts call parameters from the token stream.
  def parse_call_parameters(tk); end

  # Parses a class in `context` with `comment`
  def parse_class(container, single, tk, comment); end

  def parse_class_regular(container, declaration_context, single, name_t, given_name, comment); end

  def parse_class_singleton(container, name, comment); end

  # Generates an RDoc::Method or
  # [`RDoc::Attr`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Attr.html) from
  # `comment` by looking for :method: or :attr: directives in `comment`.
  def parse_comment(container, tk, comment); end

  def parse_comment_attr(container, type, name, comment); end

  def parse_comment_ghost(container, text, name, column, line_no, comment); end

  # Creates an RDoc::Method on `container` from `comment` if there is a
  # Signature section in the comment
  def parse_comment_tomdoc(container, tk, comment); end

  # Parses a constant in `context` with `comment`. If `ignore_constants` is
  # true, no found constants will be added to
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html).
  def parse_constant(container, tk, comment, ignore_constants = _); end

  def parse_constant_body(container, constant, is_array_or_hash); end

  # Parses a
  # [`Module#private_constant`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-private_constant)
  # or
  # [`Module#public_constant`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-public_constant)
  # call from `tk`.
  def parse_constant_visibility(container, single, tk); end

  def parse_extend_or_include(klass, container, comment); end

  def parse_identifier(container, single, tk, comment); end

  # Parses a meta-programmed attribute and creates an
  # [`RDoc::Attr`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Attr.html).
  #
  # To create foo and bar attributes on class C with comment "My attributes":
  #
  # ```ruby
  # class C
  #
  #   ##
  #   # :attr:
  #   #
  #   # My attributes
  #
  #   my_attr :foo, :bar
  #
  # end
  # ```
  #
  # To create a foo attribute on class C with comment "My attribute":
  #
  # ```ruby
  # class C
  #
  #   ##
  #   # :attr: foo
  #   #
  #   # My attribute
  #
  #   my_attr :foo, :bar
  #
  # end
  # ```
  def parse_meta_attr(context, single, tk, comment); end

  # Parses a meta-programmed method
  def parse_meta_method(container, single, tk, comment); end

  def parse_meta_method_name(comment, tk); end

  def parse_meta_method_params(container, single, meth, tk, comment); end

  # Parses a normal method defined by `def`
  def parse_method(container, single, tk, comment); end

  # Parses a method that needs to be ignored.
  def parse_method_dummy(container); end

  def parse_method_name(container); end

  def parse_method_name_regular(container, name_t); end

  def parse_method_name_singleton(container, name_t); end

  # Extracts `yield` parameters from `method`
  def parse_method_or_yield_parameters(method = _, modifiers = _); end

  # Capture the method's parameters. Along the way, look for a comment
  # containing:
  #
  # ```ruby
  # # yields: ....
  # ```
  #
  # and add this as the block\_params for the method
  def parse_method_parameters(method); end

  # Parses the parameters and body of `meth`
  def parse_method_params_and_body(container, single, meth, added_container); end

  # Parses an
  # [`RDoc::NormalModule`](https://docs.ruby-lang.org/en/2.7.0/RDoc/NormalModule.html)
  # in `container` with `comment`
  def parse_module(container, single, tk, comment); end

  # Parses an
  # [`RDoc::Require`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Require.html) in
  # `context` containing `comment`
  def parse_require(context, comment); end

  # Parses a rescue
  def parse_rescue; end

  # The core of the
  # [`Ruby`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/Ruby.html) parser.
  def parse_statements(container, single = _, current_method = _, comment = _); end

  # Parse up to `no` symbol arguments
  def parse_symbol_arg(no = _); end

  def parse_symbol_arg_paren(no); end

  def parse_symbol_arg_space(no, tk); end

  # Returns symbol text from the next token
  def parse_symbol_in_arg; end

  # Parses statements in the top-level `container`
  def parse_top_level_statements(container); end

  # Determines the visibility in `container` from `tk`
  def parse_visibility(container, single, tk); end

  # Determines the block parameter for `context`
  def parse_yield(context, single, tk, method); end

  # Directives are modifier comments that can appear after class, module, or
  # method names. For example:
  #
  # ```
  # def fred # :yields: a, b
  # ```
  #
  # or:
  #
  # ```
  # class MyClass # :nodoc:
  # ```
  #
  # We return the directive name and any parameters as a two element array if
  # the name is in `allowed`. A directive can be found anywhere up to the end of
  # the current line.
  def read_directive(allowed); end

  # Handles directives following the definition for `context` (any
  # [`RDoc::CodeObject`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html))
  # if the directives are `allowed` at this point.
  #
  # See also RDoc::Markup::PreProcess#handle\_directive
  def read_documentation_modifiers(context, allowed); end

  def record_location(container); end

  # Scans this
  # [`Ruby`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/Ruby.html) file for
  # [`Ruby`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/Ruby.html)
  # constructs
  def scan; end

  # skip the var [in] part of a 'for' statement
  def skip_for_variable; end

  # Skips the next method in `container`
  def skip_method(container); end

  # while, until, and for have an optional do
  def skip_optional_do_after_expression; end

  # Skip spaces until a comment is found
  def skip_tkspace_comment(skip_nl = _); end

  def suppress_parents(container, ancestor); end

  def tk_nl?(tk); end

  def update_visibility(container, vis_type, vis, singleton); end

  # Prints `message` to +$stderr+ unless we're being quiet
  def warn(message); end
end

# Collection of methods for writing parsers
module RDoc::Parser::RubyTools
  # Adds a token listener `obj`, but you should probably use
  # [`token_listener`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/RubyTools.html#method-i-token_listener)
  def add_token_listener(obj); end

  # Fetches the next token from the scanner
  def get_tk; end

  # Reads and returns all tokens up to one of `tokens`. Leaves the matched token
  # in the token list.
  def get_tk_until(*tokens); end

  # Retrieves a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # representation of the read tokens
  def get_tkread; end

  # Peek equivalent for
  # [`get_tkread`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/RubyTools.html#method-i-get_tkread)
  def peek_read; end

  # Peek at the next token, but don't remove it from the stream
  def peek_tk; end

  # Removes the token listener `obj`
  def remove_token_listener(obj); end

  # Resets the tools
  def reset; end

  # Skips whitespace tokens including newlines
  def skip_tkspace(skip_nl = _); end

  def tk_nl?(tk); end

  # Has `obj` listen to tokens
  def token_listener(obj); end

  # Returns `tk` to the scanner
  def unget_tk(tk); end
end

# Parse a non-source file. We basically take the whole thing as one big comment.
class RDoc::Parser::Simple < ::RDoc::Parser
  include(::RDoc::Parser::Text)

  # Prepare to parse a plain file
  def self.new(top_level, file_name, content, options, stats); end

  def content; end

  # Removes the encoding magic comment from `text`
  def remove_coding_comment(text); end

  # Removes private comments.
  #
  # Unlike
  # [`RDoc::Comment#remove_private`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Comment.html#method-i-remove_private)
  # this implementation only looks for two dashes at the beginning of the line.
  # Three or more dashes are considered to be a rule and ignored.
  def remove_private_comment(comment); end

  # Extract the file contents and attach them to the TopLevel as a comment
  def scan; end
end

# Indicates this parser is text and doesn't contain code constructs.
#
# Include this module in a
# [`RDoc::Parser`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser.html)
# subclass to make it show up as a file, not as part of a class or module.
module RDoc::Parser::Text; end

# [`RDoc::RD`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RD.html) implements the
# [`RD`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RD.html) format from the
# rdtool gem.
#
# To choose [`RD`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RD.html) as your
# only default format see [Saved Options at
# `RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#class-RDoc::Options-label-Saved+Options)
# for instructions on setting up a `.doc_options` file to store your project
# default.
#
# ## LICENSE
#
# The grammar that produces RDoc::RD::BlockParser and RDoc::RD::InlineParser is
# included in [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) under the
# Ruby License.
#
# You can find the original source for rdtool at
# https://github.com/uwabami/rdtool/
#
# You can use, re-distribute or change these files under Ruby's License or GPL.
#
# 1.  You may make and give away verbatim copies of the source form of the
#     software without restriction, provided that you duplicate all of the
#     original copyright notices and associated disclaimers.
#
# 2.  You may modify your copy of the software in any way, provided that you do
#     at least ONE of the following:
#
#     1.  place your modifications in the Public Domain or otherwise make them
#         Freely Available, such as by posting said modifications to Usenet or
#         an equivalent medium, or by allowing the author to include your
#         modifications in the software.
#
#     2.  use the modified software only within your corporation or
#         organization.
#
#     3.  give non-standard binaries non-standard names, with instructions on
#         where to get the original software distribution.
#
#     4.  make other distribution arrangements with the author.
#
#
# 3.  You may distribute the software in object code or binary form, provided
#     that you do at least ONE of the following:
#
#     1.  distribute the binaries and library files of the software, together
#         with instructions (in the manual page or equivalent) on where to get
#         the original distribution.
#
#     2.  accompany the distribution with the machine-readable source of the
#         software.
#
#     3.  give non-standard binaries non-standard names, with instructions on
#         where to get the original software distribution.
#
#     4.  make other distribution arrangements with the author.
#
#
# 4.  You may modify and include the part of the software into any other
#     software (possibly commercial). But some files in the distribution are not
#     written by the author, so that they are not under these terms.
#
#     For the list of those files and their copying conditions, see the file
#     [LEGAL](https://docs.ruby-lang.org/en/2.7.0/LEGAL.html).
#
# 5.  The scripts and library files supplied as input to or produced as output
#     from the software do not automatically fall under the copyright of the
#     software, but belong to whomever generated them, and may be sold
#     commercially, and may be aggregated with this software.
#
# 6.  THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
#     WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
#     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
class RDoc::RD
  # Parses `rd` source and returns an RDoc::Markup::Document. If the `=begin` or
  # `=end` lines are missing they will be added.
  def self.parse(rd); end
end

# [`RD`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RD.html) format parser for
# headings, paragraphs, lists, verbatim sections that exist as blocks.
class RDoc::RD::BlockParser < ::Racc::Parser
  MARK_TO_LEVEL = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  Racc_arg = T.let(T.unsafe(nil), T::Array[T.untyped])

  Racc_token_to_s_table = T.let(T.unsafe(nil), T::Array[T.untyped])

  TMPFILE = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Creates a new
  # [`RDoc::RD::BlockParser`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RD/BlockParser.html).
  # Use
  # [`parse`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RD/BlockParser.html#method-i-parse)
  # to parse an rd-format document.
  def self.new; end

  def _reduce_1(val, _values, result); end

  def _reduce_10(val, _values, result); end

  def _reduce_11(val, _values, result); end

  def _reduce_12(val, _values, result); end

  def _reduce_13(val, _values, result); end

  def _reduce_14(val, _values, result); end

  def _reduce_15(val, _values, result); end

  def _reduce_16(val, _values, result); end

  def _reduce_17(val, _values, result); end

  def _reduce_18(val, _values, result); end

  def _reduce_19(val, _values, result); end

  def _reduce_2(val, _values, result); end

  def _reduce_20(val, _values, result); end

  def _reduce_21(val, _values, result); end

  def _reduce_22(val, _values, result); end

  def _reduce_27(val, _values, result); end

  def _reduce_28(val, _values, result); end

  def _reduce_29(val, _values, result); end

  def _reduce_3(val, _values, result); end

  def _reduce_30(val, _values, result); end

  def _reduce_31(val, _values, result); end

  def _reduce_32(val, _values, result); end

  def _reduce_33(val, _values, result); end

  def _reduce_34(val, _values, result); end

  def _reduce_35(val, _values, result); end

  def _reduce_36(val, _values, result); end

  def _reduce_37(val, _values, result); end

  def _reduce_38(val, _values, result); end

  def _reduce_39(val, _values, result); end

  def _reduce_4(val, _values, result); end

  def _reduce_40(val, _values, result); end

  def _reduce_41(val, _values, result); end

  def _reduce_42(val, _values, result); end

  def _reduce_43(val, _values, result); end

  def _reduce_44(val, _values, result); end

  def _reduce_45(val, _values, result); end

  def _reduce_46(val, _values, result); end

  def _reduce_47(val, _values, result); end

  def _reduce_48(val, _values, result); end

  def _reduce_49(val, _values, result); end

  def _reduce_5(val, _values, result); end

  def _reduce_50(val, _values, result); end

  def _reduce_51(val, _values, result); end

  def _reduce_52(val, _values, result); end

  def _reduce_54(val, _values, result); end

  def _reduce_55(val, _values, result); end

  def _reduce_57(val, _values, result); end

  def _reduce_6(val, _values, result); end

  def _reduce_62(val, _values, result); end

  def _reduce_63(val, _values, result); end

  def _reduce_64(val, _values, result); end

  def _reduce_65(val, _values, result); end

  def _reduce_66(val, _values, result); end

  def _reduce_67(val, _values, result); end

  def _reduce_68(val, _values, result); end

  def _reduce_69(val, _values, result); end

  def _reduce_71(val, _values, result); end

  def _reduce_72(val, _values, result); end

  def _reduce_8(val, _values, result); end

  def _reduce_9(val, _values, result); end

  def _reduce_none(val, _values, result); end

  # Adds footnote `content` to the document
  def add_footnote(content); end

  # Adds label `label` to the document
  def add_label(label); end

  # Retrieves the content of `values` as a single
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html)
  def content(values); end

  # Footnotes for this document
  def footnotes; end

  # Path to find included files in
  def include_path; end

  # Path to find included files in
  def include_path=(_); end

  # Labels for items in this document
  def labels; end

  # Current line number
  def line_index; end

  def next_token; end

  # Raises a ParseError when invalid formatting is found
  def on_error(et, ev, _values); end

  # Creates a paragraph for `value`
  def paragraph(value); end

  # Parses `src` and returns an
  # [`RDoc::Markup::Document`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Document.html).
  def parse(src); end
end

# [`Inline`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RD/Inline.html) keeps
# track of markup and labels to create proper links.
class RDoc::RD::Inline
  # Creates a new
  # [`Inline`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RD/Inline.html) for
  # `rdoc` and `reference`.
  #
  # `rdoc` may be another
  # [`Inline`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RD/Inline.html) or a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html). If `reference`
  # is not given it will use the text from `rdoc`.
  def self.new(rdoc, reference); end

  def ==(other); end

  # Appends `more` to this inline. `more` may be a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) or another
  # [`Inline`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RD/Inline.html).
  def append(more); end

  def inspect; end

  # The markup of this reference in
  # [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) format
  def rdoc; end

  # The text of the reference
  def reference; end

  # The markup of this reference in
  # [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) format
  def to_s; end

  # Creates a new
  # [`Inline`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RD/Inline.html) for
  # `rdoc` and `reference`.
  #
  # `rdoc` may be another
  # [`Inline`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RD/Inline.html) or a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html). If `reference`
  # is not given it will use the text from `rdoc`.
  def self.new(rdoc, reference = _); end
end

# [`RD`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RD.html) format parser for
# inline markup such as emphasis, links, footnotes, etc.
class RDoc::RD::InlineParser < ::Racc::Parser
  BACK_SLASH = T.let(T.unsafe(nil), String)

  BACK_SLASH_RE = T.let(T.unsafe(nil), Regexp)

  BAR = T.let(T.unsafe(nil), String)

  BAR_RE = T.let(T.unsafe(nil), Regexp)

  CODE_CLOSE = T.let(T.unsafe(nil), String)

  CODE_CLOSE_RE = T.let(T.unsafe(nil), Regexp)

  CODE_OPEN = T.let(T.unsafe(nil), String)

  CODE_OPEN_RE = T.let(T.unsafe(nil), Regexp)

  EM_CLOSE = T.let(T.unsafe(nil), String)

  EM_CLOSE_RE = T.let(T.unsafe(nil), Regexp)

  EM_OPEN = T.let(T.unsafe(nil), String)

  EM_OPEN_RE = T.let(T.unsafe(nil), Regexp)

  FOOTNOTE_CLOSE = T.let(T.unsafe(nil), String)

  FOOTNOTE_CLOSE_RE = T.let(T.unsafe(nil), Regexp)

  FOOTNOTE_OPEN = T.let(T.unsafe(nil), String)

  FOOTNOTE_OPEN_RE = T.let(T.unsafe(nil), Regexp)

  INDEX_CLOSE = T.let(T.unsafe(nil), String)

  INDEX_CLOSE_RE = T.let(T.unsafe(nil), Regexp)

  INDEX_OPEN = T.let(T.unsafe(nil), String)

  INDEX_OPEN_RE = T.let(T.unsafe(nil), Regexp)

  KBD_CLOSE = T.let(T.unsafe(nil), String)

  KBD_CLOSE_RE = T.let(T.unsafe(nil), Regexp)

  KBD_OPEN = T.let(T.unsafe(nil), String)

  KBD_OPEN_RE = T.let(T.unsafe(nil), Regexp)

  OTHER_RE = T.let(T.unsafe(nil), Regexp)

  QUOTE = T.let(T.unsafe(nil), String)

  QUOTE_RE = T.let(T.unsafe(nil), Regexp)

  REF_CLOSE = T.let(T.unsafe(nil), String)

  REF_CLOSE_RE = T.let(T.unsafe(nil), Regexp)

  REF_OPEN = T.let(T.unsafe(nil), String)

  REF_OPEN_RE = T.let(T.unsafe(nil), Regexp)

  Racc_arg = T.let(T.unsafe(nil), T::Array[T.untyped])

  Racc_token_to_s_table = T.let(T.unsafe(nil), T::Array[T.untyped])

  SLASH = T.let(T.unsafe(nil), String)

  SLASH_RE = T.let(T.unsafe(nil), Regexp)

  URL = T.let(T.unsafe(nil), String)

  URL_RE = T.let(T.unsafe(nil), Regexp)

  VAR_CLOSE = T.let(T.unsafe(nil), String)

  VAR_CLOSE_RE = T.let(T.unsafe(nil), Regexp)

  VAR_OPEN = T.let(T.unsafe(nil), String)

  VAR_OPEN_RE = T.let(T.unsafe(nil), Regexp)

  VERB_CLOSE = T.let(T.unsafe(nil), String)

  VERB_CLOSE_RE = T.let(T.unsafe(nil), Regexp)

  VERB_OPEN = T.let(T.unsafe(nil), String)

  VERB_OPEN_RE = T.let(T.unsafe(nil), Regexp)

  # Creates a new parser for inline markup in the rd format. The `block_parser`
  # is used to for footnotes and labels in the inline text.
  def self.new(block_parser); end

  def _reduce_101(val, _values, result); end

  def _reduce_102(val, _values, result); end

  def _reduce_109(val, _values, result); end

  def _reduce_111(val, _values, result); end

  def _reduce_113(val, _values, result); end

  def _reduce_114(val, _values, result); end

  def _reduce_115(val, _values, result); end

  def _reduce_13(val, _values, result); end

  def _reduce_136(val, _values, result); end

  def _reduce_14(val, _values, result); end

  def _reduce_15(val, _values, result); end

  def _reduce_16(val, _values, result); end

  def _reduce_17(val, _values, result); end

  def _reduce_18(val, _values, result); end

  def _reduce_19(val, _values, result); end

  def _reduce_2(val, _values, result); end

  def _reduce_20(val, _values, result); end

  def _reduce_21(val, _values, result); end

  def _reduce_22(val, _values, result); end

  def _reduce_23(val, _values, result); end

  def _reduce_24(val, _values, result); end

  def _reduce_25(val, _values, result); end

  def _reduce_26(val, _values, result); end

  def _reduce_27(val, _values, result); end

  def _reduce_29(val, _values, result); end

  def _reduce_3(val, _values, result); end

  def _reduce_30(val, _values, result); end

  def _reduce_31(val, _values, result); end

  def _reduce_32(val, _values, result); end

  def _reduce_33(val, _values, result); end

  def _reduce_34(val, _values, result); end

  def _reduce_36(val, _values, result); end

  def _reduce_37(val, _values, result); end

  def _reduce_38(val, _values, result); end

  def _reduce_39(val, _values, result); end

  def _reduce_40(val, _values, result); end

  def _reduce_41(val, _values, result); end

  def _reduce_43(val, _values, result); end

  def _reduce_44(val, _values, result); end

  def _reduce_45(val, _values, result); end

  def _reduce_46(val, _values, result); end

  def _reduce_57(val, _values, result); end

  def _reduce_58(val, _values, result); end

  def _reduce_59(val, _values, result); end

  def _reduce_60(val, _values, result); end

  def _reduce_62(val, _values, result); end

  def _reduce_64(val, _values, result); end

  def _reduce_78(val, _values, result); end

  def _reduce_none(val, _values, result); end

  # Creates a new
  # [`RDoc::RD::Inline`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RD/Inline.html)
  # for the `rdoc` markup and the raw `reference`
  def inline(rdoc, reference = _); end

  # Returns the next token from the inline text
  def next_token; end

  # Returns words following an error
  def next_words_on_error; end

  # Raises a ParseError when invalid formatting is found
  def on_error(et, ev, values); end

  # Parses the `inline` text from
  # [`RD`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RD.html) format into
  # [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) format.
  def parse(inline); end

  # Returns words before the error
  def prev_words_on_error(ev); end
end

# This is the driver for generating
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RDoc.html) output. It
# handles file parsing and generation of output.
#
# To use this class to generate
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RDoc.html) output via the
# API, the recommended way is:
#
# ```ruby
# rdoc = RDoc::RDoc.new
# options = rdoc.load_options # returns an RDoc::Options instance
# # set extra options
# rdoc.document options
# ```
#
# You can also generate output like the `rdoc` executable:
#
# ```ruby
# rdoc = RDoc::RDoc.new
# rdoc.document argv
# ```
#
# Where `argv` is an array of strings, each corresponding to an argument you'd
# give rdoc on the command line. See `rdoc --help` for details.
class RDoc::RDoc
  # This is the list of supported output generators
  GENERATORS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # Creates a new
  # [`RDoc::RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RDoc.html) instance.
  # Call document to parse files and generate documentation.
  def self.new; end

  # Generates documentation or a coverage report depending upon the settings in
  # `options`.
  #
  # `options` can be either an
  # [`RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html)
  # instance or an array of strings equivalent to the strings that would be
  # passed on the command line like `%w[-q -o doc -t My\ Doc\ Title]`. document
  # will automatically call
  # [`RDoc::Options#finish`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#method-i-finish)
  # if an options instance was given.
  #
  # For a list of options, see either
  # [`RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html) or
  # `rdoc --help`.
  #
  # By default, output will be stored in a directory called "doc" below the
  # current directory, so make sure you're somewhere writable before invoking.
  def document(options); end

  # Report an error message and exit
  def error(msg); end

  def exclude; end

  def exclude=(_); end

  # Gathers a set of parseable files from the files and directories listed in
  # `files`.
  def gather_files(files); end

  # Generates documentation for `file_info` (from parse\_files) into the output
  # dir using the generator selected by the
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) options
  def generate; end

  # Generator instance used for creating output
  def generator; end

  # Generator instance used for creating output
  def generator=(_); end

  # Turns [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) from stdin
  # into HTML
  def handle_pipe; end

  # Installs a siginfo handler that prints the current filename.
  def install_siginfo_handler; end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of files and their
  # last modified times.
  def last_modified; end

  # Return a list of the files to be processed in a directory. We know that this
  # directory doesn't have a .document file, so we're looking for real files.
  # However we may well contain subdirectories which must be tested for
  # .document files.
  def list_files_in_directory(dir); end

  # Loads options from .rdoc\_options if the file exists, otherwise creates a
  # new [`RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html)
  # instance.
  def load_options; end

  # Given a list of files and directories, create a list of all the Ruby files
  # they contain.
  #
  # If `force_doc` is true we always add the given files, if false, only add
  # files that we guarantee we can parse. It is true when looking at files given
  # on the command line, false when recursing through subdirectories.
  #
  # The effect of this is that if you want a file with a non-standard extension
  # parsed, you must name it explicitly.
  def normalized_file_list(relative_files, force_doc = _, exclude_pattern = _); end

  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RDoc.html) options
  def options; end

  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RDoc.html) options
  def options=(_); end

  # Return the path name of the flag file in an output directory.
  def output_flag_file(op_dir); end

  # The .document file contains a list of file and directory name patterns,
  # representing candidates for documentation. It may also contain comments
  # (starting with '#')
  def parse_dot_doc_file(in_dir, filename); end

  # Parses `filename` and returns an
  # [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
  def parse_file(filename); end

  # Parse each file on the command line, recursively entering directories.
  def parse_files(files); end

  # Removes a siginfo handler and replaces the previous
  def remove_siginfo_handler; end

  # Removes file extensions known to be unparseable from `files` and TAGS files
  # for emacs and vim.
  def remove_unparseable(files); end

  # Create an output dir if it doesn't exist. If it does exist, but doesn't
  # contain the flag file `created.rid` then we refuse to use it, as we may
  # clobber some manually generated documentation
  def setup_output_dir(dir, force); end

  # Accessor for statistics. Available after each call to
  # [`parse_files`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html#method-i-parse_files)
  def stats; end

  # The current documentation store
  def store; end

  # Sets the current documentation tree to `store` and sets the store's rdoc
  # driver to this instance.
  def store=(store); end

  # Update the flag file in an output directory.
  def update_output_dir(op_dir, time, last = _); end

  # Add `klass` that can generate output after parsing
  def self.add_generator(klass); end

  # Active [`RDoc::RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RDoc.html)
  # instance
  def self.current; end

  # Sets the active
  # [`RDoc::RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RDoc.html) instance
  def self.current=(rdoc); end
end

# Namespace for the ri command line tool's implementation.
#
# See `ri --help` for details.
module RDoc::RI; end

# The RI driver implements the command-line ri tool.
#
# The driver supports:
# *   loading RI data from:
#     *   Ruby's standard library
#     *   RubyGems
#     *   ~/.rdoc
#     *   A user-supplied directory
#
# *   Paging output (uses RI\_PAGER environment variable, PAGER environment
#     variable or the less, more and pager programs)
# *   Interactive mode with tab-completion
# *   Abbreviated names (ri Zl shows
#     [`Zlib`](https://docs.ruby-lang.org/en/2.6.0/Zlib.html) documentation)
# *   Colorized output
# *   Merging output from multiple RI data sources
class RDoc::RI::Driver
  # Creates a new driver using `initial_options` from
  # [`::process_args`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RI/Driver.html#method-c-process_args)
  def self.new(initial_options = _); end

  # Adds paths for undocumented classes `also_in` to `out`
  def add_also_in(out, also_in); end

  # Adds a class header to `out` for class `name` which is described in
  # `classes`.
  def add_class(out, name, classes); end

  # Adds `extends` to `out`
  def add_extends(out, extends); end

  # Adds a list of `extensions` to this module of the given `type` to `out`.
  # [`add_includes`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RI/Driver.html#method-i-add_includes)
  # and
  # [`add_extends`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RI/Driver.html#method-i-add_extends)
  # call this, so you should use those directly.
  def add_extension_modules(out, type, extensions); end

  def add_extension_modules_multiple(out, store, modules); end

  def add_extension_modules_single(out, store, include); end

  # Adds "(from ...)" to `out` for `store`
  def add_from(out, store); end

  # Adds `includes` to `out`
  def add_includes(out, includes); end

  # Looks up the method `name` and adds it to `out`
  def add_method(out, name); end

  # Adds documentation for all methods in `klass` to `out`
  def add_method_documentation(out, klass); end

  # Adds a list of `methods` to `out` with a heading of `name`
  def add_method_list(out, methods, name); end

  # Returns ancestor classes of `klass`
  def ancestors_of(klass); end

  def check_did_you_mean; end

  def class_cache; end

  # Builds a
  # [`RDoc::Markup::Document`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Document.html)
  # from `found`, `klasess` and `includes`
  def class_document(name, found, klasses, includes, extends); end

  def class_document_comment(out, comment); end

  def class_document_constants(out, klass); end

  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) mapping a known
  # class or module to the stores it can be loaded from
  def classes; end

  # Returns the stores wherein `name` is found along with the classes, extends
  # and includes that match it
  def classes_and_includes_and_extends_for(name); end

  # Completes `name` based on the caches. For
  # [`Readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html)
  def complete(name); end

  def complete_klass(name, klass, selector, method, completions); end

  def complete_method(name, klass, selector, completions); end

  # Converts `document` to text and writes it to the pager
  def display(document); end

  # Outputs formatted RI data for class `name`. Groups undocumented classes
  def display_class(name); end

  # Outputs formatted RI data for method `name`
  def display_method(name); end

  # Outputs formatted RI data for the class or method `name`.
  #
  # Returns true if `name` was found, false if it was not an alternative could
  # be guessed, raises an error if `name` couldn't be guessed.
  def display_name(name); end

  # Displays each name in `name`
  def display_names(names); end

  # Outputs formatted RI data for page `name`.
  def display_page(name); end

  # Outputs a formatted RI page list for the pages in `store`.
  def display_page_list(store, pages = _, search = _); end

  # Expands abbreviated klass `klass` into a fully-qualified class. "Zl::Da"
  # will be expanded to
  # [`Zlib::DataError`](https://docs.ruby-lang.org/en/2.6.0/Zlib/DataError.html).
  def expand_class(klass); end

  # Expands the class portion of `name` into a fully-qualified class. See
  # [`expand_class`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RI/Driver.html#method-i-expand_class).
  def expand_name(name); end

  # Filters the methods in `found` trying to find a match for `name`.
  def filter_methods(found, name); end

  # Yields items matching `name` including the store they were found in, the
  # class being searched for, the class they were found in (an ancestor) the
  # types of methods to look up (from
  # [`method_type`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RI/Driver.html#method-i-method_type)),
  # and the method name being searched for
  def find_methods(name); end

  # Finds the given `pager` for jruby. Returns an
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) if `pager` was found.
  #
  # Returns false if `pager` does not exist.
  #
  # Returns nil if the jruby JVM doesn't support ProcessBuilder redirection (1.6
  # and older).
  def find_pager_jruby(pager); end

  # Finds a store that matches `name` which can be the name of a gem, "ruby",
  # "home" or "site".
  #
  # See also
  # [`RDoc::Store#source`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Store.html#method-i-source)
  def find_store(name); end

  # Creates a new
  # [`RDoc::Markup::Formatter`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Formatter.html).
  # If a formatter is given with -f, use it. If we're outputting to a pager, use
  # bs, otherwise ansi.
  def formatter(io); end

  # Is `file` in [ENV]('PATH')?
  def in_path?(file); end

  # Runs ri interactively using
  # [`Readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html) if it is
  # available.
  def interactive; end

  # Lists classes known to ri starting with `names`. If `names` is empty all
  # known classes are shown.
  def list_known_classes(names = _); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  # methods matching `name`
  def list_methods_matching(name); end

  # Loads RI data for method `name` on `klass` from `store`. `type` and `cache`
  # indicate if it is a class or instance method.
  def load_method(store, cache, klass, type, name); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of RI
  # data for methods matching `name`
  def load_methods_matching(name); end

  # Returns a filtered list of methods matching `name`
  def lookup_method(name); end

  # Builds a
  # [`RDoc::Markup::Document`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Markup/Document.html)
  # from `found`, `klasses` and `includes`
  def method_document(name, filtered); end

  # Returns the type of method (:both, :instance, :class) for `selector`
  def method_type(selector); end

  # Returns a regular expression for `name` that will match an RDoc::AnyMethod's
  # name.
  def name_regexp(name); end

  # Paginates output through a pager program.
  def page; end

  # Are we using a pager?
  def paging?; end

  # Extracts the class, selector and method name parts from `name` like
  # Foo::Bar#baz.
  #
  # NOTE: Given Foo::Bar, Bar is considered a class even though it may be a
  # method
  def parse_name(name); end

  def render_class(out, store, klass, also_in); end

  def render_method(out, store, method, name); end

  def render_method_arguments(out, arglists); end

  def render_method_comment(out, method); end

  def render_method_superclass(out, method); end

  # Looks up and displays ri data according to the options given.
  def run; end

  # Sets up a pager program to pass output through. Tries the RI\_PAGER and
  # PAGER environment variables followed by pager, less then more.
  def setup_pager; end

  # Show all method documentation following a class or module
  def show_all; end

  # Show all method documentation following a class or module
  def show_all=(_); end

  # Starts a [`WEBrick`](https://docs.ruby-lang.org/en/2.6.0/WEBrick.html)
  # server for ri.
  def start_server; end

  # An RDoc::RI::Store for each entry in the RI path
  def stores; end

  # An RDoc::RI::Store for each entry in the RI path
  def stores=(_); end

  # Controls the user of the pager vs $stdout
  def use_stdout; end

  # Controls the user of the pager vs $stdout
  def use_stdout=(_); end

  # Default options for ri
  def self.default_options; end

  # Dump `data_path` using pp
  def self.dump(data_path); end

  # Parses `argv` and returns a
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) of options
  def self.process_args(argv); end

  # Runs the ri command line executable using `argv`
  def self.run(argv = _); end
end

# Base [`Driver`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RI/Driver.html) error
# class
class RDoc::RI::Driver::Error < ::RDoc::RI::Error; end

# Raised when a name isn't found in the ri data stores
class RDoc::RI::Driver::NotFoundError < ::RDoc::RI::Driver::Error
  def self.new(klass, suggestions = _); end

  def message; end

  # Name that wasn't found
  def name; end
end

# Base [`RI`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RI.html) error class
class RDoc::RI::Error < ::RDoc::Error; end

# The directories where ri data lives.
# [`Paths`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RI/Paths.html) can be
# enumerated via
# [`::each`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RI/Paths.html#method-c-each),
# or queried individually via
# [`::system_dir`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RI/Paths.html#method-c-system_dir),
# [`::site_dir`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RI/Paths.html#method-c-site_dir),
# [`::home_dir`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RI/Paths.html#method-c-home_dir)
# and
# [`::gem_dir`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RI/Paths.html#method-c-gem_dir).
module RDoc::RI::Paths
  BASE = T.let(T.unsafe(nil), String)

  HOMEDIR = T.let(T.unsafe(nil), String)

  # Iterates over each selected path yielding the directory and type.
  #
  # Yielded types:
  # :system
  # :   Where Ruby's ri data is stored. Yielded when `system` is true
  # :site
  # :   Where ri for installed libraries are stored. Yielded when `site` is
  #     true. Normally no ri data is stored here.
  # :home
  # :   ~/.rdoc. Yielded when `home` is true.
  # :gem
  # :   ri data for an installed gem. Yielded when `gems` is true.
  # :extra
  # :   ri data directory from the command line. Yielded for each entry in
  #     `extra_dirs`
  def self.each(system = _, site = _, home = _, gems = _, *extra_dirs); end

  # The ri directory for the gem with `gem_name`.
  def self.gem_dir(name, version); end

  # The latest installed gems' ri directories. `filter` can be :all or :latest.
  #
  # A `filter` :all includes all versions of gems and includes gems without ri
  # documentation.
  def self.gemdirs(filter = _); end

  # The location of the rdoc data in the user's home directory.
  #
  # Like ::system, ri data in the user's home directory is rare and predates
  # libraries distributed via RubyGems. ri data is rarely generated into this
  # directory.
  def self.home_dir; end

  # Returns existing directories from the selected documentation directories as
  # an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html).
  #
  # See also
  # [`::each`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RI/Paths.html#method-c-each)
  def self.path(system = _, site = _, home = _, gems = _, *extra_dirs); end

  # Returns selected documentation directories including nonexistent
  # directories.
  #
  # See also
  # [`::each`](https://docs.ruby-lang.org/en/2.6.0/RDoc/RI/Paths.html#method-c-each)
  def self.raw_path(system, site, home, gems, *extra_dirs); end

  # The location of ri data installed into the site dir.
  #
  # Historically this was available for documentation installed by Ruby
  # libraries predating RubyGems. It is unlikely to contain any content for
  # modern Ruby installations.
  def self.site_dir; end

  # The location of the built-in ri data.
  #
  # This data is built automatically when `make` is run when Ruby is installed.
  # If you did not install Ruby by hand you may need to install the
  # documentation yourself. Please consult the documentation for your package
  # manager or Ruby installer for details. You can also use the rdoc-data gem to
  # install system ri data for common versions of Ruby.
  def self.system_dir; end
end

# A file loaded by
# [`require`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-require)
class RDoc::Require < ::RDoc::CodeObject
  # Creates a new
  # [`Require`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Require.html) that
  # loads `name` with `comment`
  def self.new(name, comment); end

  def inspect; end

  # Name of the required file
  def name; end

  # Name of the required file
  def name=(_); end

  def to_s; end

  # The
  # [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
  # corresponding to this require, or `nil` if not found.
  def top_level; end
end

class RDoc::RipperStateLex
  EXPR_ARG = T.let(T.unsafe(nil), Integer)

  EXPR_ARG_ANY = T.let(T.unsafe(nil), Integer)

  EXPR_BEG = T.let(T.unsafe(nil), Integer)

  EXPR_BEG_ANY = T.let(T.unsafe(nil), Integer)

  EXPR_CLASS = T.let(T.unsafe(nil), Integer)

  EXPR_CMDARG = T.let(T.unsafe(nil), Integer)

  EXPR_DOT = T.let(T.unsafe(nil), Integer)

  EXPR_END = T.let(T.unsafe(nil), Integer)

  EXPR_ENDARG = T.let(T.unsafe(nil), Integer)

  EXPR_ENDFN = T.let(T.unsafe(nil), Integer)

  EXPR_END_ANY = T.let(T.unsafe(nil), Integer)

  EXPR_FITEM = T.let(T.unsafe(nil), Integer)

  EXPR_FNAME = T.let(T.unsafe(nil), Integer)

  EXPR_LABEL = T.let(T.unsafe(nil), Integer)

  EXPR_LABELED = T.let(T.unsafe(nil), Integer)

  EXPR_MID = T.let(T.unsafe(nil), Integer)

  EXPR_NONE = T.let(T.unsafe(nil), Integer)

  EXPR_VALUE = T.let(T.unsafe(nil), Integer)

  RIPPER_HAS_LEX_STATE = T.let(T.unsafe(nil), TrueClass)

  def self.new(code); end

  def get_squashed_tk; end

  def self.end?(token); end

  def self.parse(code); end
end

class RDoc::RipperStateLex::InnerStateLex < ::Ripper::Filter
  def self.new(code); end

  def each(&block); end

  def on_default(event, tok, data); end
end

# This is a [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html)
# servlet that allows you to browse ri documentation.
#
# You can show documentation through either `ri --server` or, with RubyGems 2.0
# or newer, `gem server`. For ri, the server runs on port 8214 by default. For
# RubyGems the server runs on port 8808 by default.
#
# You can use this servlet in your own project by mounting it on a
# [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) server:
#
# ```ruby
# require 'webrick'
#
# server = WEBrick::HTTPServer.new Port: 8000
#
# server.mount '/', RDoc::Servlet
# ```
#
# If you want to mount the servlet some other place than the root, provide the
# base path when mounting:
#
# ```ruby
# server.mount '/rdoc', RDoc::Servlet, '/rdoc'
# ```
class RDoc::Servlet < ::WEBrick::HTTPServlet::AbstractServlet
  # Creates a new [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html)
  # servlet.
  #
  # Use `mount_path` when mounting the servlet somewhere other than /.
  #
  # Use `extra_doc_dirs` for additional documentation directories.
  #
  # `server` is provided automatically by
  # [`WEBrick`](https://docs.ruby-lang.org/en/2.7.0/WEBrick.html) when mounting.
  # `stores` and `cache` are provided automatically by the servlet.
  def self.new(server, stores, cache, mount_path = _, extra_doc_dirs = _); end

  # Serves the asset at the path in `req` for `generator_name` via `res`.
  def asset(generator_name, req, res); end

  # Maps an asset type to its path on the filesystem
  def asset_dirs; end

  # GET request entry point. Fills in `res` for the path, etc. in `req`.
  def do_GET(req, res); end

  # Fills in `res` with the class, module or page for `req` from `store`.
  #
  # `path` is relative to the mount\_path and is used to determine the class,
  # module or page name (/RDoc/Servlet.html becomes
  # [`RDoc::Servlet`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Servlet.html)).
  # `generator` is used to create the page.
  def documentation_page(store, generator, path, req, res); end

  # Creates the [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) search
  # index on `res` for the given `store`. `generator` must respond to
  # json\_index to build. `req` is ignored.
  def documentation_search(store, generator, req, res); end

  # Returns the
  # [`RDoc::Store`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html) and
  # path relative to `mount_path` for documentation at `path`.
  def documentation_source(path); end

  # Generates an error page for the `exception` while handling `req` on `res`.
  def error(exception, req, res); end

  # Instantiates a Darkfish generator for `store`
  def generator_for(store); end

  # Handles the If-Modified-Since HTTP header on `req` for `path`. If the file
  # has not been modified a Not Modified response is returned. If the file has
  # been modified a Last-Modified header is added to `res`.
  def if_modified_since(req, res, path = _); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # installed documentation.
  #
  # Each entry contains the documentation name (gem name, 'Ruby Documentation',
  # etc.), the path relative to the mount point, whether the documentation
  # exists, the type of documentation (See RDoc::RI::Paths#each) and the
  # filesystem to the
  # [`RDoc::Store`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html) for the
  # documentation.
  def installed_docs; end

  # Returns a 404 page built by `generator` for `req` on `res`.
  def not_found(generator, req, res, message = _); end

  # An [`RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html)
  # instance used for rendering options
  def options; end

  # Enumerates the ri paths. See RDoc::RI::Paths#each
  def ri_paths(&block); end

  # Generates the root page on `res`. `req` is ignored.
  def root(req, res); end

  # Generates a search index for the root page on `res`. `req` is ignored.
  def root_search(req, res); end

  # Displays documentation for `req` on `res`, whether that be HTML or some
  # asset.
  def show_documentation(req, res); end

  # Returns an
  # [`RDoc::Store`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html) for the
  # given `source_name` ('ruby' or a gem name).
  def store_for(source_name); end

  def self.get_instance(server, *options); end
end

# A singleton class
class RDoc::SingleClass < ::RDoc::ClassModule
  # Adds the superclass to the included modules.
  def ancestors; end

  def aref_prefix; end

  # The definition of this singleton class, `class << MyClassName`
  def definition; end
end

# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) statistics collector
# which prints a summary and report of a project's documentation totals.
class RDoc::Stats
  include(::RDoc::Text)

  # Creates a new [`Stats`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Stats.html)
  # that will have `num_files`. `verbosity` defaults to 1 which will create an
  # RDoc::Stats::Normal outputter.
  def self.new(store, num_files, verbosity = _); end

  # Records the parsing of an alias `as`.
  def add_alias(as); end

  # Records the parsing of an attribute `attribute`
  def add_attribute(attribute); end

  # Records the parsing of a class `klass`
  def add_class(klass); end

  # Records the parsing of `constant`
  def add_constant(constant); end

  # Records the parsing of `file`
  def add_file(file); end

  # Records the parsing of `method`
  def add_method(method); end

  # Records the parsing of a module `mod`
  def add_module(mod); end

  # Call this to mark the beginning of parsing for display purposes
  def begin_adding; end

  # Calculates documentation totals and percentages for classes, modules,
  # constants, attributes and methods.
  def calculate; end

  # Output level for the coverage report
  def coverage_level; end

  # Sets coverage report level. Accepted values are:
  #
  # false or nil
  # :   No report
  # 0
  # :   Classes, modules, constants, attributes, methods
  # 1
  # :   Level 0 + method parameters
  def coverage_level=(level); end

  # Returns the length and number of undocumented items in `collection`.
  def doc_stats(collection); end

  # Call this to mark the end of parsing for display purposes
  def done_adding; end

  # Count of files parsed during parsing
  def files_so_far; end

  # The documentation status of this project. `true` when 100%, `false` when
  # less than 100% and `nil` when unknown.
  #
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) by calling
  # [`calculate`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Stats.html#method-i-calculate)
  def fully_documented?; end

  # A report that says you did a great job!
  def great_job; end

  # Total number of files found
  def num_files; end

  # Calculates the percentage of items documented.
  def percent_doc; end

  # Returns a report on which items are not documented
  def report; end

  # Returns a report on undocumented attributes in ClassModule `cm`
  def report_attributes(cm); end

  # Returns a report on undocumented items in ClassModule `cm`
  def report_class_module(cm); end

  # Returns a report on undocumented constants in ClassModule `cm`
  def report_constants(cm); end

  # Returns a report on undocumented methods in ClassModule `cm`
  def report_methods(cm); end

  # Returns a summary of the collected statistics.
  def summary; end

  # Determines which parameters in `method` were not documented. Returns a total
  # parameter count and an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of undocumented
  # methods.
  def undoc_params(method); end
end

# Stats printer that prints just the files being documented with a progress bar
class RDoc::Stats::Normal < ::RDoc::Stats::Quiet
  def begin_adding; end

  def done_adding; end

  # Prints a file with a progress bar
  def print_file(files_so_far, filename); end
end

# Stats printer that prints nothing
class RDoc::Stats::Quiet
  # Creates a new
  # [`Quiet`](https://docs.ruby-lang.org/en/2.6.0/RDoc/Stats/Quiet.html) that
  # will print nothing
  def self.new(num_files); end

  # Prints a message at the beginning of parsing
  def begin_adding(*_); end

  # Prints when [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) is done
  def done_adding(*_); end

  # Prints when an alias is added
  def print_alias(*_); end

  # Prints when an attribute is added
  def print_attribute(*_); end

  # Prints when a class is added
  def print_class(*_); end

  # Prints when a constant is added
  def print_constant(*_); end

  # Prints when a file is added
  def print_file(*_); end

  # Prints when a method is added
  def print_method(*_); end

  # Prints when a module is added
  def print_module(*_); end
end

# Stats printer that prints everything documented, including the documented
# status
class RDoc::Stats::Verbose < ::RDoc::Stats::Normal
  # Returns a marker for
  # [`RDoc::CodeObject`](https://docs.ruby-lang.org/en/2.6.0/RDoc/CodeObject.html)
  # `co` being undocumented
  def nodoc(co); end

  def print_alias(as); end

  def print_attribute(attribute); end

  def print_class(klass); end

  def print_constant(constant); end

  def print_file(files_so_far, file); end

  def print_method(method); end

  def print_module(mod); end
end

# A set of rdoc data for a single project (gem, path, etc.).
#
# The store manages reading and writing ri data for a project and maintains a
# cache of methods, classes and ancestors in the store.
#
# The store maintains a
# [`cache`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html#attribute-i-cache)
# of its contents for faster lookup. After adding items to the store it must be
# flushed using
# [`save_cache`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html#method-i-save_cache).
# The cache contains the following structures:
#
# ```ruby
# @cache = {
#   :ancestors        => {}, # class name => ancestor names
#   :attributes       => {}, # class name => attributes
#   :class_methods    => {}, # class name => class methods
#   :instance_methods => {}, # class name => instance methods
#   :modules          => [], # classes and modules in this store
#   :pages            => [], # page names
# }
# ```
class RDoc::Store
  # Creates a new [`Store`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html)
  # of `type` that will load or save to `path`
  def self.new(path = _, type = _); end

  # Adds `module` as an enclosure (namespace) for the given `variable` for C
  # files.
  def add_c_enclosure(variable, namespace); end

  # Adds C variables from an
  # [`RDoc::Parser::C`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Parser/C.html)
  def add_c_variables(c_parser); end

  # Adds the file with `name` as an
  # [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
  # to the store. Returns the created
  # [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html).
  def add_file(absolute_name, relative_name = _); end

  # Returns all classes discovered by
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
  def all_classes; end

  # Returns all classes and modules discovered by
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
  def all_classes_and_modules; end

  # All TopLevels known to
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
  def all_files; end

  # Returns all modules discovered by
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
  def all_modules; end

  # Ancestors cache accessor. Maps a klass name to an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of its ancestors
  # in this store. If Foo in this store inherits from
  # [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html),
  # [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html) won't be listed
  # (it will be included from ruby's ri store).
  def ancestors; end

  # Attributes cache accessor. Maps a class to an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of its attributes.
  def attributes; end

  # Maps C variables to class or module names for each parsed C file.
  def c_class_variables; end

  def c_enclosure_classes; end

  def c_enclosure_names; end

  # Maps C variables to singleton class names for each parsed C file.
  def c_singleton_class_variables; end

  # The contents of the
  # [`Store`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html)
  def cache; end

  # Path to the cache file
  def cache_path; end

  # Path to the ri data for `klass_name`
  def class_file(klass_name); end

  # [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) methods cache
  # accessor. Maps a class to an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of its class
  # methods (not full name).
  def class_methods; end

  # Path where data for `klass_name` will be stored (methods or class data)
  def class_path(klass_name); end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of all classes known
  # to [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
  def classes_hash; end

  def clean_cache_collection(collection); end

  # Prepares the [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) code
  # object tree for use by a generator.
  #
  # It finds unique classes/modules defined, and replaces classes/modules that
  # are aliases for another one by a copy with
  # [`RDoc::ClassModule#is_alias_for`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html#attribute-i-is_alias_for)
  # set.
  #
  # It updates the
  # [`RDoc::ClassModule#constant_aliases`](https://docs.ruby-lang.org/en/2.7.0/RDoc/ClassModule.html#attribute-i-constant_aliases)
  # attribute of "real" classes or modules.
  #
  # It also completely removes the classes and modules that should be removed
  # from the documentation and the methods that have a visibility below
  # `min_visibility`, which is the `--visibility` option.
  #
  # See also
  # [`RDoc::Context#remove_from_documentation?`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html#method-i-remove_from_documentation-3F)
  def complete(min_visibility); end

  # If true this [`Store`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html)
  # will not write any files
  def dry_run; end

  # If true this [`Store`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html)
  # will not write any files
  def dry_run=(_); end

  # The encoding of the contents in the
  # [`Store`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html)
  def encoding; end

  # The encoding of the contents in the
  # [`Store`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html)
  def encoding=(_); end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of all files known
  # to [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
  def files_hash; end

  # Finds the enclosure (namespace) for the given C `variable`.
  def find_c_enclosure(variable); end

  # Finds the class with `name` in all discovered classes
  def find_class_named(name); end

  # Finds the class with `name` starting in namespace `from`
  def find_class_named_from(name, from); end

  # Finds the class or module with `name`
  def find_class_or_module(name); end

  # Finds the file with `name` in all discovered files
  def find_file_named(name); end

  # Finds the module with `name` in all discovered modules
  def find_module_named(name); end

  # Returns the
  # [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
  # that is a text file and has the given `file_name`
  def find_text_page(file_name); end

  # Finds unique classes/modules defined in `all_hash`, and returns them as an
  # array. Performs the alias updates in `all_hash`: see ::complete.
  def find_unique(all_hash); end

  # Fixes the erroneous `BasicObject < Object` in 1.9.
  #
  # Because we assumed all classes without a stated superclass inherit from
  # [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html), we have the
  # above wrong inheritance.
  #
  # We fix [`BasicObject`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html)
  # right away if we are running in a Ruby version >= 1.9.
  def fix_basic_object_inheritance; end

  # Friendly rendition of
  # [`path`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html#attribute-i-path)
  def friendly_path; end

  def inspect; end

  # Instance methods cache accessor. Maps a class to an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of its instance
  # methods (not full name).
  def instance_methods; end

  # Loads all items from this store into memory. This recreates a documentation
  # tree for use by a generator
  def load_all; end

  # Loads cache file for this store
  def load_cache; end

  # Loads ri data for `klass_name` and hooks it up to this store.
  def load_class(klass_name); end

  # Loads ri data for `klass_name`
  def load_class_data(klass_name); end

  # Loads ri data for `method_name` in `klass_name`
  def load_method(klass_name, method_name); end

  # Loads ri data for `page_name`
  def load_page(page_name); end

  # Gets the main page for this
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) store. This page is
  # used as the root of the
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) server.
  def main; end

  # Sets the main page for this
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) store.
  def main=(page); end

  # Converts the variable => ClassModule map `variables` from a C parser into a
  # variable => class name map.
  def make_variable_map(variables); end

  # Path to the ri data for `method_name` in `klass_name`
  def method_file(klass_name, method_name); end

  # Modules cache accessor. An
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of all the module
  # (and class) names in the store.
  def module_names; end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of all modules known
  # to [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
  def modules_hash; end

  # Returns the
  # [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
  # that is a text file and has the given `name`
  def page(name); end

  # Path to the ri data for `page_name`
  def page_file(page_name); end

  # Path this store reads or writes
  def path; end

  # Path this store reads or writes
  def path=(_); end

  # The [`RDoc::RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RDoc.html)
  # driver for this parse tree. This allows classes consulting the documentation
  # tree to access user-set options, for example.
  def rdoc; end

  # The [`RDoc::RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/RDoc.html)
  # driver for this parse tree. This allows classes consulting the documentation
  # tree to access user-set options, for example.
  def rdoc=(_); end

  # Removes from `all_hash` the contexts that are nodoc or have no content.
  #
  # See
  # [`RDoc::Context#remove_from_documentation?`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Context.html#method-i-remove_from_documentation-3F)
  def remove_nodoc(all_hash); end

  # Saves all entries in the store
  def save; end

  # Writes the cache file for this store
  def save_cache; end

  # Writes the ri data for `klass` (or module)
  def save_class(klass); end

  # Writes the ri data for `method` on `klass`
  def save_method(klass, method); end

  # Writes the ri data for `page`
  def save_page(page); end

  # Source of the contents of this store.
  #
  # For a store from a gem the source is the gem name. For a store from the home
  # directory the source is "home". For system ri store (the standard library
  # documentation) the source is"ruby". For a store from the site ri directory
  # the store is "site". For other stores the source is the
  # [`path`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store.html#attribute-i-path).
  def source; end

  # Gets the title for this
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) store. This is used
  # as the title in each page on the
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) server
  def title; end

  # Sets the title page for this
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) store.
  def title=(title); end

  # Type of ri datastore this was loaded from. See RDoc::RI::Driver,
  # RDoc::RI::Paths.
  def type; end

  # Type of ri datastore this was loaded from. See RDoc::RI::Driver,
  # RDoc::RI::Paths.
  def type=(_); end

  # Returns the unique classes discovered by
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html).
  #
  # ::complete must have been called prior to using this method.
  def unique_classes; end

  # Returns the unique classes and modules discovered by
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html). ::complete must
  # have been called prior to using this method.
  def unique_classes_and_modules; end

  # Returns the unique modules discovered by
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html). ::complete must
  # have been called prior to using this method.
  def unique_modules; end
end

# Errors raised from loading or saving the store
class RDoc::Store::Error < ::RDoc::Error; end

# Raised when a stored file for a class, module, page or method is missing.
class RDoc::Store::MissingFileError < ::RDoc::Store::Error
  # Creates a new
  # [`MissingFileError`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store/MissingFileError.html)
  # for the missing `file` for the given `name` that should have been in the
  # `store`.
  def self.new(store, file, name); end

  # The file the
  # [`name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store/MissingFileError.html#attribute-i-name)
  # should be saved as
  def file; end

  def message; end

  # The name of the object the
  # [`file`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Store/MissingFileError.html#attribute-i-file)
  # would be loaded from
  def name; end

  # The store the file should exist in
  def store; end
end

# [`RDoc::Task`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Task.html) creates the
# following rake tasks to generate and clean up
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) output:
#
# rdoc
# :   Main task for this [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
#     task.
#
# clobber\_rdoc
# :   Delete all the rdoc files. This target is automatically added to the main
#     clobber target.
#
# rerdoc
# :   Rebuild the rdoc files from scratch, even if they are not out of date.
#
#
# Simple Example:
#
# ```ruby
# require 'rdoc/task'
#
# RDoc::Task.new do |rdoc|
#   rdoc.main = "README.rdoc"
#   rdoc.rdoc_files.include("README.rdoc", "lib/**/*.rb")
# end
# ```
#
# The `rdoc` object passed to the block is an
# [`RDoc::Task`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Task.html) object. See
# the attributes list for the
# [`RDoc::Task`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Task.html) class for
# available customization options.
#
# ## Specifying different task names
#
# You may wish to give the task a different name, such as if you are generating
# two sets of documentation. For instance, if you want to have a development set
# of documentation including private methods:
#
# ```ruby
# require 'rdoc/task'
#
# RDoc::Task.new :rdoc_dev do |rdoc|
#   rdoc.main = "README.doc"
#   rdoc.rdoc_files.include("README.rdoc", "lib/**/*.rb")
#   rdoc.options << "--all"
# end
# ```
#
# The tasks would then be named :*rdoc\_dev*, :clobber\_*rdoc\_dev*, and
# :re*rdoc\_dev*.
#
# If you wish to have completely different task names, then pass a
# [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) as first argument.
# With the `:rdoc`, `:clobber_rdoc` and `:rerdoc` options, you can customize the
# task names to your liking.
#
# For example:
#
# ```ruby
# require 'rdoc/task'
#
# RDoc::Task.new(:rdoc => "rdoc", :clobber_rdoc => "rdoc:clean",
#                :rerdoc => "rdoc:force")
# ```
#
# This will create the tasks `:rdoc`, `:rdoc:clean` and `:rdoc:force`.
class RDoc::Task
  # Whether to run the rdoc process as an external shell (default is false)
  def external; end

  # Whether to run the rdoc process as an external shell (default is false)
  def external=(value); end

  # Name of format generator (`--format`) used by rdoc. (defaults to rdoc's
  # default)
  def generator; end

  # Name of format generator (`--format`) used by rdoc. (defaults to rdoc's
  # default)
  def generator=(value); end

  # Name of file to be used as the main, top level file of the
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html). (default is none)
  def main; end

  # Name of file to be used as the main, top level file of the
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html). (default is none)
  def main=(value); end

  # Comment markup format. rdoc, rd and tomdoc are supported. (default is
  # 'rdoc')
  def markup; end

  # Comment markup format. rdoc, rd and tomdoc are supported. (default is
  # 'rdoc')
  def markup=(value); end

  # Name of the main, top level task. (default is :rdoc)
  def name; end

  # Name of the main, top level task. (default is :rdoc)
  def name=(value); end

  # Additional list of options to be passed rdoc. (default is [])
  def options; end

  # Additional list of options to be passed rdoc. (default is [])
  def options=(value); end

  # Name of directory to receive the html output files. (default is "html")
  def rdoc_dir; end

  # Name of directory to receive the html output files. (default is "html")
  def rdoc_dir=(value); end

  # List of files to be included in the rdoc generation. (default is [])
  def rdoc_files; end

  # List of files to be included in the rdoc generation. (default is [])
  def rdoc_files=(value); end

  # Name of template to be used by rdoc. (defaults to rdoc's default)
  def template; end

  # Name of template to be used by rdoc. (defaults to rdoc's default)
  def template=(value); end

  # Title of [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
  # documentation. (defaults to rdoc's default)
  def title; end

  # Title of [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
  # documentation. (defaults to rdoc's default)
  def title=(value); end

  # Create an [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) task with
  # the given name. See the
  # [`RDoc::Task`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Task.html) class
  # overview for documentation.
  def initialize(name = :rdoc, &block); end

  # The block passed to this method will be called just before running the
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) generator. It is
  # allowed to modify
  # [`RDoc::Task`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Task.html)
  # attributes inside the block.
  def before_running_rdoc(&block); end

  # Ensures that `names` only includes names for the :rdoc, :clobber\_rdoc and
  # :rerdoc. If other names are given an
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.7.0/ArgumentError.html) is
  # raised.
  def check_names(names); end

  # [`Task`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Task.html) description for
  # the clobber rdoc task or its renamed equivalent
  def clobber_task_description(); end

  # Sets default task values
  def defaults(); end

  # Create the tasks defined by this task lib.
  def define(); end

  # List of options that will be supplied to
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
  def option_list(); end

  # [`Task`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Task.html) description for
  # the rdoc task or its renamed equivalent
  def rdoc_task_description(); end

  # [`Task`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Task.html) description for
  # the rerdoc task or its renamed description
  def rerdoc_task_description(); end
end

# Methods for manipulating comment text
module RDoc::Text
  # Maps markup formats to classes that can parse them. If the format is
  # unknown, "rdoc" format is used.
  MARKUP_FORMAT = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # Maps an encoding to a
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) of characters
  # properly transcoded for that encoding.
  #
  # See also encode\_fallback.
  TO_HTML_CHARACTERS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # Expands tab characters in `text` to eight spaces
  def expand_tabs(text); end

  # Flush `text` left based on the shortest line
  def flush_left(text); end

  # Convert a string in markup format into HTML.
  #
  # Requires the including class to implement formatter
  def markup(text); end

  # Strips hashes, expands tabs then flushes `text` to the left
  def normalize_comment(text); end

  # Normalizes `text` then builds a RDoc::Markup::Document from it
  def parse(text, format = _); end

  # The first `limit` characters of `text` as HTML
  def snippet(text, limit = _); end

  # Strips leading # characters from `text`
  def strip_hashes(text); end

  # Strips leading and trailing n characters from `text`
  def strip_newlines(text); end

  # Strips /\* \*/ style comments
  def strip_stars(text); end

  # Converts ampersand, dashes, ellipsis, quotes, copyright and registered
  # trademark symbols in `text` to properly encoded characters.
  def to_html(text); end

  # Wraps `txt` to `line_len`
  def wrap(txt, line_len = _); end

  # Transcodes `character` to `encoding` with a `fallback` character.
  def self.encode_fallback(character, encoding, fallback); end
end

# A [`TokenStream`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TokenStream.html)
# is a list of tokens, gathered during the parse of some entity (say a method).
# Entities populate these streams by being registered with the lexer. Any class
# can collect tokens by including
# [`TokenStream`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TokenStream.html).
# From the outside, you use such an object by calling the
# [`start_collecting_tokens`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TokenStream.html#method-i-start_collecting_tokens)
# method, followed by calls to
# [`add_token`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TokenStream.html#method-i-add_token)
# and pop\_token.
module RDoc::TokenStream
  # Adds one `token` to the collected tokens
  def add_token(*tokens); end

  # Adds `tokens` to the collected tokens
  def add_tokens(*tokens); end

  # Starts collecting tokens
  #
  # Also aliased as:
  # [`start_collecting_tokens`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TokenStream.html#method-i-start_collecting_tokens)
  def collect_tokens; end

  # Remove the last token from the collected tokens
  def pop_token; end

  # Alias for:
  # [`collect_tokens`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TokenStream.html#method-i-collect_tokens)
  def start_collecting_tokens; end

  # Current token stream
  def token_stream; end

  # Returns a string representation of the token stream
  def tokens_to_s; end

  # Converts `token_stream` to HTML wrapping various tokens with `<span>`
  # elements. Some tokens types are wrapped in spans with the given class names.
  # Other token types are not wrapped in spans.
  def self.to_html(token_stream); end
end

# A parser for [`TomDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TomDoc.html)
# based on [`TomDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TomDoc.html)
# 1.0.0-rc1 (02adef9b5a)
#
# The [`TomDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TomDoc.html)
# specification can be found at:
#
# http://tomdoc.org
#
# The latest version of the
# [`TomDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TomDoc.html) specification
# can be found at:
#
# https://github.com/mojombo/tomdoc/blob/master/tomdoc.md
#
# To choose [`TomDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TomDoc.html) as
# your only default format see [Saved Options at
# `RDoc::Options`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Options.html#class-RDoc::Options-label-Saved+Options)
# for instructions on setting up a `.rdoc_options` file to store your project
# default.
#
# There are a few differences between this parser and the specification. A
# best-effort was made to follow the specification as closely as possible but
# some choices to deviate were made.
#
# A future version of [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html)
# will warn when a MUST or MUST NOT is violated and may warn when a SHOULD or
# SHOULD NOT is violated.
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) will always try to
# emit documentation even if given invalid
# [`TomDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TomDoc.html).
#
# Here are some implementation choices this parser currently makes:
#
# This parser allows rdoc-style inline markup but you should not depended on it.
#
# This parser allows a space between the comment and the method body.
#
# This parser does not require the default value to be described for an optional
# argument.
#
# This parser does not examine the order of sections. An Examples section may
# precede the Arguments section.
#
# This class is documented in
# [`TomDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TomDoc.html) format. Since
# this is a subclass of the
# [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html) markup parser there
# isn't much to see here, unfortunately.
class RDoc::TomDoc < ::RDoc::Markup::Parser
  # Creates a new
  # [`TomDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TomDoc.html) parser. See
  # also
  # [`RDoc::Markup::parse`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Markup.html#method-c-parse)
  def self.new; end

  # Builds a heading from the token stream
  #
  # level
  # :   The level of heading to create
  #
  #
  # ### Returns
  #
  # Returns an RDoc::Markup::Heading
  def build_heading(level); end

  # Builds a paragraph from the token stream
  #
  # margin
  # :   Unused
  #
  #
  # ### Returns
  #
  # Returns an RDoc::Markup::Paragraph.
  def build_paragraph(margin); end

  # Builds a verbatim from the token stream. A verbatim in the Examples section
  # will be marked as in Ruby format.
  #
  # margin
  # :   The indentation from the margin for lines that belong to this verbatim
  #     section.
  #
  #
  # ### Returns
  #
  # Returns an RDoc::Markup::Verbatim
  def build_verbatim(margin); end

  def parse_text(parent, indent); end

  # Turns text into an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
  # of tokens
  #
  # text
  # :   A [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) containing
  #     TomDoc-format text.
  #
  #
  # ### Returns
  #
  # Returns self.
  def tokenize(text); end

  # Token accessor
  def tokens; end

  def self.add_post_processor; end

  # Parses [`TomDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TomDoc.html) from
  # text
  #
  # text
  # :   A [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) containing
  #     TomDoc-format text.
  #
  #
  # ### Examples
  #
  # ```ruby
  # RDoc::TomDoc.parse <<-TOMDOC
  # This method does some things
  #
  # Returns nothing.
  # TOMDOC
  # # => #<RDoc::Markup::Document:0xXXX @parts=[...], @file=nil>
  # ```
  #
  # ### Returns
  #
  # Returns an RDoc::Markup::Document representing the
  # [`TomDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TomDoc.html) format.
  def self.parse(text); end

  # Extracts the Signature section's method signature
  #
  # comment
  # :   An
  #     [`RDoc::Comment`](https://docs.ruby-lang.org/en/2.7.0/RDoc/Comment.html)
  #     that will be parsed and have the signature extracted
  #
  #
  # ### Returns
  #
  # Returns a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # containing the signature and nil if not
  def self.signature(comment); end
end

# A [`TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html) context
# is a representation of the contents of a single file
class RDoc::TopLevel < ::RDoc::Context
  MARSHAL_VERSION = T.let(T.unsafe(nil), Integer)

  # Creates a new
  # [`TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html) for the
  # file at `absolute_name`. If documentation is being generated outside the
  # source dir `relative_name` is relative to the source directory.
  def self.new(absolute_name, relative_name = _); end

  # An
  # [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
  # is equal to another with the same
  # [`relative_name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html#attribute-i-relative_name)
  #
  # Also aliased as:
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html#method-i-eql-3F)
  def ==(other); end

  # Absolute name of this file
  def absolute_name; end

  # Absolute name of this file
  def absolute_name=(_); end

  # Adds `an_alias` to `Object` instead of `self`.
  def add_alias(an_alias); end

  # Adds `constant` to `Object` instead of `self`.
  def add_constant(constant); end

  # Adds `include` to `Object` instead of `self`.
  def add_include(include); end

  # Adds `method` to `Object` instead of `self`.
  def add_method(method); end

  # Adds class or module `mod`. Used in the building phase by the Ruby parser.
  def add_to_classes_or_modules(mod); end

  # Base name of this file
  def base_name; end

  # All the classes or modules that were declared in this file. These are
  # assigned to either `#classes_hash` or `#modules_hash` once we know what they
  # really are.
  def classes_or_modules; end

  # Returns a URL for this source file on some web repository. Use the -W
  # command line option to set.
  def cvs_url; end

  def diagram; end

  def diagram=(_); end

  # Only a [`TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
  # that contains text file) will be displayed. See also
  # [`RDoc::CodeObject#display?`](https://docs.ruby-lang.org/en/2.7.0/RDoc/CodeObject.html#method-i-display-3F)
  def display?; end

  # Alias for:
  # [`==`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html#method-i-3D-3D)
  def eql?(other); end

  # This TopLevel's
  # [`File::Stat`](https://docs.ruby-lang.org/en/2.7.0/File/Stat.html) struct
  def file_stat; end

  # This TopLevel's
  # [`File::Stat`](https://docs.ruby-lang.org/en/2.7.0/File/Stat.html) struct
  def file_stat=(_); end

  # See RDoc::TopLevel::find\_class\_or\_module
  def find_class_or_module(name); end

  # Finds a class or module named `symbol`
  def find_local_symbol(symbol); end

  # Finds a module or class with `name`
  def find_module_named(name); end

  # Returns the relative name of this file
  def full_name; end

  # An
  # [`RDoc::TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
  # has the same hash as another with the same
  # [`relative_name`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html#attribute-i-relative_name)
  def hash; end

  # URL for this with a `prefix`
  def http_url(prefix); end

  def inspect; end

  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) this file was last
  # modified, if known
  def last_modified; end

  # Dumps this
  # [`TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html) for use
  # by ri. See also marshal\_load
  def marshal_dump; end

  def marshal_load(array); end

  # Alias for:
  # [`base_name`](https://docs.ruby-lang.org/en/2.6.0/RDoc/TopLevel.html#method-i-base_name)
  def name; end

  # Returns the NormalClass "Object", creating it if not found.
  #
  # Records `self` as a location in "Object".
  def object_class; end

  # Base name of this file without the extension
  def page_name; end

  # The parser class that processed this file
  def parser; end

  def parser=(_); end

  # Path to this file for use with HTML generator output.
  def path; end

  def pretty_print(q); end

  # Relative name of this file
  def relative_name; end

  # Relative name of this file
  def relative_name=(_); end

  # Search record used by RDoc::Generator::JsonIndex
  def search_record; end

  # Is this [`TopLevel`](https://docs.ruby-lang.org/en/2.7.0/RDoc/TopLevel.html)
  # from a text file instead of a source code file?
  def text?; end

  def to_s; end
end
