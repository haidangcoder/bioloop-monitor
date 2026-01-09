// Database module - SQLite connection and initialization
const Database = require('better-sqlite3');
const path = require('path');

const dbPath = path.join(__dirname, 'bioloop.db');
const db = new Database(dbPath);

// Initialize tables
db.exec(`
  CREATE TABLE IF NOT EXISTS sensor_data (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    temperature REAL,
    moisture REAL,
    temp_error REAL,
    fan_pwm INTEGER,
    pump_active INTEGER
  )
`);

console.log('[DB] SQLite initialized at', dbPath);

module.exports = db;
