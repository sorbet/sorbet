# typed: __STDLIB_INTERNAL

# :element, parent, name, attributes, children\*
# :   a = Node.new a << "B"            # => <a>B</a> a.b                 # =>
#     <a>B<b/></a> [a.b](1)                      # => <a>B<b/><b/><a>
#     [a.b](1)["x"] = "y"   # => <a>B<b/><b x="y"/></a> [a.b](0).c            #
#     => <a>B**<c/>**<b x="y"/></a> a.b.c << "D"                # =>
#     <a>B**<c>D</c>**<b x="y"/></a>
#
# [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) is an
# [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) toolkit for
# [Ruby](http://www.ruby-lang.org), in Ruby.
#
# [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) is a *pure* Ruby,
# [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) 1.0 conforming,
# [non-validating](http://www.w3.org/TR/2004/REC-xml-20040204/#sec-conformance)
# toolkit with an intuitive API.
# [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) passes 100% of the
# non-validating Oasis
# [tests](http://www.oasis-open.org/committees/xml-conformance/xml-test-suite.shtml),
# and provides tree, stream, SAX2, pull, and lightweight APIs.
# [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) also includes a full
# [XPath](http://www.w3c.org/tr/xpath) 1.0 implementation. Since Ruby 1.8,
# [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) is included in the
# standard Ruby distribution.
#
# Main page
# :   http://www.germane-software.com/software/rexml
# Author
# :   Sean Russell <serATgermaneHYPHENsoftwareDOTcom>
# [`Date`](https://docs.ruby-lang.org/en/2.7.0/Date.html)
# :   2008/019
# [`Version`](https://docs.ruby-lang.org/en/2.7.0/REXML.html#Version)
# :   3.1.7.3
#
#
# This API documentation can be downloaded from the
# [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) home page, or can be
# accessed [online](http://www.germane-software.com/software/rexml_doc)
#
# A tutorial is available in the
# [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) distribution in
# docs/tutorial.html, or can be accessed
# [online](http://www.germane-software.com/software/rexml/docs/tutorial.html)
module REXML
  COPYRIGHT = T.let(T.unsafe(nil), String)

  Copyright = T.let(T.unsafe(nil), String)

  DATE = T.let(T.unsafe(nil), String)

  REVISION = T.let(T.unsafe(nil), String)

  VERSION = T.let(T.unsafe(nil), String)
end

# This class needs:
# *   Documentation
# *   Work!  Not all types of attlists are intelligently parsed, so we just
#
# spew back out what we get in. This works, but it would be better if we
# formatted the output ourselves.
#
# AttlistDecls provide **just** enough support to allow namespace declarations.
# If you need some sort of generalized support, or have an interesting idea
# about how to map the hideous, terrible design of DTD AttlistDecls onto an
# intuitive Ruby interface, let me know. I'm desperate for anything to make DTDs
# more palateable.
class REXML::AttlistDecl < ::REXML::Child
  include(::Enumerable)

  Elem = type_member {{fixed: REXML::Attribute}}

  # Create an
  # [`AttlistDecl`](https://docs.ruby-lang.org/en/2.7.0/REXML/AttlistDecl.html),
  # pulling the information from a Source. Notice that this isn't very
  # convenient; to create an
  # [`AttlistDecl`](https://docs.ruby-lang.org/en/2.7.0/REXML/AttlistDecl.html),
  # you basically have to format it yourself, and then have the initializer
  # parse it. Sorry, but for the foreseeable future, DTD support in
  # [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) is pretty weak on
  # convenience. Have I mentioned how much I hate DTDs?
  def self.new(source); end

  # Access the attlist attribute/value pairs.
  #
  # ```ruby
  # value = attlist_decl[ attribute_name ]
  # ```
  def [](key); end

  # Iterate over the key/value pairs:
  #
  # ```
  # attlist_decl.each { |attribute_name, attribute_value| ... }
  # ```
  def each(&block); end

  # What is this?  Got me.
  def element_name; end

  # Whether an attlist declaration includes the given attribute definition
  #
  # ```
  # if attlist_decl.include? "xmlns:foobar"
  # ```
  def include?(key); end

  def node_type; end

  # Write out exactly what we got in.
  def write(out, indent = _); end
end

# Defines an Element
# [`Attribute`](https://docs.ruby-lang.org/en/2.7.0/REXML/Attribute.html); IE, a
# attribute=value pair, as in: <element attribute="value"/>. Attributes can be
# in their own namespaces. General users of
# [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) will not interact
# with the
# [`Attribute`](https://docs.ruby-lang.org/en/2.7.0/REXML/Attribute.html) class
# much.
class REXML::Attribute
  include(::REXML::Namespace)
  include(::REXML::XMLTokens)
  include(::REXML::Node)

  NEEDS_A_SECOND_CHECK = T.let(T.unsafe(nil), Regexp)

  PATTERN = T.let(T.unsafe(nil), Regexp)

  # Constructor. FIXME: The parser doesn't catch illegal characters in
  # attributes
  #
  # first
  # :   Either: an
  #     [`Attribute`](https://docs.ruby-lang.org/en/2.7.0/REXML/Attribute.html),
  #     which this new attribute will become a clone of; or a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), which is
  #     the name of this attribute
  # second
  # :   If `first` is an
  #     [`Attribute`](https://docs.ruby-lang.org/en/2.7.0/REXML/Attribute.html),
  #     then this may be an Element, or nil. If nil, then the Element parent of
  #     this attribute is the parent of the `first`
  #     [`Attribute`](https://docs.ruby-lang.org/en/2.7.0/REXML/Attribute.html).
  #     If the first argument is a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), then this
  #     must also be a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), and is the
  #     content of the attribute. If this is the content, it must be fully
  #     normalized (contain no illegal characters).
  # parent
  # :   Ignored unless `first` is a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html); otherwise,
  #     may be the Element parent of this attribute, or nil.
  #
  #
  # ```ruby
  # Attribute.new( attribute_to_clone )
  # Attribute.new( attribute_to_clone, parent_element )
  # Attribute.new( "attr", "attr_value" )
  # Attribute.new( "attr", "attr_value", parent_element )
  # ```
  def self.new(first, second = _, parent = _); end

  # Returns true if other is an
  # [`Attribute`](https://docs.ruby-lang.org/en/2.7.0/REXML/Attribute.html) and
  # has the same name and value, false otherwise.
  def ==(other); end

  # Returns a copy of this attribute
  def clone; end

  def doctype; end

  # The element to which this attribute belongs
  def element; end

  # Sets the element of which this object is an attribute. Normally, this is not
  # directly called.
  #
  # Returns this attribute
  def element=(element); end

  # Creates (and returns) a hash from both the name and value
  def hash; end

  def inspect; end

  # Returns the namespace URL, if defined, or nil otherwise
  #
  # ```ruby
  # e = Element.new("el")
  # e.add_namespace("ns", "http://url")
  # e.add_attribute("ns:a", "b")
  # e.add_attribute("nsx:a", "c")
  # e.attribute("ns:a").namespace # => "http://url"
  # e.attribute("nsx:a").namespace # => nil
  # ```
  #
  # This method always returns "" for no namespace attribute. Because the
  # default namespace doesn't apply to attribute names.
  #
  # From https://www.w3.org/TR/xml-names/#uniqAttrs
  #
  # > the default namespace does not apply to attribute names
  #
  # ```ruby
  # e = REXML::Element.new("el")
  # e.add_namespace("", "http://example.com/")
  # e.namespace # => "http://example.com/"
  # e.add_attribute("a", "b")
  # e.attribute("a").namespace # => ""
  # ```
  def namespace(arg = _); end

  def node_type; end

  # The normalized value of this attribute. That is, the attribute with entities
  # intact.
  def normalized=(_); end

  # Returns the namespace of the attribute.
  #
  # ```ruby
  # e = Element.new( "elns:myelement" )
  # e.add_attribute( "nsa:a", "aval" )
  # e.add_attribute( "b", "bval" )
  # e.attributes.get_attribute( "a" ).prefix   # -> "nsa"
  # e.attributes.get_attribute( "b" ).prefix   # -> ""
  # a = Attribute.new( "x", "y" )
  # a.prefix                                   # -> ""
  # ```
  def prefix; end

  # Removes this
  # [`Attribute`](https://docs.ruby-lang.org/en/2.7.0/REXML/Attribute.html) from
  # the tree, and returns true if successful
  #
  # This method is usually not called directly.
  def remove; end

  # Returns the attribute value, with entities replaced
  def to_s; end

  # Returns this attribute out as
  # [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) source, expanding the
  # name
  #
  # ```ruby
  # a = Attribute.new( "x", "y" )
  # a.to_string     # -> "x='y'"
  # b = Attribute.new( "ns:x", "y" )
  # b.to_string     # -> "ns:x='y'"
  # ```
  def to_string; end

  # Returns the UNNORMALIZED value of this attribute. That is, entities have
  # been expanded to their values
  def value; end

  # Writes this attribute (EG, puts 'key="value"' to the output)
  def write(output, indent = _); end

  def xpath; end
end

# A class that defines the set of
# [`Attributes`](https://docs.ruby-lang.org/en/2.7.0/REXML/Attributes.html) of
# an Element and provides operations for accessing elements in that set.
class REXML::Attributes < ::Hash
  K = type_member {{fixed: String}}
  V = type_member {{fixed: String}}
  Elem = type_member {{fixed: REXML::Attribute}}

  # Constructor
  # element
  # :   the Element of which this is an Attribute
  def self.new(element); end

  # Alias for:
  # [`add`](https://docs.ruby-lang.org/en/2.7.0/REXML/Attributes.html#method-i-add)
  def <<(attribute); end

  # Fetches an attribute value. If you want to get the Attribute itself, use
  # [`get_attribute`](https://docs.ruby-lang.org/en/2.7.0/REXML/Attributes.html#method-i-get_attribute)()
  # name
  # :   an XPath attribute name. Namespaces are relevant here.
  # Returns
  # :   the [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) value of
  #     the matching attribute, or `nil` if no matching attribute was found.
  #     This is the unnormalized value (with entities expanded).
  #
  #
  # ```ruby
  # doc = Document.new "<a foo:att='1' bar:att='2' att='&lt;'/>"
  # doc.root.attributes['att']         #-> '<'
  # doc.root.attributes['bar:att']     #-> '2'
  # ```
  def [](name); end

  # Sets an attribute, overwriting any existing attribute value by the same
  # name. Namespace is significant.
  # name
  # :   the name of the attribute
  # value
  # :   (optional) If supplied, the value of the attribute. If nil, any existing
  #     matching attribute is deleted.
  # Returns
  # :   Owning element
  #
  # ```ruby
  # doc = Document.new "<a x:foo='1' foo='3'/>"
  # doc.root.attributes['y:foo'] = '2'
  # doc.root.attributes['foo'] = '4'
  # doc.root.attributes['x:foo'] = nil
  # ```
  def []=(name, value); end

  # Adds an attribute, overriding any existing attribute by the same name.
  # Namespaces are significant.
  # attribute
  # :   An Attribute
  #
  #
  # Also aliased as:
  # [`<<`](https://docs.ruby-lang.org/en/2.7.0/REXML/Attributes.html#method-i-3C-3C)
  def add(attribute); end

  # Removes an attribute
  # attribute
  # :   either a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html),
  #     which is the name of the attribute to remove -- namespaces are
  #     significant here -- or the attribute to remove.
  # Returns
  # :   the owning element
  #
  # ```ruby
  # doc = Document.new "<a y:foo='0' x:foo='1' foo='3' z:foo='4'/>"
  # doc.root.attributes.delete 'foo'   #-> <a y:foo='0' x:foo='1' z:foo='4'/>"
  # doc.root.attributes.delete 'x:foo' #-> <a y:foo='0' z:foo='4'/>"
  # attr = doc.root.attributes.get_attribute('y:foo')
  # doc.root.attributes.delete attr    #-> <a z:foo='4'/>"
  # ```
  def delete(attribute); end

  # Deletes all attributes matching a name. Namespaces are significant.
  # name
  # :   A [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html); all
  #     attributes that match this path will be removed
  # Returns
  # :   an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of the
  #     [`Attributes`](https://docs.ruby-lang.org/en/2.7.0/REXML/Attributes.html)
  #     that were removed
  def delete_all(name); end

  # Iterates over each attribute of an Element, yielding the expanded name and
  # value as a pair of Strings.
  #
  # ```ruby
  # doc = Document.new '<a x="1" y="2"/>'
  # doc.root.attributes.each {|name, value| p name+" => "+value }
  # ```
  def each; end

  # Iterates over the attributes of an Element. Yields actual Attribute nodes,
  # not [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) values.
  #
  # ```ruby
  # doc = Document.new '<a x="1" y="2"/>'
  # doc.root.attributes.each_attribute {|attr|
  #   p attr.expanded_name+" => "+attr.value
  # }
  # ```
  def each_attribute; end

  # Fetches an attribute
  # name
  # :   the name by which to search for the attribute. Can be a `prefix:name`
  #     namespace name.
  # Returns
  # :   The first matching attribute, or nil if there was none. This
  #
  # value is an Attribute node, not the
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) value of the
  # attribute.
  #
  # ```ruby
  # doc = Document.new '<a x:foo="1" foo="2" bar="3"/>'
  # doc.root.attributes.get_attribute("foo").value    #-> "2"
  # doc.root.attributes.get_attribute("x:foo").value  #-> "1"
  # ```
  def get_attribute(name); end

  # The `get_attribute_ns` method retrieves a method by its namespace and name.
  # Thus it is possible to reliably identify an attribute even if an
  # [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) processor has changed
  # the prefix.
  #
  # [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) contributed by
  # Henrik Martensson
  def get_attribute_ns(namespace, name); end

  # Returns the number of attributes the owning Element contains.
  #
  # ```ruby
  # doc = Document "<a x='1' y='2' foo:x='3'/>"
  # doc.root.attributes.length        #-> 3
  # ```
  #
  #
  # Also aliased as:
  # [`size`](https://docs.ruby-lang.org/en/2.7.0/REXML/Attributes.html#method-i-size)
  def length; end

  def namespaces; end

  # Returns an array of Strings containing all of the prefixes declared by this
  # set of # attributes. The array does not include the default namespace
  # declaration, if one exists.
  #
  # ```ruby
  # doc = Document.new("<a xmlns='foo' xmlns:x='bar' xmlns:y='twee' "+
  #       "z='glorp' p:k='gru'/>")
  # prefixes = doc.root.attributes.prefixes    #-> ['x', 'y']
  # ```
  def prefixes; end

  # Alias for:
  # [`length`](https://docs.ruby-lang.org/en/2.7.0/REXML/Attributes.html#method-i-length)
  def size; end

  def to_a; end
