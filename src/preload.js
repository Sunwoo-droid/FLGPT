const { contextBridge, ipcRenderer } = require('electron');

// Expose protected methods that allow the renderer process to use
// the ipcRenderer without exposing the entire object
contextBridge.exposeInMainWorld('electronAPI', {
  // GPT API methods
  generateBeat: (prompt) => ipcRenderer.invoke('gpt:generateBeat', prompt),
  adjustComponent: (componentName, adjustmentPrompt, currentBeat) => 
    ipcRenderer.invoke('gpt:adjustComponent', componentName, adjustmentPrompt, currentBeat),
  chatAboutBeat: (message, conversationHistory) => 
    ipcRenderer.invoke('gpt:chat', message, conversationHistory),
});
