# typed: true
class A
  class Foo
    extend T::Helpers

    sig {returns(T.any(String, NilClass))}
    def name;end
  end

  extend T::Helpers

  sig {params(repo: String, branch: String).returns(T::Array[T.nilable(String)])}
  def automatic_job_names(repo, branch)
    job_names = T::Array[Foo].new.map(&:name)
    job_names.select do |job_name|
     job_name
    end
  end
end
