// BioLoop Monitor - Scientific MVP Backend
const express = require('express');
const cors = require('cors');
const path = require('path');

const dataRoutes = require('./routes/data');

const app = express();
const PORT = process.env.PORT || 3000;

// Middleware (MUST be before routes)
app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Debug middleware to log all requests
app.use((req, res, next) => {
  console.log(`[${new Date().toISOString()}] ${req.method} ${req.path}`);
  next();
});

// Serve static frontend files
app.use(express.static(path.join(__dirname, 'public')));

// API routes
app.use('/api/data', dataRoutes);

// Health check
app.get('/api/health', (req, res) => {
  res.json({ status: 'ok', timestamp: new Date().toISOString() });
});

// Start server (bind to 0.0.0.0 for Render compatibility)
app.listen(PORT, '0.0.0.0', () => {
  console.log(`[Server] BioLoop Monitor running on port ${PORT}`);
  console.log('[Server] Middleware: express.json() + express.urlencoded() enabled');
  console.log('[Server] API endpoints:');
  console.log('  POST /api/data        - Submit sensor data');
  console.log('  GET  /api/latest      - Get most recent reading');
  console.log('  GET  /api/history     - Get last N readings (default 200)');
});
