import React, { useState } from 'react';

const QueueMode = () => {
  const [commandQueue, setCommandQueue] = useState([]);

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

  return (
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
  );
};

export default QueueMode; 