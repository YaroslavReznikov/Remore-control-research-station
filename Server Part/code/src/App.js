import React, { useState, useEffect } from "react";
import './index.css';

const App = () => {
  const [sensorData, setSensorData] = useState({
    temperature: 0,
    distance: 0,
    light: 0,
    gas: 0,
  });
//


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
    <div className="min-h-screen bg-gray-100 p-6">
      <h1 className="text-3xl font-bold text-center text-blue-700 mb-6">ESP Sensor Dashboard</h1>

      <div className="max-w-2xl mx-auto bg-white shadow-md rounded-xl p-6">
        <h2 className="text-xl font-semibold mb-4">Sensor Readings</h2>
        <div className="grid grid-cols-2 gap-4 text-lg">
          <div className="bg-blue-100 p-4 rounded-lg">🌡️ Temperature: <strong>{sensorData.temperature}°C</strong></div>
          <div className="bg-yellow-100 p-4 rounded-lg">💡 Light: <strong>{sensorData.light}</strong></div>
          <div className="bg-green-100 p-4 rounded-lg">🛑 Gas: <strong>{sensorData.gas}</strong></div>
          <div className="bg-purple-100 p-4 rounded-lg">📏 Distance: <strong>{sensorData.distance} cm</strong></div>
        </div>
      </div>

      <div className="max-w-md mx-auto mt-8 text-center">
        <h2 className="text-xl font-semibold mb-4">Movement Controls</h2>
        <div className="flex flex-col items-center gap-3">
          <button
            onClick={() => sendCommand("forward")}
            className="bg-blue-600 text-white px-6 py-2 rounded hover:bg-blue-700"
          >
            Forward
          </button>
          <div className="flex gap-6">
            <button
              onClick={() => sendCommand("left")}
              className="bg-blue-600 text-white px-6 py-2 rounded hover:bg-blue-700"
            >
              Left
            </button>
            <button
              onClick={() => sendCommand("right")}
              className="bg-blue-600 text-white px-6 py-2 rounded hover:bg-blue-700"
            >
              Right
            </button>
          </div>
          <button
            onClick={() => sendCommand("back")}
            className="bg-blue-600 text-white px-6 py-2 rounded hover:bg-blue-700"
          >
            Back
          </button>
        </div>
      </div>
      
    </div>
  );
};

export default App;