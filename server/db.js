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
  // Create sensor_data table
  db.run(`
    CREATE TABLE IF NOT EXISTS sensor_data (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      temperature REAL,
      moisture REAL,
      temp_error REAL,
      fan_pwm INTEGER,
      pump_active INTEGER,
      phase INTEGER DEFAULT 1,
      target_temp REAL DEFAULT 35.0,
      timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
    )
  `, (err) => {
    if (err) {
      console.error('[DB] Table error:', err.message);
    } else {
      console.log('[DB] sensor_data table ready');
      
      // Add new columns if they don't exist (for existing databases)
      db.run(`ALTER TABLE sensor_data ADD COLUMN phase INTEGER DEFAULT 1`, (err) => {
        if (err && !err.message.includes('duplicate column')) {
          console.error('[DB] Error adding phase column:', err.message);
        } else {
          console.log('[DB] Phase column ready');
        }
      });
      
      db.run(`ALTER TABLE sensor_data ADD COLUMN target_temp REAL DEFAULT 35.0`, (err) => {
        if (err && !err.message.includes('duplicate column')) {
          console.error('[DB] Error adding target_temp column:', err.message);
        } else {
          console.log('[DB] Target_temp column ready');
        }
      });
    }
  });
  
  // Create demo_override table
  db.run(`
    CREATE TABLE IF NOT EXISTS demo_override (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      case_num INTEGER,
      fan_pwm INTEGER,
      pump_active INTEGER,
      active INTEGER DEFAULT 1,
      timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
    )
  `, (err) => {
    if (err) {
      console.error('[DB] demo_override table error:', err.message);
    } else {
      console.log('[DB] demo_override table ready');
    }
  });
});

module.exports = db;