end

class REXML::CData < ::REXML::Text
  ILLEGAL = T.let(T.unsafe(nil), Regexp)

  START = T.let(T.unsafe(nil), String)

  STOP = T.let(T.unsafe(nil), String)

  # ```
  # Constructor.  CData is data between <![CDATA[ ... ]]>
  # ```
  #
  # *Examples*
  #
  # ```ruby
  # CData.new( source )
  # CData.new( "Here is some CDATA" )
  # CData.new( "Some unprocessed data", respect_whitespace_TF, parent_element )
  # ```
  def self.new(first, whitespace = _, parent = _); end

  # Make a copy of this object
  #
  # *Examples*
  #
  # ```ruby
  # c = CData.new( "Some text" )
  # d = c.clone
  # d.to_s        # -> "Some text"
  # ```
  def clone; end

  # Returns the content of this
  # [`CData`](https://docs.ruby-lang.org/en/2.7.0/REXML/CData.html) object
  #
  # *Examples*
  #
  # ```ruby
  # c = CData.new( "Some text" )
  # c.to_s        # -> "Some text"
  # ```
  def to_s; end

  def value; end

  # ## DEPRECATED
  # See the rexml/formatters package
  #
  # Generates [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) output of
  # this object
  #
  # output
  # :   Where to write the string. Defaults to $stdout
  # indent
  # :   The amount to indent this node by
  # transitive
  # :   Ignored
  # ie\_hack
  # :   Ignored
  #
  #
  # *Examples*
  #
  # ```ruby
  # c = CData.new( " Some text " )
  # c.write( $stdout )     #->  <![CDATA[ Some text ]]>
  # ```
  def write(output = _, indent = _, transitive = _, ie_hack = _); end
end

# A [`Child`](https://docs.ruby-lang.org/en/2.7.0/REXML/Child.html) object is
# something contained by a parent, and this class contains methods to support
# that. Most user code will not use this class directly.
class REXML::Child
  include(::REXML::Node)

  # Constructor. Any inheritors of this class should call super to make sure
  # this method is called.
  # parent
  # :   if supplied, the parent of this child will be set to the supplied value,
  #     and self will be added to the parent
  def self.new(parent = _); end

  # This doesn't yet handle encodings
  def bytes; end

  # Returns
  # :   the document this child belongs to, or nil if this child
  #
  # belongs to no document
  def document; end

  def next_sibling; end

  # Sets the next sibling of this child. This can be used to insert a child
  # after some other child.
  #
  # ```ruby
  # a = Element.new("a")
  # b = a.add_element("b")
  # c = Element.new("c")
  # b.next_sibling = c
  # # => <a><b/><c/></a>
  # ```
  def next_sibling=(other); end

  def parent; end

  # Sets the parent of this child to the supplied argument.
  #
  # other
  # :   Must be a Parent object. If this object is the same object as the
  #     existing parent of this child, no action is taken. Otherwise, this child
  #     is removed from the current parent (if one exists), and is added to the
  #     new parent.
  # Returns
  # :   The parent added
  def parent=(other); end

  def previous_sibling; end

  # Sets the previous sibling of this child. This can be used to insert a child
  # before some other child.
  #
  # ```ruby
  # a = Element.new("a")
  # b = a.add_element("b")
  # c = Element.new("c")
  # b.previous_sibling = c
  # # => <a><b/><c/></a>
  # ```
  def previous_sibling=(other); end

  # Removes this child from the parent.
  #
  # Returns
  # :   self
  def remove; end

  # Replaces this object with another object. Basically, calls
  # Parent.replace\_child
  #
  # Returns
  # :   self
  def replace_with(child); end
end

# Represents an [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) comment;
# that is, text between <!-- ... -->
class REXML::Comment < ::REXML::Child
  include(::Comparable)

  START = T.let(T.unsafe(nil), String)

  STOP = T.let(T.unsafe(nil), String)

  # Constructor. The first argument can be one of three types: @param first If
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), the contents of
  # this comment are set to the argument. If
  # [`Comment`](https://docs.ruby-lang.org/en/2.7.0/REXML/Comment.html), the
  # argument is duplicated. If Source, the argument is scanned for a comment.
  # @param second If the first argument is a Source, this argument should be
  # nil, not supplied, or a Parent to be set as the parent of this object
  def self.new(first, second = _); end

  # Compares this
  # [`Comment`](https://docs.ruby-lang.org/en/2.7.0/REXML/Comment.html) to
  # another; the contents of the comment are used in the comparison.
  def <=>(other); end

  # Compares this
  # [`Comment`](https://docs.ruby-lang.org/en/2.7.0/REXML/Comment.html) to
  # another; the contents of the comment are used in the comparison.
  def ==(other); end

  def clone; end

  def node_type; end

  # The content text
  def string; end

  # The content text
  def string=(_); end

  # The content text
  def to_s; end

  # ## DEPRECATED
  # See
  # [`REXML::Formatters`](https://docs.ruby-lang.org/en/2.7.0/REXML/Formatters.html)
  #
  # output
  # :   Where to write the string
  # indent
  # :   An integer.   If -1, no indenting will be used; otherwise, the
  #     indentation will be this number of spaces, and children will be indented
  #     an additional amount.
  # transitive
  # :   Ignored by this class. The contents of comments are never modified.
  # ie\_hack
  # :   Needed for conformity to the child API, but not used by this class.
  def write(output, indent = _, transitive = _, ie_hack = _); end
end

# This is an abstract class. You never use this directly; it serves as a parent
# class for the specific declarations.
class REXML::Declaration < ::REXML::Child
  def self.new(src); end

  def to_s; end

  # ## DEPRECATED
  # See
  # [`REXML::Formatters`](https://docs.ruby-lang.org/en/2.7.0/REXML/Formatters.html)
  def write(output, indent); end
end

# Represents an [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) DOCTYPE
# declaration; that is, the contents of <!DOCTYPE ... >. DOCTYPES can be used to
# declare the DTD of a document, as well as being used to declare entities used
# in the document.
class REXML::DocType < ::REXML::Parent
  include(::REXML::XMLTokens)

  Elem = type_member {{fixed: REXML::Child}}

  DEFAULT_ENTITIES = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  PUBLIC = T.let(T.unsafe(nil), String)

  START = T.let(T.unsafe(nil), String)

  STOP = T.let(T.unsafe(nil), String)

  SYSTEM = T.let(T.unsafe(nil), String)

  # Constructor
  #
  # ```ruby
  # dt = DocType.new( 'foo', '-//I/Hate/External/IDs' )
  # # <!DOCTYPE foo '-//I/Hate/External/IDs'>
  # dt = DocType.new( doctype_to_clone )
  # # Incomplete.  Shallow clone of doctype
  # ```
  #
  # `Note` that the constructor:
  #
  # ```ruby
  # Doctype.new( Source.new( "<!DOCTYPE foo 'bar'>" ) )
  # ```
  #
  # is *deprecated*. Do not use it. It will probably disappear.
  def self.new(first, parent = _); end

  def add(child); end

  def attribute_of(element, attribute); end

  def attributes_of(element); end

  def clone; end

  def context; end

  # name is the name of the doctype
  # [`external_id`](https://docs.ruby-lang.org/en/2.7.0/REXML/DocType.html#attribute-i-external_id)
  # is the referenced DTD, if given
  def entities; end

  def entity(name); end

  # name is the name of the doctype
  # [`external_id`](https://docs.ruby-lang.org/en/2.7.0/REXML/DocType.html#attribute-i-external_id)
  # is the referenced DTD, if given
  def external_id; end

  # name is the name of the doctype
  # [`external_id`](https://docs.ruby-lang.org/en/2.7.0/REXML/DocType.html#attribute-i-external_id)
  # is the referenced DTD, if given
  def name; end

  # name is the name of the doctype
  # [`external_id`](https://docs.ruby-lang.org/en/2.7.0/REXML/DocType.html#attribute-i-external_id)
  # is the referenced DTD, if given
  def namespaces; end

  def node_type; end

  # Retrieves a named notation. Only notations declared in the internal DTD
  # subset can be retrieved.
  #
  # [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) contributed by
  # Henrik Martensson
  def notation(name); end

  # This method returns a list of notations that have been declared in the
  # *internal* DTD subset. Notations in the external DTD subset are not listed.
  #
  # [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) contributed by
  # Henrik Martensson
  def notations; end

  # This method retrieves the public identifier identifying the document's DTD.
  #
  # [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) contributed by
  # Henrik Martensson
  def public; end

  # This method retrieves the system identifier identifying the document's DTD
  #
  # [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) contributed by
  # Henrik Martensson
  def system; end

  # output
  # :   Where to write the string
  # indent
  # :   An integer. If -1, no indentation will be used; otherwise, the
  #     indentation will be this number of spaces, and children will be indented
  #     an additional amount.
  # transitive
  # :   Ignored
  # ie\_hack
  # :   Ignored
  def write(output, indent = _, transitive = _, ie_hack = _); end
end

# Represents a full [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html)
# document, including PIs, a doctype, etc. A
# [`Document`](https://docs.ruby-lang.org/en/2.7.0/REXML/Document.html) has a
# single child that can be accessed by root(). Note that if you want to have an
# [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) declaration written for
# a document you create, you must add one;
# [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) documents do not
# write a default declaration for you. See |DECLARATION| and |write|.
class REXML::Document < ::REXML::Element

  Elem = type_member {{fixed: REXML::Child}}

  # A convenient default [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html)
  # declaration. If you want an
  # [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) declaration, the
  # easiest way to add one is mydoc << Document::DECLARATION `DEPRECATED` Use:
  # mydoc << XMLDecl.default
  DECLARATION = T.let(T.unsafe(nil), REXML::XMLDecl)

  # Constructor @param source if supplied, must be a
  # [`Document`](https://docs.ruby-lang.org/en/2.7.0/REXML/Document.html),
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), or
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html). Documents have their
  # context and Element attributes cloned. Strings are expected to be valid
  # [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) documents. IOs are
  # expected to be sources of valid
  # [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) documents. @param
  # context if supplied, contains the context of the document; this should be a
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html).
  def self.new(source = _, context = _); end

  # Alias for:
  # [`add`](https://docs.ruby-lang.org/en/2.7.0/REXML/Document.html#method-i-add)
  def <<(child); end

  # We override this, because XMLDecls and DocTypes must go at the start of the
  # document
  #
  # Also aliased as:
  # [`<<`](https://docs.ruby-lang.org/en/2.7.0/REXML/Document.html#method-i-3C-3C)
  def add(child); end

  def add_element(arg = _, arg2 = _); end

  # Should be obvious
  def clone; end

  # @return the DocType child of the document, if one exists, and nil otherwise.
  def doctype; end

  def document; end

  # @return the XMLDecl encoding of this document as an
  # [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) object. If
  # no XMLDecl has been set, returns the default encoding.
  def encoding; end

  def entity_expansion_count; end

  # According to the [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) spec,
  # a root node has no expanded name
  #
  # Also aliased as:
  # [`name`](https://docs.ruby-lang.org/en/2.7.0/REXML/Document.html#method-i-name)
  def expanded_name; end

  # Alias for:
  # [`expanded_name`](https://docs.ruby-lang.org/en/2.7.0/REXML/Document.html#method-i-expanded_name)
  def name; end

  def node_type; end

  def record_entity_expansion; end

  # @return the root Element of the document, or nil if this document has no
  # children.
  def root; end

  # @return the XMLDecl standalone value of this document as a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). If no XMLDecl
  # has been set, returns the default setting.
  def stand_alone?; end

  # @return the XMLDecl version of this document as a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). If no XMLDecl
  # has been set, returns the default version.
  def version; end

  # Write the [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) tree out,
  # optionally with indent. This writes out the entire
  # [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) document, including
  # [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) declarations, doctype
  # declarations, and processing instructions (if any are given).
  #
  # A controversial point is whether
  # [`Document`](https://docs.ruby-lang.org/en/2.7.0/REXML/Document.html) should
  # always write the [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html)
  # declaration (<?xml version='1.0'?>) whether or not one is given by the user
  # (or source document).
  # [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) does not write one
  # if one was not specified, because it adds unnecessary bandwidth to
  # applications such as XML-RPC.
  #
  # Accept Nth argument style and options
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) style as argument.
  # The recommended style is options
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) style for one or
  # more arguments case.
  #
  # *Examples*
  #
  # ```ruby
  # Document.new("<a><b/></a>").write
  #
  # output = ""
  # Document.new("<a><b/></a>").write(output)
  #
  # output = ""
  # Document.new("<a><b/></a>").write(:output => output, :indent => 2)
  # ```
  #
  # See also the classes in the rexml/formatters package for the proper way to
  # change the default formatting of
  # [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) output.
  #
  # *Examples*
  #
  # ```ruby
  # output = ""
  # tr = Transitive.new
  # tr.write(Document.new("<a><b/></a>"), output)
  # ```
  #
  # output
  # :   output an object which supports '<< string'; this is where the document
  #     will be written.
  # indent
  # :   An integer. If -1, no indenting will be used; otherwise, the indentation
  #     will be twice this number of spaces, and children will be indented an
  #     additional amount. For a value of 3, every item will be indented 3 more
  #     levels, or 6 more spaces (2 \* 3). Defaults to -1
  # transitive
  # :   If transitive is true and indent is >= 0, then the output will be
  #     pretty-printed in such a way that the added whitespace does not affect
  #     the absolute **value** of the document -- that is, it leaves the value
  #     and number of Text nodes in the document unchanged.
  # ie\_hack
  # :   This hack inserts a space before the /> on empty tags to address a
  #     limitation of Internet Explorer. Defaults to false
  # encoding
  # :   [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) name as
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). Change
  #     output encoding to specified encoding instead of encoding in
  #     [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) declaration.
  #     Defaults to nil. It means encoding in
  #     [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) declaration is
  #     used.
  def write(*arguments); end

  # @return the XMLDecl of this document; if no XMLDecl has been set, the
  # default declaration is returned.
  def xml_decl; end

  # Get the entity expansion limit. By default the limit is set to 10000.
  #
  # Deprecated. Use
  # [`REXML::Security.entity_expansion_limit=`](https://docs.ruby-lang.org/en/2.7.0/REXML/Security.html#method-c-entity_expansion_limit-3D)
  # instead.
  def self.entity_expansion_limit; end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the entity expansion
  # limit. By default the limit is set to 10000.
  #
  # Deprecated. Use
  # [`REXML::Security.entity_expansion_limit=`](https://docs.ruby-lang.org/en/2.7.0/REXML/Security.html#method-c-entity_expansion_limit-3D)
  # instead.
  def self.entity_expansion_limit=(val); end

  # Get the entity expansion limit. By default the limit is set to 10240.
  #
  # Deprecated. Use
  # [`REXML::Security.entity_expansion_text_limit`](https://docs.ruby-lang.org/en/2.7.0/REXML/Security.html#method-c-entity_expansion_text_limit)
  # instead.
  def self.entity_expansion_text_limit; end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the entity expansion
  # limit. By default the limit is set to 10240.
  #
  # Deprecated. Use
  # [`REXML::Security.entity_expansion_text_limit=`](https://docs.ruby-lang.org/en/2.7.0/REXML/Security.html#method-c-entity_expansion_text_limit-3D)
  # instead.
  def self.entity_expansion_text_limit=(val); end

  def self.parse_stream(source, listener); end
