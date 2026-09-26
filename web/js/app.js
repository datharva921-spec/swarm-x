const canvas = document.getElementById('simCanvas');
const ctx = canvas.getContext('2d');

let GRID_WIDTH = 10;
let GRID_HEIGHT = 10;
let CELL_SIZE = canvas.width / GRID_WIDTH;

let autoRunInterval = null;
let tickSpeed = 400;
let latestState = null;

// Logging Utility
function logEvent(message, type = 'event') {
    const logContainer = document.getElementById('event-log');
    if (!logContainer) return;
    const entry = document.createElement('div');
    entry.className = `log-entry ${type}`;
    const timestamp = new Date().toLocaleTimeString();
    entry.textContent = `[${timestamp}] ${message}`;
    logContainer.appendChild(entry);
    logContainer.scrollTop = logContainer.scrollHeight;
}

// Draw Simulation Canvas
function drawSimulation(state) {
    latestState = state;
    if (state.grid) {
        GRID_WIDTH = state.grid.width || 10;
        GRID_HEIGHT = state.grid.height || 10;
        CELL_SIZE = canvas.width / GRID_WIDTH;
    }

    // Clear Canvas
    ctx.fillStyle = '#090c12';
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    // Draw Grid Lines
    ctx.strokeStyle = '#1e293b';
    ctx.lineWidth = 1;
    for (let x = 0; x <= GRID_WIDTH; x++) {
        ctx.beginPath();
        ctx.moveTo(x * CELL_SIZE, 0);
        ctx.lineTo(x * CELL_SIZE, canvas.height);
        ctx.stroke();
    }
    for (let y = 0; y <= GRID_HEIGHT; y++) {
        ctx.beginPath();
        ctx.moveTo(0, y * CELL_SIZE);
        ctx.lineTo(canvas.width, y * CELL_SIZE);
        ctx.stroke();
    }

    // Draw Obstacles
    if (state.obstacles) {
        state.obstacles.forEach(obs => {
            const ox = obs[0];
            const oy = obs[1];
            ctx.fillStyle = '#334155';
            ctx.strokeStyle = '#475569';
            ctx.lineWidth = 2;

            ctx.fillRect(ox * CELL_SIZE + 2, oy * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4);
            ctx.strokeRect(ox * CELL_SIZE + 2, oy * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4);

            // Diagonal hatch lines on obstacle
            ctx.strokeStyle = '#1e293b';
            ctx.lineWidth = 1;
            ctx.beginPath();
            ctx.moveTo(ox * CELL_SIZE + 4, oy * CELL_SIZE + CELL_SIZE - 4);
            ctx.lineTo(ox * CELL_SIZE + CELL_SIZE - 4, oy * CELL_SIZE + 4);
            ctx.stroke();
        });
    }

    // Draw Robot Planned Paths (Trajectory preview)
    if (state.robots) {
        state.robots.forEach(robot => {
            if (robot.path && robot.path.length > 1) {
                ctx.strokeStyle = 'rgba(59, 130, 246, 0.6)';
                ctx.lineWidth = 3;
                ctx.setLineDash([4, 4]);
                ctx.beginPath();
                ctx.moveTo(robot.path[0][0] * CELL_SIZE + CELL_SIZE / 2, robot.path[0][1] * CELL_SIZE + CELL_SIZE / 2);
                for (let k = 1; k < robot.path.length; k++) {
                    ctx.lineTo(robot.path[k][0] * CELL_SIZE + CELL_SIZE / 2, robot.path[k][1] * CELL_SIZE + CELL_SIZE / 2);
                }
                ctx.stroke();
                ctx.setLineDash([]);
            }
        });
    }

    // Draw Tasks
    if (state.tasks) {
        state.tasks.forEach(task => {
            const isCompleted = task.status_code === 2 || task.status === 'COMPLETED';
            if (!isCompleted) {
                const tx = task.x * CELL_SIZE + CELL_SIZE / 2;
                const ty = task.y * CELL_SIZE + CELL_SIZE / 2;
                const size = CELL_SIZE * 0.32;

                // Beacon Glow
                const grad = ctx.createRadialGradient(tx, ty, 2, tx, ty, size * 1.5);
                grad.addColorStop(0, 'rgba(245, 158, 11, 0.8)');
                grad.addColorStop(1, 'rgba(245, 158, 11, 0)');
                ctx.fillStyle = grad;
                ctx.beginPath();
                ctx.arc(tx, ty, size * 1.5, 0, Math.PI * 2);
                ctx.fill();

                // Target Diamond
                ctx.fillStyle = '#f59e0b';
                ctx.strokeStyle = '#fbbf24';
                ctx.lineWidth = 2;
                ctx.beginPath();
                ctx.moveTo(tx, ty - size);
                ctx.lineTo(tx + size, ty);
                ctx.lineTo(tx, ty + size);
                ctx.lineTo(tx - size, ty);
                ctx.closePath();
                ctx.fill();
                ctx.stroke();

                // Priority Badge
                ctx.fillStyle = '#000';
                ctx.font = 'bold 10px JetBrains Mono, monospace';
                ctx.textAlign = 'center';
                ctx.textBaseline = 'middle';
                ctx.fillText(`P${task.priority}`, tx, ty);
            }
        });
    }

    // Draw Robots
    if (state.robots) {
        state.robots.forEach(robot => {
            const rx = robot.x * CELL_SIZE + CELL_SIZE / 2;
            const ry = robot.y * CELL_SIZE + CELL_SIZE / 2;
            const radius = CELL_SIZE * 0.35;

            // Outer Glow
            const rGrad = ctx.createRadialGradient(rx, ry, radius * 0.5, rx, ry, radius * 1.6);
            rGrad.addColorStop(0, 'rgba(16, 185, 129, 0.4)');
            rGrad.addColorStop(1, 'rgba(16, 185, 129, 0)');
            ctx.fillStyle = rGrad;
            ctx.beginPath();
            ctx.arc(rx, ry, radius * 1.6, 0, Math.PI * 2);
            ctx.fill();

            // Body
            ctx.fillStyle = robot.state === 'OUT_OF_POWER' ? '#ef4444' : '#10b981';
            ctx.strokeStyle = '#a7f3d0';
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.arc(rx, ry, radius, 0, Math.PI * 2);
            ctx.fill();
            ctx.stroke();

            // Robot ID Text
            ctx.fillStyle = '#022c22';
            ctx.font = 'bold 11px JetBrains Mono, monospace';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText(`R${robot.id}`, rx, ry);
        });
    }

    // Update Telemetry Metrics UI
    document.getElementById('stat-tick').textContent = state.tick || 0;
    document.getElementById('stat-agents').textContent = state.robots ? state.robots.length : 0;
    
    if (state.stats) {
        document.getElementById('stat-completed').textContent = state.stats.completedTasks || 0;
        document.getElementById('stat-pending').textContent = state.stats.pendingTasks || 0;
    }

    // Update Algorithm dropdown if sync needed
    if (state.algorithm) {
        const select = document.getElementById('algorithm-select');
        if (select.value !== state.algorithm) {
            select.value = state.algorithm;
        }
    }

    // Update Agent Telemetry List
    const agentList = document.getElementById('agent-list');
    if (agentList && state.robots) {
        agentList.innerHTML = '';
        state.robots.forEach(robot => {
            const row = document.createElement('div');
            row.className = 'agent-row';
            const battPct = Math.max(0, Math.min(100, Math.round(robot.battery)));
            const battColor = battPct > 50 ? '#10b981' : battPct > 20 ? '#f59e0b' : '#ef4444';
            
            row.innerHTML = `
                <div class="agent-info">
                    <span class="agent-id">Robot #${robot.id}</span>
                    <span style="font-size:0.75rem; color:#8b9bb4">(${robot.x}, ${robot.y}) - ${robot.state}</span>
                </div>
                <div style="display:flex; align-items:center; gap:6px;">
                    <div class="battery-bar-container">
                        <div class="battery-bar-fill" style="width: ${battPct}%; background-color: ${battColor};"></div>
                    </div>
                    <span style="font-size:0.7rem; font-family:var(--font-mono);">${battPct}%</span>
                </div>
            `;
            agentList.appendChild(row);
        });
    }
}

