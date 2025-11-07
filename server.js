// server.js — Node backend to stream SCP client/server outputs

import express from "express";
import { WebSocketServer } from "ws";
import { spawn } from "child_process";
import path from "path";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
const PORT = 8080;

// Serve static files (HTML, CSS, JS)
app.use(express.static(path.join(__dirname, "public")));

// Create WebSocket server
const wss = new WebSocketServer({ noServer: true });

// Store active connections and their codes
const connections = new Map();

// Start HTTP server
const server = app.listen(PORT, () => {
  console.log(`🚀 SCP Live Server running at http://localhost:${PORT}`);
});

// Handle WebSocket upgrades
server.on("upgrade", (req, socket, head) => {
  wss.handleUpgrade(req, socket, head, (ws) => {
    if (req.url === "/server") {
      console.log("🟢 Launching SCP Server binary...");
      startProcess(path.join(__dirname, "bin/scp_server_two_way.exe"), ws, "server");
    } else if (req.url === "/client") {
      console.log("🟢 Launching SCP Client binary...");
      startProcess(path.join(__dirname, "bin/scp_client_two_way.exe"), ws, "client");
    }
  });
});

// Function to spawn a C binary and stream logs
function startProcess(exePath, ws, type) {
  const process = spawn(exePath);

  process.stdout.on("data", (data) => {
    const output = data.toString();
    ws.send(output);
    
    // Log connection codes for debugging
    if (output.includes("CONNECT_CODE:")) {
      console.log(`Connection code received: ${output}`);
    }
  });

  process.stderr.on("data", (data) => {
    ws.send("[ERROR] " + data.toString());
  });

  process.on("close", (code) => {
    ws.send(`\nProcess exited with code ${code}`);
  });

  // Forward browser input → process stdin
  ws.on("message", (msg) => {
    const message = msg.toString();
    
    // Handle connection code for client
    if (type === "client" && message.startsWith("CONNECT_CODE:")) {
      const code = message.replace("CONNECT_CODE:", "").trim();
      console.log(`Client attempting connection with code: ${code}`);
      // Here you can add validation logic for the connection code
    }
    
    process.stdin.write(message + "\n");
  });
}

// API endpoint to validate connection codes (optional enhancement)
app.get("/validate-code/:code", (req, res) => {
  const code = req.params.code;
  // Add your validation logic here
  res.json({ valid: true, code: code });
});