end

# Represents a tagged [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html)
# element. Elements are characterized by having children, attributes, and names,
# and can themselves be children.
class REXML::Element < ::REXML::Parent
  include(::REXML::Namespace)
  include(::REXML::XMLTokens)

  Elem = type_member {{fixed: REXML::Child}}

  UNDEFINED = T.let(T.unsafe(nil), String)

  # Constructor
  # arg
  # :   if not supplied, will be set to the default value. If a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), the name of
  #     this object will be set to the argument. If an
  #     [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html), the
  #     object will be shallowly cloned; name, attributes, and namespaces will
  #     be copied. Children will `not` be copied.
  # parent
  # :   if supplied, must be a Parent, and will be used as the parent of this
  #     object.
  # context
  # :   If supplied, must be a hash containing context items. Context items
  #     include:
  #
  # *   `:respect_whitespace` the value of this is :`all` or an array of strings
  #     being the names of the elements to respect whitespace for. Defaults to
  #     :`all`.
  # *   `:compress_whitespace` the value can be :`all` or an array of strings
  #     being the names of the elements to ignore whitespace on. Overrides
  #     :`respect_whitespace`.
  # *   `:ignore_whitespace_nodes` the value can be :`all` or an array of
  #     strings being the names of the elements in which to ignore
  #     whitespace-only nodes. If this is set, Text nodes which contain only
  #     whitespace will not be added to the document tree.
  # *   `:raw` can be :`all`, or an array of strings being the names of the
  #     elements to process in raw mode. In raw mode, special characters in text
  #     is not converted to or from entities.
  def self.new(arg = _, parent = _, context = _); end

  # Fetches an attribute value or a child.
  #
  # If [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) or
  # [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html) is specified,
  # it's treated as attribute name. Attribute value as
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) or `nil` is
  # returned. This case is shortcut of +[attributes](name)+.
  #
  # If [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) is
  # specified, it's treated as the index of child. It returns Nth child.
  #
  # ```ruby
  # doc = REXML::Document.new("<a attr='1'><b/><c/></a>")
  # doc.root["attr"]             # => "1"
  # doc.root.attributes["attr"]  # => "1"
  # doc.root[1]                  # => <c/>
  # ```
  def [](name_or_index); end

  # Adds an attribute to this element, overwriting any existing attribute by the
  # same name.
  # key
  # :   can be either an Attribute or a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). If an
  #     Attribute, the attribute is added to the list of
  #     [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html)
  #     attributes. If
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), the
  #     argument is used as the name of the new attribute, and the value
  #     parameter must be supplied.
  # value
  # :   Required if `key` is a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), and ignored
  #     if the first argument is an Attribute. This is a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), and is used
  #     as the value of the new Attribute. This should be the unnormalized value
  #     of the attribute (without entities).
  # Returns
  # :   the Attribute added
  #
  # ```ruby
  # e = Element.new 'e'
  # e.add_attribute( 'a', 'b' )               #-> <e a='b'/>
  # e.add_attribute( 'x:a', 'c' )             #-> <e a='b' x:a='c'/>
  # e.add_attribute Attribute.new('b', 'd')   #-> <e a='b' x:a='c' b='d'/>
  # ```
  def add_attribute(key, value = _); end

  # Add multiple attributes to this element.
  # hash
  # :   is either a hash, or array of arrays
  #
  # ```ruby
  # el.add_attributes( {"name1"=>"value1", "name2"=>"value2"} )
  # el.add_attributes( [ ["name1","value1"], ["name2"=>"value2"] ] )
  # ```
  def add_attributes(hash); end

  # Adds a child to this element, optionally setting attributes in the element.
  # element
  # :   optional. If
  #     [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html), the
  #     element is added. Otherwise, a new
  #     [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html) is
  #     constructed with the argument (see Element.initialize).
  # attrs
  # :   If supplied, must be a
  #     [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) containing
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) name,value
  #     pairs, which will be used to set the attributes of the new
  #     [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html).
  # Returns
  # :   the [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html)
  #     that was added
  #
  # ```ruby
  # el = doc.add_element 'my-tag'
  # el = doc.add_element 'my-tag', {'attr1'=>'val1', 'attr2'=>'val2'}
  # el = Element.new 'my-tag'
  # doc.add_element el
  # ```
  def add_element(element, attrs = _); end

  # Adds a namespace to this element.
  # prefix
  # :   the prefix string, or the namespace
  #     [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) if `uri` is not
  #     supplied
  # uri
  # :   the namespace [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html). May
  #     be nil, in which `prefix` is used as the
  #     [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html)
  #
  # Evaluates to: this
  # [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html)
  #
  # ```ruby
  # a = Element.new("a")
  # a.add_namespace("xmlns:foo", "bar" )
  # a.add_namespace("foo", "bar")  # shorthand for previous line
  # a.add_namespace("twiddle")
  # puts a   #-> <a xmlns:foo='bar' xmlns='twiddle'/>
  # ```
  def add_namespace(prefix, uri = _); end

  # A helper method to add a Text child. Actual Text instances can be added with
  # regular Parent methods, such as add() and <<()
  # text
  # :   if a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), a new
  #     Text instance is created and added to the parent. If Text, the object is
  #     added directly.
  # Returns
  # :   this [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html)
  #
  # ```ruby
  # e = Element.new('a')          #-> <e/>
  # e.add_text 'foo'              #-> <e>foo</e>
  # e.add_text Text.new(' bar')    #-> <e>foo bar</e>
  # ```
  #
  # Note that at the end of this example, the branch has **3** nodes; the 'e'
  # element and **2** Text node children.
  def add_text(text); end

  def attribute(name, namespace = _); end

  # Mechanisms for accessing attributes and child elements of this element.
  def attributes; end

  # Get an array of all CData children. IMMUTABLE
  def cdatas; end

  # Creates a shallow copy of self.
  #
  # ```ruby
  # d = Document.new "<a><b/><b/><c><d/></c></a>"
  # new_a = d.root.clone
  # puts new_a  # => "<a/>"
  # ```
  def clone; end

  # Get an array of all Comment children. IMMUTABLE
  def comments; end

  # The context holds information about the processing environment, such as
  # whitespace handling.
  def context; end

  # The context holds information about the processing environment, such as
  # whitespace handling.
  def context=(_); end

  # Removes an attribute
  # key
  # :   either an Attribute or a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). In either
  #     case, the attribute is found by matching the attribute name to the
  #     argument, and then removed. If no attribute is found, no action is
  #     taken.
  # Returns
  # :   the attribute removed, or nil if this
  #     [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html) did
  #     not contain a matching attribute
  #
  # ```ruby
  # e = Element.new('E')
  # e.add_attribute( 'name', 'Sean' )             #-> <E name='Sean'/>
  # r = e.add_attribute( 'sur:name', 'Russell' )  #-> <E name='Sean' sur:name='Russell'/>
  # e.delete_attribute( 'name' )                  #-> <E sur:name='Russell'/>
  # e.delete_attribute( r )                       #-> <E/>
  # ```
  def delete_attribute(key); end

  # Deletes a child element.
  # element
  # :   Must be an `Element`, `String`, or `Integer`. If
  #     [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html), the
  #     element is removed. If
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), the element
  #     is found (via XPath) and removed. <em>This means that any parent can
  #     remove any descendant.<em>  If
  #     [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html), the
  #     [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html)
  #     indexed by that number will be removed.
  # Returns
  # :   the element that was removed.
  #
  # ```ruby
  # doc.delete_element "/a/b/c[@id='4']"
  # doc.delete_element doc.elements["//k"]
  # doc.delete_element 1
  # ```
  def delete_element(element); end

  # Removes a namespace from this node. This only works if the namespace is
  # actually declared in this node. If no argument is passed, deletes the
  # default namespace.
  #
  # Evaluates to: this element
  #
  # ```ruby
  # doc = Document.new "<a xmlns:foo='bar' xmlns='twiddle'/>"
  # doc.root.delete_namespace
  # puts doc     # -> <a xmlns:foo='bar'/>
  # doc.root.delete_namespace 'foo'
  # puts doc     # -> <a/>
  # ```
  def delete_namespace(namespace = _); end

  # Evaluates to the document to which this element belongs, or nil if this
  # element doesn't belong to a document.
  def document; end

  # Synonym for
  # [`Element.elements`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html#attribute-i-elements).each
  def each_element(xpath = _, &block); end

  # Iterates through the child elements, yielding for each
  # [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html) that has
  # a particular attribute set.
  # key
  # :   the name of the attribute to search for
  # value
  # :   the value of the attribute
  # max
  # :   (optional) causes this method to return after yielding for this number
  #     of matching children
  # name
  # :   (optional) if supplied, this is an XPath that filters the children to
  #     check.
  #
  #
  # ```ruby
  # doc = Document.new "<a><b @id='1'/><c @id='2'/><d @id='1'/><e/></a>"
  # # Yields b, c, d
  # doc.root.each_element_with_attribute( 'id' ) {|e| p e}
  # # Yields b, d
  # doc.root.each_element_with_attribute( 'id', '1' ) {|e| p e}
  # # Yields b
  # doc.root.each_element_with_attribute( 'id', '1', 1 ) {|e| p e}
  # # Yields d
  # doc.root.each_element_with_attribute( 'id', '1', 0, 'd' ) {|e| p e}
  # ```
  def each_element_with_attribute(key, value = _, max = _, name = _, &block); end

  # Iterates through the children, yielding for each
  # [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html) that has
  # a particular text set.
  # text
  # :   the text to search for. If nil, or not supplied, will iterate over all
  #     `Element` children that contain at least one `Text` node.
  # max
  # :   (optional) causes this method to return after yielding for this number
  #     of matching children
  # name
  # :   (optional) if supplied, this is an XPath that filters the children to
  #     check.
  #
  #
  # ```ruby
  # doc = Document.new '<a><b>b</b><c>b</c><d>d</d><e/></a>'
  # # Yields b, c, d
  # doc.each_element_with_text {|e|p e}
  # # Yields b, c
  # doc.each_element_with_text('b'){|e|p e}
  # # Yields b
  # doc.each_element_with_text('b', 1){|e|p e}
  # # Yields d
  # doc.each_element_with_text(nil, 0, 'd'){|e|p e}
  # ```
  def each_element_with_text(text = _, max = _, name = _, &block); end

  # Mechanisms for accessing attributes and child elements of this element.
  def elements; end

  # Synonym for Element.to\_a This is a little slower than calling elements.each
  # directly.
  # xpath
  # :   any XPath by which to search for elements in the tree
  # Returns
  # :   an array of Elements that match the supplied path
  def get_elements(xpath); end

  # Returns the first child Text node, if any, or `nil` otherwise. This method
  # returns the actual `Text` node, rather than the
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) content.
  #
  # ```ruby
  # doc = Document.new "<p>some text <b>this is bold!</b> more text</p>"
  # # The element 'p' has two text elements, "some text " and " more text".
  # doc.root.get_text.value            #-> "some text "
  # ```
  def get_text(path = _); end

  # Evaluates to `true` if this element has any attributes set, false otherwise.
  def has_attributes?; end

  # Evaluates to `true` if this element has at least one child
  # [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html)
  #
  # ```ruby
  # doc = Document.new "<a><b/><c>Text</c></a>"
  # doc.root.has_elements               # -> true
  # doc.elements["/a/b"].has_elements   # -> false
  # doc.elements["/a/c"].has_elements   # -> false
  # ```
  def has_elements?; end

  # Evaluates to `true` if this element has at least one Text child
  def has_text?; end

  def ignore_whitespace_nodes; end

  def inspect; end

  # Get an array of all Instruction children. IMMUTABLE
  def instructions; end

  # Evaluates to the [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) for a
  # prefix, or the empty string if no such namespace is declared for this
  # element. Evaluates recursively for ancestors. Returns the default namespace,
  # if there is one.
  # prefix
  # :   the prefix to search for. If not supplied, returns the default namespace
  #     if one exists
  # Returns
  # :   the namespace [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) as a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), or nil if
  #     no such namespace exists. If the namespace is undefined, returns an
  #     empty string
  #
  # ```ruby
  # doc = Document.new("<a xmlns='1' xmlns:y='2'><b/><c xmlns:z='3'/></a>")
  # b = doc.elements['//b']
  # b.namespace           # -> '1'
  # b.namespace("y")      # -> '2'
  # ```
  def namespace(prefix = _); end

  def namespaces; end

  # Returns the next sibling that is an element, or nil if there is no
  # [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html) sibling
  # after this one
  #
  # ```ruby
  # doc = Document.new '<a><b/>text<c/></a>'
  # doc.root.elements['b'].next_element          #-> <c/>
  # doc.root.elements['c'].next_element          #-> nil
  # ```
  def next_element; end

  def node_type; end

  # Evaluates to an `Array` containing the prefixes (names) of all defined
  # namespaces at this context node.
  #
  # ```ruby
  # doc = Document.new("<a xmlns:x='1' xmlns:y='2'><b/><c xmlns:z='3'/></a>")
  # doc.elements['//b'].prefixes # -> ['x', 'y']
  # ```
  def prefixes; end

  # Returns the previous sibling that is an element, or nil if there is no
  # [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html) sibling
  # prior to this one
  #
  # ```ruby
  # doc = Document.new '<a><b/>text<c/></a>'
  # doc.root.elements['c'].previous_element          #-> <b/>
  # doc.root.elements['b'].previous_element          #-> nil
  # ```
  def previous_element; end

  # Evaluates to `true` if raw mode is set for this element. This is the case if
  # the context has :`raw` set to :`all` or an array containing the name of this
  # element.
  #
  # The evaluation is tested against `expanded_name`, and so is namespace
  # sensitive.
  def raw; end

  def root; end

  # Evaluates to the root node of the document that this element belongs to. If
  # this element doesn't belong to a document, but does belong to another
  # [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html), the
  # parent's root will be returned, until the earliest ancestor is found.
  #
  # Note that this is not the same as the document element. In the following
  # example, <a> is the document element, and the root node is the parent node
  # of the document element. You may ask yourself why the root node is useful:
  # consider the doctype and
  # [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) declaration, and any
  # processing instructions before the document element... they are children of
  # the root node, or siblings of the document element. The only time this isn't
  # true is when an
  # [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html) is
  # created that is not part of any Document. In this case, the ancestor that
  # has no parent acts as the root node.
  #
  # ```ruby
  # d = Document.new '<a><b><c/></b></a>'
  # a = d[1] ; c = a[1][1]
  # d.root_node == d   # TRUE
  # a.root_node        # namely, d
  # c.root_node        # again, d
  # ```
  def root_node; end

  # A convenience method which returns the
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) value of the
  # *first* child text element, if one exists, and `nil` otherwise.
  #
  # *Note that an element may have multiple Text elements, perhaps separated by
  # other children*. Be aware that this method only returns the first Text node.
  #
  # This method returns the `value` of the first text child node, which ignores
  # the `raw` setting, so always returns normalized text. See the Text::value
  # documentation.
  #
  # ```ruby
  # doc = Document.new "<p>some text <b>this is bold!</b> more text</p>"
  # # The element 'p' has two text elements, "some text " and " more text".
  # doc.root.text              #-> "some text "
  # ```
  def text(path = _); end

  # Sets the first Text child of this object. See text() for a discussion about
  # Text children.
  #
  # If a Text child already exists, the child is replaced by this content. This
  # means that Text content can be deleted by calling this method with a nil
  # argument. In this case, the next Text child becomes the first Text child. In
  # no case is the order of any siblings disturbed.
  # text
  # :   If a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), a new
  #     Text child is created and added to this
  #     [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html) as
  #     the first Text child. If Text, the text is set as the first Child
  #     element. If nil, then any existing first Text child is removed.
  # Returns
  # :   this
  #     [`Element`](https://docs.ruby-lang.org/en/2.7.0/REXML/Element.html).
  #
  # ```ruby
  # doc = Document.new '<a><b/></a>'
  # doc.root.text = 'Sean'      #-> '<a><b/>Sean</a>'
  # doc.root.text = 'Elliott'   #-> '<a><b/>Elliott</a>'
  # doc.root.add_element 'c'    #-> '<a><b/>Elliott<c/></a>'
  # doc.root.text = 'Russell'   #-> '<a><b/>Russell<c/></a>'
  # doc.root.text = nil         #-> '<a><b/><c/></a>'
  # ```
  def text=(text); end

  # Get an array of all Text children. IMMUTABLE
  def texts; end

  # Evaluates to `true` if whitespace is respected for this element. This is the
  # case if:
  # 1.  Neither :`respect_whitespace` nor :`compress_whitespace` has any value
  # 2.  The context has :`respect_whitespace` set to :`all` or an array
  #     containing the name of this element, and :`compress_whitespace` isn't
  #     set to :`all` or an array containing the name of this element.
  #
  # The evaluation is tested against `expanded_name`, and so is namespace
  # sensitive.
  def whitespace; end

  # ## DEPRECATED
  # See
  # [`REXML::Formatters`](https://docs.ruby-lang.org/en/2.7.0/REXML/Formatters.html)
  #
  # Writes out this element, and recursively, all children.
  # output
  # :   output an object which supports '<< string'; this is where the
  #
  # ```
  # document will be written.
  # ```
  #
  # indent
  # :   An integer. If -1, no indenting will be used; otherwise, the indentation
  #     will be this number of spaces, and children will be indented an
  #     additional amount. Defaults to -1
  # transitive
  # :   If transitive is true and indent is >= 0, then the output will be
  #     pretty-printed in such a way that the added whitespace does not affect
  #     the parse tree of the document
  # ie\_hack
  # :   This hack inserts a space before the /> on empty tags to address a
  #     limitation of Internet Explorer. Defaults to false
  #
  #
  # ```ruby
  # out = ''
  # doc.write( out )     #-> doc is written to the string 'out'
  # doc.write( $stdout ) #-> doc written to the console
  # ```
  def write(output = _, indent = _, transitive = _, ie_hack = _); end

  def xpath; end
