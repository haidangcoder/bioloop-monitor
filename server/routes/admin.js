// Admin API - Database management
const express = require('express');
const router = express.Router();
const db = require('../db');

// POST /api/admin/clear-data - Xóa toàn bộ dữ liệu cũ
router.post('/clear-data', (req, res) => {
  const sql = 'DELETE FROM sensor_data';
  
  db.run(sql, [], function(err) {
    if (err) {
      console.error('[ADMIN] Error clearing data:', err.message);
      return res.status(500).json({ 
        success: false, 
        error: err.message 
      });
    }
    
    console.log(`[ADMIN] Cleared ${this.changes} records`);
    
    res.json({ 
      success: true,
      message: `Đã xóa ${this.changes} bản ghi`,
      deleted: this.changes
    });
  });
});

// POST /api/admin/clear-old-data - Xóa dữ liệu cũ hơn X ngày
router.post('/clear-old-data', (req, res) => {
  const { days } = req.body;
  const daysAgo = days || 7;
  
  const cutoffDate = new Date(Date.now() - daysAgo * 24 * 60 * 60 * 1000).toISOString();
  
  const sql = 'DELETE FROM sensor_data WHERE timestamp < ?';
  
  db.run(sql, [cutoffDate], function(err) {
    if (err) {
      console.error('[ADMIN] Error clearing old data:', err.message);
      return res.status(500).json({ 
        success: false, 
        error: err.message 
      });
    }
    
    console.log(`[ADMIN] Cleared ${this.changes} old records (older than ${daysAgo} days)`);
    
    res.json({ 
      success: true,
      message: `Đã xóa ${this.changes} bản ghi cũ hơn ${daysAgo} ngày`,
      deleted: this.changes,
      cutoff_date: cutoffDate
    });
  });
});

// GET /api/admin/stats - Thống kê database
router.get('/stats', (req, res) => {
  const sql = `
    SELECT 
      COUNT(*) as total_records,
      MIN(timestamp) as oldest_record,
      MAX(timestamp) as newest_record,
      AVG(temperature) as avg_temp,
      AVG(moisture) as avg_moisture
    FROM sensor_data
  `;
  
  db.get(sql, [], (err, row) => {
    if (err) {
      console.error('[ADMIN] Error getting stats:', err.message);
      return res.status(500).json({ error: err.message });
    }
    
    res.json(row || {});
  });
});

module.exports = router;
