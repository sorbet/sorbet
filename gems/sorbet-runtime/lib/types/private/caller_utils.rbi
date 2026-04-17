# typed: true

module T::Private::CallerUtils
  sig do
    params(
      blk: T.proc.params(arg0: Thread::Backtrace::Location).returns(T::Boolean)
    )
      .returns(T.nilable(Thread::Backtrace::Location))
  end
  def self.find_caller(&blk); end
end