end

class REXML::ElementDecl < ::REXML::Declaration
  def self.new(src); end
end

# A class which provides filtering of children for
# [`Elements`](https://docs.ruby-lang.org/en/2.7.0/REXML/Elements.html), and
# XPath search support. You are expected to only encounter this class as the
# `element.elements` object. Therefore, you are *not* expected to instantiate
# this yourself.
class REXML::Elements
  include(::Enumerable)

  Elem = type_member {{fixed: REXML::Element}}

  # Constructor
  # parent
  # :   the parent Element
  def self.new(parent); end

  # Alias for:
  # [`add`](https://docs.ruby-lang.org/en/2.7.0/REXML/Elements.html#method-i-add)
  def <<(element = _); end

  # Fetches a child element. Filters only Element children, regardless of the
  # XPath match.
  # index
  # :   the search parameter. This is either an
  #     [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html), which
  #     will be used to find the index'th child Element, or an XPath, which will
  #     be used to search for the Element. *Because of the nature of XPath
  #     searches, any element in the connected
  #     [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) document can be
  #     fetched through any other element.*  **The
  #     [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) index is
  #     1-based, not 0-based.**  This means that the first child element is at
  #     index 1, not 0, and the +n+th element is at index `n`, not `n-1`. This
  #     is because XPath indexes element children starting from 1, not 0, and
  #     the indexes should be the same.
  # name
  # :   optional, and only used in the first argument is an
  #     [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html). In that
  #     case, the index'th child Element that has the supplied name will be
  #     returned. Note again that the indexes start at 1.
  # Returns
  # :   the first matching Element, or nil if no child matched
  #
  # ```ruby
  # doc = Document.new '<a><b/><c id="1"/><c id="2"/><d/></a>'
  # doc.root.elements[1]       #-> <b/>
  # doc.root.elements['c']     #-> <c id="1"/>
  # doc.root.elements[2,'c']   #-> <c id="2"/>
  # ```
  def [](index, name = _); end

  # Sets an element, replacing any previous matching element. If no existing
  # element is found ,the element is added.
  # index
  # :   Used to find a matching element to replace. See []().
  # element
  # :   The element to replace the existing element with the previous element
  # Returns
  # :   nil if no previous element was found.
  #
  #
  # ```ruby
  # doc = Document.new '<a/>'
  # doc.root.elements[10] = Element.new('b')    #-> <a><b/></a>
  # doc.root.elements[1]                        #-> <b/>
  # doc.root.elements[1] = Element.new('c')     #-> <a><c/></a>
  # doc.root.elements['c'] = Element.new('d')   #-> <a><d/></a>
  # ```
  def []=(index, element); end

  # Adds an element
  # element
  # :   if supplied, is either an Element,
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), or Source
  #     (see Element.initialize). If not supplied or nil, a new, default Element
  #     will be constructed
  # Returns
  # :   the added Element
  #
  # ```ruby
  # a = Element.new('a')
  # a.elements.add(Element.new('b'))  #-> <a><b/></a>
  # a.elements.add('c')               #-> <a><b/><c/></a>
  # ```
  #
  #
  # Also aliased as:
  # [`<<`](https://docs.ruby-lang.org/en/2.7.0/REXML/Elements.html#method-i-3C-3C)
  def add(element = _); end

  def collect(xpath = _); end

  # Deletes a child Element
  # element
  # :   Either an Element, which is removed directly; an xpath, where the first
  #     matching child is removed; or an
  #     [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html), where the
  #     n'th Element is removed.
  # Returns
  # :   the removed child
  #
  # ```ruby
  # doc = Document.new '<a><b/><c/><c id="1"/></a>'
  # b = doc.root.elements[1]
  # doc.root.elements.delete b           #-> <a><c/><c id="1"/></a>
  # doc.elements.delete("a/c[@id='1']")  #-> <a><c/></a>
  # doc.root.elements.delete 1           #-> <a/>
  # ```
  def delete(element); end

  # Removes multiple elements. Filters for Element children, regardless of XPath
  # matching.
  # xpath
  # :   all elements matching this
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) path are
  #     removed.
  # Returns
  # :   an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  #     [`Elements`](https://docs.ruby-lang.org/en/2.7.0/REXML/Elements.html)
  #     that have been removed
  #
  # ```ruby
  # doc = Document.new '<a><c/><c/><c/><c/></a>'
  # deleted = doc.elements.delete_all 'a/c' #-> [<c/>, <c/>, <c/>, <c/>]
  # ```
  def delete_all(xpath); end

  # Iterates through all of the child
  # [`Elements`](https://docs.ruby-lang.org/en/2.7.0/REXML/Elements.html),
  # optionally filtering them by a given XPath
  # xpath
  # :   optional. If supplied, this is a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) XPath, and
  #     is used to filter the children, so that only matching children are
  #     yielded. Note that XPaths are automatically filtered for
  #     [`Elements`](https://docs.ruby-lang.org/en/2.7.0/REXML/Elements.html),
  #     so that non-Element children will not be yielded
  #
  # ```ruby
  # doc = Document.new '<a><b/><c/><d/>sean<b/><c/><d/></a>'
  # doc.root.elements.each {|e|p e}       #-> Yields b, c, d, b, c, d elements
  # doc.root.elements.each('b') {|e|p e}  #-> Yields b, b elements
  # doc.root.elements.each('child::node()')  {|e|p e}
  # #-> Yields <b/>, <c/>, <d/>, <b/>, <c/>, <d/>
  # XPath.each(doc.root, 'child::node()', &block)
  # #-> Yields <b/>, <c/>, <d/>, sean, <b/>, <c/>, <d/>
  # ```
  def each(xpath = _); end

  # Returns `true` if there are no `Element` children, `false` otherwise
  def empty?; end

  # Returns the index of the supplied child (starting at 1), or -1 if the
  # element is not a child
  # element
  # :   an `Element` child
  def index(element); end

  def inject(xpath = _, initial = _); end

  # Returns the number of `Element` children of the parent object.
  #
  # ```ruby
  # doc = Document.new '<a>sean<b/>elliott<b/>russell<b/></a>'
  # doc.root.size            #-> 6, 3 element and 3 text nodes
  # doc.root.elements.size   #-> 3
  # ```
  def size; end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # Element children. An XPath may be supplied to filter the children. Only
  # Element children are returned, even if the supplied XPath matches
  # non-Element children.
  #
  # ```ruby
  # doc = Document.new '<a>sean<b/>elliott<c/></a>'
  # doc.root.elements.to_a                  #-> [ <b/>, <c/> ]
  # doc.root.elements.to_a("child::node()") #-> [ <b/>, <c/> ]
  # XPath.match(doc.root, "child::node()")  #-> [ sean, <b/>, elliott, <c/> ]
  # ```
  def to_a(xpath = _); end
end

module REXML::Encoding
  def decode(string); end

  def encode(string); end

  # ID --->
  # [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/REXML/Encoding.html) name
  def encoding; end

  def encoding=(encoding); end
