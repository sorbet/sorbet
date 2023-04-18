# typed: __STDLIB_INTERNAL

# This class provides a complete interface to
# [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) files and data. It
# offers tools to enable you to read and write to and from Strings or
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects, as needed.
#
# The most generic interface of the library is:
#
# ```ruby
# csv = CSV.new(string_or_io, **options)
#
# # Reading: IO object should be open for read
# csv.read # => array of rows
# # or
# csv.each do |row|
#   # ...
# end
# # or
# row = csv.shift
#
# # Writing: IO object should be open for write
# csv << row
# ```
#
# There are several specialized class methods for one-statement reading or
# writing, described in the Specialized Methods section.
#
# If a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) is passed
# into [`::new`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new), it
# is internally wrapped into a
# [`StringIO`](https://docs.ruby-lang.org/en/2.7.0/StringIO.html) object.
#
# `options` can be used for specifying the particular
# [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) flavor (column
# separators, row separators, value quoting and so on), and for data conversion,
# see [`Data`](https://docs.ruby-lang.org/en/2.7.0/Data.html) Conversion section
# for the description of the latter.
#
# ## Specialized Methods
#
# ### Reading
#
# ```ruby
# # From a file: all at once
# arr_of_rows = CSV.read("path/to/file.csv", **options)
# # iterator-style:
# CSV.foreach("path/to/file.csv", **options) do |row|
#   # ...
# end
#
# # From a string
# arr_of_rows = CSV.parse("CSV,data,String", **options)
# # or
# CSV.parse("CSV,data,String", **options) do |row|
#   # ...
# end
# ```
#
# ### Writing
#
# ```ruby
# # To a file
# CSV.open("path/to/file.csv", "wb") do |csv|
#   csv << ["row", "of", "CSV", "data"]
#   csv << ["another", "row"]
#   # ...
# end
#
# # To a String
# csv_string = CSV.generate do |csv|
#   csv << ["row", "of", "CSV", "data"]
#   csv << ["another", "row"]
#   # ...
# end
# ```
#
# ### Shortcuts
#
# ```ruby
# # Core extensions for converting one line
# csv_string = ["CSV", "data"].to_csv   # to CSV
# csv_array  = "CSV,String".parse_csv   # from CSV
#
# # CSV() method
# CSV             { |csv_out| csv_out << %w{my data here} }  # to $stdout
# CSV(csv = "")   { |csv_str| csv_str << %w{my data here} }  # to a String
# CSV($stderr)    { |csv_err| csv_err << %w{my data here} }  # to $stderr
# CSV($stdin)     { |csv_in|  csv_in.each { |row| p row } }  # from $stdin
# ```
#
# ## [`Data`](https://docs.ruby-lang.org/en/2.7.0/Data.html) Conversion
#
# ### [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) with headers
#
# [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) allows to specify column
# names of [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) file, whether
# they are in data, or provided separately. If headers are specified, reading
# methods return an instance of
# [`CSV::Table`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html), consisting
# of [`CSV::Row`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html).
#
# ```ruby
# # Headers are part of data
# data = CSV.parse(<<~ROWS, headers: true)
#   Name,Department,Salary
#   Bob,Engineering,1000
#   Jane,Sales,2000
#   John,Management,5000
# ROWS
#
# data.class      #=> CSV::Table
# data.first      #=> #<CSV::Row "Name":"Bob" "Department":"Engineering" "Salary":"1000">
# data.first.to_h #=> {"Name"=>"Bob", "Department"=>"Engineering", "Salary"=>"1000"}
#
# # Headers provided by developer
# data = CSV.parse('Bob,Engineering,1000', headers: %i[name department salary])
# data.first      #=> #<CSV::Row name:"Bob" department:"Engineering" salary:"1000">
# ```
#
# ### Typed data reading
#
# [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) allows to provide a set
# of data *converters* e.g. transformations to try on input data. Converter
# could be a symbol from
# [`CSV::Converters`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#Converters)
# constant's keys, or lambda.
#
# ```ruby
# # Without any converters:
# CSV.parse('Bob,2018-03-01,100')
# #=> [["Bob", "2018-03-01", "100"]]
#
# # With built-in converters:
# CSV.parse('Bob,2018-03-01,100', converters: %i[numeric date])
# #=> [["Bob", #<Date: 2018-03-01>, 100]]
#
# # With custom converters:
# CSV.parse('Bob,2018-03-01,100', converters: [->(v) { Time.parse(v) rescue v }])
# #=> [["Bob", 2018-03-01 00:00:00 +0200, "100"]]
# ```
#
# ## [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) and Character Encodings (M17n or Multilingualization)
#
# This new [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) parser is m17n
# savvy. The parser works in the
# [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) of the
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) or
# [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) object being read
# from or written to. Your data is never transcoded (unless you ask Ruby to
# transcode it for you) and will literally be parsed in the
# [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) it is in. Thus
# [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) will return Arrays or
# Rows of Strings in the
# [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) of your data.
# This is accomplished by transcoding the parser itself into your
# [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html).
#
# Some transcoding must take place, of course, to accomplish this multiencoding
# support. For example, `:col_sep`, `:row_sep`, and `:quote_char` must be
# transcoded to match your data. Hopefully this makes the entire process feel
# transparent, since CSV's defaults should just magically work for your data.
# However, you can set these values manually in the target
# [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) to avoid the
# translation.
#
# It's also important to note that while all of CSV's core parser is now
# [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) agnostic, some
# features are not. For example, the built-in converters will try to transcode
# data to UTF-8 before making conversions. Again, you can provide custom
# converters that are aware of your Encodings to avoid this translation. It's
# just too hard for me to support native conversions in all of Ruby's Encodings.
#
# Anyway, the practical side of this is simple: make sure
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) and
# [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) objects passed
# into [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) have the proper
# [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) set and
# everything should just work.
# [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) methods that allow you
# to open [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects
# ([`CSV::foreach()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-foreach),
# [`CSV::open()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-open),
# [`CSV::read()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-read),
# and
# [`CSV::readlines()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-readlines))
# do allow you to specify the
# [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html).
#
# One minor exception comes when generating
# [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) into a
# [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) with an
# [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) that is not
# ASCII compatible. There's no existing data for
# [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) to use to prepare itself
# and thus you will probably need to manually specify the desired
# [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) for most of
# those cases. It will try to guess using the fields in a row of output though,
# when using
# [`CSV::generate_line()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-generate_line)
# or Array#to\_csv().
#
# I try to point out any other
# [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) issues in the
# documentation of methods as they come up.
#
# This has been tested to the best of my ability with all non-"dummy" Encodings
# Ruby ships with. However, it is brave new code and may have some bugs. Please
# feel free to [report](mailto:james@grayproductions.net) any issues you find
# with it.
class CSV < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out) {{fixed: T::Array[T.nilable(String)]}}

  # The options used when no overrides are given by calling code. They are:
  #
  # **`:col_sep`**
  # :   `","`
  # **`:row_sep`**
  # :   `:auto`
  # **`:quote_char`**
  # :   `'"'`
  # **`:field_size_limit`**
  # :   `nil`
  # **`:converters`**
  # :   `nil`
  # **`:unconverted_fields`**
  # :   `nil`
  # **`:headers`**
  # :   `false`
  # **`:return_headers`**
  # :   `false`
  # **`:header_converters`**
  # :   `nil`
  # **`:skip_blanks`**
  # :   `false`
  # **`:force_quotes`**
  # :   `false`
  # **`:skip_lines`**
  # :   `nil`
  # **`:liberal_parsing`**
  # :   `false`
  # **`:quote_empty`**
  # :   `true`
  DEFAULT_OPTIONS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  # The version of the installed library.
  VERSION = T.let(T.unsafe(nil), String)

  # This method is intended as the primary interface for reading
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) files. You pass a
  # `path` and any `options` you wish to set for the read. Each row of file will
  # be passed to the provided `block` in turn.
  #
  # The `options` parameter can be anything
  # [`CSV::new()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new)
  # understands. This method also understands an additional `:encoding`
  # parameter that you can use to specify the
  # [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) of the data
  # in the file to be read. You must provide this unless your data is in
  # [`Encoding::default_external()`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html#method-c-default_external).
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) will use this to
  # determine how to parse the data. You may provide a second
  # [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) to have the
  # data transcoded as it is read. For example, `encoding: "UTF-32BE:UTF-8"`
  # would read UTF-32BE data from the file but transcode it to UTF-8 before
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) parses it.
  sig do
    params(
        path: T.any(String, ::Sorbet::Private::Static::IOLike),
        mode: String,
        options: BasicObject,
        blk: T.nilable(T.proc.params(arg0: T.any(T::Array[T.untyped], CSV::Row)).void),
    )
    .returns(T.nilable(T::Enumerator[T.any(T::Array[T.untyped], CSV::Row)]))
  end
  def self.foreach(path, mode="r", **options, &blk); end

  # This constructor will wrap either a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) or
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object passed in `data`
  # for reading and/or writing. In addition to the
  # [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) instance methods,
  # several [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) methods are
  # delegated. (See
  # [`CSV::open()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-c-open)
  # for a complete list.)  If you pass a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) for `data`, you
  # can later retrieve it (after writing to it, for example) with CSV.string().
  #
  # Note that a wrapped
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) will be
  # positioned at the beginning (for reading). If you want it at the end (for
  # writing), use
  # [`CSV::generate()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-c-generate).
  # If you want any other positioning, pass a preset
  # [`StringIO`](https://docs.ruby-lang.org/en/2.6.0/StringIO.html) object
  # instead.
  #
  # You may set any reading and/or writing preferences in the `options`
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html). Available options
  # are:
  #
  # **`:col_sep`**
  # :   The [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) placed
  #     between each field. This
  #     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) will be
  #     transcoded into the data's
  #     [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) before
  #     parsing.
  # **`:row_sep`**
  # :   The [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) appended
  #     to the end of each row. This can be set to the special `:auto` setting,
  #     which requests that
  #     [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) automatically
  #     discover this from the data. Auto-discovery reads ahead in the data
  #     looking for the next `"\r\n"`, `"\n"`, or `"\r"` sequence. A sequence
  #     will be selected even if it occurs in a quoted field, assuming that you
  #     would have the same line endings there. If none of those sequences is
  #     found, `data` is `ARGF`, `STDIN`, `STDOUT`, or `STDERR`, or the stream
  #     is only available for output, the default `$INPUT_RECORD_SEPARATOR`
  #     (`$/`) is used. Obviously, discovery takes a little time.
  #     [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) manually if speed
  #     is important. Also note that
  #     [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) objects should be
  #     opened in binary mode on Windows if this feature will be used as the
  #     line-ending translation can cause problems with resetting the document
  #     position to where it was before the read ahead. This
  #     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) will be
  #     transcoded into the data's
  #     [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) before
  #     parsing.
  # **`:quote_char`**
  # :   The character used to quote fields. This has to be a single character
  #     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html). This is
  #     useful for application that incorrectly use `'` as the quote character
  #     instead of the correct `"`.
  #     [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) will always
  #     consider a double sequence of this character to be an escaped quote.
  #     This [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) will be
  #     transcoded into the data's
  #     [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) before
  #     parsing.
  # **`:field_size_limit`**
  # :   This is a maximum size
  #     [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) will read ahead
  #     looking for the closing quote for a field. (In truth, it reads to the
  #     first line ending beyond this size.)  If a quote cannot be found within
  #     the limit [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) will
  #     raise a MalformedCSVError, assuming the data is faulty. You can use this
  #     limit to prevent what are effectively DoS attacks on the parser.
  #     However, this limit can cause a legitimate parse to fail and thus is set
  #     to `nil`, or off, by default.
  # **`:converters`**
  # :   An [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of names
  #     from the
  #     [`Converters`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#Converters)
  #     [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) and/or lambdas
  #     that handle custom conversion. A single converter doesn't have to be in
  #     an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html). All
  #     built-in converters try to transcode fields to UTF-8 before converting.
  #     The conversion will fail if the data cannot be transcoded, leaving the
  #     field unchanged.
  # **`:unconverted_fields`**
  # :   If set to `true`, an unconverted\_fields() method will be added to all
  #     returned rows (Array or
  #     [`CSV::Row`](https://docs.ruby-lang.org/en/2.6.0/CSV/Row.html)) that
  #     will return the fields as they were before conversion. Note that
  #     `:headers` supplied by
  #     [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) or
  #     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) were not
  #     fields of the document and thus will have an empty
  #     [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) attached.
  # **`:headers`**
  # :   If set to `:first_row` or `true`, the initial row of the
  #     [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) file will be
  #     treated as a row of headers. If set to an
  #     [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html), the contents
  #     will be used as the headers. If set to a
  #     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html), the
  #     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) is run
  #     through a call of
  #     [`CSV::parse_line()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-c-parse_line)
  #     with the same `:col_sep`, `:row_sep`, and `:quote_char` as this instance
  #     to produce an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html)
  #     of headers. This setting causes
  #     [`CSV#shift()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-i-shift)
  #     to return rows as
  #     [`CSV::Row`](https://docs.ruby-lang.org/en/2.6.0/CSV/Row.html) objects
  #     instead of Arrays and
  #     [`CSV#read()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-i-read)
  #     to return
  #     [`CSV::Table`](https://docs.ruby-lang.org/en/2.6.0/CSV/Table.html)
  #     objects instead of an
  #     [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of Arrays.
  # **`:return_headers`**
  # :   When `false`, header rows are silently swallowed. If set to `true`,
  #     header rows are returned in a
  #     [`CSV::Row`](https://docs.ruby-lang.org/en/2.6.0/CSV/Row.html) object
  #     with identical headers and fields (save that the fields do not go
  #     through the converters).
  # **`:write_headers`**
  # :   When `true` and `:headers` is set, a header row will be added to the
  #     output.
  # **`:header_converters`**
  # :   Identical in functionality to `:converters` save that the conversions
  #     are only made to header rows. All built-in converters try to transcode
  #     headers to UTF-8 before converting. The conversion will fail if the data
  #     cannot be transcoded, leaving the header unchanged.
  # **`:skip_blanks`**
  # :   When set to a `true` value,
  #     [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) will skip over any
  #     empty rows. Note that this setting will not skip rows that contain
  #     column separators, even if the rows contain no actual data. If you want
  #     to skip rows that contain separators but no content, consider using
  #     `:skip_lines`, or inspecting fields.compact.empty? on each row.
  # **`:force_quotes`**
  # :   When set to a `true` value,
  #     [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) will quote all
  #     [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) fields it creates.
  # **`:skip_lines`**
  # :   When set to an object responding to `match`, every line matching it is
  #     considered a comment and ignored during parsing. When set to a
  #     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html), it is first
  #     converted to a
  #     [`Regexp`](https://docs.ruby-lang.org/en/2.6.0/Regexp.html). When set to
  #     `nil` no line is considered a comment. If the passed object does not
  #     respond to `match`, `ArgumentError` is thrown.
  # **`:liberal_parsing`**
  # :   When set to a `true` value,
  #     [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) will attempt to
  #     parse input not conformant with RFC 4180, such as double quotes in
  #     unquoted fields.
  # **`:nil_value`**
  # :   When set an object, any values of an empty field are replaced by the set
  #     object, not nil.
  # **`:empty_value`**
  # :   When set an object, any values of a blank string field is replaced by
  #     the set object.
  # **`:quote_empty`**
  # :   TODO
  # **`:write_converters`**
  # :   TODO
  # **`:write_nil_value`**
  # :   TODO
  # **`:write_empty_value`**
  # :   TODO
  # **`:strip`**
  # :   TODO
  #
  #
  # See
  # [`CSV::DEFAULT_OPTIONS`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#DEFAULT_OPTIONS)
  # for the default settings.
  #
  # Options cannot be overridden in the instance methods for performance
  # reasons, so be sure to set what you want here.
  sig do
    params(
        io: T.any(::Sorbet::Private::Static::IOLike, String),
        options: T.untyped,
    )
    .void
  end
  def initialize(io=T.unsafe(nil), **options); end

  # This method can be used to easily parse
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) out of a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). You may either
  # provide a `block` which will be called with each row of the
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) in turn, or just
  # use the returned [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
  # of Arrays (when no `block` is given).
  #
  # You pass your `str` to read from, and an optional `options` containing
  # anything
  # [`CSV::new()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new)
  # understands.
  sig do
    params(
        str: T.any(String, ::Sorbet::Private::Static::IOLike),
        options: T.untyped,
    )
    .returns(
      T.any(
        CSV::Table,
        T::Array[T::Array[T.untyped]],
      )
    )
  end
  sig do
    params(
        str: T.any(String, ::Sorbet::Private::Static::IOLike),
        options: T.untyped,
        blk: T.proc.params(arg0: T.any(CSV::Row, T::Array[T.untyped])).void
    ).void
  end
  def self.parse(str, **options, &blk); end

  # This method is a shortcut for converting a single line of a
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html)
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) into an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html). Note that if
  # `line` contains multiple rows, anything beyond the first row is ignored.
  #
  # The `options` parameter can be anything
  # [`CSV::new()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new)
  # understands.
  sig do
    params(
        str: String,
        options: T::Hash[Symbol, T.untyped],
    )
    .returns(T.nilable(T::Array[T.nilable(String)]))
  end
  def self.parse_line(str, options=T.unsafe(nil)); end

  # Slurps the remaining rows and returns an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of Arrays.
  #
  # The data source must be open for reading.
  #
  # Also aliased as:
  # [`readlines`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-readlines)
  sig {returns(T::Array[T::Array[T.nilable(String)]])}
  def read; end

  # Alias for:
  # [`shift`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-shift)
  sig {returns(T.nilable(T::Array[T.nilable(String)]))}
  def readline; end

  # Use to slurp a [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) file
  # into an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of Arrays.
  # Pass the `path` to the file and any `options`
  # [`CSV::new()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new)
  # understands. This method also understands an additional `:encoding`
  # parameter that you can use to specify the
  # [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) of the data
  # in the file to be read. You must provide this unless your data is in
  # [`Encoding::default_external()`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html#method-c-default_external).
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) will use this to
  # determine how to parse the data. You may provide a second
  # [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) to have the
  # data transcoded as it is read. For example, `encoding: "UTF-32BE:UTF-8"`
  # would read UTF-32BE data from the file but transcode it to UTF-8 before
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) parses it.
  sig do
    params(
        path: String,
        options: T::Hash[Symbol, T.untyped],
    )
    .returns(T::Array[T::Array[T.nilable(String)]])
  end
  def self.read(path, options=T.unsafe(nil)); end

  # The primary write method for wrapped Strings and IOs, `row` (an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) or
  # [`CSV::Row`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html)) is converted
  # to [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) and appended to the
  # data source. When a
  # [`CSV::Row`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html) is passed,
  # only the row's fields() are appended to the output.
  #
  # The data source must be open for writing.
  #
  # Also aliased as:
  # [`add_row`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-add_row),
  # [`puts`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-puts)
  sig { params(row: T.any(T::Array[T.untyped], CSV::Row)).void }
  def <<(row); end

  # Alias for:
  # [`<<`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-3C-3C)
  sig { params(row: T.any(T::Array[T.untyped], CSV::Row)).void }
  def add_row(row); end

  # This method wraps a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) you provide, or
  # an empty default
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), in a
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) object which is passed
  # to the provided block. You can use the block to append
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) rows to the
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) and when the
  # block exits, the final
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) will be
  # returned.
  #
  # Note that a passed
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) **is** modified
  # by this method. Call dup() before passing if you need a new
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  #
  # The `options` parameter can be anything
  # [`CSV::new()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new)
  # understands. This method understands an additional `:encoding` parameter
  # when not passed a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) to set the base
  # [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) for the
  # output. [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) needs this
  # hint if you plan to output non-ASCII compatible data.
  sig { params(str: String, options: T.untyped, blk: T.proc.params(csv: CSV).void).returns(String) }
  def self.generate(str = "", **options, &blk); end

  # This method is a shortcut for converting a single row
  # ([`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)) into a
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html)
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  #
  # The `options` parameter can be anything
  # [`CSV::new()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new)
  # understands. This method understands an additional `:encoding` parameter to
  # set the base [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html)
  # for the output. This method will try to guess your
  # [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) from the
  # first non-`nil` field in `row`, if possible, but you may need to use this
  # parameter as a backup plan.
  #
  # The `:row_sep` `option` defaults to `$INPUT_RECORD_SEPARATOR` (`$/`) when
  # calling this method.
  sig { params(row: T::Array[T.nilable(String)], options: T.untyped).returns(String) }
  def self.generate_line(row, **options); end

  # This method is a convenience for building Unix-like filters for
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) data. Each row is
  # yielded to the provided block which can alter it as needed. After the block
  # returns, the row is appended to `output` altered or not.
  #
  # The `input` and `output` arguments can be anything
  # [`CSV::new()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new)
  # accepts (generally
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) or
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects). If not given,
  # they default to `ARGF` and `$stdout`.
  #
  # The `options` parameter is also filtered down to
  # [`CSV::new()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new)
  # after some clever key parsing. Any key beginning with `:in_` or `:input_`
  # will have that leading identifier stripped and will only be used in the
  # `options` [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) for the
  # `input` object. Keys starting with `:out_` or `:output_` affect only
  # `output`. All other keys are assigned to both objects.
  #
  # The `:output_row_sep` `option` defaults to `$INPUT_RECORD_SEPARATOR` (`$/`).
  def self.filter(input = _, output = _, **options); end

  # This method will return a
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) instance, just like
  # [`CSV::new()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new),
  # but the instance will be cached and returned for all future calls to this
  # method for the same `data` object (tested by
  # [`Object#object_id()`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-object_id))
  # with the same `options`.
  #
  # If a block is given, the instance is passed to the block and the return
  # value becomes the return value of the block.
  def self.instance(data = _, **options); end

  # This method opens an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object, and wraps that with
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html). This is intended as
  # the primary interface for writing a
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) file.
  #
  # You must pass a `filename` and may optionally add a `mode` for Ruby's
  # open(). You may also pass an optional
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) containing any
  # `options`
  # [`CSV::new()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new)
  # understands as the final argument.
  #
  # This method works like Ruby's open() call, in that it will pass a
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) object to a provided
  # block and close it when the block terminates, or it will return the
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) object when no block
  # is provided. (**Note**: This is different from the Ruby 1.8
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) library which passed
  # rows to the block. Use
  # [`CSV::foreach()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-foreach)
  # for that behavior.)
  #
  # You must provide a `mode` with an embedded
  # [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) designator
  # unless your data is in
  # [`Encoding::default_external()`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html#method-c-default_external).
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) will check the
  # [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) of the
  # underlying [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object (set
  # by the `mode` you pass) to determine how to parse the data. You may provide
  # a second [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) to
  # have the data transcoded as it is read just as you can with a normal call to
  # [`IO::open()`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-open).
  # For example, `"rb:UTF-32BE:UTF-8"` would read UTF-32BE data from the file
  # but transcode it to UTF-8 before
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) parses it.
  #
  # An opened [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) object will
  # delegate to many [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) methods
  # for convenience. You may call:
  #
  # *   binmode()
  # *   binmode?()
  # *   close()
  # *   close\_read()
  # *   close\_write()
  # *   closed?()
  # *   eof()
  # *   eof?()
  # *   external\_encoding()
  # *   fcntl()
  # *   fileno()
  # *   flock()
  # *   flush()
  # *   fsync()
  # *   internal\_encoding()
  # *   ioctl()
  # *   isatty()
  # *   path()
  # *   pid()
  # *   pos()
  # *   pos=()
  # *   reopen()
  # *   seek()
  # *   stat()
  # *   sync()
  # *   sync=()
  # *   tell()
  # *   [`to_i`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-to_i)()
  # *   [`to_io`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-to_io)()
  # *   truncate()
  # *   tty?()
  def self.open(filename, mode = _, **options, &blk); end

  # Alias for
  # [`CSV::read()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-read).
  def self.readlines(*args); end

  # A shortcut for:
  #
  # ```ruby
  # CSV.read( path, { headers:           true,
  #                   converters:        :numeric,
  #                   header_converters: :symbol }.merge(options) )
  # ```
  def self.table(path, **options); end

  def binmode(*args, &block); end

  def binmode?(*args, &block); end

  # The encoded `:col_sep` used in parsing and writing. See
  # [`CSV::new`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new) for
  # details.
  def col_sep; end

  # You can use this method to install a
  # [`CSV::Converters`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#Converters)
  # built-in, or provide a block that handles a custom conversion.
  #
  # If you provide a block that takes one argument, it will be passed the field
  # and is expected to return the converted value or the field itself. If your
  # block takes two arguments, it will also be passed a
  # [`CSV::FieldInfo`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#FieldInfo)
  # [`Struct`](https://docs.ruby-lang.org/en/2.7.0/Struct.html), containing
  # details about the field. Again, the block should return a converted field or
  # the field itself.
  def convert(name = _, &converter); end

  # Returns the current list of converters in effect. See
  # [`CSV::new`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new) for
  # details. Built-in converters will be returned by name, while others will be
  # returned as is.
  def converters; end

  # Yields each row of the data source in turn.
  #
  # Support for
  # [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html).
  #
  # The data source must be open for reading.
  def each(&blk); end

  # The [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html)
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) is parsing or writing
  # in. This will be the
  # [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) you receive
  # parsed data in and/or the
  # [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) data will be
  # written in.
  def encoding; end

  # Alias for:
  # [`eof?`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-eof-3F)
  def eof(*args, &block); end

  # Also aliased as:
  # [`eof`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-eof)
  def eof?(*args, &block); end

  # The limit for field size, if any. See
  # [`CSV::new`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new) for
  # details.
  def field_size_limit; end

  def flock(*args, &block); end

  # Returns `true` if all output fields are quoted. See
  # [`CSV::new`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new) for
  # details.
  def force_quotes?; end

  # Alias for:
  # [`shift`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-shift)
  def gets; end

  # Identical to
  # [`CSV#convert()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-convert),
  # but for header rows.
  #
  # Note that this method must be called before header rows are read to have any
  # effect.
  def header_convert(name = _, &converter); end

  # Returns the current list of converters in effect for headers. See
  # [`CSV::new`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new) for
  # details. Built-in converters will be returned by name, while others will be
  # returned as is.
  def header_converters; end

  # Returns `true` if the next row read will be a header row.
  def header_row?; end

  # Returns `nil` if headers will not be used, `true` if they will but have not
  # yet been read, or the actual headers after they have been read. See
  # [`CSV::new`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new) for
  # details.
  def headers; end

  # Returns a simplified description of the key
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) attributes in an ASCII
  # compatible [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  def inspect; end

  def ioctl(*args, &block); end

  # Returns `true` if illegal input is handled. See
  # [`CSV::new`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new) for
  # details.
  def liberal_parsing?; end

  # The last row read from this file.
  def line; end

  # The line number of the last row read from this file. Fields with nested
  # line-end characters will not affect this count.
  def lineno; end

  def path(*args, &block); end

  # Alias for:
  # [`<<`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-3C-3C)
  def puts(row); end

  # The encoded `:quote_char` used in parsing and writing. See
  # [`CSV::new`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new) for
  # details.
  def quote_char; end

  # Alias for:
  # [`read`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-read)
  def readlines; end

  # Returns `true` if headers will be returned as a row of results. See
  # [`CSV::new`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new) for
  # details.
  def return_headers?; end

  # Rewinds the underlying [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object and resets CSV's lineno() counter.
  def rewind; end

  # The encoded `:row_sep` used in parsing and writing. See
  # [`CSV::new`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new) for
  # details.
  def row_sep; end

  # The primary read method for wrapped Strings and IOs, a single row is pulled
  # from the data source, parsed and returned as an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of fields (if
  # header rows are not used) or a
  # [`CSV::Row`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html) (when header
  # rows are used).
  #
  # The data source must be open for reading.
  #
  # Also aliased as:
  # [`gets`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-gets),
  # [`readline`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-readline)
  def shift; end

  # Returns `true` blank lines are skipped by the parser. See
  # [`CSV::new`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new) for
  # details.
  def skip_blanks?; end

  # The regex marking a line as a comment. See
  # [`CSV::new`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new) for
  # details.
  def skip_lines; end

  def stat(*args, &block); end

  def to_i(*args, &block); end

  def to_io(*args, &block); end

  # Returns `true` if unconverted\_fields() to parsed results. See
  # [`CSV::new`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new) for
  # details.
  def unconverted_fields?; end

  # Returns `true` if headers are written in output. See
  # [`CSV::new`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-new) for
  # details.
  def write_headers?; end
