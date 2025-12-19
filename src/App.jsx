const React = require('react');

function App() {
  return React.createElement('div', {
    style: {
      padding: '40px',
      maxWidth: '800px',
      margin: '0 auto'
    }
  }, [
    React.createElement('h1', { key: 'title' }, 'FL Studio AI Assistant'),
    React.createElement('p', { key: 'subtitle' }, 'Welcome! Your AI-powered beat production assistant.'),
    React.createElement('div', {
      key: 'chat',
      style: {
        marginTop: '30px',
        padding: '20px',
        background: '#2a2a2a',
        borderRadius: '8px'
      }
    }, [
      React.createElement('h2', { key: 'chat-title' }, 'Chat Interface'),
      React.createElement('input', {
        key: 'input',
        type: 'text',
        placeholder: 'Ask me anything about beat production...',
        style: {
          width: '100%',
          padding: '12px',
          fontSize: '16px',
          borderRadius: '4px',
          border: 'none',
          marginTop: '10px'
        }
      })
    ])
  ]);
}

module.exports = App;