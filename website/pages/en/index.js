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

const PageSection = props => (
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

const Button = props => (
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
                  <strong>Available in summer 2019.</strong> Built by the team at <a href="https://stripe.com">Stripe</a>.
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
                    `Sorbet is multithreaded, scaling linearly across cores on your CPU. It checks your types in seconds, giving you feedback as you code.

[Get started with Sorbet →](docs/adopting)`
                },
                {
                  title: "Integrates with your tools",
                  content:
                    `Sorbet works with editors like VS Code, with features like autocomplete and jump to definition. It's easy to add to your CI setup.

[Try Sorbet online →](https://sorbet.run)`
                },
                {
                  title: "Gradual by design",
                  content:
                    `Sorbet works with normal Ruby, so you can keep using your existing toolchain. Add Sorbet types to your codebase one file at a time.

[Learn what makes Sorbet gradual →](docs/gradual)`
                }
              ]}
            />
          </Container>
        </div>
        <PageSection gray short>
          <div style={{ fontSize: "1.2em", textAlign: "center" }}>
            <strong>Available in summer 2019.</strong> We’re looking for early feedback to make Sorbet great:{" "}
            <a href="https://docs.google.com/forms/d/1aj1XkNwdeNeX5gzm_cMWiKC1__vUQ29qBbBxHx6tNOc/viewform?edit_requested=true">
              join the mailing list
            </a>{" "}
            to get involved.
          </div>
        </PageSection>
        <PageSection>
          <h2>Try Sorbet in your browser</h2>
          <p>
            You can use the Sorbet playground to try features like type annotations and autocomplete.
            Go to <a href="http://sorbet.run">sorbet.run</a> to see more
            examples.
          </p>
          <iframe
            style={{ width: "100%", height: "400px" }}
            src={sorbetRunExample}
          />
        </PageSection>
        <PageSection gray>
          <h2>Powerful features to streamline your editing</h2>
          <p>
            Sorbet gives your Ruby development environment IDE-like features,
            including autocomplete, in-editor documentation, and go to
            definition.
          </p>
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
              <a href="https://docs.google.com/forms/d/1aj1XkNwdeNeX5gzm_cMWiKC1__vUQ29qBbBxHx6tNOc/viewform?edit_requested=true">
                sign up for our mailing list
              </a>{" "}
              and we'll reach out when we have more to share.
            </p>
          </div>
        </PageSection>
        <PageSection gray>
          <h2>Learn more about Sorbet</h2>
          <p>
            Watch a talk from <a href="https://www.thestrangeloop.com/2018/sessions.html">Strangeloop 2018</a>, or see <a href={docUrl('talks')}>more talks</a> by the Stripe team.
          </p>
          <iframe
            style={{marginTop: "0.5em"}}
            width="853"
            height="480"
            src="https://www.youtube.com/embed/uFFJyp8vXQI"
            frameBorder="0"
            allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture"
            allowFullScreen
          />
        </PageSection>
        <PageSection short lightPurple>
          <p className="footer" style={{margin: 0}}>
            <strong>Interested in getting involved?</strong>{" "}
            Join <a href="https://docs.google.com/forms/d/1aj1XkNwdeNeX5gzm_cMWiKC1__vUQ29qBbBxHx6tNOc/viewform?edit_requested=true">
            our mailing list</a>, and read more about Sorbet in our <a href={docUrl('overview')}>documentation</a>.</p>
        </PageSection>
      </div>
    );
  }
}

module.exports = Index;
