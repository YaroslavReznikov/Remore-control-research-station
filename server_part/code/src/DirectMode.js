import React from 'react';

const DirectMode = () => {
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

  return (
    <div className="controls">
      <button onClick={() => sendCommand("forward")} className="control-button">↑</button>
      <div className="control-row">
        <button onClick={() => sendCommand("left")} className="control-button">←</button>
        <button onClick={() => sendCommand("stop")} className="control-button stop-button">⏹</button>
        <button onClick={() => sendCommand("right")} className="control-button">→</button>
      </div>
      <button onClick={() => sendCommand("back")} className="control-button">↓</button>
    </div>
  );
};

export default DirectMode; 