// Fetch State API
async function fetchState() {
    try {
        const res = await fetch('/api/state');
        const data = await res.json();
        drawSimulation(data);
    } catch (err) {
        console.error('Failed to fetch state:', err);
    }
}

// Fetch Next Tick API
async function fetchTick() {
    try {
        const res = await fetch('/api/tick');
        const data = await res.json();
        drawSimulation(data);
        if (data.tick % 5 === 0) {
            logEvent(`Tick ${data.tick}: Executed multi-agent step.`, 'event');
        }
    } catch (err) {
        console.error('Failed to step tick:', err);
    }
}

// Event Listeners
document.getElementById('btn-step').addEventListener('click', () => {
    fetchTick();
});

document.getElementById('btn-auto').addEventListener('click', () => {
    if (!autoRunInterval) {
        autoRunInterval = setInterval(fetchTick, tickSpeed);
        logEvent('Simulation Auto Run started.', 'action');
    }
});

document.getElementById('btn-stop').addEventListener('click', () => {
    if (autoRunInterval) {
        clearInterval(autoRunInterval);
        autoRunInterval = null;
        logEvent('Simulation paused.', 'action');
    }
});

document.getElementById('btn-reset').addEventListener('click', async () => {
    if (autoRunInterval) {
        clearInterval(autoRunInterval);
        autoRunInterval = null;
    }
    try {
        const res = await fetch('/api/reset');
        const data = await res.json();
        drawSimulation(data);
        logEvent('Simulation reset to initial configuration.', 'action');
    } catch (err) {
        console.error('Reset failed:', err);
    }
});