end

class REXML::Entity < ::REXML::Child
  include(::REXML::XMLTokens)

  ENTITYDECL = T.let(T.unsafe(nil), Regexp)

  ENTITYDEF = T.let(T.unsafe(nil), String)

  ENTITYVALUE = T.let(T.unsafe(nil), String)

  EXTERNALID = T.let(T.unsafe(nil), String)

  GEDECL = T.let(T.unsafe(nil), String)

  NDATADECL = T.let(T.unsafe(nil), String)

  PEDECL = T.let(T.unsafe(nil), String)

  PEDEF = T.let(T.unsafe(nil), String)

  PEREFERENCE = T.let(T.unsafe(nil), String)

  PEREFERENCE_RE = T.let(T.unsafe(nil), Regexp)

  PUBIDCHAR = T.let(T.unsafe(nil), String)

  PUBIDLITERAL = T.let(T.unsafe(nil), String)

  SYSTEMLITERAL = T.let(T.unsafe(nil), String)

  # Create a new entity. Simple entities can be constructed by passing a name,
  # value to the constructor; this creates a generic, plain entity reference.
  # For anything more complicated, you have to pass a Source to the constructor
  # with the entity definition, or use the accessor methods. `WARNING`: There is
  # no validation of entity state except when the entity is read from a stream.
  # If you start poking around with the accessors, you can easily create a
  # non-conformant
  # [`Entity`](https://docs.ruby-lang.org/en/2.7.0/REXML/Entity.html).
  #
  # ```ruby
  # e = Entity.new( 'amp', '&' )
  # ```
  def self.new(stream, value = _, parent = _, reference = _); end

  def external; end

  def name; end

  def ndata; end

  # Returns the value of this entity unprocessed -- raw. This is the normalized
  # value; that is, with all %ent; and &ent; entities intact
  def normalized; end

  def pubid; end

  def ref; end

  # Returns this entity as a string. See write().
  def to_s; end

  # Evaluates to the unnormalized value of this entity; that is, replacing all
  # entities -- both %ent; and &ent; entities. This differs from +value()+ in
  # that `value` only replaces %ent; entities.
  def unnormalized; end

  # Returns the value of this entity. At the moment, only internal entities are
  # processed. If the value contains internal references (IE, %blah;), those are
  # replaced with their values. IE, if the doctype contains:
  #
  # ```
  # <!ENTITY % foo "bar">
  # <!ENTITY yada "nanoo %foo; nanoo>
  # ```
  #
  # then:
  #
  # ```ruby
  # doctype.entity('yada').value   #-> "nanoo bar nanoo"
  # ```
  def value; end

  # Write out a fully formed, correct entity definition (assuming the
  # [`Entity`](https://docs.ruby-lang.org/en/2.7.0/REXML/Entity.html) object
  # itself is valid.)
  #
  # out
  # :   An object implementing `&lt;&lt;` to which the entity will be output
  # indent
  # :   **DEPRECATED** and ignored
  def write(out, indent = _); end

  # Evaluates whether the given string matches an entity definition, returning
  # true if so, and false otherwise.
  def self.matches?(string); end
end

# This is a set of entity constants -- the ones defined in the
# [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) specification. These are
# `gt`, `lt`, `amp`, `quot` and `apos`. CAUTION: these entities does not have
# parent and document
module REXML::EntityConst
  # +&+
  AMP = T.let(T.unsafe(nil), REXML::Entity)

  # +'+
  APOS = T.let(T.unsafe(nil), REXML::Entity)

  # +>+
  GT = T.let(T.unsafe(nil), REXML::Entity)

  # +<+
  LT = T.let(T.unsafe(nil), REXML::Entity)

  # +"+
  QUOT = T.let(T.unsafe(nil), REXML::Entity)
end

class REXML::ExternalEntity < ::REXML::Child
  def self.new(src); end

  def to_s; end

  def write(output, indent); end
end

module REXML::Formatters; end

class REXML::Formatters::Default
  # Prints out the [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html)
  # document with no formatting -- except if ie\_hack is set.
  #
  # ie\_hack
  # :   If set to true, then inserts whitespace before the close of an empty
  #     tag, so that IE's bad
  #     [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) parser doesn't
  #     choke.
  def self.new(ie_hack = _); end

  # Writes the node to some output.
  #
  # node
  # :   The node to write
  # output
  # :   A class implementing `&lt;&lt;`. Pass in an Output object to change the
  #     output encoding.
  def write(node, output); end

  protected

  def write_cdata(node, output); end

  def write_comment(node, output); end

  def write_document(node, output); end

  def write_element(node, output); end

  def write_instruction(node, output); end

  def write_text(node, output); end
end

# Pretty-prints an [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html)
# document. This destroys whitespace in text nodes and will insert carriage
# returns and indentations.
#
# TODO: Add an option to print attributes on new lines
class REXML::Formatters::Pretty < ::REXML::Formatters::Default
  # Create a new pretty printer.
  #
  # output
  # :   An object implementing
  #     '<<([`String`](https://docs.ruby-lang.org/en/2.7.0/String.html))', to
  #     which the output will be written.
  # indentation
  # :   An integer greater than 0. The indentation of each level will be this
  #     number of spaces. If this is < 1, the behavior of this object is
  #     undefined. Defaults to 2.
  # ie\_hack
  # :   If true, the printer will insert whitespace before closing empty tags,
  #     thereby allowing Internet Explorer's
  #     [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) parser to
  #     function. Defaults to false.
  def self.new(indentation = _, ie_hack = _); end

  # If compact is set to true, then the formatter will attempt to use as little
  # space as possible
  def compact; end

  # If compact is set to true, then the formatter will attempt to use as little
  # space as possible
  def compact=(_); end

  # The width of a page. Used for formatting text
  def width; end

  # The width of a page. Used for formatting text
  def width=(_); end

  protected

  def write_cdata(node, output); end

  def write_comment(node, output); end

  def write_document(node, output); end

  def write_element(node, output); end

  def write_text(node, output); end
end

# If you add a method, keep in mind two things: (1) the first argument will
# always be a list of nodes from which to filter. In the case of context methods
# (such as position), the function should return an array with a value for each
# child in the array. (2) all method calls from
# [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) will have "-" replaced
# with "\_". Therefore, in
# [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html), "local-name()" is
# identical (and actually becomes) "local\_name()"
module REXML::Functions
  INTERNAL_METHODS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # UNTESTED
  def self.boolean(object = _); end

  def self.ceiling(number); end

  def self.compare_language(lang1, lang2); end

  def self.concat(*objects); end

  # Fixed by Mike Stok
  def self.contains(string, test); end

  def self.context=(value); end

  # Returns the size of the given list of nodes.
  def self.count(node_set); end

  # UNTESTED
  def self.false; end

  def self.floor(number); end

  # Helper method.
  def self.get_namespace(node_set = _); end

  # Since [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) is
  # non-validating, this method is not implemented as it requires a DTD
  def self.id(object); end

  # UNTESTED
  def self.lang(language); end

  # Returns the last node of the given list of nodes.
  def self.last; end

  # UNTESTED
  def self.local_name(node_set = _); end

  def self.name(node_set = _); end

  def self.namespace_context; end

  def self.namespace_context=(x); end

  def self.namespace_uri(node_set = _); end

  # UNTESTED
  def self.normalize_space(string = _); end

  # UNTESTED
  def self.not(object); end

  # a string that consists of optional whitespace followed by an optional minus
  # sign followed by a Number followed by whitespace is converted to the IEEE
  # 754 number that is nearest (according to the IEEE 754 round-to-nearest rule)
  # to the mathematical value represented by the string; any other string is
  # converted to NaN
  #
  # boolean true is converted to 1; boolean false is converted to 0
  #
  # a node-set is first converted to a string as if by a call to the string
  # function and then converted in the same way as a string argument
  #
  # an object of a type other than the four basic types is converted to a number
  # in a way that is dependent on that type
  def self.number(object = _); end

  def self.position; end

  def self.processing_instruction(node); end

  def self.round(number); end

  def self.send(name, *args); end

  def self.singleton_method_added(name); end

  # Fixed by Mike Stok
  def self.starts_with(string, test); end

  # A node-set is converted to a string by returning the string-value of the
  # node in the node-set that is first in document order. If the node-set is
  # empty, an empty string is returned.
  #
  # A number is converted to a string as follows
  #
  # NaN is converted to the string NaN
  #
  # positive zero is converted to the string 0
  #
  # negative zero is converted to the string 0
  #
  # positive infinity is converted to the string Infinity
  #
  # negative infinity is converted to the string -Infinity
  #
  # if the number is an integer, the number is represented in decimal form as a
  # Number with no decimal point and no leading zeros, preceded by a minus sign
  # (-) if the number is negative
  #
  # otherwise, the number is represented in decimal form as a Number including a
  # decimal point with at least one digit before the decimal point and at least
  # one digit after the decimal point, preceded by a minus sign (-) if the
  # number is negative; there must be no leading zeros before the decimal point
  # apart possibly from the one required digit immediately before the decimal
  # point; beyond the one required digit after the decimal point there must be
  # as many, but only as many, more digits as are needed to uniquely distinguish
  # the number from all other IEEE 754 numeric values.
  #
  # The boolean false value is converted to the string false. The boolean true
  # value is converted to the string true.
  #
  # An object of a type other than the four basic types is converted to a string
  # in a way that is dependent on that type.
  def self.string(object = _); end

  # UNTESTED
  def self.string_length(string); end

  # A node-set is converted to a string by returning the concatenation of the
  # string-value of each of the children of the node in the node-set that is
  # first in document order. If the node-set is empty, an empty string is
  # returned.
  def self.string_value(o); end

  # Take equal portions of Mike Stok and Sean Russell; mix vigorously, and pour
  # into a tall, chilled glass. Serves 10,000.
  def self.substring(string, start, length = _); end

  # Kouhei fixed this too
  def self.substring_after(string, test); end

  # Kouhei fixed this
  def self.substring_before(string, test); end

  def self.sum(nodes); end

  def self.text; end

  # This is entirely Mike Stok's beast
  def self.translate(string, tr1, tr2); end

  # UNTESTED
  def self.true; end

  def self.variables; end

  def self.variables=(x); end
end

# A Source that wraps an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
# See the Source class for method documentation
class REXML::IOSource < ::REXML::Source
  # block\_size has been deprecated
  def self.new(arg, block_size = _, encoding = _); end

  def consume(pattern); end

  # @return the current line in the source
  def current_line; end

  def empty?; end

  def match(pattern, cons = _); end

  def position; end

  def read; end

  def scan(pattern, cons = _); end
end

# Represents an [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html)
# [`Instruction`](https://docs.ruby-lang.org/en/2.7.0/REXML/Instruction.html);
# IE, <? ... ?> TODO: Add parent arg (3rd arg) to constructor
class REXML::Instruction < ::REXML::Child
  START = T.let(T.unsafe(nil), String)

  STOP = T.let(T.unsafe(nil), String)

  # Constructs a new
  # [`Instruction`](https://docs.ruby-lang.org/en/2.7.0/REXML/Instruction.html)
  # @param target can be one of a number of things. If
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), then the target
  # of this instruction is set to this. If an
  # [`Instruction`](https://docs.ruby-lang.org/en/2.7.0/REXML/Instruction.html),
  # then the
  # [`Instruction`](https://docs.ruby-lang.org/en/2.7.0/REXML/Instruction.html)
  # is shallowly cloned (target and content are copied). @param content Must be
  # either a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), or a
  # Parent. Can only be a Parent if the target argument is a Source. Otherwise,
  # this [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) is set as
  # the content of this instruction.
  def self.new(target, content = _); end

  # @return true if other is an
  # [`Instruction`](https://docs.ruby-lang.org/en/2.7.0/REXML/Instruction.html),
  # and the content and target of the other matches the target and content of
  # this object.
  def ==(other); end

  def clone; end

  # target is the "name" of the
  # [`Instruction`](https://docs.ruby-lang.org/en/2.7.0/REXML/Instruction.html);
  # IE, the "tag" in <?tag ...?> content is everything else.
  def content; end

  # target is the "name" of the
  # [`Instruction`](https://docs.ruby-lang.org/en/2.7.0/REXML/Instruction.html);
  # IE, the "tag" in <?tag ...?> content is everything else.
  def content=(_); end

  def inspect; end

  def node_type; end

  # target is the "name" of the
  # [`Instruction`](https://docs.ruby-lang.org/en/2.7.0/REXML/Instruction.html);
  # IE, the "tag" in <?tag ...?> content is everything else.
  def target; end

  # target is the "name" of the
  # [`Instruction`](https://docs.ruby-lang.org/en/2.7.0/REXML/Instruction.html);
  # IE, the "tag" in <?tag ...?> content is everything else.
  def target=(_); end

  # ## DEPRECATED
  # See the rexml/formatters package
  def write(writer, indent = _, transitive = _, ie_hack = _); end
end

# Adds named attributes to an object.
module REXML::Namespace
  include(::REXML::XMLTokens)

  NAMESPLIT = T.let(T.unsafe(nil), Regexp)

  # The name of the object, valid if set
  def expanded_name; end

  # Fully expand the name, even if the prefix wasn't specified in the source
  # file.
  def fully_expanded_name; end

  # Compares names optionally WITH namespaces
  def has_name?(other, ns = _); end

  # The name of the object, valid if set
  def local_name; end

  # The name of the object, valid if set
  def name; end

  # Sets the name and the expanded name
  def name=(name); end

  # The expanded name of the object, valid if name is set
  def prefix; end

  # The expanded name of the object, valid if name is set
  def prefix=(_); end
end

