// Test script to simulate ESP32 POST request
// Run: node test-post.js

const http = require('http');

const data = JSON.stringify({
  temperature: 55.3,
  moisture: 42.5,
  temp_error: 0.3,
  fan_pwm: 128,
  pump: false
});

const options = {
  hostname: 'localhost',
  port: 3000,
  path: '/api/data',
  method: 'POST',
  headers: {
    'Content-Type': 'application/json',
    'Content-Length': data.length
  }
};

const req = http.request(options, (res) => {
  console.log(`Status: ${res.statusCode}`);
  
  res.on('data', (chunk) => {
    console.log('Response:', chunk.toString());
  });
});

req.on('error', (error) => {
  console.error('Error:', error);
});

req.write(data);
req.end();
