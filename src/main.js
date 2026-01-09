const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const OpenAI = require('openai');
require('dotenv').config();

const isDev = process.env.NODE_ENV === 'development' || !app.isPackaged;

// Initialize OpenAI in main process (secure - API key never exposed to renderer)
const openai = new OpenAI({
  apiKey: process.env.OPENAI_API_KEY,
});

// Suppress security warnings in development
if (isDev) {
  process.env.ELECTRON_DISABLE_SECURITY_WARNINGS = '1';
}

function createWindow() {
  const mainWindow = new BrowserWindow({
    width: 1200,
    height: 800,
    webPreferences: {
      nodeIntegration: false, // Security: disable node in renderer
      contextIsolation: true, // Security: enable context isolation
      preload: path.join(__dirname, 'preload.js'), // Load preload script
    }
  });

  if (isDev) {
    mainWindow.loadURL('http://localhost:5173');
    mainWindow.webContents.openDevTools();
  } else {
    mainWindow.loadFile(path.join(__dirname, '../dist/index.html'));
  }
}

// IPC Handlers for GPT API calls (secure - runs in main process)
ipcMain.handle('gpt:generateBeat', async (event, prompt) => {
  const systemPrompt = `You are an expert music production assistant specializing in beat creation. 
Analyze the user's beat request and return a structured JSON response with the following format:
{
  "tempo": 120,
  "timeSignature": "4/4",
  "key": "C minor",
  "style": "description of style",
  "instruments": [
    {
      "name": "Kick",
      "pattern": [1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0],
      "notes": ["C2"],
      "velocity": 100
    },
    {
      "name": "Snare",
      "pattern": [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0],
      "notes": ["D2"],
      "velocity": 80
    }
  ],
  "description": "detailed description of the beat"
}

Pattern arrays use 1 for hit and 0 for rest, representing 16th note subdivisions.
Return ONLY valid JSON, no additional text.`;

  try {
    const completion = await openai.chat.completions.create({
      model: "gpt-4o-mini",
      messages: [
        { role: "system", content: systemPrompt },
        { role: "user", content: prompt }
      ],
      temperature: 0.7,
      response_format: { type: "json_object" }
    });

    const responseText = completion.choices[0].message.content;
    const beatDescription = JSON.parse(responseText);
    
    return {
      success: true,
      data: beatDescription
    };
  } catch (error) {
    console.error('Error calling GPT:', error);
    return {
      success: false,
      error: error.message
    };
  }
});

ipcMain.handle('gpt:adjustComponent', async (event, componentName, adjustmentPrompt, currentBeat) => {
  const systemPrompt = `You are a music production assistant. The user wants to adjust a specific component of their beat.
Given the current beat structure and the user's request, return ONLY a JSON object with the updated component:
{
  "name": "ComponentName",
  "pattern": [array of 1s and 0s],
  "notes": ["note1", "note2"],
  "velocity": 80
}

Return ONLY valid JSON, no additional text.`;

  const userPrompt = `Current beat: ${JSON.stringify(currentBeat)}
Component to adjust: ${componentName}
User request: ${adjustmentPrompt}`;

  try {
    const completion = await openai.chat.completions.create({
      model: "gpt-4o-mini",
      messages: [
        { role: "system", content: systemPrompt },
        { role: "user", content: userPrompt }
      ],
      temperature: 0.7,
      response_format: { type: "json_object" }
    });

    const responseText = completion.choices[0].message.content;
    const updatedComponent = JSON.parse(responseText);
    
    return {
      success: true,
      data: updatedComponent
    };
  } catch (error) {
    console.error('Error adjusting component:', error);
    return {
      success: false,
      error: error.message
    };
  }
});

ipcMain.handle('gpt:chat', async (event, message, conversationHistory = []) => {
  const systemPrompt = `You are an expert music production assistant helping users create beats in FL Studio.
You understand music theory, drum patterns, sound design, and can help with:
- Beat creation and arrangement
- Sound selection and synthesis
- Mixing and effects
- Music theory questions
- FL Studio specific workflows

Be helpful, concise, and musically accurate.`;

  const messages = [
    { role: "system", content: systemPrompt },
    ...conversationHistory,
    { role: "user", content: message }
  ];

  try {
    const completion = await openai.chat.completions.create({
      model: "gpt-4o-mini",
      messages: messages,
      temperature: 0.8
    });

    const response = completion.choices[0].message.content;
    
    return {
      success: true,
      message: response
    };
  } catch (error) {
    console.error('Error in chat:', error);
    return {
      success: false,
      error: error.message
    };
  }
});

app.whenReady().then(() => {
  createWindow();

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});