# Represents a node in the tree. Nodes are never encountered except as
# superclasses of other objects. Nodes have siblings.
module REXML::Node
  # Visit all subnodes of `self` recursively
  def each_recursive(&block); end

  # [`Find`](https://docs.ruby-lang.org/en/2.7.0/Find.html) (and return) first
  # subnode (recursively) for which the block evaluates to true. Returns `nil`
  # if none was found.
  def find_first_recursive(&block); end

  def indent(to, ind); end

  # Returns the position that `self` holds in its parent's array, indexed from
  # 1.
  def index_in_parent; end

  # @return the next sibling (nil if unset)
  def next_sibling_node; end

  def parent?; end

  # @return the previous sibling (nil if unset)
  def previous_sibling_node; end

  # indent
  # :   **DEPRECATED** This parameter is now ignored. See the formatters in the
  #     [`REXML::Formatters`](https://docs.ruby-lang.org/en/2.7.0/REXML/Formatters.html)
  #     package for changing the output style.
  def to_s(indent = _); end
end

class REXML::NotationDecl < ::REXML::Child
  def self.new(name, middle, pub, sys); end

  # This method retrieves the name of the notation.
  #
  # [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) contributed by
  # Henrik Martensson
  def name; end

  def public; end

  def public=(_); end

  def system; end

  def system=(_); end

  def to_s; end

  def write(output, indent = _); end
end

class REXML::Output
  include(::REXML::Encoding)

  def self.new(real_IO, encd = _); end

  def <<(content); end

  def encoding; end

  def to_s; end
end

# A parent has children, and has methods for accessing them. The
# [`Parent`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parent.html) class is
# never encountered except as the superclass for some other object.
class REXML::Parent < ::REXML::Child
  include(::Enumerable)

  Elem = type_member {{fixed: REXML::Child}}

  # Constructor @param parent if supplied, will be set as the parent of this
  # object
  def self.new(parent = _); end

  # Alias for:
  # [`push`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parent.html#method-i-push)
  def <<(object); end

  # Fetches a child at a given index @param index the
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) index of the
  # child to fetch
  def [](index); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) an index entry. See
  # [`Array.[]=`](https://docs.ruby-lang.org/en/2.7.0/MakeMakefile.html#method-c-5B-5D-3D)
  # @param index the index of the element to set @param opt either the object to
  # set, or an [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html)
  # length @param child if opt is an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html), this is the
  # child to set @return the parent (self)
  def []=(*args); end

  # Also aliased as:
  # [`push`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parent.html#method-i-push)
  def add(object); end

  # Alias for:
  # [`to_a`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parent.html#method-i-to_a)
  def children; end

  # Deeply clones this object. This creates a complete duplicate of this
  # [`Parent`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parent.html), including
  # all descendants.
  def deep_clone; end

  def delete(object); end

  def delete_at(index); end

  def delete_if(&block); end

  # Also aliased as:
  # [`each_child`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parent.html#method-i-each_child)
  def each(&block); end

  # Alias for:
  # [`each`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parent.html#method-i-each)
  def each_child(&block); end

  def each_index(&block); end

  # Fetches the index of a given child @param child the child to get the index
  # of @return the index of the child, or nil if the object is not a child of
  # this parent.
  def index(child); end

  # Inserts an child after another child @param child1 this is either an xpath
  # or an Element. If an Element, child2 will be inserted after child1 in the
  # child list of the parent. If an xpath, child2 will be inserted after the
  # first child to match the xpath. @param child2 the child to insert @return
  # the parent (self)
  def insert_after(child1, child2); end

  # Inserts an child before another child @param child1 this is either an xpath
  # or an Element. If an Element, child2 will be inserted before child1 in the
  # child list of the parent. If an xpath, child2 will be inserted before the
  # first child to match the xpath. @param child2 the child to insert @return
  # the parent (self)
  def insert_before(child1, child2); end

  # Alias for:
  # [`size`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parent.html#method-i-size)
  def length; end

  def parent?; end

  # Also aliased as:
  # [`<<`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parent.html#method-i-3C-3C)
  #
  # Alias for:
  # [`add`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parent.html#method-i-add)
  def push(object); end

  # Replaces one child with another, making sure the nodelist is correct @param
  # to\_replace the child to replace (must be a Child) @param replacement the
  # child to insert into the nodelist (must be a Child)
  def replace_child(to_replace, replacement); end

  # @return the number of children of this parent
  #
  # Also aliased as:
  # [`length`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parent.html#method-i-length)
  def size; end

  # Also aliased as:
  # [`children`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parent.html#method-i-children)
  def to_a; end

  def unshift(object); end
end

class REXML::ParseException < ::RuntimeError
  def self.new(message, source = _, parser = _, exception = _); end

  def context; end

  def continued_exception; end

  def continued_exception=(_); end

  def line; end

  def parser; end

  def parser=(_); end

  def position; end

  def source; end

  def source=(_); end

  def to_s; end
end

module REXML::Parsers; end

# # Using the Pull Parser
# *This API is experimental, and subject to change.*
#
# ```ruby
# parser = PullParser.new( "<a>text<b att='val'/>txet</a>" )
# while parser.has_next?
#   res = parser.next
#   puts res[1]['att'] if res.start_tag? and res[0] == 'b'
# end
# ```
#
# See the PullEvent class for information on the content of the results. The
# data is identical to the arguments passed for the various events to the
# StreamListener API.
#
# Notice that:
#
# ```ruby
# parser = PullParser.new( "<a>BAD DOCUMENT" )
# while parser.has_next?
#   res = parser.next
#   raise res[1] if res.error?
# end
# ```
#
# Nat Price gave me some good ideas for the API.
class REXML::Parsers::BaseParser
  ATTDEF = T.let(T.unsafe(nil), String)

  ATTDEF_RE = T.let(T.unsafe(nil), Regexp)

  ATTLISTDECL_PATTERN = T.let(T.unsafe(nil), Regexp)

  ATTLISTDECL_START = T.let(T.unsafe(nil), Regexp)

  ATTRIBUTE_PATTERN = T.let(T.unsafe(nil), Regexp)

  ATTTYPE = T.let(T.unsafe(nil), String)

  ATTVALUE = T.let(T.unsafe(nil), String)

  CDATA_END = T.let(T.unsafe(nil), Regexp)

  CDATA_PATTERN = T.let(T.unsafe(nil), Regexp)

  CDATA_START = T.let(T.unsafe(nil), Regexp)

  CLOSE_MATCH = T.let(T.unsafe(nil), Regexp)

  COMBININGCHAR = T.let(T.unsafe(nil), String)

  COMMENT_PATTERN = T.let(T.unsafe(nil), Regexp)

  COMMENT_START = T.let(T.unsafe(nil), Regexp)

  DEFAULTDECL = T.let(T.unsafe(nil), String)

  DEFAULT_ENTITIES = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  DIGIT = T.let(T.unsafe(nil), String)

  DOCTYPE_END = T.let(T.unsafe(nil), Regexp)

  DOCTYPE_PATTERN = T.let(T.unsafe(nil), Regexp)

  DOCTYPE_START = T.let(T.unsafe(nil), Regexp)

  ELEMENTDECL_PATTERN = T.let(T.unsafe(nil), Regexp)

  ELEMENTDECL_START = T.let(T.unsafe(nil), Regexp)

  ENCODING = T.let(T.unsafe(nil), Regexp)

  ENTITYDECL = T.let(T.unsafe(nil), Regexp)

  ENTITYDEF = T.let(T.unsafe(nil), String)

  ENTITYVALUE = T.let(T.unsafe(nil), String)

  ENTITY_START = T.let(T.unsafe(nil), Regexp)

  ENUMERATEDTYPE = T.let(T.unsafe(nil), String)

  ENUMERATION = T.let(T.unsafe(nil), String)

  EREFERENCE = T.let(T.unsafe(nil), Regexp)

  EXTENDER = T.let(T.unsafe(nil), String)

  EXTERNALID = T.let(T.unsafe(nil), String)

  GEDECL = T.let(T.unsafe(nil), String)

  IDENTITY = T.let(T.unsafe(nil), Regexp)

  INSTRUCTION_PATTERN = T.let(T.unsafe(nil), Regexp)

  INSTRUCTION_START = T.let(T.unsafe(nil), Regexp)

  LETTER = T.let(T.unsafe(nil), String)

  MISSING_ATTRIBUTE_QUOTES = T.let(T.unsafe(nil), Regexp)

  NAME = T.let(T.unsafe(nil), String)

  NAMECHAR = T.let(T.unsafe(nil), String)

  NAME_STR = T.let(T.unsafe(nil), String)

  NCNAME_STR = T.let(T.unsafe(nil), String)

  NDATADECL = T.let(T.unsafe(nil), String)

  NMTOKEN = T.let(T.unsafe(nil), String)

  NMTOKENS = T.let(T.unsafe(nil), String)

  NOTATIONDECL_START = T.let(T.unsafe(nil), Regexp)

  NOTATIONTYPE = T.let(T.unsafe(nil), String)

  PEDECL = T.let(T.unsafe(nil), String)

  PEDEF = T.let(T.unsafe(nil), String)

  PEREFERENCE = T.let(T.unsafe(nil), String)

  # Entity constants
  PUBIDCHAR = T.let(T.unsafe(nil), String)

  PUBIDLITERAL = T.let(T.unsafe(nil), String)

  PUBLIC = T.let(T.unsafe(nil), Regexp)

  REFERENCE = T.let(T.unsafe(nil), String)

  REFERENCE_RE = T.let(T.unsafe(nil), Regexp)

  STANDALONE = T.let(T.unsafe(nil), Regexp)

  SYSTEM = T.let(T.unsafe(nil), Regexp)

  SYSTEMENTITY = T.let(T.unsafe(nil), Regexp)

  SYSTEMLITERAL = T.let(T.unsafe(nil), String)

  TAG_MATCH = T.let(T.unsafe(nil), Regexp)

  TEXT_PATTERN = T.let(T.unsafe(nil), Regexp)

  # Just for backward compatibility. For example, kramdown uses this. It's not
  # used in [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html).
  UNAME_STR = T.let(T.unsafe(nil), String)

  VERSION = T.let(T.unsafe(nil), Regexp)

  XMLDECL_PATTERN = T.let(T.unsafe(nil), Regexp)

  XMLDECL_START = T.let(T.unsafe(nil), Regexp)

  def self.new(source); end

  def add_listener(listener); end

  # Returns true if there are no more events
  def empty?; end

  def entity(reference, entities); end

  # Returns true if there are more events. Synonymous with !empty?
  def has_next?; end

  # Escapes all possible entities
  def normalize(input, entities = _, entity_filter = _); end

  # Peek at the `depth` event in the stack. The first element on the stack is at
  # depth 0. If `depth` is -1, will parse to the end of the input stream and
  # return the last event, which is always :end\_document. Be aware that this
  # causes the stream to be parsed up to the `depth` event, so you can
  # effectively pre-parse the entire document (pull the entire thing into
  # memory) using this method.
  def peek(depth = _); end

  def position; end

  # Returns the next event. This is a `PullEvent` object.
  def pull; end

  def source; end

  def stream=(source); end

  # Unescapes all possible entities
  def unnormalize(string, entities = _, filter = _); end

  # Push an event back on the head of the stream. This method has
  # (theoretically) infinite depth.
  def unshift(token); end
end

class REXML::Parsers::StreamParser
  def self.new(source, listener); end

  def add_listener(listener); end

  def parse; end
end

class REXML::Parsers::TreeParser
  def self.new(source, build_context = _); end

  def add_listener(listener); end

  def parse; end
end

# You don't want to use this class. Really. Use XPath, which is a wrapper for
# this class. Believe me. You don't want to poke around in here. There is
# strange, dark magic at work in this code. Beware. Go back!  Go back while you
# still can!
class REXML::Parsers::XPathParser
  include(::REXML::XMLTokens)

  # [`RelativeLocationPath`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parsers/XPathParser.html#method-i-RelativeLocationPath)
  #
  # ```
  # |                                                    Step
  #   | (AXIS_NAME '::' | '@' | '')                     AxisSpecifier
  #     NodeTest
  #       Predicate
  #   | '.' | '..'                                      AbbreviatedStep
  # |  RelativeLocationPath '/' Step
  # | RelativeLocationPath '//' Step
  # ```
  AXIS = T.let(T.unsafe(nil), Regexp)

  LITERAL = T.let(T.unsafe(nil), Regexp)

  NCNAMETEST = T.let(T.unsafe(nil), Regexp)

  NODE_TYPE = T.let(T.unsafe(nil), Regexp)

  NT = T.let(T.unsafe(nil), Regexp)

  NUMBER = T.let(T.unsafe(nil), Regexp)

  PI = T.let(T.unsafe(nil), Regexp)

  QNAME = T.let(T.unsafe(nil), Regexp)

  # |
  # [`VARIABLE_REFERENCE`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parsers/XPathParser.html#VARIABLE_REFERENCE)
  # | '(' expr ')' |
  # [`LITERAL`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parsers/XPathParser.html#LITERAL)
  # |
  # [`NUMBER`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parsers/XPathParser.html#NUMBER)
  # |
  # [`FunctionCall`](https://docs.ruby-lang.org/en/2.7.0/REXML/Parsers/XPathParser.html#method-i-FunctionCall)
  VARIABLE_REFERENCE = T.let(T.unsafe(nil), Regexp)

  def abbreviate(path); end

  def expand(path); end

  def namespaces=(namespaces); end

  def parse(path); end

  def predicate(path); end

  def predicate_to_string(path, &block); end
end

