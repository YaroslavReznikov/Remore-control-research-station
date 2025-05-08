import React, { useState, useEffect } from "react";

const App = () => {
  const [sensorData, setSensorData] = useState({
    temperature: 0,
    distance: 0,
    light: 0,
    gas: 0,
  });

  // Fetch sensor data from ESP
  useEffect(() => {
    getData(); // Initial fetch immediately
    const interval = setInterval(getData, 2000); // Every 2 seconds

    return () => clearInterval(interval); // Cleanup on unmount
  }, []);
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

  const getData = async () => {
    try {
      console.log("Asked");
      const response = await fetch("http://192.168.204.92/control", {
        method: "GET"
      });
  
      const data = await response.json();
      setSensorData(data);
      console.log("Data gotten")
      console.log(data)
    } catch (error) {
      console.error("Error getting data", error);
    }
  };


  return (
    <div className="p-4">
      <h1 className="text-2xl font-bold mb-4">ESP Sensor Dashboard</h1>

      <div className="flex flex-col items-center gap-4">
        <button onClick={() => sendCommand("forward")} className="px-6">Forward</button>
        <div className="flex gap-4">
          <button onClick={() => getData()} className="px-6">Left</button>
          <button onClick={() => sendCommand("right")} className="px-6">Right</button>
        </div>
        <button onClick={() => sendCommand("back")} className="px-6">Back</button>
      </div>
    </div>
  );
};



export default App;

