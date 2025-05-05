import React, { useState, useEffect } from "react";
import { Button, Card, CardContent } from "@/components/ui/card";

const ESPDashboard = () => {
  const [sensorData, setSensorData] = useState({
    temperature: 0,
    distance: 0,
    light: 0,
    gas: 0,
  });

  // Fetch sensor data from ESP
  const fetchSensorData = async () => {
    try {
      const response = await fetch("http://YOUR_ESP_IP_ADDRESS/sensor");
      const data = await response.json();
      setSensorData(data);
    } catch (error) {
      console.error("Error fetching sensor data:", error);
    }
  };

  // Send movement command to ESP
  const sendCommand = async (command) => {
    try {
      await fetch("http://YOUR_ESP_IP_ADDRESS/control", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ command }),
      });
    } catch (error) {
      console.error("Error sending command:", error);
    }
  };

  useEffect(() => {
    const interval = setInterval(fetchSensorData, 2000); // Fetch data every 2 seconds
    return () => clearInterval(interval);
  }, []);

  return (
    <div className="p-4">
      <h1 className="text-2xl font-bold mb-4">ESP Sensor Dashboard</h1>

      <div className="grid grid-cols-2 gap-4 mb-8">
        {Object.entries(sensorData).map(([key, value]) => (
          <Card key={key} className="p-4">
            <CardContent>
              <h2 className="text-xl capitalize">{key}</h2>
              <p className="text-3xl font-semibold">{value}</p>
            </CardContent>
          </Card>
        ))}
      </div>

      <div className="flex flex-col items-center gap-4">
        <Button onClick={() => sendCommand("forward")} className="px-6">Forward</Button>
        <div className="flex gap-4">
          <Button onClick={() => sendCommand("left")} className="px-6">Left</Button>
          <Button onClick={() => sendCommand("right")} className="px-6">Right</Button>
        </div>
        <Button onClick={() => sendCommand("back")} className="px-6">Back</Button>
      </div>
    </div>
  );
};

export default ESPDashboard;
