/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const React = require('react');

const CompLibrary = require('../../core/CompLibrary.js');

const MarkdownBlock = CompLibrary.MarkdownBlock; /* Used to read markdown */
const Container = CompLibrary.Container;
const GridBlock = CompLibrary.GridBlock;

const sorbetRunExample = "https://sorbet.run/#%23%20typed%3A%20true%0A%0A%23%20This%20code%20is%20editable!%0A%23%20You%20can%20also%20hover%20to%20see%20error%20messages%20and%20types!%0A%0Aextend%20T%3A%3ASig%0A%0Asig%20%7Bparams(name%3A%20String).returns(Integer)%7D%0Adef%20main(name)%0A%20%20puts%20%22Hello%2C%20%23%7Bname%7D!%22%0A%20%20name.length%0Aend%0A%0Amain(%22Sorbet%22)%20%23%20ok!%0Amain()%20%20%20%23%20error%3A%20Not%20enough%20arguments%20provided%20for%20method%20%60main%60%0Aman(%22%22)%20%20%23%20error%3A%20Method%20%60man%60%20does%20not%20exist";

class HomeSplash extends React.Component {
  render() {
    const {siteConfig, language = ''} = this.props;
    const {baseUrl, docsUrl} = siteConfig;
    const docsPart = `${docsUrl ? `${docsUrl}/` : ''}`;
    const langPart = `${language ? `${language}/` : ''}`;
    const docUrl = doc => `${baseUrl}${docsPart}${langPart}${doc}`;

    const SplashContainer = props => (
      <div className="homeContainer">
        <div className="homeSplashFade">
          <div className="wrapper homeWrapper">{props.children}</div>
        </div>
      </div>
    );

    const Logo = props => (
      <div className="projectLogo">
        <img src={props.img_src} alt="Project Logo" />
      </div>
    );

    const ProjectTitle = () => (
      <h2 className="projectTitle">
        {siteConfig.title}
        <small>{siteConfig.tagline}</small>
      </h2>
    );

    const PromoSection = props => (
      <div className="section promoSection">
        <div className="promoRow">
          <div className="pluginRowBlock">{props.children}</div>
        </div>
      </div>
    );

    const Button = props => (
      <div className="pluginWrapper buttonWrapper">
        <a className="button" href={props.href} target={props.target}>
          {props.children}
        </a>
      </div>
    );

    return (
      <SplashContainer>
        <Logo img_src={`${baseUrl}img/sorbet-logo-purple-sparkles.svg`} />
        <div className="inner">
          <ProjectTitle siteConfig={siteConfig} />
          <PromoSection>
            <Button href={docUrl('adopting')}>Get started</Button>
            <Button href={docUrl('overview')}>Read the docs</Button>
          </PromoSection>
        </div>
      </SplashContainer>
    );
  }
}

class Index extends React.Component {
  render() {
    const {config: siteConfig, language = ''} = this.props;

    return (
      <div>
        <HomeSplash siteConfig={siteConfig} language={language} />
        <div className="mainContainer">
        <Container align="center">
          <iframe style={{width: '100%', height: '576px'}} src={sorbetRunExample} />
        </Container>
        </div>
      </div>
    );
  }
}

module.exports = Index;
