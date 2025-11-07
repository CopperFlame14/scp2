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

// Start HTTP server
const server = app.listen(PORT, () => {
  console.log(`🚀 SCP Live Server running at http://localhost:${PORT}`);
});

// Handle WebSocket upgrades
server.on("upgrade", (req, socket, head) => {
  wss.handleUpgrade(req, socket, head, (ws) => {
    if (req.url === "/server") {
      console.log("🟢 Launching SCP Server binary...");
      startProcess(path.join(__dirname, "bin/scp_server_two_way.exe"), ws);
    } else if (req.url === "/client") {
      console.log("🟢 Launching SCP Client binary...");
      startProcess(path.join(__dirname, "bin/scp_client_two_way.exe"), ws);
    }
  });
});

// Function to spawn a C binary and stream logs
function startProcess(exePath, ws) {
  const process = spawn(exePath);

  process.stdout.on("data", (data) => {
    ws.send(data.toString());
  });

  process.stderr.on("data", (data) => {
    ws.send("[ERROR] " + data.toString());
  });

  process.on("close", (code) => {
    ws.send(`\nProcess exited with code ${code}`);
  });

  // Forward browser input → process stdin
  ws.on("message", (msg) => {
    process.stdin.write(msg + "\n");
  });
}