end

# A [`CSV::Row`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html) is part
# [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) and part
# [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html). It retains an order
# for the fields and allows duplicates just as an
# [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) would, but also
# allows you to access fields by name just as you could if they were in a
# [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html).
#
# All rows returned by [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html)
# will be constructed from this class, if header row processing is activated.
class CSV::Row < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out) {{fixed: T.untyped}}

  # Construct a new
  # [`CSV::Row`](https://docs.ruby-lang.org/en/2.6.0/CSV/Row.html) from
  # `headers` and `fields`, which are expected to be Arrays. If one
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) is shorter than
  # the other, it will be padded with `nil` objects.
  #
  # The optional `header_row` parameter can be set to `true` to indicate, via
  # [`CSV::Row.header_row?()`](https://docs.ruby-lang.org/en/2.6.0/CSV/Row.html#method-i-header_row-3F)
  # and
  # [`CSV::Row.field_row?()`](https://docs.ruby-lang.org/en/2.6.0/CSV/Row.html#method-i-field_row-3F),
  # that this is a header row. Otherwise, the row is assumes to be a field row.
  #
  # A [`CSV::Row`](https://docs.ruby-lang.org/en/2.6.0/CSV/Row.html) object
  # supports the following
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) methods through
  # delegation:
  #
  # *   empty?()
  # *   length()
  # *   size()
  sig { params(headers: T::Array[BasicObject], fields: T::Array[BasicObject]).void }
  sig { params(headers: T::Array[BasicObject], fields: T::Array[BasicObject], header_row: T::Boolean).void }
  def initialize(headers, fields, header_row); end

  # If a two-element [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
  # is provided, it is assumed to be a header and field and the pair is
  # appended. A [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) works
  # the same way with the key being the header and the value being the field.
  # Anything else is assumed to be a lone field which is appended with a `nil`
  # header.
  #
  # This method returns the row for chaining.
  def <<(arg); end

  # Returns `true` if this row contains the same headers and fields in the same
  # order as `other`.
  def ==(other); end

  # Alias for:
  # [`field`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-field)
  def [](header_or_index, minimum_index = _); end

  # Looks up the field by the semantics described in
  # [`CSV::Row.field()`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-field)
  # and assigns the `value`.
  #
  # Assigning past the end of the row with an index will set all pairs between
  # to `[nil, nil]`. Assigning to an unused header appends the new pair.
  def []=(*args); end

  # Removes a pair from the row by `header` or `index`. The pair is located as
  # described in
  # [`CSV::Row.field()`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-field).
  # The deleted pair is returned, or `nil` if a pair could not be found.
  def delete(header_or_index, minimum_index = _); end

  # The provided `block` is passed a header and field for each pair in the row
  # and expected to return `true` or `false`, depending on whether the pair
  # should be deleted.
  #
  # This method returns the row for chaining.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) is
  # returned.
  def delete_if(&block); end

  # Yields each pair of the row as header and field tuples (much like iterating
  # over a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)). This method
  # returns the row for chaining.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) is
  # returned.
  #
  # Support for
  # [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html).
  #
  # Also aliased as:
  # [`each_pair`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-each_pair)
  def each(&block); end

  # This method will fetch the field value by `header`. It has the same behavior
  # as
  # [`Hash#fetch`](https://docs.ruby-lang.org/en/2.7.0/Hash.html#method-i-fetch):
  # if there is a field with the given `header`, its value is returned.
  # Otherwise, if a block is given, it is yielded the `header` and its result is
  # returned; if a `default` is given as the second argument, it is returned;
  # otherwise a [`KeyError`](https://docs.ruby-lang.org/en/2.7.0/KeyError.html)
  # is raised.
  def fetch(header, *varargs); end

  # This method will return the field value by `header` or `index`. If a field
  # is not found, `nil` is returned.
  #
  # When provided, `offset` ensures that a header match occurs on or later than
  # the `offset` index. You can use this to find duplicate headers, without
  # resorting to hard-coding exact indices.
  #
  # Also aliased as:
  # [`[]`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-5B-5D)
  def field(header_or_index, minimum_index = _); end

  # Returns `true` if `data` matches a field in this row, and `false` otherwise.
  def field?(data); end

  # Returns `true` if this is a field row.
  def field_row?; end

  # This method accepts any number of arguments which can be headers, indices,
  # Ranges of either, or two-element Arrays containing a header and offset. Each
  # argument will be replaced with a field lookup as described in
  # [`CSV::Row.field()`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-field).
  #
  # If called with no arguments, all fields are returned.
  #
  # Also aliased as:
  # [`values_at`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-values_at)
  def fields(*headers_and_or_indices); end

  # Returns `true` if there is a field with the given `header`.
  #
  # Also aliased as:
  # [`include?`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-include-3F),
  # [`key?`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-key-3F),
  # [`member?`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-member-3F),
  # [`header?`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-header-3F)
  def has_key?(header); end

  # Alias for:
  # [`has_key?`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-has_key-3F)
  def header?(name); end

  # Returns `true` if this is a header row.
  def header_row?; end

  # Returns the headers of this row.
  def headers; end

  # Alias for:
  # [`has_key?`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-has_key-3F)
  def include?(name); end

  # This method will return the index of a field with the provided `header`. The
  # `offset` can be used to locate duplicate header names, as described in
  # [`CSV::Row.field()`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-field).
  def index(header, minimum_index = _); end

  # A summary of fields, by header, in an ASCII compatible
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  def inspect; end

  # Alias for:
  # [`has_key?`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-has_key-3F)
  def key?(header); end

  # Alias for:
  # [`has_key?`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-has_key-3F)
  def member?(header); end

  # A shortcut for appending multiple fields. Equivalent to:
  #
  # ```ruby
  # args.each { |arg| csv_row << arg }
  # ```
  #
  # This method returns the row for chaining.
  def push(*args); end

  # Internal data format used to compare equality.
  def row; end

  # Returns the row as a [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html)
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). Headers are not
  # used. Equivalent to:
  #
  # ```ruby
  # csv_row.fields.to_csv( options )
  # ```
  #
  #
  # Also aliased as:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-to_s)
  def to_csv(**options); end

  # Collapses the row into a simple
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html). Be warned that this
  # discards field order and clobbers duplicate fields.
  #
  # Also aliased as:
  # [`to_hash`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-to_hash)
  def to_h; end

  # Alias for:
  # [`to_h`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-to_h)
  def to_hash; end

  # Alias for:
  # [`to_csv`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-to_csv)
  def to_s(**options); end

  # Alias for:
  # [`fields`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html#method-i-fields)
  def values_at(*headers_and_or_indices); end
