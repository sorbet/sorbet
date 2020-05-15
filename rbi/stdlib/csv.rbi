# typed: __STDLIB_INTERNAL

# This class provides a complete interface to
# [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) files and data. It
# offers tools to enable you to read and write to and from Strings or
# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) objects, as needed.
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
# If a [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) is passed
# into [`::new`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-c-new), it
# is internally wrapped into a
# [`StringIO`](https://docs.ruby-lang.org/en/2.6.0/StringIO.html) object.
#
# `options` can be used for specifying the particular
# [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) flavor (column
# separators, row separators, value quoting and so on), and for data conversion,
# see [`Data`](https://docs.ruby-lang.org/en/2.6.0/Data.html) Conversion section
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
# ## [`Data`](https://docs.ruby-lang.org/en/2.6.0/Data.html) Conversion
#
# ### [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) with headers
#
# [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) allows to specify column
# names of [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) file, whether
# they are in data, or provided separately. If headers specified, reading
# methods return an instance of
# [`CSV::Table`](https://docs.ruby-lang.org/en/2.6.0/CSV/Table.html), consisting
# of [`CSV::Row`](https://docs.ruby-lang.org/en/2.6.0/CSV/Row.html).
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
# data = CSV.parse('Bob,Engeneering,1000', headers: %i[name department salary])
# data.first      #=> #<CSV::Row name:"Bob" department:"Engineering" salary:"1000">
# ```
#
# ### Typed data reading
#
# [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) allows to provide a set
# of data *converters* e.g. transformations to try on input data. Converter
# could be a symbol from
# [`CSV::Converters`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#Converters)
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
# ## [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) and Character Encodings (M17n or Multilingualization)
#
# This new [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) parser is m17n
# savvy. The parser works in the
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) of the
# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) or
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) object being read
# from or written to. Your data is never transcoded (unless you ask Ruby to
# transcode it for you) and will literally be parsed in the
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) it is in. Thus
# [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) will return Arrays or
# Rows of Strings in the
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) of your data.
# This is accomplished by transcoding the parser itself into your
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html).
#
# Some transcoding must take place, of course, to accomplish this multiencoding
# support. For example, `:col_sep`, `:row_sep`, and `:quote_char` must be
# transcoded to match your data. Hopefully this makes the entire process feel
# transparent, since CSV's defaults should just magically work for your data.
# However, you can set these values manually in the target
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) to avoid the
# translation.
#
# It's also important to note that while all of CSV's core parser is now
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) agnostic, some
# features are not. For example, the built-in converters will try to transcode
# data to UTF-8 before making conversions. Again, you can provide custom
# converters that are aware of your Encodings to avoid this translation. It's
# just too hard for me to support native conversions in all of Ruby's Encodings.
#
# Anyway, the practical side of this is simple:  make sure
# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) and
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) objects passed
# into [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) have the proper
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) set and
# everything should just work.
# [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) methods that allow you
# to open [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) objects
# (CSV::foreach(),
# [`CSV::open()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-c-open),
# [`CSV::read()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-c-read),
# and
# [`CSV::readlines()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-c-readlines))
# do allow you to specify the
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html).
#
# One minor exception comes when generating
# [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) into a
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) with an
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) that is not
# ASCII compatible. There's no existing data for
# [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) to use to prepare itself
# and thus you will probably need to manually specify the desired
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) for most of
# those cases. It will try to guess using the fields in a row of output though,
# when using
# [`CSV::generate_line()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-c-generate_line)
# or Array#to\_csv().
#
# I try to point out any other
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) issues in the
# documentation of methods as they come up.
#
# This has been tested to the best of my ability with all non-"dummy" Encodings
# Ruby ships with. However, it is brave new code and may have some bugs. Please
# feel free to [report](mailto:james@grayproductions.net) any issues you find
# with it.
class CSV < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out, fixed: T::Array[T.nilable(String)])

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
  # [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) files. You pass a
  # `path` and any `options` you wish to set for the read. Each row of file will
  # be passed to the provided `block` in turn.
  #
  # The `options` parameter can be anything
  # [`CSV::new()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-c-new)
  # understands. This method also understands an additional `:encoding`
  # parameter that you can use to specify the
  # [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) of the data
  # in the file to be read. You must provide this unless your data is in
  # [`Encoding::default_external()`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-default_external).
  # [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) will use this to
  # determine how to parse the data. You may provide a second
  # [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) to have the
  # data transcoded as it is read. For example, `encoding: "UTF-32BE:UTF-8"`
  # would read UTF-32BE data from the file but transcode it to UTF-8 before
  # [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) parses it.
  sig do
    type_parameters(:U).params(
        path: T.any(String, ::Sorbet::Private::Static::IOLike),
        options: T::Hash[Symbol, T.type_parameter(:U)],
        blk: T.proc.params(arg0: T::Array[T.nilable(String)]).void,
    )
    .void
  end
  def self.foreach(path, options=T.unsafe(nil), &blk); end

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
        options: T::Hash[Symbol, T.untyped],
    )
    .void
  end
  def initialize(io=T.unsafe(nil), options=T.unsafe(nil)); end

  # This method can be used to easily parse
  # [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) out of a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html). You may either
  # provide a `block` which will be called with each row of the
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) in turn, or just
  # use the returned [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html)
  # of Arrays (when no `block` is given).
  #
  # You pass your `str` to read from, and an optional `options` containing
  # anything
  # [`CSV::new()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-c-new)
  # understands.
  sig do
    params(
        str: String,
        options: T::Hash[Symbol, T.untyped],
        blk: T.nilable(T.proc.params(arg0: T::Array[T.nilable(String)]).void)
    )
    .returns(T.nilable(T::Array[T::Array[T.nilable(String)]]))
  end
  def self.parse(str, options=T.unsafe(nil), &blk); end

  # This method is a shortcut for converting a single line of a
  # [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html)
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) into an
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html). Note that if
  # `line` contains multiple rows, anything beyond the first row is ignored.
  #
  # The `options` parameter can be anything
  # [`CSV::new()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-c-new)
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
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of Arrays.
  #
  # The data source must be open for reading.
  #
  # Also aliased as:
  # [`readlines`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-i-readlines)
  sig {returns(T::Array[T::Array[T.nilable(String)]])}
  def read; end

  # Alias for:
  # [`shift`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-i-shift)
  sig {returns(T.nilable(T::Array[T.nilable(String)]))}
  def readline; end

  # Use to slurp a [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) file
  # into an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of Arrays.
  # Pass the `path` to the file and any `options`
  # [`CSV::new()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-c-new)
  # understands. This method also understands an additional `:encoding`
  # parameter that you can use to specify the
  # [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) of the data
  # in the file to be read. You must provide this unless your data is in
  # [`Encoding::default_external()`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-default_external).
  # [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) will use this to
  # determine how to parse the data. You may provide a second
  # [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) to have the
  # data transcoded as it is read. For example, `encoding: "UTF-32BE:UTF-8"`
  # would read UTF-32BE data from the file but transcode it to UTF-8 before
  # [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) parses it.
  sig do
    params(
        path: String,
        options: T::Hash[Symbol, T.untyped],
    )
    .returns(T::Array[T::Array[T.nilable(String)]])
  end
  def self.read(path, options=T.unsafe(nil)); end

  # The primary write method for wrapped Strings and IOs, `row` (an
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) or
  # [`CSV::Row`](https://docs.ruby-lang.org/en/2.6.0/CSV/Row.html)) is converted
  # to [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) and appended to the
  # data source. When a
  # [`CSV::Row`](https://docs.ruby-lang.org/en/2.6.0/CSV/Row.html) is passed,
  # only the row's fields() are appended to the output.
  #
  # The data source must be open for writing.
  #
  # Also aliased as:
  # [`add_row`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-i-add_row),
  # [`puts`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-i-puts)
  sig { params(row: T.any(T::Array[T.untyped], CSV::Row)).void }
  def <<(row); end

  # This method wraps a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) you provide, or
  # an empty default
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html), in a
  # [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) object which is passed
  # to the provided block. You can use the block to append
  # [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) rows to the
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) and when the
  # block exits, the final
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) will be
  # returned.
  #
  # Note that a passed
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) **is** modified
  # by this method. Call dup() before passing if you need a new
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html).
  #
  # The `options` parameter can be anything
  # [`CSV::new()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-c-new)
  # understands. This method understands an additional `:encoding` parameter
  # when not passed a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) to set the base
  # [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) for the
  # output. [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) needs this
  # hint if you plan to output non-ASCII compatible data.
  sig { params(str: String, options: T.untyped, blk: T.proc.params(csv: CSV).void).returns(String) }
  def self.generate(str = "", **options, &blk); end

  # This method is a shortcut for converting a single row ([`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html))
  # into a [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html).
  #
  # The `options` parameter can be anything [`CSV::new()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-c-new) understands.
  # This method understands an additional `:encoding` parameter to set the base
  # Encoding for the output.  This method will try to guess your Encoding from
  # the first non-`nil` field in `row`, if possible, but you may need to use
  # this parameter as a backup plan.
  #
  # The `:row_sep` `option` defaults to `$INPUT_RECORD_SEPARATOR`
  # (`$/`) when calling this method.
  sig { params(row: T::Array[T.nilable(String)], options: T.untyped).returns(String) }
  def self.generate_line(row, **options); end
