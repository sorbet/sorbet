# typed: __STDLIB_INTERNAL

# [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) provides SSL,
# TLS and general purpose cryptography. It wraps the
# [OpenSSL](https://www.openssl.org/) library.
#
# # Examples
#
# All examples assume you have loaded
# [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) with:
#
# ```ruby
# require 'openssl'
# ```
#
# These examples build atop each other. For example the key created in the next
# is used in throughout these examples.
#
# ## Keys
#
# ### Creating a Key
#
# This example creates a 2048 bit RSA keypair and writes it to the current
# directory.
#
# ```ruby
# key = OpenSSL::PKey::RSA.new 2048
#
# open 'private_key.pem', 'w' do |io| io.write key.to_pem end
# open 'public_key.pem', 'w' do |io| io.write key.public_key.to_pem end
# ```
#
# ### Exporting a Key
#
# Keys saved to disk without encryption are not secure as anyone who gets ahold
# of the key may use it unless it is encrypted. In order to securely export a
# key you may export it with a pass phrase.
#
# ```ruby
# cipher = OpenSSL::Cipher.new 'AES-128-CBC'
# pass_phrase = 'my secure pass phrase goes here'
#
# key_secure = key.export cipher, pass_phrase
#
# open 'private.secure.pem', 'w' do |io|
#   io.write key_secure
# end
# ```
#
# [`OpenSSL::Cipher.ciphers`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-c-ciphers)
# returns a list of available ciphers.
#
# ### Loading a Key
#
# A key can also be loaded from a file.
#
# ```ruby
# key2 = OpenSSL::PKey::RSA.new File.read 'private_key.pem'
# key2.public? # => true
# key2.private? # => true
# ```
#
# or
#
# ```ruby
# key3 = OpenSSL::PKey::RSA.new File.read 'public_key.pem'
# key3.public? # => true
# key3.private? # => false
# ```
#
# ### Loading an Encrypted Key
#
# [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) will prompt you
# for your pass phrase when loading an encrypted key. If you will not be able to
# type in the pass phrase you may provide it when loading the key:
#
# ```ruby
# key4_pem = File.read 'private.secure.pem'
# pass_phrase = 'my secure pass phrase goes here'
# key4 = OpenSSL::PKey::RSA.new key4_pem, pass_phrase
# ```
#
# ## RSA Encryption
#
# RSA provides encryption and decryption using the public and private keys. You
# can use a variety of padding methods depending upon the intended use of
# encrypted data.
#
# ### Encryption & Decryption
#
# Asymmetric public/private key encryption is slow and victim to attack in cases
# where it is used without padding or directly to encrypt larger chunks of data.
# Typical use cases for RSA encryption involve "wrapping" a symmetric key with
# the public key of the recipient who would "unwrap" that symmetric key again
# using their private key. The following illustrates a simplified example of
# such a key transport scheme. It shouldn't be used in practice, though,
# standardized protocols should always be preferred.
#
# ```ruby
# wrapped_key = key.public_encrypt key
# ```
#
# A symmetric key encrypted with the public key can only be decrypted with the
# corresponding private key of the recipient.
#
# ```ruby
# original_key = key.private_decrypt wrapped_key
# ```
#
# By default PKCS#1 padding will be used, but it is also possible to use other
# forms of padding, see PKey::RSA for further details.
#
# ### Signatures
#
# Using "private\_encrypt" to encrypt some data with the private key is
# equivalent to applying a digital signature to the data. A verifying party may
# validate the signature by comparing the result of decrypting the signature
# with "public\_decrypt" to the original data. However,
# [`OpenSSL::PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html)
# already has methods "sign" and "verify" that handle digital signatures in a
# standardized way - "private\_encrypt" and "public\_decrypt" shouldn't be used
# in practice.
#
# To sign a document, a cryptographically secure hash of the document is
# computed first, which is then signed using the private key.
#
# ```ruby
# digest = OpenSSL::Digest::SHA256.new
# signature = key.sign digest, document
# ```
#
# To validate the signature, again a hash of the document is computed and the
# signature is decrypted using the public key. The result is then compared to
# the hash just computed, if they are equal the signature was valid.
#
# ```ruby
# digest = OpenSSL::Digest::SHA256.new
# if key.verify digest, signature, document
#   puts 'Valid'
# else
#   puts 'Invalid'
# end
# ```
#
# ## PBKDF2 Password-based Encryption
#
# If supported by the underlying
# [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) version used,
# Password-based Encryption should use the features of PKCS5. If not supported
# or if required by legacy applications, the older, less secure methods
# specified in RFC 2898 are also supported (see below).
#
# PKCS5 supports PBKDF2 as it was specified in PKCS#5
# [v2.0](http://www.rsa.com/rsalabs/node.asp?id=2127). It still uses a password,
# a salt, and additionally a number of iterations that will slow the key
# derivation process down. The slower this is, the more work it requires being
# able to brute-force the resulting key.
#
# ### Encryption
#
# The strategy is to first instantiate a Cipher for encryption, and then to
# generate a random IV plus a key derived from the password using PBKDF2. PKCS
# #5 v2.0 recommends at least 8 bytes for the salt, the number of iterations
# largely depends on the hardware being used.
#
# ```
# cipher = OpenSSL::Cipher.new 'AES-128-CBC'
# cipher.encrypt
# iv = cipher.random_iv
#
# pwd = 'some hopefully not to easily guessable password'
# salt = OpenSSL::Random.random_bytes 16
# iter = 20000
# key_len = cipher.key_len
# digest = OpenSSL::Digest::SHA256.new
#
# key = OpenSSL::PKCS5.pbkdf2_hmac(pwd, salt, iter, key_len, digest)
# cipher.key = key
#
# Now encrypt the data:
#
# encrypted = cipher.update document
# encrypted << cipher.final
# ```
#
# ### Decryption
#
# Use the same steps as before to derive the symmetric AES key, this time
# setting the Cipher up for decryption.
#
# ```
# cipher = OpenSSL::Cipher.new 'AES-128-CBC'
# cipher.decrypt
# cipher.iv = iv # the one generated with #random_iv
#
# pwd = 'some hopefully not to easily guessable password'
# salt = ... # the one generated above
# iter = 20000
# key_len = cipher.key_len
# digest = OpenSSL::Digest::SHA256.new
#
# key = OpenSSL::PKCS5.pbkdf2_hmac(pwd, salt, iter, key_len, digest)
# cipher.key = key
#
# Now decrypt the data:
#
# decrypted = cipher.update encrypted
# decrypted << cipher.final
# ```
#
# ## PKCS #5 Password-based Encryption
#
# PKCS #5 is a password-based encryption standard documented at
# [RFC2898](http://www.ietf.org/rfc/rfc2898.txt). It allows a short password or
# passphrase to be used to create a secure encryption key. If possible, PBKDF2
# as described above should be used if the circumstances allow it.
#
# PKCS #5 uses a Cipher, a pass phrase and a salt to generate an encryption key.
#
# ```ruby
# pass_phrase = 'my secure pass phrase goes here'
# salt = '8 octets'
# ```
#
# ### Encryption
#
# First set up the cipher for encryption
#
# ```ruby
# encryptor = OpenSSL::Cipher.new 'AES-128-CBC'
# encryptor.encrypt
# encryptor.pkcs5_keyivgen pass_phrase, salt
# ```
#
# Then pass the data you want to encrypt through
#
# ```ruby
# encrypted = encryptor.update 'top secret document'
# encrypted << encryptor.final
# ```
#
# ### Decryption
#
# Use a new Cipher instance set up for decryption
#
# ```ruby
# decryptor = OpenSSL::Cipher.new 'AES-128-CBC'
# decryptor.decrypt
# decryptor.pkcs5_keyivgen pass_phrase, salt
# ```
#
# Then pass the data you want to decrypt through
#
# ```ruby
# plain = decryptor.update encrypted
# plain << decryptor.final
# ```
#
# ## X509 Certificates
#
# ### Creating a Certificate
#
# This example creates a self-signed certificate using an RSA key and a SHA1
# signature.
#
# ```ruby
# key = OpenSSL::PKey::RSA.new 2048
# name = OpenSSL::X509::Name.parse 'CN=nobody/DC=example'
#
# cert = OpenSSL::X509::Certificate.new
# cert.version = 2
# cert.serial = 0
# cert.not_before = Time.now
# cert.not_after = Time.now + 3600
#
# cert.public_key = key.public_key
# cert.subject = name
# ```
#
# ### Certificate Extensions
#
# You can add extensions to the certificate with OpenSSL::SSL::ExtensionFactory
# to indicate the purpose of the certificate.
#
# ```ruby
# extension_factory = OpenSSL::X509::ExtensionFactory.new nil, cert
#
# cert.add_extension \
#   extension_factory.create_extension('basicConstraints', 'CA:FALSE', true)
#
# cert.add_extension \
#   extension_factory.create_extension(
#     'keyUsage', 'keyEncipherment,dataEncipherment,digitalSignature')
#
# cert.add_extension \
#   extension_factory.create_extension('subjectKeyIdentifier', 'hash')
# ```
#
# The list of supported extensions (and in some cases their possible values) can
# be derived from the "objects.h" file in the
# [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) source code.
#
# ### Signing a Certificate
#
# To sign a certificate set the issuer and use
# [`OpenSSL::X509::Certificate#sign`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Certificate.html#method-i-sign)
# with a digest algorithm. This creates a self-signed cert because we're using
# the same name and key to sign the certificate as was used to create the
# certificate.
#
# ```ruby
# cert.issuer = name
# cert.sign key, OpenSSL::Digest::SHA1.new
#
# open 'certificate.pem', 'w' do |io| io.write cert.to_pem end
# ```
#
# ### Loading a Certificate
#
# Like a key, a cert can also be loaded from a file.
#
# ```ruby
# cert2 = OpenSSL::X509::Certificate.new File.read 'certificate.pem'
# ```
#
# ### Verifying a Certificate
#
# Certificate#verify will return true when a certificate was signed with the
# given public key.
#
# ```ruby
# raise 'certificate can not be verified' unless cert2.verify key
# ```
#
# ## Certificate Authority
#
# A certificate authority (CA) is a trusted third party that allows you to
# verify the ownership of unknown certificates. The CA issues key signatures
# that indicate it trusts the user of that key. A user encountering the key can
# verify the signature by using the CA's public key.
#
# ### CA Key
#
# CA keys are valuable, so we encrypt and save it to disk and make sure it is
# not readable by other users.
#
# ```ruby
# ca_key = OpenSSL::PKey::RSA.new 2048
# pass_phrase = 'my secure pass phrase goes here'
#
# cipher = OpenSSL::Cipher.new 'AES-128-CBC'
#
# open 'ca_key.pem', 'w', 0400 do |io|
#   io.write ca_key.export(cipher, pass_phrase)
# end
# ```
#
# ### CA Certificate
#
# A CA certificate is created the same way we created a certificate above, but
# with different extensions.
#
# ```ruby
# ca_name = OpenSSL::X509::Name.parse 'CN=ca/DC=example'
#
# ca_cert = OpenSSL::X509::Certificate.new
# ca_cert.serial = 0
# ca_cert.version = 2
# ca_cert.not_before = Time.now
# ca_cert.not_after = Time.now + 86400
#
# ca_cert.public_key = ca_key.public_key
# ca_cert.subject = ca_name
# ca_cert.issuer = ca_name
#
# extension_factory = OpenSSL::X509::ExtensionFactory.new
# extension_factory.subject_certificate = ca_cert
# extension_factory.issuer_certificate = ca_cert
#
# ca_cert.add_extension \
#   extension_factory.create_extension('subjectKeyIdentifier', 'hash')
# ```
#
# This extension indicates the CA's key may be used as a CA.
#
# ```ruby
# ca_cert.add_extension \
#   extension_factory.create_extension('basicConstraints', 'CA:TRUE', true)
# ```
#
# This extension indicates the CA's key may be used to verify signatures on both
# certificates and certificate revocations.
#
# ```ruby
# ca_cert.add_extension \
#   extension_factory.create_extension(
#     'keyUsage', 'cRLSign,keyCertSign', true)
# ```
#
# Root CA certificates are self-signed.
#
# ```ruby
# ca_cert.sign ca_key, OpenSSL::Digest::SHA1.new
# ```
#
# The CA certificate is saved to disk so it may be distributed to all the users
# of the keys this CA will sign.
#
# ```ruby
# open 'ca_cert.pem', 'w' do |io|
#   io.write ca_cert.to_pem
# end
# ```
#
# ### Certificate Signing Request
#
# The CA signs keys through a Certificate Signing Request (CSR). The CSR
# contains the information necessary to identify the key.
#
# ```ruby
# csr = OpenSSL::X509::Request.new
# csr.version = 0
# csr.subject = name
# csr.public_key = key.public_key
# csr.sign key, OpenSSL::Digest::SHA1.new
# ```
#
# A CSR is saved to disk and sent to the CA for signing.
#
# ```ruby
# open 'csr.pem', 'w' do |io|
#   io.write csr.to_pem
# end
# ```
#
# ### Creating a Certificate from a CSR
#
# Upon receiving a CSR the CA will verify it before signing it. A minimal
# verification would be to check the CSR's signature.
#
# ```ruby
# csr = OpenSSL::X509::Request.new File.read 'csr.pem'
#
# raise 'CSR can not be verified' unless csr.verify csr.public_key
# ```
#
# After verification a certificate is created, marked for various usages, signed
# with the CA key and returned to the requester.
#
# ```ruby
# csr_cert = OpenSSL::X509::Certificate.new
# csr_cert.serial = 0
# csr_cert.version = 2
# csr_cert.not_before = Time.now
# csr_cert.not_after = Time.now + 600
#
# csr_cert.subject = csr.subject
# csr_cert.public_key = csr.public_key
# csr_cert.issuer = ca_cert.subject
#
# extension_factory = OpenSSL::X509::ExtensionFactory.new
# extension_factory.subject_certificate = csr_cert
# extension_factory.issuer_certificate = ca_cert
#
# csr_cert.add_extension \
#   extension_factory.create_extension('basicConstraints', 'CA:FALSE')
#
# csr_cert.add_extension \
#   extension_factory.create_extension(
#     'keyUsage', 'keyEncipherment,dataEncipherment,digitalSignature')
#
# csr_cert.add_extension \
#   extension_factory.create_extension('subjectKeyIdentifier', 'hash')
#
# csr_cert.sign ca_key, OpenSSL::Digest::SHA1.new
#
# open 'csr_cert.pem', 'w' do |io|
#   io.write csr_cert.to_pem
# end
# ```
#
# ## SSL and TLS Connections
#
# Using our created key and certificate we can create an SSL or TLS connection.
# An SSLContext is used to set up an SSL session.
#
# ```ruby
# context = OpenSSL::SSL::SSLContext.new
# ```
#
# ### SSL Server
#
# An SSL server requires the certificate and private key to communicate securely
# with its clients:
#
# ```ruby
# context.cert = cert
# context.key = key
# ```
#
# Then create an SSLServer with a TCP server socket and the context. Use the
# SSLServer like an ordinary TCP server.
#
# ```ruby
# require 'socket'
#
# tcp_server = TCPServer.new 5000
# ssl_server = OpenSSL::SSL::SSLServer.new tcp_server, context
#
# loop do
#   ssl_connection = ssl_server.accept
#
#   data = connection.gets
#
#   response = "I got #{data.dump}"
#   puts response
#
#   connection.puts "I got #{data.dump}"
#   connection.close
# end
# ```
#
# ### SSL client
#
# An SSL client is created with a TCP socket and the context. SSLSocket#connect
# must be called to initiate the SSL handshake and start encryption. A key and
# certificate are not required for the client socket.
#
# Note that SSLSocket#close doesn't close the underlying socket by default.
# [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) SSLSocket#sync\_close to
# true if you want.
#
# ```ruby
# require 'socket'
#
# tcp_socket = TCPSocket.new 'localhost', 5000
# ssl_client = OpenSSL::SSL::SSLSocket.new tcp_socket, context
# ssl_client.sync_close = true
# ssl_client.connect
#
# ssl_client.puts "hello server!"
# puts ssl_client.gets
#
# ssl_client.close # shutdown the TLS connection and close tcp_socket
# ```
#
# ### Peer Verification
#
# An unverified SSL connection does not provide much security. For enhanced
# security the client or server can verify the certificate of its peer.
#
# The client can be modified to verify the server's certificate against the
# certificate authority's certificate:
#
# ```ruby
# context.ca_file = 'ca_cert.pem'
# context.verify_mode = OpenSSL::SSL::VERIFY_PEER
#
# require 'socket'
#
# tcp_socket = TCPSocket.new 'localhost', 5000
# ssl_client = OpenSSL::SSL::SSLSocket.new tcp_socket, context
# ssl_client.connect
#
# ssl_client.puts "hello server!"
# puts ssl_client.gets
# ```
#
# If the server certificate is invalid or `context.ca_file` is not set when
# verifying peers an
# [`OpenSSL::SSL::SSLError`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLError.html)
# will be raised.
module OpenSSL
  # Boolean indicating whether
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) is
  # FIPS-capable or not
  OPENSSL_FIPS = ::T.let(nil, ::T.untyped)
  OPENSSL_LIBRARY_VERSION = ::T.let(nil, ::T.untyped)
  # Version of [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) the
  # ruby [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) extension
  # was built with
  OPENSSL_VERSION = ::T.let(nil, ::T.untyped)
  # Version number of
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) the ruby
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) extension was
  # built with (base 16)
  OPENSSL_VERSION_NUMBER = ::T.let(nil, ::T.untyped)
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) ruby extension
  # version
  VERSION = ::T.let(nil, ::T.untyped)

  # Returns a [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html)
  # subclass by *name*
  #
  # ```ruby
  # require 'openssl'
  #
  # OpenSSL::Digest("MD5")
  # # => OpenSSL::Digest::MD5
  #
  # Digest("Foo")
  # # => NameError: wrong constant name Foo
  # ```
  sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.Digest(name); end

  sig {returns(::T.untyped)}
  def self.debug(); end

  # Turns on or off debug mode. With debug mode, all erros added to the
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) error queue
  # will be printed to stderr.
  sig do
    params(
      debug: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.debug=(debug); end

  # See any remaining errors held in queue.
  #
  # Any errors you see here are probably due to a bug in Ruby's
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # implementation.
  sig {returns(::T.untyped)}
  def self.errors(); end

  # Turns FIPS mode on or off. Turning on FIPS mode will obviously only have an
  # effect for FIPS-capable installations of the
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) library.
  # Trying to do so otherwise will result in an error.
  #
  # ### Examples
  #
  # ```ruby
  # OpenSSL.fips_mode = true   # turn FIPS mode on
  # OpenSSL.fips_mode = false  # and off again
  # ```
  sig do
    params(
      fips_mode: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.fips_mode=(fips_mode); end
end

# Abstract Syntax Notation One (or ASN.1) is a notation syntax to describe data
# structures and is defined in ITU-T X.680. ASN.1 itself does not mandate any
# encoding or parsing rules, but usually ASN.1 data structures are encoded using
# the Distinguished
# [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) Rules (DER) or
# less often the Basic
# [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html) Rules (BER)
# described in ITU-T X.690. DER and BER encodings are binary Tag-Length-Value
# (TLV) encodings that are quite concise compared to other popular data
# description formats such as
# [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html),
# [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) etc. ASN.1 data
# structures are very common in cryptographic applications, e.g. X.509 public
# key certificates or certificate revocation lists (CRLs) are all defined in
# ASN.1 and DER-encoded. ASN.1, DER and BER are the building blocks of applied
# cryptography. The
# [`ASN1`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1.html) module
# provides the necessary classes that allow generation of ASN.1 data structures
# and the methods to encode them using a DER encoding. The decode method allows
# parsing arbitrary BER-/DER-encoded data to a Ruby object that can then be
# modified and re-encoded at will.
#
# ## ASN.1 class hierarchy
#
# The base class representing ASN.1 structures is ASN1Data. ASN1Data offers
# attributes to read and set the *tag*, the *tag\_class* and finally the *value*
# of a particular ASN.1 item. Upon parsing, any tagged values (implicit or
# explicit) will be represented by ASN1Data instances because their "real type"
# can only be determined using out-of-band information from the ASN.1 type
# declaration. Since this information is normally known when encoding a type,
# all sub-classes of ASN1Data offer an additional attribute *tagging* that
# allows to encode a value implicitly (`:IMPLICIT`) or explicitly (`:EXPLICIT`).
#
# ### Constructive
#
# Constructive is, as its name implies, the base class for all constructed
# encodings, i.e. those that consist of several values, opposed to "primitive"
# encodings with just one single value. The value of an Constructive is always
# an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
#
# #### ASN1::Set and ASN1::Sequence
#
# The most common constructive encodings are SETs and SEQUENCEs, which is why
# there are two sub-classes of Constructive representing each of them.
#
# ### Primitive
#
# This is the super class of all primitive values. Primitive itself is not used
# when parsing ASN.1 data, all values are either instances of a corresponding
# sub-class of Primitive or they are instances of ASN1Data if the value was
# tagged implicitly or explicitly. Please cf. Primitive documentation for
# details on sub-classes and their respective mappings of ASN.1 data types to
# Ruby objects.
#
# ## Possible values for *tagging*
#
# When constructing an ASN1Data object the ASN.1 type definition may require
# certain elements to be either implicitly or explicitly tagged. This can be
# achieved by setting the *tagging* attribute manually for sub-classes of
# ASN1Data. Use the symbol `:IMPLICIT` for implicit tagging and `:EXPLICIT` if
# the element requires explicit tagging.
#
# ## Possible values for *tag\_class*
#
# It is possible to create arbitrary ASN1Data objects that also support a
# PRIVATE or APPLICATION tag class. Possible values for the *tag\_class*
# attribute are:
# *   `:UNIVERSAL` (the default for untagged values)
# *   `:CONTEXT_SPECIFIC` (the default for tagged values)
# *   `:APPLICATION`
# *   `:PRIVATE`
#
#
# ## Tag constants
#
# There is a constant defined for each universal tag:
# *   OpenSSL::ASN1::EOC (0)
# *   OpenSSL::ASN1::BOOLEAN (1)
# *   OpenSSL::ASN1::INTEGER (2)
# *   OpenSSL::ASN1::BIT\_STRING (3)
# *   OpenSSL::ASN1::OCTET\_STRING (4)
# *   OpenSSL::ASN1::NULL (5)
# *   OpenSSL::ASN1::OBJECT (6)
# *   OpenSSL::ASN1::ENUMERATED (10)
# *   OpenSSL::ASN1::UTF8STRING (12)
# *   OpenSSL::ASN1::SEQUENCE (16)
# *   OpenSSL::ASN1::SET (17)
# *   OpenSSL::ASN1::NUMERICSTRING (18)
# *   OpenSSL::ASN1::PRINTABLESTRING (19)
# *   OpenSSL::ASN1::T61STRING (20)
# *   OpenSSL::ASN1::VIDEOTEXSTRING (21)
# *   OpenSSL::ASN1::IA5STRING (22)
# *   OpenSSL::ASN1::UTCTIME (23)
# *   OpenSSL::ASN1::GENERALIZEDTIME (24)
# *   OpenSSL::ASN1::GRAPHICSTRING (25)
# *   OpenSSL::ASN1::ISO64STRING (26)
# *   OpenSSL::ASN1::GENERALSTRING (27)
# *   OpenSSL::ASN1::UNIVERSALSTRING (28)
# *   OpenSSL::ASN1::BMPSTRING (30)
#
#
# ## [`UNIVERSAL_TAG_NAME`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1.html#UNIVERSAL_TAG_NAME) constant
#
# An [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) that stores the
# name of a given tag number. These names are the same as the name of the tag
# constant that is additionally defined, e.g. [UNIVERSAL\_TAG\_NAME](2) =
# "INTEGER" and OpenSSL::ASN1::INTEGER = 2.
#
# ## Example usage
#
# ### Decoding and viewing a DER-encoded file
#
# ```ruby
# require 'openssl'
# require 'pp'
# der = File.binread('data.der')
# asn1 = OpenSSL::ASN1.decode(der)
# pp der
# ```
#
# ### Creating an ASN.1 structure and DER-encoding it
#
# ```ruby
# require 'openssl'
# version = OpenSSL::ASN1::Integer.new(1)
# # Explicitly 0-tagged implies context-specific tag class
# serial = OpenSSL::ASN1::Integer.new(12345, 0, :EXPLICIT, :CONTEXT_SPECIFIC)
# name = OpenSSL::ASN1::PrintableString.new('Data 1')
# sequence = OpenSSL::ASN1::Sequence.new( [ version, serial, name ] )
# der = sequence.to_der
# ```
module OpenSSL::ASN1
  BIT_STRING = ::T.let(nil, ::T.untyped)
  BMPSTRING = ::T.let(nil, ::T.untyped)
  BOOLEAN = ::T.let(nil, ::T.untyped)
  CHARACTER_STRING = ::T.let(nil, ::T.untyped)
  EMBEDDED_PDV = ::T.let(nil, ::T.untyped)
  ENUMERATED = ::T.let(nil, ::T.untyped)
  EOC = ::T.let(nil, ::T.untyped)
  EXTERNAL = ::T.let(nil, ::T.untyped)
  GENERALIZEDTIME = ::T.let(nil, ::T.untyped)
  GENERALSTRING = ::T.let(nil, ::T.untyped)
  GRAPHICSTRING = ::T.let(nil, ::T.untyped)
  IA5STRING = ::T.let(nil, ::T.untyped)
  INTEGER = ::T.let(nil, ::T.untyped)
  ISO64STRING = ::T.let(nil, ::T.untyped)
  NULL = ::T.let(nil, ::T.untyped)
  NUMERICSTRING = ::T.let(nil, ::T.untyped)
  OBJECT = ::T.let(nil, ::T.untyped)
  OBJECT_DESCRIPTOR = ::T.let(nil, ::T.untyped)
  OCTET_STRING = ::T.let(nil, ::T.untyped)
  PRINTABLESTRING = ::T.let(nil, ::T.untyped)
  REAL = ::T.let(nil, ::T.untyped)
  RELATIVE_OID = ::T.let(nil, ::T.untyped)
  SEQUENCE = ::T.let(nil, ::T.untyped)
  SET = ::T.let(nil, ::T.untyped)
  T61STRING = ::T.let(nil, ::T.untyped)
  UNIVERSALSTRING = ::T.let(nil, ::T.untyped)
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) storing tag names
  # at the tag's index.
  UNIVERSAL_TAG_NAME = ::T.let(nil, ::T.untyped)
  UTCTIME = ::T.let(nil, ::T.untyped)
  UTF8STRING = ::T.let(nil, ::T.untyped)
  VIDEOTEXSTRING = ::T.let(nil, ::T.untyped)

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.BMPString(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.BitString(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.Boolean(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.EndOfContent(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.Enumerated(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.GeneralString(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.GeneralizedTime(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.GraphicString(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.IA5String(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.ISO64String(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.Integer(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.Null(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.NumericString(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.ObjectId(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.OctetString(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.PrintableString(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.Sequence(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.Set(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.T61String(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.UTCTime(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.UTF8String(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.UniversalString(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.VideotexString(*arg0); end

  # Decodes a BER- or DER-encoded value and creates an ASN1Data instance. *der*
  # may be a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) or any
  # object that features a `.to_der` method transforming it into a
  # BER-/DER-encoded String+
  #
  # ## Example
  #
  # ```ruby
  # der = File.binread('asn1data')
  # asn1 = OpenSSL::ASN1.decode(der)
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.decode(arg0); end

  # Similar to decode with the difference that decode expects one distinct value
  # represented in *der*. decode\_all on the contrary decodes a sequence of
  # sequential BER/DER values lined up in *der* and returns them as an array.
  #
  # ## Example
  #
  # ```ruby
  # ders = File.binread('asn1data_seq')
  # asn1_ary = OpenSSL::ASN1.decode_all(ders)
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.decode_all(arg0); end

  # If a block is given, it prints out each of the elements encountered. Block
  # parameters are (in that order):
  # *   depth: The recursion depth, plus one with each constructed value being
  #     encountered
  #     ([`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html))
  # *   offset: Current byte offset
  #     ([`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html))
  # *   header length: Combined length in bytes of the Tag and Length headers.
  #     ([`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html))
  # *   length: The overall remaining length of the entire data
  #     ([`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html))
  # *   constructed: Whether this value is constructed or not (Boolean)
  # *   tag\_class: Current tag class
  #     ([`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html))
  # *   tag: The current tag number
  #     ([`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html))
  #
  #
  # ## Example
  #
  # ```ruby
  # der = File.binread('asn1data.der')
  # OpenSSL::ASN1.traverse(der) do | depth, offset, header_len, length, constructed, tag_class, tag|
  #   puts "Depth: #{depth} Offset: #{offset} Length: #{length}"
  #   puts "Header length: #{header_len} Tag: #{tag} Tag class: #{tag_class} Constructed: #{constructed}"
  # end
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.traverse(arg0); end
end

# The top-level class representing any ASN.1 object. When parsed by
# [`ASN1.decode`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1.html#method-c-decode),
# tagged values are always represented by an instance of
# [`ASN1Data`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ASN1Data.html).
#
# ## The role of [`ASN1Data`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ASN1Data.html) for parsing tagged values
#
# When encoding an ASN.1 type it is inherently clear what original type (e.g.
# INTEGER, OCTET STRING etc.) this value has, regardless of its tagging. But
# opposed to the time an ASN.1 type is to be encoded, when parsing them it is
# not possible to deduce the "real type" of tagged values. This is why tagged
# values are generally parsed into
# [`ASN1Data`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ASN1Data.html)
# instances, but with a different outcome for implicit and explicit tagging.
#
# ### Example of a parsed implicitly tagged value
#
# An implicitly 1-tagged INTEGER value will be parsed as an
# [`ASN1Data`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ASN1Data.html)
# with
# *   *tag* equal to 1
# *   *tag\_class* equal to `:CONTEXT_SPECIFIC`
# *   *value* equal to a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) that carries
#     the raw encoding of the INTEGER.
#
# This implies that a subsequent decoding step is required to completely decode
# implicitly tagged values.
#
# ### Example of a parsed explicitly tagged value
#
# An explicitly 1-tagged INTEGER value will be parsed as an
# [`ASN1Data`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ASN1Data.html)
# with
# *   *tag* equal to 1
# *   *tag\_class* equal to `:CONTEXT_SPECIFIC`
# *   *value* equal to an
#     [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) with one single
#     element, an instance of OpenSSL::ASN1::Integer, i.e. the inner element is
#     the non-tagged primitive value, and the tagging is represented in the
#     outer
#     [`ASN1Data`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ASN1Data.html)
#
#
# ## Example - Decoding an implicitly tagged INTEGER
#
# ```ruby
# int = OpenSSL::ASN1::Integer.new(1, 0, :IMPLICIT) # implicit 0-tagged
# seq = OpenSSL::ASN1::Sequence.new( [int] )
# der = seq.to_der
# asn1 = OpenSSL::ASN1.decode(der)
# # pp asn1 => #<OpenSSL::ASN1::Sequence:0x87326e0
# #              @indefinite_length=false,
# #              @tag=16,
# #              @tag_class=:UNIVERSAL,
# #              @tagging=nil,
# #              @value=
# #                [#<OpenSSL::ASN1::ASN1Data:0x87326f4
# #                   @indefinite_length=false,
# #                   @tag=0,
# #                   @tag_class=:CONTEXT_SPECIFIC,
# #                   @value="\x01">]>
# raw_int = asn1.value[0]
# # manually rewrite tag and tag class to make it an UNIVERSAL value
# raw_int.tag = OpenSSL::ASN1::INTEGER
# raw_int.tag_class = :UNIVERSAL
# int2 = OpenSSL::ASN1.decode(raw_int)
# puts int2.value # => 1
# ```
#
# ## Example - Decoding an explicitly tagged INTEGER
#
# ```ruby
# int = OpenSSL::ASN1::Integer.new(1, 0, :EXPLICIT) # explicit 0-tagged
# seq = OpenSSL::ASN1::Sequence.new( [int] )
# der = seq.to_der
# asn1 = OpenSSL::ASN1.decode(der)
# # pp asn1 => #<OpenSSL::ASN1::Sequence:0x87326e0
# #              @indefinite_length=false,
# #              @tag=16,
# #              @tag_class=:UNIVERSAL,
# #              @tagging=nil,
# #              @value=
# #                [#<OpenSSL::ASN1::ASN1Data:0x87326f4
# #                   @indefinite_length=false,
# #                   @tag=0,
# #                   @tag_class=:CONTEXT_SPECIFIC,
# #                   @value=
# #                     [#<OpenSSL::ASN1::Integer:0x85bf308
# #                        @indefinite_length=false,
# #                        @tag=2,
# #                        @tag_class=:UNIVERSAL
# #                        @tagging=nil,
# #                        @value=1>]>]>
# int2 = asn1.value[0].value[0]
# puts int2.value # => 1
# ```
class OpenSSL::ASN1::ASN1Data
  # Never `nil`. A boolean value indicating whether the encoding uses indefinite
  # length (in the case of parsing) or whether an indefinite length form shall
  # be used (in the encoding case). In DER, every value uses definite length
  # form. But in scenarios where large amounts of data need to be transferred it
  # might be desirable to have some kind of streaming support available. For
  # example, huge OCTET STRINGs are preferably sent in smaller-sized chunks,
  # each at a time. This is possible in BER by setting the length bytes of an
  # encoding to zero and by this indicating that the following value will be
  # sent in chunks. Indefinite length encodings are always constructed. The end
  # of such a stream of chunks is indicated by sending a EOC (End of Content)
  # tag. SETs and SEQUENCEs may use an indefinite length encoding, but also
  # primitive types such as e.g. OCTET STRINGS or BIT STRINGS may leverage this
  # functionality (cf. ITU-T X.690).
  sig {returns(::T.untyped)}
  def infinite_length(); end

  # Never `nil`. A boolean value indicating whether the encoding uses indefinite
  # length (in the case of parsing) or whether an indefinite length form shall
  # be used (in the encoding case). In DER, every value uses definite length
  # form. But in scenarios where large amounts of data need to be transferred it
  # might be desirable to have some kind of streaming support available. For
  # example, huge OCTET STRINGs are preferably sent in smaller-sized chunks,
  # each at a time. This is possible in BER by setting the length bytes of an
  # encoding to zero and by this indicating that the following value will be
  # sent in chunks. Indefinite length encodings are always constructed. The end
  # of such a stream of chunks is indicated by sending a EOC (End of Content)
  # tag. SETs and SEQUENCEs may use an indefinite length encoding, but also
  # primitive types such as e.g. OCTET STRINGS or BIT STRINGS may leverage this
  # functionality (cf. ITU-T X.690).
  sig do
    params(
      infinite_length: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def infinite_length=(infinite_length); end

  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
    )
    .void
  end
  def initialize(arg0, arg1, arg2); end

  # An [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html)
  # representing the tag number of this
  # [`ASN1Data`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ASN1Data.html).
  # Never `nil`.
  sig {returns(::T.untyped)}
  def tag(); end

  # An [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html)
  # representing the tag number of this
  # [`ASN1Data`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ASN1Data.html).
  # Never `nil`.
  sig do
    params(
      tag: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def tag=(tag); end

  # A [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html) representing
  # the tag class of this
  # [`ASN1Data`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ASN1Data.html).
  # Never `nil`. See
  # [`ASN1Data`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ASN1Data.html)
  # for possible values.
  sig {returns(::T.untyped)}
  def tag_class(); end

  # A [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html) representing
  # the tag class of this
  # [`ASN1Data`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ASN1Data.html).
  # Never `nil`. See
  # [`ASN1Data`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ASN1Data.html)
  # for possible values.
  sig do
    params(
      tag_class: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def tag_class=(tag_class); end

  # Encodes this
  # [`ASN1Data`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ASN1Data.html)
  # into a DER-encoded
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) value. The
  # result is DER-encoded except for the possibility of indefinite length forms.
  # Indefinite length forms are not allowed in strict DER, so strictly speaking
  # the result of such an encoding would be a BER-encoding.
  sig {returns(::T.untyped)}
  def to_der(); end

  # Carries the value of a ASN.1 type. Please confer Constructive and Primitive
  # for the mappings between ASN.1 data types and Ruby classes.
  sig {returns(::T.untyped)}
  def value(); end

  # Carries the value of a ASN.1 type. Please confer Constructive and Primitive
  # for the mappings between ASN.1 data types and Ruby classes.
  sig do
    params(
      value: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def value=(value); end
end

# Generic error class for all errors raised in
# [`ASN1`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1.html) and any of the
# classes defined in it.
class OpenSSL::ASN1::ASN1Error < OpenSSL::OpenSSLError
end

class OpenSSL::ASN1::BMPString < OpenSSL::ASN1::Primitive
end

class OpenSSL::ASN1::BitString < OpenSSL::ASN1::Primitive
  sig {returns(::T.untyped)}
  def unused_bits(); end

  sig do
    params(
      unused_bits: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unused_bits=(unused_bits); end
end

class OpenSSL::ASN1::Boolean < OpenSSL::ASN1::Primitive
end

# The parent class for all constructed encodings. The *value* attribute of a
# [`Constructive`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/Constructive.html)
# is always an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
# Attributes are the same as for ASN1Data, with the addition of *tagging*.
#
# ## SET and SEQUENCE
#
# Most constructed encodings come in the form of a SET or a SEQUENCE. These
# encodings are represented by one of the two sub-classes of Constructive:
# *   OpenSSL::ASN1::Set
# *   OpenSSL::ASN1::Sequence
#
# Please note that tagged sequences and sets are still parsed as instances of
# ASN1Data. [`Find`](https://docs.ruby-lang.org/en/2.7.0/Find.html) further
# details on tagged values there.
#
# ### Example - constructing a SEQUENCE
#
# ```ruby
# int = OpenSSL::ASN1::Integer.new(1)
# str = OpenSSL::ASN1::PrintableString.new('abc')
# sequence = OpenSSL::ASN1::Sequence.new( [ int, str ] )
# ```
#
# ### Example - constructing a SET
#
# ```ruby
# int = OpenSSL::ASN1::Integer.new(1)
# str = OpenSSL::ASN1::PrintableString.new('abc')
# set = OpenSSL::ASN1::Set.new( [ int, str ] )
# ```
class OpenSSL::ASN1::Constructive < OpenSSL::ASN1::ASN1Data
  include ::Enumerable
  Elem = type_member {{fixed: T.untyped}}
  # Calls the given block once for each element in self, passing that element as
  # parameter *asn1*. If no block is given, an enumerator is returned instead.
  #
  # ## Example
  #
  # ```ruby
  # asn1_ary.each do |asn1|
  #   puts asn1
  # end
  # ```
  sig {returns(::T.untyped)}
  def each(&blk); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # May be used as a hint for encoding a value either implicitly or explicitly
  # by setting it either to `:IMPLICIT` or to `:EXPLICIT`. *tagging* is not set
  # when a ASN.1 structure is parsed using
  # [`OpenSSL::ASN1.decode`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1.html#method-c-decode).
  sig {returns(::T.untyped)}
  def tagging(); end

  # May be used as a hint for encoding a value either implicitly or explicitly
  # by setting it either to `:IMPLICIT` or to `:EXPLICIT`. *tagging* is not set
  # when a ASN.1 structure is parsed using
  # [`OpenSSL::ASN1.decode`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1.html#method-c-decode).
  sig do
    params(
      tagging: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def tagging=(tagging); end

  # See ASN1Data#to\_der for details.
  sig {returns(::T.untyped)}
  def to_der(); end
end

class OpenSSL::ASN1::EndOfContent < OpenSSL::ASN1::ASN1Data
  sig {void}
  def initialize(); end
end

class OpenSSL::ASN1::Enumerated < OpenSSL::ASN1::Primitive
end

class OpenSSL::ASN1::GeneralString < OpenSSL::ASN1::Primitive
end

class OpenSSL::ASN1::GeneralizedTime < OpenSSL::ASN1::Primitive
end

class OpenSSL::ASN1::GraphicString < OpenSSL::ASN1::Primitive
end

class OpenSSL::ASN1::IA5String < OpenSSL::ASN1::Primitive
end

class OpenSSL::ASN1::ISO64String < OpenSSL::ASN1::Primitive
end

class OpenSSL::ASN1::Integer < OpenSSL::ASN1::Primitive
end

class OpenSSL::ASN1::Null < OpenSSL::ASN1::Primitive
end

class OpenSSL::ASN1::NumericString < OpenSSL::ASN1::Primitive
end

# Represents the primitive object id for
# [`OpenSSL::ASN1`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1.html)
class OpenSSL::ASN1::ObjectId < OpenSSL::ASN1::Primitive
  # The long name of the
  # [`ObjectId`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ObjectId.html),
  # as defined in <openssl/objects.h>.
  #
  # Also aliased as:
  # [`long_name`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ObjectId.html#method-i-long_name)
  sig {returns(::T.untyped)}
  def ln(); end

  # Alias for:
  # [`ln`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ObjectId.html#method-i-ln)
  sig {returns(::T.untyped)}
  def long_name(); end

  # Returns a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # representing the [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html)
  # Identifier in the dot notation, e.g. "1.2.3.4.5"
  sig {returns(::T.untyped)}
  def oid(); end

  # Alias for:
  # [`sn`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ObjectId.html#method-i-sn)
  sig {returns(::T.untyped)}
  def short_name(); end

  # The short name of the
  # [`ObjectId`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ObjectId.html),
  # as defined in <openssl/objects.h>.
  #
  # Also aliased as:
  # [`short_name`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ObjectId.html#method-i-short_name)
  sig {returns(::T.untyped)}
  def sn(); end

  # This adds a new
  # [`ObjectId`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ObjectId.html)
  # to the internal tables. Where *object\_id* is the numerical form,
  # *short\_name* is the short name, and *long\_name* is the long name.
  #
  # Returns `true` if successful. Raises an
  # [`OpenSSL::ASN1::ASN1Error`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ASN1Error.html)
  # if it fails.
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.register(arg0, arg1, arg2); end
end

class OpenSSL::ASN1::OctetString < OpenSSL::ASN1::Primitive
end

# The parent class for all primitive encodings. Attributes are the same as for
# ASN1Data, with the addition of *tagging*.
# [`Primitive`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/Primitive.html)
# values can never be encoded with indefinite length form, thus it is not
# possible to set the *indefinite\_length* attribute for
# [`Primitive`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/Primitive.html)
# and its sub-classes.
#
# ## [`Primitive`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/Primitive.html) sub-classes and their mapping to Ruby classes
# *   OpenSSL::ASN1::EndOfContent    <=> *value* is always `nil`
# *   OpenSSL::ASN1::Boolean         <=> *value* is `true` or `false`
# *   OpenSSL::ASN1::Integer         <=> *value* is an
#     [`OpenSSL::BN`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html)
# *   OpenSSL::ASN1::BitString       <=> *value* is a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
# *   OpenSSL::ASN1::OctetString     <=> *value* is a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
# *   OpenSSL::ASN1::Null            <=> *value* is always `nil`
# *   OpenSSL::ASN1::Object          <=> *value* is a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
# *   OpenSSL::ASN1::Enumerated      <=> *value* is an
#     [`OpenSSL::BN`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html)
# *   OpenSSL::ASN1::UTF8String      <=> *value* is a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
# *   OpenSSL::ASN1::NumericString   <=> *value* is a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
# *   OpenSSL::ASN1::PrintableString <=> *value* is a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
# *   OpenSSL::ASN1::T61String       <=> *value* is a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
# *   OpenSSL::ASN1::VideotexString  <=> *value* is a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
# *   OpenSSL::ASN1::IA5String       <=> *value* is a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
# *   OpenSSL::ASN1::UTCTime         <=> *value* is a
#     [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html)
# *   OpenSSL::ASN1::GeneralizedTime <=> *value* is a
#     [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html)
# *   OpenSSL::ASN1::GraphicString   <=> *value* is a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
# *   OpenSSL::ASN1::ISO64String     <=> *value* is a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
# *   OpenSSL::ASN1::GeneralString   <=> *value* is a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
# *   OpenSSL::ASN1::UniversalString <=> *value* is a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
# *   OpenSSL::ASN1::BMPString       <=> *value* is a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
#
#
# ## OpenSSL::ASN1::BitString
#
# ### Additional attributes
# *unused\_bits*: if the underlying BIT STRING's length is a multiple of 8 then
# *unused\_bits* is 0. Otherwise *unused\_bits* indicates the number of bits
# that are to be ignored in the final octet of the BitString's *value*.
#
# ## [`OpenSSL::ASN1::ObjectId`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/ObjectId.html)
#
# NOTE: While
# [`OpenSSL::ASN1::ObjectId.new`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/Primitive.html#method-c-new)
# will allocate a new ObjectId, it is not typically allocated this way, but
# rather that are received from parsed
# [`ASN1`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1.html) encodings.
#
# ### Additional attributes
# *   *sn*: the short name as defined in <openssl/objects.h>.
# *   *ln*: the long name as defined in <openssl/objects.h>.
# *   *oid*: the object identifier as a
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), e.g.
#     "1.2.3.4.5"
# *   *short\_name*: alias for *sn*.
# *   *long\_name*: alias for *ln*.
#
#
# ## Examples
# With the [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) of
# OpenSSL::ASN1::EndOfContent, each
# [`Primitive`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/Primitive.html)
# class constructor takes at least one parameter, the *value*.
#
# ### Creating EndOfContent
#
# ```ruby
# eoc = OpenSSL::ASN1::EndOfContent.new
# ```
#
# ### Creating any other [`Primitive`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1/Primitive.html)
#
# ```
# prim = <class>.new(value) # <class> being one of the sub-classes except EndOfContent
# prim_zero_tagged_implicit = <class>.new(value, 0, :IMPLICIT)
# prim_zero_tagged_explicit = <class>.new(value, 0, :EXPLICIT)
# ```
class OpenSSL::ASN1::Primitive < OpenSSL::ASN1::ASN1Data
  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # May be used as a hint for encoding a value either implicitly or explicitly
  # by setting it either to `:IMPLICIT` or to `:EXPLICIT`. *tagging* is not set
  # when a ASN.1 structure is parsed using
  # [`OpenSSL::ASN1.decode`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1.html#method-c-decode).
  sig {returns(::T.untyped)}
  def tagging(); end

  # May be used as a hint for encoding a value either implicitly or explicitly
  # by setting it either to `:IMPLICIT` or to `:EXPLICIT`. *tagging* is not set
  # when a ASN.1 structure is parsed using
  # [`OpenSSL::ASN1.decode`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/ASN1.html#method-c-decode).
  sig do
    params(
      tagging: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def tagging=(tagging); end

  # See ASN1Data#to\_der for details.
  sig {returns(::T.untyped)}
  def to_der(); end
end

class OpenSSL::ASN1::PrintableString < OpenSSL::ASN1::Primitive
end

class OpenSSL::ASN1::Sequence < OpenSSL::ASN1::Constructive
  Elem = type_member {{fixed: T.untyped}}
end

class OpenSSL::ASN1::Set < OpenSSL::ASN1::Constructive
  Elem = type_member {{fixed: T.untyped}}
end

class OpenSSL::ASN1::T61String < OpenSSL::ASN1::Primitive
end

class OpenSSL::ASN1::UTCTime < OpenSSL::ASN1::Primitive
end

class OpenSSL::ASN1::UTF8String < OpenSSL::ASN1::Primitive
end

class OpenSSL::ASN1::UniversalString < OpenSSL::ASN1::Primitive
end

class OpenSSL::ASN1::VideotexString < OpenSSL::ASN1::Primitive
end

class OpenSSL::BN
  include ::Comparable
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def %(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def *(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def **(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def +(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def -(arg0); end


  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def <<(arg0); end

  # Alias for:
  # [`cmp`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html#method-i-cmp)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def <=>(arg0); end

  # Returns `true` only if *obj* has the same value as *bn*. Contrast this with
  # [`OpenSSL::BN#eql?`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html#method-i-eql-3F),
  # which requires obj to be
  # [`OpenSSL::BN`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html).
  #
  # Also aliased as:
  # [`===`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html#method-i-3D-3D-3D)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ==(arg0); end

  # Alias for:
  # [`==`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html#method-i-3D-3D)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ===(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def >>(arg0); end

  # Tests bit *bit* in *bn* and returns `true` if set, `false` if not set.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def bit_set?(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def clear_bit!(arg0); end

  # Also aliased as:
  # [`<=>`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html#method-i-3C-3D-3E)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def cmp(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def coerce(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def copy(arg0); end

  # Returns `true` only if *obj* is a `OpenSSL::BN` with the same value as *bn*.
  # Contrast this with OpenSSL::BN#==, which performs type conversions.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def eql?(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def gcd(arg0); end

  # Returns a hash code for this object.
  #
  # See also
  # [`Object#hash`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-hash).
  sig {returns(::T.untyped)}
  def hash(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def lshift!(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def mask_bits!(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def mod_add(arg0, arg1); end

  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def mod_exp(arg0, arg1); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def mod_inverse(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def mod_mul(arg0, arg1); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def mod_sqr(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def mod_sub(arg0, arg1); end

  sig {returns(::T.untyped)}
  def num_bits(); end

  sig {returns(::T.untyped)}
  def num_bytes(); end

  sig {returns(::T.untyped)}
  def odd?(); end

  sig {returns(::T.untyped)}
  def one?(); end

  sig do
    params(
      q: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def pretty_print(q); end

  # Performs a Miller-Rabin probabilistic primality test with *checks*
  # iterations. If *checks* is not specified, a number of iterations is used
  # that yields a false positive rate of at most 2^-80 for random input.
  #
  # ### Parameters
  # *   *checks* - integer
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def prime?(*arg0); end

  # Performs a Miller-Rabin primality test. This is same as
  # [`prime?`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html#method-i-prime-3F)
  # except this first attempts trial divisions with some small primes.
  #
  # ### Parameters
  # *   *checks* - integer
  # *   *trial\_div* - boolean
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def prime_fasttest?(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def rshift!(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def set_bit!(arg0); end

  sig {returns(::T.untyped)}
  def sqr(); end

  sig {returns(::T.untyped)}
  def to_bn(); end

  # Also aliased as:
  # [`to_int`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html#method-i-to_int)
  sig {returns(::T.untyped)}
  def to_i(); end

  # Alias for:
  # [`to_i`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html#method-i-to_i)
  sig {returns(::T.untyped)}
  def to_int(); end

  # ### Parameters
  # *   *base* - [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html)
  #     Valid values:
  #     *   0 - MPI
  #     *   2 - binary
  #     *   10 - the default
  #     *   16 - hex
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def to_s(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ucmp(arg0); end

  sig {returns(::T.untyped)}
  def zero?(); end

  # Generates a random prime number of bit length *bits*. If *safe* is set to
  # `true`, generates a safe prime. If *add* is specified, generates a prime
  # that fulfills condition `p % add = rem`.
  #
  # ### Parameters
  # *   *bits* - integer
  # *   *safe* - boolean
  # *   *add* - [`BN`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html)
  # *   *rem* - [`BN`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.generate_prime(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pseudo_rand(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pseudo_rand_range(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.rand(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.rand_range(arg0); end
end

# Generic [`Error`](https://docs.ruby-lang.org/en/2.7.0/Error.html) for all of
# [`OpenSSL::BN`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html) (big num)
class OpenSSL::BNError < OpenSSL::OpenSSLError
end

# [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) buffering mix-in module.
#
# This module allows an
# [`OpenSSL::SSL::SSLSocket`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html)
# to behave like an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
#
# You typically won't use this module directly, you can see it implemented in
# [`OpenSSL::SSL::SSLSocket`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html).
module OpenSSL::Buffering
  include ::Enumerable
  Elem = type_member {{fixed: T.untyped}}

  # Default size to read from or write to the SSLSocket for buffer operations.
  BLOCK_SIZE = ::T.let(nil, ::T.untyped)

  # Writes *s* to the stream. *s* will be converted to a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) using `.to_s`
  # method.
  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def <<(s); end

  # Closes the SSLSocket and flushes any unwritten data.
  sig {returns(::T.untyped)}
  def close(); end

  # Executes the block for every line in the stream where lines are separated by
  # *eol*.
  #
  # See also
  # [`gets`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-gets)
  #
  # Also aliased as:
  # [`each_line`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-each_line)
  sig do
    params(
      eol: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def each(eol=T.unsafe(nil), &blk); end

  # Calls the given block once for each byte in the stream.
  sig {returns(::T.untyped)}
  def each_byte(); end

  # Alias for:
  # [`each`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-each)
  sig do
    params(
      eol: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def each_line(eol=T.unsafe(nil)); end

  # Alias for:
  # [`eof?`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-eof-3F)
  sig {returns(::T.untyped)}
  def eof(); end

  # Returns true if the stream is at file which means there is no more data to
  # be read.
  #
  # Also aliased as:
  # [`eof`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-eof)
  sig {returns(::T.untyped)}
  def eof?(); end

  # Flushes buffered data to the SSLSocket.
  sig {returns(::T.untyped)}
  def flush(); end

  # Reads one character from the stream. Returns nil if called at end of file.
  sig {returns(::T.untyped)}
  def getc(); end

  # Reads the next "line" from the stream. Lines are separated by *eol*. If
  # *limit* is provided the result will not be longer than the given number of
  # bytes.
  #
  # *eol* may be a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # or [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html).
  #
  # Unlike
  # [`IO#gets`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-gets) the
  # line read will not be assigned to +$\_+.
  #
  # Unlike
  # [`IO#gets`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-gets) the
  # separator must be provided if a limit is provided.
  sig do
    params(
      eol: ::T.untyped,
      limit: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def gets(eol=T.unsafe(nil), limit=T.unsafe(nil)); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # Writes *args* to the stream.
  #
  # See [`IO#print`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-print)
  # for full details.
  sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def print(*args); end

  # Formats and writes to the stream converting parameters under control of the
  # format string.
  #
  # See
  # [`Kernel#sprintf`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-sprintf)
  # for format string details.
  sig do
    params(
      s: ::T.untyped,
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def printf(s, *args); end

  # Writes *args* to the stream along with a record separator.
  #
  # See [`IO#puts`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-puts)
  # for full details.
  sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def puts(*args); end

  # Reads *size* bytes from the stream. If *buf* is provided it must reference a
  # string which will receive the data.
  #
  # See [`IO#read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read)
  # for full details.
  sig do
    params(
      size: ::T.untyped,
      buf: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def read(size=T.unsafe(nil), buf=T.unsafe(nil)); end

  # Reads at most *maxlen* bytes in the non-blocking manner.
  #
  # When no data can be read without blocking it raises
  # [`OpenSSL::SSL::SSLError`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLError.html)
  # extended by
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # or
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html).
  #
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # means SSL needs to read internally so
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-read_nonblock)
  # should be called again when the underlying
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) is readable.
  #
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html)
  # means SSL needs to write internally so
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-read_nonblock)
  # should be called again after the underlying
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) is writable.
  #
  # [`OpenSSL::Buffering#read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-read_nonblock)
  # needs two rescue clause as follows:
  #
  # ```ruby
  # # emulates blocking read (readpartial).
  # begin
  #   result = ssl.read_nonblock(maxlen)
  # rescue IO::WaitReadable
  #   IO.select([io])
  #   retry
  # rescue IO::WaitWritable
  #   IO.select(nil, [io])
  #   retry
  # end
  # ```
  #
  # Note that one reason that
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-read_nonblock)
  # writes to the underlying [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # is when the peer requests a new TLS/SSL handshake. See openssl the FAQ for
  # more details. http://www.openssl.org/support/faq.html
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-read_nonblock)
  # should not raise an IO::Wait\*able exception, but return the symbol
  # `:wait_writable` or `:wait_readable` instead. At EOF, it will return `nil`
  # instead of raising
  # [`EOFError`](https://docs.ruby-lang.org/en/2.7.0/EOFError.html).
  sig do
    params(
      maxlen: ::T.untyped,
      buf: ::T.untyped,
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def read_nonblock(maxlen, buf=T.unsafe(nil), exception: T.unsafe(nil)); end

  # Reads a one-character string from the stream. Raises an
  # [`EOFError`](https://docs.ruby-lang.org/en/2.7.0/EOFError.html) at end of
  # file.
  sig {returns(::T.untyped)}
  def readchar(); end

  # Reads a line from the stream which is separated by *eol*.
  #
  # Raises [`EOFError`](https://docs.ruby-lang.org/en/2.7.0/EOFError.html) if at
  # end of file.
  sig do
    params(
      eol: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def readline(eol=T.unsafe(nil)); end

  # Reads lines from the stream which are separated by *eol*.
  #
  # See also
  # [`gets`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-gets)
  sig do
    params(
      eol: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def readlines(eol=T.unsafe(nil)); end

  # Reads at most *maxlen* bytes from the stream. If *buf* is provided it must
  # reference a string which will receive the data.
  #
  # See
  # [`IO#readpartial`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-readpartial)
  # for full details.
  sig do
    params(
      maxlen: ::T.untyped,
      buf: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def readpartial(maxlen, buf=T.unsafe(nil)); end

  # The "sync mode" of the SSLSocket.
  #
  # See [`IO#sync`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-sync)
  # for full details.
  sig {returns(::T.untyped)}
  def sync(); end

  # The "sync mode" of the SSLSocket.
  #
  # See [`IO#sync`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-sync)
  # for full details.
  sig do
    params(
      sync: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def sync=(sync); end

  # Pushes character *c* back onto the stream such that a subsequent buffered
  # character read will return it.
  #
  # Unlike
  # [`IO#getc`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-getc)
  # multiple bytes may be pushed back onto the stream.
  #
  # Has no effect on unbuffered reads (such as sysread).
  sig do
    params(
      c: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ungetc(c); end

  # Writes *s* to the stream. If the argument is not a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) it will be
  # converted using `.to_s` method. Returns the number of bytes written.
  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write(*s); end

  # Writes *s* in the non-blocking manner.
  #
  # If there is buffered data, it is flushed first. This may block.
  #
  # [`write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-write_nonblock)
  # returns number of bytes written to the SSL connection.
  #
  # When no data can be written without blocking it raises
  # [`OpenSSL::SSL::SSLError`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLError.html)
  # extended by
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # or
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html).
  #
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # means SSL needs to read internally so
  # [`write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-write_nonblock)
  # should be called again after the underlying
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) is readable.
  #
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html)
  # means SSL needs to write internally so
  # [`write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-write_nonblock)
  # should be called again after underlying
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) is writable.
  #
  # So
  # [`OpenSSL::Buffering#write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-write_nonblock)
  # needs two rescue clause as follows.
  #
  # ```ruby
  # # emulates blocking write.
  # begin
  #   result = ssl.write_nonblock(str)
  # rescue IO::WaitReadable
  #   IO.select([io])
  #   retry
  # rescue IO::WaitWritable
  #   IO.select(nil, [io])
  #   retry
  # end
  # ```
  #
  # Note that one reason that
  # [`write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-write_nonblock)
  # reads from the underlying
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) is when the peer
  # requests a new TLS/SSL handshake. See the openssl FAQ for more details.
  # http://www.openssl.org/support/faq.html
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-write_nonblock)
  # should not raise an IO::Wait\*able exception, but return the symbol
  # `:wait_writable` or `:wait_readable` instead.
  sig do
    params(
      s: ::T.untyped,
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write_nonblock(s, exception: T.unsafe(nil)); end
end

# Provides symmetric algorithms for encryption and decryption. The algorithms
# that are available depend on the particular version of
# [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) that is
# installed.
#
# ### Listing all supported algorithms
#
# A list of supported algorithms can be obtained by
#
# ```ruby
# puts OpenSSL::Cipher.ciphers
# ```
#
# ### Instantiating a [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html)
#
# There are several ways to create a
# [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html) instance.
# Generally, a
# [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html) algorithm
# is categorized by its name, the key length in bits and the cipher mode to be
# used. The most generic way to create a
# [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html) is the
# following
#
# ```ruby
# cipher = OpenSSL::Cipher.new('<name>-<key length>-<mode>')
# ```
#
# That is, a string consisting of the hyphenated concatenation of the individual
# components name, key length and mode. Either all uppercase or all lowercase
# strings may be used, for example:
#
# ```ruby
# cipher = OpenSSL::Cipher.new('AES-128-CBC')
# ```
#
# For each algorithm supported, there is a class defined under the
# [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html) class that
# goes by the name of the cipher, e.g. to obtain an instance of AES, you could
# also use
#
# ```ruby
# # these are equivalent
# cipher = OpenSSL::Cipher::AES.new(128, :CBC)
# cipher = OpenSSL::Cipher::AES.new(128, 'CBC')
# cipher = OpenSSL::Cipher::AES.new('128-CBC')
# ```
#
# Finally, due to its wide-spread use, there are also extra classes defined for
# the different key sizes of AES
#
# ```ruby
# cipher = OpenSSL::Cipher::AES128.new(:CBC)
# cipher = OpenSSL::Cipher::AES192.new(:CBC)
# cipher = OpenSSL::Cipher::AES256.new(:CBC)
# ```
#
# ### Choosing either encryption or decryption mode
#
# Encryption and decryption are often very similar operations for symmetric
# algorithms, this is reflected by not having to choose different classes for
# either operation, both can be done using the same class. Still, after
# obtaining a
# [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html) instance,
# we need to tell the instance what it is that we intend to do with it, so we
# need to call either
#
# ```ruby
# cipher.encrypt
# ```
#
# or
#
# ```ruby
# cipher.decrypt
# ```
#
# on the [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html)
# instance. This should be the first call after creating the instance, otherwise
# configuration that has already been set could get lost in the process.
#
# ### Choosing a key
#
# Symmetric encryption requires a key that is the same for the encrypting and
# for the decrypting party and after initial key establishment should be kept as
# private information. There are a lot of ways to create insecure keys, the most
# notable is to simply take a password as the key without processing the
# password further. A simple and secure way to create a key for a particular
# [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html) is
#
# ```ruby
# cipher = OpenSSL::AES256.new(:CFB)
# cipher.encrypt
# key = cipher.random_key # also sets the generated key on the Cipher
# ```
#
# If you absolutely need to use passwords as encryption keys, you should use
# Password-Based Key Derivation Function 2 (PBKDF2) by generating the key with
# the help of the functionality provided by
# [`OpenSSL::PKCS5.pbkdf2_hmac_sha1`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKCS5.html#method-i-pbkdf2_hmac_sha1)
# or
# [`OpenSSL::PKCS5.pbkdf2_hmac`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKCS5.html#method-i-pbkdf2_hmac).
#
# Although there is
# [`Cipher#pkcs5_keyivgen`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-pkcs5_keyivgen),
# its use is deprecated and it should only be used in legacy applications
# because it does not use the newer PKCS#5 v2 algorithms.
#
# ### Choosing an IV
#
# The cipher modes CBC, CFB, OFB and CTR all need an "initialization vector", or
# short, IV. ECB mode is the only mode that does not require an IV, but there is
# almost no legitimate use case for this mode because of the fact that it does
# not sufficiently hide plaintext patterns. Therefore
#
# **You should never use ECB mode unless you are absolutely sure that you
# absolutely need it**
#
# Because of this, you will end up with a mode that explicitly requires an IV in
# any case. Although the IV can be seen as public information, i.e. it may be
# transmitted in public once generated, it should still stay unpredictable to
# prevent certain kinds of attacks. Therefore, ideally
#
# **Always create a secure random IV for every encryption of your
# [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html)**
#
# A new, random IV should be created for every encryption of data. Think of the
# IV as a nonce (number used once) - it's public but random and unpredictable. A
# secure random IV can be created as follows
#
# ```ruby
# cipher = ...
# cipher.encrypt
# key = cipher.random_key
# iv = cipher.random_iv # also sets the generated IV on the Cipher
# ```
#
# Although the key is generally a random value, too, it is a bad choice as an
# IV. There are elaborate ways how an attacker can take advantage of such an IV.
# As a general rule of thumb, exposing the key directly or indirectly should be
# avoided at all cost and exceptions only be made with good reason.
#
# ### Calling [`Cipher#final`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-final)
#
# ECB (which should not be used) and CBC are both block-based modes. This means
# that unlike for the other streaming-based modes, they operate on fixed-size
# blocks of data, and therefore they require a "finalization" step to produce or
# correctly decrypt the last block of data by appropriately handling some form
# of padding. Therefore it is essential to add the output of
# [`OpenSSL::Cipher#final`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-final)
# to your encryption/decryption buffer or you will end up with decryption errors
# or truncated data.
#
# Although this is not really necessary for streaming-mode ciphers, it is still
# recommended to apply the same pattern of adding the output of
# [`Cipher#final`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-final)
# there as well - it also enables you to switch between modes more easily in the
# future.
#
# ### Encrypting and decrypting some data
#
# ```ruby
# data = "Very, very confidential data"
#
# cipher = OpenSSL::Cipher::AES.new(128, :CBC)
# cipher.encrypt
# key = cipher.random_key
# iv = cipher.random_iv
#
# encrypted = cipher.update(data) + cipher.final
# ...
# decipher = OpenSSL::Cipher::AES.new(128, :CBC)
# decipher.decrypt
# decipher.key = key
# decipher.iv = iv
#
# plain = decipher.update(encrypted) + decipher.final
#
# puts data == plain #=> true
# ```
#
# ### Authenticated Encryption and Associated [`Data`](https://docs.ruby-lang.org/en/2.7.0/Data.html) (AEAD)
#
# If the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) version
# used supports it, an Authenticated Encryption mode (such as GCM or CCM) should
# always be preferred over any unauthenticated mode. Currently,
# [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) supports AE only
# in combination with Associated
# [`Data`](https://docs.ruby-lang.org/en/2.7.0/Data.html) (AEAD) where
# additional associated data is included in the encryption process to compute a
# tag at the end of the encryption. This tag will also be used in the decryption
# process and by verifying its validity, the authenticity of a given ciphertext
# is established.
#
# This is superior to unauthenticated modes in that it allows to detect if
# somebody effectively changed the ciphertext after it had been encrypted. This
# prevents malicious modifications of the ciphertext that could otherwise be
# exploited to modify ciphertexts in ways beneficial to potential attackers.
#
# An associated data is used where there is additional information, such as
# headers or some metadata, that must be also authenticated but not necessarily
# need to be encrypted. If no associated data is needed for encryption and later
# decryption, the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
# library still requires a value to be set - "" may be used in case none is
# available.
#
# An example using the GCM (Galois/Counter Mode). You have 16 bytes *key*, 12
# bytes (96 bits) *nonce* and the associated data *auth\_data*. Be sure not to
# reuse the *key* and *nonce* pair. Reusing an nonce ruins the security
# guarantees of GCM mode.
#
# ```ruby
# cipher = OpenSSL::Cipher::AES.new(128, :GCM).encrypt
# cipher.key = key
# cipher.iv = nonce
# cipher.auth_data = auth_data
#
# encrypted = cipher.update(data) + cipher.final
# tag = cipher.auth_tag # produces 16 bytes tag by default
# ```
#
# Now you are the receiver. You know the *key* and have received *nonce*,
# *auth\_data*, *encrypted* and *tag* through an untrusted network. Note that
# GCM accepts an arbitrary length tag between 1 and 16 bytes. You may
# additionally need to check that the received tag has the correct length, or
# you allow attackers to forge a valid single byte tag for the tampered
# ciphertext with a probability of 1/256.
#
# ```ruby
# raise "tag is truncated!" unless tag.bytesize == 16
# decipher = OpenSSL::Cipher::AES.new(128, :GCM).decrypt
# decipher.key = key
# decipher.iv = nonce
# decipher.auth_tag = tag
# decipher.auth_data = auth_data
#
# decrypted = decipher.update(encrypted) + decipher.final
#
# puts data == decrypted #=> true
# ```
class OpenSSL::Cipher
  # Sets the cipher's additional authenticated data. This field must be set when
  # using AEAD cipher modes such as GCM or CCM. If no associated data shall be
  # used, this method must **still** be called with a value of "". The contents
  # of this field should be non-sensitive data which will be added to the
  # ciphertext to generate the authentication tag which validates the contents
  # of the ciphertext.
  #
  # The AAD must be set prior to encryption or decryption. In encryption mode,
  # it must be set after calling
  # [`Cipher#encrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-encrypt)
  # and setting
  # [`Cipher#key=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-key-3D)
  # and
  # [`Cipher#iv=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-iv-3D).
  # When decrypting, the authenticated data must be set after key, iv and
  # especially **after** the authentication tag has been set. I.e. set it only
  # after calling
  # [`Cipher#decrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-decrypt),
  # [`Cipher#key=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-key-3D),
  # [`Cipher#iv=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-iv-3D)
  # and
  # [`Cipher#auth_tag=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-auth_tag-3D)
  # first.
  sig do
    params(
      auth_data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def auth_data=(auth_data); end

  # Gets the authentication tag generated by Authenticated Encryption
  # [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html) modes
  # (GCM for example). This tag may be stored along with the ciphertext, then
  # set on the decryption cipher to authenticate the contents of the ciphertext
  # against changes. If the optional integer parameter *tag\_len* is given, the
  # returned tag will be *tag\_len* bytes long. If the parameter is omitted, the
  # default length of 16 bytes or the length previously set by
  # [`auth_tag_len=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-auth_tag_len-3D)
  # will be used. For maximum security, the longest possible should be chosen.
  #
  # The tag may only be retrieved after calling
  # [`Cipher#final`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-final).
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def auth_tag(*arg0); end

  # Sets the authentication tag to verify the integrity of the ciphertext. This
  # can be called only when the cipher supports AE. The tag must be set after
  # calling
  # [`Cipher#decrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-decrypt),
  # [`Cipher#key=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-key-3D)
  # and
  # [`Cipher#iv=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-iv-3D),
  # but before calling
  # [`Cipher#final`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-final).
  # After all decryption is performed, the tag is verified automatically in the
  # call to
  # [`Cipher#final`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-final).
  #
  # For OCB mode, the tag length must be supplied with
  # [`auth_tag_len=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-auth_tag_len-3D)
  # beforehand.
  sig do
    params(
      auth_tag: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def auth_tag=(auth_tag); end

  # Sets the length of the authentication tag to be generated or to be given for
  # AEAD ciphers that requires it as in input parameter. Note that not all AEAD
  # ciphers support this method.
  #
  # In OCB mode, the length must be supplied both when encrypting and when
  # decrypting, and must be before specifying an IV.
  sig do
    params(
      auth_tag_len: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def auth_tag_len=(auth_tag_len); end

  # Indicated whether this
  # [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html) instance
  # uses an Authenticated Encryption mode.
  sig {returns(::T.untyped)}
  def authenticated?(); end

  # Returns the size in bytes of the blocks on which this
  # [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html) operates
  # on.
  sig {returns(::T.untyped)}
  def block_size(); end

  # Initializes the
  # [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html) for
  # decryption.
  #
  # Make sure to call
  # [`Cipher#encrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-encrypt)
  # or
  # [`Cipher#decrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-decrypt)
  # before using any of the following methods:
  #
  #     [`key=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-key-3D), [`iv=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-iv-3D), [`random_key`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-random_key), [`random_iv`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-random_iv), [`pkcs5_keyivgen`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-pkcs5_keyivgen)
  # :
  #
  # Internally calls EVP\_CipherInit\_ex(ctx, NULL, NULL, NULL, NULL, 0).
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def decrypt(*arg0); end

  # Initializes the
  # [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html) for
  # encryption.
  #
  # Make sure to call
  # [`Cipher#encrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-encrypt)
  # or
  # [`Cipher#decrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-decrypt)
  # before using any of the following methods:
  #
  #     [`key=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-key-3D), [`iv=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-iv-3D), [`random_key`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-random_key), [`random_iv`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-random_iv), [`pkcs5_keyivgen`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-pkcs5_keyivgen)
  # :
  #
  # Internally calls EVP\_CipherInit\_ex(ctx, NULL, NULL, NULL, NULL, 1).
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def encrypt(*arg0); end

  # Returns the remaining data held in the cipher object. Further calls to
  # [`Cipher#update`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-update)
  # or
  # [`Cipher#final`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-final)
  # will return garbage. This call should always be made as the last call of an
  # encryption or decryption operation, after having fed the entire plaintext or
  # ciphertext to the
  # [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html)
  # instance.
  #
  # If an authenticated cipher was used, a CipherError is raised if the tag
  # could not be authenticated successfully. Only call this method after setting
  # the authentication tag and passing the entire contents of the ciphertext
  # into the cipher.
  sig {returns(::T.untyped)}
  def final(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(arg0); end

  # Sets the cipher IV. Please note that since you should never be using ECB
  # mode, an IV is always explicitly required and should be set prior to
  # encryption. The IV itself can be safely transmitted in public, but it should
  # be unpredictable to prevent certain kinds of attacks. You may use
  # [`Cipher#random_iv`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-random_iv)
  # to create a secure random IV.
  #
  # Only call this method after calling
  # [`Cipher#encrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-encrypt)
  # or
  # [`Cipher#decrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-decrypt).
  sig do
    params(
      iv: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def iv=(iv); end

  # Returns the expected length in bytes for an IV for this
  # [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html).
  sig {returns(::T.untyped)}
  def iv_len(); end

  # Sets the IV/nonce length of the
  # [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html).
  # Normally block ciphers don't allow changing the IV length, but some make use
  # of IV for 'nonce'. You may need this for interoperability with other
  # applications.
  sig do
    params(
      iv_len: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def iv_len=(iv_len); end

  # Sets the cipher key. To generate a key, you should either use a secure
  # random byte string or, if the key is to be derived from a password, you
  # should rely on PBKDF2 functionality provided by
  # [`OpenSSL::PKCS5`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKCS5.html).
  # To generate a secure random-based key,
  # [`Cipher#random_key`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-random_key)
  # may be used.
  #
  # Only call this method after calling
  # [`Cipher#encrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-encrypt)
  # or
  # [`Cipher#decrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-decrypt).
  sig do
    params(
      key: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def key=(key); end

  # Returns the key length in bytes of the
  # [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html).
  sig {returns(::T.untyped)}
  def key_len(); end

  # Sets the key length of the cipher. If the cipher is a fixed length cipher
  # then attempting to set the key length to any value other than the fixed
  # value is an error.
  #
  # Under normal circumstances you do not need to call this method (and probably
  # shouldn't).
  #
  # See EVP\_CIPHER\_CTX\_set\_key\_length for further information.
  sig do
    params(
      key_len: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def key_len=(key_len); end

  # Returns the name of the cipher which may differ slightly from the original
  # name provided.
  sig {returns(::T.untyped)}
  def name(); end

  # Enables or disables padding. By default encryption operations are padded
  # using standard block padding and the padding is checked and removed when
  # decrypting. If the pad parameter is zero then no padding is performed, the
  # total amount of data encrypted or decrypted must then be a multiple of the
  # block size or an error will occur.
  #
  # See EVP\_CIPHER\_CTX\_set\_padding for further information.
  sig do
    params(
      padding: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def padding=(padding); end

  # Generates and sets the key/IV based on a password.
  #
  # **WARNING**: This method is only PKCS5 v1.5 compliant when using RC2,
  # RC4-40, or DES with MD5 or SHA1. Using anything else (like AES) will
  # generate the key/iv using an
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) specific
  # method. This method is deprecated and should no longer be used. Use a PKCS5
  # v2 key generation method from
  # [`OpenSSL::PKCS5`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKCS5.html)
  # instead.
  #
  # ### Parameters
  # *   *salt* must be an 8 byte string if provided.
  # *   *iterations* is an integer with a default of 2048.
  # *   *digest* is a
  #     [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html) object that
  #     defaults to 'MD5'
  #
  #
  # A minimum of 1000 iterations is recommended.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def pkcs5_keyivgen(*arg0); end

  # Generate a random IV with
  # [`OpenSSL::Random.random_bytes`](https://docs.ruby-lang.org/en/2.7.0/Random.html#method-c-random_bytes)
  # and sets it to the cipher, and returns it.
  #
  # You must call
  # [`encrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-encrypt)
  # or
  # [`decrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-decrypt)
  # before calling this method.
  sig {returns(::T.untyped)}
  def random_iv(); end

  # Generate a random key with
  # [`OpenSSL::Random.random_bytes`](https://docs.ruby-lang.org/en/2.7.0/Random.html#method-c-random_bytes)
  # and sets it to the cipher, and returns it.
  #
  # You must call
  # [`encrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-encrypt)
  # or
  # [`decrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-decrypt)
  # before calling this method.
  sig {returns(::T.untyped)}
  def random_key(); end

  # Fully resets the internal state of the
  # [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html). By
  # using this, the same
  # [`Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html) instance
  # may be used several times for encryption or decryption tasks.
  #
  # Internally calls EVP\_CipherInit\_ex(ctx, NULL, NULL, NULL, NULL, -1).
  sig {returns(::T.untyped)}
  def reset(); end

  # Encrypts data in a streaming fashion. Hand consecutive blocks of data to the
  # [`update`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-update)
  # method in order to encrypt it. Returns the encrypted data chunk. When done,
  # the output of
  # [`Cipher#final`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html#method-i-final)
  # should be additionally added to the result.
  #
  # If *buffer* is given, the encryption/decryption result will be written to
  # it. *buffer* will be resized automatically.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def update(*arg0); end

  # Returns the names of all available ciphers in an array.
  sig {returns(::T.untyped)}
  def self.ciphers(); end
end

class OpenSSL::Cipher::AES < OpenSSL::Cipher
  sig do
    params(
      args: ::T.untyped,
    )
    .void
  end
  def initialize(*args); end
end

class OpenSSL::Cipher::AES128 < OpenSSL::Cipher
  sig do
    params(
      mode: ::T.untyped,
    )
    .void
  end
  def initialize(mode=T.unsafe(nil)); end
end

class OpenSSL::Cipher::AES192 < OpenSSL::Cipher
  sig do
    params(
      mode: ::T.untyped,
    )
    .void
  end
  def initialize(mode=T.unsafe(nil)); end
end

class OpenSSL::Cipher::AES256 < OpenSSL::Cipher
  sig do
    params(
      mode: ::T.untyped,
    )
    .void
  end
  def initialize(mode=T.unsafe(nil)); end
end

class OpenSSL::Cipher::BF < OpenSSL::Cipher
  sig do
    params(
      args: ::T.untyped,
    )
    .void
  end
  def initialize(*args); end
end

class OpenSSL::Cipher::CAST5 < OpenSSL::Cipher
  sig do
    params(
      args: ::T.untyped,
    )
    .void
  end
  def initialize(*args); end
end

class OpenSSL::Cipher::CipherError < OpenSSL::OpenSSLError
end

class OpenSSL::Cipher::DES < OpenSSL::Cipher
  sig do
    params(
      args: ::T.untyped,
    )
    .void
  end
  def initialize(*args); end
end

class OpenSSL::Cipher::IDEA < OpenSSL::Cipher
  sig do
    params(
      args: ::T.untyped,
    )
    .void
  end
  def initialize(*args); end
end

class OpenSSL::Cipher::RC2 < OpenSSL::Cipher
  sig do
    params(
      args: ::T.untyped,
    )
    .void
  end
  def initialize(*args); end
end

class OpenSSL::Cipher::RC4 < OpenSSL::Cipher
  sig do
    params(
      args: ::T.untyped,
    )
    .void
  end
  def initialize(*args); end
end

class OpenSSL::Cipher::RC5 < OpenSSL::Cipher
  sig do
    params(
      args: ::T.untyped,
    )
    .void
  end
  def initialize(*args); end
end

# # [`OpenSSL::Config`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Config.html)
#
# Configuration for the openssl library.
#
# Many system's installation of openssl library will depend on your system
# configuration. See the value of
# [`OpenSSL::Config::DEFAULT_CONFIG_FILE`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Config.html#DEFAULT_CONFIG_FILE)
# for the location of the file for your host.
#
# See also http://www.openssl.org/docs/apps/config.html
class OpenSSL::Config
  include ::Enumerable
  Elem = type_member {{fixed: T.untyped}}

  # The default system configuration file for openssl
  DEFAULT_CONFIG_FILE = ::T.let(nil, ::T.untyped)

  # Get a specific *section* from the current configuration
  #
  # Given the following configurating file being loaded:
  #
  # ```ruby
  # config = OpenSSL::Config.load('foo.cnf')
  #   #=> #<OpenSSL::Config sections=["default"]>
  # puts config.to_s
  #   #=> [ default ]
  #   #   foo=bar
  # ```
  #
  # You can get a hash of the specific section like so:
  #
  # ```ruby
  # config['default']
  #   #=> {"foo"=>"bar"}
  # ```
  sig do
    params(
      section: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def [](section); end

  # Sets a specific *section* name with a
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) *pairs*.
  #
  # Given the following configuration being created:
  #
  # ```ruby
  # config = OpenSSL::Config.new
  #   #=> #<OpenSSL::Config sections=[]>
  # config['default'] = {"foo"=>"bar","baz"=>"buz"}
  #   #=> {"foo"=>"bar", "baz"=>"buz"}
  # puts config.to_s
  #   #=> [ default ]
  #   #   foo=bar
  #   #   baz=buz
  # ```
  #
  # It's important to note that this will essentially merge any of the keys in
  # *pairs* with the existing *section*. For example:
  #
  # ```ruby
  # config['default']
  #   #=> {"foo"=>"bar", "baz"=>"buz"}
  # config['default'] = {"foo" => "changed"}
  #   #=> {"foo"=>"changed"}
  # config['default']
  #   #=> {"foo"=>"changed", "baz"=>"buz"}
  # ```
  sig do
    params(
      section: ::T.untyped,
      pairs: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def []=(section, pairs); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the target *key* with
  # a given *value* under a specific *section*.
  #
  # Given the following configurating file being loaded:
  #
  # ```ruby
  # config = OpenSSL::Config.load('foo.cnf')
  #   #=> #<OpenSSL::Config sections=["default"]>
  # puts config.to_s
  #   #=> [ default ]
  #   #   foo=bar
  # ```
  #
  # You can set the value of *foo* under the *default* section to a new value:
  #
  # ```ruby
  # config.add_value('default', 'foo', 'buzz')
  #   #=> "buzz"
  # puts config.to_s
  #   #=> [ default ]
  #   #   foo=buzz
  # ```
  sig do
    params(
      section: ::T.untyped,
      key: ::T.untyped,
      value: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_value(section, key, value); end

  sig {returns(::T.untyped)}
  def data(); end

  # For a block.
  #
  # Receive the section and its pairs for the current configuration.
  #
  # ```ruby
  # config.each do |section, key, value|
  #   # ...
  # end
  # ```
  sig {returns(::T.untyped)}
  def each(&blk); end

  # Gets the value of *key* from the given *section*
  #
  # Given the following configurating file being loaded:
  #
  # ```ruby
  # config = OpenSSL::Config.load('foo.cnf')
  #   #=> #<OpenSSL::Config sections=["default"]>
  # puts config.to_s
  #   #=> [ default ]
  #   #   foo=bar
  # ```
  #
  # You can get a specific value from the config if you know the *section* and
  # *key* like so:
  #
  # ```ruby
  # config.get_value('default','foo')
  #   #=> "bar"
  # ```
  sig do
    params(
      section: ::T.untyped,
      key: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def get_value(section, key); end

  sig do
    params(
      filename: ::T.untyped,
    )
    .void
  end
  def initialize(filename=T.unsafe(nil)); end

  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) representation
  # of this configuration object, including the class name and its sections.
  sig {returns(::T.untyped)}
  def inspect(); end

  sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def section(name); end

  # Get the names of all sections in the current configuration
  sig {returns(::T.untyped)}
  def sections(); end

  # Get the parsable form of the current configuration
  #
  # Given the following configuration being created:
  #
  # ```ruby
  # config = OpenSSL::Config.new
  #   #=> #<OpenSSL::Config sections=[]>
  # config['default'] = {"foo"=>"bar","baz"=>"buz"}
  #   #=> {"foo"=>"bar", "baz"=>"buz"}
  # puts config.to_s
  #   #=> [ default ]
  #   #   foo=bar
  #   #   baz=buz
  # ```
  #
  # You can parse get the serialized configuration using
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Config.html#method-i-to_s)
  # and then parse it later:
  #
  # ```ruby
  # serialized_config = config.to_s
  # # much later...
  # new_config = OpenSSL::Config.parse(serialized_config)
  #   #=> #<OpenSSL::Config sections=["default"]>
  # puts new_config
  #   #=> [ default ]
  #       foo=bar
  #       baz=buz
  # ```
  sig {returns(::T.untyped)}
  def to_s(); end

  sig do
    params(
      arg1: ::T.untyped,
      arg2: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def value(arg1, arg2=T.unsafe(nil)); end

  sig do
    params(
      data: ::T.untyped,
      section: ::T.untyped,
      key: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.get_key_string(data, section, key); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.load(*arg0); end

  # Parses a given *string* as a blob that contains configuration for
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html).
  #
  # If the source of the [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) is
  # a file, then consider using parse\_config.
  sig do
    params(
      string: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.parse(string); end

  # Parses the configuration data read from *io*, see also parse.
  #
  # Raises a ConfigError on invalid configuration data.
  sig do
    params(
      io: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.parse_config(io); end
end

# General error for openssl library configuration files. Including formatting,
# parsing errors, etc.
class OpenSSL::ConfigError < OpenSSL::OpenSSLError
end

# [`OpenSSL::Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html)
# allows you to compute message digests (sometimes interchangeably called
# "hashes") of arbitrary data that are cryptographically secure, i.e. a
# [`Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html) implements
# a secure one-way function.
#
# One-way functions offer some useful properties. E.g. given two distinct inputs
# the probability that both yield the same output is highly unlikely. Combined
# with the fact that every message digest algorithm has a fixed-length output of
# just a few bytes, digests are often used to create unique identifiers for
# arbitrary data. A common example is the creation of a unique id for binary
# documents that are stored in a database.
#
# Another useful characteristic of one-way functions (and thus the name) is that
# given a digest there is no indication about the original data that produced
# it, i.e. the only way to identify the original input is to "brute-force"
# through every possible combination of inputs.
#
# These characteristics make one-way functions also ideal companions for public
# key signature algorithms: instead of signing an entire document, first a hash
# of the document is produced with a considerably faster message digest
# algorithm and only the few bytes of its output need to be signed using the
# slower public key algorithm. To validate the integrity of a signed document,
# it suffices to re-compute the hash and verify that it is equal to that in the
# signature.
#
# Among the supported message digest algorithms are:
# *   SHA, SHA1, SHA224, SHA256, SHA384 and SHA512
# *   MD2, MD4, MDC2 and MD5
# *   RIPEMD160
# *   DSS, DSS1 (Pseudo algorithms to be used for DSA signatures. DSS is equal
#     to SHA and DSS1 is equal to SHA1)
#
#
# For each of these algorithms, there is a sub-class of
# [`Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html) that can
# be instantiated as simply as e.g.
#
# ```ruby
# digest = OpenSSL::Digest::SHA1.new
# ```
#
# ### Mapping between [`Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html) class and sn/ln
#
# The sn (short names) and ln (long names) are defined in <openssl/object.h> and
# <openssl/obj\_mac.h>. They are textual representations of ASN.1 OBJECT
# IDENTIFIERs. Each supported digest algorithm has an OBJECT IDENTIFIER
# associated to it and those again have short/long names assigned to them. E.g.
# the OBJECT IDENTIFIER for SHA-1 is 1.3.14.3.2.26 and its sn is "SHA1" and its
# ln is "sha1".
# #### MD2
# *   sn: MD2
# *   ln: md2
#
# #### MD4
# *   sn: MD4
# *   ln: md4
#
# #### MD5
# *   sn: MD5
# *   ln: md5
#
# #### SHA
# *   sn: SHA
# *   ln: SHA
#
# #### SHA-1
# *   sn: SHA1
# *   ln: sha1
#
# #### SHA-224
# *   sn: SHA224
# *   ln: sha224
#
# #### SHA-256
# *   sn: SHA256
# *   ln: sha256
#
# #### SHA-384
# *   sn: SHA384
# *   ln: sha384
#
# #### SHA-512
# *   sn: SHA512
# *   ln: sha512
#
#
# "Breaking" a message digest algorithm means defying its one-way function
# characteristics, i.e. producing a collision or finding a way to get to the
# original data by means that are more efficient than brute-forcing etc. Most of
# the supported digest algorithms can be considered broken in this sense, even
# the very popular MD5 and SHA1 algorithms. Should security be your highest
# concern, then you should probably rely on SHA224, SHA256, SHA384 or SHA512.
#
# ### Hashing a file
#
# ```ruby
# data = File.read('document')
# sha256 = OpenSSL::Digest::SHA256.new
# digest = sha256.digest(data)
# ```
#
# ### Hashing several pieces of data at once
#
# ```ruby
# data1 = File.read('file1')
# data2 = File.read('file2')
# data3 = File.read('file3')
# sha256 = OpenSSL::Digest::SHA256.new
# sha256 << data1
# sha256 << data2
# sha256 << data3
# digest = sha256.digest
# ```
#
# ### Reuse a [`Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html) instance
#
# ```ruby
# data1 = File.read('file1')
# sha256 = OpenSSL::Digest::SHA256.new
# digest1 = sha256.digest(data1)
#
# data2 = File.read('file2')
# sha256.reset
# digest2 = sha256.digest(data2)
# ```
class OpenSSL::Digest < Digest::Class
  # Alias for: update
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def <<(arg0); end

  # Returns the block length of the digest algorithm, i.e. the length in bytes
  # of an individual block. Most modern algorithms partition a message to be
  # digested into a sequence of fix-sized blocks that are processed
  # consecutively.
  #
  # ### Example
  #
  # ```ruby
  # digest = OpenSSL::Digest::SHA1.new
  # puts digest.block_length # => 64
  # ```
  sig {returns(::T.untyped)}
  def block_length(); end

  # Returns the output size of the digest, i.e. the length in bytes of the final
  # message digest result.
  #
  # ### Example
  #
  # ```ruby
  # digest = OpenSSL::Digest::SHA1.new
  # puts digest.digest_length # => 20
  # ```
  sig {returns(::T.untyped)}
  def digest_length(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # Returns the sn of this
  # [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html) algorithm.
  #
  # ### Example
  #
  # ```ruby
  # digest = OpenSSL::Digest::SHA512.new
  # puts digest.name # => SHA512
  # ```
  sig {returns(::T.untyped)}
  def name(); end

  # Resets the [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html) in
  # the sense that any Digest#update that has been performed is abandoned and
  # the [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html) is set to
  # its initial state again.
  sig {returns(::T.untyped)}
  def reset(); end

  # Not every message digest can be computed in one single pass. If a message
  # digest is to be computed from several subsequent sources, then each may be
  # passed individually to the
  # [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html) instance.
  #
  # ### Example
  #
  # ```ruby
  # digest = OpenSSL::Digest::SHA256.new
  # digest.update('First input')
  # digest << 'Second input' # equivalent to digest.update('Second input')
  # result = digest.digest
  # ```
  #
  #
  # Also aliased as: <<
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def update(arg0); end

  # Return the hash value computed with *name*
  # [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html). *name* is
  # either the long name or short name of a supported digest algorithm.
  #
  # ### Examples
  #
  # ```ruby
  # OpenSSL::Digest.digest("SHA256", "abc")
  # ```
  #
  # which is equivalent to:
  #
  # ```ruby
  # OpenSSL::Digest::SHA256.digest("abc")
  # ```
  sig do
    params(
      name: ::T.untyped,
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(name, data); end
end

class OpenSSL::Digest::DSS < OpenSSL::Digest
  sig do
    params(
      data: ::T.untyped,
    )
    .void
  end
  def initialize(data=T.unsafe(nil)); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(data); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(data); end
end

class OpenSSL::Digest::DSS1 < OpenSSL::Digest
  sig do
    params(
      data: ::T.untyped,
    )
    .void
  end
  def initialize(data=T.unsafe(nil)); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(data); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(data); end
end

# Generic [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html)
# class that is raised if an error occurs during a
# [`Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html) operation.
class OpenSSL::Digest::DigestError < OpenSSL::OpenSSLError
end

# Provides functionality of various KDFs (key derivation function).
#
# KDF is typically used for securely deriving arbitrary length symmetric
# keys to be used with an OpenSSL::Cipher from passwords. Another use case
# is for storing passwords: Due to the ability to tweak the effort of
# computation by increasing the iteration count, computation can be slowed
# down artificially in order to render possible attacks infeasible.
#
# Currently, OpenSSL::KDF provides implementations for the following KDF:
#
# * PKCS #5 PBKDF2 (Password-Based Key Derivation Function 2) in
#   combination with HMAC
# * scrypt
# * HKDF
#
# == Examples
# === Generating a 128 bit key for a Cipher (e.g. AES)
#   pass = "secret"
#   salt = OpenSSL::Random.random_bytes(16)
#   iter = 20_000
#   key_len = 16
#   key = OpenSSL::KDF.pbkdf2_hmac(pass, salt: salt, iterations: iter,
#                                  length: key_len, hash: "sha1")
#
# === Storing Passwords
#   pass = "secret"
#   # store this with the generated value
#   salt = OpenSSL::Random.random_bytes(16)
#   iter = 20_000
#   hash = OpenSSL::Digest::SHA256.new
#   len = hash.digest_length
#   # the final value to be stored
#   value = OpenSSL::KDF.pbkdf2_hmac(pass, salt: salt, iterations: iter,
#                                    length: len, hash: hash)
#
# == Important Note on Checking Passwords
# When comparing passwords provided by the user with previously stored
# values, a common mistake made is comparing the two values using "==".
# Typically, "==" short-circuits on evaluation, and is therefore
# vulnerable to timing attacks. The proper way is to use a method that
# always takes the same amount of time when comparing two values, thus
# not leaking any information to potential attackers. To compare two
# values, the following could be used:
#
#   def eql_time_cmp(a, b)
#     unless a.length == b.length
#       return false
#     end
#     cmp = b.bytes
#     result = 0
#     a.bytes.each_with_index {|c,i|
#       result |= c ^ cmp[i]
#     }
#     result == 0
#   end
#
# Please note that the premature return in case of differing lengths
# typically does not leak valuable information - when using PBKDF2, the
# length of the values to be compared is of fixed size.
module OpenSSL::KDF
  # PKCS #5 PBKDF2 (Password-Based Key Derivation Function 2) in combination
  # with HMAC. Takes _pass_, _salt_ and _iterations_, and then derives a key
  # of _length_ bytes.
  #
  # For more information about PBKDF2, see RFC 2898 Section 5.2
  # (https://tools.ietf.org/html/rfc2898#section-5.2).
  #
  # === Parameters
  # pass       :: The passphrase.
  # salt       :: The salt. Salts prevent attacks based on dictionaries of common
  #               passwords and attacks based on rainbow tables. It is a public
  #               value that can be safely stored along with the password (e.g.
  #               if the derived value is used for password storage).
  # iterations :: The iteration count. This provides the ability to tune the
  #               algorithm. It is better to use the highest count possible for
  #               the maximum resistance to brute-force attacks.
  # length     :: The desired length of the derived key in octets.
  # hash       :: The hash algorithm used with HMAC for the PRF. May be a String
  #               representing the algorithm name, or an instance of
  #               OpenSSL::Digest.
  sig do
    params(
      pass: String,
      salt: String,
      iterations: Integer,
      length: Integer,
      hash: T.any(String, OpenSSL::Digest)
    )
    .returns(String)
  end
  def self.pbkdf2_hmac(pass, salt:, iterations:, length:, hash:); end

  # Derives a key from _pass_ using given parameters with the scrypt
  # password-based key derivation function. The result can be used for password
  # storage.
  #
  # scrypt is designed to be memory-hard and more secure against brute-force
  # attacks using custom hardwares than alternative KDFs such as PBKDF2 or
  # bcrypt.
  #
  # The keyword arguments _N_, _r_ and _p_ can be used to tune scrypt. RFC 7914
  # (published on 2016-08, https://tools.ietf.org/html/rfc7914#section-2) states
  # that using values r=8 and p=1 appears to yield good results.
  #
  # See RFC 7914 (https://tools.ietf.org/html/rfc7914) for more information.
  #
  # === Parameters
  # pass   :: Passphrase.
  # salt   :: Salt.
  # N      :: CPU/memory cost parameter. This must be a power of 2.
  # r      :: Block size parameter.
  # p      :: Parallelization parameter.
  # length :: Length in octets of the derived key.
  #
  # === Example
  #   pass = "password"
  #   salt = SecureRandom.random_bytes(16)
  #   dk = OpenSSL::KDF.scrypt(pass, salt: salt, N: 2**14, r: 8, p: 1, length: 32)
  #   p dk #=> "\xDA\xE4\xE2...\x7F\xA1\x01T"
  sig do
    params(
      pass: String,
      kwargs: T.untyped
    )
    .returns(String)
  end
  def self.scrypt(pass, **kwargs); end

  # HMAC-based Extract-and-Expand Key Derivation Function (HKDF) as specified in
  # {RFC 5869}[https://tools.ietf.org/html/rfc5869].
  #
  # New in OpenSSL 1.1.0.
  #
  # === Parameters
  # _ikm_::
  #   The input keying material.
  # _salt_::
  #   The salt.
  # _info_::
  #   The context and application specific information.
  # _length_::
  #   The output length in octets. Must be <= <tt>255 * HashLen</tt>, where
  #   HashLen is the length of the hash function output in octets.
  # _hash_::
  #   The hash function.
  sig do
      params(
        ikm: String,
        salt: String,
        info: String,
        length: Integer,
        hash: T.any(String, OpenSSL::Digest)
      )
      .returns(String)
    end
    def self.hkdf(ikm, salt:, info:, length:, hash:); end
end

# Generic exception class raised if an error occurs in OpenSSL::KDF module.
class OpenSSL::KDF::KDFError < OpenSSL::OpenSSLError
end

class OpenSSL::Digest::MD2 < OpenSSL::Digest
  sig do
    params(
      data: ::T.untyped,
    )
    .void
  end
  def initialize(data=T.unsafe(nil)); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(data); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(data); end
end

class OpenSSL::Digest::MD4 < OpenSSL::Digest
  sig do
    params(
      data: ::T.untyped,
    )
    .void
  end
  def initialize(data=T.unsafe(nil)); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(data); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(data); end
end

class OpenSSL::Digest::MD5 < OpenSSL::Digest
  sig do
    params(
      data: ::T.untyped,
    )
    .void
  end
  def initialize(data=T.unsafe(nil)); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(data); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(data); end
end

class OpenSSL::Digest::MDC2 < OpenSSL::Digest
  sig do
    params(
      data: ::T.untyped,
    )
    .void
  end
  def initialize(data=T.unsafe(nil)); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(data); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(data); end
end

class OpenSSL::Digest::RIPEMD160 < OpenSSL::Digest
  sig do
    params(
      data: ::T.untyped,
    )
    .void
  end
  def initialize(data=T.unsafe(nil)); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(data); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(data); end
end

class OpenSSL::Digest::SHA < OpenSSL::Digest
  sig do
    params(
      data: ::T.untyped,
    )
    .void
  end
  def initialize(data=T.unsafe(nil)); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(data); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(data); end
end

class OpenSSL::Digest::SHA1 < OpenSSL::Digest
  sig do
    params(
      data: ::T.untyped,
    )
    .void
  end
  def initialize(data=T.unsafe(nil)); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(data); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(data); end
end

class OpenSSL::Digest::SHA224 < OpenSSL::Digest
  sig do
    params(
      data: ::T.untyped,
    )
    .void
  end
  def initialize(data=T.unsafe(nil)); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(data); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(data); end
end

class OpenSSL::Digest::SHA256 < OpenSSL::Digest
  sig do
    params(
      data: ::T.untyped,
    )
    .void
  end
  def initialize(data=T.unsafe(nil)); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(data); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(data); end
end

class OpenSSL::Digest::SHA384 < OpenSSL::Digest
  sig do
    params(
      data: ::T.untyped,
    )
    .void
  end
  def initialize(data=T.unsafe(nil)); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(data); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(data); end
end

class OpenSSL::Digest::SHA512 < OpenSSL::Digest
  sig do
    params(
      data: ::T.untyped,
    )
    .void
  end
  def initialize(data=T.unsafe(nil)); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(data); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(data); end
end

# This class is the access to openssl's ENGINE cryptographic module
# implementation.
#
# See also, https://www.openssl.org/docs/crypto/engine.html
class OpenSSL::Engine
  METHOD_ALL = ::T.let(nil, ::T.untyped)
  METHOD_CIPHERS = ::T.let(nil, ::T.untyped)
  METHOD_DH = ::T.let(nil, ::T.untyped)
  METHOD_DIGESTS = ::T.let(nil, ::T.untyped)
  METHOD_DSA = ::T.let(nil, ::T.untyped)
  METHOD_NONE = ::T.let(nil, ::T.untyped)
  METHOD_RAND = ::T.let(nil, ::T.untyped)
  METHOD_RSA = ::T.let(nil, ::T.untyped)

  # Returns a new instance of
  # [`OpenSSL::Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html)
  # by *name*, if it is available in this engine.
  #
  # An EngineError will be raised if the cipher is unavailable.
  #
  # ```
  # e = OpenSSL::Engine.by_id("openssl")
  #  => #<OpenSSL::Engine id="openssl" name="Software engine support">
  # e.cipher("RC4")
  #  => #<OpenSSL::Cipher:0x007fc5cacc3048>
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def cipher(arg0); end

  # Returns an array of command definitions for the current engine
  sig {returns(::T.untyped)}
  def cmds(); end

  # Sends the given *command* to this engine.
  #
  # Raises an EngineError if the command fails.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ctrl_cmd(*arg0); end

  # Returns a new instance of
  # [`OpenSSL::Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html)
  # by *name*.
  #
  # Will raise an EngineError if the digest is unavailable.
  #
  # ```ruby
  # e = OpenSSL::Engine.by_id("openssl")
  #   #=> #<OpenSSL::Engine id="openssl" name="Software engine support">
  # e.digest("SHA1")
  #   #=> #<OpenSSL::Digest: da39a3ee5e6b4b0d3255bfef95601890afd80709>
  # e.digest("zomg")
  #   #=> OpenSSL::Engine::EngineError: no such digest `zomg'
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def digest(arg0); end

  # Releases all internal structural references for this engine.
  #
  # May raise an EngineError if the engine is unavailable
  sig {returns(::T.untyped)}
  def finish(); end

  # Gets the id for this engine.
  #
  # ```ruby
  # OpenSSL::Engine.load
  # OpenSSL::Engine.engines #=> [#<OpenSSL::Engine#>, ...]
  # OpenSSL::Engine.engines.first.id
  #   #=> "rsax"
  # ```
  sig {returns(::T.untyped)}
  def id(); end

  # Pretty prints this engine.
  sig {returns(::T.untyped)}
  def inspect(); end

  # Loads the given private key identified by *id* and *data*.
  #
  # An EngineError is raised of the
  # [`OpenSSL::PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html) is
  # unavailable.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def load_private_key(*arg0); end

  # Loads the given public key identified by *id* and *data*.
  #
  # An EngineError is raised of the
  # [`OpenSSL::PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html) is
  # unavailable.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def load_public_key(*arg0); end

  # Get the descriptive name for this engine.
  #
  # ```ruby
  # OpenSSL::Engine.load
  # OpenSSL::Engine.engines #=> [#<OpenSSL::Engine#>, ...]
  # OpenSSL::Engine.engines.first.name
  #   #=> "RSAX engine support"
  # ```
  sig {returns(::T.untyped)}
  def name(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the defaults for this
  # engine with the given *flag*.
  #
  # These flags are used to control combinations of algorithm methods.
  #
  # *flag* can be one of the following, other flags are available depending on
  # your OS.
  #
  # All flags
  # :   0xFFFF
  # No flags
  # :   0x0000
  #
  #
  # See also <openssl/engine.h>
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def set_default(arg0); end

  # Fetches the engine as specified by the *id*
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  #
  # ```
  # OpenSSL::Engine.by_id("openssl")
  #  => #<OpenSSL::Engine id="openssl" name="Software engine support">
  # ```
  #
  # See
  # [`OpenSSL::Engine.engines`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Engine.html#method-c-engines)
  # for the currently loaded engines.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.by_id(arg0); end

  # It is only necessary to run cleanup when engines are loaded via
  # [`OpenSSL::Engine.load`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Engine.html#method-c-load).
  # However, running cleanup before exit is recommended.
  #
  # Note that this is needed and works only in
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) < 1.1.0.
  sig {returns(::T.untyped)}
  def self.cleanup(); end

  # Returns an array of currently loaded engines.
  sig {returns(::T.untyped)}
  def self.engines(); end

  # This method loads engines. If *name* is nil, then all builtin engines are
  # loaded. Otherwise, the given *name*, as a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html),  is loaded if
  # available to your runtime, and returns true. If *name* is not found, then
  # nil is returned.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.load(*arg0); end
end

# This is the generic exception for
# [`OpenSSL::Engine`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Engine.html)
# related errors
class OpenSSL::Engine::EngineError < OpenSSL::OpenSSLError
end

# This module contains configuration information about the SSL extension, for
# example if socket support is enabled, or the host name TLS extension is
# enabled. Constants in this module will always be defined, but contain `true`
# or `false` values depending on the configuration of your
# [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) installation.
module OpenSSL::ExtConfig
  HAVE_TLSEXT_HOST_NAME = ::T.let(nil, ::T.untyped)
  OPENSSL_NO_SOCK = ::T.let(nil, ::T.untyped)
  TLS_DH_anon_WITH_AES_256_GCM_SHA384 = ::T.let(nil, ::T.untyped)

end

# [`OpenSSL::HMAC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/HMAC.html)
# allows computing Hash-based Message Authentication Code
# ([`HMAC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/HMAC.html)). It is a
# type of message authentication code (MAC) involving a hash function in
# combination with a key.
# [`HMAC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/HMAC.html) can be used to
# verify the integrity of a message as well as the authenticity.
#
# [`OpenSSL::HMAC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/HMAC.html) has a
# similar interface to
# [`OpenSSL::Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html).
#
# ### HMAC-SHA256 using one-shot interface
#
# ```ruby
# key = "key"
# data = "message-to-be-authenticated"
# mac = OpenSSL::HMAC.hexdigest("SHA256", key, data)
# #=> "cddb0db23f469c8bf072b21fd837149bd6ace9ab771cceef14c9e517cc93282e"
# ```
#
# ### HMAC-SHA256 using incremental interface
#
# ```ruby
# data1 = File.read("file1")
# data2 = File.read("file2")
# key = "key"
# digest = OpenSSL::Digest::SHA256.new
# hmac = OpenSSL::HMAC.new(key, digest)
# hmac << data1
# hmac << data2
# mac = hmac.digest
# ```
class OpenSSL::HMAC
  # Alias for:
  # [`update`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/HMAC.html#method-i-update)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def <<(arg0); end

  # Returns the authentication code an instance represents as a binary string.
  #
  # ### Example
  #
  # ```ruby
  # instance = OpenSSL::HMAC.new('key', OpenSSL::Digest.new('sha1'))
  # #=> f42bb0eeb018ebbd4597ae7213711ec60760843f
  # instance.digest
  # #=> "\xF4+\xB0\xEE\xB0\x18\xEB\xBDE\x97\xAEr\x13q\x1E\xC6\a`\x84?"
  # ```
  sig {returns(::T.untyped)}
  def digest(); end

  # Returns the authentication code an instance represents as a hex-encoded
  # string.
  #
  # Also aliased as:
  # [`inspect`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/HMAC.html#method-i-inspect),
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/HMAC.html#method-i-to_s)
  sig {returns(::T.untyped)}
  def hexdigest(); end

  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .void
  end
  def initialize(arg0, arg1); end

  # Alias for:
  # [`hexdigest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/HMAC.html#method-i-hexdigest)
  sig {returns(::T.untyped)}
  def inspect(); end

  # Returns *hmac* as it was when it was first initialized, with all processed
  # data cleared from it.
  #
  # ### Example
  #
  # ```ruby
  # data = "The quick brown fox jumps over the lazy dog"
  # instance = OpenSSL::HMAC.new('key', OpenSSL::Digest.new('sha1'))
  # #=> f42bb0eeb018ebbd4597ae7213711ec60760843f
  #
  # instance.update(data)
  # #=> de7c9b85b8b78aa6bc8a7a36f70a90701c9db4d9
  # instance.reset
  # #=> f42bb0eeb018ebbd4597ae7213711ec60760843f
  # ```
  sig {returns(::T.untyped)}
  def reset(); end

  # Alias for:
  # [`hexdigest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/HMAC.html#method-i-hexdigest)
  sig {returns(::T.untyped)}
  def to_s(); end

  # Returns *hmac* updated with the message to be authenticated. Can be called
  # repeatedly with chunks of the message.
  #
  # ### Example
  #
  # ```ruby
  # first_chunk = 'The quick brown fox jumps '
  # second_chunk = 'over the lazy dog'
  #
  # instance.update(first_chunk)
  # #=> 5b9a8038a65d571076d97fe783989e52278a492a
  # instance.update(second_chunk)
  # #=> de7c9b85b8b78aa6bc8a7a36f70a90701c9db4d9
  # ```
  #
  #
  # Also aliased as:
  # [`<<`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/HMAC.html#method-i-3C-3C)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def update(arg0); end

  # Returns the authentication code as a binary string. The *digest* parameter
  # specifies the digest algorithm to use. This may be a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) representing the
  # algorithm name or an instance of
  # [`OpenSSL::Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html).
  #
  # ### Example
  #
  # ```ruby
  # key = 'key'
  # data = 'The quick brown fox jumps over the lazy dog'
  #
  # hmac = OpenSSL::HMAC.digest('sha1', key, data)
  # #=> "\xDE|\x9B\x85\xB8\xB7\x8A\xA6\xBC\x8Az6\xF7\n\x90p\x1C\x9D\xB4\xD9"
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.digest(arg0, arg1, arg2); end

  # Returns the authentication code as a hex-encoded string. The *digest*
  # parameter specifies the digest algorithm to use. This may be a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) representing the
  # algorithm name or an instance of
  # [`OpenSSL::Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html).
  #
  # ### Example
  #
  # ```ruby
  # key = 'key'
  # data = 'The quick brown fox jumps over the lazy dog'
  #
  # hmac = OpenSSL::HMAC.hexdigest('sha1', key, data)
  # #=> "de7c9b85b8b78aa6bc8a7a36f70a90701c9db4d9"
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.hexdigest(arg0, arg1, arg2); end
end

# Document-class:
# [`OpenSSL::HMAC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/HMAC.html)
#
# [`OpenSSL::HMAC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/HMAC.html)
# allows computing Hash-based Message Authentication Code (HMAC). It is a type
# of message authentication code (MAC) involving a hash function in combination
# with a key. HMAC can be used to verify the integrity of a message as well as
# the authenticity.
#
# [`OpenSSL::HMAC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/HMAC.html) has a
# similar interface to
# [`OpenSSL::Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html).
#
# ### HMAC-SHA256 using one-shot interface
#
# ```ruby
# key = "key"
# data = "message-to-be-authenticated"
# mac = OpenSSL::HMAC.hexdigest("SHA256", key, data)
# #=> "cddb0db23f469c8bf072b21fd837149bd6ace9ab771cceef14c9e517cc93282e"
# ```
#
# ### HMAC-SHA256 using incremental interface
#
# ```ruby
# data1 = File.read("file1")
# data2 = File.read("file2")
# key = "key"
# digest = OpenSSL::Digest::SHA256.new
# hmac = OpenSSL::HMAC.new(key, digest)
# hmac << data1
# hmac << data2
# mac = hmac.digest
# ```
class OpenSSL::HMACError < OpenSSL::OpenSSLError
end

# [`OpenSSL::Netscape`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape.html)
# is a namespace for SPKI (Simple Public Key Infrastructure) which implements
# Signed Public Key and Challenge. See [RFC
# 2692](http://tools.ietf.org/html/rfc2692) and [RFC
# 2693](http://tools.ietf.org/html/rfc2692) for details.
module OpenSSL::Netscape
end

# A Simple Public Key Infrastructure implementation (pronounced "spooky"). The
# structure is defined as
#
# ```
# PublicKeyAndChallenge ::= SEQUENCE {
#   spki SubjectPublicKeyInfo,
#   challenge IA5STRING
# }
#
# SignedPublicKeyAndChallenge ::= SEQUENCE {
#   publicKeyAndChallenge PublicKeyAndChallenge,
#   signatureAlgorithm AlgorithmIdentifier,
#   signature BIT STRING
# }
# ```
#
# where the definitions of SubjectPublicKeyInfo and AlgorithmIdentifier can be
# found in RFC5280.
# [`SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html) is
# typically used in browsers for generating a public/private key pair and a
# subsequent certificate request, using the HTML <keygen> element.
#
# ## Examples
#
# ### Creating an [`SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html)
#
# ```ruby
# key = OpenSSL::PKey::RSA.new 2048
# spki = OpenSSL::Netscape::SPKI.new
# spki.challenge = "RandomChallenge"
# spki.public_key = key.public_key
# spki.sign(key, OpenSSL::Digest::SHA256.new)
# #send a request containing this to a server generating a certificate
# ```
#
# ### Verifying an [`SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html) request
#
# ```ruby
# request = #...
# spki = OpenSSL::Netscape::SPKI.new request
# unless spki.verify(spki.public_key)
#   # signature is invalid
# end
# #proceed
# ```
class OpenSSL::Netscape::SPKI
  # Returns the challenge string associated with this
  # [`SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html).
  sig {returns(::T.untyped)}
  def challenge(); end

  # ### Parameters
  # *   *str* - the challenge string to be set for this instance
  #
  #
  # Sets the challenge to be associated with the
  # [`SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html).
  # May be used by the server, e.g. to prevent replay.
  sig do
    params(
      challenge: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def challenge=(challenge); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # Returns the public key associated with the
  # [`SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html), an
  # instance of
  # [`OpenSSL::PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html).
  sig {returns(::T.untyped)}
  def public_key(); end

  # ### Parameters
  # *   *pub* - the public key to be set for this instance
  #
  #
  # Sets the public key to be associated with the
  # [`SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html), an
  # instance of
  # [`OpenSSL::PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html).
  # This should be the public key corresponding to the private key used for
  # signing the
  # [`SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html).
  sig do
    params(
      public_key: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def public_key=(public_key); end

  # ### Parameters
  # *   *key* - the private key to be used for signing this instance
  # *   *digest* - the digest to be used for signing this instance
  #
  #
  # To sign an
  # [`SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html),
  # the private key corresponding to the public key set for this instance should
  # be used, in addition to a digest algorithm in the form of an
  # [`OpenSSL::Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html).
  # The private key should be an instance of
  # [`OpenSSL::PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html).
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def sign(arg0, arg1); end

  # Returns the DER encoding of this
  # [`SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html).
  sig {returns(::T.untyped)}
  def to_der(); end

  # Returns the PEM encoding of this
  # [`SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html).
  #
  # Also aliased as:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html#method-i-to_s)
  sig {returns(::T.untyped)}
  def to_pem(); end

  # Alias for:
  # [`to_pem`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html#method-i-to_pem)
  sig {returns(::T.untyped)}
  def to_s(); end

  # Returns a textual representation of this
  # [`SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html),
  # useful for debugging purposes.
  sig {returns(::T.untyped)}
  def to_text(); end

  # ### Parameters
  # *   *key* - the public key to be used for verifying the
  #     [`SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html)
  #     signature
  #
  #
  # Returns `true` if the signature is valid, `false` otherwise. To verify an
  # [`SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html),
  # the public key contained within the
  # [`SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html)
  # should be used.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def verify(arg0); end
end

# Generic [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html)
# class that is raised if an error occurs during an operation on an instance of
# [`OpenSSL::Netscape::SPKI`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Netscape/SPKI.html).
class OpenSSL::Netscape::SPKIError < OpenSSL::OpenSSLError
end

# [`OpenSSL::OCSP`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP.html)
# implements Online Certificate Status Protocol requests and responses.
#
# Creating and sending an
# [`OCSP`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP.html) request
# requires a subject certificate that contains an
# [`OCSP`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP.html) URL in an
# authorityInfoAccess extension and the issuer certificate for the subject
# certificate. First, load the issuer and subject certificates:
#
# ```ruby
# subject = OpenSSL::X509::Certificate.new subject_pem
# issuer  = OpenSSL::X509::Certificate.new issuer_pem
# ```
#
# To create the request we need to create a certificate ID for the subject
# certificate so the CA knows which certificate we are asking about:
#
# ```ruby
# digest = OpenSSL::Digest::SHA1.new
# certificate_id =
#   OpenSSL::OCSP::CertificateId.new subject, issuer, digest
# ```
#
# Then create a request and add the certificate ID to it:
#
# ```ruby
# request = OpenSSL::OCSP::Request.new
# request.add_certid certificate_id
# ```
#
# Adding a nonce to the request protects against replay attacks but not all CA
# process the nonce.
#
# ```ruby
# request.add_nonce
# ```
#
# To submit the request to the CA for verification we need to extract the
# [`OCSP`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP.html)
# [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) from the subject
# certificate:
#
# ```ruby
# authority_info_access = subject.extensions.find do |extension|
#   extension.oid == 'authorityInfoAccess'
# end
#
# descriptions = authority_info_access.value.split "\n"
# ocsp = descriptions.find do |description|
#   description.start_with? 'OCSP'
# end
#
# require 'uri'
#
# ocsp_uri = URI ocsp[/URI:(.*)/, 1]
# ```
#
# To submit the request we'll POST the request to the
# [`OCSP`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP.html)
# [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html) (per RFC 2560). Note
# that we only handle HTTP requests and don't handle any redirects in this
# example, so this is insufficient for serious use.
#
# ```ruby
# require 'net/http'
#
# http_response =
#   Net::HTTP.start ocsp_uri.hostname, ocsp.port do |http|
#     http.post ocsp_uri.path, request.to_der,
#               'content-type' => 'application/ocsp-request'
# end
#
# response = OpenSSL::OCSP::Response.new http_response.body
# response_basic = response.basic
# ```
#
# First we check if the response has a valid signature. Without a valid
# signature we cannot trust it. If you get a failure here you may be missing a
# system certificate store or may be missing the intermediate certificates.
#
# ```ruby
# store = OpenSSL::X509::Store.new
# store.set_default_paths
#
# unless response_basic.verify [], store then
#   raise 'response is not signed by a trusted certificate'
# end
# ```
#
# The response contains the status information (success/fail). We can display
# the status as a string:
#
# ```ruby
# puts response.status_string #=> successful
# ```
#
# Next we need to know the response details to determine if the response matches
# our request. First we check the nonce. Again, not all CAs support a nonce. See
# Request#check\_nonce for the meanings of the return values.
#
# ```ruby
# p request.check_nonce basic_response #=> value from -1 to 3
# ```
#
# Then extract the status information for the certificate from the basic
# response.
#
# ```ruby
# single_response = basic_response.find_response(certificate_id)
#
# unless single_response
#   raise 'basic_response does not have the status for the certificiate'
# end
# ```
#
# Then check the validity. A status issued in the future must be rejected.
#
# ```
# unless single_response.check_validity
#   raise 'this_update is in the future or next_update time has passed'
# end
#
# case single_response.cert_status
# when OpenSSL::OCSP::V_CERTSTATUS_GOOD
#   puts 'certificate is still valid'
# when OpenSSL::OCSP::V_CERTSTATUS_REVOKED
#   puts "certificate has been revoked at #{single_response.revocation_time}"
# when OpenSSL::OCSP::V_CERTSTATUS_UNKNOWN
#   puts 'responder doesn't know about the certificate'
# end
# ```
module OpenSSL::OCSP
  # (This flag is not used by
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) 1.0.1g)
  NOCASIGN = ::T.let(nil, ::T.untyped)
  # Do not include certificates in the response
  NOCERTS = ::T.let(nil, ::T.untyped)
  # Do not verify the certificate chain on the response
  NOCHAIN = ::T.let(nil, ::T.untyped)
  # Do not make additional signing certificate checks
  NOCHECKS = ::T.let(nil, ::T.untyped)
  # (This flag is not used by
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) 1.0.1g)
  NODELEGATED = ::T.let(nil, ::T.untyped)
  # Do not check trust
  NOEXPLICIT = ::T.let(nil, ::T.untyped)
  # Do not search certificates contained in the response for a signer
  NOINTERN = ::T.let(nil, ::T.untyped)
  # Do not check the signature on the response
  NOSIGS = ::T.let(nil, ::T.untyped)
  # Do not include producedAt time in response
  NOTIME = ::T.let(nil, ::T.untyped)
  # Do not verify the response at all
  NOVERIFY = ::T.let(nil, ::T.untyped)
  # Identify the response by signing the certificate key ID
  RESPID_KEY = ::T.let(nil, ::T.untyped)
  # Internal error in issuer
  RESPONSE_STATUS_INTERNALERROR = ::T.let(nil, ::T.untyped)
  # Illegal confirmation request
  RESPONSE_STATUS_MALFORMEDREQUEST = ::T.let(nil, ::T.untyped)
  # You must sign the request and resubmit
  RESPONSE_STATUS_SIGREQUIRED = ::T.let(nil, ::T.untyped)
  # Response has valid confirmations
  RESPONSE_STATUS_SUCCESSFUL = ::T.let(nil, ::T.untyped)
  # Try again later
  RESPONSE_STATUS_TRYLATER = ::T.let(nil, ::T.untyped)
  # Your request is unauthorized.
  RESPONSE_STATUS_UNAUTHORIZED = ::T.let(nil, ::T.untyped)
  # The certificate subject's name or other information changed
  REVOKED_STATUS_AFFILIATIONCHANGED = ::T.let(nil, ::T.untyped)
  # This CA certificate was revoked due to a key compromise
  REVOKED_STATUS_CACOMPROMISE = ::T.let(nil, ::T.untyped)
  # The certificate is on hold
  REVOKED_STATUS_CERTIFICATEHOLD = ::T.let(nil, ::T.untyped)
  # The certificate is no longer needed
  REVOKED_STATUS_CESSATIONOFOPERATION = ::T.let(nil, ::T.untyped)
  # The certificate was revoked due to a key compromise
  REVOKED_STATUS_KEYCOMPROMISE = ::T.let(nil, ::T.untyped)
  # The certificate was revoked for an unknown reason
  REVOKED_STATUS_NOSTATUS = ::T.let(nil, ::T.untyped)
  # The certificate was previously on hold and should now be removed from the
  # CRL
  REVOKED_STATUS_REMOVEFROMCRL = ::T.let(nil, ::T.untyped)
  # The certificate was superseded by a new certificate
  REVOKED_STATUS_SUPERSEDED = ::T.let(nil, ::T.untyped)
  # The certificate was revoked for an unspecified reason
  REVOKED_STATUS_UNSPECIFIED = ::T.let(nil, ::T.untyped)
  # Do not verify additional certificates
  TRUSTOTHER = ::T.let(nil, ::T.untyped)
  # Indicates the certificate is not revoked but does not necessarily mean the
  # certificate was issued or that this response is within the certificate's
  # validity interval
  V_CERTSTATUS_GOOD = ::T.let(nil, ::T.untyped)
  # Indicates the certificate has been revoked either permanently or temporarily
  # (on hold).
  V_CERTSTATUS_REVOKED = ::T.let(nil, ::T.untyped)
  # Indicates the responder does not know about the certificate being requested.
  V_CERTSTATUS_UNKNOWN = ::T.let(nil, ::T.untyped)
  # The responder ID is based on the public key.
  V_RESPID_KEY = ::T.let(nil, ::T.untyped)
  # The responder ID is based on the key name.
  V_RESPID_NAME = ::T.let(nil, ::T.untyped)

end

# An
# [`OpenSSL::OCSP::BasicResponse`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/BasicResponse.html)
# contains the status of a certificate check which is created from an
# [`OpenSSL::OCSP::Request`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/Request.html).
# A
# [`BasicResponse`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/BasicResponse.html)
# is more detailed than a Response.
class OpenSSL::OCSP::BasicResponse
  # Adds *nonce* to this response. If no nonce was provided a random nonce will
  # be added.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_nonce(*arg0); end

  # Adds a certificate status for *certificate\_id*. *status* is the status, and
  # must be one of these:
  #
  # *   OpenSSL::OCSP::V\_CERTSTATUS\_GOOD
  # *   OpenSSL::OCSP::V\_CERTSTATUS\_REVOKED
  # *   OpenSSL::OCSP::V\_CERTSTATUS\_UNKNOWN
  #
  #
  # *reason* and *revocation\_time* can be given only when *status* is
  # OpenSSL::OCSP::V\_CERTSTATUS\_REVOKED. *reason* describes the reason for the
  # revocation, and must be one of OpenSSL::OCSP::REVOKED\_STATUS\_\* constants.
  # *revocation\_time* is the time when the certificate is revoked.
  #
  # *this\_update* and *next\_update* indicate the time at which ths status is
  # verified to be correct and the time at or before which newer information
  # will be available, respectively. *next\_update* is optional.
  #
  # *extensions* is an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
  # of
  # [`OpenSSL::X509::Extension`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Extension.html)
  # to be included in the SingleResponse. This is also optional.
  #
  # Note that the times, *revocation\_time*, *this\_update* and *next\_update*
  # can be specified in either of
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) or
  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) object. If they are
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html), it is treated
  # as the relative seconds from the current time.
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
      arg3: ::T.untyped,
      arg4: ::T.untyped,
      arg5: ::T.untyped,
      arg6: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_status(arg0, arg1, arg2, arg3, arg4, arg5, arg6); end

  # Copies the nonce from *request* into this response. Returns 1 on success and
  # 0 on failure.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def copy_nonce(arg0); end

  # Returns a SingleResponse whose CertId matches with *certificate\_id*, or
  # `nil` if this
  # [`BasicResponse`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/BasicResponse.html)
  # does not contain it.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def find_response(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # SingleResponse for this
  # [`BasicResponse`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/BasicResponse.html).
  sig {returns(::T.untyped)}
  def responses(); end

  # Signs this [`OCSP`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP.html)
  # response using the *cert*, *key* and optional *digest*. This behaves in the
  # similar way as
  # [`OpenSSL::OCSP::Request#sign`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/Request.html#method-i-sign).
  #
  # *flags* can include:
  # OpenSSL::OCSP::NOCERTS
  # :   don't include certificates
  # OpenSSL::OCSP::NOTIME
  # :   don't set producedAt
  # OpenSSL::OCSP::RESPID\_KEY
  # :   use signer's public key hash as responderID
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def sign(*arg0); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # statuses for this response. Each status contains a CertificateId, the status
  # (0 for good, 1 for revoked, 2 for unknown), the reason for the status, the
  # revocation time, the time of this update, the time for the next update and a
  # list of
  # [`OpenSSL::X509::Extension`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Extension.html).
  #
  # This should be superseded by
  # [`BasicResponse#responses`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/BasicResponse.html#method-i-responses)
  # and
  # [`find_response`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/BasicResponse.html#method-i-find_response)
  # that return SingleResponse.
  sig {returns(::T.untyped)}
  def status(); end

  # Encodes this basic response into a DER-encoded string.
  sig {returns(::T.untyped)}
  def to_der(); end

  # Verifies the signature of the response using the given *certificates* and
  # *store*. This works in the similar way as
  # [`OpenSSL::OCSP::Request#verify`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/Request.html#method-i-verify).
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def verify(*arg0); end
end

# An
# [`OpenSSL::OCSP::CertificateId`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/CertificateId.html)
# identifies a certificate to the CA so that a status check can be performed.
class OpenSSL::OCSP::CertificateId
  # Compares this certificate id with *other* and returns `true` if they are the
  # same.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def cmp(arg0); end

  # Compares this certificate id's issuer with *other* and returns `true` if
  # they are the same.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def cmp_issuer(arg0); end

  # Returns the ln (long name) of the hash algorithm used to generate the
  # issuerNameHash and the issuerKeyHash values.
  sig {returns(::T.untyped)}
  def hash_algorithm(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # Returns the issuerKeyHash of this certificate ID, the hash of the issuer's
  # public key.
  sig {returns(::T.untyped)}
  def issuer_key_hash(); end

  # Returns the issuerNameHash of this certificate ID, the hash of the issuer's
  # distinguished name calculated with the hashAlgorithm.
  sig {returns(::T.untyped)}
  def issuer_name_hash(); end

  # Returns the serial number of the certificate for which status is being
  # requested.
  sig {returns(::T.untyped)}
  def serial(); end

  # Encodes this certificate identifier into a DER-encoded string.
  sig {returns(::T.untyped)}
  def to_der(); end
end

# [`OCSP`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP.html) error class.
class OpenSSL::OCSP::OCSPError < OpenSSL::OpenSSLError
end

# An
# [`OpenSSL::OCSP::Request`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/Request.html)
# contains the certificate information for determining if a certificate has been
# revoked or not. A
# [`Request`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/Request.html) can
# be created for a certificate or from a DER-encoded request created elsewhere.
class OpenSSL::OCSP::Request
  # Adds *certificate\_id* to the request.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_certid(arg0); end

  # Adds a *nonce* to the
  # [`OCSP`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP.html) request. If
  # no nonce is given a random one will be generated.
  #
  # The nonce is used to prevent replay attacks but some servers do not support
  # it.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_nonce(*arg0); end

  # Returns all certificate IDs in this request.
  sig {returns(::T.untyped)}
  def certid(); end

  # Checks the nonce validity for this request and *response*.
  #
  # The return value is one of the following:
  #
  # -1
  # :   nonce in request only.
  # 0
  # :   nonces both present and not equal.
  # 1
  # :   nonces present and equal.
  # 2
  # :   nonces both absent.
  # 3
  # :   nonce present in response only.
  #
  #
  # For most responses, clients can check *result* > 0. If a responder doesn't
  # handle nonces `result.nonzero?` may be necessary. A result of `0` is always
  # an error.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def check_nonce(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # Signs this [`OCSP`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP.html)
  # request using *cert*, *key* and optional *digest*. If *digest* is not
  # specified, SHA-1 is used. *certs* is an optional
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of additional
  # certificates which are included in the request in addition to the signer
  # certificate. Note that if *certs* is `nil` or not given, flag
  # OpenSSL::OCSP::NOCERTS is enabled. Pass an empty array to include only the
  # signer certificate.
  #
  # *flags* is a bitwise OR of the following constants:
  #
  # OpenSSL::OCSP::NOCERTS
  # :   Don't include any certificates in the request. *certs* will be ignored.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def sign(*arg0); end

  # Returns this request as a DER-encoded string
  sig {returns(::T.untyped)}
  def to_der(); end

  # Verifies this request using the given *certificates* and *store*.
  # *certificates* is an array of
  # [`OpenSSL::X509::Certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Certificate.html),
  # *store* is an
  # [`OpenSSL::X509::Store`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Store.html).
  #
  # Note that `false` is returned if the request does not have a signature. Use
  # [`signed?`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/Request.html#method-i-signed-3F)
  # to check whether the request is signed or not.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def verify(*arg0); end
end

# An
# [`OpenSSL::OCSP::Response`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/Response.html)
# contains the status of a certificate check which is created from an
# [`OpenSSL::OCSP::Request`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/Request.html).
class OpenSSL::OCSP::Response
  # Returns a BasicResponse for this response
  sig {returns(::T.untyped)}
  def basic(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # Returns the status of the response.
  sig {returns(::T.untyped)}
  def status(); end

  # Returns a status string for the response.
  sig {returns(::T.untyped)}
  def status_string(); end

  # Returns this response as a DER-encoded string.
  sig {returns(::T.untyped)}
  def to_der(); end

  # Creates an
  # [`OpenSSL::OCSP::Response`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/Response.html)
  # from *status* and *basic\_response*.
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.create(arg0, arg1); end
end

# An
# [`OpenSSL::OCSP::SingleResponse`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/SingleResponse.html)
# represents an [`OCSP`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP.html)
# [`SingleResponse`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/SingleResponse.html)
# structure, which contains the basic information of the status of the
# certificate.
class OpenSSL::OCSP::SingleResponse
  # Returns the status of the certificate identified by the certid. The return
  # value may be one of these constant:
  #
  # *   V\_CERTSTATUS\_GOOD
  # *   V\_CERTSTATUS\_REVOKED
  # *   V\_CERTSTATUS\_UNKNOWN
  #
  #
  # When the status is V\_CERTSTATUS\_REVOKED, the time at which the certificate
  # was revoked can be retrieved by
  # [`revocation_time`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/SingleResponse.html#method-i-revocation_time).
  sig {returns(::T.untyped)}
  def cert_status(); end

  # Returns the CertificateId for which this
  # [`SingleResponse`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/SingleResponse.html)
  # is.
  sig {returns(::T.untyped)}
  def certid(); end

  # Checks the validity of thisUpdate and nextUpdate fields of this
  # [`SingleResponse`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/SingleResponse.html).
  # This checks the current time is within the range thisUpdate to nextUpdate.
  #
  # It is possible that the
  # [`OCSP`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP.html) request
  # takes a few seconds or the time is not accurate. To avoid rejecting a valid
  # response, this method allows the times to be within *nsec* seconds of the
  # current time.
  #
  # Some responders don't set the nextUpdate field. This may cause a very old
  # response to be considered valid. The *maxsec* parameter can be used to limit
  # the age of responses.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def check_validity(*arg0); end

  sig {returns(::T.untyped)}
  def extensions(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(arg0); end

  sig {returns(::T.untyped)}
  def next_update(); end

  sig {returns(::T.untyped)}
  def revocation_reason(); end

  sig {returns(::T.untyped)}
  def revocation_time(); end

  sig {returns(::T.untyped)}
  def this_update(); end

  # Encodes this
  # [`SingleResponse`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/OCSP/SingleResponse.html)
  # into a DER-encoded string.
  sig {returns(::T.untyped)}
  def to_der(); end
end

# Generic error, common for all classes under
# [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) module
class OpenSSL::OpenSSLError < StandardError
end

# Defines a file format commonly used to store private keys with accompanying
# public key certificates, protected with a password-based symmetric key.
class OpenSSL::PKCS12
  sig {returns(::T.untyped)}
  def ca_certs(); end

  sig {returns(::T.untyped)}
  def certificate(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  sig {returns(::T.untyped)}
  def key(); end

  sig {returns(::T.untyped)}
  def to_der(); end

  # ### Parameters
  # *   *pass* - string
  # *   *name* - A string describing the key.
  # *   *key* - Any PKey.
  # *   *cert* - A X509::Certificate.
  #     *   The public\_key portion of the certificate must contain a valid
  #         public key.
  #     *   The not\_before and not\_after fields must be filled in.
  #
  # *   *ca* - An optional array of X509::Certificate's.
  # *   *key\_pbe* - string
  # *   *cert\_pbe* - string
  # *   *key\_iter* - integer
  # *   *mac\_iter* - integer
  # *   *keytype* - An integer representing an MSIE specific extension.
  #
  #
  # Any optional arguments may be supplied as `nil` to preserve the
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) defaults.
  #
  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for PKCS12\_create().
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.create(*arg0); end
end

class OpenSSL::PKCS12::PKCS12Error < OpenSSL::OpenSSLError
end

module OpenSSL::PKCS5
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
      arg3: ::T.untyped,
      arg4: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pbkdf2_hmac(arg0, arg1, arg2, arg3, arg4); end

  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
      arg3: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pbkdf2_hmac_sha1(arg0, arg1, arg2, arg3); end
end

class OpenSSL::PKCS5::PKCS5Error < OpenSSL::OpenSSLError
end

class OpenSSL::PKCS7
  BINARY = ::T.let(nil, ::T.untyped)
  DETACHED = ::T.let(nil, ::T.untyped)
  NOATTR = ::T.let(nil, ::T.untyped)
  NOCERTS = ::T.let(nil, ::T.untyped)
  NOCHAIN = ::T.let(nil, ::T.untyped)
  NOINTERN = ::T.let(nil, ::T.untyped)
  NOSIGS = ::T.let(nil, ::T.untyped)
  NOSMIMECAP = ::T.let(nil, ::T.untyped)
  NOVERIFY = ::T.let(nil, ::T.untyped)
  TEXT = ::T.let(nil, ::T.untyped)

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_certificate(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_crl(arg0); end

  # Also aliased as:
  # [`data=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKCS7.html#method-i-data-3D)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_data(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_recipient(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_signer(arg0); end

  sig {returns(::T.untyped)}
  def certificates(); end

  sig do
    params(
      certificates: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def certificates=(certificates); end

  sig do
    params(
      cipher: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def cipher=(cipher); end

  sig {returns(::T.untyped)}
  def crls(); end

  sig do
    params(
      crls: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def crls=(crls); end

  sig {returns(::T.untyped)}
  def data(); end

  # Alias for:
  # [`add_data`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKCS7.html#method-i-add_data)
  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def data=(data); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def decrypt(*arg0); end

  sig {returns(::T.untyped)}
  def detached(); end

  sig do
    params(
      detached: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def detached=(detached); end

  sig {returns(::T.untyped)}
  def detached?(); end

  sig {returns(::T.untyped)}
  def error_string(); end

  sig do
    params(
      error_string: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def error_string=(error_string); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  sig {returns(::T.untyped)}
  def recipients(); end

  sig {returns(::T.untyped)}
  def signers(); end

  sig {returns(::T.untyped)}
  def to_der(); end

  # Also aliased as:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKCS7.html#method-i-to_s)
  sig {returns(::T.untyped)}
  def to_pem(); end

  # Alias for:
  # [`to_pem`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKCS7.html#method-i-to_pem)
  sig {returns(::T.untyped)}
  def to_s(); end

  sig {returns(::T.untyped)}
  def type(); end

  sig do
    params(
      type: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def type=(type); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def verify(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.encrypt(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.read_smime(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.sign(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.write_smime(*arg0); end
end

class OpenSSL::PKCS7::PKCS7Error < OpenSSL::OpenSSLError
end

class OpenSSL::PKCS7::RecipientInfo
  sig {returns(::T.untyped)}
  def enc_key(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(arg0); end

  sig {returns(::T.untyped)}
  def issuer(); end

  sig {returns(::T.untyped)}
  def serial(); end
end

class OpenSSL::PKCS7::SignerInfo
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
    )
    .void
  end
  def initialize(arg0, arg1, arg2); end

  # Also aliased as:
  # [`name`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKCS7/SignerInfo.html#method-i-name)
  sig {returns(::T.untyped)}
  def issuer(); end

  # Alias for:
  # [`issuer`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKCS7/SignerInfo.html#method-i-issuer)
  sig {returns(::T.untyped)}
  def name(); end

  sig {returns(::T.untyped)}
  def serial(); end

  sig {returns(::T.untyped)}
  def signed_time(); end
end

# ## Asymmetric Public Key Algorithms
#
# Asymmetric public key algorithms solve the problem of establishing and sharing
# secret keys to en-/decrypt messages. The key in such an algorithm consists of
# two parts: a public key that may be distributed to others and a private key
# that needs to remain secret.
#
# Messages encrypted with a public key can only be decrypted by recipients that
# are in possession of the associated private key. Since public key algorithms
# are considerably slower than symmetric key algorithms (cf.
# [`OpenSSL::Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html))
# they are often used to establish a symmetric key shared between two parties
# that are in possession of each other's public key.
#
# Asymmetric algorithms offer a lot of nice features that are used in a lot of
# different areas. A very common application is the creation and validation of
# digital signatures. To sign a document, the signatory generally uses a message
# digest algorithm (cf.
# [`OpenSSL::Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html))
# to compute a digest of the document that is then encrypted (i.e. signed) using
# the private key. Anyone in possession of the public key may then verify the
# signature by computing the message digest of the original document on their
# own, decrypting the signature using the signatory's public key and comparing
# the result to the message digest they previously computed. The signature is
# valid if and only if the decrypted signature is equal to this message digest.
#
# The [`PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html) module
# offers support for three popular public/private key algorithms:
# *   RSA
#     ([`OpenSSL::PKey::RSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html))
# *   DSA
#     ([`OpenSSL::PKey::DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html))
# *   Elliptic Curve Cryptography
#     ([`OpenSSL::PKey::EC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html))
#
# Each of these implementations is in fact a sub-class of the abstract
# [`PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html) class which
# offers the interface for supporting digital signatures in the form of
# PKey#sign and PKey#verify.
#
# ## Diffie-Hellman Key Exchange
#
# Finally [`PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html) also
# features
# [`OpenSSL::PKey::DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html),
# an implementation of the Diffie-Hellman key exchange protocol based on
# discrete logarithms in finite fields, the same basis that DSA is built on. The
# Diffie-Hellman protocol can be used to exchange (symmetric) keys over insecure
# channels without needing any prior joint knowledge between the participating
# parties. As the security of DH demands relatively long "public keys" (i.e. the
# part that is overtly transmitted between participants) DH tends to be quite
# slow. If security or speed is your primary concern,
# [`OpenSSL::PKey::EC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html)
# offers another implementation of the Diffie-Hellman protocol.
module OpenSSL::PKey
  DEFAULT_TMP_DH_CALLBACK = ::T.let(nil, ::T.untyped)

  # Reads a DER or PEM encoded string from *string* or *io* and returns an
  # instance of the appropriate
  # [`PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html) class.
  #
  # ### Parameters
  # *   \_string+ is a DER- or PEM-encoded string containing an arbitrary
  #     private or public key.
  # *   *io* is an instance of
  #     [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) containing a DER- or
  #     PEM-encoded arbitrary private or public key.
  # *   *pwd* is an optional password in case *string* or *io* is an encrypted
  #     PEM resource.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.read(*arg0); end
end

# An implementation of the Diffie-Hellman key exchange protocol based on
# discrete logarithms in finite fields, the same basis that DSA is built on.
#
# ### Accessor methods for the Diffie-Hellman parameters
# [`DH#p`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-p)
# :   The prime (an
#     [`OpenSSL::BN`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html)) of
#     the Diffie-Hellman parameters.
# DH#g
# :   The generator (an
#     [`OpenSSL::BN`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html)) g of
#     the Diffie-Hellman parameters.
# DH#pub\_key
# :   The per-session public key (an
#     [`OpenSSL::BN`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html))
#     matching the private key. This needs to be passed to
#     [`DH#compute_key`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html#method-i-compute_key).
# DH#priv\_key
# :   The per-session private key, an
#     [`OpenSSL::BN`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html).
#
#
# ### Example of a key exchange
#
# ```ruby
# dh1 = OpenSSL::PKey::DH.new(2048)
# der = dh1.public_key.to_der #you may send this publicly to the participating party
# dh2 = OpenSSL::PKey::DH.new(der)
# dh2.generate_key! #generate the per-session key pair
# symm_key1 = dh1.compute_key(dh2.pub_key)
# symm_key2 = dh2.compute_key(dh1.pub_key)
#
# puts symm_key1 == symm_key2 # => true
# ```
class OpenSSL::PKey::DH < OpenSSL::PKey::PKey
  DEFAULT_1024 = ::T.let(nil, ::T.untyped)
  DEFAULT_2048 = ::T.let(nil, ::T.untyped)

  # Returns a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # containing a shared secret computed from the other party's public value. See
  # DH\_compute\_key() for further information.
  #
  # ### Parameters
  # *   *pub\_bn* is a
  #     [`OpenSSL::BN`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html),
  #     **not** the
  #     [`DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html)
  #     instance returned by
  #     [`DH#public_key`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html#method-i-public_key)
  #     as that contains the
  #     [`DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html)
  #     parameters only.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def compute_key(arg0); end

  # Encodes this
  # [`DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html) to its PEM
  # encoding. Note that any existing per-session public/private keys will
  # **not** get encoded, just the Diffie-Hellman parameters will be encoded.
  #
  # Also aliased as:
  # [`to_pem`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html#method-i-to_pem),
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html#method-i-to_s)
  sig {returns(::T.untyped)}
  def export(); end

  sig {returns(::T.untyped)}
  def g(); end

  sig do
    params(
      g: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def g=(g); end

  # Generates a private and public key unless a private key already exists. If
  # this [`DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html)
  # instance was generated from public
  # [`DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html) parameters
  # (e.g. by encoding the result of
  # [`DH#public_key`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html#method-i-public_key)),
  # then this method needs to be called first in order to generate the
  # per-session keys before performing the actual key exchange.
  #
  # ### Example
  #
  # ```ruby
  # dh = OpenSSL::PKey::DH.new(2048)
  # public_key = dh.public_key #contains no private/public key yet
  # public_key.generate_key!
  # puts public_key.private? # => true
  # ```
  sig {returns(::T.untyped)}
  def generate_key!(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  sig {returns(::T.untyped)}
  def p(); end

  sig do
    params(
      p: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def p=(p); end

  # Stores all parameters of key to the hash INSECURE: PRIVATE INFORMATIONS CAN
  # LEAK OUT!!! Don't use :-)) (I's up to you)
  sig {returns(::T.untyped)}
  def params(); end

  # Validates the Diffie-Hellman parameters associated with this instance. It
  # checks whether a safe prime and a suitable generator are used. If this is
  # not the case, `false` is returned.
  sig {returns(::T.untyped)}
  def params_ok?(); end

  sig {returns(::T.untyped)}
  def priv_key(); end

  sig do
    params(
      priv_key: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def priv_key=(priv_key); end

  # Indicates whether this
  # [`DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html) instance
  # has a private key associated with it or not. The private key may be
  # retrieved with DH#priv\_key.
  sig {returns(::T.untyped)}
  def private?(); end

  sig {returns(::T.untyped)}
  def pub_key(); end

  sig do
    params(
      pub_key: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def pub_key=(pub_key); end

  # Indicates whether this
  # [`DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html) instance
  # has a public key associated with it or not. The public key may be retrieved
  # with DH#pub\_key.
  sig {returns(::T.untyped)}
  def public?(); end

  # Returns a new
  # [`DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html) instance
  # that carries just the public information, i.e. the prime *p* and the
  # generator *g*, but no public/private key yet. Such a pair may be generated
  # using
  # [`DH#generate_key!`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html#method-i-generate_key-21).
  # The "public key" needed for a key exchange with
  # [`DH#compute_key`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html#method-i-compute_key)
  # is considered as per-session information and may be retrieved with
  # DH#pub\_key once a key pair has been generated. If the current instance
  # already contains private information (and thus a valid public/private key
  # pair), this information will no longer be present in the new instance
  # generated by
  # [`DH#public_key`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html#method-i-public_key).
  # This feature is helpful for publishing the Diffie-Hellman parameters without
  # leaking any of the private per-session information.
  #
  # ### Example
  #
  # ```ruby
  # dh = OpenSSL::PKey::DH.new(2048) # has public and private key set
  # public_key = dh.public_key # contains only prime and generator
  # parameters = public_key.to_der # it's safe to publish this
  # ```
  sig {returns(::T.untyped)}
  def public_key(); end

  sig {returns(::T.untyped)}
  def q(); end

  sig do
    params(
      q: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def q=(q); end

  # Sets *pub\_key* and *priv\_key* for the
  # [`DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html) instance.
  # *priv\_key* may be `nil`.
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def set_key(arg0, arg1); end

  # Sets *p*, *q*, *g* to the
  # [`DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html) instance.
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def set_pqg(arg0, arg1, arg2); end

  # Encodes this
  # [`DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html) to its DER
  # encoding. Note that any existing per-session public/private keys will
  # **not** get encoded, just the Diffie-Hellman parameters will be encoded.
  sig {returns(::T.untyped)}
  def to_der(); end

  # Alias for:
  # [`export`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html#method-i-export)
  sig {returns(::T.untyped)}
  def to_pem(); end

  # Alias for:
  # [`export`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html#method-i-export)
  sig {returns(::T.untyped)}
  def to_s(); end

  # Prints all parameters of key to buffer INSECURE: PRIVATE INFORMATIONS CAN
  # LEAK OUT!!! Don't use :-)) (I's up to you)
  sig {returns(::T.untyped)}
  def to_text(); end

  # Creates a new
  # [`DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html) instance
  # from scratch by generating the private and public components alike.
  #
  # ### Parameters
  # *   *size* is an integer representing the desired key size. Keys smaller
  #     than 1024 bits should be considered insecure.
  # *   *generator* is a small number > 1, typically 2 or 5.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.generate(*arg0); end
end

# Generic exception that is raised if an operation on a DH
# [`PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html) fails
# unexpectedly or in case an instantiation of an instance of DH fails due to
# non-conformant input data.
class OpenSSL::PKey::DHError < OpenSSL::PKey::PKeyError
end

# [`DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html), the
# Digital Signature Algorithm, is specified in NIST's FIPS 186-3. It is an
# asymmetric public key algorithm that may be used similar to e.g. RSA.
class OpenSSL::PKey::DSA < OpenSSL::PKey::PKey
  # Encodes this
  # [`DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html) to its
  # PEM encoding.
  #
  # ### Parameters
  # *   *cipher* is an
  #     [`OpenSSL::Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html).
  # *   *password* is a string containing your password.
  #
  #
  # ### Examples
  #
  # ```
  # DSA.to_pem -> aString
  # DSA.to_pem(cipher, 'mypassword') -> aString
  # ```
  #
  #
  # Also aliased as:
  # [`to_pem`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html#method-i-to_pem),
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html#method-i-to_s)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def export(*arg0); end

  sig {returns(::T.untyped)}
  def g(); end

  sig do
    params(
      g: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def g=(g); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  sig {returns(::T.untyped)}
  def p(); end

  sig do
    params(
      p: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def p=(p); end

  # Stores all parameters of key to the hash INSECURE: PRIVATE INFORMATIONS CAN
  # LEAK OUT!!! Don't use :-)) (I's up to you)
  sig {returns(::T.untyped)}
  def params(); end

  sig {returns(::T.untyped)}
  def priv_key(); end

  sig do
    params(
      priv_key: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def priv_key=(priv_key); end

  # Indicates whether this
  # [`DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html) instance
  # has a private key associated with it or not. The private key may be
  # retrieved with DSA#private\_key.
  sig {returns(::T.untyped)}
  def private?(); end

  sig {returns(::T.untyped)}
  def pub_key(); end

  sig do
    params(
      pub_key: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def pub_key=(pub_key); end

  # Indicates whether this
  # [`DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html) instance
  # has a public key associated with it or not. The public key may be retrieved
  # with
  # [`DSA#public_key`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html#method-i-public_key).
  sig {returns(::T.untyped)}
  def public?(); end

  # Returns a new
  # [`DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html) instance
  # that carries just the public key information. If the current instance has
  # also private key information, this will no longer be present in the new
  # instance. This feature is helpful for publishing the public key information
  # without leaking any of the private information.
  #
  # ### Example
  #
  # ```ruby
  # dsa = OpenSSL::PKey::DSA.new(2048) # has public and private information
  # pub_key = dsa.public_key # has only the public part available
  # pub_key_der = pub_key.to_der # it's safe to publish this
  # ```
  sig {returns(::T.untyped)}
  def public_key(); end

  sig {returns(::T.untyped)}
  def q(); end

  sig do
    params(
      q: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def q=(q); end

  # Sets *pub\_key* and *priv\_key* for the
  # [`DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html) instance.
  # *priv\_key* may be `nil`.
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def set_key(arg0, arg1); end

  # Sets *p*, *q*, *g* to the
  # [`DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html) instance.
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def set_pqg(arg0, arg1, arg2); end

  # Computes and returns the
  # [`DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html) signature
  # of *string*, where *string* is expected to be an already-computed message
  # digest of the original input data. The signature is issued using the private
  # key of this
  # [`DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html) instance.
  #
  # ### Parameters
  # *   *string* is a message digest of the original input data to be signed.
  #
  #
  # ### Example
  #
  # ```ruby
  # dsa = OpenSSL::PKey::DSA.new(2048)
  # doc = "Sign me"
  # digest = OpenSSL::Digest::SHA1.digest(doc)
  # sig = dsa.syssign(digest)
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def syssign(arg0); end

  # Verifies whether the signature is valid given the message digest input. It
  # does so by validating *sig* using the public key of this
  # [`DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html) instance.
  #
  # ### Parameters
  # *   *digest* is a message digest of the original input data to be signed
  # *   *sig* is a
  #     [`DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html)
  #     signature value
  #
  #
  # ### Example
  #
  # ```ruby
  # dsa = OpenSSL::PKey::DSA.new(2048)
  # doc = "Sign me"
  # digest = OpenSSL::Digest::SHA1.digest(doc)
  # sig = dsa.syssign(digest)
  # puts dsa.sysverify(digest, sig) # => true
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def sysverify(arg0, arg1); end

  # Encodes this
  # [`DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html) to its
  # DER encoding.
  sig {returns(::T.untyped)}
  def to_der(); end

  # Alias for:
  # [`export`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html#method-i-export)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def to_pem(*arg0); end

  # Alias for:
  # [`export`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html#method-i-export)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def to_s(*arg0); end

  # Prints all parameters of key to buffer INSECURE: PRIVATE INFORMATIONS CAN
  # LEAK OUT!!! Don't use :-)) (I's up to you)
  sig {returns(::T.untyped)}
  def to_text(); end

  # Creates a new
  # [`DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html) instance
  # by generating a private/public key pair from scratch.
  #
  # ### Parameters
  # *   *size* is an integer representing the desired key size.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.generate(arg0); end
end

# Generic exception that is raised if an operation on a DSA
# [`PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html) fails
# unexpectedly or in case an instantiation of an instance of DSA fails due to
# non-conformant input data.
class OpenSSL::PKey::DSAError < OpenSSL::PKey::PKeyError
end

# [`OpenSSL::PKey::EC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html)
# provides access to Elliptic Curve Digital Signature Algorithm (ECDSA) and
# Elliptic Curve Diffie-Hellman (ECDH).
#
# ### Key exchange
#
# ```ruby
# ec1 = OpenSSL::PKey::EC.generate("prime256v1")
# ec2 = OpenSSL::PKey::EC.generate("prime256v1")
# # ec1 and ec2 have own private key respectively
# shared_key1 = ec1.dh_compute_key(ec2.public_key)
# shared_key2 = ec2.dh_compute_key(ec1.public_key)
#
# p shared_key1 == shared_key2 #=> true
# ```
class OpenSSL::PKey::EC < OpenSSL::PKey::PKey
  NAMED_CURVE = ::T.let(nil, ::T.untyped)

  # Raises an exception if the key is invalid.
  #
  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_KEY\_check\_key()
  sig {returns(::T.untyped)}
  def check_key(); end

  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for ECDH\_compute\_key()
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def dh_compute_key(arg0); end

  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for ECDSA\_sign()
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def dsa_sign_asn1(arg0); end

  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for ECDSA\_verify()
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def dsa_verify_asn1(arg0, arg1); end

  # Outputs the [`EC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html)
  # key in PEM encoding. If *cipher* and *pass\_phrase* are given they will be
  # used to encrypt the key. *cipher* must be an
  # [`OpenSSL::Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html)
  # instance. Note that encryption will only be effective for a private key,
  # public keys will always be encoded in plain text.
  #
  # Also aliased as:
  # [`to_pem`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html#method-i-to_pem)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def export(*arg0); end

  # Alias for:
  # [`generate_key!`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html#method-i-generate_key-21)
  sig {returns(::T.untyped)}
  def generate_key(); end

  # Generates a new random private and public key.
  #
  # See also the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_KEY\_generate\_key()
  #
  # ### Example
  #
  # ```ruby
  # ec = OpenSSL::PKey::EC.new("prime256v1")
  # p ec.private_key # => nil
  # ec.generate_key!
  # p ec.private_key # => #<OpenSSL::BN XXXXXX>
  # ```
  #
  #
  # Also aliased as:
  # [`generate_key`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html#method-i-generate_key)
  sig {returns(::T.untyped)}
  def generate_key!(); end

  # Returns the
  # [`EC::Group`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC/Group.html)
  # that the key is associated with. Modifying the returned group does not
  # affect *key*.
  sig {returns(::T.untyped)}
  def group(); end

  # Sets the
  # [`EC::Group`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC/Group.html)
  # for the key. The group structure is internally copied so modification to
  # *group* after assigning to a key has no effect on the key.
  sig do
    params(
      group: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def group=(group); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # Returns whether this
  # [`EC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html) instance
  # has a private key. The private key (BN) can be retrieved with
  # [`EC#private_key`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html#method-i-private_key).
  #
  # Also aliased as:
  # [`private_key?`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html#method-i-private_key-3F)
  sig {returns(::T.untyped)}
  def private?(); end

  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_KEY\_get0\_private\_key()
  sig {returns(::T.untyped)}
  def private_key(); end

  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_KEY\_set\_private\_key()
  sig do
    params(
      private_key: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def private_key=(private_key); end

  # Alias for:
  # [`private?`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html#method-i-private-3F)
  sig {returns(::T.untyped)}
  def private_key?(); end

  # Returns whether this
  # [`EC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html) instance
  # has a public key. The public key
  # ([`EC::Point`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC/Point.html))
  # can be retrieved with
  # [`EC#public_key`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html#method-i-public_key).
  #
  # Also aliased as:
  # [`public_key?`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html#method-i-public_key-3F)
  sig {returns(::T.untyped)}
  def public?(); end

  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_KEY\_get0\_public\_key()
  sig {returns(::T.untyped)}
  def public_key(); end

  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_KEY\_set\_public\_key()
  sig do
    params(
      public_key: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def public_key=(public_key); end

  # Alias for:
  # [`public?`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html#method-i-public-3F)
  sig {returns(::T.untyped)}
  def public_key?(); end

  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for i2d\_ECPrivateKey\_bio()
  sig {returns(::T.untyped)}
  def to_der(); end

  # Alias for:
  # [`export`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html#method-i-export)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def to_pem(*arg0); end

  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_KEY\_print()
  sig {returns(::T.untyped)}
  def to_text(); end

  # Obtains a list of all predefined curves by the
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html). Curve names
  # are returned as sn.
  #
  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_get\_builtin\_curves().
  sig {returns(::T.untyped)}
  def self.builtin_curves(); end

  # Creates a new
  # [`EC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html) instance
  # with a new random private and public key.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.generate(arg0); end
end

class OpenSSL::PKey::EC::Group
  # Alias for:
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC/Group.html#method-i-eql-3F)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ==(arg0); end

  # Returns the flags set on the group.
  #
  # See also
  # [`asn1_flag=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC/Group.html#method-i-asn1_flag-3D).
  sig {returns(::T.untyped)}
  def asn1_flag(); end

  # Sets flags on the group. The flag value is used to determine how to encode
  # the group: encode explicit parameters or named curve using an OID.
  #
  # The flag value can be either of:
  #
  # *   EC::NAMED\_CURVE
  # *   EC::EXPLICIT\_CURVE
  #
  #
  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_GROUP\_set\_asn1\_flag().
  sig do
    params(
      asn1_flag: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def asn1_flag=(asn1_flag); end

  # Returns the cofactor of the group.
  #
  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_GROUP\_get\_cofactor()
  sig {returns(::T.untyped)}
  def cofactor(); end

  # Returns the curve name (sn).
  #
  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_GROUP\_get\_curve\_name()
  sig {returns(::T.untyped)}
  def curve_name(); end

  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_GROUP\_get\_degree()
  sig {returns(::T.untyped)}
  def degree(); end

  # Returns `true` if the two groups use the same curve and have the same
  # parameters, `false` otherwise.
  #
  # Also aliased as:
  # [`==`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC/Group.html#method-i-3D-3D)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def eql?(arg0); end

  # Returns the generator of the group.
  #
  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_GROUP\_get0\_generator()
  sig {returns(::T.untyped)}
  def generator(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # Returns the order of the group.
  #
  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_GROUP\_get\_order()
  sig {returns(::T.untyped)}
  def order(); end

  # Returns the form how
  # [`EC::Point`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC/Point.html)
  # data is encoded as ASN.1.
  #
  # See also
  # [`point_conversion_form=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC/Group.html#method-i-point_conversion_form-3D).
  sig {returns(::T.untyped)}
  def point_conversion_form(); end

  # Sets the form how
  # [`EC::Point`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC/Point.html)
  # data is encoded as ASN.1 as defined in X9.62.
  #
  # *format* can be one of these:
  #
  # `:compressed`
  # :   Encoded as z||x, where z is an octet indicating which solution of the
  #     equation y is. z will be 0x02 or 0x03.
  # `:uncompressed`
  # :   Encoded as z||x||y, where z is an octet 0x04.
  # `:hybrid`
  # :   Encodes as z||x||y, where z is an octet indicating which solution of the
  #     equation y is. z will be 0x06 or 0x07.
  #
  #
  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_GROUP\_set\_point\_conversion\_form()
  sig do
    params(
      point_conversion_form: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def point_conversion_form=(point_conversion_form); end

  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_GROUP\_get0\_seed()
  sig {returns(::T.untyped)}
  def seed(); end

  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_GROUP\_set\_seed()
  sig do
    params(
      seed: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def seed=(seed); end

  # Sets the curve parameters. *generator* must be an instance of
  # [`EC::Point`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC/Point.html)
  # that is on the curve. *order* and *cofactor* are integers.
  #
  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for EC\_GROUP\_set\_generator()
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def set_generator(arg0, arg1, arg2); end

  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for i2d\_ECPKParameters\_bio()
  sig {returns(::T.untyped)}
  def to_der(); end

  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for PEM\_write\_bio\_ECPKParameters()
  sig {returns(::T.untyped)}
  def to_pem(); end

  # See the [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # documentation for ECPKParameters\_print()
  sig {returns(::T.untyped)}
  def to_text(); end
end

class OpenSSL::PKey::EC::Group::Error < OpenSSL::OpenSSLError
end

class OpenSSL::PKey::EC::Point
  # Alias for:
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC/Point.html#method-i-eql-3F)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ==(arg0); end

  # Also aliased as:
  # [`==`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC/Point.html#method-i-3D-3D)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def eql?(arg0); end

  sig {returns(::T.untyped)}
  def group(); end

  sig {returns(::T.untyped)}
  def infinity?(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  sig {returns(::T.untyped)}
  def invert!(); end

  sig {returns(::T.untyped)}
  def make_affine!(); end

  # Performs elliptic curve point multiplication.
  #
  # The first form calculates `bn1 * point + bn2 * G`, where `G` is the
  # generator of the group of *point*. *bn2* may be omitted, and in that case,
  # the result is just `bn1 * point`.
  #
  # The second form calculates `bns[0] * point + bns[1] * points[0] + ... +
  # bns[-1] * points[-1] + bn2 * G`. *bn2* may be omitted. *bns* must be an
  # array of
  # [`OpenSSL::BN`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html).
  # *points* must be an array of
  # [`OpenSSL::PKey::EC::Point`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC/Point.html).
  # Please note that `points[0]` is not multiplied by `bns[0]`, but `bns[1]`.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def mul(*arg0); end

  sig {returns(::T.untyped)}
  def on_curve?(); end

  sig {returns(::T.untyped)}
  def set_to_infinity!(); end

  # Returns the octet string representation of the
  # [`EC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html) point as an
  # instance of
  # [`OpenSSL::BN`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/BN.html).
  #
  # If *conversion\_form* is not given, the *point\_conversion\_form* attribute
  # set to the group is used.
  #
  # See
  # [`to_octet_string`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC/Point.html#method-i-to_octet_string)
  # for more information.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def to_bn(*arg0); end
end

class OpenSSL::PKey::EC::Point::Error < OpenSSL::OpenSSLError
end

class OpenSSL::PKey::ECError < OpenSSL::PKey::PKeyError
end

# An abstract class that bundles signature creation
# ([`PKey#sign`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/PKey.html#method-i-sign))
# and validation
# ([`PKey#verify`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/PKey.html#method-i-verify))
# that is common to all implementations except
# [`OpenSSL::PKey::DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html)
# *   [`OpenSSL::PKey::RSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html)
# *   [`OpenSSL::PKey::DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html)
# *   [`OpenSSL::PKey::EC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html)
class OpenSSL::PKey::PKey
  sig {void}
  def initialize(); end

  # To sign the [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # *data*, *digest*, an instance of
  # [`OpenSSL::Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html),
  # must be provided. The return value is again a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) containing the
  # signature. A PKeyError is raised should errors occur. Any previous state of
  # the [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html) instance is
  # irrelevant to the signature outcome, the digest instance is reset to its
  # initial state during the operation.
  #
  # ## Example
  #
  # ```ruby
  # data = 'Sign me!'
  # digest = OpenSSL::Digest::SHA256.new
  # pkey = OpenSSL::PKey::RSA.new(2048)
  # signature = pkey.sign(digest, data)
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def sign(arg0, arg1); end

  # To verify the [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # *signature*, *digest*, an instance of
  # [`OpenSSL::Digest`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Digest.html),
  # must be provided to re-compute the message digest of the original *data*,
  # also a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). The
  # return value is `true` if the signature is valid, `false` otherwise. A
  # PKeyError is raised should errors occur. Any previous state of the
  # [`Digest`](https://docs.ruby-lang.org/en/2.7.0/Digest.html) instance is
  # irrelevant to the validation outcome, the digest instance is reset to its
  # initial state during the operation.
  #
  # ## Example
  #
  # ```ruby
  # data = 'Sign me!'
  # digest = OpenSSL::Digest::SHA256.new
  # pkey = OpenSSL::PKey::RSA.new(2048)
  # signature = pkey.sign(digest, data)
  # pub_key = pkey.public_key
  # puts pub_key.verify(digest, signature, data) # => true
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def verify(arg0, arg1, arg2); end
end

# Raised when errors occur during PKey#sign or PKey#verify.
class OpenSSL::PKey::PKeyError < OpenSSL::OpenSSLError
end

# [`RSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html) is an
# asymmetric public key algorithm that has been formalized in RFC 3447. It is in
# widespread use in public key infrastructures (PKI) where certificates (cf.
# [`OpenSSL::X509::Certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Certificate.html))
# often are issued on the basis of a public/private
# [`RSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html) key pair.
# [`RSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html) is used in
# a wide field of applications such as secure (symmetric) key exchange, e.g.
# when establishing a secure TLS/SSL connection. It is also used in various
# digital signature schemes.
class OpenSSL::PKey::RSA < OpenSSL::PKey::PKey
  NO_PADDING = ::T.let(nil, ::T.untyped)
  PKCS1_OAEP_PADDING = ::T.let(nil, ::T.untyped)
  PKCS1_PADDING = ::T.let(nil, ::T.untyped)
  SSLV23_PADDING = ::T.let(nil, ::T.untyped)

  sig {returns(::T.untyped)}
  def d(); end

  sig do
    params(
      d: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def d=(d); end

  sig {returns(::T.untyped)}
  def dmp1(); end

  sig do
    params(
      dmp1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def dmp1=(dmp1); end

  sig {returns(::T.untyped)}
  def dmq1(); end

  sig do
    params(
      dmq1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def dmq1=(dmq1); end

  sig {returns(::T.untyped)}
  def e(); end

  sig do
    params(
      e: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def e=(e); end

  # Outputs this keypair in PEM encoding. If *cipher* and *pass\_phrase* are
  # given they will be used to encrypt the key. *cipher* must be an
  # [`OpenSSL::Cipher`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Cipher.html)
  # instance.
  #
  # Also aliased as:
  # [`to_pem`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html#method-i-to_pem),
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html#method-i-to_s)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def export(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  sig {returns(::T.untyped)}
  def iqmp(); end

  sig do
    params(
      iqmp: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def iqmp=(iqmp); end

  sig {returns(::T.untyped)}
  def n(); end

  sig do
    params(
      n: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def n=(n); end

  sig {returns(::T.untyped)}
  def p(); end

  sig do
    params(
      p: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def p=(p); end

  # THIS METHOD IS INSECURE, PRIVATE INFORMATION CAN LEAK OUT!!!
  #
  # Stores all parameters of key to the hash. The hash has keys 'n', 'e', 'd',
  # 'p', 'q', 'dmp1', 'dmq1', 'iqmp'.
  #
  # Don't use :-)) (It's up to you)
  sig {returns(::T.untyped)}
  def params(); end

  # Does this keypair contain a private key?
  sig {returns(::T.untyped)}
  def private?(); end

  # Decrypt *string*, which has been encrypted with the public key, with the
  # private key. *padding* defaults to PKCS1\_PADDING.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def private_decrypt(*arg0); end

  # Encrypt *string* with the private key. *padding* defaults to PKCS1\_PADDING.
  # The encrypted string output can be decrypted using
  # [`public_decrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html#method-i-public_decrypt).
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def private_encrypt(*arg0); end

  # The return value is always `true` since every private key is also a public
  # key.
  sig {returns(::T.untyped)}
  def public?(); end

  # Decrypt *string*, which has been encrypted with the private key, with the
  # public key. *padding* defaults to PKCS1\_PADDING.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def public_decrypt(*arg0); end

  # Encrypt *string* with the public key. *padding* defaults to PKCS1\_PADDING.
  # The encrypted string output can be decrypted using
  # [`private_decrypt`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html#method-i-private_decrypt).
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def public_encrypt(*arg0); end

  # Makes new [`RSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html)
  # instance containing the public key from the private key.
  sig {returns(::T.untyped)}
  def public_key(); end

  sig {returns(::T.untyped)}
  def q(); end

  sig do
    params(
      q: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def q=(q); end

  # Sets *dmp1*, *dmq1*, *iqmp* for the
  # [`RSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html) instance.
  # They are calculated by `d mod (p - 1)`, `d mod (q - 1)` and `q^(-1) mod p`
  # respectively.
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def set_crt_params(arg0, arg1, arg2); end

  # Sets *p*, *q* for the
  # [`RSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html) instance.
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def set_factors(arg0, arg1); end

  # Sets *n*, *e*, *d* for the
  # [`RSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html) instance.
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
      arg2: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def set_key(arg0, arg1, arg2); end

  # Signs *data* using the Probabilistic Signature Scheme (RSA-PSS) and returns
  # the calculated signature.
  #
  # RSAError will be raised if an error occurs.
  #
  # See
  # [`verify_pss`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html#method-i-verify_pss)
  # for the verification operation.
  #
  # ### Parameters
  # *digest*
  # :   A [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) containing
  #     the message digest algorithm name.
  # *data*
  # :   A [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). The data
  #     to be signed.
  # *salt\_length*
  # :   The length in octets of the salt. Two special values are reserved:
  #     `:digest` means the digest length, and `:max` means the maximum possible
  #     length for the combination of the private key and the selected message
  #     digest algorithm.
  # *mgf1\_hash*
  # :   The hash algorithm used in MGF1 (the currently supported mask generation
  #     function (MGF)).
  #
  #
  # ### Example
  #
  # ```ruby
  # data = "Sign me!"
  # pkey = OpenSSL::PKey::RSA.new(2048)
  # signature = pkey.sign_pss("SHA256", data, salt_length: :max, mgf1_hash: "SHA256")
  # pub_key = pkey.public_key
  # puts pub_key.verify_pss("SHA256", signature, data,
  #                         salt_length: :auto, mgf1_hash: "SHA256") # => true
  # ```
  sig do
    params(
      digest: String,
      data: String,
      salt_length: T.any(Integer, Symbol),
      mgf1_hash: String
    ).returns(String)
  end
  def sign_pss(digest, data, salt_length:, mgf1_hash:); end

  # Outputs this keypair in DER encoding.
  sig {returns(::T.untyped)}
  def to_der(); end

  # Alias for:
  # [`export`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html#method-i-export)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def to_pem(*arg0); end

  # Alias for:
  # [`export`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html#method-i-export)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def to_s(*arg0); end

  # THIS METHOD IS INSECURE, PRIVATE INFORMATION CAN LEAK OUT!!!
  #
  # Dumps all parameters of a keypair to a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  #
  # Don't use :-)) (It's up to you)
  sig {returns(::T.untyped)}
  def to_text(); end

  # Generates an
  # [`RSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html) keypair.
  # *size* is an integer representing the desired key size. Keys smaller than
  # 1024 should be considered insecure. *exponent* is an odd number normally 3,
  # 17, or 65537.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.generate(*arg0); end

  # Verifies *data* using the Probabilistic Signature Scheme (RSA-PSS).
  #
  # The return value is `true` if the signature is valid, `false` otherwise.
  # RSAError will be raised if an error occurs.
  #
  # See
  # [`sign_pss`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html#method-i-sign_pss)
  # for the signing operation and an example code.
  #
  # ### Parameters
  # *digest*
  # :   A [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) containing
  #     the message digest algorithm name.
  # *data*
  # :   A [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). The data
  #     to be signed.
  # *salt\_length*
  # :   The length in octets of the salt. Two special values are reserved:
  #     `:digest` means the digest length, and `:auto` means automatically
  #     determining the length based on the signature.
  # *mgf1\_hash*
  # :   The hash algorithm used in MGF1.
  sig do
    params(
      digest: String,
      signature: String,
      data: String,
      salt_length: T.any(Integer, Symbol),
      mgf1_hash: String
    ).returns(T::Boolean)
  end
  def verify_pss(digest, signature, data, salt_length:, mgf1_hash:); end
end

# Generic exception that is raised if an operation on an RSA
# [`PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html) fails
# unexpectedly or in case an instantiation of an instance of RSA fails due to
# non-conformant input data.
class OpenSSL::PKey::RSAError < OpenSSL::PKey::PKeyError
end

module OpenSSL::Random
  # Same as ::egd\_bytes but queries 255 bytes by default.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.egd(arg0); end

  # Queries the entropy gathering daemon EGD on socket path given by *filename*.
  #
  # Fetches *length* number of bytes and uses ::add to seed the
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) built-in PRNG.
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.egd_bytes(arg0, arg1); end

  # Reads bytes from *filename* and adds them to the PRNG.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.load_random_file(arg0); end

  # Generates a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) with
  # *length* number of pseudo-random bytes.
  #
  # Pseudo-random byte sequences generated by ::pseudo\_bytes will be unique if
  # they are of sufficient length, but are not necessarily unpredictable.
  #
  # ### Example
  #
  # ```ruby
  # OpenSSL::Random.pseudo_bytes(12)
  # #=> "..."
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pseudo_bytes(arg0); end

  # Mixes the bytes from *str* into the Pseudo
  # [`Random`](https://docs.ruby-lang.org/en/2.7.0/Random.html) Number
  # Generator(PRNG) state.
  #
  # Thus, if the data from *str* are unpredictable to an adversary, this
  # increases the uncertainty about the state and makes the PRNG output less
  # predictable.
  #
  # The *entropy* argument is (the lower bound of) an estimate of how much
  # randomness is contained in *str*, measured in bytes.
  #
  # ### Example
  #
  # ```ruby
  # pid = $$
  # now = Time.now
  # ary = [now.to_i, now.nsec, 1000, pid]
  # OpenSSL::Random.add(ary.join, 0.0)
  # OpenSSL::Random.seed(ary.join)
  # ```
  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.random_add(arg0, arg1); end

  # Generates a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) with
  # *length* number of cryptographically strong pseudo-random bytes.
  #
  # ### Example
  #
  # ```ruby
  # OpenSSL::Random.random_bytes(12)
  # #=> "..."
  # ```
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.random_bytes(arg0); end

  # ::seed is equivalent to ::add where *entropy* is length of *str*.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.seed(arg0); end

  # Return `true` if the PRNG has been seeded with enough data, `false`
  # otherwise.
  sig {returns(::T.untyped)}
  def self.status?(); end

  # Writes a number of random generated bytes (currently 1024) to *filename*
  # which can be used to initialize the PRNG by calling ::load\_random\_file in
  # a later session.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.write_random_file(arg0); end
end

class OpenSSL::Random::RandomError < OpenSSL::OpenSSLError
end

# Use SSLContext to set up the parameters for a TLS (former
# [`SSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL.html)) connection.
# Both client and server TLS connections are supported, SSLSocket and SSLServer
# may be used in conjunction with an instance of SSLContext to set up
# connections.
module OpenSSL::SSL
  OP_ALL = ::T.let(nil, ::T.untyped)
  OP_CIPHER_SERVER_PREFERENCE = ::T.let(nil, ::T.untyped)
  OP_DONT_INSERT_EMPTY_FRAGMENTS = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 1.0.1k and 1.0.2.
  OP_EPHEMERAL_RSA = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 1.1.0.
  OP_MICROSOFT_BIG_SSLV3_BUFFER = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 1.1.0.
  OP_MICROSOFT_SESS_ID_BUG = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 0.9.7h and 0.9.8b.
  OP_MSIE_SSLV2_RSA_PADDING = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 1.1.0.
  OP_NETSCAPE_CA_DN_BUG = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 1.1.0.
  OP_NETSCAPE_CHALLENGE_BUG = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 1.1.0.
  OP_NETSCAPE_DEMO_CIPHER_CHANGE_BUG = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 0.9.8q and 1.0.0c.
  OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG = ::T.let(nil, ::T.untyped)
  OP_NO_COMPRESSION = ::T.let(nil, ::T.untyped)
  OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 1.1.0.
  OP_NO_SSLv2 = ::T.let(nil, ::T.untyped)
  OP_NO_SSLv3 = ::T.let(nil, ::T.untyped)
  OP_NO_TICKET = ::T.let(nil, ::T.untyped)
  OP_NO_TLSv1 = ::T.let(nil, ::T.untyped)
  OP_NO_TLSv1_1 = ::T.let(nil, ::T.untyped)
  OP_NO_TLSv1_2 = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 1.0.1.
  OP_PKCS1_CHECK_1 = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 1.0.1.
  OP_PKCS1_CHECK_2 = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 1.1.0.
  OP_SINGLE_DH_USE = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 1.1.0.
  OP_SINGLE_ECDH_USE = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 1.1.0.
  OP_SSLEAY_080_CLIENT_DH_BUG = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 1.0.1h and 1.0.2.
  OP_SSLREF2_REUSE_CERT_TYPE_BUG = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 1.1.0.
  OP_TLS_BLOCK_PADDING_BUG = ::T.let(nil, ::T.untyped)
  # Deprecated in [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # 1.1.0.
  OP_TLS_D5_BUG = ::T.let(nil, ::T.untyped)
  OP_TLS_ROLLBACK_BUG = ::T.let(nil, ::T.untyped)
  VERIFY_CLIENT_ONCE = ::T.let(nil, ::T.untyped)
  VERIFY_FAIL_IF_NO_PEER_CERT = ::T.let(nil, ::T.untyped)
  VERIFY_NONE = ::T.let(nil, ::T.untyped)
  VERIFY_PEER = ::T.let(nil, ::T.untyped)

  sig do
    params(
      cert: ::T.untyped,
      hostname: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.verify_certificate_identity(cert, hostname); end

  sig do
    params(
      hostname: ::T.untyped,
      san: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.verify_hostname(hostname, san); end

  sig do
    params(
      domain_component: ::T.untyped,
      san_component: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.verify_wildcard(domain_component, san_component); end
end

# An
# [`SSLContext`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html)
# is used to set various options regarding certificates, algorithms,
# verification, session caching, etc. The
# [`SSLContext`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html)
# is used to create an SSLSocket.
#
# All attributes must be set before creating an SSLSocket as the
# [`SSLContext`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html)
# will be frozen afterward.
class OpenSSL::SSL::SSLContext
  DEFAULT_CERT_STORE = ::T.let(nil, ::T.untyped)
  DEFAULT_PARAMS = ::T.let(nil, ::T.untyped)
  # Both client and server sessions are added to the session cache
  SESSION_CACHE_BOTH = ::T.let(nil, ::T.untyped)
  # Client sessions are added to the session cache
  SESSION_CACHE_CLIENT = ::T.let(nil, ::T.untyped)
  # Normally the session cache is checked for expired sessions every 255
  # connections. Since this may lead to a delay that cannot be controlled, the
  # automatic flushing may be disabled and
  # [`flush_sessions`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-flush_sessions)
  # can be called explicitly.
  SESSION_CACHE_NO_AUTO_CLEAR = ::T.let(nil, ::T.untyped)
  # Enables both
  # [`SESSION_CACHE_NO_INTERNAL_LOOKUP`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#SESSION_CACHE_NO_INTERNAL_LOOKUP)
  # and
  # [`SESSION_CACHE_NO_INTERNAL_STORE`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#SESSION_CACHE_NO_INTERNAL_STORE).
  SESSION_CACHE_NO_INTERNAL = ::T.let(nil, ::T.untyped)
  # Always perform external lookups of sessions even if they are in the internal
  # cache.
  #
  # This flag has no effect on clients
  SESSION_CACHE_NO_INTERNAL_LOOKUP = ::T.let(nil, ::T.untyped)
  # Never automatically store sessions in the internal store.
  SESSION_CACHE_NO_INTERNAL_STORE = ::T.let(nil, ::T.untyped)
  # No session caching for client or server
  SESSION_CACHE_OFF = ::T.let(nil, ::T.untyped)
  # Server sessions are added to the session cache
  SESSION_CACHE_SERVER = ::T.let(nil, ::T.untyped)

  sig do
    params(
      certificate: ::OpenSSL::X509::Certificate,
      pkey: ::OpenSSL::PKey::PKey,
      extra_certs: ::T.nilable(T::Array[::OpenSSL::X509::Certificate])
    )
    .returns(::OpenSSL::SSL::SSLContext)
  end
  def add_certificate(certificate, pkey, extra_certs = nil); end

  # The path to a file containing a PEM-format CA certificate
  sig {returns(::T.untyped)}
  def ca_file(); end

  # The path to a file containing a PEM-format CA certificate
  sig do
    params(
      ca_file: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ca_file=(ca_file); end

  # The path to a directory containing CA certificates in PEM format.
  #
  # Files are looked up by subject's X509 name's hash value.
  sig {returns(::T.untyped)}
  def ca_path(); end

  # The path to a directory containing CA certificates in PEM format.
  #
  # Files are looked up by subject's X509 name's hash value.
  sig do
    params(
      ca_path: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ca_path=(ca_path); end

  # Context certificate
  #
  # The *cert*, *key*, and *extra\_chain\_cert* attributes are deprecated. It is
  # recommended to use
  # [`add_certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-add_certificate)
  # instead.
  sig {returns(::T.untyped)}
  def cert(); end

  # Context certificate
  #
  # The *cert*, *key*, and *extra\_chain\_cert* attributes are deprecated. It is
  # recommended to use
  # [`add_certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-add_certificate)
  # instead.
  sig do
    params(
      cert: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def cert=(cert); end

  # An
  # [`OpenSSL::X509::Store`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Store.html)
  # used for certificate verification.
  sig {returns(::T.untyped)}
  def cert_store(); end

  # An
  # [`OpenSSL::X509::Store`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Store.html)
  # used for certificate verification.
  sig do
    params(
      cert_store: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def cert_store=(cert_store); end

  # The list of cipher suites configured for this context.
  sig {returns(::T.untyped)}
  def ciphers(); end

  # Sets the list of available cipher suites for this context. Note in a server
  # context some ciphers require the appropriate certificates. For example, an
  # RSA cipher suite can only be chosen when an RSA certificate is available.
  sig do
    params(
      ciphers: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ciphers=(ciphers); end

  # A certificate or [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
  # of certificates that will be sent to the client.
  sig {returns(::T.untyped)}
  def client_ca(); end

  # A certificate or [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
  # of certificates that will be sent to the client.
  sig do
    params(
      client_ca: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def client_ca=(client_ca); end

  # A callback invoked when a client certificate is requested by a server and no
  # certificate has been set.
  #
  # The callback is invoked with a Session and must return an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) containing an
  # [`OpenSSL::X509::Certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Certificate.html)
  # and an
  # [`OpenSSL::PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html). If
  # any other value is returned the handshake is suspended.
  sig {returns(::T.untyped)}
  def client_cert_cb(); end

  # A callback invoked when a client certificate is requested by a server and no
  # certificate has been set.
  #
  # The callback is invoked with a Session and must return an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) containing an
  # [`OpenSSL::X509::Certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Certificate.html)
  # and an
  # [`OpenSSL::PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html). If
  # any other value is returned the handshake is suspended.
  sig do
    params(
      client_cert_cb: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def client_cert_cb=(client_cert_cb); end

  # Sets the list of "supported elliptic curves" for this context.
  #
  # For a TLS client, the list is directly used in the Supported Elliptic Curves
  # Extension. For a server, the list is used by
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) to determine
  # the set of shared curves.
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) will pick the
  # most appropriate one from it.
  #
  # Note that this works differently with old
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) (<= 1.0.1).
  # Only one curve can be set, and this has no effect for TLS clients.
  #
  # ### Example
  #
  # ```ruby
  # ctx1 = OpenSSL::SSL::SSLContext.new
  # ctx1.ecdh_curves = "X25519:P-256:P-224"
  # svr = OpenSSL::SSL::SSLServer.new(tcp_svr, ctx1)
  # Thread.new { svr.accept }
  #
  # ctx2 = OpenSSL::SSL::SSLContext.new
  # ctx2.ecdh_curves = "P-256"
  # cli = OpenSSL::SSL::SSLSocket.new(tcp_sock, ctx2)
  # cli.connect
  #
  # p cli.tmp_key.group.curve_name
  # # => "prime256v1" (is an alias for NIST P-256)
  # ```
  sig do
    params(
      ecdh_curves: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ecdh_curves=(ecdh_curves); end

  # An [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of extra X509
  # certificates to be added to the certificate chain.
  #
  # The *cert*, *key*, and *extra\_chain\_cert* attributes are deprecated. It is
  # recommended to use
  # [`add_certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-add_certificate)
  # instead.
  sig {returns(::T.untyped)}
  def extra_chain_cert(); end

  # An [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of extra X509
  # certificates to be added to the certificate chain.
  #
  # The *cert*, *key*, and *extra\_chain\_cert* attributes are deprecated. It is
  # recommended to use
  # [`add_certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-add_certificate)
  # instead.
  sig do
    params(
      extra_chain_cert: ::T.untyped
    )
    .returns(::T.untyped)
  end
  def extra_chain_cert=(extra_chain_cert); end

  # Removes sessions in the internal cache that have expired at *time*.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def flush_sessions(*arg0); end

  # Alias for:
  # [`setup`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-setup)
  sig {returns(::T.untyped)}
  def freeze(); end

  sig do
    params(
      version: ::T.untyped,
    )
    .void
  end
  def initialize(version=T.unsafe(nil)); end

  # Context private key
  #
  # The *cert*, *key*, and *extra\_chain\_cert* attributes are deprecated. It is
  # recommended to use
  # [`add_certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-add_certificate)
  # instead.
  sig {returns(::T.untyped)}
  def key(); end

  # Context private key
  #
  # The *cert*, *key*, and *extra\_chain\_cert* attributes are deprecated. It is
  # recommended to use
  # [`add_certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-add_certificate)
  # instead.
  sig do
    params(
      key: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def key=(key); end

  # An [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) of
  # Strings. Each [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # represents a protocol to be advertised as the list of supported protocols
  # for Next Protocol Negotiation. Supported in
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) 1.0.1 and
  # higher. Has no effect on the client side. If not set explicitly, the NPN
  # extension will not be sent by the server in the handshake.
  #
  # ### Example
  #
  # ```ruby
  # ctx.npn_protocols = ["http/1.1", "spdy/2"]
  # ```
  sig {returns(::T.untyped)}
  def npn_protocols(); end

  # An [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) of
  # Strings. Each [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # represents a protocol to be advertised as the list of supported protocols
  # for Next Protocol Negotiation. Supported in
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) 1.0.1 and
  # higher. Has no effect on the client side. If not set explicitly, the NPN
  # extension will not be sent by the server in the handshake.
  #
  # ### Example
  #
  # ```ruby
  # ctx.npn_protocols = ["http/1.1", "spdy/2"]
  # ```
  sig do
    params(
      npn_protocols: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def npn_protocols=(npn_protocols); end

  # A callback invoked on the client side when the client needs to select a
  # protocol from the list sent by the server. Supported in
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) 1.0.1 and
  # higher. The client MUST select a protocol of those advertised by the server.
  # If none is acceptable, raising an error in the callback will cause the
  # handshake to fail. Not setting this callback explicitly means not supporting
  # the NPN extension on the client - any protocols advertised by the server
  # will be ignored.
  #
  # ### Example
  #
  # ```ruby
  # ctx.npn_select_cb = lambda do |protocols|
  #   # inspect the protocols and select one
  #   protocols.first
  # end
  # ```
  sig {returns(::T.untyped)}
  def npn_select_cb(); end

  # A callback invoked on the client side when the client needs to select a
  # protocol from the list sent by the server. Supported in
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) 1.0.1 and
  # higher. The client MUST select a protocol of those advertised by the server.
  # If none is acceptable, raising an error in the callback will cause the
  # handshake to fail. Not setting this callback explicitly means not supporting
  # the NPN extension on the client - any protocols advertised by the server
  # will be ignored.
  #
  # ### Example
  #
  # ```ruby
  # ctx.npn_select_cb = lambda do |protocols|
  #   # inspect the protocols and select one
  #   protocols.first
  # end
  # ```
  sig do
    params(
      npn_select_cb: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def npn_select_cb=(npn_select_cb); end

  # Gets various [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # options.
  sig {returns(::T.untyped)}
  def options(); end

  # Sets various [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html)
  # options.
  sig do
    params(
      options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def options=(options); end

  # A callback invoked whenever a new handshake is initiated. May be used to
  # disable renegotiation entirely.
  #
  # The callback is invoked with the active SSLSocket. The callback's return
  # value is irrelevant, normal return indicates "approval" of the renegotiation
  # and will continue the process. To forbid renegotiation and to cancel the
  # process, an [`Error`](https://docs.ruby-lang.org/en/2.7.0/Error.html) may be
  # raised within the callback.
  #
  # ### Disable client renegotiation
  #
  # When running a server, it is often desirable to disable client renegotiation
  # entirely. You may use a callback as follows to implement this feature:
  #
  # ```ruby
  # num_handshakes = 0
  # ctx.renegotiation_cb = lambda do |ssl|
  #   num_handshakes += 1
  #   raise RuntimeError.new("Client renegotiation disabled") if num_handshakes > 1
  # end
  # ```
  sig {returns(::T.untyped)}
  def renegotiation_cb(); end

  # A callback invoked whenever a new handshake is initiated. May be used to
  # disable renegotiation entirely.
  #
  # The callback is invoked with the active SSLSocket. The callback's return
  # value is irrelevant, normal return indicates "approval" of the renegotiation
  # and will continue the process. To forbid renegotiation and to cancel the
  # process, an [`Error`](https://docs.ruby-lang.org/en/2.7.0/Error.html) may be
  # raised within the callback.
  #
  # ### Disable client renegotiation
  #
  # When running a server, it is often desirable to disable client renegotiation
  # entirely. You may use a callback as follows to implement this feature:
  #
  # ```ruby
  # num_handshakes = 0
  # ctx.renegotiation_cb = lambda do |ssl|
  #   num_handshakes += 1
  #   raise RuntimeError.new("Client renegotiation disabled") if num_handshakes > 1
  # end
  # ```
  sig do
    params(
      renegotiation_cb: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def renegotiation_cb=(renegotiation_cb); end

  # Returns the security level for the context.
  #
  # See also
  # [`OpenSSL::SSL::SSLContext#security_level=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-security_level-3D).
  sig {returns(::T.untyped)}
  def security_level(); end

  # Sets the security level for the context.
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) limits
  # parameters according to the level. The "parameters" include: ciphersuites,
  # curves, key sizes, certificate signature algorithms, protocol version and so
  # on. For example, level 1 rejects parameters offering below 80 bits of
  # security, such as ciphersuites using MD5 for the MAC or RSA keys shorter
  # than 1024 bits.
  #
  # Note that attempts to set such parameters with insufficient security are
  # also blocked. You need to lower the level first.
  #
  # This feature is not supported in
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) < 1.1.0, and
  # setting the level to other than 0 will raise
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.7.0/NotImplementedError.html).
  # Level 0 means everything is permitted, the same behavior as previous
  # versions of [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html).
  #
  # See the manpage of SSL\_CTX\_set\_security\_level(3) for details.
  sig do
    params(
      security_level: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def security_level=(security_level); end

  # A callback invoked at connect time to distinguish between multiple server
  # names.
  #
  # The callback is invoked with an SSLSocket and a server name. The callback
  # must return an
  # [`SSLContext`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html)
  # for the server name or nil.
  sig {returns(::T.untyped)}
  def servername_cb(); end

  # A callback invoked at connect time to distinguish between multiple server
  # names.
  #
  # The callback is invoked with an SSLSocket and a server name. The callback
  # must return an
  # [`SSLContext`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html)
  # for the server name or nil.
  sig do
    params(
      servername_cb: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def servername_cb=(servername_cb); end

  # Adds *session* to the session cache.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def session_add(arg0); end

  # The current session cache mode.
  sig {returns(::T.untyped)}
  def session_cache_mode(); end

  # Sets the [`SSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL.html)
  # session cache mode. Bitwise-or together the desired SESSION\_CACHE\_\*
  # constants to set. See SSL\_CTX\_set\_session\_cache\_mode(3) for details.
  sig do
    params(
      session_cache_mode: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def session_cache_mode=(session_cache_mode); end

  # Returns the current session cache size. Zero is used to represent an
  # unlimited cache size.
  sig {returns(::T.untyped)}
  def session_cache_size(); end

  # Sets the session cache size. Returns the previously valid session cache
  # size. Zero is used to represent an unlimited session cache size.
  sig do
    params(
      session_cache_size: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def session_cache_size=(session_cache_size); end

  # Returns a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) containing
  # the following keys:
  #
  # :accept
  # :   Number of started SSL/TLS handshakes in server mode
  # :accept\_good
  # :   Number of established SSL/TLS sessions in server mode
  # :accept\_renegotiate
  # :   Number of start renegotiations in server mode
  # :cache\_full
  # :   Number of sessions that were removed due to cache overflow
  # :cache\_hits
  # :   Number of successfully reused connections
  # :cache\_misses
  # :   Number of sessions proposed by clients that were not found in the cache
  # :cache\_num
  # :   Number of sessions in the internal session cache
  # :cb\_hits
  # :   Number of sessions retrieved from the external cache in server mode
  # :connect
  # :   Number of started SSL/TLS handshakes in client mode
  # :connect\_good
  # :   Number of established SSL/TLS sessions in client mode
  # :connect\_renegotiate
  # :   Number of start renegotiations in client mode
  # :timeouts
  # :   Number of sessions proposed by clients that were found in the cache but
  #     had expired due to timeouts
  sig {returns(::T.untyped)}
  def session_cache_stats(); end

  # A callback invoked on a server when a session is proposed by the client but
  # the session could not be found in the server's internal cache.
  #
  # The callback is invoked with the SSLSocket and session id. The callback may
  # return a Session from an external cache.
  sig {returns(::T.untyped)}
  def session_get_cb(); end

  # A callback invoked on a server when a session is proposed by the client but
  # the session could not be found in the server's internal cache.
  #
  # The callback is invoked with the SSLSocket and session id. The callback may
  # return a Session from an external cache.
  sig do
    params(
      session_get_cb: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def session_get_cb=(session_get_cb); end

  # Sets the context in which a session can be reused. This allows sessions for
  # multiple applications to be distinguished, for example, by name.
  sig {returns(::T.untyped)}
  def session_id_context(); end

  # Sets the context in which a session can be reused. This allows sessions for
  # multiple applications to be distinguished, for example, by name.
  sig do
    params(
      session_id_context: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def session_id_context=(session_id_context); end

  # A callback invoked when a new session was negotiated.
  #
  # The callback is invoked with an SSLSocket. If `false` is returned the
  # session will be removed from the internal cache.
  sig {returns(::T.untyped)}
  def session_new_cb(); end

  # A callback invoked when a new session was negotiated.
  #
  # The callback is invoked with an SSLSocket. If `false` is returned the
  # session will be removed from the internal cache.
  sig do
    params(
      session_new_cb: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def session_new_cb=(session_new_cb); end

  # Removes *session* from the session cache.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def session_remove(arg0); end

  # A callback invoked when a session is removed from the internal cache.
  #
  # The callback is invoked with an
  # [`SSLContext`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html)
  # and a Session.
  #
  # IMPORTANT NOTE: It is currently not possible to use this safely in a
  # multi-threaded application. The callback is called inside a global lock and
  # it can randomly cause deadlock on Ruby thread switching.
  sig {returns(::T.untyped)}
  def session_remove_cb(); end

  # A callback invoked when a session is removed from the internal cache.
  #
  # The callback is invoked with an
  # [`SSLContext`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html)
  # and a Session.
  #
  # IMPORTANT NOTE: It is currently not possible to use this safely in a
  # multi-threaded application. The callback is called inside a global lock and
  # it can randomly cause deadlock on Ruby thread switching.
  sig do
    params(
      session_remove_cb: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def session_remove_cb=(session_remove_cb); end

  # Sets saner defaults optimized for the use with HTTP-like protocols.
  #
  # If a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) *params* is
  # given, the parameters are overridden with it. The keys in *params* must be
  # assignment methods on
  # [`SSLContext`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html).
  #
  # If the
  # [`verify_mode`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#attribute-i-verify_mode)
  # is not VERIFY\_NONE and
  # [`ca_file`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#attribute-i-ca_file),
  # [`ca_path`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#attribute-i-ca_path)
  # and
  # [`cert_store`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#attribute-i-cert_store)
  # are not set then the system default certificate store is used.
  sig do
    params(
      params: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def set_params(params=T.unsafe(nil)); end

  # This method is called automatically when a new SSLSocket is created.
  # However, it is not thread-safe and must be called before creating SSLSocket
  # objects in a multi-threaded program.
  #
  # Also aliased as:
  # [`freeze`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-freeze)
  sig {returns(::T.untyped)}
  def setup(); end

  # Maximum session lifetime in seconds.
  sig {returns(::T.untyped)}
  def ssl_timeout(); end

  # Maximum session lifetime in seconds.
  sig do
    params(
      ssl_timeout: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ssl_timeout=(ssl_timeout); end

  # Sets the SSL/TLS protocol version for the context. This forces connections
  # to use only the specified protocol version. This is deprecated and only
  # provided for backwards compatibility. Use
  # [`min_version=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-min_version-3D)
  # and
  # [`max_version=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-max_version-3D)
  # instead.
  #
  # ### History
  # As the name hints, this used to call the SSL\_CTX\_set\_ssl\_version()
  # function which sets the
  # [`SSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL.html) method used
  # for connections created from the context. As of Ruby/OpenSSL 2.1, this
  # accessor method is implemented to call
  # [`min_version=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-min_version-3D)
  # and
  # [`max_version=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-max_version-3D)
  # instead.
  sig do
    params(
      ssl_version: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ssl_version=(ssl_version); end

  # Maximum session lifetime in seconds.
  sig {returns(::T.untyped)}
  def timeout(); end

  # Maximum session lifetime in seconds.
  sig do
    params(
      timeout: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def timeout=(timeout); end

  # A callback invoked when DH parameters are required.
  #
  # The callback is invoked with the Session for the key exchange, an flag
  # indicating the use of an export cipher and the keylength required.
  #
  # The callback must return an
  # [`OpenSSL::PKey::DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html)
  # instance of the correct key length.
  sig {returns(::T.untyped)}
  def tmp_dh_callback(); end

  # A callback invoked when DH parameters are required.
  #
  # The callback is invoked with the Session for the key exchange, an flag
  # indicating the use of an export cipher and the keylength required.
  #
  # The callback must return an
  # [`OpenSSL::PKey::DH`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DH.html)
  # instance of the correct key length.
  sig do
    params(
      tmp_dh_callback: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def tmp_dh_callback=(tmp_dh_callback); end

  # A callback invoked when ECDH parameters are required.
  #
  # The callback is invoked with the Session for the key exchange, an flag
  # indicating the use of an export cipher and the keylength required.
  #
  # The callback is deprecated. This does not work with recent versions of
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html). Use
  # [`OpenSSL::SSL::SSLContext#ecdh_curves=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-ecdh_curves-3D)
  # instead.
  sig {returns(::T.untyped)}
  def tmp_ecdh_callback(); end

  # A callback invoked when ECDH parameters are required.
  #
  # The callback is invoked with the Session for the key exchange, an flag
  # indicating the use of an export cipher and the keylength required.
  #
  # The callback is deprecated. This does not work with recent versions of
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html). Use
  # [`OpenSSL::SSL::SSLContext#ecdh_curves=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#method-i-ecdh_curves-3D)
  # instead.
  sig do
    params(
      tmp_ecdh_callback: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def tmp_ecdh_callback=(tmp_ecdh_callback); end

  # A callback for additional certificate verification. The callback is invoked
  # for each certificate in the chain.
  #
  # The callback is invoked with two values. *preverify\_ok* indicates indicates
  # if the verification was passed (`true`) or not (`false`). *store\_context*
  # is an
  # [`OpenSSL::X509::StoreContext`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/StoreContext.html)
  # containing the context used for certificate verification.
  #
  # If the callback returns `false`, the chain verification is immediately
  # stopped and a bad\_certificate alert is then sent.
  sig {returns(::T.untyped)}
  def verify_callback(); end

  # A callback for additional certificate verification. The callback is invoked
  # for each certificate in the chain.
  #
  # The callback is invoked with two values. *preverify\_ok* indicates indicates
  # if the verification was passed (`true`) or not (`false`). *store\_context*
  # is an
  # [`OpenSSL::X509::StoreContext`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/StoreContext.html)
  # containing the context used for certificate verification.
  #
  # If the callback returns `false`, the chain verification is immediately
  # stopped and a bad\_certificate alert is then sent.
  sig do
    params(
      verify_callback: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def verify_callback=(verify_callback); end

  # Number of CA certificates to walk when verifying a certificate chain.
  sig {returns(::T.untyped)}
  def verify_depth(); end

  # Number of CA certificates to walk when verifying a certificate chain.
  sig do
    params(
      verify_depth: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def verify_depth=(verify_depth); end

  # Whether to check the server certificate is valid for the hostname.
  #
  # In order to make this work,
  # [`verify_mode`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#attribute-i-verify_mode)
  # must be set to VERIFY\_PEER and the server hostname must be given by
  # [`OpenSSL::SSL::SSLSocket#hostname=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html#method-i-hostname-3D).
  sig {returns(::T.untyped)}
  def verify_hostname(); end

  # Whether to check the server certificate is valid for the hostname.
  #
  # In order to make this work,
  # [`verify_mode`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLContext.html#attribute-i-verify_mode)
  # must be set to VERIFY\_PEER and the server hostname must be given by
  # [`OpenSSL::SSL::SSLSocket#hostname=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html#method-i-hostname-3D).
  sig do
    params(
      verify_hostname: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def verify_hostname=(verify_hostname); end

  # Session verification mode.
  #
  # Valid modes are VERIFY\_NONE, VERIFY\_PEER, VERIFY\_CLIENT\_ONCE,
  # VERIFY\_FAIL\_IF\_NO\_PEER\_CERT and defined on
  # [`OpenSSL::SSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL.html)
  #
  # The default mode is VERIFY\_NONE, which does not perform any verification at
  # all.
  #
  # See SSL\_CTX\_set\_verify(3) for details.
  sig {returns(::T.untyped)}
  def verify_mode(); end

  # Session verification mode.
  #
  # Valid modes are VERIFY\_NONE, VERIFY\_PEER, VERIFY\_CLIENT\_ONCE,
  # VERIFY\_FAIL\_IF\_NO\_PEER\_CERT and defined on
  # [`OpenSSL::SSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL.html)
  #
  # The default mode is VERIFY\_NONE, which does not perform any verification at
  # all.
  #
  # See SSL\_CTX\_set\_verify(3) for details.
  sig do
    params(
      verify_mode: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def verify_mode=(verify_mode); end
end

# Generic error class raised by SSLSocket and SSLContext.
class OpenSSL::SSL::SSLError < OpenSSL::OpenSSLError
end

class OpenSSL::SSL::SSLErrorWaitReadable < OpenSSL::SSL::SSLError
  include ::IO::WaitReadable
end

class OpenSSL::SSL::SSLErrorWaitWritable < OpenSSL::SSL::SSLError
  include ::IO::WaitWritable
end

# [`SSLServer`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLServer.html)
# represents a TCP/IP server socket with Secure Sockets Layer.
class OpenSSL::SSL::SSLServer
  include ::OpenSSL::SSL::SocketForwarder
  # Works similar to
  # [`TCPServer#accept`](https://docs.ruby-lang.org/en/2.7.0/TCPServer.html#method-i-accept).
  sig {returns(::T.untyped)}
  def accept(); end

  # See [`IO#close`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-close)
  # for details.
  sig {returns(::T.untyped)}
  def close(); end

  sig do
    params(
      svr: ::T.untyped,
      ctx: ::T.untyped,
    )
    .void
  end
  def initialize(svr, ctx); end

  # See
  # [`TCPServer#listen`](https://docs.ruby-lang.org/en/2.7.0/TCPServer.html#method-i-listen)
  # for details.
  sig do
    params(
      backlog: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def listen(backlog=T.unsafe(nil)); end

  # See
  # [`BasicSocket#shutdown`](https://docs.ruby-lang.org/en/2.7.0/BasicSocket.html#method-i-shutdown)
  # for details.
  sig do
    params(
      how: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def shutdown(how=T.unsafe(nil)); end

  # When true then
  # [`accept`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLServer.html#method-i-accept)
  # works exactly the same as
  # [`TCPServer#accept`](https://docs.ruby-lang.org/en/2.7.0/TCPServer.html#method-i-accept)
  sig {returns(::T.untyped)}
  def start_immediately(); end

  # When true then
  # [`accept`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLServer.html#method-i-accept)
  # works exactly the same as
  # [`TCPServer#accept`](https://docs.ruby-lang.org/en/2.7.0/TCPServer.html#method-i-accept)
  sig do
    params(
      start_immediately: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def start_immediately=(start_immediately); end

  # Returns the
  # [`TCPServer`](https://docs.ruby-lang.org/en/2.7.0/TCPServer.html) passed to
  # the
  # [`SSLServer`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLServer.html)
  # when initialized.
  sig {returns(::T.untyped)}
  def to_io(); end
end

class OpenSSL::SSL::SSLSocket
  Elem = type_member {{fixed: T.untyped}}

  include ::OpenSSL::SSL::SocketForwarder
  include ::OpenSSL::Buffering
  include ::Enumerable
  # Waits for a SSL/TLS client to initiate a handshake. The handshake may be
  # started after unencrypted data has been sent over the socket.
  sig {returns(::T.untyped)}
  def accept(); end

  # Initiates the SSL/TLS handshake as a server in non-blocking manner.
  #
  # ```ruby
  # # emulates blocking accept
  # begin
  #   ssl.accept_nonblock
  # rescue IO::WaitReadable
  #   IO.select([s2])
  #   retry
  # rescue IO::WaitWritable
  #   IO.select(nil, [s2])
  #   retry
  # end
  # ```
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`accept_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html#method-i-accept_nonblock)
  # should not raise an
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # or
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html)
  # exception, but return the symbol `:wait_readable` or `:wait_writable`
  # instead.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def accept_nonblock(*arg0); end

  # The X509 certificate for this socket endpoint.
  sig {returns(::T.untyped)}
  def cert(); end

  # Returns the cipher suite actually used in the current session, or nil if no
  # session has been established.
  sig {returns(::T.untyped)}
  def cipher(); end

  # Returns the list of client CAs. Please note that in contrast to
  # SSLContext#client\_ca= no array of X509::Certificate is returned but
  # X509::Name instances of the CA's subject distinguished name.
  #
  # In server mode, returns the list set by SSLContext#client\_ca=. In client
  # mode, returns the list of client CAs sent from the server.
  sig {returns(::T.untyped)}
  def client_ca(); end

  # Initiates an SSL/TLS handshake with a server. The handshake may be started
  # after unencrypted data has been sent over the socket.
  sig {returns(::T.untyped)}
  def connect(); end

  # Initiates the SSL/TLS handshake as a client in non-blocking manner.
  #
  # ```ruby
  # # emulates blocking connect
  # begin
  #   ssl.connect_nonblock
  # rescue IO::WaitReadable
  #   IO.select([s2])
  #   retry
  # rescue IO::WaitWritable
  #   IO.select(nil, [s2])
  #   retry
  # end
  # ```
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`connect_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html#method-i-connect_nonblock)
  # should not raise an
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # or
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html)
  # exception, but return the symbol `:wait_readable` or `:wait_writable`
  # instead.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def connect_nonblock(*arg0); end

  # The SSLContext object used in this connection.
  sig {returns(::T.untyped)}
  def context(); end

  sig {returns(::T.untyped)}
  def hostname(); end

  # Sets the server hostname used for SNI. This needs to be set before
  # [`SSLSocket#connect`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html#method-i-connect).
  sig do
    params(
      hostname: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def hostname=(hostname); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # The underlying [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object.
  sig {returns(::T.untyped)}
  def io(); end

  # Returns the protocol string that was finally selected by the client during
  # the handshake.
  sig {returns(::T.untyped)}
  def npn_protocol(); end

  # The X509 certificate for this socket's peer.
  sig {returns(::T.untyped)}
  def peer_cert(); end

  # The X509 certificate chain for this socket's peer.
  sig {returns(::T.untyped)}
  def peer_cert_chain(); end

  # The number of bytes that are immediately available for reading.
  sig {returns(::T.untyped)}
  def pending(); end

  # Perform hostname verification following RFC 6125.
  #
  # This method MUST be called after calling
  # [`connect`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html#method-i-connect)
  # to ensure that the hostname of a remote peer has been verified.
  sig do
    params(
      hostname: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def post_connection_check(hostname); end

  # Returns the SSLSession object currently used, or nil if the session is not
  # established.
  sig {returns(::T.untyped)}
  def session(); end

  # Sets the Session to be used when the connection is established.
  sig do
    params(
      session: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def session=(session); end

  # Returns `true` if a reused session was negotiated during the handshake.
  sig {returns(::T.untyped)}
  def session_reused?(); end

  # Returns a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # representing the SSL/TLS version that was negotiated for the connection, for
  # example "TLSv1.2".
  sig {returns(::T.untyped)}
  def ssl_version(); end

  # A description of the current connection state. This is for diagnostic
  # purposes only.
  sig {returns(::T.untyped)}
  def state(); end

  # Whether to close the underlying socket as well, when the SSL/TLS connection
  # is shut down. This defaults to `false`.
  sig {returns(::T.untyped)}
  def sync_close(); end

  # Whether to close the underlying socket as well, when the SSL/TLS connection
  # is shut down. This defaults to `false`.
  sig do
    params(
      sync_close: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def sync_close=(sync_close); end

  # Sends "close notify" to the peer and tries to shut down the
  # [`SSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL.html) connection
  # gracefully.
  #
  # If
  # [`sync_close`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html#attribute-i-sync_close)
  # is set to `true`, the underlying
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) is also closed.
  sig {returns(::T.untyped)}
  def sysclose(); end

  # Reads *length* bytes from the
  # [`SSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL.html) connection. If
  # a pre-allocated *buffer* is provided the data will be written into it.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def sysread(*arg0); end

  # Writes *string* to the
  # [`SSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL.html) connection.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def syswrite(arg0); end

  # The underlying [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object.
  sig {returns(::T.untyped)}
  def to_io(); end

  # Returns the result of the peer certificates verification. See verify(1) for
  # error values and descriptions.
  #
  # If no peer certificate was presented X509\_V\_OK is returned.
  sig {returns(::T.untyped)}
  def verify_result(); end
end

class OpenSSL::SSL::Session
  # Returns `true` if the two
  # [`Session`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/Session.html) is
  # the same, `false` if not.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ==(arg0); end

  # Returns the
  # [`Session`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/Session.html)
  # ID.
  sig {returns(::T.untyped)}
  def id(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(arg0); end

  # Returns the time at which the session was established.
  sig {returns(::T.untyped)}
  def time(); end

  # Sets start time of the session.
  # [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) resolution is in
  # seconds.
  sig do
    params(
      time: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def time=(time); end

  # Returns the timeout value set for the session, in seconds from the
  # established time.
  sig {returns(::T.untyped)}
  def timeout(); end

  # Sets how long until the session expires in seconds.
  sig do
    params(
      timeout: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def timeout=(timeout); end

  # Returns an ASN1 encoded
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) that contains
  # the
  # [`Session`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/Session.html)
  # object.
  sig {returns(::T.untyped)}
  def to_der(); end

  # Returns a PEM encoded
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) that contains
  # the
  # [`Session`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/Session.html)
  # object.
  sig {returns(::T.untyped)}
  def to_pem(); end

  # Shows everything in the
  # [`Session`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/Session.html)
  # object. This is for diagnostic purposes.
  sig {returns(::T.untyped)}
  def to_text(); end
end

class OpenSSL::SSL::Session::SessionError < OpenSSL::OpenSSLError
end

module OpenSSL::SSL::SocketForwarder
  sig {returns(::T.untyped)}
  def addr(); end

  sig {returns(::T.untyped)}
  def closed?(); end

  sig do
    params(
      flag: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def do_not_reverse_lookup=(flag); end

  sig do
    params(
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def fcntl(*args); end

  sig do
    params(
      level: ::T.untyped,
      optname: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def getsockopt(level, optname); end

  sig {returns(::T.untyped)}
  def peeraddr(); end

  sig do
    params(
      level: ::T.untyped,
      optname: ::T.untyped,
      optval: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def setsockopt(level, optname, optval); end
end

module OpenSSL::X509
  DEFAULT_CERT_AREA = ::T.let(nil, ::T.untyped)
  DEFAULT_CERT_DIR = ::T.let(nil, ::T.untyped)
  DEFAULT_CERT_DIR_ENV = ::T.let(nil, ::T.untyped)
  DEFAULT_CERT_FILE = ::T.let(nil, ::T.untyped)
  DEFAULT_CERT_FILE_ENV = ::T.let(nil, ::T.untyped)
  DEFAULT_PRIVATE_DIR = ::T.let(nil, ::T.untyped)
  PURPOSE_ANY = ::T.let(nil, ::T.untyped)
  PURPOSE_CRL_SIGN = ::T.let(nil, ::T.untyped)
  PURPOSE_NS_SSL_SERVER = ::T.let(nil, ::T.untyped)
  PURPOSE_OCSP_HELPER = ::T.let(nil, ::T.untyped)
  PURPOSE_SMIME_ENCRYPT = ::T.let(nil, ::T.untyped)
  PURPOSE_SMIME_SIGN = ::T.let(nil, ::T.untyped)
  PURPOSE_SSL_CLIENT = ::T.let(nil, ::T.untyped)
  PURPOSE_SSL_SERVER = ::T.let(nil, ::T.untyped)
  PURPOSE_TIMESTAMP_SIGN = ::T.let(nil, ::T.untyped)
  TRUST_COMPAT = ::T.let(nil, ::T.untyped)
  TRUST_EMAIL = ::T.let(nil, ::T.untyped)
  TRUST_OBJECT_SIGN = ::T.let(nil, ::T.untyped)
  TRUST_OCSP_REQUEST = ::T.let(nil, ::T.untyped)
  TRUST_OCSP_SIGN = ::T.let(nil, ::T.untyped)
  TRUST_SSL_CLIENT = ::T.let(nil, ::T.untyped)
  TRUST_SSL_SERVER = ::T.let(nil, ::T.untyped)
  TRUST_TSA = ::T.let(nil, ::T.untyped)
  V_ERR_AKID_ISSUER_SERIAL_MISMATCH = ::T.let(nil, ::T.untyped)
  V_ERR_AKID_SKID_MISMATCH = ::T.let(nil, ::T.untyped)
  V_ERR_APPLICATION_VERIFICATION = ::T.let(nil, ::T.untyped)
  V_ERR_CERT_CHAIN_TOO_LONG = ::T.let(nil, ::T.untyped)
  V_ERR_CERT_HAS_EXPIRED = ::T.let(nil, ::T.untyped)
  V_ERR_CERT_NOT_YET_VALID = ::T.let(nil, ::T.untyped)
  V_ERR_CERT_REJECTED = ::T.let(nil, ::T.untyped)
  V_ERR_CERT_REVOKED = ::T.let(nil, ::T.untyped)
  V_ERR_CERT_SIGNATURE_FAILURE = ::T.let(nil, ::T.untyped)
  V_ERR_CERT_UNTRUSTED = ::T.let(nil, ::T.untyped)
  V_ERR_CRL_HAS_EXPIRED = ::T.let(nil, ::T.untyped)
  V_ERR_CRL_NOT_YET_VALID = ::T.let(nil, ::T.untyped)
  V_ERR_CRL_SIGNATURE_FAILURE = ::T.let(nil, ::T.untyped)
  V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT = ::T.let(nil, ::T.untyped)
  V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD = ::T.let(nil, ::T.untyped)
  V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD = ::T.let(nil, ::T.untyped)
  V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD = ::T.let(nil, ::T.untyped)
  V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD = ::T.let(nil, ::T.untyped)
  V_ERR_INVALID_CA = ::T.let(nil, ::T.untyped)
  V_ERR_INVALID_PURPOSE = ::T.let(nil, ::T.untyped)
  V_ERR_KEYUSAGE_NO_CERTSIGN = ::T.let(nil, ::T.untyped)
  V_ERR_OUT_OF_MEM = ::T.let(nil, ::T.untyped)
  V_ERR_PATH_LENGTH_EXCEEDED = ::T.let(nil, ::T.untyped)
  V_ERR_SELF_SIGNED_CERT_IN_CHAIN = ::T.let(nil, ::T.untyped)
  V_ERR_SUBJECT_ISSUER_MISMATCH = ::T.let(nil, ::T.untyped)
  V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY = ::T.let(nil, ::T.untyped)
  V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE = ::T.let(nil, ::T.untyped)
  V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE = ::T.let(nil, ::T.untyped)
  V_ERR_UNABLE_TO_GET_CRL = ::T.let(nil, ::T.untyped)
  V_ERR_UNABLE_TO_GET_ISSUER_CERT = ::T.let(nil, ::T.untyped)
  V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY = ::T.let(nil, ::T.untyped)
  V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE = ::T.let(nil, ::T.untyped)
  V_FLAG_ALLOW_PROXY_CERTS = ::T.let(nil, ::T.untyped)
  V_FLAG_CHECK_SS_SIGNATURE = ::T.let(nil, ::T.untyped)
  V_FLAG_CRL_CHECK = ::T.let(nil, ::T.untyped)
  V_FLAG_CRL_CHECK_ALL = ::T.let(nil, ::T.untyped)
  V_FLAG_EXPLICIT_POLICY = ::T.let(nil, ::T.untyped)
  V_FLAG_EXTENDED_CRL_SUPPORT = ::T.let(nil, ::T.untyped)
  V_FLAG_IGNORE_CRITICAL = ::T.let(nil, ::T.untyped)
  V_FLAG_INHIBIT_ANY = ::T.let(nil, ::T.untyped)
  V_FLAG_INHIBIT_MAP = ::T.let(nil, ::T.untyped)
  V_FLAG_NOTIFY_POLICY = ::T.let(nil, ::T.untyped)
  V_FLAG_NO_ALT_CHAINS = ::T.let(nil, ::T.untyped)
  V_FLAG_POLICY_CHECK = ::T.let(nil, ::T.untyped)
  V_FLAG_USE_DELTAS = ::T.let(nil, ::T.untyped)
  V_FLAG_X509_STRICT = ::T.let(nil, ::T.untyped)
  V_OK = ::T.let(nil, ::T.untyped)

end

class OpenSSL::X509::Attribute
  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  sig {returns(::T.untyped)}
  def oid(); end

  sig do
    params(
      oid: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def oid=(oid); end

  sig {returns(::T.untyped)}
  def to_der(); end

  sig {returns(::T.untyped)}
  def value(); end

  sig do
    params(
      value: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def value=(value); end
end

class OpenSSL::X509::AttributeError < OpenSSL::OpenSSLError
end

class OpenSSL::X509::CRL
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_extension(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_revoked(arg0); end

  # Gets X509v3 extensions as array of X509Ext objects
  sig {returns(::T.untyped)}
  def extensions(); end

  # Sets X509\_EXTENSIONs
  sig do
    params(
      extensions: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def extensions=(extensions); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  sig {returns(::T.untyped)}
  def issuer(); end

  sig do
    params(
      issuer: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def issuer=(issuer); end

  sig {returns(::T.untyped)}
  def last_update(); end

  sig do
    params(
      last_update: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def last_update=(last_update); end

  sig {returns(::T.untyped)}
  def next_update(); end

  sig do
    params(
      next_update: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def next_update=(next_update); end

  sig {returns(::T.untyped)}
  def revoked(); end

  sig do
    params(
      revoked: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def revoked=(revoked); end

  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def sign(arg0, arg1); end

  sig {returns(::T.untyped)}
  def signature_algorithm(); end

  sig {returns(::T.untyped)}
  def to_der(); end

  # Also aliased as:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/CRL.html#method-i-to_s)
  sig {returns(::T.untyped)}
  def to_pem(); end

  # Alias for:
  # [`to_pem`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/CRL.html#method-i-to_pem)
  sig {returns(::T.untyped)}
  def to_s(); end

  sig {returns(::T.untyped)}
  def to_text(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def verify(arg0); end

  sig {returns(::T.untyped)}
  def version(); end

  sig do
    params(
      version: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def version=(version); end
end

class OpenSSL::X509::CRLError < OpenSSL::OpenSSLError
end

# Implementation of an X.509 certificate as specified in RFC 5280. Provides
# access to a certificate's attributes and allows certificates to be read from a
# string, but also supports the creation of new certificates from scratch.
#
# ### Reading a certificate from a file
#
# [`Certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Certificate.html)
# is capable of handling DER-encoded certificates and certificates encoded in
# OpenSSL's PEM format.
#
# ```ruby
# raw = File.read "cert.cer" # DER- or PEM-encoded
# certificate = OpenSSL::X509::Certificate.new raw
# ```
#
# ### Saving a certificate to a file
#
# A certificate may be encoded in DER format
#
# ```ruby
# cert = ...
# File.open("cert.cer", "wb") { |f| f.print cert.to_der }
# ```
#
# or in PEM format
#
# ```ruby
# cert = ...
# File.open("cert.pem", "wb") { |f| f.print cert.to_pem }
# ```
#
# X.509 certificates are associated with a private/public key pair, typically a
# RSA, DSA or ECC key (see also
# [`OpenSSL::PKey::RSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/RSA.html),
# [`OpenSSL::PKey::DSA`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/DSA.html)
# and
# [`OpenSSL::PKey::EC`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey/EC.html)),
# the public key itself is stored within the certificate and can be accessed in
# form of an
# [`OpenSSL::PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html).
# Certificates are typically used to be able to associate some form of identity
# with a key pair, for example web servers serving pages over HTTPs use
# certificates to authenticate themselves to the user.
#
# The public key infrastructure (PKI) model relies on trusted certificate
# authorities ("root CAs") that issue these certificates, so that end users need
# to base their trust just on a selected few authorities that themselves again
# vouch for subordinate CAs issuing their certificates to end users.
#
# The [`OpenSSL::X509`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509.html)
# module provides the tools to set up an independent PKI, similar to scenarios
# where the 'openssl' command line tool is used for issuing certificates in a
# private PKI.
#
# ### Creating a root CA certificate and an end-entity certificate
#
# First, we need to create a "self-signed" root certificate. To do so, we need
# to generate a key first. Please note that the choice of "1" as a serial number
# is considered a security flaw for real certificates. Secure choices are
# integers in the two-digit byte range and ideally not sequential but secure
# random numbers, steps omitted here to keep the example concise.
#
# ```ruby
# root_key = OpenSSL::PKey::RSA.new 2048 # the CA's public/private key
# root_ca = OpenSSL::X509::Certificate.new
# root_ca.version = 2 # cf. RFC 5280 - to make it a "v3" certificate
# root_ca.serial = 1
# root_ca.subject = OpenSSL::X509::Name.parse "/DC=org/DC=ruby-lang/CN=Ruby CA"
# root_ca.issuer = root_ca.subject # root CA's are "self-signed"
# root_ca.public_key = root_key.public_key
# root_ca.not_before = Time.now
# root_ca.not_after = root_ca.not_before + 2 * 365 * 24 * 60 * 60 # 2 years validity
# ef = OpenSSL::X509::ExtensionFactory.new
# ef.subject_certificate = root_ca
# ef.issuer_certificate = root_ca
# root_ca.add_extension(ef.create_extension("basicConstraints","CA:TRUE",true))
# root_ca.add_extension(ef.create_extension("keyUsage","keyCertSign, cRLSign", true))
# root_ca.add_extension(ef.create_extension("subjectKeyIdentifier","hash",false))
# root_ca.add_extension(ef.create_extension("authorityKeyIdentifier","keyid:always",false))
# root_ca.sign(root_key, OpenSSL::Digest::SHA256.new)
# ```
#
# The next step is to create the end-entity certificate using the root CA
# certificate.
#
# ```ruby
# key = OpenSSL::PKey::RSA.new 2048
# cert = OpenSSL::X509::Certificate.new
# cert.version = 2
# cert.serial = 2
# cert.subject = OpenSSL::X509::Name.parse "/DC=org/DC=ruby-lang/CN=Ruby certificate"
# cert.issuer = root_ca.subject # root CA is the issuer
# cert.public_key = key.public_key
# cert.not_before = Time.now
# cert.not_after = cert.not_before + 1 * 365 * 24 * 60 * 60 # 1 years validity
# ef = OpenSSL::X509::ExtensionFactory.new
# ef.subject_certificate = cert
# ef.issuer_certificate = root_ca
# cert.add_extension(ef.create_extension("keyUsage","digitalSignature", true))
# cert.add_extension(ef.create_extension("subjectKeyIdentifier","hash",false))
# cert.sign(root_key, OpenSSL::Digest::SHA256.new)
# ```
class OpenSSL::X509::Certificate
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_extension(arg0); end

  # Returns `true` if *key* is the corresponding private key to the Subject
  # Public Key Information, `false` otherwise.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def check_private_key(arg0); end

  sig {returns(::T.untyped)}
  def extensions(); end

  sig do
    params(
      extensions: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def extensions=(extensions); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  sig {returns(::T.untyped)}
  def inspect(); end

  sig {returns(::T.untyped)}
  def issuer(); end

  sig do
    params(
      issuer: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def issuer=(issuer); end

  sig {returns(::T.untyped)}
  def not_after(); end

  sig do
    params(
      not_after: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def not_after=(not_after); end

  sig {returns(::T.untyped)}
  def not_before(); end

  sig do
    params(
      not_before: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def not_before=(not_before); end

  sig do
    params(
      q: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def pretty_print(q); end

  sig {returns(::T.untyped)}
  def public_key(); end

  sig do
    params(
      public_key: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def public_key=(public_key); end

  sig {returns(::T.untyped)}
  def serial(); end

  sig do
    params(
      serial: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def serial=(serial); end

  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def sign(arg0, arg1); end

  sig {returns(::T.untyped)}
  def signature_algorithm(); end

  sig {returns(::T.untyped)}
  def subject(); end

  sig do
    params(
      subject: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def subject=(subject); end

  sig {returns(::T.untyped)}
  def to_der(); end

  # Also aliased as:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Certificate.html#method-i-to_s)
  sig {returns(::T.untyped)}
  def to_pem(); end

  # Alias for:
  # [`to_pem`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Certificate.html#method-i-to_pem)
  sig {returns(::T.untyped)}
  def to_s(); end

  sig {returns(::T.untyped)}
  def to_text(); end

  # Verifies the signature of the certificate, with the public key *key*. *key*
  # must be an instance of
  # [`OpenSSL::PKey`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/PKey.html).
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def verify(arg0); end

  sig {returns(::T.untyped)}
  def version(); end

  sig do
    params(
      version: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def version=(version); end
end

class OpenSSL::X509::CertificateError < OpenSSL::OpenSSLError
end

class OpenSSL::X509::Extension
  sig do
    params(
      critical: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def critical=(critical); end

  sig {returns(::T.untyped)}
  def critical?(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  sig {returns(::T.untyped)}
  def oid(); end

  sig do
    params(
      oid: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def oid=(oid); end

  sig {returns(::T.untyped)}
  def to_a(); end

  sig {returns(::T.untyped)}
  def to_der(); end

  sig {returns(::T.untyped)}
  def to_h(); end

  sig {returns(::T.untyped)}
  def to_s(); end

  sig {returns(::T.untyped)}
  def value(); end

  sig do
    params(
      value: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def value=(value); end
end

class OpenSSL::X509::ExtensionError < OpenSSL::OpenSSLError
end

class OpenSSL::X509::ExtensionFactory
  sig {returns(::T.untyped)}
  def config(); end

  sig do
    params(
      config: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def config=(config); end

  # Creates a new
  # [`X509::Extension`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Extension.html)
  # with passed values. See also x509v3\_config(5).
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def create_ext(*arg0); end

  sig do
    params(
      ary: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def create_ext_from_array(ary); end

  sig do
    params(
      hash: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def create_ext_from_hash(hash); end

  sig do
    params(
      str: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def create_ext_from_string(str); end

  sig do
    params(
      arg: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def create_extension(*arg); end

  sig {returns(::T.untyped)}
  def crl(); end

  sig do
    params(
      crl: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def crl=(crl); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  sig {returns(::T.untyped)}
  def issuer_certificate(); end

  sig do
    params(
      issuer_certificate: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def issuer_certificate=(issuer_certificate); end

  sig {returns(::T.untyped)}
  def subject_certificate(); end

  sig do
    params(
      subject_certificate: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def subject_certificate=(subject_certificate); end

  sig {returns(::T.untyped)}
  def subject_request(); end

  sig do
    params(
      subject_request: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def subject_request=(subject_request); end
end

# An X.509 name represents a hostname, email address or other entity associated
# with a public key.
#
# You can create a
# [`Name`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html) by
# parsing a distinguished name
# [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) or by supplying
# the distinguished name as an
# [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
#
# ```ruby
# name = OpenSSL::X509::Name.parse 'CN=nobody/DC=example'
#
# name = OpenSSL::X509::Name.new [['CN', 'nobody'], ['DC', 'example']]
# ```
class OpenSSL::X509::Name
  include ::Comparable
  # A flag for
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html#method-i-to_s).
  #
  # Breaks the name returned into multiple lines if longer than 80 characters.
  COMPAT = ::T.let(nil, ::T.untyped)
  # The default object type for name entries.
  DEFAULT_OBJECT_TYPE = ::T.let(nil, ::T.untyped)
  # A flag for
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html#method-i-to_s).
  #
  # Returns a multiline format.
  MULTILINE = ::T.let(nil, ::T.untyped)
  # The default object type template for name entries.
  OBJECT_TYPE_TEMPLATE = ::T.let(nil, ::T.untyped)
  # A flag for
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html#method-i-to_s).
  #
  # Returns a more readable format than
  # [`RFC2253`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html#RFC2253).
  ONELINE = ::T.let(nil, ::T.untyped)
  # A flag for
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html#method-i-to_s).
  #
  # Returns an
  # [`RFC2253`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html#RFC2253)
  # format name.
  RFC2253 = ::T.let(nil, ::T.untyped)

  # Alias for:
  # [`cmp`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html#method-i-cmp)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def <=>(arg0); end

  # Adds a new entry with the given *oid* and *value* to this name. The *oid* is
  # an object identifier defined in ASN.1. Some common OIDs are:
  #
  # C
  # :   Country
  #     [`Name`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html)
  # CN
  # :   Common
  #     [`Name`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html)
  # DC
  # :   Domain Component
  # O
  # :   Organization
  #     [`Name`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html)
  # OU
  # :   Organizational Unit
  #     [`Name`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html)
  # ST
  # :   State or Province
  #     [`Name`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html)
  #
  #
  # The optional keyword parameters *loc* and *set* specify where to insert the
  # new attribute. Refer to the manpage of X509\_NAME\_add\_entry(3) for
  # details. *loc* defaults to -1 and *set* defaults to 0. This appends a
  # single-valued RDN to the end.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_entry(*arg0); end

  # Compares this
  # [`Name`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html) with
  # *other* and returns `0` if they are the same and `-1` or +`1` if they are
  # greater or less than each other respectively.
  #
  # Also aliased as:
  # [`<=>`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html#method-i-3C-3D-3E)
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def cmp(arg0); end

  # Returns true if *name* and *other* refer to the same hash key.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def eql?(arg0); end

  # The hash value returned is suitable for use as a certificate's filename in a
  # CA path.
  sig {returns(::T.untyped)}
  def hash(); end

  # Returns an MD5 based hash used in
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) 0.9.X.
  sig {returns(::T.untyped)}
  def hash_old(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  sig do
    params(
      q: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def pretty_print(q); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
  # representation of the distinguished name suitable for passing to
  # [`::new`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html#method-c-new)
  sig {returns(::T.untyped)}
  def to_a(); end

  # Converts the name to DER encoding
  sig {returns(::T.untyped)}
  def to_der(); end

  # Returns a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # representation of the Distinguished
  # [`Name`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html).
  # *format* is one of:
  #
  # *   [`OpenSSL::X509::Name::COMPAT`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html#COMPAT)
  # *   [`OpenSSL::X509::Name::RFC2253`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html#RFC2253)
  # *   [`OpenSSL::X509::Name::ONELINE`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html#ONELINE)
  # *   [`OpenSSL::X509::Name::MULTILINE`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html#MULTILINE)
  #
  #
  # If *format* is omitted, the largely broken and traditional
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) format is
  # used.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def to_s(*arg0); end

  # Alias for:
  # [`parse_openssl`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html#method-c-parse_openssl)
  sig do
    params(
      str: ::T.untyped,
      template: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.parse(str, template=T.unsafe(nil)); end

  # Also aliased as:
  # [`parse`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Name.html#method-c-parse)
  sig do
    params(
      str: ::T.untyped,
      template: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.parse_openssl(str, template=T.unsafe(nil)); end

  sig do
    params(
      str: ::T.untyped,
      template: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.parse_rfc2253(str, template=T.unsafe(nil)); end
end

module OpenSSL::X509::Name::RFC2253DN
  AttributeType = ::T.let(nil, ::T.untyped)
  AttributeValue = ::T.let(nil, ::T.untyped)
  HexChar = ::T.let(nil, ::T.untyped)
  HexPair = ::T.let(nil, ::T.untyped)
  HexString = ::T.let(nil, ::T.untyped)
  Pair = ::T.let(nil, ::T.untyped)
  QuoteChar = ::T.let(nil, ::T.untyped)
  Special = ::T.let(nil, ::T.untyped)
  StringChar = ::T.let(nil, ::T.untyped)
  TypeAndValue = ::T.let(nil, ::T.untyped)

  sig do
    params(
      str: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.expand_hexstring(str); end

  sig do
    params(
      str: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.expand_pair(str); end

  sig do
    params(
      str1: ::T.untyped,
      str2: ::T.untyped,
      str3: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.expand_value(str1, str2, str3); end

  sig do
    params(
      dn: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.scan(dn); end
end

class OpenSSL::X509::NameError < OpenSSL::OpenSSLError
end

class OpenSSL::X509::Request
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_attribute(arg0); end

  sig {returns(::T.untyped)}
  def attributes(); end

  sig do
    params(
      attributes: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def attributes=(attributes); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  sig {returns(::T.untyped)}
  def public_key(); end

  sig do
    params(
      public_key: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def public_key=(public_key); end

  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def sign(arg0, arg1); end

  sig {returns(::T.untyped)}
  def signature_algorithm(); end

  sig {returns(::T.untyped)}
  def subject(); end

  sig do
    params(
      subject: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def subject=(subject); end

  sig {returns(::T.untyped)}
  def to_der(); end

  # Also aliased as:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Request.html#method-i-to_s)
  sig {returns(::T.untyped)}
  def to_pem(); end

  # Alias for:
  # [`to_pem`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Request.html#method-i-to_pem)
  sig {returns(::T.untyped)}
  def to_s(); end

  sig {returns(::T.untyped)}
  def to_text(); end

  # Checks that cert signature is made with PRIVversion of this PUBLIC 'key'
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def verify(arg0); end

  sig {returns(::T.untyped)}
  def version(); end

  sig do
    params(
      version: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def version=(version); end
end

class OpenSSL::X509::RequestError < OpenSSL::OpenSSLError
end

class OpenSSL::X509::Revoked
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_extension(arg0); end

  # Gets X509v3 extensions as array of X509Ext objects
  sig {returns(::T.untyped)}
  def extensions(); end

  # Sets X509\_EXTENSIONs
  sig do
    params(
      extensions: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def extensions=(extensions); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  sig {returns(::T.untyped)}
  def serial(); end

  sig do
    params(
      serial: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def serial=(serial); end

  sig {returns(::T.untyped)}
  def time(); end

  sig do
    params(
      time: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def time=(time); end
end

class OpenSSL::X509::RevokedError < OpenSSL::OpenSSLError
end

# The [`X509`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509.html)
# certificate store holds trusted CA certificates used to verify peer
# certificates.
#
# The easiest way to create a useful certificate store is:
#
# ```ruby
# cert_store = OpenSSL::X509::Store.new
# cert_store.set_default_paths
# ```
#
# This will use your system's built-in certificates.
#
# If your system does not have a default set of certificates you can obtain a
# set extracted from Mozilla CA certificate store by cURL maintainers here:
# https://curl.haxx.se/docs/caextract.html (You may wish to use the
# firefox-db2pem.sh script to extract the certificates from a local install to
# avoid man-in-the-middle attacks.)
#
# After downloading or generating a cacert.pem from the above link you can
# create a certificate store from the pem file like this:
#
# ```ruby
# cert_store = OpenSSL::X509::Store.new
# cert_store.add_file 'cacert.pem'
# ```
#
# The certificate store can be used with an SSLSocket like this:
#
# ```ruby
# ssl_context = OpenSSL::SSL::SSLContext.new
# ssl_context.verify_mode = OpenSSL::SSL::VERIFY_PEER
# ssl_context.cert_store = cert_store
#
# tcp_socket = TCPSocket.open 'example.com', 443
#
# ssl_socket = OpenSSL::SSL::SSLSocket.new tcp_socket, ssl_context
# ```
class OpenSSL::X509::Store
  # Adds the
  # [`OpenSSL::X509::Certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Certificate.html)
  # *cert* to the certificate store.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_cert(arg0); end

  # Adds the
  # [`OpenSSL::X509::CRL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/CRL.html)
  # *crl* to the store.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_crl(arg0); end

  # Adds the certificates in *file* to the certificate store. *file* is the path
  # to the file, and the file contains one or more certificates in PEM format
  # concatenated together.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_file(arg0); end

  # Adds *path* as the hash dir to be looked up by the store.
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_path(arg0); end

  # The certificate chain constructed by the last call of
  # [`verify`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Store.html#method-i-verify).
  sig {returns(::T.untyped)}
  def chain(); end

  # The error code set by the last call of
  # [`verify`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Store.html#method-i-verify).
  sig {returns(::T.untyped)}
  def error(); end

  # The description for the error code set by the last call of
  # [`verify`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Store.html#method-i-verify).
  sig {returns(::T.untyped)}
  def error_string(); end

  # Sets *flags* to the
  # [`Store`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Store.html).
  # *flags* consists of zero or more of the constants defined in with name
  # V\_FLAG\_\* or'ed together.
  sig do
    params(
      flags: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def flags=(flags); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # Sets the store's purpose to *purpose*. If specified, the verifications on
  # the store will check every untrusted certificate's extensions are consistent
  # with the purpose. The purpose is specified by constants:
  #
  # *   X509::PURPOSE\_SSL\_CLIENT
  # *   X509::PURPOSE\_SSL\_SERVER
  # *   X509::PURPOSE\_NS\_SSL\_SERVER
  # *   X509::PURPOSE\_SMIME\_SIGN
  # *   X509::PURPOSE\_SMIME\_ENCRYPT
  # *   X509::PURPOSE\_CRL\_SIGN
  # *   X509::PURPOSE\_ANY
  # *   X509::PURPOSE\_OCSP\_HELPER
  # *   X509::PURPOSE\_TIMESTAMP\_SIGN
  sig do
    params(
      purpose: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def purpose=(purpose); end

  # Configures *store* to look up CA certificates from the system default
  # certificate store as needed basis. The location of the store can usually be
  # determined by:
  #
  # *   OpenSSL::X509::DEFAULT\_CERT\_FILE
  # *   OpenSSL::X509::DEFAULT\_CERT\_DIR
  sig {returns(::T.untyped)}
  def set_default_paths(); end

  # Sets the time to be used in verifications.
  sig do
    params(
      time: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def time=(time); end

  sig do
    params(
      trust: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def trust=(trust); end

  # Performs a certificate verification on the
  # [`OpenSSL::X509::Certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Certificate.html)
  # *cert*.
  #
  # *chain* can be an array of
  # [`OpenSSL::X509::Certificate`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Certificate.html)
  # that is used to construct the certificate chain.
  #
  # If a block is given, it overrides the callback set by
  # [`verify_callback=`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Store.html#method-i-verify_callback-3D).
  #
  # After finishing the verification, the error information can be retrieved by
  # [`error`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Store.html#attribute-i-error),
  # [`error_string`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Store.html#attribute-i-error_string),
  # and the resulting complete certificate chain can be retrieved by
  # [`chain`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/Store.html#attribute-i-chain).
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def verify(*arg0); end

  # The callback for additional certificate verification. It is invoked for each
  # untrusted certificate in the chain.
  #
  # The callback is invoked with two values, a boolean that indicates if the
  # pre-verification by
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) has succeeded
  # or not, and the StoreContext in use. The callback must return either true or
  # false.
  sig {returns(::T.untyped)}
  def verify_callback(); end

  # General callback for
  # [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) verify
  sig do
    params(
      verify_callback: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def verify_callback=(verify_callback); end
end

# A
# [`StoreContext`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/StoreContext.html)
# is used while validating a single certificate and holds the status involved.
class OpenSSL::X509::StoreContext
  sig {returns(::T.untyped)}
  def chain(); end

  sig {returns(::T.untyped)}
  def cleanup(); end

  sig {returns(::T.untyped)}
  def current_cert(); end

  sig {returns(::T.untyped)}
  def current_crl(); end

  sig {returns(::T.untyped)}
  def error(); end

  sig do
    params(
      error: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def error=(error); end

  sig {returns(::T.untyped)}
  def error_depth(); end

  # Returns the error string corresponding to the error code retrieved by
  # [`error`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/X509/StoreContext.html#method-i-error).
  sig {returns(::T.untyped)}
  def error_string(); end

  # Sets the verification flags to the context. See Store#flags=.
  sig do
    params(
      flags: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def flags=(flags); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .void
  end
  def initialize(*arg0); end

  # Sets the purpose of the context. See Store#purpose=.
  sig do
    params(
      purpose: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def purpose=(purpose); end

  # Sets the time used in the verification. If not set, the current time is
  # used.
  sig do
    params(
      time: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def time=(time); end

  sig do
    params(
      trust: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def trust=(trust); end

  sig {returns(::T.untyped)}
  def verify(); end
end

class OpenSSL::X509::StoreError < OpenSSL::OpenSSLError
end
