// BioLoop Monitor - Scientific Dashboard JavaScript
// Features: Biological Phase Indicator, Optimal Range Overlay, Stability Index

// Chart.js instances
let tempChart, moistureChart, fanChart;

// Stability tracking
let stabilityHistory = [];
const STABILITY_WINDOW = 20; // Track last 20 readings

// Biological Phase Detection based on temperature
function getBiologicalPhase(temperature) {
    if (temperature >= 45 && temperature <= 65) {
        return {
            name: 'THERMOPHILIC',
            nameVi: 'Pha Nhiệt Cao',
            description: 'Vi sinh vật ưa nhiệt hoạt động mạnh, tiêu diệt mầm bệnh và cỏ dại',
            color: '#f97316',
            segment: 'thermo'
        };
    } else if (temperature >= 20 && temperature < 45) {
        return {
            name: 'MESOPHILIC',
            nameVi: 'Pha Ấm',
            description: 'Vi sinh vật ưa ấm phân hủy chất hữu cơ, giai đoạn khởi đầu',
            color: '#3b82f6',
            segment: 'meso'
        };
    } else if (temperature < 20) {
        return {
            name: 'MATURATION',
            nameVi: 'Pha Ổn Định',
            description: 'Compost đang ổn định và trưởng thành, sẵn sàng sử dụng',
            color: '#22c55e',
            segment: 'mature'
        };
    } else {
        return {
            name: 'COOLING',
            nameVi: 'Đang Làm Mát',
            description: 'Nhiệt độ quá cao, hệ thống đang làm mát để bảo vệ vi sinh vật',
            color: '#ef4444',
            segment: 'thermo'
        };
    }
}

// Calculate Compost Stability Index (0-100)
function calculateStabilityIndex(historyData) {
    if (!historyData || historyData.length === 0) return 0;
    
    let optimalCount = 0;
    const recentData = historyData.slice(-STABILITY_WINDOW);
    
    recentData.forEach(d => {
        const tempOptimal = d.temperature >= 45 && d.temperature <= 55;
        const moistureOptimal = d.moisture >= 50 && d.moisture <= 60;
        if (tempOptimal && moistureOptimal) optimalCount++;
        else if (tempOptimal || moistureOptimal) optimalCount += 0.5;
    });
    
    return Math.round((optimalCount / recentData.length) * 100);
}

function getStabilityLabel(index) {
    if (index >= 80) return { text: 'Xuất sắc - Điều kiện lý tưởng', color: '#10b981' };
    if (index >= 60) return { text: 'Tốt - Hoạt động hiệu quả', color: '#22c55e' };
    if (index >= 40) return { text: 'Trung bình - Cần theo dõi', color: '#eab308' };
    if (index >= 20) return { text: 'Thấp - Cần điều chỉnh', color: '#f97316' };
    return { text: 'Cảnh báo - Kiểm tra hệ thống', color: '#ef4444' };
}

// Update Biological Phase UI
function updatePhaseIndicator(temperature) {
    const phase = getBiologicalPhase(temperature);
    
    document.getElementById('phase-name').textContent = phase.nameVi;
    document.getElementById('phase-name').style.color = phase.color;
    document.getElementById('phase-description').textContent = phase.description;
    
    // Update phase bar segments
    ['meso', 'thermo', 'mature'].forEach(seg => {
        const el = document.getElementById(`seg-${seg}`);
        el.classList.toggle('inactive', seg !== phase.segment);
    });
}

// Update Stability Gauge
function updateStabilityGauge(historyData) {
    const index = calculateStabilityIndex(historyData);
    const label = getStabilityLabel(index);
    
    // Update needle rotation (-90deg = 0, 90deg = 100)
    const rotation = -90 + (index * 1.8);
    document.getElementById('gauge-needle').style.transform = 
        `translateX(-50%) rotate(${rotation}deg)`;
    
    document.getElementById('stability-value').textContent = index;
    document.getElementById('stability-value').style.color = label.color;
    document.getElementById('stability-label').textContent = label.text;
}


