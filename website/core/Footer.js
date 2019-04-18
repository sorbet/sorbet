const React = require('react');

class Footer extends React.Component {
  render() {
      return <>
          <script async src="https://www.googletagmanager.com/gtag/js?id=UA-119877071-2"></script>
          <script>{injectGA()}</script>
      </>;
  }
}
const injectGA = () => {
    if (typeof window == 'undefined') {
        return;
    }
    window.dataLayer = window.dataLayer || [];
    function gtag(){dataLayer.push(arguments);}
    gtag('js', new Date());

    gtag('config', 'UA-119877071-2');
}

module.exports = Footer;
