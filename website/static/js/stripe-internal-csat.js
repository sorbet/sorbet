(() => {
  if (document.querySelector('#csat-extension-config') != null) {
    return;
  }

  const selector = '.mainContainer.docsContainer > .wrapper';
  const main = document.querySelector(selector);
  if (main != null) {
    const csatDiv = document.createElement('div');
    csatDiv.id = 'csat-extension-config';
    const config = {
      embedded: true,
      prompt: 'Feedback for #sorbet?',
      notify: '#sorbet',
    };
    csatDiv.setAttribute('data-config', JSON.stringify(config));
    main.appendChild(csatDiv);
  }
})();
