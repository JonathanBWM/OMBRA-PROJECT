#!/bin/bash
# OMBRA Project Management Dashboard Launcher
# Starts both backend (port 1337) and frontend (port 3000)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "═══════════════════════════════════════════════════════════════════"
echo "  OMBRA Project Management Dashboard"
echo "═══════════════════════════════════════════════════════════════════"
echo ""

# Check if backend dependencies are installed
if ! python3 -c "import fastapi" 2>/dev/null; then
    echo "[!] Installing backend dependencies..."
    pip3 install fastapi uvicorn
fi

# Start backend
echo "[*] Starting backend on port 1337..."
cd "$SCRIPT_DIR/backend"
python3 -m uvicorn main:app --host 0.0.0.0 --port 1337 --reload &
BACKEND_PID=$!

# Give backend time to start
sleep 2

# Check if frontend dependencies are installed
if [ ! -d "$SCRIPT_DIR/frontend/node_modules" ]; then
    echo "[*] Installing frontend dependencies..."
    cd "$SCRIPT_DIR/frontend"
    npm install
fi

# Start frontend
echo "[*] Starting frontend on port 3000..."
cd "$SCRIPT_DIR/frontend"
npm run dev &
FRONTEND_PID=$!

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo "  Dashboard running!"
echo "  Backend:  http://localhost:1337"
echo "  Frontend: http://localhost:3000"
echo "  API Docs: http://localhost:1337/docs"
echo "═══════════════════════════════════════════════════════════════════"
echo ""
echo "Press Ctrl+C to stop both servers..."

# Handle shutdown
cleanup() {
    echo ""
    echo "[*] Shutting down..."
    kill $BACKEND_PID 2>/dev/null
    kill $FRONTEND_PID 2>/dev/null
    exit 0
}

trap cleanup SIGINT SIGTERM

# Wait for both processes
wait
