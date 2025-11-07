import express from "express";
import { WebSocketServer } from "ws";
import { spawn } from "child_process";
import path from "path";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
const PORT = process.env.PORT || 8080;

// Serve static files
app.use(express.static(path.join(__dirname, "public")));

// Routes for direct access
app.get("/", (req, res) => {
    res.sendFile(path.join(__dirname, "public", "index.html"));
});

app.get("/server", (req, res) => {
    res.sendFile(path.join(__dirname, "public", "server.html"));
});

app.get("/client", (req, res) => {
    res.sendFile(path.join(__dirname, "public", "client.html"));
});

// WebSocket server
const wss = new WebSocketServer({ noServer: true });

// Start HTTP server
const server = app.listen(PORT, () => {
    console.log(`🚀 SCP Live Server running at http://localhost:${PORT}`);
});

// Handle WebSocket upgrades
server.on("upgrade", (req, socket, head) => {
    wss.handleUpgrade(req, socket, head, (ws) => {
        if (req.url === "/server") {
            console.log("🟢 Launching SCP Server...");
            startProcess(path.join(__dirname, "bin", "scp_server_two_way.exe"), ws);
        } else if (req.url === "/client") {
            console.log("🟢 Launching SCP Client...");
            startProcess(path.join(__dirname, "bin", "scp_client_two_way.exe"), ws);
        }
    });
});

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
        process.stdin.write(msg.toString() + "\n");
    });
}
