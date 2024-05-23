# typed: __STDLIB_INTERNAL

# The [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) mixin
# provides collection classes with several traversal and searching methods, and
# with the ability to sort. The class must provide a method each, which yields
# successive members of the collection. If
# [`Enumerable#max`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-max),
# [`min`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-min), or
# [`sort`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-sort) is
# used, the objects in the collection must also implement a meaningful `<=>`
# operator, as these methods rely on an ordering between members of the
# collection.
module Enumerable
  extend T::Generic
  Elem = type_member(:out)

  abstract!

  sig do
    abstract.
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig {returns(T.self_type)}
  def each(&blk); end

  # Passes each element of the collection to the given block. The method returns
  # `true` if the block never returns `false` or `nil`. If the block is not
  # given, Ruby adds an implicit block of `{ |obj| obj }` which will cause
  # [`all?`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-all-3F)
  # to return `true` when none of the collection members are `false` or `nil`.
  #
  # If instead a pattern is supplied, the method returns whether `pattern ===
  # element` for every collection member.
  #
  # ```ruby
  # %w[ant bear cat].all? { |word| word.length >= 3 } #=> true
  # %w[ant bear cat].all? { |word| word.length >= 4 } #=> false
  # %w[ant bear cat].all?(/t/)                        #=> false
  # [1, 2i, 3.14].all?(Numeric)                       #=> true
  # [nil, true, 99].all?                              #=> false
  # [].all?                                           #=> true
  # ```
  sig {returns(T::Boolean)}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Boolean)
  end
  sig { params(pattern: T.untyped).returns(T::Boolean) }
  def all?(pattern = nil, &blk); end

  # Passes each element of the collection to the given block. The method returns
  # `true` if the block ever returns a value other than `false` or `nil`. If the
  # block is not given, Ruby adds an implicit block of `{ |obj| obj }` that will
  # cause
  # [`any?`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-any-3F)
  # to return `true` if at least one of the collection members is not `false` or
  # `nil`.
  #
  # If instead a pattern is supplied, the method returns whether `pattern ===
  # element` for any collection member.
  #
  # ```ruby
  # %w[ant bear cat].any? { |word| word.length >= 3 } #=> true
  # %w[ant bear cat].any? { |word| word.length >= 4 } #=> true
  # %w[ant bear cat].any?(/d/)                        #=> false
  # [nil, true, 99].any?(Integer)                     #=> true
  # [nil, true, 99].any?                              #=> true
  # [].any?                                           #=> false
  # ```
  sig {returns(T::Boolean)}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Boolean)
  end
  sig { params(pattern: T.untyped).returns(T::Boolean) }
  def any?(pattern = nil, &blk); end

  # Returns an enumerator object generated from this enumerator and given
  # enumerables.
  #
  # ```ruby
  # e = (1..3).chain([4, 5])
  # e.to_a #=> [1, 2, 3, 4, 5]
  # ```
  sig do
    type_parameters(:U).params(enums: T::Enumerable[T.type_parameter(:U)])
    .returns(
      T::Enumerator::Chain[T.any(Elem, T.type_parameter(:U))]
    )
  end
  def chain(*enums); end

  # Enumerates over the items, chunking them together based on the return value
  # of the block.
  #
  # Consecutive elements which return the same block value are chunked together.
  #
  # For example, consecutive even numbers and odd numbers can be chunked as
  # follows.
  #
  # ```ruby
  # [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5].chunk { |n|
  #   n.even?
  # }.each { |even, ary|
  #   p [even, ary]
  # }
  # #=> [false, [3, 1]]
  # #   [true, [4]]
  # #   [false, [1, 5, 9]]
  # #   [true, [2, 6]]
  # #   [false, [5, 3, 5]]
  # ```
  #
  # This method is especially useful for sorted series of elements. The
  # following example counts words for each initial letter.
  #
  # ```ruby
  # open("/usr/share/dict/words", "r:iso-8859-1") { |f|
  #   f.chunk { |line| line.ord }.each { |ch, lines| p [ch.chr, lines.length] }
  # }
  # #=> ["\n", 1]
  # #   ["A", 1327]
  # #   ["B", 1372]
  # #   ["C", 1507]
  # #   ["D", 791]
  # #   ...
  # ```
  #
  # The following key values have special meaning:
  # *   `nil` and `:_separator` specifies that the elements should be dropped.
  # *   `:_alone` specifies that the element should be chunked by itself.
  #
  #
  # Any other symbols that begin with an underscore will raise an error:
  #
  # ```ruby
  # items.chunk { |item| :_underscore }
  # #=> RuntimeError: symbols beginning with an underscore are reserved
  # ```
  #
  # `nil` and `:_separator` can be used to ignore some elements.
  #
  # For example, the sequence of hyphens in svn log can be eliminated as
  # follows:
  #
  # ```ruby
  # sep = "-"*72 + "\n"
  # IO.popen("svn log README") { |f|
  #   f.chunk { |line|
  #     line != sep || nil
  #   }.each { |_, lines|
  #     pp lines
  #   }
  # }
  # #=> ["r20018 | knu | 2008-10-29 13:20:42 +0900 (Wed, 29 Oct 2008) | 2 lines\n",
  # #    "\n",
  # #    "* README, README.ja: Update the portability section.\n",
  # #    "\n"]
  # #   ["r16725 | knu | 2008-05-31 23:34:23 +0900 (Sat, 31 May 2008) | 2 lines\n",
  # #    "\n",
  # #    "* README, README.ja: Add a note about default C flags.\n",
  # #    "\n"]
  # #   ...
  # ```
  #
  # Paragraphs separated by empty lines can be parsed as follows:
  #
  # ```ruby
  # File.foreach("README").chunk { |line|
  #   /\A\s*\z/ !~ line || nil
  # }.each { |_, lines|
  #   pp lines
  # }
  # ```
  #
  # `:_alone` can be used to force items into their own chunk. For example, you
  # can put lines that contain a URL by themselves, and chunk the rest of the
  # lines together, like this:
  #
  # ```ruby
  # pattern = /http/
  # open(filename) { |f|
  #   f.chunk { |line| line =~ pattern ? :_alone : true }.each { |key, lines|
  #     pp lines
  #   }
  # }
  # ```
  #
  # If no block is given, an enumerator to `chunk` is returned instead.
  def chunk; end

  # Returns a new array with the results of running *block* once for every
  # element in *enum*.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # (1..4).map { |i| i*i }      #=> [1, 4, 9, 16]
  # (1..4).collect { "cat"  }   #=> ["cat", "cat", "cat", "cat"]
  # ```
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  sig {returns(T::Enumerator[Elem])}
  def collect(&blk); end

  # Returns a new array with the concatenated results of running *block* once
  # for every element in *enum*.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # [1, 2, 3, 4].flat_map { |e| [e, -e] } #=> [1, -1, 2, -2, 3, -3, 4, -4]
  # [[1, 2], [3, 4]].flat_map { |e| e + [100] } #=> [1, 2, 100, 3, 4, 100]
  # ```
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T::Enumerator[T.type_parameter(:U)]),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  def collect_concat(&blk); end

  # Enumerates over the items, chunking them together based on the return value
  # of the block.
  #
  # Consecutive elements which return the same block value are chunked together.
  #
  # For example, consecutive even numbers and odd numbers can be chunked as
  # follows.
  #
  # ```ruby
  # [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5].chunk { |n|
  #   n.even?
  # }.each { |even, ary|
  #   p [even, ary]
  # }
  # #=> [false, [3, 1]]
  # #   [true, [4]]
  # #   [false, [1, 5, 9]]
  # #   [true, [2, 6]]
  # #   [false, [5, 3, 5]]
  # ```
  #
  # This method is especially useful for sorted series of elements. The
  # following example counts words for each initial letter.
  #
  # ```ruby
  # open("/usr/share/dict/words", "r:iso-8859-1") { |f|
  #   f.chunk { |line| line.ord }.each { |ch, lines| p [ch.chr, lines.length] }
  # }
  # #=> ["\n", 1]
  # #   ["A", 1327]
  # #   ["B", 1372]
  # #   ["C", 1507]
  # #   ["D", 791]
  # #   ...
  # ```
  #
  # The following key values have special meaning:
  # *   `nil` and `:_separator` specifies that the elements should be dropped.
  # *   `:_alone` specifies that the element should be chunked by itself.
  #
  #
  # Any other symbols that begin with an underscore will raise an error:
  #
  # ```ruby
  # items.chunk { |item| :_underscore }
  # #=> RuntimeError: symbols beginning with an underscore are reserved
  # ```
  #
  # `nil` and `:_separator` can be used to ignore some elements.
  #
  # For example, the sequence of hyphens in svn log can be eliminated as
  # follows:
  #
  # ```ruby
  # sep = "-"*72 + "\n"
  # IO.popen("svn log README") { |f|
  #   f.chunk { |line|
  #     line != sep || nil
  #   }.each { |_, lines|
  #     pp lines
  #   }
  # }
  # #=> ["r20018 | knu | 2008-10-29 13:20:42 +0900 (Wed, 29 Oct 2008) | 2 lines\n",
  # #    "\n",
  # #    "* README, README.ja: Update the portability section.\n",
  # #    "\n"]
  # #   ["r16725 | knu | 2008-05-31 23:34:23 +0900 (Sat, 31 May 2008) | 2 lines\n",
  # #    "\n",
  # #    "* README, README.ja: Add a note about default C flags.\n",
  # #    "\n"]
  # #   ...
  # ```
  #
  # Paragraphs separated by empty lines can be parsed as follows:
  #
  # ```ruby
  # File.foreach("README").chunk { |line|
  #   /\A\s*\z/ !~ line || nil
  # }.each { |_, lines|
  #   pp lines
  # }
  # ```
  #
  # `:_alone` can be used to force items into their own chunk. For example, you
  # can put lines that contain a URL by themselves, and chunk the rest of the
  # lines together, like this:
  #
  # ```ruby
  # pattern = /http/
  # open(filename) { |f|
  #   f.chunk { |line| line =~ pattern ? :_alone : true }.each { |key, lines|
  #     pp lines
  #   }
  # }
  # ```
  #
  # If no block is given, an enumerator to `chunk` is returned instead.
  sig do
    type_parameters(:U)
      .params(blk: T.proc.params(elt: Elem).returns(T.type_parameter(:U)))
      .returns(T::Enumerable[[T.type_parameter(:U), T::Array[Elem]]])
  end
  sig { returns(T::Enumerator[Elem]) }
  def chunk(&blk); end

  # Creates an enumerator for each chunked elements. The beginnings of chunks
  # are defined by the block.
  #
  # This method split each chunk using adjacent elements, *elt\_before* and
  # *elt\_after*, in the receiver enumerator. This method split chunks between
  # *elt\_before* and *elt\_after* where the block returns `false`.
  #
  # The block is called the length of the receiver enumerator minus one.
  #
  # The result enumerator yields the chunked elements as an array. So `each`
  # method can be called as follows:
  #
  # ```
  # enum.chunk_while { |elt_before, elt_after| bool }.each { |ary| ... }
  # ```
  #
  # Other methods of the
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) class
  # and [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html)
  # module, such as `to_a`, `map`, etc., are also usable.
  #
  # For example, one-by-one increasing subsequence can be chunked as follows:
  #
  # ```ruby
  # a = [1,2,4,9,10,11,12,15,16,19,20,21]
  # b = a.chunk_while {|i, j| i+1 == j }
  # p b.to_a #=> [[1, 2], [4], [9, 10, 11, 12], [15, 16], [19, 20, 21]]
  # c = b.map {|a| a.length < 3 ? a : "#{a.first}-#{a.last}" }
  # p c #=> [[1, 2], [4], "9-12", [15, 16], "19-21"]
  # d = c.join(",")
  # p d #=> "1,2,4,9-12,15,16,19-21"
  # ```
  #
  # Increasing (non-decreasing) subsequence can be chunked as follows:
  #
  # ```ruby
  # a = [0, 9, 2, 2, 3, 2, 7, 5, 9, 5]
  # p a.chunk_while {|i, j| i <= j }.to_a
  # #=> [[0, 9], [2, 2, 3], [2, 7], [5, 9], [5]]
  # ```
  #
  # Adjacent evens and odds can be chunked as follows:
  # ([`Enumerable#chunk`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-chunk)
  # is another way to do it.)
  #
  # ```ruby
  # a = [7, 5, 9, 2, 0, 7, 9, 4, 2, 0]
  # p a.chunk_while {|i, j| i.even? == j.even? }.to_a
  # #=> [[7, 5, 9], [2, 0], [7, 9], [4, 2, 0]]
  # ```
  #
  # [`Enumerable#slice_when`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-slice_when)
  # does the same, except splitting when the block returns `true` instead of
  # `false`.
  sig {
    params(blk: T.proc.params(elt_before: Elem, elt_after: Elem).returns(T::Boolean))
    .returns(T::Enumerable[T::Array[Elem]])
  }
  def chunk_while(&blk); end

  # Returns the number of items in `enum` through enumeration. If an argument is
  # given, the number of items in `enum` that are equal to `item` are counted.
  # If a block is given, it counts the number of elements yielding a true value.
  #
  # ```ruby
  # ary = [1, 2, 4, 2]
  # ary.count               #=> 4
  # ary.count(2)            #=> 2
  # ary.count{ |x| x%2==0 } #=> 3
  # ```
  sig {returns(Integer)}
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(Integer)
  end
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(Integer)
  end
  def count(arg0=T.unsafe(nil), &blk); end

  # Calls *block* for each element of *enum* repeatedly *n* times or forever if
  # none or `nil` is given. If a non-positive number is given or the collection
  # is empty, does nothing. Returns `nil` if the loop has finished without
  # getting interrupted.
  #
  # [`Enumerable#cycle`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-cycle)
  # saves elements in an internal array so changes to *enum* after the first
  # pass have no effect.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # a = ["a", "b", "c"]
  # a.cycle { |x| puts x }  # print, a, b, c, a, b, c,.. forever.
  # a.cycle(2) { |x| puts x }  # print, a, b, c, a, b, c.
  # ```
  sig do
    params(
        n: Integer,
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(NilClass)
  end
  sig do
    params(
        n: Integer,
    )
    .returns(T::Enumerator[Elem])
  end
  def cycle(n=T.unsafe(nil), &blk); end

  # Passes each entry in *enum* to *block*. Returns the first for which *block*
  # is not false. If no object matches, calls *ifnone* and returns its result
  # when it is specified, or returns `nil` otherwise.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # (1..100).detect  #=> #<Enumerator: 1..100:detect>
  # (1..100).find    #=> #<Enumerator: 1..100:find>
  #
  # (1..10).detect         { |i| i % 5 == 0 && i % 7 == 0 }   #=> nil
  # (1..10).find           { |i| i % 5 == 0 && i % 7 == 0 }   #=> nil
  # (1..10).detect(-> {0}) { |i| i % 5 == 0 && i % 7 == 0 }   #=> 0
  # (1..10).find(-> {0})   { |i| i % 5 == 0 && i % 7 == 0 }   #=> 0
  # (1..100).detect        { |i| i % 5 == 0 && i % 7 == 0 }   #=> 35
  # (1..100).find          { |i| i % 5 == 0 && i % 7 == 0 }   #=> 35
  # ```
  sig do
    type_parameters(:U)
      .params(
        ifnone: T.proc.returns(T.type_parameter(:U)),
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
      )
      .returns(T.any(T.type_parameter(:U), Elem))
  end
  sig do
    type_parameters(:U)
      .params(
        ifnone: T.proc.returns(T.type_parameter(:U)),
      )
      .returns(T::Enumerator[T.any(T.type_parameter(:U), Elem)])
  end
  sig do
    params(
      blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    returns(T::Enumerator[Elem])
  end
  def detect(ifnone=T.unsafe(nil), &blk); end

  # Drops first n elements from *enum*, and returns rest elements in an array.
  #
  # ```ruby
  # a = [1, 2, 3, 4, 5, 0]
  # a.drop(3)             #=> [4, 5, 0]
  # ```
  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Elem])
  end
  def drop(n); end

  # Drops elements up to, but not including, the first element for which the
  # block returns `nil` or `false` and returns an array containing the remaining
  # elements.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # a = [1, 2, 3, 4, 5, 0]
  # a.drop_while { |i| i < 3 }   #=> [3, 4, 5, 0]
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def drop_while(&blk); end

  # Iterates the given block for each array of consecutive <n> elements. If no
  # block is given, returns an enumerator.
  #
  # e.g.:
  #
  # ```ruby
  # (1..10).each_cons(3) { |a| p a }
  # # outputs below
  # [1, 2, 3]
  # [2, 3, 4]
  # [3, 4, 5]
  # [4, 5, 6]
  # [5, 6, 7]
  # [6, 7, 8]
  # [7, 8, 9]
  # [8, 9, 10]
  # ```
  sig do
    params(
        n: Integer,
        blk: T.proc.params(arg0: T::Array[Elem]).returns(BasicObject),
    )
    .returns(NilClass)
  end
  sig do
    params(
        n: Integer,
    )
    .returns(T::Enumerator[T::Array[Elem]])
  end
  def each_cons(n, &blk); end

  # Calls *block* once for each element in `self`, passing that element as a
  # parameter, converting multiple values from yield to an array.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # class Foo
  #   include Enumerable
  #   def each
  #     yield 1
  #     yield 1, 2
  #     yield
  #   end
  # end
  # Foo.new.each_entry{ |o| p o }
  # ```
  #
  # produces:
  #
  # ```ruby
  # 1
  # [1, 2]
  # nil
  # ```
  def each_entry(*_, &blk); end

  # Calls *block* with two arguments, the item and its index, for each item in
  # *enum*. Given arguments are passed through to each().
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # hash = Hash.new
  # %w(cat dog wombat).each_with_index { |item, index|
  #   hash[item] = index
  # }
  # hash   #=> {"cat"=>0, "dog"=>1, "wombat"=>2}
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Integer).returns(BasicObject),
    )
    .returns(T::Enumerable[Elem])
  end
  sig {returns(T::Enumerator[[Elem, Integer]])}
  def each_with_index(&blk); end

  # Iterates the given block for each element with an arbitrary object given,
  # and returns the initially given object.
  #
  # If no block is given, returns an enumerator.
  #
  # ```ruby
  # evens = (1..10).each_with_object([]) { |i, a| a << i*2 }
  # #=> [2, 4, 6, 8, 10, 12, 14, 16, 18, 20]
  # ```
  sig do
    type_parameters(:U).params(
        arg0: T.type_parameter(:U),
        blk: T.proc.params(arg0: Elem, arg1: T.type_parameter(:U)).returns(BasicObject),
    )
    .returns(T.type_parameter(:U))
  end
  sig do
    type_parameters(:U).params(
        arg0: T.type_parameter(:U),
    )
    .returns(T::Enumerator[[Elem, T.type_parameter(:U)]])
  end
  def each_with_object(arg0, &blk); end

  # Returns an array containing the items in *enum*.
  #
  # ```ruby
  # (1..7).to_a                       #=> [1, 2, 3, 4, 5, 6, 7]
  # { 'a'=>1, 'b'=>2, 'c'=>3 }.to_a   #=> [["a", 1], ["b", 2], ["c", 3]]
  #
  # require 'prime'
  # Prime.entries 10                  #=> [2, 3, 5, 7]
  # ```
  sig {returns(T::Array[Elem])}
  def entries(); end

  # Returns an array containing all elements of `enum` for which the given
  # `block` returns a true value.
  #
  # The *find\_all* and *select* methods are aliases. There is no performance
  # benefit to either.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # (1..10).find_all { |i|  i % 3 == 0 }   #=> [3, 6, 9]
  #
  # [1,2,3,4,5].select { |num|  num.even?  }   #=> [2, 4]
  #
  # [:foo, :bar].filter { |x| x == :foo }   #=> [:foo]
  # ```
  #
  # See also
  # [`Enumerable#reject`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-reject),
  # [`Enumerable#grep`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-grep).
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def find_all(&blk); end

  # Returns a new array containing the truthy results (everything except `false`
  # or `nil`) of running the `block` for every element in `enum`.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # (1..10).filter_map { |i| i * 2 if i.even? } #=> [4, 8, 12, 16, 20]
  # ```
  sig do
     type_parameters(:T)
     .params(blk: T.proc.params(arg0: Elem).returns(T.any(NilClass, FalseClass, T.type_parameter(:T))))
     .returns(T::Array[T.type_parameter(:T)])
  end
  sig {returns(T::Enumerator[Elem])}
  def filter_map(&blk); end

  # Compares each entry in *enum* with *value* or passes to *block*. Returns the
  # index for the first for which the evaluated value is non-false. If no object
  # matches, returns `nil`
  #
  # If neither block nor argument is given, an enumerator is returned instead.
  #
  # ```ruby
  # (1..10).find_index  { |i| i % 5 == 0 && i % 7 == 0 }  #=> nil
  # (1..100).find_index { |i| i % 5 == 0 && i % 7 == 0 }  #=> 34
  # (1..100).find_index(50)                               #=> 49
  # ```
  sig do
    params(
        value: BasicObject,
    )
    .returns(T.nilable(Integer))
  end
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.nilable(Integer))
  end
  sig {returns(T::Enumerator[Elem])}
  def find_index(value=T.unsafe(nil), &blk); end

  # Returns the first element, or the first `n` elements, of the enumerable. If
  # the enumerable is empty, the first form returns `nil`, and the second form
  # returns an empty array.
  #
  # ```ruby
  # %w[foo bar baz].first     #=> "foo"
  # %w[foo bar baz].first(2)  #=> ["foo", "bar"]
  # %w[foo bar baz].first(10) #=> ["foo", "bar", "baz"]
  # [].first                  #=> nil
  # [].first(10)              #=> []
  # ```
  sig {returns(T.nilable(Elem))}
  sig do
    params(
        n: Integer,
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  def first(n=T.unsafe(nil)); end

  # Returns an array of every element in *enum* for which `Pattern === element`.
  # If the optional *block* is supplied, each matching element is passed to it,
  # and the block's result is stored in the output array.
  #
  # ```ruby
  # (1..100).grep 38..44   #=> [38, 39, 40, 41, 42, 43, 44]
  # c = IO.constants
  # c.grep(/SEEK/)         #=> [:SEEK_SET, :SEEK_CUR, :SEEK_END]
  # res = c.grep(/SEEK/) { |v| IO.const_get(v) }
  # res                    #=> [0, 1, 2]
  # ```
  sig do
    type_parameters(:Instance)
      .params(
          arg0: T::Class[T.type_parameter(:Instance)],
      )
      .returns(T::Array[T.all(Elem, T.type_parameter(:Instance))])
  end
  sig do
    type_parameters(:Instance, :U)
      .params(
          arg0: T::Class[T.type_parameter(:Instance)],
          blk: T.proc.params(arg0: T.type_parameter(:Instance)).returns(T.type_parameter(:U)),
      )
      .returns(T::Array[T.type_parameter(:U)])
  end
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Array[Elem])
  end
  sig do
    type_parameters(:U).params(
        arg0: BasicObject,
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  def grep(arg0, &blk); end

  # Inverted version of
  # [`Enumerable#grep`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-grep).
  # Returns an array of every element in *enum* for which not `Pattern ===
  # element`.
  #
  # ```ruby
  # (1..10).grep_v 2..5   #=> [1, 6, 7, 8, 9, 10]
  # res =(1..10).grep_v(2..5) { |v| v * 2 }
  # res                    #=> [2, 12, 14, 16, 18, 20]
  # ```
  def grep_v(_); end

  # Groups the collection by result of the block. Returns a hash where the keys
  # are the evaluated result from the block and the values are arrays of
  # elements in the collection that correspond to the key.
  #
  # If no block is given an enumerator is returned.
  #
  # ```ruby
  # (1..6).group_by { |i| i%3 }   #=> {0=>[3, 6], 1=>[1, 4], 2=>[2, 5]}
  # ```
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Hash[T.type_parameter(:U), T::Array[Elem]])
  end
  sig {returns(T::Enumerator[Elem])}
  def group_by(&blk); end

  # Returns `true` if any member of *enum* equals *obj*. Equality is tested
  # using `==`.
  #
  # ```ruby
  # (1..10).include? 5  #=> true
  # (1..10).include? 15 #=> false
  # (1..10).member? 5   #=> true
  # (1..10).member? 15  #=> false
  # ```
  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Boolean)
  end
  def include?(arg0); end

  # Combines all elements of *enum* by applying a binary operation, specified by
  # a block or a symbol that names a method or operator.
  #
  # The *inject* and *reduce* methods are aliases. There is no performance
  # benefit to either.
  #
  # If you specify a block, then for each element in *enum* the block is passed
  # an accumulator value (*memo*) and the element. If you specify a symbol
  # instead, then each element in the collection will be passed to the named
  # method of *memo*. In either case, the result becomes the new value for
  # *memo*. At the end of the iteration, the final value of *memo* is the return
  # value for the method.
  #
  # If you do not explicitly specify an *initial* value for *memo*, then the
  # first element of collection is used as the initial value of *memo*.
  #
  # ```ruby
  # # Sum some numbers
  # (5..10).reduce(:+)                             #=> 45
  # # Same using a block and inject
  # (5..10).inject { |sum, n| sum + n }            #=> 45
  # # Multiply some numbers
  # (5..10).reduce(1, :*)                          #=> 151200
  # # Same using a block
  # (5..10).inject(1) { |product, n| product * n } #=> 151200
  # # find the longest word
  # longest = %w{ cat sheep bear }.inject do |memo, word|
  #    memo.length > word.length ? memo : word
  # end
  # longest                                        #=> "sheep"
  # ```
  sig do
    type_parameters(:U).params(
        initial: T.type_parameter(:U),
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  sig do
    params(
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  sig do
    params(
        initial: T.untyped,
        blk: T.proc.params(arg0: T.untyped, arg1: Elem).returns(T.untyped),
    )
    .returns(T.untyped)
  end
  sig do
    params(
        blk: T.proc.params(arg0: T.untyped, arg1: Elem).returns(T.untyped),
    )
    .returns(T.untyped)
  end
  def inject(initial=T.unsafe(nil), arg0=T.unsafe(nil), &blk); end

  # Returns the object in *enum* with the maximum value. The first form assumes
  # all objects implement
  # [`Comparable`](https://docs.ruby-lang.org/en/2.7.0/Comparable.html); the
  # second uses the block to return *a <=> b*.
  #
  # ```ruby
  # a = %w(albatross dog horse)
  # a.max                                   #=> "horse"
  # a.max { |a, b| a.length <=> b.length }  #=> "albatross"
  # ```
  #
  # If the `n` argument is given, maximum `n` elements are returned as an array,
  # sorted in descending order.
  #
  # ```ruby
  # a = %w[albatross dog horse]
  # a.max(2)                                  #=> ["horse", "dog"]
  # a.max(2) {|a, b| a.length <=> b.length }  #=> ["albatross", "horse"]
  # [5, 1, 3, 4, 2].max(3)                    #=> [5, 4, 3]
  # ```
  sig {returns(T.nilable(Elem))}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def max(arg0=T.unsafe(nil), &blk); end

  # Returns the object in *enum* that gives the maximum value from the given
  # block.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # a = %w(albatross dog horse)
  # a.max_by { |x| x.length }   #=> "albatross"
  # ```
  #
  # If the `n` argument is given, maximum `n` elements are returned as an array.
  # These `n` elements are sorted by the value from the given block, in
  # descending order.
  #
  # ```ruby
  # a = %w[albatross dog horse]
  # a.max_by(2) {|x| x.length } #=> ["albatross", "horse"]
  # ```
  #
  # enum.max\_by(n) can be used to implement weighted random sampling. Following
  # example implements and use Enumerable#wsample.
  #
  # ```ruby
  # module Enumerable
  #   # weighted random sampling.
  #   #
  #   # Pavlos S. Efraimidis, Paul G. Spirakis
  #   # Weighted random sampling with a reservoir
  #   # Information Processing Letters
  #   # Volume 97, Issue 5 (16 March 2006)
  #   def wsample(n)
  #     self.max_by(n) {|v| rand ** (1.0/yield(v)) }
  #   end
  # end
  # e = (-20..20).to_a*10000
  # a = e.wsample(20000) {|x|
  #   Math.exp(-(x/5.0)**2) # normal distribution
  # }
  # # a is 20000 samples from e.
  # p a.length #=> 20000
  # h = a.group_by {|x| x }
  # -10.upto(10) {|x| puts "*" * (h[x].length/30.0).to_i if h[x] }
  # #=> *
  # #   ***
  # #   ******
  # #   ***********
  # #   ******************
  # #   *****************************
  # #   *****************************************
  # #   ****************************************************
  # #   ***************************************************************
  # #   ********************************************************************
  # #   ***********************************************************************
  # #   ***********************************************************************
  # #   **************************************************************
  # #   ****************************************************
  # #   ***************************************
  # #   ***************************
  # #   ******************
  # #   ***********
  # #   *******
  # #   ***
  # #   *
  # ```
  sig {returns(T::Enumerator[Elem])}
  ### The block returning T::Array[BasicObject] is just a stopgap solution to get better
  ### signatures. In reality, it should be recursively defined as an Array of elements of
  ### the same T.any
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(T.any(Comparable, T::Array[BasicObject])),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Enumerator[Elem])
  end
  ### The block returning T::Array[BasicObject] is just a stopgap solution to get better
  ### signatures. In reality, it should be recursively defined as an Array of elements of
  ### the same T.any
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Elem).returns(T.any(Comparable, T::Array[BasicObject])),
    )
    .returns(T::Array[Elem])
  end
  def max_by(arg0=T.unsafe(nil), &blk); end

  # Returns the object in *enum* with the minimum value. The first form assumes
  # all objects implement
  # [`Comparable`](https://docs.ruby-lang.org/en/2.7.0/Comparable.html); the
  # second uses the block to return *a <=> b*.
  #
  # ```ruby
  # a = %w(albatross dog horse)
  # a.min                                   #=> "albatross"
  # a.min { |a, b| a.length <=> b.length }  #=> "dog"
  # ```
  #
  # If the `n` argument is given, minimum `n` elements are returned as a sorted
  # array.
  #
  # ```ruby
  # a = %w[albatross dog horse]
  # a.min(2)                                  #=> ["albatross", "dog"]
  # a.min(2) {|a, b| a.length <=> b.length }  #=> ["dog", "horse"]
  # [5, 1, 3, 4, 2].min(3)                    #=> [1, 2, 3]
  # ```
  sig {returns(T.nilable(Elem))}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def min(arg0=T.unsafe(nil), &blk); end

  # Returns the object in *enum* that gives the minimum value from the given
  # block.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # a = %w(albatross dog horse)
  # a.min_by { |x| x.length }   #=> "dog"
  # ```
  #
  # If the `n` argument is given, minimum `n` elements are returned as an array.
  # These `n` elements are sorted by the value from the given block.
  #
  # ```ruby
  # a = %w[albatross dog horse]
  # p a.min_by(2) {|x| x.length } #=> ["dog", "horse"]
  # ```
  sig {returns(T::Enumerator[Elem])}
  ### The block returning T::Array[BasicObject] is just a stopgap solution to get better
  ### signatures. In reality, it should be recursively defined as an Array of elements of
  ### the same T.any
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(T.any(Comparable, T::Array[BasicObject])),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Enumerator[Elem])
  end
  ### The block returning T::Array[BasicObject] is just a stopgap solution to get better
  ### signatures. In reality, it should be recursively defined as an Array of elements of
  ### the same T.any
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Elem).returns(T.any(Comparable, T::Array[BasicObject])),
    )
    .returns(T::Array[Elem])
  end
  def min_by(arg0=T.unsafe(nil), &blk); end

  # Returns a two element array which contains the minimum and the maximum value
  # in the enumerable. The first form assumes all objects implement
  # [`Comparable`](https://docs.ruby-lang.org/en/2.7.0/Comparable.html); the
  # second uses the block to return *a <=> b*.
  #
  # ```ruby
  # a = %w(albatross dog horse)
  # a.minmax                                  #=> ["albatross", "horse"]
  # a.minmax { |a, b| a.length <=> b.length } #=> ["dog", "albatross"]
  # ```
  sig {returns([T.nilable(Elem), T.nilable(Elem)])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns([T.nilable(Elem), T.nilable(Elem)])
  end
  def minmax(&blk); end

  # Returns a two element array containing the objects in *enum* that correspond
  # to the minimum and maximum values respectively from the given block.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # a = %w(albatross dog horse)
  # a.minmax_by { |x| x.length }   #=> ["dog", "albatross"]
  # ```
  sig {returns([T.nilable(Elem), T.nilable(Elem)])}
  ### The block returning T::Array[BasicObject] is just a stopgap solution to get better
  ### signatures. In reality, it should be recursively defined as an Array of elements of
  ### the same T.any
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(T.any(Comparable, T::Array[BasicObject])),
    )
    .returns(T::Enumerator[Elem])
  end
  def minmax_by(&blk); end

  # Passes each element of the collection to the given block. The method returns
  # `true` if the block never returns `true` for all elements. If the block is
  # not given, `none?` will return `true` only if none of the collection members
  # is true.
  #
  # If instead a pattern is supplied, the method returns whether `pattern ===
  # element` for none of the collection members.
  #
  # ```ruby
  # %w{ant bear cat}.none? { |word| word.length == 5 } #=> true
  # %w{ant bear cat}.none? { |word| word.length >= 4 } #=> false
  # %w{ant bear cat}.none?(/d/)                        #=> true
  # [1, 3.14, 42].none?(Float)                         #=> false
  # [].none?                                           #=> true
  # [nil].none?                                        #=> true
  # [nil, false].none?                                 #=> true
  # [nil, false, true].none?                           #=> false
  # ```
  sig {returns(T::Boolean)}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Boolean)
  end
  sig { params(pattern: T.untyped).returns(T::Boolean) }
  def none?(pattern = nil, &blk); end

  # Passes each element of the collection to the given block. The method returns
  # `true` if the block returns `true` exactly once. If the block is not given,
  # `one?` will return `true` only if exactly one of the collection members is
  # true.
  #
  # If instead a pattern is supplied, the method returns whether `pattern ===
  # element` for exactly one collection member.
  #
  # ```ruby
  # %w{ant bear cat}.one? { |word| word.length == 4 }  #=> true
  # %w{ant bear cat}.one? { |word| word.length > 4 }   #=> false
  # %w{ant bear cat}.one? { |word| word.length < 4 }   #=> false
  # %w{ant bear cat}.one?(/t/)                         #=> false
  # [ nil, true, 99 ].one?                             #=> false
  # [ nil, true, false ].one?                          #=> true
  # [ nil, true, 99 ].one?(Integer)                    #=> true
  # [].one?                                            #=> false
  # ```
  sig {returns(T::Boolean)}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Boolean)
  end
  sig { params(pattern: T.untyped).returns(T::Boolean) }
  def one?(pattern = nil, &blk); end

  # Returns two arrays, the first containing the elements of *enum* for which
  # the block evaluates to true, the second containing the rest.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # (1..6).partition { |v| v.even? }  #=> [[2, 4, 6], [1, 3, 5]]
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns([T::Array[Elem], T::Array[Elem]])
  end
  sig {returns(T::Enumerator[Elem])}
  def partition(&blk); end

  # Returns an array for all elements of `enum` for which the given `block`
  # returns `false`.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # (1..10).reject { |i|  i % 3 == 0 }   #=> [1, 2, 4, 5, 7, 8, 10]
  #
  # [1, 2, 3, 4, 5].reject { |num| num.even? } #=> [1, 3, 5]
  # ```
  #
  # See also
  # [`Enumerable#find_all`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-find_all).
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def reject(&blk); end

  # Builds a temporary array and traverses that array in reverse order.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # (1..3).reverse_each { |v| p v }
  # ```
  #
  # produces:
  #
  # ```ruby
  # 3
  # 2
  # 1
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Enumerator[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def reverse_each(&blk); end

  # Creates an enumerator for each chunked elements. The ends of chunks are
  # defined by *pattern* and the block.
  #
  # If `pattern === elt` returns `true` or the block returns `true` for the
  # element, the element is end of a chunk.
  #
  # The `===` and *block* is called from the first element to the last element
  # of *enum*.
  #
  # The result enumerator yields the chunked elements as an array. So `each`
  # method can be called as follows:
  #
  # ```
  # enum.slice_after(pattern).each { |ary| ... }
  # enum.slice_after { |elt| bool }.each { |ary| ... }
  # ```
  #
  # Other methods of the
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) class
  # and [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html)
  # module, such as `map`, etc., are also usable.
  #
  # For example, continuation lines (lines end with backslash) can be
  # concatenated as follows:
  #
  # ```ruby
  # lines = ["foo\n", "bar\\\n", "baz\n", "\n", "qux\n"]
  # e = lines.slice_after(/(?<!\\)\n\z/)
  # p e.to_a
  # #=> [["foo\n"], ["bar\\\n", "baz\n"], ["\n"], ["qux\n"]]
  # p e.map {|ll| ll[0...-1].map {|l| l.sub(/\\\n\z/, "") }.join + ll.last }
  # #=>["foo\n", "barbaz\n", "\n", "qux\n"]
  # ```
  def slice_after(*_); end

  # Creates an enumerator for each chunked elements. The beginnings of chunks
  # are defined by *pattern* and the block.
  #
  # If `pattern === elt` returns `true` or the block returns `true` for the
  # element, the element is beginning of a chunk.
  #
  # The `===` and *block* is called from the first element to the last element
  # of *enum*. The result for the first element is ignored.
  #
  # The result enumerator yields the chunked elements as an array. So `each`
  # method can be called as follows:
  #
  # ```
  # enum.slice_before(pattern).each { |ary| ... }
  # enum.slice_before { |elt| bool }.each { |ary| ... }
  # ```
  #
  # Other methods of the
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) class
  # and [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html)
  # module, such as `to_a`, `map`, etc., are also usable.
  #
  # For example, iteration over ChangeLog entries can be implemented as follows:
  #
  # ```ruby
  # # iterate over ChangeLog entries.
  # open("ChangeLog") { |f|
  #   f.slice_before(/\A\S/).each { |e| pp e }
  # }
  #
  # # same as above.  block is used instead of pattern argument.
  # open("ChangeLog") { |f|
  #   f.slice_before { |line| /\A\S/ === line }.each { |e| pp e }
  # }
  # ```
  #
  # "svn proplist -R" produces multiline output for each file. They can be
  # chunked as follows:
  #
  # ```ruby
  # IO.popen([{"LC_ALL"=>"C"}, "svn", "proplist", "-R"]) { |f|
  #   f.lines.slice_before(/\AProp/).each { |lines| p lines }
  # }
  # #=> ["Properties on '.':\n", "  svn:ignore\n", "  svk:merge\n"]
  # #   ["Properties on 'goruby.c':\n", "  svn:eol-style\n"]
  # #   ["Properties on 'complex.c':\n", "  svn:mime-type\n", "  svn:eol-style\n"]
  # #   ["Properties on 'regparse.c':\n", "  svn:eol-style\n"]
  # #   ...
  # ```
  #
  # If the block needs to maintain state over multiple elements, local variables
  # can be used. For example, three or more consecutive increasing numbers can
  # be squashed as follows (see `chunk_while` for a better way):
  #
  # ```ruby
  # a = [0, 2, 3, 4, 6, 7, 9]
  # prev = a[0]
  # p a.slice_before { |e|
  #   prev, prev2 = e, prev
  #   prev2 + 1 != e
  # }.map { |es|
  #   es.length <= 2 ? es.join(",") : "#{es.first}-#{es.last}"
  # }.join(",")
  # #=> "0,2-4,6,7,9"
  # ```
  #
  # However local variables should be used carefully if the result enumerator is
  # enumerated twice or more. The local variables should be initialized for each
  # enumeration.
  # [`Enumerator.new`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html#method-c-new)
  # can be used to do it.
  #
  # ```ruby
  # # Word wrapping.  This assumes all characters have same width.
  # def wordwrap(words, maxwidth)
  #   Enumerator.new {|y|
  #     # cols is initialized in Enumerator.new.
  #     cols = 0
  #     words.slice_before { |w|
  #       cols += 1 if cols != 0
  #       cols += w.length
  #       if maxwidth < cols
  #         cols = w.length
  #         true
  #       else
  #         false
  #       end
  #     }.each {|ws| y.yield ws }
  #   }
  # end
  # text = (1..20).to_a.join(" ")
  # enum = wordwrap(text.split(/\s+/), 10)
  # puts "-"*10
  # enum.each { |ws| puts ws.join(" ") } # first enumeration.
  # puts "-"*10
  # enum.each { |ws| puts ws.join(" ") } # second enumeration generates same result as the first.
  # puts "-"*10
  # #=> ----------
  # #   1 2 3 4 5
  # #   6 7 8 9 10
  # #   11 12 13
  # #   14 15 16
  # #   17 18 19
  # #   20
  # #   ----------
  # #   1 2 3 4 5
  # #   6 7 8 9 10
  # #   11 12 13
  # #   14 15 16
  # #   17 18 19
  # #   20
  # #   ----------
  # ```
  #
  # mbox contains series of mails which start with Unix From line. So each mail
  # can be extracted by slice before Unix From line.
  #
  # ```ruby
  # # parse mbox
  # open("mbox") { |f|
  #   f.slice_before { |line|
  #     line.start_with? "From "
  #   }.each { |mail|
  #     unix_from = mail.shift
  #     i = mail.index("\n")
  #     header = mail[0...i]
  #     body = mail[(i+1)..-1]
  #     body.pop if body.last == "\n"
  #     fields = header.slice_before { |line| !" \t".include?(line[0]) }.to_a
  #     p unix_from
  #     pp fields
  #     pp body
  #   }
  # }
  #
  # # split mails in mbox (slice before Unix From line after an empty line)
  # open("mbox") { |f|
  #   emp = true
  #   f.slice_before { |line|
  #     prevemp = emp
  #     emp = line == "\n"
  #     prevemp && line.start_with?("From ")
  #   }.each { |mail|
  #     mail.pop if mail.last == "\n"
  #     pp mail
  #   }
  # }
  # ```
  def slice_before(*_); end

  # Creates an enumerator for each chunked elements. The beginnings of chunks
  # are defined by the block.
  #
  # This method split each chunk using adjacent elements, *elt\_before* and
  # *elt\_after*, in the receiver enumerator. This method split chunks between
  # *elt\_before* and *elt\_after* where the block returns `true`.
  #
  # The block is called the length of the receiver enumerator minus one.
  #
  # The result enumerator yields the chunked elements as an array. So `each`
  # method can be called as follows:
  #
  # ```
  # enum.slice_when { |elt_before, elt_after| bool }.each { |ary| ... }
  # ```
  #
  # Other methods of the
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) class
  # and [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html)
  # module, such as `to_a`, `map`, etc., are also usable.
  #
  # For example, one-by-one increasing subsequence can be chunked as follows:
  #
  # ```ruby
  # a = [1,2,4,9,10,11,12,15,16,19,20,21]
  # b = a.slice_when {|i, j| i+1 != j }
  # p b.to_a #=> [[1, 2], [4], [9, 10, 11, 12], [15, 16], [19, 20, 21]]
  # c = b.map {|a| a.length < 3 ? a : "#{a.first}-#{a.last}" }
  # p c #=> [[1, 2], [4], "9-12", [15, 16], "19-21"]
  # d = c.join(",")
  # p d #=> "1,2,4,9-12,15,16,19-21"
  # ```
  #
  # Near elements (threshold: 6) in sorted array can be chunked as follows:
  #
  # ```ruby
  # a = [3, 11, 14, 25, 28, 29, 29, 41, 55, 57]
  # p a.slice_when {|i, j| 6 < j - i }.to_a
  # #=> [[3], [11, 14], [25, 28, 29, 29], [41], [55, 57]]
  # ```
  #
  # Increasing (non-decreasing) subsequence can be chunked as follows:
  #
  # ```ruby
  # a = [0, 9, 2, 2, 3, 2, 7, 5, 9, 5]
  # p a.slice_when {|i, j| i > j }.to_a
  # #=> [[0, 9], [2, 2, 3], [2, 7], [5, 9], [5]]
  # ```
  #
  # Adjacent evens and odds can be chunked as follows:
  # ([`Enumerable#chunk`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-chunk)
  # is another way to do it.)
  #
  # ```ruby
  # a = [7, 5, 9, 2, 0, 7, 9, 4, 2, 0]
  # p a.slice_when {|i, j| i.even? != j.even? }.to_a
  # #=> [[7, 5, 9], [2, 0], [7, 9], [4, 2, 0]]
  # ```
  #
  # Paragraphs (non-empty lines with trailing empty lines) can be chunked as
  # follows: (See
  # [`Enumerable#chunk`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-chunk)
  # to ignore empty lines.)
  #
  # ```ruby
  # lines = ["foo\n", "bar\n", "\n", "baz\n", "qux\n"]
  # p lines.slice_when {|l1, l2| /\A\s*\z/ =~ l1 && /\S/ =~ l2 }.to_a
  # #=> [["foo\n", "bar\n", "\n"], ["baz\n", "qux\n"]]
  # ```
  #
  # [`Enumerable#chunk_while`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-chunk_while)
  # does the same, except splitting when the block returns `false` instead of
  # `true`.
  def slice_when; end

  # Returns an array containing the items in *enum* sorted.
  #
  # Comparisons for the sort will be done using the items' own `<=>` operator or
  # using an optional code block.
  #
  # The block must implement a comparison between `a` and `b` and return an
  # integer less than 0 when `b` follows `a`, `0` when `a` and `b` are
  # equivalent, or an integer greater than 0 when `a` follows `b`.
  #
  # The result is not guaranteed to be stable. When the comparison of two
  # elements returns `0`, the order of the elements is unpredictable.
  #
  # ```ruby
  # %w(rhea kea flea).sort           #=> ["flea", "kea", "rhea"]
  # (1..10).sort { |a, b| b <=> a }  #=> [10, 9, 8, 7, 6, 5, 4, 3, 2, 1]
  # ```
  #
  # See also
  # [`Enumerable#sort_by`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-sort_by).
  # It implements a Schwartzian transform which is useful when key computation
  # or comparison is expensive.
  sig {returns(T::Array[Elem])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def sort(&blk); end
  ### The block returning T::Array[BasicObject] is just a stopgap solution to get better
  ### signatures. In reality, it should be recursively defined as an Array of elements of
  ### the same T.any
  # Sorts *enum* using a set of keys generated by mapping the values in *enum*
  # through the given block.
  #
  # The result is not guaranteed to be stable. When two keys are equal, the
  # order of the corresponding elements is unpredictable.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # %w{apple pear fig}.sort_by { |word| word.length }
  #               #=> ["fig", "pear", "apple"]
  # ```
  #
  # The current implementation of
  # [`sort_by`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-sort_by)
  # generates an array of tuples containing the original collection element and
  # the mapped value. This makes
  # [`sort_by`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-sort_by)
  # fairly expensive when the keysets are simple.
  #
  # ```ruby
  # require 'benchmark'
  #
  # a = (1..100000).map { rand(100000) }
  #
  # Benchmark.bm(10) do |b|
  #   b.report("Sort")    { a.sort }
  #   b.report("Sort by") { a.sort_by { |a| a } }
  # end
  # ```
  #
  # *produces:*
  #
  # ```
  # user     system      total        real
  # Sort        0.180000   0.000000   0.180000 (  0.175469)
  # Sort by     1.980000   0.040000   2.020000 (  2.013586)
  # ```
  #
  # However, consider the case where comparing the keys is a non-trivial
  # operation. The following code sorts some files on modification time using
  # the basic
  # [`sort`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-sort)
  # method.
  #
  # ```ruby
  # files = Dir["*"]
  # sorted = files.sort { |a, b| File.new(a).mtime <=> File.new(b).mtime }
  # sorted   #=> ["mon", "tues", "wed", "thurs"]
  # ```
  #
  # This sort is inefficient: it generates two new
  # [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) objects during every
  # comparison. A slightly better technique is to use the
  # [`Kernel#test`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-test)
  # method to generate the modification times directly.
  #
  # ```ruby
  # files = Dir["*"]
  # sorted = files.sort { |a, b|
  #   test(?M, a) <=> test(?M, b)
  # }
  # sorted   #=> ["mon", "tues", "wed", "thurs"]
  # ```
  #
  # This still generates many unnecessary
  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) objects. A more
  # efficient technique is to cache the sort keys (modification times in this
  # case) before the sort. Perl users often call this approach a Schwartzian
  # transform, after Randal Schwartz. We construct a temporary array, where each
  # element is an array containing our sort key along with the filename. We sort
  # this array, and then extract the filename from the result.
  #
  # ```ruby
  # sorted = Dir["*"].collect { |f|
  #    [test(?M, f), f]
  # }.sort.collect { |f| f[1] }
  # sorted   #=> ["mon", "tues", "wed", "thurs"]
  # ```
  #
  # This is exactly what
  # [`sort_by`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-sort_by)
  # does internally.
  #
  # ```ruby
  # sorted = Dir["*"].sort_by { |f| test(?M, f) }
  # sorted   #=> ["mon", "tues", "wed", "thurs"]
  # ```
  #
  # To produce the reverse of a specific order, the following can be used:
  #
  # ```
  # ary.sort_by { ... }.reverse!
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(T.any(Comparable, T::Array[BasicObject])),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def sort_by(&blk); end

  # Returns the sum of elements in an
  # [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html).
  #
  # If a block is given, the block is applied to each element before addition.
  #
  # If *enum* is empty, it returns *init*.
  #
  # For example:
  #
  # ```ruby
  # { 1 => 10, 2 => 20 }.sum {|k, v| k * v }  #=> 50
  # (1..10).sum                               #=> 55
  # (1..10).sum {|v| v * 2 }                  #=> 110
  # ('a'..'z').sum                            #=> TypeError
  # ```
  #
  # This method can be used for non-numeric objects by explicit *init* argument.
  #
  # ```ruby
  # { 1 => 10, 2 => 20 }.sum([])                   #=> [1, 10, 2, 20]
  # "a\nb\nc".each_line.lazy.map(&:chomp).sum("")  #=> "abc"
  # ```
  #
  # If the method is applied to an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) range without
  # a block, the sum is not done by iteration, but instead using Gauss's
  # summation formula.
  #
  # [`Enumerable#sum`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-sum)
  # method may not respect method redefinition of "+" methods such as
  # [`Integer#+`](https://docs.ruby-lang.org/en/2.7.0/Integer.html#method-i-2B),
  # or "each" methods such as
  # [`Range#each`](https://docs.ruby-lang.org/en/2.7.0/Range.html#method-i-each).
  sig do
    returns(T.all(Elem, Numeric))
  end
  sig do
    params(
      init: BasicObject,
    )
    .returns(T.untyped)
  end
  sig do
    type_parameters(:U)
      .params(
        blk: T.proc.params(arg0: Elem).returns(T.all(Numeric, T.type_parameter(:U)))
      )
      .returns(T.type_parameter(:U))
  end
  sig do
    params(
      init: BasicObject,
      blk: T.proc.params(arg0: Elem).returns(T.untyped)
    )
    .returns(T.untyped)
  end
  def sum(init=0, &blk); end

  # Returns first n elements from *enum*.
  #
  # ```ruby
  # a = [1, 2, 3, 4, 5, 0]
  # a.take(3)             #=> [1, 2, 3]
  # a.take(30)            #=> [1, 2, 3, 4, 5, 0]
  # ```
  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Elem])
  end
  def take(n); end

  # Passes elements to the block until the block returns `nil` or `false`, then
  # stops iterating and returns an array of all prior elements.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # a = [1, 2, 3, 4, 5, 0]
  # a.take_while { |i| i < 3 }   #=> [1, 2]
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def take_while(&blk); end

  # Tallies the collection, i.e., counts the occurrences of each element.
  # Returns a hash with the elements of the collection as keys and the
  # corresponding counts as values.
  #
  # ```ruby
  # ["a", "b", "c", "b"].tally  #=> {"a"=>1, "b"=>2, "c"=>1}
  # ```
  #
  # If a hash is given, the number of occurrences is added to each value
  # in the hash, and the hash is returned. The value corresponding to
  # each element must be an integer.
  sig {params(hash: T::Hash[Elem, Integer]).returns(T::Hash[Elem, Integer])}
  def tally(hash = {}); end

  ### Implemented in C++

  # Returns the result of interpreting *enum* as a list of `[key, value]` pairs.
  #
  # ```ruby
  # %i[hello world].each_with_index.to_h
  #   # => {:hello => 0, :world => 1}
  # ```
  #
  # If a block is given, the results of the block on each element of the enum
  # will be used as pairs.
  #
  # ```ruby
  # (1..5).to_h {|x| [x, x ** 2]}
  #   #=> {1=>1, 2=>4, 3=>9, 4=>16, 5=>25}
  # ```
  sig do
    type_parameters(:U, :V).params(
      blk: T.nilable(T.proc.params(arg0: Elem).returns([T.type_parameter(:U), T.type_parameter(:V)])))
      .returns(T::Hash[T.type_parameter(:U), T.type_parameter(:V)])
  end
  def to_h(&blk); end

  # Returns a new array by removing duplicate values in `self`.
  #
  # See also
  # [`Array#uniq`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-uniq).
  def uniq(&blk); end

  # Iterates the given block for each slice of <n> elements. If no block is
  # given, returns an enumerator.
  #
  # ```ruby
  # (1..10).each_slice(3) { |a| p a }
  # # outputs below
  # [1, 2, 3]
  # [4, 5, 6]
  # [7, 8, 9]
  # [10]
  # ```
  sig do
    params(
        n: Integer,
        blk: T.proc.params(arg0: T::Array[Elem]).returns(BasicObject),
    )
    .returns(NilClass)
  end
  sig do
    params(
        n: Integer,
    )
    .returns(T::Enumerator[T::Array[Elem]])
  end
  def each_slice(n, &blk); end

  # Passes each entry in *enum* to *block*. Returns the first for which *block*
  # is not false. If no object matches, calls *ifnone* and returns its result
  # when it is specified, or returns `nil` otherwise.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # (1..100).detect  #=> #<Enumerator: 1..100:detect>
  # (1..100).find    #=> #<Enumerator: 1..100:find>
  #
  # (1..10).detect         { |i| i % 5 == 0 && i % 7 == 0 }   #=> nil
  # (1..10).find           { |i| i % 5 == 0 && i % 7 == 0 }   #=> nil
  # (1..10).detect(-> {0}) { |i| i % 5 == 0 && i % 7 == 0 }   #=> 0
  # (1..10).find(-> {0})   { |i| i % 5 == 0 && i % 7 == 0 }   #=> 0
  # (1..100).detect        { |i| i % 5 == 0 && i % 7 == 0 }   #=> 35
  # (1..100).find          { |i| i % 5 == 0 && i % 7 == 0 }   #=> 35
  # ```
  sig do
    type_parameters(:U)
      .params(
        ifnone: T.proc.returns(T.type_parameter(:U)),
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
      )
      .returns(T.any(T.type_parameter(:U), Elem))
  end
  sig do
    type_parameters(:U)
      .params(
        ifnone: T.proc.returns(T.type_parameter(:U)),
      )
      .returns(T::Enumerator[T.any(T.type_parameter(:U), Elem)])
  end
  sig do
    params(
      blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    returns(T::Enumerator[Elem])
  end
  def find(ifnone=T.unsafe(nil), &blk); end

  ### N.B. this signature is wrong; Our generic method implementation
  ### cannot model the correct signature, so we pass through the return
  ### type of the block and then fix it up in an ad-hoc way in Ruby. A
  ### more-correct signature might be:
  ###   sig do
  ###     type_parameters(:U).params(
  ###       blk: T.proc.params(arg0: Elem).returns(T.any(T::Array[T.type_parameter(:U)], T.type_parameter(:U)),
  ###     )
  ###     .returns(T.type_parameter(:U))
  ###   end
  ###
  ### But that would require a lot more sophistication from our generic
  ### method inference.

  # Returns a new array with the concatenated results of running *block* once
  # for every element in *enum*.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # [1, 2, 3, 4].flat_map { |e| [e, -e] } #=> [1, -1, 2, -2, 3, -3, 4, -4]
  # [[1, 2], [3, 4]].flat_map { |e| e + [100] } #=> [1, 2, 100, 3, 4, 100]
  # ```
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  sig {returns(T::Enumerator[Elem])}
  def flat_map(&blk); end

  # Returns a new array with the results of running *block* once for every
  # element in *enum*.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # (1..4).map { |i| i*i }      #=> [1, 4, 9, 16]
  # (1..4).collect { "cat"  }   #=> ["cat", "cat", "cat", "cat"]
  # ```
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  sig {returns(T::Enumerator[Elem])}
  def map(&blk); end

  # Returns `true` if any member of *enum* equals *obj*. Equality is tested
  # using `==`.
  #
  # ```ruby
  # (1..10).include? 5  #=> true
  # (1..10).include? 15 #=> false
  # (1..10).member? 5   #=> true
  # (1..10).member? 15  #=> false
  # ```
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def member?(arg0); end

  # Combines all elements of *enum* by applying a binary operation, specified by
  # a block or a symbol that names a method or operator.
  #
  # The *inject* and *reduce* methods are aliases. There is no performance
  # benefit to either.
  #
  # If you specify a block, then for each element in *enum* the block is passed
  # an accumulator value (*memo*) and the element. If you specify a symbol
  # instead, then each element in the collection will be passed to the named
  # method of *memo*. In either case, the result becomes the new value for
  # *memo*. At the end of the iteration, the final value of *memo* is the return
  # value for the method.
  #
  # If you do not explicitly specify an *initial* value for *memo*, then the
  # first element of collection is used as the initial value of *memo*.
  #
  # ```ruby
  # # Sum some numbers
  # (5..10).reduce(:+)                             #=> 45
  # # Same using a block and inject
  # (5..10).inject { |sum, n| sum + n }            #=> 45
  # # Multiply some numbers
  # (5..10).reduce(1, :*)                          #=> 151200
  # # Same using a block
  # (5..10).inject(1) { |product, n| product * n } #=> 151200
  # # find the longest word
  # longest = %w{ cat sheep bear }.inject do |memo, word|
  #    memo.length > word.length ? memo : word
  # end
  # longest                                        #=> "sheep"
  # ```
  sig do
    type_parameters(:U).params(
        initial: T.type_parameter(:U),
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  sig do
    params(
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  sig do
    type_parameters(:U).params(
        initial: T.type_parameter(:U),
        blk: T.proc.params(arg0: T.type_parameter(:U), arg1: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: T.any(Elem, T.type_parameter(:U)), arg1: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T.nilable(T.type_parameter(:U)))
  end
  def reduce(initial=T.unsafe(nil), arg0=T.unsafe(nil), &blk); end

  # Returns an array containing all elements of `enum` for which the given
  # `block` returns a true value.
  #
  # The *find\_all* and *select* methods are aliases. There is no performance
  # benefit to either.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # (1..10).find_all { |i|  i % 3 == 0 }   #=> [3, 6, 9]
  #
  # [1,2,3,4,5].select { |num|  num.even?  }   #=> [2, 4]
  #
  # [:foo, :bar].filter { |x| x == :foo }   #=> [:foo]
  # ```
  #
  # See also
  # [`Enumerable#reject`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-reject),
  # [`Enumerable#grep`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-grep).
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def select(&blk); end

  # Returns an array containing all elements of `enum` for which the given
  # `block` returns a true value.
  #
  # The *find\_all* and *select* methods are aliases. There is no performance
  # benefit to either.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # (1..10).find_all { |i|  i % 3 == 0 }   #=> [3, 6, 9]
  #
  # [1,2,3,4,5].select { |num|  num.even?  }   #=> [2, 4]
  #
  # [:foo, :bar].filter { |x| x == :foo }   #=> [:foo]
  # ```
  #
  # See also
  # [`Enumerable#reject`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-reject),
  # [`Enumerable#grep`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-grep).
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T::Enumerator[Elem])}
  def filter(&blk); end

  # Returns an array containing the items in *enum*.
  #
  # ```ruby
  # (1..7).to_a                       #=> [1, 2, 3, 4, 5, 6, 7]
  # { 'a'=>1, 'b'=>2, 'c'=>3 }.to_a   #=> [["a", 1], ["b", 2], ["c", 3]]
  #
  # require 'prime'
  # Prime.entries 10                  #=> [2, 3, 5, 7]
  # ```
  sig {returns(T::Array[Elem])}
  def to_a(); end

  # Returns an
  # [`Enumerator::Lazy`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html),
  # which redefines most
  # [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) methods
  # to postpone enumeration and enumerate values only on an as-needed basis.
  #
  # ### Example
  #
  # The following program finds pythagorean triples:
  #
  # ```ruby
  # def pythagorean_triples
  #   (1..Float::INFINITY).lazy.flat_map {|z|
  #     (1..z).flat_map {|x|
  #       (x..z).select {|y|
  #         x**2 + y**2 == z**2
  #       }.map {|y|
  #         [x, y, z]
  #       }
  #     }
  #   }
  # end
  # # show first ten pythagorean triples
  # p pythagorean_triples.take(10).force # take is lazy, so force is needed
  # p pythagorean_triples.first(10)      # first is eager
  # # show pythagorean triples less than 100
  # p pythagorean_triples.take_while { |*, z| z < 100 }.force
  # ```
  sig { returns(T::Enumerator::Lazy[Elem])}
  def lazy(); end

  # Takes one element from *enum* and merges corresponding elements from each
  # *args*. This generates a sequence of *n*-element arrays, where *n* is one
  # more than the count of arguments. The length of the resulting sequence will
  # be `enum#size`. If the size of any argument is less than `enum#size`, `nil`
  # values are supplied. If a block is given, it is invoked for each output
  # array, otherwise an array of arrays is returned.
  #
  # ```ruby
  # a = [ 4, 5, 6 ]
  # b = [ 7, 8, 9 ]
  #
  # a.zip(b)                 #=> [[4, 7], [5, 8], [6, 9]]
  # [1, 2, 3].zip(a, b)      #=> [[1, 4, 7], [2, 5, 8], [3, 6, 9]]
  # [1, 2].zip(a, b)         #=> [[1, 4, 7], [2, 5, 8]]
  # a.zip([1, 2], [8])       #=> [[4, 1, 8], [5, 2, nil], [6, nil, nil]]
  #
  # c = []
  # a.zip(b) { |x, y| c << x + y }  #=> nil
  # c                               #=> [11, 13, 15]
  # ```
  sig do
    type_parameters(:U).params(
        arg: T::Enumerable[T.type_parameter(:U)]
    ).returns(T::Array[[Elem, T.nilable(T.type_parameter(:U))]])
  end
  sig do
    type_parameters(:U).params(
        arg: T::Enumerable[T.type_parameter(:U)],
        blk: T.proc.params(x: Elem, y: T.nilable(T.type_parameter(:U))).void
    ).void
  end
  def zip(*arg, &blk); end
end
