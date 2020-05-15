# typed: __STDLIB_INTERNAL

# scanf for Ruby
#
# ## Description
#
# scanf is an implementation of the C function scanf(3), modified as necessary
# for Ruby compatibility.
#
# The methods provided are
# [`String#scanf`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-scanf),
# [`IO#scanf`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-scanf), and
# [`Kernel#scanf`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-scanf).
# [`Kernel#scanf`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-scanf)
# is a wrapper around STDIN.scanf.
# [`IO#scanf`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-scanf) can
# be used on any [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) stream,
# including file handles and sockets. scanf can be called either with or without
# a block.
#
# [`Scanf`](https://docs.ruby-lang.org/en/2.6.0/Scanf.html) scans an input
# string or stream according to a **format**, as described below in Conversions,
# and returns an array of matches between the format and the input. The format
# is defined in a string, and is similar (though not identical) to the formats
# used in
# [`Kernel#printf`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-printf)
# and
# [`Kernel#sprintf`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-sprintf).
#
# The format may contain **conversion specifiers**, which tell scanf what form
# (type) each particular matched substring should be converted to (e.g., decimal
# integer, floating point number, literal string, etc.)  The matches and
# conversions take place from left to right, and the conversions themselves are
# returned as an array.
#
# The format string may also contain characters other than those in the
# conversion specifiers. Whitespace (blanks, tabs, or newlines) in the format
# string matches any amount of whitespace, including none, in the input.
# Everything else matches only itself.
#
# Scanning stops, and scanf returns, when any input character fails to match the
# specifications in the format string, or when input is exhausted, or when
# everything in the format string has been matched. All matches found up to the
# stopping point are returned in the return array (or yielded to the block, if a
# block was given).
#
# ## Basic usage
#
# ```ruby
# require 'scanf'
#
# # String#scanf and IO#scanf take a single argument, the format string
# array = a_string.scanf("%d%s")
# array = an_io.scanf("%d%s")
#
# # Kernel#scanf reads from STDIN
# array = scanf("%d%s")
# ```
#
# ## Block usage
#
# When called with a block, scanf keeps scanning the input, cycling back to the
# beginning of the format string, and yields a new array of conversions to the
# block every time the format string is matched (including partial matches, but
# not including complete failures). The actual return value of scanf when called
# with a block is an array containing the results of all the executions of the
# block.
#
# ```ruby
# str = "123 abc 456 def 789 ghi"
# str.scanf("%d%s") { |num,str| [ num * 2, str.upcase ] }
# # => [[246, "ABC"], [912, "DEF"], [1578, "GHI"]]
# ```
#
# ## Conversions
#
# The single argument to scanf is a format string, which generally includes one
# or more conversion specifiers. Conversion specifiers begin with the percent
# character ('%') and include information about what scanf should next scan for
# (string, decimal number, single character, etc.).
#
# There may be an optional maximum field width, expressed as a decimal integer,
# between the % and the conversion. If no width is given, a default of
# 'infinity' is used (with the exception of the %c specifier; see below).
# Otherwise, given a field width of *n* for a given conversion, at most *n*
# characters are scanned in processing that conversion. Before conversion
# begins, most conversions skip whitespace in the input string; this whitespace
# is not counted against the field width.
#
# The following conversions are available.
#
# %
# :   Matches a literal `%'. That is, `%%' in the format string matches a single
#     input `%' character. No conversion is done, and the resulting '%' is not
#     included in the return array.
#
# d
# :   Matches an optionally signed decimal integer.
#
# u
# :   Same as d.
#
# i
# :   Matches an optionally signed integer. The integer is read in base 16 if it
#     begins with '0x' or '0X', in base 8 if it begins with '0', and in base 10
#     other- wise. Only characters that correspond to the base are recognized.
#
# o
# :   Matches an optionally signed octal integer.
#
# x, X
# :   Matches an optionally signed hexadecimal integer,
#
# a, e, f, g, A, E, F, G
# :   Matches an optionally signed floating-point number.
#
# s
# :   Matches a sequence of non-white-space character. The input string stops at
#     whitespace or at the maximum field width, whichever occurs first.
#
# c
# :   Matches a single character, or a sequence of *n* characters if a field
#     width of *n* is specified. The usual skip of leading white space is
#     suppressed. To skip whitespace first, use an explicit space in the format.
#
# [
# :   Matches a nonempty sequence of characters from the specified set of
#     accepted characters. The usual skip of leading whitespace is suppressed.
#     This bracketed sub-expression is interpreted exactly like a character
#     class in a Ruby regular expression. (In fact, it is placed as-is in a
#     regular expression.)  The matching against the input string ends with the
#     appearance of a character not in (or, with a circumflex, in) the set, or
#     when the field width runs out, whichever comes first.
#
#
# ### Assignment suppression
#
# To require that a particular match occur, but without including the result in
# the return array, place the **assignment suppression flag**, which is the star
# character ('\*'), immediately after the leading '%' of a format specifier
# (just before the field width, if any).
#
# ## scanf for Ruby compared with scanf in C
#
# scanf for Ruby is based on the C function scanf(3), but with modifications,
# dictated mainly by the underlying differences between the languages.
#
# ### Unimplemented flags and specifiers
#
# *   The only flag implemented in scanf for Ruby is '`*`' (ignore upcoming
#     conversion). Many of the flags available in C versions of scanf(3) have to
#     do with the type of upcoming pointer arguments, and are meaningless in
#     Ruby.
#
# *   The `n` specifier (store number of characters consumed so far in next
#     pointer) is not implemented.
#
# *   The `p` specifier (match a pointer value) is not implemented.
#
#
# ### Altered specifiers
#
# o, u, x, X
# :   In scanf for Ruby, all of these specifiers scan for an optionally signed
#     integer, rather than for an unsigned integer like their C counterparts.
#
#
# ### Return values
#
# scanf for Ruby returns an array of successful conversions, whereas scanf(3)
# returns the number of conversions successfully completed. (See below for more
# details on scanf for Ruby's return values.)
#
# ## Return values
#
# Without a block, scanf returns an array containing all the conversions it has
# found. If none are found, scanf will return an empty array. An unsuccessful
# match is never ignored, but rather always signals the end of the scanning
# operation. If the first unsuccessful match takes place after one or more
# successful matches have already taken place, the returned array will contain
# the results of those successful matches.
#
# With a block scanf returns a 'map'-like array of transformations from the
# block -- that is, an array reflecting what the block did with each yielded
# result from the iterative scanf operation. (See "Block usage", above.)
#
# ## Current limitations and bugs
#
# When using
# [`IO#scanf`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-scanf) under
# Windows, make sure you open your files in binary mode:
#
# ```ruby
# File.open("filename", "rb")
# ```
#
# so that scanf can keep track of characters correctly.
#
# Support for character classes is reasonably complete (since it essentially
# piggy-backs on Ruby's regular expression handling of character classes), but
# users are advised that character class testing has not been exhaustive, and
# that they should exercise some caution in using any of the more complex and/or
# arcane character class idioms.
#
# ## License and copyright
#
# Copyright
# :   (c) 2002-2003 David Alan Black
# License
# :   Distributed on the same licensing terms as Ruby itself
#
#
# ## Warranty disclaimer
#
# This software is provided "as is" and without any express or implied
# warranties, including, without limitation, the implied warranties of
# merchantability and fitness for a particular purpose.
#
# ## Credits and acknowledgements
#
# scanf was developed as the major activity of the Austin Ruby Codefest (Austin,
# Texas, August 2002).
#
# Principal author
# :   David Alan Black (mailto:dblack@superlink.net)
# Co-author
# :   Hal Fulton (mailto:hal9000@hypermetrics.com)
# Project contributors
# :   Nolan Darilek, Jason Johnston
#
#
# Thanks to Hal Fulton for hosting the Codefest.
#
# Thanks to Matz for suggestions about the class design.
#
# Thanks to Gavin Sinclair for some feedback on the documentation.
#
# The text for parts of this document, especially the Description and
# Conversions sections, above, were adapted from the Linux Programmer's Manual
# manpage for scanf(3), dated 1995-11-01.
#
# ## Bugs and bug reports
#
# scanf for Ruby is based on something of an amalgam of C scanf implementations
# and documentation, rather than on a single canonical description. Suggestions
# for features and behaviors which appear in other scanfs, and would be
# meaningful in Ruby, are welcome, as are reports of suspicious behaviors and/or
# bugs. (Please see "Credits and acknowledgements", above, for email addresses.)
module Scanf
end

class Scanf::FormatSpecifier
end

class Scanf::FormatString
end
