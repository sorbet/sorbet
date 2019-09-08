# typed: __STDLIB_INTERNAL

# This class implements a pretty printing algorithm. It finds line breaks and
# nice indentations for grouped structure.
#
# By default, the class assumes that primitive elements are strings and each
# byte in the strings have single column in width. But it can be used for other
# situations by giving suitable arguments for some methods:
# *   newline object and space generation block for
#     [`PrettyPrint.new`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint.html#method-c-new)
# *   optional width argument for
#     [`PrettyPrint#text`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint.html#method-i-text)
# *   [`PrettyPrint#breakable`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint.html#method-i-breakable)
#
#
# There are several candidate uses:
# *   text formatting using proportional fonts
# *   multibyte characters which has columns different to number of bytes
# *   non-string formatting
#
#
# ## Bugs
# *   Box based formatting?
# *   Other (better) model/algorithm?
#
#
# Report any bugs at http://bugs.ruby-lang.org
#
# ## References
# Christian Lindig, Strictly Pretty, March 2000,
# http://www.st.cs.uni-sb.de/~lindig/papers/#pretty
#
# Philip Wadler, A prettier printer, March 1998,
# http://homepages.inf.ed.ac.uk/wadler/topics/language-design.html#prettier
#
# ## Author
# Tanaka Akira <akr@fsij.org>
class PrettyPrint
  # This is a convenience method which is same as follows:
  #
  # ```
  # begin
  #   q = PrettyPrint.new(output, maxwidth, newline, &genspace)
  #   ...
  #   q.flush
  #   output
  # end
  # ```
  def self.format(output = nil, maxwidth = nil, newline = nil, genspace = nil); end
  # This is similar to
  # [`PrettyPrint::format`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint.html#method-c-format)
  # but the result has no breaks.
  #
  # `maxwidth`, `newline` and `genspace` are ignored.
  #
  # The invocation of `breakable` in the block doesn't break a line and is
  # treated as just an invocation of `text`.
  def self.singleline_format(output = nil, maxwidth = nil, newline = nil, genspace = nil); end
  # Breaks the buffer into lines that are shorter than
  # [`maxwidth`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint.html#attribute-i-maxwidth)
  def break_outmost_groups; end
  # This says "you can break a line here if necessary", and a `width`-column
  # text `sep` is inserted if a line is not broken at the point.
  #
  # If `sep` is not specified, " " is used.
  #
  # If `width` is not specified, `sep.length` is used. You will have to specify
  # this when `sep` is a multibyte character, for example.
  def breakable(sep = nil, width = nil); end
  # Returns the group most recently added to the stack.
  #
  # Contrived example:
  #
  # ```
  # out = ""
  # => ""
  # q = PrettyPrint.new(out)
  # => #<PrettyPrint:0x82f85c0 @output="", @maxwidth=79, @newline="\n", @genspace=#<Proc:0x82f8368@/home/vbatts/.rvm/rubies/ruby-head/lib/ruby/2.0.0/prettyprint.rb:82 (lambda)>, @output_width=0, @buffer_width=0, @buffer=[], @group_stack=[#<PrettyPrint::Group:0x82f8138 @depth=0, @breakables=[], @break=false>], @group_queue=#<PrettyPrint::GroupQueue:0x82fb7c0 @queue=[[#<PrettyPrint::Group:0x82f8138 @depth=0, @breakables=[], @break=false>]]>, @indent=0>
  # q.group {
  #   q.text q.current_group.inspect
  #   q.text q.newline
  #   q.group(q.current_group.depth + 1) {
  #     q.text q.current_group.inspect
  #     q.text q.newline
  #     q.group(q.current_group.depth + 1) {
  #       q.text q.current_group.inspect
  #       q.text q.newline
  #       q.group(q.current_group.depth + 1) {
  #         q.text q.current_group.inspect
  #         q.text q.newline
  #       }
  #     }
  #   }
  # }
  # => 284
  #  puts out
  # #<PrettyPrint::Group:0x8354758 @depth=1, @breakables=[], @break=false>
  # #<PrettyPrint::Group:0x8354550 @depth=2, @breakables=[], @break=false>
  # #<PrettyPrint::Group:0x83541cc @depth=3, @breakables=[], @break=false>
  # #<PrettyPrint::Group:0x8347e54 @depth=4, @breakables=[], @break=false>
  # ```
  def current_group; end
  # This is similar to
  # [`breakable`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint.html#method-i-breakable)
  # except the decision to break or not is determined individually.
  #
  # Two
  # [`fill_breakable`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint.html#method-i-fill_breakable)
  # under a group may cause 4 results: (break,break), (break,non-break),
  # (non-break,break), (non-break,non-break). This is different to
  # [`breakable`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint.html#method-i-breakable)
  # because two
  # [`breakable`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint.html#method-i-breakable)
  # under a group may cause 2 results: (break,break), (non-break,non-break).
  #
  # The text `sep` is inserted if a line is not broken at this point.
  #
  # If `sep` is not specified, " " is used.
  #
  # If `width` is not specified, `sep.length` is used. You will have to specify
  # this when `sep` is a multibyte character, for example.
  def fill_breakable(sep = nil, width = nil); end
  # outputs buffered data.
  def flush; end
  # A lambda or [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html), that
  # takes one argument, of a Fixnum, and returns the corresponding number of
  # spaces.
  #
  # By default this is:
  #
  # ```ruby
  # lambda {|n| ' ' * n}
  # ```
  def genspace; end
  # Groups line break hints added in the block. The line break hints are all to
  # be used or not.
  #
  # If `indent` is specified, the method call is regarded as nested by
  # nest(indent) { ... }.
  #
  # If `open_obj` is specified, `text open_obj, open_width` is called before
  # grouping. If `close_obj` is specified, `text close_obj, close_width` is
  # called after grouping.
  def group(indent = nil, open_obj = nil, close_obj = nil, open_width = nil, close_width = nil); end
  # The PrettyPrint::GroupQueue of groups in stack to be pretty printed
  def group_queue; end
  # Takes a block and queues a new group that is indented 1 level further.
  def group_sub; end
  # The number of spaces to be indented
  def indent; end
  def initialize(output = nil, maxwidth = nil, newline = nil, &genspace); end
  # The maximum width of a line, before it is separated in to a newline
  #
  # This defaults to 79, and should be a Fixnum
  def maxwidth; end
  # Increases left margin after newline with `indent` for line breaks added in
  # the block.
  def nest(indent); end
  # The value that is appended to `output` to add a new line.
  #
  # This defaults to "n", and should be
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html)
  def newline; end
  # The output object.
  #
  # This defaults to '', and should accept the << method
  def output; end
  # This adds `obj` as a text of `width` columns in width.
  #
  # If `width` is not specified, obj.length is used.
  def text(obj, width = nil); end