end

# A [`CSV::Row`](https://docs.ruby-lang.org/en/2.6.0/CSV/Row.html) is part
# [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) and part
# [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html). It retains an order
# for the fields and allows duplicates just as an
# [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) would, but also
# allows you to access fields by name just as you could if they were in a
# [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html).
#
# All rows returned by [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html)
# will be constructed from this class, if header row processing is activated.
class CSV::Row < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out, fixed: T.nilable(String))
end

# A [`FieldInfo`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#FieldInfo)
# [`Struct`](https://docs.ruby-lang.org/en/2.6.0/Struct.html) contains details
# about a field's position in the data source it was read from.
# [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) will pass this
# [`Struct`](https://docs.ruby-lang.org/en/2.6.0/Struct.html) to some blocks
# that make decisions based on field structure. See
# [`CSV.convert_fields()`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-i-convert_fields)
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
  Elem = type_member(:out, fixed: T.untyped)
end

# The error thrown when the parser encounters illegal
# [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) formatting.
class CSV::MalformedCSVError < RuntimeError
end

# A [`CSV::Table`](https://docs.ruby-lang.org/en/2.6.0/CSV/Table.html) is a
# two-dimensional data structure for representing
# [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html) documents. Tables allow
# you to work with the data by row or column, manipulate the data, and even
# convert the results back to
# [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html), if needed.
#
# All tables returned by [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html)
# will be constructed from this class, if header row processing is activated.
class CSV::Table < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)
end

# Passes `args` to
# [`CSV::instance`](https://docs.ruby-lang.org/en/2.6.0/CSV.html#method-c-instance).
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
  )
  .returns(CSV)
end
def CSV(io=T.unsafe(nil), options=T.unsafe(nil)); end
