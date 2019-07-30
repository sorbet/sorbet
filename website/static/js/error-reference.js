(() => {
  // Validate the string because we're about to inject it into the page.
  const errorCode = window.location.hash.substring(1);
  if (errorCode == null || errorCode.match(/\d\d\d\d/) == null) {
    return;
  }

  if (document.getElementById(errorCode) != null) {
    return;
  }

  const msg = `error code <strong>${errorCode}</strong>`;
  document.getElementById('missing-error-code').innerHTML = msg;

  const box = document.getElementById('missing-doc-for-error-code-box');
  box.classList.toggle('is-hidden');

  document.getElementById('missing-doc-for-error-code-scroll').scrollIntoView();
})();