end

class PrettyPrint::Text
  def add(obj, width); end
  def initialize; end
  def output(out, output_width); end
  def width; end
end

class PrettyPrint::Breakable
  def indent; end
  def initialize(sep, width, q); end
  def obj; end
  def output(out, output_width); end
  def width; end
end

class PrettyPrint::Group
  def break; end
  def break?; end
  def breakables; end
  def depth; end
  def first?; end
  def initialize(depth); end
end

class PrettyPrint::GroupQueue
  def delete(group); end
  def deq; end
  def enq(group); end
  def initialize(*groups); end
end

# [`PrettyPrint::SingleLine`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint/SingleLine.html)
# is used by
# [`PrettyPrint.singleline_format`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint.html#method-c-singleline_format)
#
# It is passed to be similar to a
# [`PrettyPrint`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint.html) object
# itself, by responding to:
# *   [`text`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint/SingleLine.html#method-i-text)
# *   [`breakable`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint/SingleLine.html#method-i-breakable)
# *   nest
# *   [`group`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint/SingleLine.html#method-i-group)
# *   flush
# *   [`first?`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint/SingleLine.html#method-i-first-3F)
#
#
# but instead, the output has no line breaks
class PrettyPrint::SingleLine
  # Appends `sep` to the text to be output. By default `sep` is ' '
  #
  # `width` argument is here for compatibility. It is a noop argument.
  def breakable(sep = nil, width = nil); end
  # This is used as a predicate, and ought to be called first.
  def first?; end
  def flush; end
  # Opens a block for grouping objects to be pretty printed.
  #
  # Arguments:
  # *   `indent` - noop argument. Present for compatibility.
  # *   `open_obj` - text appended before the &blok. Default is ''
  # *   `close_obj` - text appended after the &blok. Default is ''
  # *   `open_width` - noop argument. Present for compatibility.
  # *   `close_width` - noop argument. Present for compatibility.
  def group(indent = nil, open_obj = nil, close_obj = nil, open_width = nil, close_width = nil); end
  def initialize(output, maxwidth = nil, newline = nil); end
  def nest(indent); end
  # Add `obj` to the text to be output.
  #
  # `width` argument is here for compatibility. It is a noop argument.
  def text(obj, width = nil); end
