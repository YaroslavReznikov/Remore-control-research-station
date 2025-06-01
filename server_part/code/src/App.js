import React, { useState, useEffect } from "react";
import './index.css';

const App = () => {
  const [sensorData, setSensorData] = useState({
    temperature: 0,
    distance: 0,
    light: 0,
    gas: 0,
  });

  const [isQueueMode, setIsQueueMode] = useState(false);
  const [commandQueue, setCommandQueue] = useState([]);

  const sendCommand = async (command) => {
    try {
      console.log("Sending command:", command);
      await fetch("http://192.168.204.92/control", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ command }),
      });
    } catch (error) {
      console.error("Error sending command:", error);
    }
  };

  const sendQueue = async () => {
    try {
      console.log("Sending command queue:", commandQueue);
      await fetch("http://192.168.204.92/control", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ commands: commandQueue }),
      });
      setCommandQueue([]); // Clear queue after sending
    } catch (error) {
      console.error("Error sending command queue:", error);
    }
  };

  const addToQueue = (command) => {
    setCommandQueue([...commandQueue, command]);
  };

  const removeFromQueue = (index) => {
    const newQueue = commandQueue.filter((_, i) => i !== index);
    setCommandQueue(newQueue);
  };

  const getData = async () => {
    try {
      console.log("Requesting data...");
      const response = await fetch("http://192.168.204.92/control", {
        method: "GET",
      });
      const data = await response.json();
      setSensorData(data);
      console.log("Received data:", data);
    } catch (error) {
      console.error("Error getting data", error);
    }
  };

  return (
    <div className="app-container">
      <h1 className="app-title">ESP Sensor Dashboard</h1>

      <div className="mode-switch">
        <button 
          className={`mode-button ${!isQueueMode ? 'active' : ''}`}
          onClick={() => setIsQueueMode(false)}
        >
          Direct Mode
        </button>
        <button 
          className={`mode-button ${isQueueMode ? 'active' : ''}`}
          onClick={() => setIsQueueMode(true)}
        >
          Queue Mode
        </button>
      </div>

      <div className="sensor-box">
        <h2 className="section-title">Sensor Readings</h2>
        <div className="sensor-grid">
          <div className="sensor-item">🌡️ Temperature: <strong>{sensorData.temperature}°C</strong></div>
          <div className="sensor-item">💡 Light: <strong>{sensorData.light}</strong></div>
          <div className="sensor-item">🛑 Gas: <strong>{sensorData.gas}</strong></div>
          <div className="sensor-item">📏 Distance: <strong>{sensorData.distance} cm</strong></div>
        </div>
      </div>

      <div className="controls-box">
        <h2 className="section-title">Movement Controls</h2>
        {isQueueMode ? (
          <>
            <div className="controls">
              <button onClick={() => addToQueue("forward")} className="control-button">↑</button>
              <div className="control-row">
                <button onClick={() => addToQueue("left")} className="control-button">←</button>
                <button onClick={() => addToQueue("right")} className="control-button">→</button>
              </div>
              <button onClick={() => addToQueue("back")} className="control-button">↓</button>
            </div>
            
            <div className="queue-container">
              <h3 className="queue-title">Command Queue</h3>
              <div className="queue-list">
                {commandQueue.map((command, index) => (
                  <div key={index} className="queue-item">
                    <span className="command-text">
                      {command === "forward" ? "↑" :
                       command === "back" ? "↓" :
                       command === "left" ? "←" : "→"}
                    </span>
                    <button 
                      onClick={() => removeFromQueue(index)}
                      className="remove-button"
                    >
                      ×
                    </button>
                  </div>
                ))}
              </div>
              {commandQueue.length > 0 && (
                <button 
                  onClick={sendQueue}
                  className="send-queue-button"
                >
                  Send Commands
                </button>
              )}
            </div>
          </>
        ) : (
          <div className="controls">
            <button onClick={() => sendCommand("forward")} className="control-button">↑</button>
            <div className="control-row">
              <button onClick={() => sendCommand("left")} className="control-button">←</button>
              <button onClick={() => sendCommand("stop")} className="control-button stop-button">⏹</button>
              <button onClick={() => sendCommand("right")} className="control-button">→</button>
            </div>
            <button onClick={() => sendCommand("back")} className="control-button">↓</button>
          </div>
        )}
      </div>
    </div>
  );
};

export default App;