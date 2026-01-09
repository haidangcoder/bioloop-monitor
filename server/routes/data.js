// REST API routes for sensor data
const express = require('express');
const router = express.Router();
const db = require('../db');

// POST /api/data - ESP32 sends sensor readings
router.post('/', (req, res) => {
  // Debug: Log received body
  console.log('[POST /api/data] Received body:', req.body);
  
  const { temperature, moisture, temp_error, fan_pwm, pump } = req.body;
  
  // Validate required fields
  if (temperature === undefined || moisture === undefined || 
      temp_error === undefined || fan_pwm === undefined || pump === undefined) {
    return res.status(400).json({ 
      success: false, 
      error: 'Missing required fields' 
    });
  }
  
  // Validate data types
  if (typeof temperature !== 'number' || typeof moisture !== 'number' ||
      typeof temp_error !== 'number' || typeof fan_pwm !== 'number' ||
      typeof pump !== 'boolean') {
    return res.status(400).json({ 
      success: false, 
      error: 'Invalid data types' 
    });
  }
  
  // Insert into database (sqlite3 async API)
  const sql = `
    INSERT INTO sensor_data (temperature, moisture, temp_error, fan_pwm, pump_active)
    VALUES (?, ?, ?, ?, ?)
  `;
  
  db.run(sql, [temperature, moisture, temp_error, fan_pwm, pump ? 1 : 0], function(err) {
    if (err) {
      console.error('[POST /api/data] Database error:', err.message);
      return res.status(500).json({ 
        success: false, 
        error: err.message 
      });
    }
    
    res.json({ 
      success: true, 
      id: this.lastID,
      timestamp: new Date().toISOString()
    });
  });
});

// GET /api/latest - Get most recent compost log entry
router.get('/latest', (req, res) => {
  const sql = `
    SELECT * FROM sensor_data 
    ORDER BY timestamp DESC 
    LIMIT 1
  `;
  
  db.get(sql, [], (err, row) => {
    if (err) {
      console.error('[GET /api/latest] Database error:', err.message);
      return res.status(500).json({ error: err.message });
    }
    
    res.json(row || {});
  });
});

// GET /api/history?limit=200 - Get last N entries (default 200)
router.get('/history', (req, res) => {
  const limit = parseInt(req.query.limit) || 200;
  
  const sql = `
    SELECT * FROM sensor_data 
    ORDER BY timestamp DESC 
    LIMIT ?
  `;
  
  db.all(sql, [limit], (err, rows) => {
    if (err) {
      console.error('[GET /api/history] Database error:', err.message);
      return res.status(500).json({ error: err.message });
    }
    
    // Return in ascending order (oldest to newest)
    res.json(rows.reverse());
  });
});

module.exports = router;
