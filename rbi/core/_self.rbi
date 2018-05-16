# typed: true
class ArgumentError < StandardError
  sig.returns(ArgumentError)
  def clone(); end

  sig.returns(ArgumentError)
  def dup(); end

  sig.returns(ArgumentError)
  def freeze(); end

  sig.returns(ArgumentError)
  def taint(); end

  sig.returns(ArgumentError)
  def trust(); end

  sig.returns(ArgumentError)
  def untaint(); end

  sig.returns(ArgumentError)
  def untrust(); end
end

class Array < Object
  sig.returns(T::Array[Elem])
  def clone(); end

  sig.returns(T::Array[Elem])
  def dup(); end

  sig.returns(T::Array[Elem])
  def freeze(); end

  sig.returns(T::Array[Elem])
  def taint(); end

  sig.returns(T::Array[Elem])
  def trust(); end

  sig.returns(T::Array[Elem])
  def untaint(); end

  sig.returns(T::Array[Elem])
  def untrust(); end
end

class Benchmark::Job < Object
  sig.returns(Benchmark::Job)
  def clone(); end

  sig.returns(Benchmark::Job)
  def dup(); end

  sig.returns(Benchmark::Job)
  def freeze(); end

  sig.returns(Benchmark::Job)
  def taint(); end

  sig.returns(Benchmark::Job)
  def trust(); end

  sig.returns(Benchmark::Job)
  def untaint(); end

  sig.returns(Benchmark::Job)
  def untrust(); end
end

class Benchmark::Report < Object
  sig.returns(Benchmark::Report)
  def clone(); end

  sig.returns(Benchmark::Report)
  def dup(); end

  sig.returns(Benchmark::Report)
  def freeze(); end

  sig.returns(Benchmark::Report)
  def taint(); end

  sig.returns(Benchmark::Report)
  def trust(); end

  sig.returns(Benchmark::Report)
  def untaint(); end

  sig.returns(Benchmark::Report)
  def untrust(); end
end

class Benchmark::Tms < Object
  sig.returns(Benchmark::Tms)
  def clone(); end

  sig.returns(Benchmark::Tms)
  def dup(); end

  sig.returns(Benchmark::Tms)
  def freeze(); end

  sig.returns(Benchmark::Tms)
  def taint(); end

  sig.returns(Benchmark::Tms)
  def trust(); end

  sig.returns(Benchmark::Tms)
  def untaint(); end

  sig.returns(Benchmark::Tms)
  def untrust(); end
end

class BigDecimal < Numeric
  sig.returns(BigDecimal)
  def clone(); end

  sig.returns(BigDecimal)
  def dup(); end

  sig.returns(BigDecimal)
  def freeze(); end

  sig.returns(BigDecimal)
  def taint(); end

  sig.returns(BigDecimal)
  def trust(); end

  sig.returns(BigDecimal)
  def untaint(); end

  sig.returns(BigDecimal)
  def untrust(); end
end

class Binding < Object
  sig.returns(Binding)
  def clone(); end

  sig.returns(Binding)
  def dup(); end

  sig.returns(Binding)
  def freeze(); end

  sig.returns(Binding)
  def taint(); end

  sig.returns(Binding)
  def trust(); end

  sig.returns(Binding)
  def untaint(); end

  sig.returns(Binding)
  def untrust(); end
end

class CSV < Object
  sig.returns(CSV)
  def clone(); end

  sig.returns(CSV)
  def dup(); end

  sig.returns(CSV)
  def freeze(); end

  sig.returns(CSV)
  def taint(); end

  sig.returns(CSV)
  def trust(); end

  sig.returns(CSV)
  def untaint(); end

  sig.returns(CSV)
  def untrust(); end
end

class CSV::FieldInfo < Struct
  sig.returns(CSV::FieldInfo)
  def clone(); end

  sig.returns(CSV::FieldInfo)
  def dup(); end

  sig.returns(CSV::FieldInfo)
  def freeze(); end

  sig.returns(CSV::FieldInfo)
  def taint(); end

  sig.returns(CSV::FieldInfo)
  def trust(); end

  sig.returns(CSV::FieldInfo)
  def untaint(); end

  sig.returns(CSV::FieldInfo)
  def untrust(); end
end

class CSV::MalformedCSVError < RuntimeError
  sig.returns(CSV::MalformedCSVError)
  def clone(); end

  sig.returns(CSV::MalformedCSVError)
  def dup(); end

  sig.returns(CSV::MalformedCSVError)
  def freeze(); end

  sig.returns(CSV::MalformedCSVError)
  def taint(); end

  sig.returns(CSV::MalformedCSVError)
  def trust(); end

  sig.returns(CSV::MalformedCSVError)
  def untaint(); end

  sig.returns(CSV::MalformedCSVError)
  def untrust(); end
end

class CSV::Row < Object
  sig.returns(CSV::Row[Elem])
  def clone(); end

  sig.returns(CSV::Row[Elem])
  def dup(); end

  sig.returns(CSV::Row[Elem])
  def freeze(); end

  sig.returns(CSV::Row[Elem])
  def taint(); end

  sig.returns(CSV::Row[Elem])
  def trust(); end

  sig.returns(CSV::Row[Elem])
  def untaint(); end

  sig.returns(CSV::Row[Elem])
  def untrust(); end
end

class CSV::Table < Object
  sig.returns(CSV::Table[Elem])
  def clone(); end

  sig.returns(CSV::Table[Elem])
  def dup(); end

  sig.returns(CSV::Table[Elem])
  def freeze(); end

  sig.returns(CSV::Table[Elem])
  def taint(); end

  sig.returns(CSV::Table[Elem])
  def trust(); end

  sig.returns(CSV::Table[Elem])
  def untaint(); end

  sig.returns(CSV::Table[Elem])
  def untrust(); end
end

