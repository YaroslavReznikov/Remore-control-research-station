import React, { useState, useEffect } from "react";

const App = () => {
  const [sensorData, setSensorData] = useState({
    temperature: 0,
    distance: 0,
    light: 0,
    gas: 0,
  });

  // Fetch sensor data from ESP

  // Send movement command to E
  const sendCommand = async (command) => {
    try {
      console.log(command)
      await fetch("http://192.168.204.92/control", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ command }),
      });
    } catch (error) {
      console.error("Error sending command:", error);
    }
  };


  return (
    <div className="p-4">
      <h1 className="text-2xl font-bold mb-4">ESP Sensor Dashboard</h1>

      <div className="flex flex-col items-center gap-4">
        <button onClick={() => sendCommand("forward")} className="px-6">Forward</button>
        <div className="flex gap-4">
          <button onClick={() => sendCommand("left")} className="px-6">Left</button>
          <button onClick={() => sendCommand("right")} className="px-6">Right</button>
        </div>
        <button onClick={() => sendCommand("back")} className="px-6">Back</button>
      </div>
    </div>
  );
};



export default App;

