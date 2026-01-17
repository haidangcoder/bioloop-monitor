// Demo mode API - Override control for testing
const express = require('express');
const router = express.Router();
const db = require('../db');

// POST /api/demo/override - Set override mode
router.post('/override', (req, res) => {
  console.log('[DEMO] Override request:', req.body);
  
  const { case_num, fan_pwm, pump_active } = req.body;
  
  if (!case_num || fan_pwm === undefined || pump_active === undefined) {
    return res.status(400).json({ 
      success: false, 
      error: 'Missing required fields' 
    });
  }
  
  // Clear old overrides first
  db.run('DELETE FROM demo_override WHERE active = 1', [], (err) => {
    if (err) {
      console.error('[DEMO] Error clearing old overrides:', err.message);
    }
    
    // Store override command in database
    const sql = `
      INSERT INTO demo_override (case_num, fan_pwm, pump_active, active)
      VALUES (?, ?, ?, 1)
    `;
    
    // Force pump_active to integer (0 or 1)
    const pumpInt = pump_active ? 1 : 0;
    console.log(`[DEMO] Storing: case=${case_num}, fan=${fan_pwm}, pump=${pumpInt}`);
    
    db.run(sql, [case_num, fan_pwm, pumpInt], function(err) {
      if (err) {
        console.error('[DEMO] Database error:', err.message);
        return res.status(500).json({ 
          success: false, 
          error: err.message 
        });
      }
      
      console.log(`[DEMO] Override set: Case ${case_num}, Fan=${fan_pwm}, Pump=${pumpInt}`);
      
      res.json({ 
        success: true, 
        id: this.lastID,
        message: `Demo Case ${case_num} activated`
      });
    });
  });
});

// GET /api/demo/override - Get current override mode
router.get('/override', (req, res) => {
  const sql = `
    SELECT * FROM demo_override 
    WHERE active = 1 
    ORDER BY timestamp DESC 
    LIMIT 1
  `;
  
  db.get(sql, [], (err, row) => {
    if (err) {
      console.error('[DEMO] Database error:', err.message);
      return res.status(500).json({ error: err.message });
    }
    
    if (!row) {
      return res.json({ active: false });
    }
    
    // Check if override is still valid (within 5 minutes)
    const overrideTime = new Date(row.timestamp);
    const now = new Date();
    const diffMinutes = (now - overrideTime) / 1000 / 60;
    
    if (diffMinutes > 5) {
      // Expire old override
      db.run('UPDATE demo_override SET active = 0 WHERE id = ?', [row.id]);
      return res.json({ active: false });
    }
    
    res.json({
      active: true,
      case_num: row.case_num,
      fan_pwm: row.fan_pwm,
      pump_active: row.pump_active === 1,
      timestamp: row.timestamp
    });
  });
});

// GET /api/demo/status - Lightweight endpoint for ESP32 polling
router.get('/status', (req, res) => {
  const sql = `
    SELECT case_num, fan_pwm, pump_active, timestamp 
    FROM demo_override 
    WHERE active = 1 
    ORDER BY timestamp DESC 
    LIMIT 1
  `;
  
  db.get(sql, [], (err, row) => {
    if (err) {
      return res.status(500).json({ active: 0 });
    }
    
    if (!row) {
      return res.json({ active: 0 });
    }
    
    // Check if override is still valid (within 5 minutes)
    const overrideTime = new Date(row.timestamp);
    const now = new Date();
    const diffMinutes = (now - overrideTime) / 1000 / 60;
    
    if (diffMinutes > 5) {
      // Expire old override
      db.run('UPDATE demo_override SET active = 0 WHERE timestamp = ?', [row.timestamp]);
      return res.json({ active: 0 });
    }
    
    // Return minimal data for ESP32
    res.json({
      active: 1,
      fan: row.fan_pwm,
      pump: row.pump_active
    });
  });
});

// POST /api/demo/clear - Clear override mode
router.post('/clear', (req, res) => {
  const sql = 'UPDATE demo_override SET active = 0';
  
  db.run(sql, [], function(err) {
    if (err) {
      console.error('[DEMO] Database error:', err.message);
      return res.status(500).json({ 
        success: false, 
        error: err.message 
      });
    }
    
    console.log('[DEMO] Override cleared');
    
    res.json({ 
      success: true,
      message: 'Demo mode cleared, returning to normal operation'
    });
  });
});

module.exports = router;
