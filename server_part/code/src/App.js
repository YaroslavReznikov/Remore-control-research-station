import React, { useState } from "react";
import DirectMode from './DirectMode';
import QueueMode from './QueueMode';
import SensorDashboard from './SensorDashboard';
import './index.css';

const App = () => {
  const [isQueueMode, setIsQueueMode] = useState(false);

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

      <SensorDashboard />

      <div className="controls-box">
        <h2 className="section-title">Movement Controls</h2>
        {isQueueMode ? <QueueMode /> : <DirectMode />}
      </div>
    </div>
  );
};

export default App;