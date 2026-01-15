// server/db.js
const sqlite3 = require('sqlite3').verbose();
const path = require('path');

const dbPath = path.join(__dirname, 'bioloop.db');

const db = new sqlite3.Database(dbPath, (err) => {
  if (err) {
    console.error('[DB] Failed to connect:', err.message);
  } else {
    console.log('[DB] SQLite initialized at', dbPath);
  }
});

db.serialize(() => {
  db.run(`
    CREATE TABLE IF NOT EXISTS sensor_data (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      temperature REAL,
      moisture REAL,
      fan_pwm INTEGER,
      pump_active INTEGER,
      created_at DATETIME DEFAULT CURRENT_TIMESTAMP
    )
  `, (err) => {
    if (err) {
      console.error('[DB] Table error:', err.message);
    } else {
      console.log('[DB] Table ready');
    }
  });
});

module.exports = db;