// Initialize charts with optimal range overlays
function initCharts() {
    // Temperature Chart with Optimal Zone (45-55°C)
    tempChart = new Chart(document.getElementById('tempChart'), {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Nhiệt độ (°C)',
                data: [],
                borderColor: '#f97316',
                backgroundColor: 'rgba(249, 115, 22, 0.1)',
                tension: 0.4,
                fill: false,
                borderWidth: 2,
                pointRadius: 2
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: {
                    min: 20,
                    max: 70,
                    grid: { color: 'rgba(255,255,255,0.1)' },
                    ticks: { color: '#94a3b8' },
                    title: { display: true, text: 'Nhiệt độ (°C)', color: '#94a3b8' }
                },
                x: {
                    grid: { color: 'rgba(255,255,255,0.1)' },
                    ticks: { color: '#94a3b8', maxRotation: 0 },
                    title: { display: true, text: 'Thời gian', color: '#94a3b8' }
                }
            },
            plugins: {
                legend: { display: false },
                annotation: {
                    annotations: {
                        optimalZone: {
                            type: 'box',
                            yMin: 45,
                            yMax: 55,
                            backgroundColor: 'rgba(34, 197, 94, 0.15)',
                            borderColor: 'rgba(34, 197, 94, 0.5)',
                            borderWidth: 1
                        },
                        optimalLine: {
                            type: 'line',
                            yMin: 50,
                            yMax: 50,
                            borderColor: 'rgba(34, 197, 94, 0.8)',
                            borderWidth: 2,
                            borderDash: [5, 5],
                            label: {
                                display: true,
                                content: 'Mục tiêu: 50°C',
                                position: 'end',
                                backgroundColor: 'rgba(34, 197, 94, 0.8)',
                                color: '#fff',
                                font: { size: 10 }
                            }
                        }
                    }
                }
            }
        }
    });

    // Moisture Chart with Optimal Zone (50-60%)
    moistureChart = new Chart(document.getElementById('moistureChart'), {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Độ ẩm (%)',
                data: [],
                borderColor: '#3b82f6',
                backgroundColor: 'rgba(59, 130, 246, 0.1)',
                tension: 0.4,
                fill: false,
                borderWidth: 2,
                pointRadius: 2
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: {
                    min: 0,
                    max: 100,
                    grid: { color: 'rgba(255,255,255,0.1)' },
                    ticks: { color: '#94a3b8' },
                    title: { display: true, text: 'Độ ẩm (%)', color: '#94a3b8' }
                },
                x: {
                    grid: { color: 'rgba(255,255,255,0.1)' },
                    ticks: { color: '#94a3b8', maxRotation: 0 },
                    title: { display: true, text: 'Thời gian', color: '#94a3b8' }
                }
            },
            plugins: {
                legend: { display: false },
                annotation: {
                    annotations: {
                        optimalZone: {
                            type: 'box',
                            yMin: 50,
                            yMax: 60,
                            backgroundColor: 'rgba(34, 197, 94, 0.15)',
                            borderColor: 'rgba(34, 197, 94, 0.5)',
                            borderWidth: 1
                        },
                        optimalLine: {
                            type: 'line',
                            yMin: 55,
                            yMax: 55,
                            borderColor: 'rgba(34, 197, 94, 0.8)',
                            borderWidth: 2,
                            borderDash: [5, 5],
                            label: {
                                display: true,
                                content: 'Mục tiêu: 55%',
                                position: 'end',
                                backgroundColor: 'rgba(34, 197, 94, 0.8)',
                                color: '#fff',
                                font: { size: 10 }
                            }
                        }
                    }
                }
            }
        }
    });

    // Fan PWM Chart
    fanChart = new Chart(document.getElementById('fanChart'), {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Fan PWM',
                data: [],
                borderColor: '#8b5cf6',
                backgroundColor: 'rgba(139, 92, 246, 0.1)',
                tension: 0.4,
                fill: true,
                borderWidth: 2,
                pointRadius: 2
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: {
                    min: 0,
                    max: 255,
                    grid: { color: 'rgba(255,255,255,0.1)' },
                    ticks: { color: '#94a3b8' },
                    title: { display: true, text: 'PWM (0-255)', color: '#94a3b8' }
                },
                x: {
                    grid: { color: 'rgba(255,255,255,0.1)' },
                    ticks: { color: '#94a3b8', maxRotation: 0 },
                    title: { display: true, text: 'Thời gian', color: '#94a3b8' }
                }
            },
            plugins: {
                legend: { display: false }
            }
        }
    });
}