document.getElementById('speed-slider').addEventListener('input', (e) => {
    tickSpeed = parseInt(e.target.value);
    document.getElementById('speed-val').textContent = `${tickSpeed}ms`;
    if (autoRunInterval) {
        clearInterval(autoRunInterval);
        autoRunInterval = setInterval(fetchTick, tickSpeed);
    }
});

document.getElementById('algorithm-select').addEventListener('change', async (e) => {
    const algo = e.target.value;
    try {
        const res = await fetch(`/api/strategy?algo=${algo}`);
        const data = await res.json();
        drawSimulation(data);
        logEvent(`Navigation strategy switched to: ${algo}`, 'system');
    } catch (err) {
        console.error('Failed to change strategy:', err);
    }
});

document.getElementById('btn-clear-log').addEventListener('click', () => {
    const logContainer = document.getElementById('event-log');
    if (logContainer) logContainer.innerHTML = '';
});

// Canvas Interactive Click Handling
canvas.addEventListener('click', async (e) => {
    const rect = canvas.getBoundingClientRect();
    const clickX = e.clientX - rect.left;
    const clickY = e.clientY - rect.top;

    const gridX = Math.floor(clickX / CELL_SIZE);
    const gridY = Math.floor(clickY / CELL_SIZE);

    if (gridX < 0 || gridX >= GRID_WIDTH || gridY < 0 || gridY >= GRID_HEIGHT) return;

    const mode = document.querySelector('input[name="click-mode"]:checked').value;

    if (mode === 'obstacle') {
        const isExistingObstacle = latestState && latestState.obstacles &&
            latestState.obstacles.some(obs => obs[0] === gridX && obs[1] === gridY);
        const action = isExistingObstacle ? 'remove' : 'add';

        try {
            const res = await fetch(`/api/obstacle?x=${gridX}&y=${gridY}&action=${action}`);
            const data = await res.json();
            drawSimulation(data);
            logEvent(`Obstacle ${action}ed at (${gridX}, ${gridY}).`, 'action');
        } catch (err) {
            console.error('Obstacle update failed:', err);
        }
    } else if (mode === 'task') {
        try {
            const res = await fetch(`/api/add_task?x=${gridX}&y=${gridY}&priority=2`);
            const data = await res.json();
            drawSimulation(data);
            logEvent(`New Task added at (${gridX}, ${gridY}).`, 'action');
        } catch (err) {
            console.error('Add task failed:', err);
        }
    }
});

// Initial load on page ready
fetchState();