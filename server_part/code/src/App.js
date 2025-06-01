import React, { useState, useEffect } from "react";
import './index.css';

const App = () => {
  const [sensorData, setSensorData] = useState({
    temperature: 0,
    distance: 0,
    light: 0,
    gas: 0,
  });

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
        <div className="controls">
          <button onClick={() => sendCommand("forward")} className="control-button">Forward</button>
          <div className="control-row">
            <button onClick={() => sendCommand("left")} className="control-button">Left</button>
            <button onClick={() => sendCommand("right")} className="control-button">Right</button>
          </div>
          <button onClick={() => sendCommand("back")} className="control-button">Back</button>
        </div>
      </div>
    </div>
  );
};

export default App;