// Fetch latest reading and update status cards
async function fetchLatest() {
    try {
        const response = await fetch('/api/data/latest');
        const data = await response.json();
        
        if (data.id) {
            // Update status cards
            document.getElementById('current-temp').textContent = data.temperature.toFixed(1);
            document.getElementById('current-moisture').textContent = data.moisture.toFixed(1);
            document.getElementById('current-fan').textContent = data.fan_pwm;
            
            // Update pump status
            const pumpElement = document.getElementById('current-pump');
            pumpElement.textContent = data.pump_active ? 'BẬT' : 'TẮT';
            pumpElement.className = data.pump_active ? 'value pump-on' : 'value pump-off';
            
            // Update biological phase indicator
            updatePhaseIndicator(data.temperature);
            
            // Update timestamp
            document.getElementById('last-update').textContent = new Date(data.timestamp).toLocaleString('vi-VN', {timeZone: 'Asia/Ho_Chi_Minh'});
        }
    } catch (error) {
        console.error('Error fetching latest data:', error);
        document.getElementById('last-update').textContent = 'Lỗi kết nối';
    }
}

// Fetch history and update charts + table
async function fetchHistory() {
    try {
        const response = await fetch('/api/data/history?limit=100');
        const data = await response.json();
        
        if (data.length === 0) {
            console.log('No historical data available yet');
            return;
        }
        
        // Update stability gauge with history
        updateStabilityGauge(data);
        
        // Prepare data for charts
        const labels = data.map(d => {
            const date = new Date(d.timestamp);
            return date.toLocaleTimeString('vi-VN', { hour: '2-digit', minute: '2-digit' });
        });
        const temps = data.map(d => d.temperature);
        const moistures = data.map(d => d.moisture);
        const fans = data.map(d => d.fan_pwm);
        
        // Update temperature chart
        tempChart.data.labels = labels;
        tempChart.data.datasets[0].data = temps;
        tempChart.update('none');
        
        // Update moisture chart
        moistureChart.data.labels = labels;
        moistureChart.data.datasets[0].data = moistures;
        moistureChart.update('none');
        
        // Update fan PWM chart
        fanChart.data.labels = labels;
        fanChart.data.datasets[0].data = fans;
        fanChart.update('none');
        
        // Update data log table
        updateTable(data.slice(-20).reverse());
        
    } catch (error) {
        console.error('Error fetching history:', error);
    }
}

// Update the data log table
function updateTable(data) {
    const tableBody = document.getElementById('data-table');
    tableBody.innerHTML = '';
    
    if (data.length === 0) {
        tableBody.innerHTML = '<tr><td colspan="6" style="text-align: center; color: #64748b;">Chưa có dữ liệu</td></tr>';
        return;
    }
    
    data.forEach(row => {
        const tr = document.createElement('tr');
        // Convert UTC to Vietnam time (GMT+7)
        const date = new Date(row.timestamp);
        const vnTime = new Date(date.getTime() + (7 * 60 * 60 * 1000));
        
        tr.innerHTML = `
            <td>${vnTime.toLocaleString('vi-VN')}</td>
            <td>${row.temperature.toFixed(1)}</td>
            <td>${row.moisture.toFixed(1)}</td>
            <td>${row.temp_error.toFixed(2)}</td>
            <td>${row.fan_pwm}</td>
            <td class="${row.pump_active ? 'pump-on' : 'pump-off'}">
                ${row.pump_active ? 'BẬT' : 'TẮT'}
            </td>
        `;
        tableBody.appendChild(tr);
    });
}

// Initialize dashboard
function init() {
    console.log('BioLoop Monitor Scientific Dashboard initialized');
    
    // Initialize charts
    initCharts();
    
    // Initial data load
    fetchLatest();
    fetchHistory();
    
    // Set up auto-refresh intervals
    setInterval(fetchLatest, 3000);   // Update status every 3 seconds
    setInterval(fetchHistory, 10000); // Update charts/table every 10 seconds
}

// Start when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
} else {
    init();
}
