// REST API routes for sensor data
const express = require('express');
const router = express.Router();
const db = require('../db');

// POST /api/data - ESP32 sends sensor readings
router.post('/', (req, res) => {
  // Debug: Log received body
  console.log('[POST /api/data] Received body:', req.body);
  
  const { temperature, moisture, temp_error, fan_pwm, pump, phase, target_temp } = req.body;
  
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
    INSERT INTO sensor_data (temperature, moisture, temp_error, fan_pwm, pump_active, phase, target_temp)
    VALUES (?, ?, ?, ?, ?, ?, ?)
  `;
  
  const phaseValue = phase !== undefined ? phase : 1;
  const targetTempValue = target_temp !== undefined ? target_temp : 35.0;
  
  db.run(sql, [temperature, moisture, temp_error, fan_pwm, pump ? 1 : 0, phaseValue, targetTempValue], function(err) {
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

// GET /api/daily-performance - Calculate daily performance metrics
router.get('/daily-performance', (req, res) => {
  const twentyFourHoursAgo = new Date(Date.now() - 24 * 60 * 60 * 1000).toISOString();
  
  const sql = `
    SELECT * FROM sensor_data 
    WHERE timestamp > ? 
    ORDER BY timestamp ASC
  `;
  
  db.all(sql, [twentyFourHoursAgo], (err, rows) => {
    if (err) {
      console.error('[GET /daily-performance] Database error:', err.message);
      return res.status(500).json({ error: err.message });
    }
    
    if (!rows || rows.length === 0) {
      return res.json({
        score: 0,
        rating: 'Chưa có dữ liệu',
        stars: 0,
        stats: {
          optimalTime: 0,
          avgTemp: 0,
          avgMoisture: 0,
          fanActivations: 0,
          pumpActivations: 0,
          totalReadings: 0
        }
      });
    }
    
    // Calculate metrics
    let optimalCount = 0;
    let tempSum = 0;
    let moistureSum = 0;
    let fanActivations = 0;
    let pumpActivations = 0;
    let lastFanState = false;
    let lastPumpState = false;
    
    rows.forEach(row => {
      // Check if in optimal range
      const tempOptimal = row.temperature >= 45 && row.temperature <= 55;
      const moistureOptimal = row.moisture >= 50 && row.moisture <= 60;
      if (tempOptimal && moistureOptimal) optimalCount++;
      
      tempSum += row.temperature;
      moistureSum += row.moisture;
      
      // Count activations (state changes from OFF to ON)
      if (row.fan && !lastFanState) fanActivations++;
      if (row.pump_active && !lastPumpState) pumpActivations++;
      lastFanState = row.fan;
      lastPumpState = row.pump_active;
    });
    
    const totalReadings = rows.length;
    const optimalPercentage = (optimalCount / totalReadings) * 100;
    const avgTemp = tempSum / totalReadings;
    const avgMoisture = moistureSum / totalReadings;
    
    // Calculate score (0-100)
    let score = 0;
    
    // Temperature score (40 points max)
    if (avgTemp >= 45 && avgTemp <= 55) {
      score += 40;
    } else if (avgTemp >= 40 && avgTemp < 45) {
      score += 20;
    } else if (avgTemp > 55 && avgTemp <= 60) {
      score += 30;
    }
    
    // Moisture score (40 points max)
    if (avgMoisture >= 50 && avgMoisture <= 60) {
      score += 40;
    } else if (avgMoisture >= 45 && avgMoisture < 50) {
      score += 20;
    } else if (avgMoisture > 60 && avgMoisture <= 65) {
      score += 30;
    }
    
    // Stability score (20 points max) - based on optimal time
    score += (optimalPercentage / 100) * 20;
    
    // Rating
    let rating = '';
    let stars = 0;
    if (score >= 90) {
      rating = 'Xuất sắc';
      stars = 5;
    } else if (score >= 70) {
      rating = 'Tốt';
      stars = 4;
    } else if (score >= 50) {
      rating = 'Trung bình';
      stars = 3;
    } else if (score >= 30) {
      rating = 'Cần cải thiện';
      stars = 2;
    } else {
      rating = 'Yếu';
      stars = 1;
    }
    
    res.json({
      score: Math.round(score),
      rating,
      stars,
      stats: {
        optimalTime: Math.round(optimalPercentage),
        avgTemp: parseFloat(avgTemp.toFixed(1)),
        avgMoisture: parseFloat(avgMoisture.toFixed(1)),
        fanActivations,
        pumpActivations,
        totalReadings
      }
    });
  });
});

module.exports = router;
