# typed: true
class A
  class Foo
    extend T::Sig

    sig {returns(T.any(String, NilClass))}
    def name;end
  end

  extend T::Sig

  sig {params(repo: String, branch: String).returns(T::Array[T.nilable(String)])}
  def automatic_job_names(repo, branch)
    job_names = T::Array[Foo].new.map(&:name)
    job_names.select do |job_name|
     job_name
    end
  end
end
