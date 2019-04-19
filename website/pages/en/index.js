/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const React = require("react");

const CompLibrary = require("../../core/CompLibrary.js");

const MarkdownBlock = CompLibrary.MarkdownBlock; /* Used to read markdown */
const Container = CompLibrary.Container;
const GridBlock = CompLibrary.GridBlock;

const sorbetRunExample =
  "https://sorbet.run/#%23%20typed%3A%20true%0A%0A%23%20This%20code%20is%20editable!%0A%23%20You%20can%20also%20hover%20to%20see%20error%20messages%20and%20types!%0A%0Aextend%20T%3A%3ASig%0A%0Asig%20%7Bparams(name%3A%20String).returns(Integer)%7D%0Adef%20main(name)%0A%20%20puts%20%22Hello%2C%20%23%7Bname%7D!%22%0A%20%20name.length%0Aend%0A%0Amain(%22Sorbet%22)%20%23%20ok!%0Amain()%20%20%20%23%20error%3A%20Not%20enough%20arguments%20provided%20for%20method%20%60main%60%0Aman(%22%22)%20%20%23%20error%3A%20Method%20%60man%60%20does%20not%20exist";

const signUpLink =
  "https://docs.google.com/forms/d/1aj1XkNwdeNeX5gzm_cMWiKC1__vUQ29qBbBxHx6tNOc/viewform";

const mainExample = `
\`\`\`ruby
# typed: true
extend T::Sig

sig {params(name: String).returns(Integer)}
def main(name)
  puts "Hello, #{name}!"
  name.length
end

main("Sorbet") # ok!
main()   # error: Not enough arguments provided for method \`main\`
man("")  # error: Method \`man\` does not exist
\`\`\`
`

// TODO(jez) No inline styles
const PageSection = (props) => (
  <div
    style={{
      backgroundColor: (props.gray ? "#f5f5f5" :
        (props.lightPurple ? "#6255b0": "white")),
      color: props.lightPurple ? "white" : "inherit",
      padding: props.short ? "2em 0" : "3em 0"
    }}
    className="pageSection"
  >
    <div className="wrapper">{props.children}</div>
  </div>
);

// TODO(jez) No inline style
const Button = (props) => (
  <a
    className="button"
    style={{ marginRight: "10px" }}
    href={props.href}
    target={props.target}
  >
    {props.children}
  </a>
);

