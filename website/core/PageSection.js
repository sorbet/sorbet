const React = require('react');

// TODO(jez) No inline styles
const PageSection = (props) => {
  var className = 'pageSection';
  if (props.className && props.className != '') {
    className += props.className;
  }
  return (
    <div
      style={{
        backgroundColor: props.gray
          ? '#f5f5f5'
          : props.lightPurple
          ? '#6255b0'
          : 'white',
        color: props.lightPurple ? 'white' : 'inherit',
        padding: props.short ? '2em 0' : '3em 0',
      }}
      className={className}
    >
      <div className="wrapper">{props.children}</div>
    </div>
  );
};

module.exports = PageSection;
