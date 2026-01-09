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
  
  try {
    const stmt = db.prepare(`
      INSERT INTO sensor_data (temperature, moisture, temp_error, fan_pwm, pump_active)
      VALUES (?, ?, ?, ?, ?)
    `);
    const result = stmt.run(temperature, moisture, temp_error, fan_pwm, pump ? 1 : 0);
    
    res.json({ 
      success: true, 
      id: result.lastInsertRowid,
      timestamp: new Date().toISOString()
    });
  } catch (err) {
    res.status(500).json({ 
      success: false, 
      error: err.message 
    });
  }
});

// GET /api/latest - Get most recent compost log entry
router.get('/latest', (req, res) => {
  try {
    const row = db.prepare(`
      SELECT * FROM sensor_data 
      ORDER BY timestamp DESC 
      LIMIT 1
    `).get();
    
    res.json(row || {});
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// GET /api/history?limit=200 - Get last N entries (default 200)
router.get('/history', (req, res) => {
  const limit = parseInt(req.query.limit) || 200;
  
  try {
    const rows = db.prepare(`
      SELECT * FROM sensor_data 
      ORDER BY timestamp DESC 
      LIMIT ?
    `).all(limit);
    
    // Return in ascending order (oldest to newest)
    res.json(rows.reverse());
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

module.exports = router;