end

# A pretty-printer for Ruby objects.
#
# ## What [`PP`](https://docs.ruby-lang.org/en/2.6.0/PP.html) Does
#
# Standard output by
# [`p`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-p) returns
# this:
#
# ```ruby
# #<PP:0x81fedf0 @genspace=#<Proc:0x81feda0>, @group_queue=#<PrettyPrint::GroupQueue:0x81fed3c @queue=[[#<PrettyPrint::Group:0x81fed78 @breakables=[], @depth=0, @break=false>], []]>, @buffer=[], @newline="\n", @group_stack=[#<PrettyPrint::Group:0x81fed78 @breakables=[], @depth=0, @break=false>], @buffer_width=0, @indent=0, @maxwidth=79, @output_width=2, @output=#<IO:0x8114ee4>>
# ```
#
# Pretty-printed output returns this:
#
# ```
# #<PP:0x81fedf0
#  @buffer=[],
#  @buffer_width=0,
#  @genspace=#<Proc:0x81feda0>,
#  @group_queue=
#   #<PrettyPrint::GroupQueue:0x81fed3c
#    @queue=
#     [[#<PrettyPrint::Group:0x81fed78 @break=false, @breakables=[], @depth=0>],
#      []]>,
#  @group_stack=
#   [#<PrettyPrint::Group:0x81fed78 @break=false, @breakables=[], @depth=0>],
#  @indent=0,
#  @maxwidth=79,
#  @newline="\n",
#  @output=#<IO:0x8114ee4>,
#  @output_width=2>
# ```
#
# ## Usage
#
# ```
# pp(obj)             #=> obj
# pp obj              #=> obj
# pp(obj1, obj2, ...) #=> [obj1, obj2, ...]
# pp()                #=> nil
# ```
#
# Output `obj(s)` to `$>` in pretty printed format.
#
# It returns `obj(s)`.
#
# ## Output Customization
#
# To define a customized pretty printing function for your classes, redefine
# method `#pretty_print(pp)` in the class.
#
# `#pretty_print` takes the `pp` argument, which is an instance of the
# [`PP`](https://docs.ruby-lang.org/en/2.6.0/PP.html) class. The method uses
# [`text`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint.html#method-i-text),
# [`breakable`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint.html#method-i-breakable),
# [`nest`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint.html#method-i-nest),
# [`group`](https://docs.ruby-lang.org/en/2.6.0/PrettyPrint.html#method-i-group)
# and [`pp`](https://docs.ruby-lang.org/en/2.6.0/PP/PPMethods.html#method-i-pp)
# to print the object.
#
# ## Pretty-Print [`JSON`](https://docs.ruby-lang.org/en/2.6.0/JSON.html)
#
# To pretty-print [`JSON`](https://docs.ruby-lang.org/en/2.6.0/JSON.html) refer
# to
# [`JSON#pretty_generate`](https://docs.ruby-lang.org/en/2.6.0/JSON.html#method-i-pretty_generate).
#
# ## Author
# Tanaka Akira <akr@fsij.org>
class PP < PrettyPrint
  include PP::PPMethods
  def self.mcall(obj, mod, meth, *args, &block); end
  # Outputs `obj` to `out` in pretty printed format of `width` columns in width.
  #
  # If `out` is omitted, `$>` is assumed. If `width` is omitted, 79 is assumed.
  #
  # [`PP.pp`](https://docs.ruby-lang.org/en/2.6.0/PP.html#method-c-pp) returns
  # `out`.
  def self.pp(obj, out = nil, width = nil); end
  # Returns the sharing detection flag as a boolean value. It is false by
  # default.
  def self.sharing_detection; end
  # Returns the sharing detection flag as a boolean value. It is false by
  # default.
  def self.sharing_detection=(arg0); end
  # Outputs `obj` to `out` like
  # [`PP.pp`](https://docs.ruby-lang.org/en/2.6.0/PP.html#method-c-pp) but with
  # no indent and newline.
  #
  # [`PP.singleline_pp`](https://docs.ruby-lang.org/en/2.6.0/PP.html#method-c-singleline_pp)
  # returns `out`.
  def self.singleline_pp(obj, out = nil); end
