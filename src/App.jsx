import React, { useState, useRef, useEffect } from 'react';
import { generateBeatFromPrompt, adjustBeatComponent, chatAboutBeat } from './services/gptService';
import './App.css'; // Optional: for styling

function App() {
  const [messages, setMessages] = useState([]);
  const [inputValue, setInputValue] = useState('');
  const [isLoading, setIsLoading] = useState(false);
  const [currentBeat, setCurrentBeat] = useState(null);
  const [conversationHistory, setConversationHistory] = useState([]);
  const messagesEndRef = useRef(null);

  // Auto-scroll to bottom when new messages arrive
  const scrollToBottom = () => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  };

  useEffect(() => {
    scrollToBottom();
  }, [messages]);

  // Add a message to the chat
  const addMessage = (text, sender = 'user') => {
    const newMessage = {
      id: Date.now(),
      text,
      sender,
      timestamp: new Date().toLocaleTimeString()
    };
    setMessages(prev => [...prev, newMessage]);
    return newMessage;
  };

  // Handle beat generation
  const handleGenerateBeat = async (prompt) => {
    setIsLoading(true);
    addMessage(prompt, 'user');
    
    // Add to conversation history
    const userMessage = { role: 'user', content: prompt };
    setConversationHistory(prev => [...prev, userMessage]);

    try {
      const result = await generateBeatFromPrompt(prompt);
      
      if (result.success) {
        setCurrentBeat(result.data);
        
        // Format beat description for display
        const beatInfo = `🎵 Beat Generated!\n\n` +
          `Tempo: ${result.data.tempo} BPM\n` +
          `Key: ${result.data.key}\n` +
          `Time Signature: ${result.data.timeSignature}\n` +
          `Style: ${result.data.style}\n\n` +
          `Instruments: ${result.data.instruments.map(i => i.name).join(', ')}\n\n` +
          `${result.data.description}`;
        
        addMessage(beatInfo, 'assistant');
        
        // Add assistant response to conversation history
        setConversationHistory(prev => [...prev, { 
          role: 'assistant', 
          content: beatInfo 
        }]);
      } else {
        addMessage(`❌ Error: ${result.error}`, 'assistant');
      }
    } catch (error) {
      addMessage(`❌ Error: ${error.message}`, 'assistant');
      console.error('Beat generation error:', error);
    } finally {
      setIsLoading(false);
    }
  };

  // Handle component adjustment
  const handleAdjustComponent = async (componentName, adjustmentPrompt) => {
    if (!currentBeat) {
      addMessage('Please generate a beat first!', 'assistant');
      return;
    }

    setIsLoading(true);
    addMessage(`Adjusting ${componentName}: ${adjustmentPrompt}`, 'user');
    
    setConversationHistory(prev => [...prev, { 
      role: 'user', 
      content: `Adjust ${componentName}: ${adjustmentPrompt}` 
    }]);

    try {
      const result = await adjustBeatComponent(componentName, adjustmentPrompt, currentBeat);
      
      if (result.success) {
        // Update the beat with the adjusted component
        const updatedBeat = { ...currentBeat };
        const componentIndex = updatedBeat.instruments.findIndex(
          inst => inst.name === componentName
        );
        
        if (componentIndex !== -1) {
          updatedBeat.instruments[componentIndex] = result.data;
        } else {
          updatedBeat.instruments.push(result.data);
        }
        
        setCurrentBeat(updatedBeat);
        addMessage(`✅ ${componentName} updated successfully!`, 'assistant');
        
        setConversationHistory(prev => [...prev, { 
          role: 'assistant', 
          content: `${componentName} has been adjusted.` 
        }]);
      } else {
        addMessage(`❌ Error: ${result.error}`, 'assistant');
      }
    } catch (error) {
      addMessage(`❌ Error: ${error.message}`, 'assistant');
      console.error('Component adjustment error:', error);
    } finally {
      setIsLoading(false);
    }
  };

  // Handle general chat
  const handleChat = async (message) => {
    setIsLoading(true);
    addMessage(message, 'user');
    
    setConversationHistory(prev => [...prev, { role: 'user', content: message }]);

    try {
      const result = await chatAboutBeat(message, conversationHistory);
      
      if (result.success) {
        addMessage(result.message, 'assistant');
        setConversationHistory(prev => [...prev, { 
          role: 'assistant', 
          content: result.message 
        }]);
      } else {
        addMessage(`❌ Error: ${result.error}`, 'assistant');
      }
    } catch (error) {
      addMessage(`❌ Error: ${error.message}`, 'assistant');
      console.error('Chat error:', error);
    } finally {
      setIsLoading(false);
    }
  };

  // Handle form submission
  const handleSubmit = async (e) => {
    e.preventDefault();
    if (!inputValue.trim() || isLoading) return;

    const prompt = inputValue.trim();
    setInputValue('');

    // Detect if it's a beat generation request or general chat
    const beatKeywords = ['beat', 'make', 'create', 'generate', 'produce', 'type of'];
    const isBeatRequest = beatKeywords.some(keyword => 
      prompt.toLowerCase().includes(keyword)
    );

    if (isBeatRequest) {
      await handleGenerateBeat(prompt);
    } else {
      await handleChat(prompt);
    }
  };

  // Handle component click for adjustment
  const handleComponentClick = (componentName) => {
    const adjustment = prompt(`Adjust ${componentName}:\n(e.g., "make it more punchy", "add ghost notes", "change the pattern")`);
    if (adjustment && adjustment.trim()) {
      handleAdjustComponent(componentName, adjustment.trim());
    }
  };

  return (
    <div style={{
      display: 'flex',
      flexDirection: 'column',
      height: '100vh',
      background: '#1a1a1a',
      color: '#ffffff',
      fontFamily: '-apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif'
    }}>
      {/* Header */}
      <div style={{
        padding: '20px',
        borderBottom: '1px solid #333',
        background: '#2a2a2a'
      }}>
        <h1 style={{ margin: 0, fontSize: '24px' }}>🎵 FL Studio AI Assistant</h1>
        <p style={{ margin: '5px 0 0 0', color: '#aaa', fontSize: '14px' }}>
          Your AI-powered beat production assistant
        </p>
      </div>

      {/* Messages Area */}
      <div style={{
        flex: 1,
        overflowY: 'auto',
        padding: '20px',
        display: 'flex',
        flexDirection: 'column',
        gap: '15px'
      }}>
        {messages.length === 0 && (
          <div style={{
            textAlign: 'center',
            color: '#888',
            marginTop: '50px'
          }}>
            <p>👋 Welcome! Try asking:</p>
            <p style={{ marginTop: '10px', fontSize: '14px' }}>
              "Create a gospel-sampling Kanye West type of beat"
            </p>
            <p style={{ fontSize: '14px' }}>
              or "What's a good BPM for trap beats?"
            </p>
          </div>
        )}

        {messages.map((message) => (
          <div
            key={message.id}
            style={{
              alignSelf: message.sender === 'user' ? 'flex-end' : 'flex-start',
              maxWidth: '70%',
              padding: '12px 16px',
              borderRadius: '12px',
              background: message.sender === 'user' ? '#4a9eff' : '#2a2a2a',
              whiteSpace: 'pre-wrap',
              wordWrap: 'break-word',
              fontSize: '14px',
              lineHeight: '1.5'
            }}
          >
            {message.text}
            <div style={{
              fontSize: '11px',
              opacity: 0.7,
              marginTop: '5px'
            }}>
              {message.timestamp}
            </div>
          </div>
        ))}

        {isLoading && (
          <div style={{
            alignSelf: 'flex-start',
            padding: '12px 16px',
            borderRadius: '12px',
            background: '#2a2a2a',
            color: '#888'
          }}>
            Thinking...
          </div>
        )}

        <div ref={messagesEndRef} />
      </div>

      {/* Current Beat Display */}
      {currentBeat && (
        <div style={{
          padding: '15px 20px',
          borderTop: '1px solid #333',
          background: '#252525',
          maxHeight: '150px',
          overflowY: 'auto'
        }}>
          <div style={{ fontSize: '12px', color: '#aaa', marginBottom: '8px' }}>
            Current Beat: {currentBeat.style} • {currentBeat.tempo} BPM • {currentBeat.key}
          </div>
          <div style={{ display: 'flex', gap: '10px', flexWrap: 'wrap' }}>
            {currentBeat.instruments.map((instrument, idx) => (
              <button
                key={idx}
                onClick={() => handleComponentClick(instrument.name)}
                style={{
                  padding: '6px 12px',
                  background: '#4a9eff',
                  border: 'none',
                  borderRadius: '6px',
                  color: 'white',
                  cursor: 'pointer',
                  fontSize: '12px',
                  transition: 'background 0.2s'
                }}
                onMouseOver={(e) => e.target.style.background = '#5aaeff'}
                onMouseOut={(e) => e.target.style.background = '#4a9eff'}
                title="Click to adjust this component"
              >
                {instrument.name}
              </button>
            ))}
          </div>
        </div>
      )}

      {/* Input Form */}
      <form onSubmit={handleSubmit} style={{
        padding: '20px',
        borderTop: '1px solid #333',
        background: '#2a2a2a'
      }}>
        <div style={{ display: 'flex', gap: '10px' }}>
          <input
            type="text"
            value={inputValue}
            onChange={(e) => setInputValue(e.target.value)}
            placeholder="Ask me to create a beat or ask a question..."
            disabled={isLoading}
            style={{
              flex: 1,
              padding: '12px 16px',
              fontSize: '14px',
              borderRadius: '8px',
              border: '1px solid #444',
              background: '#1a1a1a',
              color: '#fff',
              outline: 'none'
            }}
            onFocus={(e) => e.target.style.borderColor = '#4a9eff'}
            onBlur={(e) => e.target.style.borderColor = '#444'}
          />
          <button
            type="submit"
            disabled={isLoading || !inputValue.trim()}
            style={{
              padding: '12px 24px',
              fontSize: '14px',
              borderRadius: '8px',
              border: 'none',
              background: isLoading || !inputValue.trim() ? '#444' : '#4a9eff',
              color: '#fff',
              cursor: isLoading || !inputValue.trim() ? 'not-allowed' : 'pointer',
              fontWeight: '500',
              transition: 'background 0.2s'
            }}
          >
            {isLoading ? '...' : 'Send'}
          </button>
        </div>
      </form>
    </div>
  );
}

export default App;