import React, { useState, useEffect } from 'react';
import Settings from './Settings';

const SensorDashboard = () => {
  const [sensorData, setSensorData] = useState({
    temperature: 0,
    distance: 0,
    light: 0,
    gas: 0,
  });

  const [isSettingsOpen, setIsSettingsOpen] = useState(false);
  const [settings, setSettings] = useState({
    tempUnit: 'C',
    distanceUnit: 'cm',
    lightUnit: 'lux',
    gasUnit: 'percent'
  });

  const convertTemperature = (celsius) => {
    switch (settings.tempUnit) {
      case 'F':
        return (celsius * 9/5) + 32;
      case 'K':
        return celsius + 273.15;
      default:
        return celsius;
    }
  };

  const convertDistance = (cm) => {
    switch (settings.distanceUnit) {
      case 'm':
        return cm / 100;
      case 'in':
        return cm / 2.54;
      case 'ft':
        return cm / 30.48;
      default:
        return cm;
    }
  };

  const convertLight = (lux) => {
    if (settings.lightUnit === 'percent') {
      // Assuming max light level is 10000 lux
      return (lux / 10000) * 100;
    }
    return lux;
  };

  const convertGas = (percentage) => {
    if (settings.gasUnit === 'ppm') {
      // Convert percentage to PPM (1% = 10000 PPM)
      return percentage * 10000;
    }
    return percentage;
  };

  const getUnitSymbol = (type) => {
    switch (type) {
      case 'temp':
        return settings.tempUnit === 'C' ? '°C' : 
               settings.tempUnit === 'F' ? '°F' : 'K';
      case 'distance':
        return settings.distanceUnit;
      case 'light':
        return settings.lightUnit === 'lux' ? 'lux' : '%';
      case 'gas':
        return settings.gasUnit === 'percent' ? '%' : 'ppm';
      default:
        return '';
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

  // Fetch data when component mounts
  useEffect(() => {
    getData();
    const interval = setInterval(getData, 5000); // Fetch every 5 seconds
    return () => clearInterval(interval); // Cleanup on unmount
  }, []);

  return (
    <>
      <div className="sensor-box">
        <div className="sensor-header">
          <h2 className="section-title">Sensor Readings</h2>
          <button 
            className="settings-button"
            onClick={() => setIsSettingsOpen(true)}
          >
            ⚙️
          </button>
        </div>
        <div className="sensor-grid">
          <div className="sensor-item">
            🌡️ Temperature: <strong>
              {convertTemperature(sensorData.temperature).toFixed(1)}
              {getUnitSymbol('temp')}
            </strong>
          </div>
          <div className="sensor-item">
            💡 Light: <strong>
              {convertLight(sensorData.light).toFixed(1)}
              {getUnitSymbol('light')}
            </strong>
          </div>
          <div className="sensor-item">
            🛑 Gas: <strong>
              {convertGas(sensorData.gas).toFixed(1)}
              {getUnitSymbol('gas')}
            </strong>
          </div>
          <div className="sensor-item">
            📏 Distance: <strong>
              {convertDistance(sensorData.distance).toFixed(1)}
              {getUnitSymbol('distance')}
            </strong>
          </div>
        </div>
      </div>

      <Settings 
        isOpen={isSettingsOpen}
        onClose={() => setIsSettingsOpen(false)}
        settings={settings}
        onSave={setSettings}
      />
    </>
  );
};

export default SensorDashboard; 