class REXML::QuickPath
  include(::REXML::XMLTokens)
  include(::REXML::Functions)

  # A base [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) object to be
  # used when initializing a default empty namespaces set.
  EMPTY_HASH = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  OPERAND_ = T.let(T.unsafe(nil), String)

  def self.attribute(name); end

  def self.axe(elements, axe_name, rest); end

  def self.each(element, path, namespaces = _, &block); end

  # Given an array of nodes it filters the array based on the path. The result
  # is that when this method returns, the array will contain elements which
  # match the path
  def self.filter(elements, path); end

  def self.first(element, path, namespaces = _); end

  def self.function(elements, fname, rest); end

  def self.match(element, path, namespaces = _); end

  def self.method_missing(id, *args); end

  def self.name; end

  def self.parse_args(element, string); end

  # A predicate filters a node-set with respect to an axis to produce a new
  # node-set. For each node in the node-set to be filtered, the PredicateExpr is
  # evaluated with that node as the context node, with the number of nodes in
  # the node-set as the context size, and with the proximity position of the
  # node in the node-set with respect to the axis as the context position; if
  # PredicateExpr evaluates to true for that node, the node is included in the
  # new node-set; otherwise, it is not included.
  #
  # A PredicateExpr is evaluated by evaluating the Expr and converting the
  # result to a boolean. If the result is a number, the result will be converted
  # to true if the number is equal to the context position and will be converted
  # to false otherwise; if the result is not a number, then the result will be
  # converted as if by a call to the boolean function. Thus a location path
  # [para](3) is equivalent to [para](position()=3).
  def self.predicate(elements, path); end
end

# A template for stream parser listeners. Note that the declarations
# (attlistdecl, elementdecl, etc) are trivially processed;
# [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) doesn't yet handle
# doctype entity declarations, so you have to parse them out yourself.
# ### Missing methods from SAX2
#
# ```ruby
# ignorable_whitespace
# ```
#
# ### Methods extending SAX2
# `WARNING` These methods are certainly going to change, until DTDs are fully
# supported. Be aware of this.
#
# ```ruby
# start_document
# end_document
# doctype
# elementdecl
# attlistdecl
# entitydecl
# notationdecl
# cdata
# xmldecl
# comment
# ```
module REXML::SAX2Listener
  # If a doctype includes an ATTLIST declaration, it will cause this method to
  # be called. The content is the declaration itself, unparsed. EG, <!ATTLIST el
  # attr CDATA #REQUIRED> will come to this method as "el attr CDATA #REQUIRED".
  # This is the same for all of the .\*decl methods.
  def attlistdecl(element, pairs, contents); end

  # Called when <![CDATA[ ... ]]> is encountered in a document. @p content "..."
  def cdata(content); end

  def characters(text); end

  # Called when a comment is encountered. @p comment The content of the comment
  def comment(comment); end

  # Handles a doctype declaration. Any attributes of the doctype which are not
  # supplied will be nil. # EG, <!DOCTYPE me PUBLIC "foo" "bar"> @p name the
  # name of the doctype; EG, "me" @p pub\_sys "PUBLIC", "SYSTEM", or nil. EG,
  # "PUBLIC" @p long\_name the supplied long name, or nil. EG, "foo" @p uri the
  # uri of the doctype, or nil. EG, "bar"
  def doctype(name, pub_sys, long_name, uri); end

  # <!ELEMENT ...>
  def elementdecl(content); end

  def end_document; end

  def end_element(uri, localname, qname); end

  def end_prefix_mapping(prefix); end

  # <!ENTITY ...> The argument passed to this method is an array of the entity
  # declaration. It can be in a number of formats, but in general it returns
  # (example, result):
  #
  # ```
  # <!ENTITY % YN '"Yes"'>
  # ["%", "YN", "\"Yes\""]
  # <!ENTITY % YN 'Yes'>
  # ["%", "YN", "Yes"]
  # <!ENTITY WhatHeSaid "He said %YN;">
  # ["WhatHeSaid", "He said %YN;"]
  # <!ENTITY open-hatch SYSTEM "http://www.textuality.com/boilerplate/OpenHatch.xml">
  # ["open-hatch", "SYSTEM", "http://www.textuality.com/boilerplate/OpenHatch.xml"]
  # <!ENTITY open-hatch PUBLIC "-//Textuality//TEXT Standard open-hatch boilerplate//EN" "http://www.textuality.com/boilerplate/OpenHatch.xml">
  # ```
  #
  # "open-hatch", "PUBLIC", "-//Textuality//TEXT Standard open-hatch boilerplate//EN", "http://www.textuality.com/boilerplate/OpenHatch.xml"
  # :   <!ENTITY hatch-pic SYSTEM "../grafix/OpenHatch.gif" NDATA gif>
  #     "hatch-pic", "SYSTEM", "../grafix/OpenHatch.gif", "NDATA", "gif"
  # :
  def entitydecl(declaration); end

  # <!NOTATION ...>
  def notationdecl(name, public_or_system, public_id, system_id); end

  def processing_instruction(target, data); end

  def progress(position); end

  def start_document; end

  def start_element(uri, localname, qname, attributes); end

  def start_prefix_mapping(prefix, uri); end

  # Called when an [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) PI is
  # encountered in the document. EG: <?xml version="1.0" encoding="utf"?> @p
  # version the version attribute value. EG, "1.0" @p encoding the encoding
  # attribute value, or nil. EG, "utf" @p standalone the standalone attribute
  # value, or nil. EG, nil @p spaced the declaration is followed by a line break
  def xmldecl(version, encoding, standalone); end
end

module REXML::Security
  # Get the entity expansion limit. By default the limit is set to 10000.
  def self.entity_expansion_limit; end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the entity expansion
  # limit. By default the limit is set to 10000.
  def self.entity_expansion_limit=(val); end

  # Get the entity expansion limit. By default the limit is set to 10240.
  def self.entity_expansion_text_limit; end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the entity expansion
  # limit. By default the limit is set to 10240.
  def self.entity_expansion_text_limit=(val); end
end

# A [`Source`](https://docs.ruby-lang.org/en/2.7.0/REXML/Source.html) can be
# searched for patterns, and wraps buffers and other objects and provides
# consumption of text
class REXML::Source
  include(::REXML::Encoding)

  # Constructor @param arg must be a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), and should be a
  # valid [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) document @param
  # encoding if non-null, sets the encoding of the source to this value,
  # overriding all encoding detection
  def self.new(arg, encoding = _); end

  # The current buffer (what we're going to read next)
  def buffer; end

  def consume(pattern); end

  # @return the current line in the source
  def current_line; end

  # @return true if the
  # [`Source`](https://docs.ruby-lang.org/en/2.7.0/REXML/Source.html) is
  # exhausted
  def empty?; end

  def encoding; end

  # Inherited from
  # [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) Overridden
  # to support optimized en/decoding
  def encoding=(enc); end

  # The line number of the last consumed text
  def line; end

  def match(pattern, cons = _); end

  def match_to(char, pattern); end

  def match_to_consume(char, pattern); end

  def position; end

  def read; end

  # Scans the source for a given pattern. Note, that this is not your usual
  # scan() method. For one thing, the pattern argument has some requirements;
  # for another, the source can be consumed. You can easily confuse this method.
  # Originally, the patterns were easier to construct and this method more
  # robust, because this method generated search regexps on the fly; however,
  # this was computationally expensive and slowed down the entire
  # [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) package
  # considerably, since this is by far the most commonly called method. @param
  # pattern must be a
  # [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html), and must be in
  # the form of /^s\*(#{your pattern, with no groups})(.\*)/. The first group
  # will be returned; the second group is used if the consume flag is set.
  # @param consume if true, the pattern returned will be consumed, leaving
  # everything after it in the
  # [`Source`](https://docs.ruby-lang.org/en/2.7.0/REXML/Source.html). @return
  # the pattern, if found, or nil if the
  # [`Source`](https://docs.ruby-lang.org/en/2.7.0/REXML/Source.html) is empty
  # or the pattern is not found.
  def scan(pattern, cons = _); end
end

# Generates Source-s. USE THIS CLASS.
class REXML::SourceFactory
  # Generates a Source object @param arg Either a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), or an
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) @return a Source, or nil
  # if a bad argument was given
  def self.create_from(arg); end
end

# A template for stream parser listeners. Note that the declarations
# (attlistdecl, elementdecl, etc) are trivially processed;
# [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) doesn't yet handle
# doctype entity declarations, so you have to parse them out yourself.
module REXML::StreamListener
  # If a doctype includes an ATTLIST declaration, it will cause this method to
  # be called. The content is the declaration itself, unparsed. EG, <!ATTLIST el
  # attr CDATA #REQUIRED> will come to this method as "el attr CDATA #REQUIRED".
  # This is the same for all of the .\*decl methods.
  def attlistdecl(element_name, attributes, raw_content); end

  # Called when <![CDATA[ ... ]]> is encountered in a document. @p content "..."
  def cdata(content); end

  # Called when a comment is encountered. @p comment The content of the comment
  def comment(comment); end

  # Handles a doctype declaration. Any attributes of the doctype which are not
  # supplied will be nil. # EG, <!DOCTYPE me PUBLIC "foo" "bar"> @p name the
  # name of the doctype; EG, "me" @p pub\_sys "PUBLIC", "SYSTEM", or nil. EG,
  # "PUBLIC" @p long\_name the supplied long name, or nil. EG, "foo" @p uri the
  # uri of the doctype, or nil. EG, "bar"
  def doctype(name, pub_sys, long_name, uri); end

  # Called when the doctype is done
  def doctype_end; end

  # <!ELEMENT ...>
  def elementdecl(content); end

  # Called when %foo; is encountered in a doctype declaration. @p content "foo"
  def entity(content); end

  # <!ENTITY ...> The argument passed to this method is an array of the entity
  # declaration. It can be in a number of formats, but in general it returns
  # (example, result):
  #
  # ```
  # <!ENTITY % YN '"Yes"'>
  # ["YN", "\"Yes\"", "%"]
  # <!ENTITY % YN 'Yes'>
  # ["YN", "Yes", "%"]
  # <!ENTITY WhatHeSaid "He said %YN;">
  # ["WhatHeSaid", "He said %YN;"]
  # <!ENTITY open-hatch SYSTEM "http://www.textuality.com/boilerplate/OpenHatch.xml">
  # ["open-hatch", "SYSTEM", "http://www.textuality.com/boilerplate/OpenHatch.xml"]
  # <!ENTITY open-hatch PUBLIC "-//Textuality//TEXT Standard open-hatch boilerplate//EN" "http://www.textuality.com/boilerplate/OpenHatch.xml">
  # ["open-hatch", "PUBLIC", "-//Textuality//TEXT Standard open-hatch boilerplate//EN", "http://www.textuality.com/boilerplate/OpenHatch.xml"]
  # <!ENTITY hatch-pic SYSTEM "../grafix/OpenHatch.gif" NDATA gif>
  # ["hatch-pic", "SYSTEM", "../grafix/OpenHatch.gif", "gif"]
  # ```
  def entitydecl(content); end

  # Called when an instruction is encountered. EG: <?xsl sheet='foo'?> @p name
  # the instruction name; in the example, "xsl" @p instruction the rest of the
  # instruction. In the example, "sheet='foo'"
  def instruction(name, instruction); end

  # <!NOTATION ...>
  def notationdecl(content); end

  # Called when the end tag is reached. In the case of <tag/>,
  # [`tag_end`](https://docs.ruby-lang.org/en/2.7.0/REXML/StreamListener.html#method-i-tag_end)
  # will be called immediately after
  # [`tag_start`](https://docs.ruby-lang.org/en/2.7.0/REXML/StreamListener.html#method-i-tag_start)
  # @p the name of the tag
  def tag_end(name); end

  # Called when a tag is encountered. @p name the tag name @p attrs an array of
  # arrays of attribute/value pairs, suitable for use with assoc or rassoc. IE,
  # <tag attr1="value1" attr2="value2"> will result in
  # [`tag_start`](https://docs.ruby-lang.org/en/2.7.0/REXML/StreamListener.html#method-i-tag_start)(
  # "tag", # [["[attr1","value1"],]("attr2","value2")])
  def tag_start(name, attrs); end

  # Called when text is encountered in the document @p text the text content.
  def text(text); end

  # Called when an [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) PI is
  # encountered in the document. EG: <?xml version="1.0" encoding="utf"?> @p
  # version the version attribute value. EG, "1.0" @p encoding the encoding
  # attribute value, or nil. EG, "utf" @p standalone the standalone attribute
  # value, or nil. EG, nil
  def xmldecl(version, encoding, standalone); end
end

class REXML::SyncEnumerator
  include(::Enumerable)

  Elem = type_member {{fixed: REXML::Element}}

  # Creates a new
  # [`SyncEnumerator`](https://docs.ruby-lang.org/en/2.6.0/REXML/SyncEnumerator.html)
  # which enumerates rows of given
  # [`Enumerable`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html) objects.
  def self.new(*enums); end

  # Enumerates rows of the
  # [`Enumerable`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html) objects.
  def each; end

  # Returns the number of enumerated
  # [`Enumerable`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html) objects,
  # i.e. the size of each row.
  def length; end

  # Returns the number of enumerated
  # [`Enumerable`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html) objects,
  # i.e. the size of each row.
  def size; end
end