end

# A [`FieldInfo`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#FieldInfo)
# [`Struct`](https://docs.ruby-lang.org/en/2.7.0/Struct.html) contains details
# about a field's position in the data source it was read from.
# [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) will pass this
# [`Struct`](https://docs.ruby-lang.org/en/2.7.0/Struct.html) to some blocks
# that make decisions based on field structure. See
# [`CSV.convert_fields()`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-i-convert_fields)
# for an example.
#
# **`index`**
# :   The zero-based index of the field in its row.
# **`line`**
# :   The line of the data source this row is from.
# **`header`**
# :   The header for the column, when available.
class CSV::FieldInfo < Struct
  extend T::Generic
  Elem = type_member(:out) {{fixed: T.untyped}}
end

# The error thrown when the parser encounters illegal
# [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) formatting.
class CSV::MalformedCSVError < RuntimeError
  sig { params(message: String, line_number: Integer).void }
  def initialize(message, line_number); end
end

# A [`CSV::Table`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html) is a
# two-dimensional data structure for representing
# [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html) documents. Tables allow
# you to work with the data by row or column, manipulate the data, and even
# convert the results back to
# [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html), if needed.
#
# All tables returned by [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html)
# will be constructed from this class, if header row processing is activated.
class CSV::Table < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out) {{fixed: T.any(CSV::Row, T::Array[T.untyped])}}

  # Constructs a new
  # [`CSV::Table`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html) from
  # `array_of_rows`, which are expected to be
  # [`CSV::Row`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html) objects. All
  # rows are assumed to have the same headers.
  #
  # The optional `headers` parameter can be set to
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of headers. If
  # headers aren't set, headers are fetched from
  # [`CSV::Row`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html) objects.
  # Otherwise, headers() method will return headers being set in headers
  # argument.
  #
  # A [`CSV::Table`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html) object
  # supports the following
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) methods through
  # delegation:
  #
  # *   empty?()
  # *   length()
  # *   size()
  sig { params(array_of_rows: T::Array[CSV::Row], headers: T::Array[BasicObject]).returns(CSV::Table) }
  def self.new(array_of_rows, headers: nil); end

  # Adds a new row to the bottom end of this table. You can provide an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html), which will be
  # converted to a
  # [`CSV::Row`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html) (inheriting
  # the table's headers()), or a
  # [`CSV::Row`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html).
  #
  # This method returns the table for chaining.
  sig do
    params(
      row_or_array: T.any(CSV::Row, T::Array[T.untyped])
    ).returns(T.self_type)
  end
  def <<(row_or_array); end

  # Returns `true` if all rows of this table ==() `other`'s rows.
  sig { params(other: CSV::Table).returns(T::Boolean) }
  def ==(other); end

  # In the default mixed mode, this method returns rows for index access and
  # columns for header access. You can force the index association by first
  # calling
  # [`by_col`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html#method-i-by_col)!()
  # or
  # [`by_row`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html#method-i-by_row)!().
  #
  # Columns are returned as an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of values.
  # Altering that [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) has
  # no effect on the table.
  sig { params(index_or_header: Integer).returns(T.nilable(Elem)) }
  sig { params(index_or_header: T::Range[Integer]).returns(T::Array[Elem]) }
  sig { params(index_or_header: BasicObject).returns(T::Array[T.untyped]) }
  def [](index_or_header); end

  # In the default mixed mode, this method assigns rows for index access and
  # columns for header access. You can force the index association by first
  # calling
  # [`by_col`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html#method-i-by_col)!()
  # or
  # [`by_row`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html#method-i-by_row)!().
  #
  # Rows may be set to an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of values (which
  # will inherit the table's headers()) or a
  # [`CSV::Row`](https://docs.ruby-lang.org/en/2.7.0/CSV/Row.html).
  #
  # Columns may be set to a single value, which is copied to each row of the
  # column, or an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # values. Arrays of values are assigned to rows top to bottom in row major
  # order. Excess values are ignored and if the
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) does not have a
  # value for each row the extra rows will receive a `nil`.
  #
  # Assigning to an existing column or row clobbers the data. Assigning to new
  # columns creates them at the right end of the table.
  sig{ type_parameters(:V).params(index_or_header: BasicObject, value: T.type_parameter(:V)).returns(T.type_parameter(:V)) }
  def []=(index_or_header, value); end

  # Returns a duplicate table object, in column mode. This is handy for chaining
  # in a single call without changing the table mode, but be aware that this
  # method can consume a fair amount of memory for bigger data sets.
  #
  # This method returns the duplicate table for chaining. Don't chain
  # destructive methods (like []=()) this way though, since you are working with
  # a duplicate.
  sig { returns(CSV::Table) }
  def by_col; end

  # Switches the mode of this table to column mode. All calls to indexing and
  # iteration methods will work with columns until the mode is changed again.
  #
  # This method returns the table and is safe to chain.
  sig { returns(CSV::Table) }
  def by_col!; end

  # Returns a duplicate table object, in mixed mode. This is handy for chaining
  # in a single call without changing the table mode, but be aware that this
  # method can consume a fair amount of memory for bigger data sets.
  #
  # This method returns the duplicate table for chaining. Don't chain
  # destructive methods (like []=()) this way though, since you are working with
  # a duplicate.
  sig { returns(CSV::Table) }
  def by_col_or_row; end

  # Switches the mode of this table to mixed mode. All calls to indexing and
  # iteration methods will use the default intelligent indexing system until the
  # mode is changed again. In mixed mode an index is assumed to be a row
  # reference while anything else is assumed to be column access by headers.
  #
  # This method returns the table and is safe to chain.
  sig { returns(CSV::Table) }
  def by_col_or_row!; end

  # Returns a duplicate table object, in row mode. This is handy for chaining in
  # a single call without changing the table mode, but be aware that this method
  # can consume a fair amount of memory for bigger data sets.
  #
  # This method returns the duplicate table for chaining. Don't chain
  # destructive methods (like []=()) this way though, since you are working with
  # a duplicate.
  sig { returns(CSV::Table) }
  def by_row; end

  # Switches the mode of this table to row mode. All calls to indexing and
  # iteration methods will work with rows until the mode is changed again.
  #
  # This method returns the table and is safe to chain.
  sig { returns(CSV::Table) }
  def by_row!; end

  # Removes and returns the indicated columns or rows. In the default mixed mode
  # indices refer to rows and everything else is assumed to be a column headers.
  # Use
  # [`by_col`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html#method-i-by_col)!()
  # or
  # [`by_row`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html#method-i-by_row)!()
  # to force the lookup.
  sig do
    params(
      indexes_or_headers: BasicObject
    ).returns(T.any(T.nilable(Elem), T::Array[T.nilable(Elem)]))
  end
  def delete(*indexes_or_headers); end

  # Removes any column or row for which the block returns `true`. In the default
  # mixed mode or row mode, iteration is the standard row major walking of rows.
  # In column mode, iteration will `yield` two element tuples containing the
  # column name and an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
  # of values for that column.
  #
  # This method returns the table for chaining.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) is
  # returned.
  sig { params(blk: T.proc.params(arg0: T.untyped).returns(T.untyped)).returns(T.self_type) }
  sig { returns(T::Enumerator[T.untyped]) }
  def delete_if(&blk); end

  # Extracts the nested value specified by the sequence of `index` or `header`
  # objects by calling dig at each step, returning nil if any intermediate step
  # is nil.
  sig { params(index_or_header: BasicObject).returns(Elem) }
  sig { params(index_or_header: BasicObject, index_or_headers: BasicObject).returns(T.untyped) }
  def dig(index_or_header, *index_or_headers); end

  # In the default mixed mode or row mode, iteration is the standard row major
  # walking of rows. In column mode, iteration will `yield` two element tuples
  # containing the column name and an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of values for that
  # column.
  #
  # This method returns the table for chaining.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) is
  # returned.
  sig { params(block: T.proc.params(arg0: T.untyped).void).returns(T.self_type) }
  sig { returns(T::Enumerator[T.untyped]) }
  def each(&block); end

  sig { returns(T::Boolean) }
  def empty?; end

  # Returns the headers for the first row of this table (assumed to match all
  # other rows). The headers
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) passed to
  # [`CSV::Table.new`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html#method-c-new)
  # is returned for empty tables.
  sig { returns(T::Array[T.untyped]) }
  def headers; end

  # Shows the mode and size of this table in a US-ASCII
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  sig { returns(String) }
  def inspect; end

  sig { returns(Integer) }
  def length; end

  # The current access mode for indexing and iteration.
  sig { returns(Symbol) }
  def mode; end

  # A shortcut for appending multiple rows. Equivalent to:
  #
  # ```ruby
  # rows.each { |row| self << row }
  # ```
  #
  # This method returns the table for chaining.
  sig do
    params(
      rows: T.any(CSV::Row, T::Array[T.untyped])
    ).returns(T.self_type)
  end
  def push(*rows); end

  sig { returns(Integer) }
  def size; end

  # Returns the table as an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of Arrays. Headers
  # will be the first row, then all of the field rows will follow.
  sig { returns(T::Array[T::Array[T.untyped]]) }
  def to_a; end

  # Returns the table as a complete
  # [`CSV`](https://docs.ruby-lang.org/en/2.7.0/CSV.html)
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). Headers will be
  # listed first, then all of the field rows.
  #
  # This method assumes you want the
  # [`Table.headers()`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html#method-i-headers),
  # unless you explicitly pass `:write_headers => false`.
  #
  # Also aliased as:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html#method-i-to_s)
  sig { params(write_headers: T::Boolean, options: T.untyped).returns(String) }
  def to_csv(write_headers: true, **options); end

  # Alias for:
  # [`to_csv`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html#method-i-to_csv)
  sig { params(write_headers: T::Boolean, options: T.untyped).returns(String) }
  def to_s(write_headers: true, **options); end

  # The mixed mode default is to treat a list of indices as row access,
  # returning the rows indicated. Anything else is considered columnar access.
  # For columnar access, the return set has an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) for each row with
  # the values indicated by the headers in each
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html). You can force
  # column or row mode using
  # [`by_col`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html#method-i-by_col)!()
  # or
  # [`by_row`](https://docs.ruby-lang.org/en/2.7.0/CSV/Table.html#method-i-by_row)!().
  #
  # You cannot mix column and row access.
  sig do
    params(
      indices_or_headers: BasicObject
    ).returns(T.any(T.nilable(Elem), T::Array[T.nilable(Elem)]))
  end
  def values_at(*indices_or_headers); end
end

# Passes `args` to
# [`CSV::instance`](https://docs.ruby-lang.org/en/2.7.0/CSV.html#method-c-instance).
#
# ```ruby
# CSV("CSV,data").read
#   #=> [["CSV", "data"]]
# ```
#
# If a block is given, the instance is passed the block and the return value
# becomes the return value of the block.
#
# ```ruby
# CSV("CSV,data") { |c|
#   c.read.any? { |a| a.include?("data") }
# } #=> true
#
# CSV("CSV,data") { |c|
#   c.read.any? { |a| a.include?("zombies") }
# } #=> false
# ```
sig do
  params(
      io: T.any(::Sorbet::Private::Static::IOLike, String),
      options: T::Hash[Symbol, T.untyped],
      blk: T.untyped,
  )
  .returns(CSV)
end
def CSV(io=T.unsafe(nil), options=T.unsafe(nil), &blk); end
