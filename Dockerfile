FROM dockerregistry.local.corp.stripe.com/stripe/ubuntu-16.04:latest

RUN apt-get update && apt-get install -y clang-4.0 clang-format-16.04

RUN /usr/stripe/bin/docker/stripe-install-go 1.9 d70eadefce8e160638a9a6db97f7192d8463069ab33138893ad3bf31b0650a79
ENV GOPATH /gopath
ENV PATH $PATH:/usr/local/go/bin:$GOPATH/bin
RUN mkdir -p "$GOPATH" && \
  go get -d -u github.com/bazelbuild/buildifier/buildifier && \
  go generate github.com/bazelbuild/buildifier/build && \
  go install github.com/bazelbuild/buildifier/buildifier

WORKDIR /src

ENV PATH $PATH:/src/

CMD /src/jenkins-build.sh
