import React, { useState } from 'react';

const QueueMode = () => {
  const [commandQueue, setCommandQueue] = useState([]);
  const [isLimitReached, setIsLimitReached] = useState(false);
  const QUEUE_LIMIT = 40;

  const sendQueue = async () => {
    try {
      console.log("Sending command queue:", commandQueue);
      await fetch("http://192.168.204.92/control", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ commands: commandQueue }),
      });
      setCommandQueue([]); // Clear queue after sending
      setIsLimitReached(false);
    } catch (error) {
      console.error("Error sending command queue:", error);
    }
  };

  const addToQueue = (command) => {
    if (commandQueue.length >= QUEUE_LIMIT) {
      setIsLimitReached(true);
      setTimeout(() => setIsLimitReached(false), 2000); // Hide warning after 2 seconds
      return;
    }
    setCommandQueue([...commandQueue, command]);
  };

  const removeFromQueue = (index) => {
    const newQueue = commandQueue.filter((_, i) => i !== index);
    setCommandQueue(newQueue);
    setIsLimitReached(false);
  };

  return (
    <div className="controls-and-queue">
      <div className="controls">
        <button 
          onClick={() => addToQueue("forward")} 
          className={`control-button ${isLimitReached ? 'disabled' : ''}`}
          disabled={isLimitReached}
        >↑</button>
        <div className="control-row">
          <button 
            onClick={() => addToQueue("left")} 
            className={`control-button ${isLimitReached ? 'disabled' : ''}`}
            disabled={isLimitReached}
          >←</button>
          <button 
            onClick={() => addToQueue("right")} 
            className={`control-button ${isLimitReached ? 'disabled' : ''}`}
            disabled={isLimitReached}
          >→</button>
        </div>
        <button 
          onClick={() => addToQueue("back")} 
          className={`control-button ${isLimitReached ? 'disabled' : ''}`}
          disabled={isLimitReached}
        >↓</button>
      </div>
      
      <div className="queue-container">
        <div className="queue-header">
          <h3 className="queue-title">Command Queue</h3>
          <span className="queue-count">
            {commandQueue.length}/{QUEUE_LIMIT}
          </span>
        </div>
        {isLimitReached && (
          <div className="queue-limit-warning">
            Queue limit reached (max {QUEUE_LIMIT} commands)
          </div>
        )}
        <div className="queue-list">
          {commandQueue.map((command, index) => (
            <div 
              key={index} 
              className="queue-item"
              style={{ gridRow: Math.floor(index / 8) + 1, gridColumn: (index % 8) + 1 }}
            >
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
    </div>
  );
};

export default QueueMode; 