class Index extends React.Component {
  render() {
    const { config: siteConfig, language = "" } = this.props;
    const { baseUrl, docsUrl } = siteConfig;
    const docsPart = `${docsUrl ? `${docsUrl}/` : ""}`;
    const langPart = `${language ? `${language}/` : ""}`;
    const docUrl = doc => `${baseUrl}${docsPart}${langPart}${doc}`;

    return (
      <div>
        <div className="homeContainer">
          <div className="homeSplashFade">
            <div className="wrapper homeWrapper">
              <div className="projectLogo">
                <img
                  src={`${baseUrl}img/sorbet-logo-white-sparkles.svg`}
                  alt="Sorbet Logo"
                />
              </div>
              <div className="inner">
                <h2
                  className="projectTitle"
                  style={{ maxWidth: "850px", textAlign: "left" }}
                >
                  Sorbet is a fast, powerful type checker designed for Ruby.
                </h2>
                <p style={{ paddingTop: "1em" }}>
                  <strong>Coming Summer 2019.</strong> Built with 💜 at <a href="https://stripe.com">Stripe</a>.
                </p>
                <div style={{ paddingTop: "1.2em" }}>
                  <Button href={docUrl("overview")}>Read the docs</Button>
                  <Button href="http://sorbet.run">Try it online</Button>
                </div>
              </div>
            </div>
          </div>
          <Container align="center" className="featuresContainer">
            <GridBlock
              align="center"
              layout="threeColumn"
              contents={[
                {
                  title: "Fast and scalable",
                  content:
                    "Sorbet is multithreaded, scaling linearly across cores on your CPU. It checks your types in seconds, giving you feedback as you code.\n\n[Get started with Sorbet →](docs/adopting)"
                },
                {
                  title: "IDE Ready",
                  content:
                    "Sorbet works with your favorite editor to provide IDE features like autocomplete and jump to definition. It's easy to add to your CI setup.\n\n[Try Sorbet online →](https://sorbet.run)"
                },
                {
                  title: "Gradual by design",
                  content:
                    "Sorbet works with normal Ruby, so you can keep using your existing toolchain. Add Sorbet types to your codebase one file at a time.\n\n[Learn what makes Sorbet gradual →](docs/gradual)"
                }
              ]}
            />
          </Container>
        </div>
        <PageSection gray short>
          <div style={{ fontSize: "1.2em", textAlign: "center" }}>
            <strong>Coming Summer 2019</strong>. We're looking for early
            feedback to make Sorbet great:{" "}
            <a href={signUpLink}>sign up here</a> to get involved!
          </div>
        </PageSection>
        <PageSection>
          <h2>A Taste of Sorbet</h2>
          <div className="row">
            <div className="column">
              <p>
                Sorbet is 100% compatible with Ruby. It type checks normal
                method definitions, and introduces backwards-compatible syntax
                for method signatures.
              </p>
              <p>
                Explicit method signatures make Sorbet useful for anyone
                reading the code too (not just the author). Type annotations
                serve as a tool for understanding long after they're written.
              </p>
              <p>
                Sorbet is designed to be useful, not burdensome. Explicit
                annotations are repaid with clear error messages, increased
                safety, and increased productivity.
              </p>
            </div>
            <div className="column">
              <MarkdownBlock>{mainExample}</MarkdownBlock>
            </div>
          </div>
        </PageSection>
        <PageSection gray>
          <h2>Designed to be interactive</h2>
          <div className="row">
            <div className="column">
              <p>
                Sorbet gives your Ruby development environment IDE-like features,
                including autocomplete, in-editor documentation, and go to
                definition. The implementation leverages the{" "}
                <a href="https://langserver.org/">Language Server Protocol</a>{" "}
                to be compatible with your favorite editor.
              </p>
            </div>
            <div className="column">
              <p>
                In the time we've spent adopting Sorbet at Stripe, countless
                people have told us that adding types to existing code or
                writing new code feels interactive, like{" "}
                <a href="https://sorbet.run/talks/RubyKaigi2019/#/28">pair-programming with the type checker</a>.
                People ask Sorbet questions, and it responds in seconds—or
                faster.
              </p>
            </div>
          </div>
          <img
            style={{ width: "100%", marginTop: "0.5em" }}
            src={`${baseUrl}img/autocompleteAndDocs1.gif`}
            alt="GIF of autocomplete in editor"
          />
        </PageSection>
        <PageSection short>
          <div className="pageSectionColumns">
            <p style={{display: "flex", flexDirection: "row", paddingRight: "50px", margin: 0}}>
              <img
                src={`${baseUrl}img/stripe-logo.svg`}
                alt="Stripe Logo"
                style={{ height: "50px", marginLeft: "-10px", marginRight: "30px" }}
              />
              <img
                src={`${baseUrl}img/coinbase.png`}
                alt="Coinbase Logo"
                style={{ height: "50px", marginLeft: "-5px", position: "relative", top: "-4px"}}
              />
            </p>
            <p style={{flex: 1, margin: 0}}>
              Sorbet is currently used by the teams at Stripe and Coinbase. If
              you'd like to try it at your company,{" "}
              <a href={signUpLink}>
                sign up for our mailing list
              </a>{" "}
              and we'll reach out when we have more to share!
            </p>
          </div>
        </PageSection>
        <PageSection gray>
          <h2>Gradual Typing of Ruby at Scale</h2>
          <div className="row">
            <div className="column">
              <a href="https://www.youtube.com/watch?v=uFFJyp8vXQI">
                  <img src={`${baseUrl}img/talk-thumb.png`}
                  alt="Link to Sorbet talk video"
                  style={{ width: "100%", maxWidth: "850px", marginTop: "0.5em"}} />
              </a>
            </div>
            <div className="column">
              <p>
                Much has already been said about Sorbet. Here's a talk we gave at{" "}
                <a href="docs/talks/strange-loop-2018">Strange Loop 2018</a>:
              </p>
              <blockquote className="monotone">
                <p>
                  Stripe maintains an extremely large and growing Ruby code
                  base with hundreds of developers and millions of lines of
                  code. Continuing to scale development is one of the most
                  critical tasks to maintaining product velocity and the
                  productivity of Stripe engineering.
                </p>
                <p>
                  This talk shares our experience building a type checker, and
                  adopting it in our massive codebase. We start with a
                  discussion of core design decisions we made in early days of
                  the project, and evaluate how they withstood reality of
                  production use.
                </p>
              </blockquote>
            </div>
          </div>
        </PageSection>
        <PageSection>
          <h1>Dive into the docs</h1>
          <GridBlock
            layout="threeColumn"
            contents={[
              {
                title: "Gradual Type Checking",
                content:
                  "Sorbet is a **gradual** type checker, which enables incremental adoption. On the other hand, it'll be unfamiliar to those expecting a traditional statically typed language.\n\n[Learn why gradual is different →](docs/gradual)"
              },
              {
                title: "Enabling Static Checks",
                content:
                  "Sorbet works with 100% of Ruby, but it does not work the same for all Ruby. Developers can opt into more static checks to get even more safety and productivity.\n\n[How to enable static static checks →](docs/static)"
              },
              {
                title: "Flow-sensitive Typing",
                content:
                  "One way Sorbet works to be useful without getting in the way is with control-flow-sensitive typing. Sorbet tracks the effect of control on types, instead of asking for type annotations.\n\n[See the power of flow-sensitivity →](docs/flow-sensitive)"
              }
            ]}
          />
        </PageSection>
        <PageSection short lightPurple>
          <p className="footer">
            © 2019 Stripe{" · "}
            <a href="docs/adopting">Get started</a>{" · "}
            <a href="docs/">Docs</a>{" · "}
            <a href="https://sorbet.run">Try it online</a>{" · "}
            <a href="talks/">Talks</a>
          </p>
        </PageSection>
      </div>
    );
  }
}

module.exports = Index;
