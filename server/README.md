# BioLoop Monitor - Server

Scientific MVP backend for BioLoop composting control system.

## Tech Stack
- Node.js + Express
- SQLite3 (async)
- REST API

## Local Development

```bash
cd server
npm install
npm start
```

Server runs on http://localhost:3000

## Render Deployment

### Configuration:
- **Root Directory**: `server`
- **Build Command**: `npm install`
- **Start Command**: `npm start`
- **Environment**: Node

### Environment Variables:
- `PORT` - Automatically set by Render

### What Happens on Deploy:
1. Render navigates to `/server` directory
2. Runs `npm install` (installs sqlite3 for Linux)
3. Runs `npm start` (starts `node index.js`)
4. Server binds to `0.0.0.0:$PORT`
5. Database `bioloop.db` is created automatically

## API Endpoints

- `POST /api/data` - Submit sensor data from ESP32
- `GET /api/latest` - Get most recent reading
- `GET /api/history?limit=200` - Get historical data
- `GET /api/health` - Health check

## Dashboard

Visit the root URL to see the live dashboard:
- Real-time sensor data
- Temperature & fan PWM charts
- Data log table

## Database Schema

```sql
CREATE TABLE sensor_data (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
  temperature REAL,
  moisture REAL,
  temp_error REAL,
  fan_pwm INTEGER,
  pump_active INTEGER
);
```

## Notes

- Database file is created in the same directory as index.js
- All queries use async callbacks (sqlite3 API)
- CORS enabled for ESP32 communication
- Static files served from `public/` folder