end

module PP::PPMethods
  # Check whether the object\_id `id` is in the current buffer of objects to be
  # pretty printed. Used to break cycles in chains of objects to be pretty
  # printed.
  def check_inspect_key(id); end
  # A convenience method which is same as follows:
  #
  # ```ruby
  # text ','
  # breakable
  # ```
  def comma_breakable; end
  # Yields to a block and preserves the previous set of objects being printed.
  def guard_inspect_key; end
  # A convenience method, like
  # [`object_group`](https://docs.ruby-lang.org/en/2.6.0/PP/PPMethods.html#method-i-object_group),
  # but also reformats the Object's object\_id.
  def object_address_group(obj, &block); end
  # A convenience method which is same as follows:
  #
  # ```
  # group(1, '#<' + obj.class.name, '>') { ... }
  # ```
  def object_group(obj, &block); end
  # Removes an object from the set of objects being pretty printed.
  def pop_inspect_key(id); end
  # Adds `obj` to the pretty printing buffer using Object#pretty\_print or
  # Object#pretty\_print\_cycle.
  #
  # Object#pretty\_print\_cycle is used when `obj` is already printed, a.k.a the
  # object reference chain has a cycle.
  def pp(obj); end
  # A pretty print for a [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html)
  def pp_hash(obj); end
  # A present standard failsafe for pretty printing any given
  # [`Object`](https://docs.ruby-lang.org/en/2.6.0/Object.html)
  def pp_object(obj); end
  # Adds the object\_id `id` to the set of objects being pretty printed, so as
  # to not repeat objects.
  def push_inspect_key(id); end
  # Adds a separated list. The list is separated by comma with breakable space,
  # by default.
  #
  # [`seplist`](https://docs.ruby-lang.org/en/2.6.0/PP/PPMethods.html#method-i-seplist)
  # iterates the `list` using `iter_method`. It yields each object to the block
  # given for
  # [`seplist`](https://docs.ruby-lang.org/en/2.6.0/PP/PPMethods.html#method-i-seplist).
  # The procedure `separator_proc` is called between each yields.
  #
  # If the iteration is zero times, `separator_proc` is not called at all.
  #
  # If `separator_proc` is nil or not given, +lambda {
  # [`comma_breakable`](https://docs.ruby-lang.org/en/2.6.0/PP/PPMethods.html#method-i-comma_breakable)
  # }+ is used. If `iter_method` is not given, :each is used.
  #
  # For example, following 3 code fragments has similar effect.
  #
  # ```ruby
  # q.seplist([1,2,3]) {|v| xxx v }
  #
  # q.seplist([1,2,3], lambda { q.comma_breakable }, :each) {|v| xxx v }
  #
  # xxx 1
  # q.comma_breakable
  # xxx 2
  # q.comma_breakable
  # xxx 3
  # ```
  def seplist(list, sep = nil, iter_method = nil); end
end

class PP::SingleLine < PrettyPrint::SingleLine
  include PP::PPMethods
end

module PP::ObjectMixin
  def pretty_print(q); end
  def pretty_print_cycle(q); end
  def pretty_print_inspect; end
  def pretty_print_instance_variables; end
end
