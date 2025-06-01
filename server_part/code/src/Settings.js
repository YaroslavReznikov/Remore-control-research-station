import React, { useState } from 'react';

const Settings = ({ isOpen, onClose, settings, onSave }) => {
  const [tempUnit, setTempUnit] = useState(settings.tempUnit);
  const [distanceUnit, setDistanceUnit] = useState(settings.distanceUnit);
  const [lightUnit, setLightUnit] = useState(settings.lightUnit);
  const [gasUnit, setGasUnit] = useState(settings.gasUnit);

  const handleSave = () => {
    onSave({
      tempUnit,
      distanceUnit,
      lightUnit,
      gasUnit
    });
    onClose();
  };

  if (!isOpen) return null;

  return (
    <div className="settings-modal">
      <div className="settings-content">
        <h2>Settings</h2>
        
        <div className="settings-group">
          <label>Temperature</label>
          <select value={tempUnit} onChange={(e) => setTempUnit(e.target.value)}>
            <option value="C">Celsius (°C)</option>
            <option value="F">Fahrenheit (°F)</option>
            <option value="K">Kelvin (K)</option>
          </select>
        </div>

        <div className="settings-group">
          <label>Distance</label>
          <select value={distanceUnit} onChange={(e) => setDistanceUnit(e.target.value)}>
            <option value="cm">Centimeters (cm)</option>
            <option value="m">Meters (m)</option>
            <option value="in">Inches (in)</option>
            <option value="ft">Feet (ft)</option>
          </select>
        </div>

        <div className="settings-group">
          <label>Light Level</label>
          <select value={lightUnit} onChange={(e) => setLightUnit(e.target.value)}>
            <option value="lux">Lux</option>
            <option value="percent">Percentage (%)</option>
          </select>
        </div>

        <div className="settings-group">
          <label>Gas Level</label>
          <select value={gasUnit} onChange={(e) => setGasUnit(e.target.value)}>
            <option value="percent">Percentage (%)</option>
            <option value="ppm">PPM</option>
          </select>
        </div>

        <div className="settings-buttons">
          <button onClick={onClose} className="cancel-button">Cancel</button>
          <button onClick={handleSave} className="save-button">Save</button>
        </div>
      </div>
    </div>
  );
};

export default Settings; 