# Represents text nodes in an
# [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) document
class REXML::Text < ::REXML::Child
  include(::Comparable)

  EREFERENCE = T.let(T.unsafe(nil), Regexp)

  NEEDS_A_SECOND_CHECK = T.let(T.unsafe(nil), Regexp)

  NUMERICENTITY = T.let(T.unsafe(nil), Regexp)

  REFERENCE = T.let(T.unsafe(nil), Regexp)

  SETUTITSBUS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Characters which are substituted in written strings
  SLAICEPS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # The order in which the substitutions occur
  SPECIALS = T.let(T.unsafe(nil), T::Array[T.untyped])

  SUBSTITUTES = T.let(T.unsafe(nil), T::Array[T.untyped])

  VALID_CHAR = T.let(T.unsafe(nil), T::Array[T.untyped])

  VALID_XML_CHARS = T.let(T.unsafe(nil), Regexp)

  # Constructor `arg` if a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), the content is
  # set to the [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). If a
  # [`Text`](https://docs.ruby-lang.org/en/2.7.0/REXML/Text.html), the object is
  # shallowly cloned.
  #
  # `respect_whitespace` (boolean, false) if true, whitespace is respected
  #
  # `parent` (nil) if this is a Parent object, the parent will be set to this.
  #
  # `raw` (nil) This argument can be given three values. If true, then the value
  # of used to construct this object is expected to contain no unescaped
  # [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) markup, and
  # [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) will not change
  # the text. If this value is false, the string may contain any characters, and
  # [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) will escape any
  # and all defined entities whose values are contained in the text. If this
  # value is nil (the default), then the raw value of the parent will be used as
  # the raw value for this node. If there is no raw value for the parent, and no
  # value is supplied, the default is false. Use this field if you have entities
  # defined for some text, and you don't want
  # [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) to escape that
  # text in output.
  #
  # ```ruby
  # Text.new( "<&", false, nil, false ) #-> "&lt;&amp;"
  # Text.new( "&lt;&amp;", false, nil, false ) #-> "&amp;lt;&amp;amp;"
  # Text.new( "<&", false, nil, true )  #-> Parse exception
  # Text.new( "&lt;&amp;", false, nil, true )  #-> "&lt;&amp;"
  # # Assume that the entity "s" is defined to be "sean"
  # # and that the entity    "r" is defined to be "russell"
  # Text.new( "sean russell" )          #-> "&s; &r;"
  # Text.new( "sean russell", false, nil, true ) #-> "sean russell"
  # ```
  #
  # `entity_filter` (nil) This can be an array of entities to match in the
  # supplied text. This argument is only useful if `raw` is set to false.
  #
  # ```ruby
  # Text.new( "sean russell", false, nil, false, ["s"] ) #-> "&s; russell"
  # Text.new( "sean russell", false, nil, true, ["s"] ) #-> "sean russell"
  # ```
  #
  # In the last example, the `entity_filter` argument is ignored.
  #
  # `illegal` INTERNAL USE ONLY
  def self.new(arg, respect_whitespace = _, parent = _, raw = _, entity_filter = _, illegal = _); end

  # Appends text to this text node. The text is appended in the `raw` mode of
  # this text node.
  #
  # `returns` the text itself to enable method chain like 'text << "XXX" <<
  # "YYY"'.
  def <<(to_append); end

  # `other` a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) or a
  # [`Text`](https://docs.ruby-lang.org/en/2.7.0/REXML/Text.html) `returns` the
  # result of
  # ([`to_s`](https://docs.ruby-lang.org/en/2.7.0/REXML/Text.html#method-i-to_s)
  # <=> arg.to\_s)
  def <=>(other); end

  def clone; end

  def doctype; end

  def empty?; end

  def indent_text(string, level = _, style = _, indentfirstline = _); end

  def inspect; end

  def node_type; end

  def parent=(parent); end

  # If `raw` is true, then
  # [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) leaves the value
  # alone
  def raw; end

  # If `raw` is true, then
  # [`REXML`](https://docs.ruby-lang.org/en/2.7.0/REXML.html) leaves the value
  # alone
  def raw=(_); end

  # Returns the string value of this text node. This string is always escaped,
  # meaning that it is a valid
  # [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) text node string, and
  # all entities that can be escaped, have been inserted. This method respects
  # the entity filter set in the constructor.
  #
  # ```ruby
  # # Assume that the entity "s" is defined to be "sean", and that the
  # # entity "r" is defined to be "russell"
  # t = Text.new( "< & sean russell", false, nil, false, ['s'] )
  # t.to_s   #-> "&lt; &amp; &s; russell"
  # t = Text.new( "< & &s; russell", false, nil, false )
  # t.to_s   #-> "&lt; &amp; &s; russell"
  # u = Text.new( "sean russell", false, nil, true )
  # u.to_s   #-> "sean russell"
  # ```
  def to_s; end

  # Returns the string value of this text. This is the text without entities, as
  # it might be used programmatically, or printed to the console. This ignores
  # the 'raw' attribute setting, and any entity\_filter.
  #
  # ```ruby
  # # Assume that the entity "s" is defined to be "sean", and that the
  # # entity "r" is defined to be "russell"
  # t = Text.new( "< & sean russell", false, nil, false, ['s'] )
  # t.value   #-> "< & sean russell"
  # t = Text.new( "< & &s; russell", false, nil, false )
  # t.value   #-> "< & sean russell"
  # u = Text.new( "sean russell", false, nil, true )
  # u.value   #-> "sean russell"
  # ```
  def value; end

  # Sets the contents of this text node. This expects the text to be
  # unnormalized. It returns self.
  #
  # ```ruby
  # e = Element.new( "a" )
  # e.add_text( "foo" )   # <a>foo</a>
  # e[0].value = "bar"    # <a>bar</a>
  # e[0].value = "<a>"    # <a>&lt;a&gt;</a>
  # ```
  def value=(val); end

  def wrap(string, width, addnewline = _); end

  # ## DEPRECATED
  # See
  # [`REXML::Formatters`](https://docs.ruby-lang.org/en/2.7.0/REXML/Formatters.html)
  def write(writer, indent = _, transitive = _, ie_hack = _); end

  # Writes out text, substituting special characters beforehand. `out` A
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html),
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html), or any other object
  # supporting <<( [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) )
  # `input` the text to substitute and the write out
  #
  # ```ruby
  # z=utf8.unpack("U*")
  # ascOut=""
  # z.each{|r|
  #   if r <  0x100
  #     ascOut.concat(r.chr)
  #   else
  #     ascOut.concat(sprintf("&#x%x;", r))
  #   end
  # }
  # puts ascOut
  # ```
  def write_with_substitution(out, input); end

  # FIXME This probably won't work properly
  def xpath; end

  # check for illegal characters
  def self.check(string, pattern, doctype); end

  def self.expand(ref, doctype, filter); end

  # Escapes all possible entities
  def self.normalize(input, doctype = _, entity_filter = _); end

  # Reads text, substituting entities
  def self.read_with_substitution(input, illegal = _); end

  # Unescapes all possible entities
  def self.unnormalize(string, doctype = _, filter = _, illegal = _); end
end

class REXML::UndefinedNamespaceException < ::REXML::ParseException
  def self.new(prefix, source, parser); end
end

module REXML::Validation; end

class REXML::Validation::ValidationException < ::RuntimeError
  def self.new(msg); end
end

REXML::Version = T.let(T.unsafe(nil), String)

# NEEDS DOCUMENTATION
class REXML::XMLDecl < ::REXML::Child
  include(::REXML::Encoding)

  DEFAULT_ENCODING = T.let(T.unsafe(nil), String)

  DEFAULT_STANDALONE = T.let(T.unsafe(nil), String)

  DEFAULT_VERSION = T.let(T.unsafe(nil), String)

  START = T.let(T.unsafe(nil), String)

  STOP = T.let(T.unsafe(nil), String)

  def self.new(version = _, encoding = _, standalone = _); end

  def ==(other); end

  def clone; end

  def dowrite; end

  # Also aliased as:
  # [`old_enc=`](https://docs.ruby-lang.org/en/2.7.0/REXML/XMLDecl.html#method-i-old_enc-3D)
  def encoding=(enc); end

  def inspect; end

  def node_type; end

  def nowrite; end

  # Alias for:
  # [`encoding=`](https://docs.ruby-lang.org/en/2.7.0/REXML/XMLDecl.html#method-i-encoding-3D)
  def old_enc=(encoding); end

  def stand_alone?; end

  def standalone; end

  def standalone=(_); end

  def version; end

  def version=(_); end

  # indent
  # :   Ignored. There must be no whitespace before an
  #     [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) declaration
  # transitive
  # :   Ignored
  # ie\_hack
  # :   Ignored
  def write(writer, indent = _, transitive = _, ie_hack = _); end

  def writeencoding; end

  def writethis; end

  def xmldecl(version, encoding, standalone); end

  # Only use this if you do not want the
  # [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) declaration to be
  # written; this object is ignored by the
  # [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) writer. Otherwise,
  # instantiate your own
  # [`XMLDecl`](https://docs.ruby-lang.org/en/2.7.0/REXML/XMLDecl.html) and add
  # it to the document.
  #
  # Note that [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) 1.1
  # documents **must** include an
  # [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) declaration
  def self.default; end
end

# Defines a number of tokens used for parsing
# [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html). Not for general
# consumption.
module REXML::XMLTokens
  NAME = T.let(T.unsafe(nil), String)

  NAMECHAR = T.let(T.unsafe(nil), String)

  NAME_CHAR = T.let(T.unsafe(nil), String)

  NAME_START_CHAR = T.let(T.unsafe(nil), String)

  NAME_STR = T.let(T.unsafe(nil), String)

  NCNAME_STR = T.let(T.unsafe(nil), String)

  NMTOKEN = T.let(T.unsafe(nil), String)

  NMTOKENS = T.let(T.unsafe(nil), String)

  REFERENCE = T.let(T.unsafe(nil), String)
end

# Wrapper class. Use this class to access the
# [`XPath`](https://docs.ruby-lang.org/en/2.7.0/REXML/XPath.html) functions.
class REXML::XPath
  include(::REXML::Functions)

  # A base [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) object,
  # supposing to be used when initializing a default empty namespaces set, but
  # is currently unused. TODO: either set the namespaces=EMPTY\_HASH, or
  # deprecate this.
  EMPTY_HASH = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # Iterates over nodes that match the given path, calling the supplied block
  # with the match.
  # element
  # :   The context element
  # path
  # :   The xpath to search for. If not supplied or nil, defaults to '\*'
  # namespaces
  # :   If supplied, a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
  #     which defines a namespace mapping
  # variables
  # :   If supplied, a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
  #     which maps $variables in the query to values. This can be used to avoid
  #     [`XPath`](https://docs.ruby-lang.org/en/2.7.0/REXML/XPath.html)
  #     injection attacks or to automatically handle escaping string values.
  #
  #
  # ```
  # XPath.each( node ) { |el| ... }
  # XPath.each( node, '/*[@attr='v']' ) { |el| ... }
  # XPath.each( node, 'ancestor::x' ) { |el| ... }
  # XPath.each( node, '/book/publisher/text()=$publisher', {}, {"publisher"=>"O'Reilly"}) \
  #   {|el| ... }
  # ```
  def self.each(element, path = _, namespaces = _, variables = _, options = _, &block); end

  # Finds and returns the first node that matches the supplied xpath.
  # element
  # :   The context element
  # path
  # :   The xpath to search for. If not supplied or nil, returns the first node
  #     matching '\*'.
  # namespaces
  # :   If supplied, a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
  #     which defines a namespace mapping.
  # variables
  # :   If supplied, a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
  #     which maps $variables in the query to values. This can be used to avoid
  #     [`XPath`](https://docs.ruby-lang.org/en/2.7.0/REXML/XPath.html)
  #     injection attacks or to automatically handle escaping string values.
  #
  #
  # ```
  # XPath.first( node )
  # XPath.first( doc, "//b"} )
  # XPath.first( node, "a/x:b", { "x"=>"http://doofus" } )
  # XPath.first( node, '/book/publisher/text()=$publisher', {}, {"publisher"=>"O'Reilly"})
  # ```
  def self.first(element, path = nil, namespaces = nil, variables = {}, options = {}, &block); end

  # Iterates over nodes that match the given path, calling the supplied block
  # with the match.
  # element
  # :   The context element
  # path
  # :   The xpath to search for. If not supplied or nil, defaults to '\*'
  # namespaces
  # :   If supplied, a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
  #     which defines a namespace mapping
  # variables
  # :   If supplied, a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
  #     which maps $variables in the query to values. This can be used to avoid
  #     [`XPath`](https://docs.ruby-lang.org/en/2.7.0/REXML/XPath.html)
  #     injection attacks or to automatically handle escaping string values.
  #
  #
  # ```
  # XPath.each( node ) { |el| ... }
  # XPath.each( node, '/*[@attr='v']' ) { |el| ... }
  # XPath.each( node, 'ancestor::x' ) { |el| ... }
  # XPath.each( node, '/book/publisher/text()=$publisher', {}, {"publisher"=>"O'Reilly"}) \
  #   {|el| ... }
  # ```
  def self.each(element, path = nil, namespaces = nil, variables = {}, options = {}, &block); end

  # Returns an array of nodes matching a given
  # [`XPath`](https://docs.ruby-lang.org/en/2.7.0/REXML/XPath.html).
  def self.match(element, path = nil, namespaces = nil, variables = {}, options = {}); end
end

# You don't want to use this class. Really. Use XPath, which is a wrapper for
# this class. Believe me. You don't want to poke around in here. There is
# strange, dark magic at work in this code. Beware. Go back!  Go back while you
# still can!
class REXML::XPathParser
  include(::REXML::XMLTokens)

  ALL = T.let(T.unsafe(nil), T::Array[T.untyped])

  ELEMENTS = T.let(T.unsafe(nil), T::Array[T.untyped])

  LITERAL = T.let(T.unsafe(nil), Regexp)

  def self.new; end

  def []=(variable_name, value); end

  # Performs a depth-first (document order) XPath search, and returns the first
  # match. This is the fastest, lightest way to return a single result.
  #
  # FIXME: This method is incomplete!
  def first(path_stack, node); end

  def get_first(path, nodeset); end

  def match(path_stack, nodeset); end

  def namespaces=(namespaces = _); end

  def parse(path, nodeset); end

  def predicate(path, nodeset); end

  def variables=(vars = _); end
end
