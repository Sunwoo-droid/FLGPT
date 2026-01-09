/**
 * Generate a beat description from a user prompt
 * @param {string} prompt - User's beat request (e.g., "a gospel-sampling kanye west type of beat")
 * @returns {Promise<Object>} Structured beat description
 */
export async function generateBeatFromPrompt(prompt) {
    if (!window.electronAPI) {
      throw new Error('Electron API not available. Make sure you\'re running in Electron.');
    }
    
    return await window.electronAPI.generateBeat(prompt);
  }
  
  /**
   * Refine or adjust a specific component of the beat
   * @param {string} componentName - Name of the component to adjust (e.g., "Kick", "Snare")
   * @param {string} adjustmentPrompt - User's adjustment request
   * @param {Object} currentBeat - Current beat description
   * @returns {Promise<Object>} Updated component data
   */
  export async function adjustBeatComponent(componentName, adjustmentPrompt, currentBeat) {
    if (!window.electronAPI) {
      throw new Error('Electron API not available. Make sure you\'re running in Electron.');
    }
    
    return await window.electronAPI.adjustComponent(componentName, adjustmentPrompt, currentBeat);
  }
  
  /**
   * General chat function for beat production questions
   * @param {string} message - User's message
   * @param {Array} conversationHistory - Previous messages in the conversation
   * @returns {Promise<Object>} GPT response
   */
  export async function chatAboutBeat(message, conversationHistory = []) {
    if (!window.electronAPI) {
      throw new Error('Electron API not available. Make sure you\'re running in Electron.');
    }
    
    return await window.electronAPI.chatAboutBeat(message, conversationHistory);
  }