class Class < Module
  sig.returns(Class)
  def clone(); end

  sig.returns(Class)
  def dup(); end

  sig.returns(Class)
  def freeze(); end

  sig(
      arg0: Module,
  )
  .returns(Class)
  def include(*arg0); end

  sig(
      arg0: Module,
  )
  .returns(Class)
  def prepend(*arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(Class)
  def private_class_method(*arg0); end

  sig(
      arg0: Symbol,
  )
  .returns(Class)
  def private_constant(*arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(Class)
  def public_class_method(*arg0); end

  sig(
      arg0: Symbol,
  )
  .returns(Class)
  def public_constant(*arg0); end

  sig.returns(Class)
  def taint(); end

  sig.returns(Class)
  def trust(); end

  sig.returns(Class)
  def untaint(); end

  sig.returns(Class)
  def untrust(); end
end

class ClosedQueueError < StopIteration
  sig.returns(ClosedQueueError)
  def clone(); end

  sig.returns(ClosedQueueError)
  def dup(); end

  sig.returns(ClosedQueueError)
  def freeze(); end

  sig.returns(ClosedQueueError)
  def taint(); end

  sig.returns(ClosedQueueError)
  def trust(); end

  sig.returns(ClosedQueueError)
  def untaint(); end

  sig.returns(ClosedQueueError)
  def untrust(); end
end

class Complex < Numeric
  sig.returns(Complex)
  def clone(); end

  sig.returns(Complex)
  def dup(); end

  sig.returns(Complex)
  def freeze(); end

  sig.returns(Complex)
  def taint(); end

  sig.returns(Complex)
  def trust(); end

  sig.returns(Complex)
  def untaint(); end

  sig.returns(Complex)
  def untrust(); end
end

class Data < Object
  sig.returns(Data)
  def clone(); end

  sig.returns(Data)
  def dup(); end

  sig.returns(Data)
  def freeze(); end

  sig.returns(Data)
  def taint(); end

  sig.returns(Data)
  def trust(); end

  sig.returns(Data)
  def untaint(); end

  sig.returns(Data)
  def untrust(); end
end

class Date < Object
  sig.returns(Date)
  def clone(); end

  sig.returns(Date)
  def dup(); end

  sig.returns(Date)
  def freeze(); end

  sig.returns(Date)
  def taint(); end

  sig.returns(Date)
  def trust(); end

  sig.returns(Date)
  def untaint(); end

  sig.returns(Date)
  def untrust(); end
end

class Date::Infinity < Numeric
  sig.returns(Date::Infinity)
  def clone(); end

  sig.returns(Date::Infinity)
  def dup(); end

  sig.returns(Date::Infinity)
  def freeze(); end

  sig.returns(Date::Infinity)
  def taint(); end

  sig.returns(Date::Infinity)
  def trust(); end

  sig.returns(Date::Infinity)
  def untaint(); end

  sig.returns(Date::Infinity)
  def untrust(); end
end

class DateTime < Date
  sig.returns(DateTime)
  def clone(); end

  sig.returns(DateTime)
  def dup(); end

  sig.returns(DateTime)
  def freeze(); end

  sig.returns(DateTime)
  def taint(); end

  sig.returns(DateTime)
  def trust(); end

  sig.returns(DateTime)
  def untaint(); end

  sig.returns(DateTime)
  def untrust(); end
end

class DidYouMean::ClassNameChecker < Object
  sig.returns(DidYouMean::ClassNameChecker)
  def clone(); end

  sig.returns(DidYouMean::ClassNameChecker)
  def dup(); end

  sig.returns(DidYouMean::ClassNameChecker)
  def freeze(); end

  sig.returns(DidYouMean::ClassNameChecker)
  def taint(); end

  sig.returns(DidYouMean::ClassNameChecker)
  def trust(); end

  sig.returns(DidYouMean::ClassNameChecker)
  def untaint(); end

  sig.returns(DidYouMean::ClassNameChecker)
  def untrust(); end
end

class DidYouMean::Formatter < Object
  sig.returns(DidYouMean::Formatter)
  def clone(); end

  sig.returns(DidYouMean::Formatter)
  def dup(); end

  sig.returns(DidYouMean::Formatter)
  def freeze(); end

  sig.returns(DidYouMean::Formatter)
  def taint(); end

  sig.returns(DidYouMean::Formatter)
  def trust(); end

  sig.returns(DidYouMean::Formatter)
  def untaint(); end

  sig.returns(DidYouMean::Formatter)
  def untrust(); end
end

class DidYouMean::MethodNameChecker < Object
  sig.returns(DidYouMean::MethodNameChecker)
  def clone(); end

  sig.returns(DidYouMean::MethodNameChecker)
  def dup(); end

  sig.returns(DidYouMean::MethodNameChecker)
  def freeze(); end

  sig.returns(DidYouMean::MethodNameChecker)
  def taint(); end

  sig.returns(DidYouMean::MethodNameChecker)
  def trust(); end

  sig.returns(DidYouMean::MethodNameChecker)
  def untaint(); end

  sig.returns(DidYouMean::MethodNameChecker)
  def untrust(); end
end

class DidYouMean::NullChecker < Object
  sig.returns(DidYouMean::NullChecker)
  def clone(); end

  sig.returns(DidYouMean::NullChecker)
  def dup(); end

  sig.returns(DidYouMean::NullChecker)
  def freeze(); end

  sig.returns(DidYouMean::NullChecker)
  def taint(); end

  sig.returns(DidYouMean::NullChecker)
  def trust(); end

  sig.returns(DidYouMean::NullChecker)
  def untaint(); end

  sig.returns(DidYouMean::NullChecker)
  def untrust(); end
end

class DidYouMean::SpellChecker < Object
  sig.returns(DidYouMean::SpellChecker)
  def clone(); end

  sig.returns(DidYouMean::SpellChecker)
  def dup(); end

  sig.returns(DidYouMean::SpellChecker)
  def freeze(); end

  sig.returns(DidYouMean::SpellChecker)
  def taint(); end

  sig.returns(DidYouMean::SpellChecker)
  def trust(); end

  sig.returns(DidYouMean::SpellChecker)
  def untaint(); end

  sig.returns(DidYouMean::SpellChecker)
  def untrust(); end
end

class DidYouMean::VariableNameChecker < Object
  sig.returns(DidYouMean::VariableNameChecker)
  def clone(); end

  sig.returns(DidYouMean::VariableNameChecker)
  def dup(); end

  sig.returns(DidYouMean::VariableNameChecker)
  def freeze(); end

  sig.returns(DidYouMean::VariableNameChecker)
  def taint(); end

  sig.returns(DidYouMean::VariableNameChecker)
  def trust(); end

  sig.returns(DidYouMean::VariableNameChecker)
  def untaint(); end

  sig.returns(DidYouMean::VariableNameChecker)
  def untrust(); end
end

class Dir < Object
  sig.returns(Dir)
  def clone(); end

  sig.returns(Dir)
  def dup(); end

  sig.returns(Dir)
  def freeze(); end

  sig.returns(Dir)
  def taint(); end

  sig.returns(Dir)
  def trust(); end

  sig.returns(Dir)
  def untaint(); end

  sig.returns(Dir)
  def untrust(); end
end

class EOFError < IOError
  sig.returns(EOFError)
  def clone(); end

  sig.returns(EOFError)
  def dup(); end

  sig.returns(EOFError)
  def freeze(); end

  sig.returns(EOFError)
  def taint(); end

  sig.returns(EOFError)
  def trust(); end

  sig.returns(EOFError)
  def untaint(); end

  sig.returns(EOFError)
  def untrust(); end
end

class Encoding < Object
  sig.returns(Encoding)
  def clone(); end

  sig.returns(Encoding)
  def dup(); end

  sig.returns(Encoding)
  def freeze(); end

  sig.returns(Encoding)
  def taint(); end

  sig.returns(Encoding)
  def trust(); end

  sig.returns(Encoding)
  def untaint(); end

  sig.returns(Encoding)
  def untrust(); end
end

class Encoding::CompatibilityError < EncodingError
  sig.returns(Encoding::CompatibilityError)
  def clone(); end

  sig.returns(Encoding::CompatibilityError)
  def dup(); end

  sig.returns(Encoding::CompatibilityError)
  def freeze(); end

  sig.returns(Encoding::CompatibilityError)
  def taint(); end

  sig.returns(Encoding::CompatibilityError)
  def trust(); end

  sig.returns(Encoding::CompatibilityError)
  def untaint(); end

  sig.returns(Encoding::CompatibilityError)
  def untrust(); end
end

class Encoding::Converter < Data
  sig.returns(Encoding::Converter)
  def clone(); end

  sig.returns(Encoding::Converter)
  def dup(); end

  sig.returns(Encoding::Converter)
  def freeze(); end

  sig.returns(Encoding::Converter)
  def taint(); end

  sig.returns(Encoding::Converter)
  def trust(); end

  sig.returns(Encoding::Converter)
  def untaint(); end

  sig.returns(Encoding::Converter)
  def untrust(); end
end

class Encoding::ConverterNotFoundError < EncodingError
  sig.returns(Encoding::ConverterNotFoundError)
  def clone(); end

  sig.returns(Encoding::ConverterNotFoundError)
  def dup(); end

  sig.returns(Encoding::ConverterNotFoundError)
  def freeze(); end

  sig.returns(Encoding::ConverterNotFoundError)
  def taint(); end

  sig.returns(Encoding::ConverterNotFoundError)
  def trust(); end

  sig.returns(Encoding::ConverterNotFoundError)
  def untaint(); end

  sig.returns(Encoding::ConverterNotFoundError)
  def untrust(); end
end

class Encoding::InvalidByteSequenceError < EncodingError
  sig.returns(Encoding::InvalidByteSequenceError)
  def clone(); end

  sig.returns(Encoding::InvalidByteSequenceError)
  def dup(); end

  sig.returns(Encoding::InvalidByteSequenceError)
  def freeze(); end

  sig.returns(Encoding::InvalidByteSequenceError)
  def taint(); end

  sig.returns(Encoding::InvalidByteSequenceError)
  def trust(); end

  sig.returns(Encoding::InvalidByteSequenceError)
  def untaint(); end

  sig.returns(Encoding::InvalidByteSequenceError)
  def untrust(); end
end

class Encoding::UndefinedConversionError < EncodingError
  sig.returns(Encoding::UndefinedConversionError)
  def clone(); end

  sig.returns(Encoding::UndefinedConversionError)
  def dup(); end

  sig.returns(Encoding::UndefinedConversionError)
  def freeze(); end

  sig.returns(Encoding::UndefinedConversionError)
  def taint(); end

  sig.returns(Encoding::UndefinedConversionError)
  def trust(); end

  sig.returns(Encoding::UndefinedConversionError)
  def untaint(); end

  sig.returns(Encoding::UndefinedConversionError)
  def untrust(); end
end

class EncodingError < StandardError
  sig.returns(EncodingError)
  def clone(); end

  sig.returns(EncodingError)
  def dup(); end

  sig.returns(EncodingError)
  def freeze(); end

  sig.returns(EncodingError)
  def taint(); end

  sig.returns(EncodingError)
  def trust(); end

  sig.returns(EncodingError)
  def untaint(); end

  sig.returns(EncodingError)
  def untrust(); end
end

class Enumerator < Object
  sig.returns(Enumerator[Elem])
  def clone(); end

  sig.returns(Enumerator[Elem])
  def dup(); end

  sig.returns(Enumerator[Elem])
  def freeze(); end

  sig.returns(Enumerator[Elem])
  def taint(); end

  sig.returns(Enumerator[Elem])
  def trust(); end

  sig.returns(Enumerator[Elem])
  def untaint(); end

  sig.returns(Enumerator[Elem])
  def untrust(); end
end

class Enumerator::Generator < Object
  sig.returns(Enumerator::Generator[Elem])
  def clone(); end

  sig.returns(Enumerator::Generator[Elem])
  def dup(); end

  sig.returns(Enumerator::Generator[Elem])
  def freeze(); end

  sig.returns(Enumerator::Generator[Elem])
  def taint(); end

  sig.returns(Enumerator::Generator[Elem])
  def trust(); end

  sig.returns(Enumerator::Generator[Elem])
  def untaint(); end

  sig.returns(Enumerator::Generator[Elem])
  def untrust(); end
end

class Enumerator::Lazy < Enumerator
  sig.returns(Enumerator::Lazy[Elem])
  def clone(); end

  sig.returns(Enumerator::Lazy[Elem])
  def dup(); end

  sig.returns(Enumerator::Lazy[Elem])
  def each(); end

  sig.returns(Enumerator::Lazy[Elem])
  def freeze(); end

  sig.returns(Enumerator::Lazy[Elem])
  def rewind(); end

  sig.returns(Enumerator::Lazy[Elem])
  def taint(); end

  sig.returns(Enumerator::Lazy[Elem])
  def trust(); end

  sig.returns(Enumerator::Lazy[Elem])
  def untaint(); end

  sig.returns(Enumerator::Lazy[Elem])
  def untrust(); end
end

class Enumerator::Yielder < Object
  sig.returns(Enumerator::Yielder)
  def clone(); end

  sig.returns(Enumerator::Yielder)
  def dup(); end

  sig.returns(Enumerator::Yielder)
  def freeze(); end

  sig.returns(Enumerator::Yielder)
  def taint(); end

  sig.returns(Enumerator::Yielder)
  def trust(); end

  sig.returns(Enumerator::Yielder)
  def untaint(); end

  sig.returns(Enumerator::Yielder)
  def untrust(); end
end

class Errno::E2BIG < SystemCallError
  sig.returns(Errno::E2BIG)
  def clone(); end

  sig.returns(Errno::E2BIG)
  def dup(); end

  sig.returns(Errno::E2BIG)
  def freeze(); end

  sig.returns(Errno::E2BIG)
  def taint(); end

  sig.returns(Errno::E2BIG)
  def trust(); end

  sig.returns(Errno::E2BIG)
  def untaint(); end

  sig.returns(Errno::E2BIG)
  def untrust(); end
end

class Errno::EACCES < SystemCallError
  sig.returns(Errno::EACCES)
  def clone(); end

  sig.returns(Errno::EACCES)
  def dup(); end

  sig.returns(Errno::EACCES)
  def freeze(); end

  sig.returns(Errno::EACCES)
  def taint(); end

  sig.returns(Errno::EACCES)
  def trust(); end

  sig.returns(Errno::EACCES)
  def untaint(); end

  sig.returns(Errno::EACCES)
  def untrust(); end
end

class Errno::EADDRINUSE < SystemCallError
  sig.returns(Errno::EADDRINUSE)
  def clone(); end

  sig.returns(Errno::EADDRINUSE)
  def dup(); end

  sig.returns(Errno::EADDRINUSE)
  def freeze(); end

  sig.returns(Errno::EADDRINUSE)
  def taint(); end

  sig.returns(Errno::EADDRINUSE)
  def trust(); end

  sig.returns(Errno::EADDRINUSE)
  def untaint(); end

  sig.returns(Errno::EADDRINUSE)
  def untrust(); end
end

class Errno::EADDRNOTAVAIL < SystemCallError
  sig.returns(Errno::EADDRNOTAVAIL)
  def clone(); end

  sig.returns(Errno::EADDRNOTAVAIL)
  def dup(); end

  sig.returns(Errno::EADDRNOTAVAIL)
  def freeze(); end

  sig.returns(Errno::EADDRNOTAVAIL)
  def taint(); end

  sig.returns(Errno::EADDRNOTAVAIL)
  def trust(); end

  sig.returns(Errno::EADDRNOTAVAIL)
  def untaint(); end

  sig.returns(Errno::EADDRNOTAVAIL)
  def untrust(); end
end

class Errno::EADV < SystemCallError
  sig.returns(Errno::EADV)
  def clone(); end

  sig.returns(Errno::EADV)
  def dup(); end

  sig.returns(Errno::EADV)
  def freeze(); end

  sig.returns(Errno::EADV)
  def taint(); end

  sig.returns(Errno::EADV)
  def trust(); end

  sig.returns(Errno::EADV)
  def untaint(); end

  sig.returns(Errno::EADV)
  def untrust(); end
end

class Errno::EAFNOSUPPORT < SystemCallError
  sig.returns(Errno::EAFNOSUPPORT)
  def clone(); end

  sig.returns(Errno::EAFNOSUPPORT)
  def dup(); end

  sig.returns(Errno::EAFNOSUPPORT)
  def freeze(); end

  sig.returns(Errno::EAFNOSUPPORT)
  def taint(); end

  sig.returns(Errno::EAFNOSUPPORT)
  def trust(); end

  sig.returns(Errno::EAFNOSUPPORT)
  def untaint(); end

  sig.returns(Errno::EAFNOSUPPORT)
  def untrust(); end
end

class Errno::EAGAIN < SystemCallError
  sig.returns(Errno::EAGAIN)
  def clone(); end

  sig.returns(Errno::EAGAIN)
  def dup(); end

  sig.returns(Errno::EAGAIN)
  def freeze(); end

  sig.returns(Errno::EAGAIN)
  def taint(); end

  sig.returns(Errno::EAGAIN)
  def trust(); end

  sig.returns(Errno::EAGAIN)
  def untaint(); end

  sig.returns(Errno::EAGAIN)
  def untrust(); end
end

class Errno::EALREADY < SystemCallError
  sig.returns(Errno::EALREADY)
  def clone(); end

  sig.returns(Errno::EALREADY)
  def dup(); end

  sig.returns(Errno::EALREADY)
  def freeze(); end

  sig.returns(Errno::EALREADY)
  def taint(); end

  sig.returns(Errno::EALREADY)
  def trust(); end

  sig.returns(Errno::EALREADY)
  def untaint(); end

  sig.returns(Errno::EALREADY)
  def untrust(); end
end

class Errno::EBADE < SystemCallError
  sig.returns(Errno::EBADE)
  def clone(); end

  sig.returns(Errno::EBADE)
  def dup(); end

  sig.returns(Errno::EBADE)
  def freeze(); end

  sig.returns(Errno::EBADE)
  def taint(); end

  sig.returns(Errno::EBADE)
  def trust(); end

  sig.returns(Errno::EBADE)
  def untaint(); end

  sig.returns(Errno::EBADE)
  def untrust(); end
end

class Errno::EBADF < SystemCallError
  sig.returns(Errno::EBADF)
  def clone(); end

  sig.returns(Errno::EBADF)
  def dup(); end

  sig.returns(Errno::EBADF)
  def freeze(); end

  sig.returns(Errno::EBADF)
  def taint(); end

  sig.returns(Errno::EBADF)
  def trust(); end

  sig.returns(Errno::EBADF)
  def untaint(); end

  sig.returns(Errno::EBADF)
  def untrust(); end
end

class Errno::EBADFD < SystemCallError
  sig.returns(Errno::EBADFD)
  def clone(); end

  sig.returns(Errno::EBADFD)
  def dup(); end

  sig.returns(Errno::EBADFD)
  def freeze(); end

  sig.returns(Errno::EBADFD)
  def taint(); end

  sig.returns(Errno::EBADFD)
  def trust(); end

  sig.returns(Errno::EBADFD)
  def untaint(); end

  sig.returns(Errno::EBADFD)
  def untrust(); end
end

class Errno::EBADMSG < SystemCallError
  sig.returns(Errno::EBADMSG)
  def clone(); end

  sig.returns(Errno::EBADMSG)
  def dup(); end

  sig.returns(Errno::EBADMSG)
  def freeze(); end

  sig.returns(Errno::EBADMSG)
  def taint(); end

  sig.returns(Errno::EBADMSG)
  def trust(); end

  sig.returns(Errno::EBADMSG)
  def untaint(); end

  sig.returns(Errno::EBADMSG)
  def untrust(); end
end

class Errno::EBADR < SystemCallError
  sig.returns(Errno::EBADR)
  def clone(); end

  sig.returns(Errno::EBADR)
  def dup(); end

  sig.returns(Errno::EBADR)
  def freeze(); end

  sig.returns(Errno::EBADR)
  def taint(); end

  sig.returns(Errno::EBADR)
  def trust(); end

  sig.returns(Errno::EBADR)
  def untaint(); end

  sig.returns(Errno::EBADR)
  def untrust(); end
end

class Errno::EBADRQC < SystemCallError
  sig.returns(Errno::EBADRQC)
  def clone(); end

  sig.returns(Errno::EBADRQC)
  def dup(); end

  sig.returns(Errno::EBADRQC)
  def freeze(); end

  sig.returns(Errno::EBADRQC)
  def taint(); end

  sig.returns(Errno::EBADRQC)
  def trust(); end

  sig.returns(Errno::EBADRQC)
  def untaint(); end

  sig.returns(Errno::EBADRQC)
  def untrust(); end
end

class Errno::EBADSLT < SystemCallError
  sig.returns(Errno::EBADSLT)
  def clone(); end

  sig.returns(Errno::EBADSLT)
  def dup(); end

  sig.returns(Errno::EBADSLT)
  def freeze(); end

  sig.returns(Errno::EBADSLT)
  def taint(); end

  sig.returns(Errno::EBADSLT)
  def trust(); end

  sig.returns(Errno::EBADSLT)
  def untaint(); end

  sig.returns(Errno::EBADSLT)
  def untrust(); end
end

class Errno::EBFONT < SystemCallError
  sig.returns(Errno::EBFONT)
  def clone(); end

  sig.returns(Errno::EBFONT)
  def dup(); end

  sig.returns(Errno::EBFONT)
  def freeze(); end

  sig.returns(Errno::EBFONT)
  def taint(); end

  sig.returns(Errno::EBFONT)
  def trust(); end

  sig.returns(Errno::EBFONT)
  def untaint(); end

  sig.returns(Errno::EBFONT)
  def untrust(); end
end

class Errno::EBUSY < SystemCallError
  sig.returns(Errno::EBUSY)
  def clone(); end

  sig.returns(Errno::EBUSY)
  def dup(); end

  sig.returns(Errno::EBUSY)
  def freeze(); end

  sig.returns(Errno::EBUSY)
  def taint(); end

  sig.returns(Errno::EBUSY)
  def trust(); end

  sig.returns(Errno::EBUSY)
  def untaint(); end

  sig.returns(Errno::EBUSY)
  def untrust(); end
end

class Errno::ECANCELED < SystemCallError
  sig.returns(Errno::ECANCELED)
  def clone(); end

  sig.returns(Errno::ECANCELED)
  def dup(); end

  sig.returns(Errno::ECANCELED)
  def freeze(); end

  sig.returns(Errno::ECANCELED)
  def taint(); end

  sig.returns(Errno::ECANCELED)
  def trust(); end

  sig.returns(Errno::ECANCELED)
  def untaint(); end

  sig.returns(Errno::ECANCELED)
  def untrust(); end
end

class Errno::ECHILD < SystemCallError
  sig.returns(Errno::ECHILD)
  def clone(); end

  sig.returns(Errno::ECHILD)
  def dup(); end

  sig.returns(Errno::ECHILD)
  def freeze(); end

  sig.returns(Errno::ECHILD)
  def taint(); end

  sig.returns(Errno::ECHILD)
  def trust(); end

  sig.returns(Errno::ECHILD)
  def untaint(); end

  sig.returns(Errno::ECHILD)
  def untrust(); end
end

class Errno::ECHRNG < SystemCallError
  sig.returns(Errno::ECHRNG)
  def clone(); end

  sig.returns(Errno::ECHRNG)
  def dup(); end

  sig.returns(Errno::ECHRNG)
  def freeze(); end

  sig.returns(Errno::ECHRNG)
  def taint(); end

  sig.returns(Errno::ECHRNG)
  def trust(); end

  sig.returns(Errno::ECHRNG)
  def untaint(); end

  sig.returns(Errno::ECHRNG)
  def untrust(); end
end

class Errno::ECOMM < SystemCallError
  sig.returns(Errno::ECOMM)
  def clone(); end

  sig.returns(Errno::ECOMM)
  def dup(); end

  sig.returns(Errno::ECOMM)
  def freeze(); end

  sig.returns(Errno::ECOMM)
  def taint(); end

  sig.returns(Errno::ECOMM)
  def trust(); end

  sig.returns(Errno::ECOMM)
  def untaint(); end

  sig.returns(Errno::ECOMM)
  def untrust(); end
end

class Errno::ECONNABORTED < SystemCallError
  sig.returns(Errno::ECONNABORTED)
  def clone(); end

  sig.returns(Errno::ECONNABORTED)
  def dup(); end

  sig.returns(Errno::ECONNABORTED)
  def freeze(); end

  sig.returns(Errno::ECONNABORTED)
  def taint(); end

  sig.returns(Errno::ECONNABORTED)
  def trust(); end

  sig.returns(Errno::ECONNABORTED)
  def untaint(); end

  sig.returns(Errno::ECONNABORTED)
  def untrust(); end
end

class Errno::ECONNREFUSED < SystemCallError
  sig.returns(Errno::ECONNREFUSED)
  def clone(); end

  sig.returns(Errno::ECONNREFUSED)
  def dup(); end

  sig.returns(Errno::ECONNREFUSED)
  def freeze(); end

  sig.returns(Errno::ECONNREFUSED)
  def taint(); end

  sig.returns(Errno::ECONNREFUSED)
  def trust(); end

  sig.returns(Errno::ECONNREFUSED)
  def untaint(); end

  sig.returns(Errno::ECONNREFUSED)
  def untrust(); end
end

class Errno::ECONNRESET < SystemCallError
  sig.returns(Errno::ECONNRESET)
  def clone(); end

  sig.returns(Errno::ECONNRESET)
  def dup(); end

  sig.returns(Errno::ECONNRESET)
  def freeze(); end

  sig.returns(Errno::ECONNRESET)
  def taint(); end

  sig.returns(Errno::ECONNRESET)
  def trust(); end

  sig.returns(Errno::ECONNRESET)
  def untaint(); end

  sig.returns(Errno::ECONNRESET)
  def untrust(); end
end

class Errno::EDEADLK < SystemCallError
  sig.returns(Errno::EDEADLK)
  def clone(); end

  sig.returns(Errno::EDEADLK)
  def dup(); end

  sig.returns(Errno::EDEADLK)
  def freeze(); end

  sig.returns(Errno::EDEADLK)
  def taint(); end

  sig.returns(Errno::EDEADLK)
  def trust(); end

  sig.returns(Errno::EDEADLK)
  def untaint(); end

  sig.returns(Errno::EDEADLK)
  def untrust(); end
end

class Errno::EDESTADDRREQ < SystemCallError
  sig.returns(Errno::EDESTADDRREQ)
  def clone(); end

  sig.returns(Errno::EDESTADDRREQ)
  def dup(); end

  sig.returns(Errno::EDESTADDRREQ)
  def freeze(); end

  sig.returns(Errno::EDESTADDRREQ)
  def taint(); end

  sig.returns(Errno::EDESTADDRREQ)
  def trust(); end

  sig.returns(Errno::EDESTADDRREQ)
  def untaint(); end

  sig.returns(Errno::EDESTADDRREQ)
  def untrust(); end
end

class Errno::EDOM < SystemCallError
  sig.returns(Errno::EDOM)
  def clone(); end

  sig.returns(Errno::EDOM)
  def dup(); end

  sig.returns(Errno::EDOM)
  def freeze(); end

  sig.returns(Errno::EDOM)
  def taint(); end

  sig.returns(Errno::EDOM)
  def trust(); end

  sig.returns(Errno::EDOM)
  def untaint(); end

  sig.returns(Errno::EDOM)
  def untrust(); end
end

class Errno::EDOTDOT < SystemCallError
  sig.returns(Errno::EDOTDOT)
  def clone(); end

  sig.returns(Errno::EDOTDOT)
  def dup(); end

  sig.returns(Errno::EDOTDOT)
  def freeze(); end

  sig.returns(Errno::EDOTDOT)
  def taint(); end

  sig.returns(Errno::EDOTDOT)
  def trust(); end

  sig.returns(Errno::EDOTDOT)
  def untaint(); end

  sig.returns(Errno::EDOTDOT)
  def untrust(); end
end

class Errno::EDQUOT < SystemCallError
  sig.returns(Errno::EDQUOT)
  def clone(); end

  sig.returns(Errno::EDQUOT)
  def dup(); end

  sig.returns(Errno::EDQUOT)
  def freeze(); end

  sig.returns(Errno::EDQUOT)
  def taint(); end

  sig.returns(Errno::EDQUOT)
  def trust(); end

  sig.returns(Errno::EDQUOT)
  def untaint(); end

  sig.returns(Errno::EDQUOT)
  def untrust(); end
end

class Errno::EEXIST < SystemCallError
  sig.returns(Errno::EEXIST)
  def clone(); end

  sig.returns(Errno::EEXIST)
  def dup(); end

  sig.returns(Errno::EEXIST)
  def freeze(); end

  sig.returns(Errno::EEXIST)
  def taint(); end

  sig.returns(Errno::EEXIST)
  def trust(); end

  sig.returns(Errno::EEXIST)
  def untaint(); end

  sig.returns(Errno::EEXIST)
  def untrust(); end
end

class Errno::EFAULT < SystemCallError
  sig.returns(Errno::EFAULT)
  def clone(); end

  sig.returns(Errno::EFAULT)
  def dup(); end

  sig.returns(Errno::EFAULT)
  def freeze(); end

  sig.returns(Errno::EFAULT)
  def taint(); end

  sig.returns(Errno::EFAULT)
  def trust(); end

  sig.returns(Errno::EFAULT)
  def untaint(); end

  sig.returns(Errno::EFAULT)
  def untrust(); end
end

class Errno::EFBIG < SystemCallError
  sig.returns(Errno::EFBIG)
  def clone(); end

  sig.returns(Errno::EFBIG)
  def dup(); end

  sig.returns(Errno::EFBIG)
  def freeze(); end

  sig.returns(Errno::EFBIG)
  def taint(); end

  sig.returns(Errno::EFBIG)
  def trust(); end

  sig.returns(Errno::EFBIG)
  def untaint(); end

  sig.returns(Errno::EFBIG)
  def untrust(); end
end

class Errno::EHOSTDOWN < SystemCallError
  sig.returns(Errno::EHOSTDOWN)
  def clone(); end

  sig.returns(Errno::EHOSTDOWN)
  def dup(); end

  sig.returns(Errno::EHOSTDOWN)
  def freeze(); end

  sig.returns(Errno::EHOSTDOWN)
  def taint(); end

  sig.returns(Errno::EHOSTDOWN)
  def trust(); end

  sig.returns(Errno::EHOSTDOWN)
  def untaint(); end

  sig.returns(Errno::EHOSTDOWN)
  def untrust(); end
end

class Errno::EHOSTUNREACH < SystemCallError
  sig.returns(Errno::EHOSTUNREACH)
  def clone(); end

  sig.returns(Errno::EHOSTUNREACH)
  def dup(); end

  sig.returns(Errno::EHOSTUNREACH)
  def freeze(); end

  sig.returns(Errno::EHOSTUNREACH)
  def taint(); end

  sig.returns(Errno::EHOSTUNREACH)
  def trust(); end

  sig.returns(Errno::EHOSTUNREACH)
  def untaint(); end

  sig.returns(Errno::EHOSTUNREACH)
  def untrust(); end
end

class Errno::EHWPOISON < SystemCallError
  sig.returns(Errno::EHWPOISON)
  def clone(); end

  sig.returns(Errno::EHWPOISON)
  def dup(); end

  sig.returns(Errno::EHWPOISON)
  def freeze(); end

  sig.returns(Errno::EHWPOISON)
  def taint(); end

  sig.returns(Errno::EHWPOISON)
  def trust(); end

  sig.returns(Errno::EHWPOISON)
  def untaint(); end

  sig.returns(Errno::EHWPOISON)
  def untrust(); end
end

class Errno::EIDRM < SystemCallError
  sig.returns(Errno::EIDRM)
  def clone(); end

  sig.returns(Errno::EIDRM)
  def dup(); end

  sig.returns(Errno::EIDRM)
  def freeze(); end

  sig.returns(Errno::EIDRM)
  def taint(); end

  sig.returns(Errno::EIDRM)
  def trust(); end

  sig.returns(Errno::EIDRM)
  def untaint(); end

  sig.returns(Errno::EIDRM)
  def untrust(); end
end

class Errno::EILSEQ < SystemCallError
  sig.returns(Errno::EILSEQ)
  def clone(); end

  sig.returns(Errno::EILSEQ)
  def dup(); end

  sig.returns(Errno::EILSEQ)
  def freeze(); end

  sig.returns(Errno::EILSEQ)
  def taint(); end

  sig.returns(Errno::EILSEQ)
  def trust(); end

  sig.returns(Errno::EILSEQ)
  def untaint(); end

  sig.returns(Errno::EILSEQ)
  def untrust(); end
end

class Errno::EINPROGRESS < SystemCallError
  sig.returns(Errno::EINPROGRESS)
  def clone(); end

  sig.returns(Errno::EINPROGRESS)
  def dup(); end

  sig.returns(Errno::EINPROGRESS)
  def freeze(); end

  sig.returns(Errno::EINPROGRESS)
  def taint(); end

  sig.returns(Errno::EINPROGRESS)
  def trust(); end

  sig.returns(Errno::EINPROGRESS)
  def untaint(); end

  sig.returns(Errno::EINPROGRESS)
  def untrust(); end
end

class Errno::EINTR < SystemCallError
  sig.returns(Errno::EINTR)
  def clone(); end

  sig.returns(Errno::EINTR)
  def dup(); end

  sig.returns(Errno::EINTR)
  def freeze(); end

  sig.returns(Errno::EINTR)
  def taint(); end

  sig.returns(Errno::EINTR)
  def trust(); end

  sig.returns(Errno::EINTR)
  def untaint(); end

  sig.returns(Errno::EINTR)
  def untrust(); end
end

class Errno::EINVAL < SystemCallError
  sig.returns(Errno::EINVAL)
  def clone(); end

  sig.returns(Errno::EINVAL)
  def dup(); end

  sig.returns(Errno::EINVAL)
  def freeze(); end

  sig.returns(Errno::EINVAL)
  def taint(); end

  sig.returns(Errno::EINVAL)
  def trust(); end

  sig.returns(Errno::EINVAL)
  def untaint(); end

  sig.returns(Errno::EINVAL)
  def untrust(); end
end

class Errno::EIO < SystemCallError
  sig.returns(Errno::EIO)
  def clone(); end

  sig.returns(Errno::EIO)
  def dup(); end

  sig.returns(Errno::EIO)
  def freeze(); end

  sig.returns(Errno::EIO)
  def taint(); end

  sig.returns(Errno::EIO)
  def trust(); end

  sig.returns(Errno::EIO)
  def untaint(); end

  sig.returns(Errno::EIO)
  def untrust(); end
end

class Errno::EISCONN < SystemCallError
  sig.returns(Errno::EISCONN)
  def clone(); end

  sig.returns(Errno::EISCONN)
  def dup(); end

  sig.returns(Errno::EISCONN)
  def freeze(); end

  sig.returns(Errno::EISCONN)
  def taint(); end

  sig.returns(Errno::EISCONN)
  def trust(); end

  sig.returns(Errno::EISCONN)
  def untaint(); end

  sig.returns(Errno::EISCONN)
  def untrust(); end
end

class Errno::EISDIR < SystemCallError
  sig.returns(Errno::EISDIR)
  def clone(); end

  sig.returns(Errno::EISDIR)
  def dup(); end

  sig.returns(Errno::EISDIR)
  def freeze(); end

  sig.returns(Errno::EISDIR)
  def taint(); end

  sig.returns(Errno::EISDIR)
  def trust(); end

  sig.returns(Errno::EISDIR)
  def untaint(); end

  sig.returns(Errno::EISDIR)
  def untrust(); end
end

class Errno::EISNAM < SystemCallError
  sig.returns(Errno::EISNAM)
  def clone(); end

  sig.returns(Errno::EISNAM)
  def dup(); end

  sig.returns(Errno::EISNAM)
  def freeze(); end

  sig.returns(Errno::EISNAM)
  def taint(); end

  sig.returns(Errno::EISNAM)
  def trust(); end

  sig.returns(Errno::EISNAM)
  def untaint(); end

  sig.returns(Errno::EISNAM)
  def untrust(); end
end

class Errno::EKEYEXPIRED < SystemCallError
  sig.returns(Errno::EKEYEXPIRED)
  def clone(); end

  sig.returns(Errno::EKEYEXPIRED)
  def dup(); end

  sig.returns(Errno::EKEYEXPIRED)
  def freeze(); end

  sig.returns(Errno::EKEYEXPIRED)
  def taint(); end

  sig.returns(Errno::EKEYEXPIRED)
  def trust(); end

  sig.returns(Errno::EKEYEXPIRED)
  def untaint(); end

  sig.returns(Errno::EKEYEXPIRED)
  def untrust(); end
end

class Errno::EKEYREJECTED < SystemCallError
  sig.returns(Errno::EKEYREJECTED)
  def clone(); end

  sig.returns(Errno::EKEYREJECTED)
  def dup(); end

  sig.returns(Errno::EKEYREJECTED)
  def freeze(); end

  sig.returns(Errno::EKEYREJECTED)
  def taint(); end

  sig.returns(Errno::EKEYREJECTED)
  def trust(); end

  sig.returns(Errno::EKEYREJECTED)
  def untaint(); end

  sig.returns(Errno::EKEYREJECTED)
  def untrust(); end
end

class Errno::EKEYREVOKED < SystemCallError
  sig.returns(Errno::EKEYREVOKED)
  def clone(); end

  sig.returns(Errno::EKEYREVOKED)
  def dup(); end

  sig.returns(Errno::EKEYREVOKED)
  def freeze(); end

  sig.returns(Errno::EKEYREVOKED)
  def taint(); end

  sig.returns(Errno::EKEYREVOKED)
  def trust(); end

  sig.returns(Errno::EKEYREVOKED)
  def untaint(); end

  sig.returns(Errno::EKEYREVOKED)
  def untrust(); end
end

class Errno::EL2HLT < SystemCallError
  sig.returns(Errno::EL2HLT)
  def clone(); end

  sig.returns(Errno::EL2HLT)
  def dup(); end

  sig.returns(Errno::EL2HLT)
  def freeze(); end

  sig.returns(Errno::EL2HLT)
  def taint(); end

  sig.returns(Errno::EL2HLT)
  def trust(); end

  sig.returns(Errno::EL2HLT)
  def untaint(); end

  sig.returns(Errno::EL2HLT)
  def untrust(); end
end

class Errno::EL2NSYNC < SystemCallError
  sig.returns(Errno::EL2NSYNC)
  def clone(); end

  sig.returns(Errno::EL2NSYNC)
  def dup(); end

  sig.returns(Errno::EL2NSYNC)
  def freeze(); end

  sig.returns(Errno::EL2NSYNC)
  def taint(); end

  sig.returns(Errno::EL2NSYNC)
  def trust(); end

  sig.returns(Errno::EL2NSYNC)
  def untaint(); end

  sig.returns(Errno::EL2NSYNC)
  def untrust(); end
end

class Errno::EL3HLT < SystemCallError
  sig.returns(Errno::EL3HLT)
  def clone(); end

  sig.returns(Errno::EL3HLT)
  def dup(); end

  sig.returns(Errno::EL3HLT)
  def freeze(); end

  sig.returns(Errno::EL3HLT)
  def taint(); end

  sig.returns(Errno::EL3HLT)
  def trust(); end

  sig.returns(Errno::EL3HLT)
  def untaint(); end

  sig.returns(Errno::EL3HLT)
  def untrust(); end
end

class Errno::EL3RST < SystemCallError
  sig.returns(Errno::EL3RST)
  def clone(); end

  sig.returns(Errno::EL3RST)
  def dup(); end

  sig.returns(Errno::EL3RST)
  def freeze(); end

  sig.returns(Errno::EL3RST)
  def taint(); end

  sig.returns(Errno::EL3RST)
  def trust(); end

  sig.returns(Errno::EL3RST)
  def untaint(); end

  sig.returns(Errno::EL3RST)
  def untrust(); end
end

class Errno::ELIBACC < SystemCallError
  sig.returns(Errno::ELIBACC)
  def clone(); end

  sig.returns(Errno::ELIBACC)
  def dup(); end

  sig.returns(Errno::ELIBACC)
  def freeze(); end

  sig.returns(Errno::ELIBACC)
  def taint(); end

  sig.returns(Errno::ELIBACC)
  def trust(); end

  sig.returns(Errno::ELIBACC)
  def untaint(); end

  sig.returns(Errno::ELIBACC)
  def untrust(); end
end

class Errno::ELIBBAD < SystemCallError
  sig.returns(Errno::ELIBBAD)
  def clone(); end

  sig.returns(Errno::ELIBBAD)
  def dup(); end

  sig.returns(Errno::ELIBBAD)
  def freeze(); end

  sig.returns(Errno::ELIBBAD)
  def taint(); end

  sig.returns(Errno::ELIBBAD)
  def trust(); end

  sig.returns(Errno::ELIBBAD)
  def untaint(); end

  sig.returns(Errno::ELIBBAD)
  def untrust(); end
end

class Errno::ELIBEXEC < SystemCallError
  sig.returns(Errno::ELIBEXEC)
  def clone(); end

  sig.returns(Errno::ELIBEXEC)
  def dup(); end

  sig.returns(Errno::ELIBEXEC)
  def freeze(); end

  sig.returns(Errno::ELIBEXEC)
  def taint(); end

  sig.returns(Errno::ELIBEXEC)
  def trust(); end

  sig.returns(Errno::ELIBEXEC)
  def untaint(); end

  sig.returns(Errno::ELIBEXEC)
  def untrust(); end
end

class Errno::ELIBMAX < SystemCallError
  sig.returns(Errno::ELIBMAX)
  def clone(); end

  sig.returns(Errno::ELIBMAX)
  def dup(); end

  sig.returns(Errno::ELIBMAX)
  def freeze(); end

  sig.returns(Errno::ELIBMAX)
  def taint(); end

  sig.returns(Errno::ELIBMAX)
  def trust(); end

  sig.returns(Errno::ELIBMAX)
  def untaint(); end

  sig.returns(Errno::ELIBMAX)
  def untrust(); end
end

class Errno::ELIBSCN < SystemCallError
  sig.returns(Errno::ELIBSCN)
  def clone(); end

  sig.returns(Errno::ELIBSCN)
  def dup(); end

  sig.returns(Errno::ELIBSCN)
  def freeze(); end

  sig.returns(Errno::ELIBSCN)
  def taint(); end

  sig.returns(Errno::ELIBSCN)
  def trust(); end

  sig.returns(Errno::ELIBSCN)
  def untaint(); end

  sig.returns(Errno::ELIBSCN)
  def untrust(); end
end

class Errno::ELNRNG < SystemCallError
  sig.returns(Errno::ELNRNG)
  def clone(); end

  sig.returns(Errno::ELNRNG)
  def dup(); end

  sig.returns(Errno::ELNRNG)
  def freeze(); end

  sig.returns(Errno::ELNRNG)
  def taint(); end

  sig.returns(Errno::ELNRNG)
  def trust(); end

  sig.returns(Errno::ELNRNG)
  def untaint(); end

  sig.returns(Errno::ELNRNG)
  def untrust(); end
end

class Errno::ELOOP < SystemCallError
  sig.returns(Errno::ELOOP)
  def clone(); end

  sig.returns(Errno::ELOOP)
  def dup(); end

  sig.returns(Errno::ELOOP)
  def freeze(); end

  sig.returns(Errno::ELOOP)
  def taint(); end

  sig.returns(Errno::ELOOP)
  def trust(); end

  sig.returns(Errno::ELOOP)
  def untaint(); end

  sig.returns(Errno::ELOOP)
  def untrust(); end
end

class Errno::EMEDIUMTYPE < SystemCallError
  sig.returns(Errno::EMEDIUMTYPE)
  def clone(); end

  sig.returns(Errno::EMEDIUMTYPE)
  def dup(); end

  sig.returns(Errno::EMEDIUMTYPE)
  def freeze(); end

  sig.returns(Errno::EMEDIUMTYPE)
  def taint(); end

  sig.returns(Errno::EMEDIUMTYPE)
  def trust(); end

  sig.returns(Errno::EMEDIUMTYPE)
  def untaint(); end

  sig.returns(Errno::EMEDIUMTYPE)
  def untrust(); end
end

class Errno::EMFILE < SystemCallError
  sig.returns(Errno::EMFILE)
  def clone(); end

  sig.returns(Errno::EMFILE)
  def dup(); end

  sig.returns(Errno::EMFILE)
  def freeze(); end

  sig.returns(Errno::EMFILE)
  def taint(); end

  sig.returns(Errno::EMFILE)
  def trust(); end

  sig.returns(Errno::EMFILE)
  def untaint(); end

  sig.returns(Errno::EMFILE)
  def untrust(); end
end

class Errno::EMLINK < SystemCallError
  sig.returns(Errno::EMLINK)
  def clone(); end

  sig.returns(Errno::EMLINK)
  def dup(); end

  sig.returns(Errno::EMLINK)
  def freeze(); end

  sig.returns(Errno::EMLINK)
  def taint(); end

  sig.returns(Errno::EMLINK)
  def trust(); end

  sig.returns(Errno::EMLINK)
  def untaint(); end

  sig.returns(Errno::EMLINK)
  def untrust(); end
end

class Errno::EMSGSIZE < SystemCallError
  sig.returns(Errno::EMSGSIZE)
  def clone(); end

  sig.returns(Errno::EMSGSIZE)
  def dup(); end

  sig.returns(Errno::EMSGSIZE)
  def freeze(); end

  sig.returns(Errno::EMSGSIZE)
  def taint(); end

  sig.returns(Errno::EMSGSIZE)
  def trust(); end

  sig.returns(Errno::EMSGSIZE)
  def untaint(); end

  sig.returns(Errno::EMSGSIZE)
  def untrust(); end
end

class Errno::EMULTIHOP < SystemCallError
  sig.returns(Errno::EMULTIHOP)
  def clone(); end

  sig.returns(Errno::EMULTIHOP)
  def dup(); end

  sig.returns(Errno::EMULTIHOP)
  def freeze(); end

  sig.returns(Errno::EMULTIHOP)
  def taint(); end

  sig.returns(Errno::EMULTIHOP)
  def trust(); end

  sig.returns(Errno::EMULTIHOP)
  def untaint(); end

  sig.returns(Errno::EMULTIHOP)
  def untrust(); end
end

class Errno::ENAMETOOLONG < SystemCallError
  sig.returns(Errno::ENAMETOOLONG)
  def clone(); end

  sig.returns(Errno::ENAMETOOLONG)
  def dup(); end

  sig.returns(Errno::ENAMETOOLONG)
  def freeze(); end

  sig.returns(Errno::ENAMETOOLONG)
  def taint(); end

  sig.returns(Errno::ENAMETOOLONG)
  def trust(); end

  sig.returns(Errno::ENAMETOOLONG)
  def untaint(); end

  sig.returns(Errno::ENAMETOOLONG)
  def untrust(); end
end

class Errno::ENAVAIL < SystemCallError
  sig.returns(Errno::ENAVAIL)
  def clone(); end

  sig.returns(Errno::ENAVAIL)
  def dup(); end

  sig.returns(Errno::ENAVAIL)
  def freeze(); end

  sig.returns(Errno::ENAVAIL)
  def taint(); end

  sig.returns(Errno::ENAVAIL)
  def trust(); end

  sig.returns(Errno::ENAVAIL)
  def untaint(); end

  sig.returns(Errno::ENAVAIL)
  def untrust(); end
end

class Errno::ENETDOWN < SystemCallError
  sig.returns(Errno::ENETDOWN)
  def clone(); end

  sig.returns(Errno::ENETDOWN)
  def dup(); end

  sig.returns(Errno::ENETDOWN)
  def freeze(); end

  sig.returns(Errno::ENETDOWN)
  def taint(); end

  sig.returns(Errno::ENETDOWN)
  def trust(); end

  sig.returns(Errno::ENETDOWN)
  def untaint(); end

  sig.returns(Errno::ENETDOWN)
  def untrust(); end
end

class Errno::ENETRESET < SystemCallError
  sig.returns(Errno::ENETRESET)
  def clone(); end

  sig.returns(Errno::ENETRESET)
  def dup(); end

  sig.returns(Errno::ENETRESET)
  def freeze(); end

  sig.returns(Errno::ENETRESET)
  def taint(); end

  sig.returns(Errno::ENETRESET)
  def trust(); end

  sig.returns(Errno::ENETRESET)
  def untaint(); end

  sig.returns(Errno::ENETRESET)
  def untrust(); end
end

class Errno::ENETUNREACH < SystemCallError
  sig.returns(Errno::ENETUNREACH)
  def clone(); end

  sig.returns(Errno::ENETUNREACH)
  def dup(); end

  sig.returns(Errno::ENETUNREACH)
  def freeze(); end

  sig.returns(Errno::ENETUNREACH)
  def taint(); end

  sig.returns(Errno::ENETUNREACH)
  def trust(); end

  sig.returns(Errno::ENETUNREACH)
  def untaint(); end

  sig.returns(Errno::ENETUNREACH)
  def untrust(); end
end

class Errno::ENFILE < SystemCallError
  sig.returns(Errno::ENFILE)
  def clone(); end

  sig.returns(Errno::ENFILE)
  def dup(); end

  sig.returns(Errno::ENFILE)
  def freeze(); end

  sig.returns(Errno::ENFILE)
  def taint(); end

  sig.returns(Errno::ENFILE)
  def trust(); end

  sig.returns(Errno::ENFILE)
  def untaint(); end

  sig.returns(Errno::ENFILE)
  def untrust(); end
end

class Errno::ENOANO < SystemCallError
  sig.returns(Errno::ENOANO)
  def clone(); end

  sig.returns(Errno::ENOANO)
  def dup(); end

  sig.returns(Errno::ENOANO)
  def freeze(); end

  sig.returns(Errno::ENOANO)
  def taint(); end

  sig.returns(Errno::ENOANO)
  def trust(); end

  sig.returns(Errno::ENOANO)
  def untaint(); end

  sig.returns(Errno::ENOANO)
  def untrust(); end
end

class Errno::ENOBUFS < SystemCallError
  sig.returns(Errno::ENOBUFS)
  def clone(); end

  sig.returns(Errno::ENOBUFS)
  def dup(); end

  sig.returns(Errno::ENOBUFS)
  def freeze(); end

  sig.returns(Errno::ENOBUFS)
  def taint(); end

  sig.returns(Errno::ENOBUFS)
  def trust(); end

  sig.returns(Errno::ENOBUFS)
  def untaint(); end

  sig.returns(Errno::ENOBUFS)
  def untrust(); end
end

class Errno::ENOCSI < SystemCallError
  sig.returns(Errno::ENOCSI)
  def clone(); end

  sig.returns(Errno::ENOCSI)
  def dup(); end

  sig.returns(Errno::ENOCSI)
  def freeze(); end

  sig.returns(Errno::ENOCSI)
  def taint(); end

  sig.returns(Errno::ENOCSI)
  def trust(); end

  sig.returns(Errno::ENOCSI)
  def untaint(); end

  sig.returns(Errno::ENOCSI)
  def untrust(); end
end

class Errno::ENODATA < SystemCallError
  sig.returns(Errno::ENODATA)
  def clone(); end

  sig.returns(Errno::ENODATA)
  def dup(); end

  sig.returns(Errno::ENODATA)
  def freeze(); end

  sig.returns(Errno::ENODATA)
  def taint(); end

  sig.returns(Errno::ENODATA)
  def trust(); end

  sig.returns(Errno::ENODATA)
  def untaint(); end

  sig.returns(Errno::ENODATA)
  def untrust(); end
end

class Errno::ENODEV < SystemCallError
  sig.returns(Errno::ENODEV)
  def clone(); end

  sig.returns(Errno::ENODEV)
  def dup(); end

  sig.returns(Errno::ENODEV)
  def freeze(); end

  sig.returns(Errno::ENODEV)
  def taint(); end

  sig.returns(Errno::ENODEV)
  def trust(); end

  sig.returns(Errno::ENODEV)
  def untaint(); end

  sig.returns(Errno::ENODEV)
  def untrust(); end
end

class Errno::ENOENT < SystemCallError
  sig.returns(Errno::ENOENT)
  def clone(); end

  sig.returns(Errno::ENOENT)
  def dup(); end

  sig.returns(Errno::ENOENT)
  def freeze(); end

  sig.returns(Errno::ENOENT)
  def taint(); end

  sig.returns(Errno::ENOENT)
  def trust(); end

  sig.returns(Errno::ENOENT)
  def untaint(); end

  sig.returns(Errno::ENOENT)
  def untrust(); end
end

class Errno::ENOEXEC < SystemCallError
  sig.returns(Errno::ENOEXEC)
  def clone(); end

  sig.returns(Errno::ENOEXEC)
  def dup(); end

  sig.returns(Errno::ENOEXEC)
  def freeze(); end

  sig.returns(Errno::ENOEXEC)
  def taint(); end

  sig.returns(Errno::ENOEXEC)
  def trust(); end

  sig.returns(Errno::ENOEXEC)
  def untaint(); end

  sig.returns(Errno::ENOEXEC)
  def untrust(); end
end

class Errno::ENOKEY < SystemCallError
  sig.returns(Errno::ENOKEY)
  def clone(); end

  sig.returns(Errno::ENOKEY)
  def dup(); end

  sig.returns(Errno::ENOKEY)
  def freeze(); end

  sig.returns(Errno::ENOKEY)
  def taint(); end

  sig.returns(Errno::ENOKEY)
  def trust(); end

  sig.returns(Errno::ENOKEY)
  def untaint(); end

  sig.returns(Errno::ENOKEY)
  def untrust(); end
end

class Errno::ENOLCK < SystemCallError
  sig.returns(Errno::ENOLCK)
  def clone(); end

  sig.returns(Errno::ENOLCK)
  def dup(); end

  sig.returns(Errno::ENOLCK)
  def freeze(); end

  sig.returns(Errno::ENOLCK)
  def taint(); end

  sig.returns(Errno::ENOLCK)
  def trust(); end

  sig.returns(Errno::ENOLCK)
  def untaint(); end

  sig.returns(Errno::ENOLCK)
  def untrust(); end
end

class Errno::ENOLINK < SystemCallError
  sig.returns(Errno::ENOLINK)
  def clone(); end

  sig.returns(Errno::ENOLINK)
  def dup(); end

  sig.returns(Errno::ENOLINK)
  def freeze(); end

  sig.returns(Errno::ENOLINK)
  def taint(); end

  sig.returns(Errno::ENOLINK)
  def trust(); end

  sig.returns(Errno::ENOLINK)
  def untaint(); end

  sig.returns(Errno::ENOLINK)
  def untrust(); end
end

class Errno::ENOMEDIUM < SystemCallError
  sig.returns(Errno::ENOMEDIUM)
  def clone(); end

  sig.returns(Errno::ENOMEDIUM)
  def dup(); end

  sig.returns(Errno::ENOMEDIUM)
  def freeze(); end

  sig.returns(Errno::ENOMEDIUM)
  def taint(); end

  sig.returns(Errno::ENOMEDIUM)
  def trust(); end

  sig.returns(Errno::ENOMEDIUM)
  def untaint(); end

  sig.returns(Errno::ENOMEDIUM)
  def untrust(); end
end

class Errno::ENOMEM < SystemCallError
  sig.returns(Errno::ENOMEM)
  def clone(); end

  sig.returns(Errno::ENOMEM)
  def dup(); end

  sig.returns(Errno::ENOMEM)
  def freeze(); end

  sig.returns(Errno::ENOMEM)
  def taint(); end

  sig.returns(Errno::ENOMEM)
  def trust(); end

  sig.returns(Errno::ENOMEM)
  def untaint(); end

  sig.returns(Errno::ENOMEM)
  def untrust(); end
end

class Errno::ENOMSG < SystemCallError
  sig.returns(Errno::ENOMSG)
  def clone(); end

  sig.returns(Errno::ENOMSG)
  def dup(); end

  sig.returns(Errno::ENOMSG)
  def freeze(); end

  sig.returns(Errno::ENOMSG)
  def taint(); end

  sig.returns(Errno::ENOMSG)
  def trust(); end

  sig.returns(Errno::ENOMSG)
  def untaint(); end

  sig.returns(Errno::ENOMSG)
  def untrust(); end
end

class Errno::ENONET < SystemCallError
  sig.returns(Errno::ENONET)
  def clone(); end

  sig.returns(Errno::ENONET)
  def dup(); end

  sig.returns(Errno::ENONET)
  def freeze(); end

  sig.returns(Errno::ENONET)
  def taint(); end

  sig.returns(Errno::ENONET)
  def trust(); end

  sig.returns(Errno::ENONET)
  def untaint(); end

  sig.returns(Errno::ENONET)
  def untrust(); end
end

class Errno::ENOPKG < SystemCallError
  sig.returns(Errno::ENOPKG)
  def clone(); end

  sig.returns(Errno::ENOPKG)
  def dup(); end

  sig.returns(Errno::ENOPKG)
  def freeze(); end

  sig.returns(Errno::ENOPKG)
  def taint(); end

  sig.returns(Errno::ENOPKG)
  def trust(); end

  sig.returns(Errno::ENOPKG)
  def untaint(); end

  sig.returns(Errno::ENOPKG)
  def untrust(); end
end

class Errno::ENOPROTOOPT < SystemCallError
  sig.returns(Errno::ENOPROTOOPT)
  def clone(); end

  sig.returns(Errno::ENOPROTOOPT)
  def dup(); end

  sig.returns(Errno::ENOPROTOOPT)
  def freeze(); end

  sig.returns(Errno::ENOPROTOOPT)
  def taint(); end

  sig.returns(Errno::ENOPROTOOPT)
  def trust(); end

  sig.returns(Errno::ENOPROTOOPT)
  def untaint(); end

  sig.returns(Errno::ENOPROTOOPT)
  def untrust(); end
end

class Errno::ENOSPC < SystemCallError
  sig.returns(Errno::ENOSPC)
  def clone(); end

  sig.returns(Errno::ENOSPC)
  def dup(); end

  sig.returns(Errno::ENOSPC)
  def freeze(); end

  sig.returns(Errno::ENOSPC)
  def taint(); end

  sig.returns(Errno::ENOSPC)
  def trust(); end

  sig.returns(Errno::ENOSPC)
  def untaint(); end

  sig.returns(Errno::ENOSPC)
  def untrust(); end
end

class Errno::ENOSR < SystemCallError
  sig.returns(Errno::ENOSR)
  def clone(); end

  sig.returns(Errno::ENOSR)
  def dup(); end

  sig.returns(Errno::ENOSR)
  def freeze(); end

  sig.returns(Errno::ENOSR)
  def taint(); end

  sig.returns(Errno::ENOSR)
  def trust(); end

  sig.returns(Errno::ENOSR)
  def untaint(); end

  sig.returns(Errno::ENOSR)
  def untrust(); end
end

class Errno::ENOSTR < SystemCallError
  sig.returns(Errno::ENOSTR)
  def clone(); end

  sig.returns(Errno::ENOSTR)
  def dup(); end

  sig.returns(Errno::ENOSTR)
  def freeze(); end

  sig.returns(Errno::ENOSTR)
  def taint(); end

  sig.returns(Errno::ENOSTR)
  def trust(); end

  sig.returns(Errno::ENOSTR)
  def untaint(); end

  sig.returns(Errno::ENOSTR)
  def untrust(); end
end

class Errno::ENOSYS < SystemCallError
  sig.returns(Errno::ENOSYS)
  def clone(); end

  sig.returns(Errno::ENOSYS)
  def dup(); end

  sig.returns(Errno::ENOSYS)
  def freeze(); end

  sig.returns(Errno::ENOSYS)
  def taint(); end

  sig.returns(Errno::ENOSYS)
  def trust(); end

  sig.returns(Errno::ENOSYS)
  def untaint(); end

  sig.returns(Errno::ENOSYS)
  def untrust(); end
end

class Errno::ENOTBLK < SystemCallError
  sig.returns(Errno::ENOTBLK)
  def clone(); end

  sig.returns(Errno::ENOTBLK)
  def dup(); end

  sig.returns(Errno::ENOTBLK)
  def freeze(); end

  sig.returns(Errno::ENOTBLK)
  def taint(); end

  sig.returns(Errno::ENOTBLK)
  def trust(); end

  sig.returns(Errno::ENOTBLK)
  def untaint(); end

  sig.returns(Errno::ENOTBLK)
  def untrust(); end
end

class Errno::ENOTCONN < SystemCallError
  sig.returns(Errno::ENOTCONN)
  def clone(); end

  sig.returns(Errno::ENOTCONN)
  def dup(); end

  sig.returns(Errno::ENOTCONN)
  def freeze(); end

  sig.returns(Errno::ENOTCONN)
  def taint(); end

  sig.returns(Errno::ENOTCONN)
  def trust(); end

  sig.returns(Errno::ENOTCONN)
  def untaint(); end

  sig.returns(Errno::ENOTCONN)
  def untrust(); end
end

class Errno::ENOTDIR < SystemCallError
  sig.returns(Errno::ENOTDIR)
  def clone(); end

  sig.returns(Errno::ENOTDIR)
  def dup(); end

  sig.returns(Errno::ENOTDIR)
  def freeze(); end

  sig.returns(Errno::ENOTDIR)
  def taint(); end

  sig.returns(Errno::ENOTDIR)
  def trust(); end

  sig.returns(Errno::ENOTDIR)
  def untaint(); end

  sig.returns(Errno::ENOTDIR)
  def untrust(); end
end

class Errno::ENOTEMPTY < SystemCallError
  sig.returns(Errno::ENOTEMPTY)
  def clone(); end

  sig.returns(Errno::ENOTEMPTY)
  def dup(); end

  sig.returns(Errno::ENOTEMPTY)
  def freeze(); end

  sig.returns(Errno::ENOTEMPTY)
  def taint(); end

  sig.returns(Errno::ENOTEMPTY)
  def trust(); end

  sig.returns(Errno::ENOTEMPTY)
  def untaint(); end

  sig.returns(Errno::ENOTEMPTY)
  def untrust(); end
end

class Errno::ENOTNAM < SystemCallError
  sig.returns(Errno::ENOTNAM)
  def clone(); end

  sig.returns(Errno::ENOTNAM)
  def dup(); end

  sig.returns(Errno::ENOTNAM)
  def freeze(); end

  sig.returns(Errno::ENOTNAM)
  def taint(); end

  sig.returns(Errno::ENOTNAM)
  def trust(); end

  sig.returns(Errno::ENOTNAM)
  def untaint(); end

  sig.returns(Errno::ENOTNAM)
  def untrust(); end
end

class Errno::ENOTRECOVERABLE < SystemCallError
  sig.returns(Errno::ENOTRECOVERABLE)
  def clone(); end

  sig.returns(Errno::ENOTRECOVERABLE)
  def dup(); end

  sig.returns(Errno::ENOTRECOVERABLE)
  def freeze(); end

  sig.returns(Errno::ENOTRECOVERABLE)
  def taint(); end

  sig.returns(Errno::ENOTRECOVERABLE)
  def trust(); end

  sig.returns(Errno::ENOTRECOVERABLE)
  def untaint(); end

  sig.returns(Errno::ENOTRECOVERABLE)
  def untrust(); end
end

class Errno::ENOTSOCK < SystemCallError
  sig.returns(Errno::ENOTSOCK)
  def clone(); end

  sig.returns(Errno::ENOTSOCK)
  def dup(); end

  sig.returns(Errno::ENOTSOCK)
  def freeze(); end

  sig.returns(Errno::ENOTSOCK)
  def taint(); end

  sig.returns(Errno::ENOTSOCK)
  def trust(); end

  sig.returns(Errno::ENOTSOCK)
  def untaint(); end

  sig.returns(Errno::ENOTSOCK)
  def untrust(); end
end

class Errno::ENOTTY < SystemCallError
  sig.returns(Errno::ENOTTY)
  def clone(); end

  sig.returns(Errno::ENOTTY)
  def dup(); end

  sig.returns(Errno::ENOTTY)
  def freeze(); end

  sig.returns(Errno::ENOTTY)
  def taint(); end

  sig.returns(Errno::ENOTTY)
  def trust(); end

  sig.returns(Errno::ENOTTY)
  def untaint(); end

  sig.returns(Errno::ENOTTY)
  def untrust(); end
end

class Errno::ENOTUNIQ < SystemCallError
  sig.returns(Errno::ENOTUNIQ)
  def clone(); end

  sig.returns(Errno::ENOTUNIQ)
  def dup(); end

  sig.returns(Errno::ENOTUNIQ)
  def freeze(); end

  sig.returns(Errno::ENOTUNIQ)
  def taint(); end

  sig.returns(Errno::ENOTUNIQ)
  def trust(); end

  sig.returns(Errno::ENOTUNIQ)
  def untaint(); end

  sig.returns(Errno::ENOTUNIQ)
  def untrust(); end
end

class Errno::ENXIO < SystemCallError
  sig.returns(Errno::ENXIO)
  def clone(); end

  sig.returns(Errno::ENXIO)
  def dup(); end

  sig.returns(Errno::ENXIO)
  def freeze(); end

  sig.returns(Errno::ENXIO)
  def taint(); end

  sig.returns(Errno::ENXIO)
  def trust(); end

  sig.returns(Errno::ENXIO)
  def untaint(); end

  sig.returns(Errno::ENXIO)
  def untrust(); end
end

class Errno::EOPNOTSUPP < SystemCallError
  sig.returns(Errno::EOPNOTSUPP)
  def clone(); end

  sig.returns(Errno::EOPNOTSUPP)
  def dup(); end

  sig.returns(Errno::EOPNOTSUPP)
  def freeze(); end

  sig.returns(Errno::EOPNOTSUPP)
  def taint(); end

  sig.returns(Errno::EOPNOTSUPP)
  def trust(); end

  sig.returns(Errno::EOPNOTSUPP)
  def untaint(); end

  sig.returns(Errno::EOPNOTSUPP)
  def untrust(); end
end

class Errno::EOVERFLOW < SystemCallError
  sig.returns(Errno::EOVERFLOW)
  def clone(); end

  sig.returns(Errno::EOVERFLOW)
  def dup(); end

  sig.returns(Errno::EOVERFLOW)
  def freeze(); end

  sig.returns(Errno::EOVERFLOW)
  def taint(); end

  sig.returns(Errno::EOVERFLOW)
  def trust(); end

  sig.returns(Errno::EOVERFLOW)
  def untaint(); end

  sig.returns(Errno::EOVERFLOW)
  def untrust(); end
end

class Errno::EOWNERDEAD < SystemCallError
  sig.returns(Errno::EOWNERDEAD)
  def clone(); end

  sig.returns(Errno::EOWNERDEAD)
  def dup(); end

  sig.returns(Errno::EOWNERDEAD)
  def freeze(); end

  sig.returns(Errno::EOWNERDEAD)
  def taint(); end

  sig.returns(Errno::EOWNERDEAD)
  def trust(); end

  sig.returns(Errno::EOWNERDEAD)
  def untaint(); end

  sig.returns(Errno::EOWNERDEAD)
  def untrust(); end
end

class Errno::EPERM < SystemCallError
  sig.returns(Errno::EPERM)
  def clone(); end

  sig.returns(Errno::EPERM)
  def dup(); end

  sig.returns(Errno::EPERM)
  def freeze(); end

  sig.returns(Errno::EPERM)
  def taint(); end

  sig.returns(Errno::EPERM)
  def trust(); end

  sig.returns(Errno::EPERM)
  def untaint(); end

  sig.returns(Errno::EPERM)
  def untrust(); end
end

class Errno::EPFNOSUPPORT < SystemCallError
  sig.returns(Errno::EPFNOSUPPORT)
  def clone(); end

  sig.returns(Errno::EPFNOSUPPORT)
  def dup(); end

  sig.returns(Errno::EPFNOSUPPORT)
  def freeze(); end

  sig.returns(Errno::EPFNOSUPPORT)
  def taint(); end

  sig.returns(Errno::EPFNOSUPPORT)
  def trust(); end

  sig.returns(Errno::EPFNOSUPPORT)
  def untaint(); end

  sig.returns(Errno::EPFNOSUPPORT)
  def untrust(); end
end

class Errno::EPIPE < SystemCallError
  sig.returns(Errno::EPIPE)
  def clone(); end

  sig.returns(Errno::EPIPE)
  def dup(); end

  sig.returns(Errno::EPIPE)
  def freeze(); end

  sig.returns(Errno::EPIPE)
  def taint(); end

  sig.returns(Errno::EPIPE)
  def trust(); end

  sig.returns(Errno::EPIPE)
  def untaint(); end

  sig.returns(Errno::EPIPE)
  def untrust(); end
end

class Errno::EPROTO < SystemCallError
  sig.returns(Errno::EPROTO)
  def clone(); end

  sig.returns(Errno::EPROTO)
  def dup(); end

  sig.returns(Errno::EPROTO)
  def freeze(); end

  sig.returns(Errno::EPROTO)
  def taint(); end

  sig.returns(Errno::EPROTO)
  def trust(); end

  sig.returns(Errno::EPROTO)
  def untaint(); end

  sig.returns(Errno::EPROTO)
  def untrust(); end
end

class Errno::EPROTONOSUPPORT < SystemCallError
  sig.returns(Errno::EPROTONOSUPPORT)
  def clone(); end

  sig.returns(Errno::EPROTONOSUPPORT)
  def dup(); end

  sig.returns(Errno::EPROTONOSUPPORT)
  def freeze(); end

  sig.returns(Errno::EPROTONOSUPPORT)
  def taint(); end

  sig.returns(Errno::EPROTONOSUPPORT)
  def trust(); end

  sig.returns(Errno::EPROTONOSUPPORT)
  def untaint(); end

  sig.returns(Errno::EPROTONOSUPPORT)
  def untrust(); end
end

class Errno::EPROTOTYPE < SystemCallError
  sig.returns(Errno::EPROTOTYPE)
  def clone(); end

  sig.returns(Errno::EPROTOTYPE)
  def dup(); end

  sig.returns(Errno::EPROTOTYPE)
  def freeze(); end

  sig.returns(Errno::EPROTOTYPE)
  def taint(); end

  sig.returns(Errno::EPROTOTYPE)
  def trust(); end

  sig.returns(Errno::EPROTOTYPE)
  def untaint(); end

  sig.returns(Errno::EPROTOTYPE)
  def untrust(); end
end

class Errno::ERANGE < SystemCallError
  sig.returns(Errno::ERANGE)
  def clone(); end

  sig.returns(Errno::ERANGE)
  def dup(); end

  sig.returns(Errno::ERANGE)
  def freeze(); end

  sig.returns(Errno::ERANGE)
  def taint(); end

  sig.returns(Errno::ERANGE)
  def trust(); end

  sig.returns(Errno::ERANGE)
  def untaint(); end

  sig.returns(Errno::ERANGE)
  def untrust(); end
end

class Errno::EREMCHG < SystemCallError
  sig.returns(Errno::EREMCHG)
  def clone(); end

  sig.returns(Errno::EREMCHG)
  def dup(); end

  sig.returns(Errno::EREMCHG)
  def freeze(); end

  sig.returns(Errno::EREMCHG)
  def taint(); end

  sig.returns(Errno::EREMCHG)
  def trust(); end

  sig.returns(Errno::EREMCHG)
  def untaint(); end

  sig.returns(Errno::EREMCHG)
  def untrust(); end
end

class Errno::EREMOTE < SystemCallError
  sig.returns(Errno::EREMOTE)
  def clone(); end

  sig.returns(Errno::EREMOTE)
  def dup(); end

  sig.returns(Errno::EREMOTE)
  def freeze(); end

  sig.returns(Errno::EREMOTE)
  def taint(); end

  sig.returns(Errno::EREMOTE)
  def trust(); end

  sig.returns(Errno::EREMOTE)
  def untaint(); end

  sig.returns(Errno::EREMOTE)
  def untrust(); end
end

class Errno::EREMOTEIO < SystemCallError
  sig.returns(Errno::EREMOTEIO)
  def clone(); end

  sig.returns(Errno::EREMOTEIO)
  def dup(); end

  sig.returns(Errno::EREMOTEIO)
  def freeze(); end

  sig.returns(Errno::EREMOTEIO)
  def taint(); end

  sig.returns(Errno::EREMOTEIO)
  def trust(); end

  sig.returns(Errno::EREMOTEIO)
  def untaint(); end

  sig.returns(Errno::EREMOTEIO)
  def untrust(); end
end

class Errno::ERESTART < SystemCallError
  sig.returns(Errno::ERESTART)
  def clone(); end

  sig.returns(Errno::ERESTART)
  def dup(); end

  sig.returns(Errno::ERESTART)
  def freeze(); end

  sig.returns(Errno::ERESTART)
  def taint(); end

  sig.returns(Errno::ERESTART)
  def trust(); end

  sig.returns(Errno::ERESTART)
  def untaint(); end

  sig.returns(Errno::ERESTART)
  def untrust(); end
end

class Errno::ERFKILL < SystemCallError
  sig.returns(Errno::ERFKILL)
  def clone(); end

  sig.returns(Errno::ERFKILL)
  def dup(); end

  sig.returns(Errno::ERFKILL)
  def freeze(); end

  sig.returns(Errno::ERFKILL)
  def taint(); end

  sig.returns(Errno::ERFKILL)
  def trust(); end

  sig.returns(Errno::ERFKILL)
  def untaint(); end

  sig.returns(Errno::ERFKILL)
  def untrust(); end
end

class Errno::EROFS < SystemCallError
  sig.returns(Errno::EROFS)
  def clone(); end

  sig.returns(Errno::EROFS)
  def dup(); end

  sig.returns(Errno::EROFS)
  def freeze(); end

  sig.returns(Errno::EROFS)
  def taint(); end

  sig.returns(Errno::EROFS)
  def trust(); end

  sig.returns(Errno::EROFS)
  def untaint(); end

  sig.returns(Errno::EROFS)
  def untrust(); end
end

class Errno::ESHUTDOWN < SystemCallError
  sig.returns(Errno::ESHUTDOWN)
  def clone(); end

  sig.returns(Errno::ESHUTDOWN)
  def dup(); end

  sig.returns(Errno::ESHUTDOWN)
  def freeze(); end

  sig.returns(Errno::ESHUTDOWN)
  def taint(); end

  sig.returns(Errno::ESHUTDOWN)
  def trust(); end

  sig.returns(Errno::ESHUTDOWN)
  def untaint(); end

  sig.returns(Errno::ESHUTDOWN)
  def untrust(); end
end

class Errno::ESOCKTNOSUPPORT < SystemCallError
  sig.returns(Errno::ESOCKTNOSUPPORT)
  def clone(); end

  sig.returns(Errno::ESOCKTNOSUPPORT)
  def dup(); end

  sig.returns(Errno::ESOCKTNOSUPPORT)
  def freeze(); end

  sig.returns(Errno::ESOCKTNOSUPPORT)
  def taint(); end

  sig.returns(Errno::ESOCKTNOSUPPORT)
  def trust(); end

  sig.returns(Errno::ESOCKTNOSUPPORT)
  def untaint(); end

  sig.returns(Errno::ESOCKTNOSUPPORT)
  def untrust(); end
end

class Errno::ESPIPE < SystemCallError
  sig.returns(Errno::ESPIPE)
  def clone(); end

  sig.returns(Errno::ESPIPE)
  def dup(); end

  sig.returns(Errno::ESPIPE)
  def freeze(); end

  sig.returns(Errno::ESPIPE)
  def taint(); end

  sig.returns(Errno::ESPIPE)
  def trust(); end

  sig.returns(Errno::ESPIPE)
  def untaint(); end

  sig.returns(Errno::ESPIPE)
  def untrust(); end
end

class Errno::ESRCH < SystemCallError
  sig.returns(Errno::ESRCH)
  def clone(); end

  sig.returns(Errno::ESRCH)
  def dup(); end

  sig.returns(Errno::ESRCH)
  def freeze(); end

  sig.returns(Errno::ESRCH)
  def taint(); end

  sig.returns(Errno::ESRCH)
  def trust(); end

  sig.returns(Errno::ESRCH)
  def untaint(); end

  sig.returns(Errno::ESRCH)
  def untrust(); end
end

class Errno::ESRMNT < SystemCallError
  sig.returns(Errno::ESRMNT)
  def clone(); end

  sig.returns(Errno::ESRMNT)
  def dup(); end

  sig.returns(Errno::ESRMNT)
  def freeze(); end

  sig.returns(Errno::ESRMNT)
  def taint(); end

  sig.returns(Errno::ESRMNT)
  def trust(); end

  sig.returns(Errno::ESRMNT)
  def untaint(); end

  sig.returns(Errno::ESRMNT)
  def untrust(); end
end

class Errno::ESTALE < SystemCallError
  sig.returns(Errno::ESTALE)
  def clone(); end

  sig.returns(Errno::ESTALE)
  def dup(); end

  sig.returns(Errno::ESTALE)
  def freeze(); end

  sig.returns(Errno::ESTALE)
  def taint(); end

  sig.returns(Errno::ESTALE)
  def trust(); end

  sig.returns(Errno::ESTALE)
  def untaint(); end

  sig.returns(Errno::ESTALE)
  def untrust(); end
end

class Errno::ESTRPIPE < SystemCallError
  sig.returns(Errno::ESTRPIPE)
  def clone(); end

  sig.returns(Errno::ESTRPIPE)
  def dup(); end

  sig.returns(Errno::ESTRPIPE)
  def freeze(); end

  sig.returns(Errno::ESTRPIPE)
  def taint(); end

  sig.returns(Errno::ESTRPIPE)
  def trust(); end

  sig.returns(Errno::ESTRPIPE)
  def untaint(); end

  sig.returns(Errno::ESTRPIPE)
  def untrust(); end
end

class Errno::ETIME < SystemCallError
  sig.returns(Errno::ETIME)
  def clone(); end

  sig.returns(Errno::ETIME)
  def dup(); end

  sig.returns(Errno::ETIME)
  def freeze(); end

  sig.returns(Errno::ETIME)
  def taint(); end

  sig.returns(Errno::ETIME)
  def trust(); end

  sig.returns(Errno::ETIME)
  def untaint(); end

  sig.returns(Errno::ETIME)
  def untrust(); end
end

class Errno::ETIMEDOUT < SystemCallError
  sig.returns(Errno::ETIMEDOUT)
  def clone(); end

  sig.returns(Errno::ETIMEDOUT)
  def dup(); end

  sig.returns(Errno::ETIMEDOUT)
  def freeze(); end

  sig.returns(Errno::ETIMEDOUT)
  def taint(); end

  sig.returns(Errno::ETIMEDOUT)
  def trust(); end

  sig.returns(Errno::ETIMEDOUT)
  def untaint(); end

  sig.returns(Errno::ETIMEDOUT)
  def untrust(); end
end

class Errno::ETOOMANYREFS < SystemCallError
  sig.returns(Errno::ETOOMANYREFS)
  def clone(); end

  sig.returns(Errno::ETOOMANYREFS)
  def dup(); end

  sig.returns(Errno::ETOOMANYREFS)
  def freeze(); end

  sig.returns(Errno::ETOOMANYREFS)
  def taint(); end

  sig.returns(Errno::ETOOMANYREFS)
  def trust(); end

  sig.returns(Errno::ETOOMANYREFS)
  def untaint(); end

  sig.returns(Errno::ETOOMANYREFS)
  def untrust(); end
end

class Errno::ETXTBSY < SystemCallError
  sig.returns(Errno::ETXTBSY)
  def clone(); end

  sig.returns(Errno::ETXTBSY)
  def dup(); end

  sig.returns(Errno::ETXTBSY)
  def freeze(); end

  sig.returns(Errno::ETXTBSY)
  def taint(); end

  sig.returns(Errno::ETXTBSY)
  def trust(); end

  sig.returns(Errno::ETXTBSY)
  def untaint(); end

  sig.returns(Errno::ETXTBSY)
  def untrust(); end
end

class Errno::EUCLEAN < SystemCallError
  sig.returns(Errno::EUCLEAN)
  def clone(); end

  sig.returns(Errno::EUCLEAN)
  def dup(); end

  sig.returns(Errno::EUCLEAN)
  def freeze(); end

  sig.returns(Errno::EUCLEAN)
  def taint(); end

  sig.returns(Errno::EUCLEAN)
  def trust(); end

  sig.returns(Errno::EUCLEAN)
  def untaint(); end

  sig.returns(Errno::EUCLEAN)
  def untrust(); end
end

class Errno::EUNATCH < SystemCallError
  sig.returns(Errno::EUNATCH)
  def clone(); end

  sig.returns(Errno::EUNATCH)
  def dup(); end

  sig.returns(Errno::EUNATCH)
  def freeze(); end

  sig.returns(Errno::EUNATCH)
  def taint(); end

  sig.returns(Errno::EUNATCH)
  def trust(); end

  sig.returns(Errno::EUNATCH)
  def untaint(); end

  sig.returns(Errno::EUNATCH)
  def untrust(); end
end

class Errno::EUSERS < SystemCallError
  sig.returns(Errno::EUSERS)
  def clone(); end

  sig.returns(Errno::EUSERS)
  def dup(); end

  sig.returns(Errno::EUSERS)
  def freeze(); end

  sig.returns(Errno::EUSERS)
  def taint(); end

  sig.returns(Errno::EUSERS)
  def trust(); end

  sig.returns(Errno::EUSERS)
  def untaint(); end

  sig.returns(Errno::EUSERS)
  def untrust(); end
end

class Errno::EXDEV < SystemCallError
  sig.returns(Errno::EXDEV)
  def clone(); end

  sig.returns(Errno::EXDEV)
  def dup(); end

  sig.returns(Errno::EXDEV)
  def freeze(); end

  sig.returns(Errno::EXDEV)
  def taint(); end

  sig.returns(Errno::EXDEV)
  def trust(); end

  sig.returns(Errno::EXDEV)
  def untaint(); end

  sig.returns(Errno::EXDEV)
  def untrust(); end
end

class Errno::EXFULL < SystemCallError
  sig.returns(Errno::EXFULL)
  def clone(); end

  sig.returns(Errno::EXFULL)
  def dup(); end

  sig.returns(Errno::EXFULL)
  def freeze(); end

  sig.returns(Errno::EXFULL)
  def taint(); end

  sig.returns(Errno::EXFULL)
  def trust(); end

  sig.returns(Errno::EXFULL)
  def untaint(); end

  sig.returns(Errno::EXFULL)
  def untrust(); end
end

class Errno::NOERROR < SystemCallError
  sig.returns(Errno::NOERROR)
  def clone(); end

  sig.returns(Errno::NOERROR)
  def dup(); end

  sig.returns(Errno::NOERROR)
  def freeze(); end

  sig.returns(Errno::NOERROR)
  def taint(); end

  sig.returns(Errno::NOERROR)
  def trust(); end

  sig.returns(Errno::NOERROR)
  def untaint(); end

  sig.returns(Errno::NOERROR)
  def untrust(); end
end

class Etc::Group < Struct
  sig.returns(Etc::Group)
  def clone(); end

  sig.returns(Etc::Group)
  def dup(); end

  sig.returns(Etc::Group)
  def freeze(); end

  sig.returns(Etc::Group)
  def taint(); end

  sig.returns(Etc::Group)
  def trust(); end

  sig.returns(Etc::Group)
  def untaint(); end

  sig.returns(Etc::Group)
  def untrust(); end
end

class Etc::Passwd < Struct
  sig.returns(Etc::Passwd)
  def clone(); end

  sig.returns(Etc::Passwd)
  def dup(); end

  sig.returns(Etc::Passwd)
  def freeze(); end

  sig.returns(Etc::Passwd)
  def taint(); end

  sig.returns(Etc::Passwd)
  def trust(); end

  sig.returns(Etc::Passwd)
  def untaint(); end

  sig.returns(Etc::Passwd)
  def untrust(); end
end

class Exception < Object
  sig.returns(Exception)
  def clone(); end

  sig.returns(Exception)
  def dup(); end

  sig.returns(Exception)
  def freeze(); end

  sig.returns(Exception)
  def taint(); end

  sig.returns(Exception)
  def trust(); end

  sig.returns(Exception)
  def untaint(); end

  sig.returns(Exception)
  def untrust(); end
end

class FalseClass < Object
  sig.returns(FalseClass)
  def clone(); end

  sig.returns(FalseClass)
  def dup(); end

  sig.returns(FalseClass)
  def freeze(); end

  sig.returns(FalseClass)
  def taint(); end

  sig.returns(FalseClass)
  def trust(); end

  sig.returns(FalseClass)
  def untaint(); end

  sig.returns(FalseClass)
  def untrust(); end
end

class Fiber < Object
  sig.returns(Fiber)
  def clone(); end

  sig.returns(Fiber)
  def dup(); end

  sig.returns(Fiber)
  def freeze(); end

  sig.returns(Fiber)
  def taint(); end

  sig.returns(Fiber)
  def trust(); end

  sig.returns(Fiber)
  def untaint(); end

  sig.returns(Fiber)
  def untrust(); end
end

class FiberError < StandardError
  sig.returns(FiberError)
  def clone(); end

  sig.returns(FiberError)
  def dup(); end

  sig.returns(FiberError)
  def freeze(); end

  sig.returns(FiberError)
  def taint(); end

  sig.returns(FiberError)
  def trust(); end

  sig.returns(FiberError)
  def untaint(); end

  sig.returns(FiberError)
  def untrust(); end
end

class File < IO
  sig(
      arg0: BasicObject,
  )
  .returns(File)
  def <<(arg0); end

  sig.returns(File)
  def binmode(); end

  sig.returns(File)
  def clone(); end

  sig.returns(File)
  def dup(); end

  sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(File)
  def each(sep=_, limit=_, &blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(File)
  def each_byte(&blk); end

  sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(File)
  def each_char(&blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(File)
  def each_codepoint(&blk); end

  sig.returns(File)
  def flush(); end

  sig.returns(File)
  def freeze(); end

  sig(
      ext_or_ext_int_enc: T.any(String, Encoding),
  )
  .returns(File)
  sig(
      ext_or_ext_int_enc: T.any(String, Encoding),
      int_enc: T.any(String, Encoding),
  )
  .returns(File)
  def set_encoding(ext_or_ext_int_enc=_, int_enc=_); end

  sig.returns(File)
  def taint(); end

  sig.returns(File)
  def to_io(); end

  sig.returns(File)
  def trust(); end

  sig.returns(File)
  def untaint(); end

  sig.returns(File)
  def untrust(); end
end

class File::Stat < Object
  sig.returns(File::Stat)
  def clone(); end

  sig.returns(File::Stat)
  def dup(); end

  sig.returns(File::Stat)
  def freeze(); end

  sig.returns(File::Stat)
  def taint(); end

  sig.returns(File::Stat)
  def trust(); end

  sig.returns(File::Stat)
  def untaint(); end

  sig.returns(File::Stat)
  def untrust(); end
end

class FileUtils::Entry_ < Object
  sig.returns(FileUtils::Entry_)
  def clone(); end

  sig.returns(FileUtils::Entry_)
  def dup(); end

  sig.returns(FileUtils::Entry_)
  def freeze(); end

  sig.returns(FileUtils::Entry_)
  def taint(); end

  sig.returns(FileUtils::Entry_)
  def trust(); end

  sig.returns(FileUtils::Entry_)
  def untaint(); end

  sig.returns(FileUtils::Entry_)
  def untrust(); end
end

class Float < Numeric
  sig.returns(Float)
  def clone(); end

  sig.returns(Float)
  def dup(); end

  sig.returns(Float)
  def freeze(); end

  sig.returns(Float)
  def taint(); end

  sig.returns(Float)
  def trust(); end

  sig.returns(Float)
  def untaint(); end

  sig.returns(Float)
  def untrust(); end
end

class FloatDomainError < RangeError
  sig.returns(FloatDomainError)
  def clone(); end

  sig.returns(FloatDomainError)
  def dup(); end

  sig.returns(FloatDomainError)
  def freeze(); end

  sig.returns(FloatDomainError)
  def taint(); end

  sig.returns(FloatDomainError)
  def trust(); end

  sig.returns(FloatDomainError)
  def untaint(); end

  sig.returns(FloatDomainError)
  def untrust(); end
end

class Gem::BasicSpecification < Object
  sig.returns(Gem::BasicSpecification)
  def clone(); end

  sig.returns(Gem::BasicSpecification)
  def dup(); end

  sig.returns(Gem::BasicSpecification)
  def freeze(); end

  sig.returns(Gem::BasicSpecification)
  def taint(); end

  sig.returns(Gem::BasicSpecification)
  def trust(); end

  sig.returns(Gem::BasicSpecification)
  def untaint(); end

  sig.returns(Gem::BasicSpecification)
  def untrust(); end
end

class Gem::CommandLineError < Gem::Exception
  sig.returns(Gem::CommandLineError)
  def clone(); end

  sig.returns(Gem::CommandLineError)
  def dup(); end

  sig.returns(Gem::CommandLineError)
  def freeze(); end

  sig.returns(Gem::CommandLineError)
  def taint(); end

  sig.returns(Gem::CommandLineError)
  def trust(); end

  sig.returns(Gem::CommandLineError)
  def untaint(); end

  sig.returns(Gem::CommandLineError)
  def untrust(); end
end

class Gem::ConflictError < Gem::LoadError
  sig.returns(Gem::ConflictError)
  def clone(); end

  sig.returns(Gem::ConflictError)
  def dup(); end

  sig.returns(Gem::ConflictError)
  def freeze(); end

  sig.returns(Gem::ConflictError)
  def taint(); end

  sig.returns(Gem::ConflictError)
  def trust(); end

  sig.returns(Gem::ConflictError)
  def untaint(); end

  sig.returns(Gem::ConflictError)
  def untrust(); end
end

class Gem::Dependency < Object
  sig.returns(Gem::Dependency)
  def clone(); end

  sig.returns(Gem::Dependency)
  def dup(); end

  sig.returns(Gem::Dependency)
  def freeze(); end

  sig.returns(Gem::Dependency)
  def taint(); end

  sig.returns(Gem::Dependency)
  def trust(); end

  sig.returns(Gem::Dependency)
  def untaint(); end

  sig.returns(Gem::Dependency)
  def untrust(); end
end

class Gem::DependencyError < Gem::Exception
  sig.returns(Gem::DependencyError)
  def clone(); end

  sig.returns(Gem::DependencyError)
  def dup(); end

  sig.returns(Gem::DependencyError)
  def freeze(); end

  sig.returns(Gem::DependencyError)
  def taint(); end

  sig.returns(Gem::DependencyError)
  def trust(); end

  sig.returns(Gem::DependencyError)
  def untaint(); end

  sig.returns(Gem::DependencyError)
  def untrust(); end
end

class Gem::DependencyRemovalException < Gem::Exception
  sig.returns(Gem::DependencyRemovalException)
  def clone(); end

  sig.returns(Gem::DependencyRemovalException)
  def dup(); end

  sig.returns(Gem::DependencyRemovalException)
  def freeze(); end

  sig.returns(Gem::DependencyRemovalException)
  def taint(); end

  sig.returns(Gem::DependencyRemovalException)
  def trust(); end

  sig.returns(Gem::DependencyRemovalException)
  def untaint(); end

  sig.returns(Gem::DependencyRemovalException)
  def untrust(); end
end

class Gem::DependencyResolutionError < Gem::DependencyError
  sig.returns(Gem::DependencyResolutionError)
  def clone(); end

  sig.returns(Gem::DependencyResolutionError)
  def dup(); end

  sig.returns(Gem::DependencyResolutionError)
  def freeze(); end

  sig.returns(Gem::DependencyResolutionError)
  def taint(); end

  sig.returns(Gem::DependencyResolutionError)
  def trust(); end

  sig.returns(Gem::DependencyResolutionError)
  def untaint(); end

  sig.returns(Gem::DependencyResolutionError)
  def untrust(); end
end

class Gem::DocumentError < Gem::Exception
  sig.returns(Gem::DocumentError)
  def clone(); end

  sig.returns(Gem::DocumentError)
  def dup(); end

  sig.returns(Gem::DocumentError)
  def freeze(); end

  sig.returns(Gem::DocumentError)
  def taint(); end

  sig.returns(Gem::DocumentError)
  def trust(); end

  sig.returns(Gem::DocumentError)
  def untaint(); end

  sig.returns(Gem::DocumentError)
  def untrust(); end
end

class Gem::EndOfYAMLException < Gem::Exception
  sig.returns(Gem::EndOfYAMLException)
  def clone(); end

  sig.returns(Gem::EndOfYAMLException)
  def dup(); end

  sig.returns(Gem::EndOfYAMLException)
  def freeze(); end

  sig.returns(Gem::EndOfYAMLException)
  def taint(); end

  sig.returns(Gem::EndOfYAMLException)
  def trust(); end

  sig.returns(Gem::EndOfYAMLException)
  def untaint(); end

  sig.returns(Gem::EndOfYAMLException)
  def untrust(); end
end

class Gem::ErrorReason < Object
  sig.returns(Gem::ErrorReason)
  def clone(); end

  sig.returns(Gem::ErrorReason)
  def dup(); end

  sig.returns(Gem::ErrorReason)
  def freeze(); end

  sig.returns(Gem::ErrorReason)
  def taint(); end

  sig.returns(Gem::ErrorReason)
  def trust(); end

  sig.returns(Gem::ErrorReason)
  def untaint(); end

  sig.returns(Gem::ErrorReason)
  def untrust(); end
end

class Gem::Exception < RuntimeError
  sig.returns(Gem::Exception)
  def clone(); end

  sig.returns(Gem::Exception)
  def dup(); end

  sig.returns(Gem::Exception)
  def freeze(); end

  sig.returns(Gem::Exception)
  def taint(); end

  sig.returns(Gem::Exception)
  def trust(); end

  sig.returns(Gem::Exception)
  def untaint(); end

  sig.returns(Gem::Exception)
  def untrust(); end
end

class Gem::FilePermissionError < Gem::Exception
  sig.returns(Gem::FilePermissionError)
  def clone(); end

  sig.returns(Gem::FilePermissionError)
  def dup(); end

  sig.returns(Gem::FilePermissionError)
  def freeze(); end

  sig.returns(Gem::FilePermissionError)
  def taint(); end

  sig.returns(Gem::FilePermissionError)
  def trust(); end

  sig.returns(Gem::FilePermissionError)
  def untaint(); end

  sig.returns(Gem::FilePermissionError)
  def untrust(); end
end

class Gem::FormatException < Gem::Exception
  sig.returns(Gem::FormatException)
  def clone(); end

  sig.returns(Gem::FormatException)
  def dup(); end

  sig.returns(Gem::FormatException)
  def freeze(); end

  sig.returns(Gem::FormatException)
  def taint(); end

  sig.returns(Gem::FormatException)
  def trust(); end

  sig.returns(Gem::FormatException)
  def untaint(); end

  sig.returns(Gem::FormatException)
  def untrust(); end
end

class Gem::GemNotFoundException < Gem::Exception
  sig.returns(Gem::GemNotFoundException)
  def clone(); end

  sig.returns(Gem::GemNotFoundException)
  def dup(); end

  sig.returns(Gem::GemNotFoundException)
  def freeze(); end

  sig.returns(Gem::GemNotFoundException)
  def taint(); end

  sig.returns(Gem::GemNotFoundException)
  def trust(); end

  sig.returns(Gem::GemNotFoundException)
  def untaint(); end

  sig.returns(Gem::GemNotFoundException)
  def untrust(); end
end

class Gem::GemNotInHomeException < Gem::Exception
  sig.returns(Gem::GemNotInHomeException)
  def clone(); end

  sig.returns(Gem::GemNotInHomeException)
  def dup(); end

  sig.returns(Gem::GemNotInHomeException)
  def freeze(); end

  sig.returns(Gem::GemNotInHomeException)
  def taint(); end

  sig.returns(Gem::GemNotInHomeException)
  def trust(); end

  sig.returns(Gem::GemNotInHomeException)
  def untaint(); end

  sig.returns(Gem::GemNotInHomeException)
  def untrust(); end
end

class Gem::ImpossibleDependenciesError < Gem::Exception
  sig.returns(Gem::ImpossibleDependenciesError)
  def clone(); end

  sig.returns(Gem::ImpossibleDependenciesError)
  def dup(); end

  sig.returns(Gem::ImpossibleDependenciesError)
  def freeze(); end

  sig.returns(Gem::ImpossibleDependenciesError)
  def taint(); end

  sig.returns(Gem::ImpossibleDependenciesError)
  def trust(); end

  sig.returns(Gem::ImpossibleDependenciesError)
  def untaint(); end

  sig.returns(Gem::ImpossibleDependenciesError)
  def untrust(); end
end

class Gem::InstallError < Gem::Exception
  sig.returns(Gem::InstallError)
  def clone(); end

  sig.returns(Gem::InstallError)
  def dup(); end

  sig.returns(Gem::InstallError)
  def freeze(); end

  sig.returns(Gem::InstallError)
  def taint(); end

  sig.returns(Gem::InstallError)
  def trust(); end

  sig.returns(Gem::InstallError)
  def untaint(); end

  sig.returns(Gem::InstallError)
  def untrust(); end
end

class Gem::InvalidSpecificationException < Gem::Exception
  sig.returns(Gem::InvalidSpecificationException)
  def clone(); end

  sig.returns(Gem::InvalidSpecificationException)
  def dup(); end

  sig.returns(Gem::InvalidSpecificationException)
  def freeze(); end

  sig.returns(Gem::InvalidSpecificationException)
  def taint(); end

  sig.returns(Gem::InvalidSpecificationException)
  def trust(); end

  sig.returns(Gem::InvalidSpecificationException)
  def untaint(); end

  sig.returns(Gem::InvalidSpecificationException)
  def untrust(); end
end

class Gem::List < Object
  sig.returns(Gem::List[Elem])
  def clone(); end

  sig.returns(Gem::List[Elem])
  def dup(); end

  sig.returns(Gem::List[Elem])
  def freeze(); end

  sig.returns(Gem::List[Elem])
  def taint(); end

  sig.returns(Gem::List[Elem])
  def trust(); end

  sig.returns(Gem::List[Elem])
  def untaint(); end

  sig.returns(Gem::List[Elem])
  def untrust(); end
end

class Gem::LoadError < LoadError
  sig.returns(Gem::LoadError)
  def clone(); end

  sig.returns(Gem::LoadError)
  def dup(); end

  sig.returns(Gem::LoadError)
  def freeze(); end

  sig.returns(Gem::LoadError)
  def taint(); end

  sig.returns(Gem::LoadError)
  def trust(); end

  sig.returns(Gem::LoadError)
  def untaint(); end

  sig.returns(Gem::LoadError)
  def untrust(); end
end

class Gem::MissingSpecError < Gem::LoadError
  sig.returns(Gem::MissingSpecError)
  def clone(); end

  sig.returns(Gem::MissingSpecError)
  def dup(); end

  sig.returns(Gem::MissingSpecError)
  def freeze(); end

  sig.returns(Gem::MissingSpecError)
  def taint(); end

  sig.returns(Gem::MissingSpecError)
  def trust(); end

  sig.returns(Gem::MissingSpecError)
  def untaint(); end

  sig.returns(Gem::MissingSpecError)
  def untrust(); end
end

class Gem::MissingSpecVersionError < Gem::MissingSpecError
  sig.returns(Gem::MissingSpecVersionError)
  def clone(); end

  sig.returns(Gem::MissingSpecVersionError)
  def dup(); end

  sig.returns(Gem::MissingSpecVersionError)
  def freeze(); end

  sig.returns(Gem::MissingSpecVersionError)
  def taint(); end

  sig.returns(Gem::MissingSpecVersionError)
  def trust(); end

  sig.returns(Gem::MissingSpecVersionError)
  def untaint(); end

  sig.returns(Gem::MissingSpecVersionError)
  def untrust(); end
end

class Gem::OperationNotSupportedError < Gem::Exception
  sig.returns(Gem::OperationNotSupportedError)
  def clone(); end

  sig.returns(Gem::OperationNotSupportedError)
  def dup(); end

  sig.returns(Gem::OperationNotSupportedError)
  def freeze(); end

  sig.returns(Gem::OperationNotSupportedError)
  def taint(); end

  sig.returns(Gem::OperationNotSupportedError)
  def trust(); end

  sig.returns(Gem::OperationNotSupportedError)
  def untaint(); end

  sig.returns(Gem::OperationNotSupportedError)
  def untrust(); end
end

class Gem::PathSupport < Object
  sig.returns(Gem::PathSupport)
  def clone(); end

  sig.returns(Gem::PathSupport)
  def dup(); end

  sig.returns(Gem::PathSupport)
  def freeze(); end

  sig.returns(Gem::PathSupport)
  def taint(); end

  sig.returns(Gem::PathSupport)
  def trust(); end

  sig.returns(Gem::PathSupport)
  def untaint(); end

  sig.returns(Gem::PathSupport)
  def untrust(); end
end

class Gem::Platform < Object
  sig.returns(Gem::Platform)
  def clone(); end

  sig.returns(Gem::Platform)
  def dup(); end

  sig.returns(Gem::Platform)
  def freeze(); end

  sig.returns(Gem::Platform)
  def taint(); end

  sig.returns(Gem::Platform)
  def trust(); end

  sig.returns(Gem::Platform)
  def untaint(); end

  sig.returns(Gem::Platform)
  def untrust(); end
end

class Gem::PlatformMismatch < Gem::ErrorReason
  sig.returns(Gem::PlatformMismatch)
  def clone(); end

  sig.returns(Gem::PlatformMismatch)
  def dup(); end

  sig.returns(Gem::PlatformMismatch)
  def freeze(); end

  sig.returns(Gem::PlatformMismatch)
  def taint(); end

  sig.returns(Gem::PlatformMismatch)
  def trust(); end

  sig.returns(Gem::PlatformMismatch)
  def untaint(); end

  sig.returns(Gem::PlatformMismatch)
  def untrust(); end
end

class Gem::RemoteError < Gem::Exception
  sig.returns(Gem::RemoteError)
  def clone(); end

  sig.returns(Gem::RemoteError)
  def dup(); end

  sig.returns(Gem::RemoteError)
  def freeze(); end

  sig.returns(Gem::RemoteError)
  def taint(); end

  sig.returns(Gem::RemoteError)
  def trust(); end

  sig.returns(Gem::RemoteError)
  def untaint(); end

  sig.returns(Gem::RemoteError)
  def untrust(); end
end

class Gem::RemoteInstallationCancelled < Gem::Exception
  sig.returns(Gem::RemoteInstallationCancelled)
  def clone(); end

  sig.returns(Gem::RemoteInstallationCancelled)
  def dup(); end

  sig.returns(Gem::RemoteInstallationCancelled)
  def freeze(); end

  sig.returns(Gem::RemoteInstallationCancelled)
  def taint(); end

  sig.returns(Gem::RemoteInstallationCancelled)
  def trust(); end

  sig.returns(Gem::RemoteInstallationCancelled)
  def untaint(); end

  sig.returns(Gem::RemoteInstallationCancelled)
  def untrust(); end
end

class Gem::RemoteInstallationSkipped < Gem::Exception
  sig.returns(Gem::RemoteInstallationSkipped)
  def clone(); end

  sig.returns(Gem::RemoteInstallationSkipped)
  def dup(); end

  sig.returns(Gem::RemoteInstallationSkipped)
  def freeze(); end

  sig.returns(Gem::RemoteInstallationSkipped)
  def taint(); end

  sig.returns(Gem::RemoteInstallationSkipped)
  def trust(); end

  sig.returns(Gem::RemoteInstallationSkipped)
  def untaint(); end

  sig.returns(Gem::RemoteInstallationSkipped)
  def untrust(); end
end

class Gem::RemoteSourceException < Gem::Exception
  sig.returns(Gem::RemoteSourceException)
  def clone(); end

  sig.returns(Gem::RemoteSourceException)
  def dup(); end

  sig.returns(Gem::RemoteSourceException)
  def freeze(); end

  sig.returns(Gem::RemoteSourceException)
  def taint(); end

  sig.returns(Gem::RemoteSourceException)
  def trust(); end

  sig.returns(Gem::RemoteSourceException)
  def untaint(); end

  sig.returns(Gem::RemoteSourceException)
  def untrust(); end
end

class Gem::Requirement < Object
  sig.returns(Gem::Requirement)
  def clone(); end

  sig.returns(Gem::Requirement)
  def dup(); end

  sig.returns(Gem::Requirement)
  def freeze(); end

  sig.returns(Gem::Requirement)
  def taint(); end

  sig.returns(Gem::Requirement)
  def trust(); end

  sig.returns(Gem::Requirement)
  def untaint(); end

  sig.returns(Gem::Requirement)
  def untrust(); end
end

class Gem::Requirement::BadRequirementError < ArgumentError
  sig.returns(Gem::Requirement::BadRequirementError)
  def clone(); end

  sig.returns(Gem::Requirement::BadRequirementError)
  def dup(); end

  sig.returns(Gem::Requirement::BadRequirementError)
  def freeze(); end

  sig.returns(Gem::Requirement::BadRequirementError)
  def taint(); end

  sig.returns(Gem::Requirement::BadRequirementError)
  def trust(); end

  sig.returns(Gem::Requirement::BadRequirementError)
  def untaint(); end

  sig.returns(Gem::Requirement::BadRequirementError)
  def untrust(); end
end

class Gem::RubyVersionMismatch < Gem::Exception
  sig.returns(Gem::RubyVersionMismatch)
  def clone(); end

  sig.returns(Gem::RubyVersionMismatch)
  def dup(); end

  sig.returns(Gem::RubyVersionMismatch)
  def freeze(); end

  sig.returns(Gem::RubyVersionMismatch)
  def taint(); end

  sig.returns(Gem::RubyVersionMismatch)
  def trust(); end

  sig.returns(Gem::RubyVersionMismatch)
  def untaint(); end

  sig.returns(Gem::RubyVersionMismatch)
  def untrust(); end
end

class Gem::SourceFetchProblem < Gem::ErrorReason
  sig.returns(Gem::SourceFetchProblem)
  def clone(); end

  sig.returns(Gem::SourceFetchProblem)
  def dup(); end

  sig.returns(Gem::SourceFetchProblem)
  def freeze(); end

  sig.returns(Gem::SourceFetchProblem)
  def taint(); end

  sig.returns(Gem::SourceFetchProblem)
  def trust(); end

  sig.returns(Gem::SourceFetchProblem)
  def untaint(); end

  sig.returns(Gem::SourceFetchProblem)
  def untrust(); end
end

class Gem::SpecificGemNotFoundException < Gem::GemNotFoundException
  sig.returns(Gem::SpecificGemNotFoundException)
  def clone(); end

  sig.returns(Gem::SpecificGemNotFoundException)
  def dup(); end

  sig.returns(Gem::SpecificGemNotFoundException)
  def freeze(); end

  sig.returns(Gem::SpecificGemNotFoundException)
  def taint(); end

  sig.returns(Gem::SpecificGemNotFoundException)
  def trust(); end

  sig.returns(Gem::SpecificGemNotFoundException)
  def untaint(); end

  sig.returns(Gem::SpecificGemNotFoundException)
  def untrust(); end
end

class Gem::Specification < Gem::BasicSpecification
  sig.returns(Gem::Specification)
  def clone(); end

  sig.returns(Gem::Specification)
  def dup(); end

  sig.returns(Gem::Specification)
  def freeze(); end

  sig.returns(Gem::Specification)
  def taint(); end

  sig.returns(Gem::Specification)
  def trust(); end

  sig.returns(Gem::Specification)
  def untaint(); end

  sig.returns(Gem::Specification)
  def untrust(); end
end

class Gem::StubSpecification < Gem::BasicSpecification
  sig.returns(Gem::StubSpecification)
  def clone(); end

  sig.returns(Gem::StubSpecification)
  def dup(); end

  sig.returns(Gem::StubSpecification)
  def freeze(); end

  sig.returns(Gem::StubSpecification)
  def taint(); end

  sig.returns(Gem::StubSpecification)
  def trust(); end

  sig.returns(Gem::StubSpecification)
  def untaint(); end

  sig.returns(Gem::StubSpecification)
  def untrust(); end
end

class Gem::StubSpecification::StubLine < Object
  sig.returns(Gem::StubSpecification::StubLine)
  def clone(); end

  sig.returns(Gem::StubSpecification::StubLine)
  def dup(); end

  sig.returns(Gem::StubSpecification::StubLine)
  def freeze(); end

  sig.returns(Gem::StubSpecification::StubLine)
  def taint(); end

  sig.returns(Gem::StubSpecification::StubLine)
  def trust(); end

  sig.returns(Gem::StubSpecification::StubLine)
  def untaint(); end

  sig.returns(Gem::StubSpecification::StubLine)
  def untrust(); end
end

class Gem::SystemExitException < SystemExit
  sig.returns(Gem::SystemExitException)
  def clone(); end

  sig.returns(Gem::SystemExitException)
  def dup(); end

  sig.returns(Gem::SystemExitException)
  def freeze(); end

  sig.returns(Gem::SystemExitException)
  def taint(); end

  sig.returns(Gem::SystemExitException)
  def trust(); end

  sig.returns(Gem::SystemExitException)
  def untaint(); end

  sig.returns(Gem::SystemExitException)
  def untrust(); end
end

class Gem::UnsatisfiableDependencyError < Gem::DependencyError
  sig.returns(Gem::UnsatisfiableDependencyError)
  def clone(); end

  sig.returns(Gem::UnsatisfiableDependencyError)
  def dup(); end

  sig.returns(Gem::UnsatisfiableDependencyError)
  def freeze(); end

  sig.returns(Gem::UnsatisfiableDependencyError)
  def taint(); end

  sig.returns(Gem::UnsatisfiableDependencyError)
  def trust(); end

  sig.returns(Gem::UnsatisfiableDependencyError)
  def untaint(); end

  sig.returns(Gem::UnsatisfiableDependencyError)
  def untrust(); end
end

class Gem::VerificationError < Gem::Exception
  sig.returns(Gem::VerificationError)
  def clone(); end

  sig.returns(Gem::VerificationError)
  def dup(); end

  sig.returns(Gem::VerificationError)
  def freeze(); end

  sig.returns(Gem::VerificationError)
  def taint(); end

  sig.returns(Gem::VerificationError)
  def trust(); end

  sig.returns(Gem::VerificationError)
  def untaint(); end

  sig.returns(Gem::VerificationError)
  def untrust(); end
end

class Gem::Version < Object
  sig.returns(Gem::Version)
  def clone(); end

  sig.returns(Gem::Version)
  def dup(); end

  sig.returns(Gem::Version)
  def freeze(); end

  sig.returns(Gem::Version)
  def taint(); end

  sig.returns(Gem::Version)
  def trust(); end

  sig.returns(Gem::Version)
  def untaint(); end

  sig.returns(Gem::Version)
  def untrust(); end
end

class Hash < Object
  sig.returns(T::Hash[K, V])
  def clone(); end

  sig.returns(T::Hash[K, V])
  def dup(); end

  sig.returns(T::Hash[K, V])
  def freeze(); end

  sig.returns(T::Hash[K, V])
  def taint(); end

  sig.returns(T::Hash[K, V])
  def trust(); end

  sig.returns(T::Hash[K, V])
  def untaint(); end

  sig.returns(T::Hash[K, V])
  def untrust(); end
end

class IO < Object
  sig.returns(IO)
  def clone(); end

  sig.returns(IO)
  def dup(); end

  sig.returns(IO)
  def freeze(); end

  sig.returns(IO)
  def taint(); end

  sig.returns(IO)
  def trust(); end

  sig.returns(IO)
  def untaint(); end

  sig.returns(IO)
  def untrust(); end
end

class IO::EAGAINWaitReadable < Errno::EAGAIN
  sig.returns(IO::EAGAINWaitReadable)
  def clone(); end

  sig.returns(IO::EAGAINWaitReadable)
  def dup(); end

  sig.returns(IO::EAGAINWaitReadable)
  def freeze(); end

  sig.returns(IO::EAGAINWaitReadable)
  def taint(); end

  sig.returns(IO::EAGAINWaitReadable)
  def trust(); end

  sig.returns(IO::EAGAINWaitReadable)
  def untaint(); end

  sig.returns(IO::EAGAINWaitReadable)
  def untrust(); end
end

class IO::EAGAINWaitWritable < Errno::EAGAIN
  sig.returns(IO::EAGAINWaitWritable)
  def clone(); end

  sig.returns(IO::EAGAINWaitWritable)
  def dup(); end

  sig.returns(IO::EAGAINWaitWritable)
  def freeze(); end

  sig.returns(IO::EAGAINWaitWritable)
  def taint(); end

  sig.returns(IO::EAGAINWaitWritable)
  def trust(); end

  sig.returns(IO::EAGAINWaitWritable)
  def untaint(); end

  sig.returns(IO::EAGAINWaitWritable)
  def untrust(); end
end

class IO::EINPROGRESSWaitReadable < Errno::EINPROGRESS
  sig.returns(IO::EINPROGRESSWaitReadable)
  def clone(); end

  sig.returns(IO::EINPROGRESSWaitReadable)
  def dup(); end

  sig.returns(IO::EINPROGRESSWaitReadable)
  def freeze(); end

  sig.returns(IO::EINPROGRESSWaitReadable)
  def taint(); end

  sig.returns(IO::EINPROGRESSWaitReadable)
  def trust(); end

  sig.returns(IO::EINPROGRESSWaitReadable)
  def untaint(); end

  sig.returns(IO::EINPROGRESSWaitReadable)
  def untrust(); end
end

class IO::EINPROGRESSWaitWritable < Errno::EINPROGRESS
  sig.returns(IO::EINPROGRESSWaitWritable)
  def clone(); end

  sig.returns(IO::EINPROGRESSWaitWritable)
  def dup(); end

  sig.returns(IO::EINPROGRESSWaitWritable)
  def freeze(); end

  sig.returns(IO::EINPROGRESSWaitWritable)
  def taint(); end

  sig.returns(IO::EINPROGRESSWaitWritable)
  def trust(); end

  sig.returns(IO::EINPROGRESSWaitWritable)
  def untaint(); end

  sig.returns(IO::EINPROGRESSWaitWritable)
  def untrust(); end
end

class IOError < StandardError
  sig.returns(IOError)
  def clone(); end

  sig.returns(IOError)
  def dup(); end

  sig.returns(IOError)
  def freeze(); end

  sig.returns(IOError)
  def taint(); end

  sig.returns(IOError)
  def trust(); end

  sig.returns(IOError)
  def untaint(); end

  sig.returns(IOError)
  def untrust(); end
end

class IndexError < StandardError
  sig.returns(IndexError)
  def clone(); end

  sig.returns(IndexError)
  def dup(); end

  sig.returns(IndexError)
  def freeze(); end

  sig.returns(IndexError)
  def taint(); end

  sig.returns(IndexError)
  def trust(); end

  sig.returns(IndexError)
  def untaint(); end

  sig.returns(IndexError)
  def untrust(); end
end

class Integer < Numeric
  sig.returns(Integer)
  def clone(); end

  sig.returns(Integer)
  def dup(); end

  sig.returns(Integer)
  def freeze(); end

  sig.returns(Integer)
  def taint(); end

  sig.returns(Integer)
  def trust(); end

  sig.returns(Integer)
  def untaint(); end

  sig.returns(Integer)
  def untrust(); end
end

class Interrupt < SignalException
  sig.returns(Interrupt)
  def clone(); end

  sig.returns(Interrupt)
  def dup(); end

  sig.returns(Interrupt)
  def freeze(); end

  sig.returns(Interrupt)
  def taint(); end

  sig.returns(Interrupt)
  def trust(); end

  sig.returns(Interrupt)
  def untaint(); end

  sig.returns(Interrupt)
  def untrust(); end
end

class KeyError < IndexError
  sig.returns(KeyError)
  def clone(); end

  sig.returns(KeyError)
  def dup(); end

  sig.returns(KeyError)
  def freeze(); end

  sig.returns(KeyError)
  def taint(); end

  sig.returns(KeyError)
  def trust(); end

  sig.returns(KeyError)
  def untaint(); end

  sig.returns(KeyError)
  def untrust(); end
end

class LoadError < ScriptError
  sig.returns(LoadError)
  def clone(); end

  sig.returns(LoadError)
  def dup(); end

  sig.returns(LoadError)
  def freeze(); end

  sig.returns(LoadError)
  def taint(); end

  sig.returns(LoadError)
  def trust(); end

  sig.returns(LoadError)
  def untaint(); end

  sig.returns(LoadError)
  def untrust(); end
end

class LocalJumpError < StandardError
  sig.returns(LocalJumpError)
  def clone(); end

  sig.returns(LocalJumpError)
  def dup(); end

  sig.returns(LocalJumpError)
  def freeze(); end

  sig.returns(LocalJumpError)
  def taint(); end

  sig.returns(LocalJumpError)
  def trust(); end

  sig.returns(LocalJumpError)
  def untaint(); end

  sig.returns(LocalJumpError)
  def untrust(); end
end

class MatchData < Object
  sig.returns(MatchData)
  def clone(); end

  sig.returns(MatchData)
  def dup(); end

  sig.returns(MatchData)
  def freeze(); end

  sig.returns(MatchData)
  def taint(); end

  sig.returns(MatchData)
  def trust(); end

  sig.returns(MatchData)
  def untaint(); end

  sig.returns(MatchData)
  def untrust(); end
end

class Math::DomainError < StandardError
  sig.returns(Math::DomainError)
  def clone(); end

  sig.returns(Math::DomainError)
  def dup(); end

  sig.returns(Math::DomainError)
  def freeze(); end

  sig.returns(Math::DomainError)
  def taint(); end

  sig.returns(Math::DomainError)
  def trust(); end

  sig.returns(Math::DomainError)
  def untaint(); end

  sig.returns(Math::DomainError)
  def untrust(); end
end

class Method < Object
  sig.returns(Method)
  def clone(); end

  sig.returns(Method)
  def dup(); end

  sig.returns(Method)
  def freeze(); end

  sig.returns(Method)
  def taint(); end

  sig.returns(Method)
  def trust(); end

  sig.returns(Method)
  def untaint(); end

  sig.returns(Method)
  def untrust(); end
end

class Module < Object
  sig.returns(Module)
  def clone(); end

  sig.returns(Module)
  def dup(); end

  sig.returns(Module)
  def freeze(); end

  sig.returns(Module)
  def taint(); end

  sig.returns(Module)
  def trust(); end

  sig.returns(Module)
  def untaint(); end

  sig.returns(Module)
  def untrust(); end
end

class Monitor < Object
  sig.returns(Monitor)
  def clone(); end

  sig.returns(Monitor)
  def dup(); end

  sig.returns(Monitor)
  def freeze(); end

  sig.returns(Monitor)
  def taint(); end

  sig.returns(Monitor)
  def trust(); end

  sig.returns(Monitor)
  def untaint(); end

  sig.returns(Monitor)
  def untrust(); end
end

class MonitorMixin::ConditionVariable < Object
  sig.returns(MonitorMixin::ConditionVariable)
  def clone(); end

  sig.returns(MonitorMixin::ConditionVariable)
  def dup(); end

  sig.returns(MonitorMixin::ConditionVariable)
  def freeze(); end

  sig.returns(MonitorMixin::ConditionVariable)
  def taint(); end

  sig.returns(MonitorMixin::ConditionVariable)
  def trust(); end

  sig.returns(MonitorMixin::ConditionVariable)
  def untaint(); end

  sig.returns(MonitorMixin::ConditionVariable)
  def untrust(); end
end

class MonitorMixin::ConditionVariable::Timeout < Exception
  sig.returns(MonitorMixin::ConditionVariable::Timeout)
  def clone(); end

  sig.returns(MonitorMixin::ConditionVariable::Timeout)
  def dup(); end

  sig.returns(MonitorMixin::ConditionVariable::Timeout)
  def freeze(); end

  sig.returns(MonitorMixin::ConditionVariable::Timeout)
  def taint(); end

  sig.returns(MonitorMixin::ConditionVariable::Timeout)
  def trust(); end

  sig.returns(MonitorMixin::ConditionVariable::Timeout)
  def untaint(); end

  sig.returns(MonitorMixin::ConditionVariable::Timeout)
  def untrust(); end
end

class NameError < StandardError
  sig.returns(NameError)
  def clone(); end

  sig.returns(NameError)
  def dup(); end

  sig.returns(NameError)
  def freeze(); end

  sig.returns(NameError)
  def taint(); end

  sig.returns(NameError)
  def trust(); end

  sig.returns(NameError)
  def untaint(); end

  sig.returns(NameError)
  def untrust(); end
end

class NilClass < Object
  sig.returns(NilClass)
  def clone(); end

  sig.returns(NilClass)
  def dup(); end

  sig.returns(NilClass)
  def freeze(); end

  sig.returns(NilClass)
  def taint(); end

  sig.returns(NilClass)
  def trust(); end

  sig.returns(NilClass)
  def untaint(); end

  sig.returns(NilClass)
  def untrust(); end
end

class NoMemoryError < Exception
  sig.returns(NoMemoryError)
  def clone(); end

  sig.returns(NoMemoryError)
  def dup(); end

  sig.returns(NoMemoryError)
  def freeze(); end

  sig.returns(NoMemoryError)
  def taint(); end

  sig.returns(NoMemoryError)
  def trust(); end

  sig.returns(NoMemoryError)
  def untaint(); end

  sig.returns(NoMemoryError)
  def untrust(); end
end

class NoMethodError < NameError
  sig.returns(NoMethodError)
  def clone(); end

  sig.returns(NoMethodError)
  def dup(); end

  sig.returns(NoMethodError)
  def freeze(); end

  sig.returns(NoMethodError)
  def taint(); end

  sig.returns(NoMethodError)
  def trust(); end

  sig.returns(NoMethodError)
  def untaint(); end

  sig.returns(NoMethodError)
  def untrust(); end
end

class NotImplementedError < ScriptError
  sig.returns(NotImplementedError)
  def clone(); end

  sig.returns(NotImplementedError)
  def dup(); end

  sig.returns(NotImplementedError)
  def freeze(); end

  sig.returns(NotImplementedError)
  def taint(); end

  sig.returns(NotImplementedError)
  def trust(); end

  sig.returns(NotImplementedError)
  def untaint(); end

  sig.returns(NotImplementedError)
  def untrust(); end
end

class Numeric < Object
  sig.returns(Numeric)
  def clone(); end

  sig.returns(Numeric)
  def dup(); end

  sig.returns(Numeric)
  def freeze(); end

  sig.returns(Numeric)
  def taint(); end

  sig.returns(Numeric)
  def trust(); end

  sig.returns(Numeric)
  def untaint(); end

  sig.returns(Numeric)
  def untrust(); end
end

class Object < BasicObject
  sig.returns(Object)
  def clone(); end

  sig.returns(Object)
  def dup(); end

  sig.returns(Object)
  def freeze(); end

  sig.returns(Object)
  def taint(); end

  sig.returns(Object)
  def trust(); end

  sig.returns(Object)
  def untaint(); end

  sig.returns(Object)
  def untrust(); end
end

class ObjectSpace::WeakMap < Object
  sig.returns(ObjectSpace::WeakMap[Elem])
  def clone(); end

  sig.returns(ObjectSpace::WeakMap[Elem])
  def dup(); end

  sig.returns(ObjectSpace::WeakMap[Elem])
  def freeze(); end

  sig.returns(ObjectSpace::WeakMap[Elem])
  def taint(); end

  sig.returns(ObjectSpace::WeakMap[Elem])
  def trust(); end

  sig.returns(ObjectSpace::WeakMap[Elem])
  def untaint(); end

  sig.returns(ObjectSpace::WeakMap[Elem])
  def untrust(); end
end

class Pathname < Object
  sig.returns(Pathname)
  def clone(); end

  sig.returns(Pathname)
  def dup(); end

  sig.returns(Pathname)
  def freeze(); end

  sig.returns(Pathname)
  def taint(); end

  sig.returns(Pathname)
  def trust(); end

  sig.returns(Pathname)
  def untaint(); end

  sig.returns(Pathname)
  def untrust(); end
end

class Proc < Object
  sig.returns(Proc)
  def clone(); end

  sig.returns(Proc)
  def dup(); end

  sig.returns(Proc)
  def freeze(); end

  sig.returns(Proc)
  def taint(); end

  sig.returns(Proc)
  def trust(); end

  sig.returns(Proc)
  def untaint(); end

  sig.returns(Proc)
  def untrust(); end
end

class Process::Status < Object
  sig.returns(Process::Status)
  def clone(); end

  sig.returns(Process::Status)
  def dup(); end

  sig.returns(Process::Status)
  def freeze(); end

  sig.returns(Process::Status)
  def taint(); end

  sig.returns(Process::Status)
  def trust(); end

  sig.returns(Process::Status)
  def untaint(); end

  sig.returns(Process::Status)
  def untrust(); end
end

class Process::Tms < Struct
  sig.returns(Process::Tms)
  def clone(); end

  sig.returns(Process::Tms)
  def dup(); end

  sig.returns(Process::Tms)
  def freeze(); end

  sig.returns(Process::Tms)
  def taint(); end

  sig.returns(Process::Tms)
  def trust(); end

  sig.returns(Process::Tms)
  def untaint(); end

  sig.returns(Process::Tms)
  def untrust(); end
end

class Process::Waiter < Thread
  sig.returns(Process::Waiter)
  def clone(); end

  sig.returns(Process::Waiter)
  def dup(); end

  sig.returns(Process::Waiter)
  def freeze(); end

  sig.returns(Process::Waiter)
  def taint(); end

  sig.returns(Process::Waiter)
  def trust(); end

  sig.returns(Process::Waiter)
  def untaint(); end

  sig.returns(Process::Waiter)
  def untrust(); end
end

class Random < Object
  sig.returns(Random)
  def clone(); end

  sig.returns(Random)
  def dup(); end

  sig.returns(Random)
  def freeze(); end

  sig.returns(Random)
  def taint(); end

  sig.returns(Random)
  def trust(); end

  sig.returns(Random)
  def untaint(); end

  sig.returns(Random)
  def untrust(); end
end

class Range < Object
  sig.returns(T::Range[Elem])
  def clone(); end

  sig.returns(T::Range[Elem])
  def dup(); end

  sig.returns(T::Range[Elem])
  def freeze(); end

  sig.returns(T::Range[Elem])
  def taint(); end

  sig.returns(T::Range[Elem])
  def trust(); end

  sig.returns(T::Range[Elem])
  def untaint(); end

  sig.returns(T::Range[Elem])
  def untrust(); end
end

class RangeError < StandardError
  sig.returns(RangeError)
  def clone(); end

  sig.returns(RangeError)
  def dup(); end

  sig.returns(RangeError)
  def freeze(); end

  sig.returns(RangeError)
  def taint(); end

  sig.returns(RangeError)
  def trust(); end

  sig.returns(RangeError)
  def untaint(); end

  sig.returns(RangeError)
  def untrust(); end
end

class Rational < Numeric
  sig.returns(Rational)
  def clone(); end

  sig.returns(Rational)
  def dup(); end

  sig.returns(Rational)
  def freeze(); end

  sig.returns(Rational)
  def taint(); end

  sig.returns(Rational)
  def trust(); end

  sig.returns(Rational)
  def untaint(); end

  sig.returns(Rational)
  def untrust(); end
end

class Regexp < Object
  sig.returns(Regexp)
  def clone(); end

  sig.returns(Regexp)
  def dup(); end

  sig.returns(Regexp)
  def freeze(); end

  sig.returns(Regexp)
  def taint(); end

  sig.returns(Regexp)
  def trust(); end

  sig.returns(Regexp)
  def untaint(); end

  sig.returns(Regexp)
  def untrust(); end
end

class RegexpError < StandardError
  sig.returns(RegexpError)
  def clone(); end

  sig.returns(RegexpError)
  def dup(); end

  sig.returns(RegexpError)
  def freeze(); end

  sig.returns(RegexpError)
  def taint(); end

  sig.returns(RegexpError)
  def trust(); end

  sig.returns(RegexpError)
  def untaint(); end

  sig.returns(RegexpError)
  def untrust(); end
end

class RubyVM < Object
  sig.returns(RubyVM)
  def clone(); end

  sig.returns(RubyVM)
  def dup(); end

  sig.returns(RubyVM)
  def freeze(); end

  sig.returns(RubyVM)
  def taint(); end

  sig.returns(RubyVM)
  def trust(); end

  sig.returns(RubyVM)
  def untaint(); end

  sig.returns(RubyVM)
  def untrust(); end
end

class RubyVM::InstructionSequence < Object
  sig.returns(RubyVM::InstructionSequence)
  def clone(); end

  sig.returns(RubyVM::InstructionSequence)
  def dup(); end

  sig.returns(RubyVM::InstructionSequence)
  def freeze(); end

  sig.returns(RubyVM::InstructionSequence)
  def taint(); end

  sig.returns(RubyVM::InstructionSequence)
  def trust(); end

  sig.returns(RubyVM::InstructionSequence)
  def untaint(); end

  sig.returns(RubyVM::InstructionSequence)
  def untrust(); end
end

class RuntimeError < StandardError
  sig.returns(RuntimeError)
  def clone(); end

  sig.returns(RuntimeError)
  def dup(); end

  sig.returns(RuntimeError)
  def freeze(); end

  sig.returns(RuntimeError)
  def taint(); end

  sig.returns(RuntimeError)
  def trust(); end

  sig.returns(RuntimeError)
  def untaint(); end

  sig.returns(RuntimeError)
  def untrust(); end
end

class ScriptError < Exception
  sig.returns(ScriptError)
  def clone(); end

  sig.returns(ScriptError)
  def dup(); end

  sig.returns(ScriptError)
  def freeze(); end

  sig.returns(ScriptError)
  def taint(); end

  sig.returns(ScriptError)
  def trust(); end

  sig.returns(ScriptError)
  def untaint(); end

  sig.returns(ScriptError)
  def untrust(); end
end

class SecurityError < Exception
  sig.returns(SecurityError)
  def clone(); end

  sig.returns(SecurityError)
  def dup(); end

  sig.returns(SecurityError)
  def freeze(); end

  sig.returns(SecurityError)
  def taint(); end

  sig.returns(SecurityError)
  def trust(); end

  sig.returns(SecurityError)
  def untaint(); end

  sig.returns(SecurityError)
  def untrust(); end
end

class Set < Object
  sig.returns(T::Set[Elem])
  def clone(); end

  sig.returns(T::Set[Elem])
  def dup(); end

  sig.returns(T::Set[Elem])
  def freeze(); end

  sig.returns(T::Set[Elem])
  def taint(); end

  sig.returns(T::Set[Elem])
  def trust(); end

  sig.returns(T::Set[Elem])
  def untaint(); end

  sig.returns(T::Set[Elem])
  def untrust(); end
end

class SignalException < Exception
  sig.returns(SignalException)
  def clone(); end

  sig.returns(SignalException)
  def dup(); end

  sig.returns(SignalException)
  def freeze(); end

  sig.returns(SignalException)
  def taint(); end

  sig.returns(SignalException)
  def trust(); end

  sig.returns(SignalException)
  def untaint(); end

  sig.returns(SignalException)
  def untrust(); end
end

class SortedSet < Set
  sig(
      o: Elem,
  )
  .returns(SortedSet[Elem])
  def add(o); end

  sig.returns(SortedSet[Elem])
  def clear(); end

  sig.returns(SortedSet[Elem])
  def clone(); end

  sig(
      o: Elem,
  )
  .returns(SortedSet[Elem])
  def delete(o); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(SortedSet[Elem])
  def delete_if(&blk); end

  sig.returns(SortedSet[Elem])
  def dup(); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(SortedSet[Elem])
  def each(&blk); end

  sig.returns(SortedSet[Elem])
  def freeze(); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(SortedSet[Elem])
  def keep_if(&blk); end

  sig(
      enum: T::Enumerable[Elem],
  )
  .returns(SortedSet[Elem])
  def merge(enum); end

  sig(
      enum: T::Enumerable[Elem],
  )
  .returns(SortedSet[Elem])
  def subtract(enum); end

  sig.returns(SortedSet[Elem])
  def taint(); end

  sig.returns(SortedSet[Elem])
  def trust(); end

  sig.returns(SortedSet[Elem])
  def untaint(); end

  sig.returns(SortedSet[Elem])
  def untrust(); end
end

class StandardError < Exception
  sig.returns(StandardError)
  def clone(); end

  sig.returns(StandardError)
  def dup(); end

  sig.returns(StandardError)
  def freeze(); end

  sig.returns(StandardError)
  def taint(); end

  sig.returns(StandardError)
  def trust(); end

  sig.returns(StandardError)
  def untaint(); end

  sig.returns(StandardError)
  def untrust(); end
end

class StopIteration < IndexError
  sig.returns(StopIteration)
  def clone(); end

  sig.returns(StopIteration)
  def dup(); end

  sig.returns(StopIteration)
  def freeze(); end

  sig.returns(StopIteration)
  def taint(); end

  sig.returns(StopIteration)
  def trust(); end

  sig.returns(StopIteration)
  def untaint(); end

  sig.returns(StopIteration)
  def untrust(); end
end

class String < Object
  sig.returns(String)
  def clone(); end

  sig.returns(String)
  def dup(); end

  sig.returns(String)
  def freeze(); end

  sig.returns(String)
  def taint(); end

  sig.returns(String)
  def trust(); end

  sig.returns(String)
  def untaint(); end

  sig.returns(String)
  def untrust(); end
end

class StringIO < Data
  sig.returns(StringIO)
  def clone(); end

  sig.returns(StringIO)
  def dup(); end

  sig.returns(StringIO)
  def freeze(); end

  sig.returns(StringIO)
  def taint(); end

  sig.returns(StringIO)
  def trust(); end

  sig.returns(StringIO)
  def untaint(); end

  sig.returns(StringIO)
  def untrust(); end
end

class StringScanner < Object
  sig.returns(StringScanner)
  def clone(); end

  sig.returns(StringScanner)
  def dup(); end

  sig.returns(StringScanner)
  def freeze(); end

  sig.returns(StringScanner)
  def taint(); end

  sig.returns(StringScanner)
  def trust(); end

  sig.returns(StringScanner)
  def untaint(); end

  sig.returns(StringScanner)
  def untrust(); end
end

class StringScanner::Error < StandardError
  sig.returns(StringScanner::Error)
  def clone(); end

  sig.returns(StringScanner::Error)
  def dup(); end

  sig.returns(StringScanner::Error)
  def freeze(); end

  sig.returns(StringScanner::Error)
  def taint(); end

  sig.returns(StringScanner::Error)
  def trust(); end

  sig.returns(StringScanner::Error)
  def untaint(); end

  sig.returns(StringScanner::Error)
  def untrust(); end
end

class Struct < Object
  sig.returns(Struct)
  def clone(); end

  sig.returns(Struct)
  def dup(); end

  sig.returns(Struct)
  def freeze(); end

  sig.returns(Struct)
  def taint(); end

  sig.returns(Struct)
  def trust(); end

  sig.returns(Struct)
  def untaint(); end

  sig.returns(Struct)
  def untrust(); end
end

class Symbol < Object
  sig.returns(Symbol)
  def clone(); end

  sig.returns(Symbol)
  def dup(); end

  sig.returns(Symbol)
  def freeze(); end

  sig.returns(Symbol)
  def taint(); end

  sig.returns(Symbol)
  def trust(); end

  sig.returns(Symbol)
  def untaint(); end

  sig.returns(Symbol)
  def untrust(); end
end

class SyntaxError < ScriptError
  sig.returns(SyntaxError)
  def clone(); end

  sig.returns(SyntaxError)
  def dup(); end

  sig.returns(SyntaxError)
  def freeze(); end

  sig.returns(SyntaxError)
  def taint(); end

  sig.returns(SyntaxError)
  def trust(); end

  sig.returns(SyntaxError)
  def untaint(); end

  sig.returns(SyntaxError)
  def untrust(); end
end

class SystemCallError < StandardError
  sig.returns(SystemCallError)
  def clone(); end

  sig.returns(SystemCallError)
  def dup(); end

  sig.returns(SystemCallError)
  def freeze(); end

  sig.returns(SystemCallError)
  def taint(); end

  sig.returns(SystemCallError)
  def trust(); end

  sig.returns(SystemCallError)
  def untaint(); end

  sig.returns(SystemCallError)
  def untrust(); end
end

class SystemExit < Exception
  sig.returns(SystemExit)
  def clone(); end

  sig.returns(SystemExit)
  def dup(); end

  sig.returns(SystemExit)
  def freeze(); end

  sig.returns(SystemExit)
  def taint(); end

  sig.returns(SystemExit)
  def trust(); end

  sig.returns(SystemExit)
  def untaint(); end

  sig.returns(SystemExit)
  def untrust(); end
end

class SystemStackError < Exception
  sig.returns(SystemStackError)
  def clone(); end

  sig.returns(SystemStackError)
  def dup(); end

  sig.returns(SystemStackError)
  def freeze(); end

  sig.returns(SystemStackError)
  def taint(); end

  sig.returns(SystemStackError)
  def trust(); end

  sig.returns(SystemStackError)
  def untaint(); end

  sig.returns(SystemStackError)
  def untrust(); end
end

class Thread < Object
  sig.returns(Thread)
  def clone(); end

  sig.returns(Thread)
  def dup(); end

  sig.returns(Thread)
  def freeze(); end

  sig.returns(Thread)
  def taint(); end

  sig.returns(Thread)
  def trust(); end

  sig.returns(Thread)
  def untaint(); end

  sig.returns(Thread)
  def untrust(); end
end

class Thread::Backtrace < Object
  sig.returns(Thread::Backtrace)
  def clone(); end

  sig.returns(Thread::Backtrace)
  def dup(); end

  sig.returns(Thread::Backtrace)
  def freeze(); end

  sig.returns(Thread::Backtrace)
  def taint(); end

  sig.returns(Thread::Backtrace)
  def trust(); end

  sig.returns(Thread::Backtrace)
  def untaint(); end

  sig.returns(Thread::Backtrace)
  def untrust(); end
end

class Thread::Backtrace::Location < Object
  sig.returns(Thread::Backtrace::Location)
  def clone(); end

  sig.returns(Thread::Backtrace::Location)
  def dup(); end

  sig.returns(Thread::Backtrace::Location)
  def freeze(); end

  sig.returns(Thread::Backtrace::Location)
  def taint(); end

  sig.returns(Thread::Backtrace::Location)
  def trust(); end

  sig.returns(Thread::Backtrace::Location)
  def untaint(); end

  sig.returns(Thread::Backtrace::Location)
  def untrust(); end
end

class Thread::ConditionVariable < Object
  sig.returns(Thread::ConditionVariable)
  def clone(); end

  sig.returns(Thread::ConditionVariable)
  def dup(); end

  sig.returns(Thread::ConditionVariable)
  def freeze(); end

  sig.returns(Thread::ConditionVariable)
  def taint(); end

  sig.returns(Thread::ConditionVariable)
  def trust(); end

  sig.returns(Thread::ConditionVariable)
  def untaint(); end

  sig.returns(Thread::ConditionVariable)
  def untrust(); end
end

class Thread::Mutex < Object
  sig.returns(Thread::Mutex)
  def clone(); end

  sig.returns(Thread::Mutex)
  def dup(); end

  sig.returns(Thread::Mutex)
  def freeze(); end

  sig.returns(Thread::Mutex)
  def taint(); end

  sig.returns(Thread::Mutex)
  def trust(); end

  sig.returns(Thread::Mutex)
  def untaint(); end

  sig.returns(Thread::Mutex)
  def untrust(); end
end

class Thread::Queue < Object
  sig.returns(Thread::Queue)
  def clone(); end

  sig.returns(Thread::Queue)
  def dup(); end

  sig.returns(Thread::Queue)
  def freeze(); end

  sig.returns(Thread::Queue)
  def taint(); end

  sig.returns(Thread::Queue)
  def trust(); end

  sig.returns(Thread::Queue)
  def untaint(); end

  sig.returns(Thread::Queue)
  def untrust(); end
end

class Thread::SizedQueue < Thread::Queue
  sig.returns(Thread::SizedQueue)
  def clone(); end

  sig.returns(Thread::SizedQueue)
  def dup(); end

  sig.returns(Thread::SizedQueue)
  def freeze(); end

  sig.returns(Thread::SizedQueue)
  def taint(); end

  sig.returns(Thread::SizedQueue)
  def trust(); end

  sig.returns(Thread::SizedQueue)
  def untaint(); end

  sig.returns(Thread::SizedQueue)
  def untrust(); end
end

class ThreadError < StandardError
  sig.returns(ThreadError)
  def clone(); end

  sig.returns(ThreadError)
  def dup(); end

  sig.returns(ThreadError)
  def freeze(); end

  sig.returns(ThreadError)
  def taint(); end

  sig.returns(ThreadError)
  def trust(); end

  sig.returns(ThreadError)
  def untaint(); end

  sig.returns(ThreadError)
  def untrust(); end
end

class ThreadGroup < Object
  sig.returns(ThreadGroup)
  def clone(); end

  sig.returns(ThreadGroup)
  def dup(); end

  sig.returns(ThreadGroup)
  def freeze(); end

  sig.returns(ThreadGroup)
  def taint(); end

  sig.returns(ThreadGroup)
  def trust(); end

  sig.returns(ThreadGroup)
  def untaint(); end

  sig.returns(ThreadGroup)
  def untrust(); end
end

class Time < Object
  sig.returns(Time)
  def clone(); end

  sig.returns(Time)
  def dup(); end

  sig.returns(Time)
  def freeze(); end

  sig.returns(Time)
  def taint(); end

  sig.returns(Time)
  def trust(); end

  sig.returns(Time)
  def untaint(); end

  sig.returns(Time)
  def untrust(); end
end

class TracePoint < Object
  sig.returns(TracePoint)
  def clone(); end

  sig.returns(TracePoint)
  def dup(); end

  sig.returns(TracePoint)
  def freeze(); end

  sig.returns(TracePoint)
  def taint(); end

  sig.returns(TracePoint)
  def trust(); end

  sig.returns(TracePoint)
  def untaint(); end

  sig.returns(TracePoint)
  def untrust(); end
end

class TrueClass < Object
  sig.returns(TrueClass)
  def clone(); end

  sig.returns(TrueClass)
  def dup(); end

  sig.returns(TrueClass)
  def freeze(); end

  sig.returns(TrueClass)
  def taint(); end

  sig.returns(TrueClass)
  def trust(); end

  sig.returns(TrueClass)
  def untaint(); end

  sig.returns(TrueClass)
  def untrust(); end
end

class TypeError < StandardError
  sig.returns(TypeError)
  def clone(); end

  sig.returns(TypeError)
  def dup(); end

  sig.returns(TypeError)
  def freeze(); end

  sig.returns(TypeError)
  def taint(); end

  sig.returns(TypeError)
  def trust(); end

  sig.returns(TypeError)
  def untaint(); end

  sig.returns(TypeError)
  def untrust(); end
end

class URI::BadURIError < URI::Error
  sig.returns(URI::BadURIError)
  def clone(); end

  sig.returns(URI::BadURIError)
  def dup(); end

  sig.returns(URI::BadURIError)
  def freeze(); end

  sig.returns(URI::BadURIError)
  def taint(); end

  sig.returns(URI::BadURIError)
  def trust(); end

  sig.returns(URI::BadURIError)
  def untaint(); end

  sig.returns(URI::BadURIError)
  def untrust(); end
end

class URI::Error < StandardError
  sig.returns(URI::Error)
  def clone(); end

  sig.returns(URI::Error)
  def dup(); end

  sig.returns(URI::Error)
  def freeze(); end

  sig.returns(URI::Error)
  def taint(); end

  sig.returns(URI::Error)
  def trust(); end

  sig.returns(URI::Error)
  def untaint(); end

  sig.returns(URI::Error)
  def untrust(); end
end

class URI::FTP < URI::Generic
  sig.returns(URI::FTP)
  def clone(); end

  sig.returns(URI::FTP)
  def dup(); end

  sig.returns(URI::FTP)
  def freeze(); end

  sig.returns(URI::FTP)
  def taint(); end

  sig.returns(URI::FTP)
  def trust(); end

  sig.returns(URI::FTP)
  def untaint(); end

  sig.returns(URI::FTP)
  def untrust(); end
end

class URI::Generic < Object
  sig.returns(URI::Generic)
  def clone(); end

  sig.returns(URI::Generic)
  def dup(); end

  sig.returns(URI::Generic)
  def freeze(); end

  sig.returns(URI::Generic)
  def taint(); end

  sig.returns(URI::Generic)
  def trust(); end

  sig.returns(URI::Generic)
  def untaint(); end

  sig.returns(URI::Generic)
  def untrust(); end
end

class URI::HTTP < URI::Generic
  sig.returns(URI::HTTP)
  def clone(); end

  sig.returns(URI::HTTP)
  def dup(); end

  sig.returns(URI::HTTP)
  def freeze(); end

  sig.returns(URI::HTTP)
  def taint(); end

  sig.returns(URI::HTTP)
  def trust(); end

  sig.returns(URI::HTTP)
  def untaint(); end

  sig.returns(URI::HTTP)
  def untrust(); end
end

class URI::HTTPS < URI::HTTP
  sig.returns(URI::HTTPS)
  def clone(); end

  sig.returns(URI::HTTPS)
  def dup(); end

  sig.returns(URI::HTTPS)
  def freeze(); end

  sig.returns(URI::HTTPS)
  def taint(); end

  sig.returns(URI::HTTPS)
  def trust(); end

  sig.returns(URI::HTTPS)
  def untaint(); end

  sig.returns(URI::HTTPS)
  def untrust(); end
end

class URI::InvalidComponentError < URI::Error
  sig.returns(URI::InvalidComponentError)
  def clone(); end

  sig.returns(URI::InvalidComponentError)
  def dup(); end

  sig.returns(URI::InvalidComponentError)
  def freeze(); end

  sig.returns(URI::InvalidComponentError)
  def taint(); end

  sig.returns(URI::InvalidComponentError)
  def trust(); end

  sig.returns(URI::InvalidComponentError)
  def untaint(); end

  sig.returns(URI::InvalidComponentError)
  def untrust(); end
end

class URI::InvalidURIError < URI::Error
  sig.returns(URI::InvalidURIError)
  def clone(); end

  sig.returns(URI::InvalidURIError)
  def dup(); end

  sig.returns(URI::InvalidURIError)
  def freeze(); end

  sig.returns(URI::InvalidURIError)
  def taint(); end

  sig.returns(URI::InvalidURIError)
  def trust(); end

  sig.returns(URI::InvalidURIError)
  def untaint(); end

  sig.returns(URI::InvalidURIError)
  def untrust(); end
end

class URI::LDAP < URI::Generic
  sig.returns(URI::LDAP)
  def clone(); end

  sig.returns(URI::LDAP)
  def dup(); end

  sig.returns(URI::LDAP)
  def freeze(); end

  sig.returns(URI::LDAP)
  def taint(); end

  sig.returns(URI::LDAP)
  def trust(); end

  sig.returns(URI::LDAP)
  def untaint(); end

  sig.returns(URI::LDAP)
  def untrust(); end
end

class URI::LDAPS < URI::LDAP
  sig.returns(URI::LDAPS)
  def clone(); end

  sig.returns(URI::LDAPS)
  def dup(); end

  sig.returns(URI::LDAPS)
  def freeze(); end

  sig.returns(URI::LDAPS)
  def taint(); end

  sig.returns(URI::LDAPS)
  def trust(); end

  sig.returns(URI::LDAPS)
  def untaint(); end

  sig.returns(URI::LDAPS)
  def untrust(); end
end

class URI::MailTo < URI::Generic
  sig.returns(URI::MailTo)
  def clone(); end

  sig.returns(URI::MailTo)
  def dup(); end

  sig.returns(URI::MailTo)
  def freeze(); end

  sig.returns(URI::MailTo)
  def taint(); end

  sig.returns(URI::MailTo)
  def trust(); end

  sig.returns(URI::MailTo)
  def untaint(); end

  sig.returns(URI::MailTo)
  def untrust(); end
end

class URI::RFC2396_Parser < Object
  sig.returns(URI::RFC2396_Parser)
  def clone(); end

  sig.returns(URI::RFC2396_Parser)
  def dup(); end

  sig.returns(URI::RFC2396_Parser)
  def freeze(); end

  sig.returns(URI::RFC2396_Parser)
  def taint(); end

  sig.returns(URI::RFC2396_Parser)
  def trust(); end

  sig.returns(URI::RFC2396_Parser)
  def untaint(); end

  sig.returns(URI::RFC2396_Parser)
  def untrust(); end
end

class URI::RFC3986_Parser < Object
  sig.returns(URI::RFC3986_Parser)
  def clone(); end

  sig.returns(URI::RFC3986_Parser)
  def dup(); end

  sig.returns(URI::RFC3986_Parser)
  def freeze(); end

  sig.returns(URI::RFC3986_Parser)
  def taint(); end

  sig.returns(URI::RFC3986_Parser)
  def trust(); end

  sig.returns(URI::RFC3986_Parser)
  def untaint(); end

  sig.returns(URI::RFC3986_Parser)
  def untrust(); end
end
class UnboundMethod < Object
  sig.returns(UnboundMethod)
  def clone(); end

  sig.returns(UnboundMethod)
  def dup(); end

  sig.returns(UnboundMethod)
  def freeze(); end

  sig.returns(UnboundMethod)
  def taint(); end

  sig.returns(UnboundMethod)
  def trust(); end

  sig.returns(UnboundMethod)
  def untaint(); end

  sig.returns(UnboundMethod)
  def untrust(); end
end

class UncaughtThrowError < ArgumentError
  sig.returns(UncaughtThrowError)
  def clone(); end

  sig.returns(UncaughtThrowError)
  def dup(); end

  sig.returns(UncaughtThrowError)
  def freeze(); end

  sig.returns(UncaughtThrowError)
  def taint(); end

  sig.returns(UncaughtThrowError)
  def trust(); end

  sig.returns(UncaughtThrowError)
  def untaint(); end

  sig.returns(UncaughtThrowError)
  def untrust(); end
end

class ZeroDivisionError < StandardError
  sig.returns(ZeroDivisionError)
  def clone(); end

  sig.returns(ZeroDivisionError)
  def dup(); end

  sig.returns(ZeroDivisionError)
  def freeze(); end

  sig.returns(ZeroDivisionError)
  def taint(); end

  sig.returns(ZeroDivisionError)
  def trust(); end

  sig.returns(ZeroDivisionError)
  def untaint(); end

  sig.returns(ZeroDivisionError)
  def untrust(); end
end

