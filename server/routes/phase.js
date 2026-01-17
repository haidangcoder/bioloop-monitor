// Phase management API - Quản lý giai đoạn compost theo thời gian
const express = require('express');
const router = express.Router();
const db = require('../db');

// Initialize phase_config table
db.serialize(() => {
  db.run(`
    CREATE TABLE IF NOT EXISTS phase_config (
      id INTEGER PRIMARY KEY CHECK (id = 1),
      start_date TEXT NOT NULL,
      current_phase INTEGER DEFAULT 1,
      phase_1_days INTEGER DEFAULT 7,
      phase_2_days INTEGER DEFAULT 14,
      phase_3_days INTEGER DEFAULT 30,
      updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
    )
  `, (err) => {
    if (err) {
      console.error('[PHASE] Table error:', err.message);
    } else {
      console.log('[PHASE] phase_config table ready');
      
      // Insert default config if not exists
      db.run(`
        INSERT OR IGNORE INTO phase_config (id, start_date, current_phase)
        VALUES (1, datetime('now'), 1)
      `);
    }
  });
});

// GET /api/phase - Lấy thông tin pha hiện tại
router.get('/', (req, res) => {
  const sql = `SELECT * FROM phase_config WHERE id = 1`;
  
  db.get(sql, [], (err, row) => {
    if (err) {
      return res.status(500).json({ error: err.message });
    }
    
    if (!row) {
      return res.status(404).json({ error: 'Phase config not found' });
    }
    
    // Tính số ngày đã ủ
    const startDate = new Date(row.start_date);
    const now = new Date();
    const daysElapsed = Math.floor((now - startDate) / (1000 * 60 * 60 * 24));
    
    // Tự động phát hiện pha dựa trên số ngày
    let autoPhase = 1;
    let phaseDay = daysElapsed;
    let phaseName = 'Mesophilic';
    let targetTemp = 35.0;
    
    if (daysElapsed < row.phase_1_days) {
      autoPhase = 1;
      phaseDay = daysElapsed;
      phaseName = 'Mesophilic';
      targetTemp = 35.0;
    } else if (daysElapsed < row.phase_1_days + row.phase_2_days) {
      autoPhase = 2;
      phaseDay = daysElapsed - row.phase_1_days;
      phaseName = 'Thermophilic';
      targetTemp = 52.5;
    } else {
      autoPhase = 3;
      phaseDay = daysElapsed - row.phase_1_days - row.phase_2_days;
      phaseName = 'Maturation';
      targetTemp = 35.0;
    }
    
    res.json({
      start_date: row.start_date,
      days_elapsed: daysElapsed,
      current_phase: row.current_phase,
      auto_phase: autoPhase,
      phase_day: phaseDay,
      phase_name: phaseName,
      target_temp: targetTemp,
      phase_1_days: row.phase_1_days,
      phase_2_days: row.phase_2_days,
      phase_3_days: row.phase_3_days
    });
  });
});

// POST /api/phase/reset - Khởi tạo lại thùng compost mới
router.post('/reset', (req, res) => {
  const { phase_1_days, phase_2_days, phase_3_days } = req.body;
  
  const sql = `
    UPDATE phase_config 
    SET start_date = datetime('now'),
        current_phase = 1,
        phase_1_days = COALESCE(?, phase_1_days),
        phase_2_days = COALESCE(?, phase_2_days),
        phase_3_days = COALESCE(?, phase_3_days),
        updated_at = datetime('now')
    WHERE id = 1
  `;
  
  db.run(sql, [phase_1_days, phase_2_days, phase_3_days], function(err) {
    if (err) {
      return res.status(500).json({ error: err.message });
    }
    
    console.log('[PHASE] Reset to new batch');
    res.json({ 
      success: true,
      message: 'Đã khởi tạo thùng compost mới',
      start_date: new Date().toISOString()
    });
  });
});

// POST /api/phase/set - Chuyển pha thủ công (override)
router.post('/set', (req, res) => {
  const { phase } = req.body;
  
  if (!phase || phase < 1 || phase > 3) {
    return res.status(400).json({ error: 'Invalid phase (1-3)' });
  }
  
  const sql = `
    UPDATE phase_config 
    SET current_phase = ?,
        updated_at = datetime('now')
    WHERE id = 1
  `;
  
  db.run(sql, [phase], function(err) {
    if (err) {
      return res.status(500).json({ error: err.message });
    }
    
    console.log(`[PHASE] Manually set to phase ${phase}`);
    res.json({ 
      success: true,
      message: `Đã chuyển sang pha ${phase}`,
      phase: phase
    });
  });
});

module.exports = router;
