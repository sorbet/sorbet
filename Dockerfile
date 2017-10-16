FROM dockerregistry.local.corp.stripe.com/stripe/ubuntu-16.04:latest

RUN add-apt-repository ppa:openjdk-r/ppa && apt-get update && apt-get install -y openjdk-8-jdk zip
RUN update-java-alternatives -s java-1.8.0-openjdk-amd64
ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64/
ENV PATH $JAVA_HOME/bin:$PATH
RUN export JAVA_HOME

RUN apt-get update && apt-get install -y clang-4.0 clang-format-4.0

# Dependencies for the parser
RUN apt-get update && apt-get install -y ragel bison ruby

RUN /usr/stripe/bin/docker/stripe-install-go 1.9 d70eadefce8e160638a9a6db97f7192d8463069ab33138893ad3bf31b0650a79
ENV GOPATH /gopath
ENV PATH $PATH:/usr/local/go/bin:$GOPATH/bin
RUN mkdir -p "$GOPATH" && \
  go get -d -u github.com/bazelbuild/buildifier/buildifier && \
  go generate github.com/bazelbuild/buildifier/build && \
  go install github.com/bazelbuild/buildifier/buildifier

# Upgrade to clang-format 5.0.0
#
# Eventually we probably want to force this entire toolchain, too, but
# we can defer that.
RUN curl -Lo /tmp/clang-5.0.0.tar.xz http://releases.llvm.org/5.0.0/clang+llvm-5.0.0-linux-x86_64-ubuntu16.04.tar.xz && \
  tar -C /usr/local -Jxf /tmp/clang-5.0.0.tar.xz && \
  rm -f /tmp/clang-5.0.0.tar.xz
RUN ln -nsf /usr/local/clang+llvm-5.0.0-linux-x86_64-ubuntu16.04/bin/clang-format /usr/bin/clang-format

WORKDIR /src

ENV PATH $PATH:/src/

CMD /src/jenkins-build.sh
