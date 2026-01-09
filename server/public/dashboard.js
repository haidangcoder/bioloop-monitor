// BioLoop Monitor - Dashboard JavaScript
// Data flow: API → fetch → update DOM & charts

// Chart.js instances (initialized in HTML)
let tempChart, fanChart;

// Fuzzy interpretation helper functions
// Based on design document fuzzy set definitions

function getTempErrorState(tempError) {
    // COLD: tempError < -2
    // OPTIMAL: -5 to +5 (centered at 0)
    // HOT: tempError > +2
    if (tempError < -2) return { label: 'COLD', color: '#2196F3' };
    if (tempError > 2) return { label: 'HOT', color: '#F44336' };
    return { label: 'OPTIMAL', color: '#4CAF50' };
}

function getMoistureState(moisture) {
    // DRY: 0-35%
    // OPTIMAL: 30-70% (centered at 50)
    // WET: 65-100%
    if (moisture < 35) return { label: 'DRY', color: '#FF9800' };
    if (moisture > 65) return { label: 'WET', color: '#2196F3' };
    return { label: 'OPTIMAL', color: '#4CAF50' };
}

function getFanLevel(fanPwm) {
    // OFF: 0-40
    // LOW: 30-130 (centered at 80)
    // MEDIUM: 100-200 (centered at 150)
    // HIGH: 180-255
    if (fanPwm <= 40) return { label: 'OFF', color: '#9E9E9E' };
    if (fanPwm <= 130) return { label: 'LOW', color: '#4CAF50' };
    if (fanPwm <= 200) return { label: 'MEDIUM', color: '#FF9800' };
    return { label: 'HIGH', color: '#F44336' };
}

// Initialize charts
function initCharts() {
    tempChart = new Chart(document.getElementById('tempChart'), {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Temperature (°C)',
                data: [],
                borderColor: '#FF6384',
                backgroundColor: 'rgba(255, 99, 132, 0.1)',
                tension: 0.4,
                fill: true
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: {
                    beginAtZero: false,
                    title: { display: true, text: 'Temperature (°C)' }
                },
                x: {
                    title: { display: true, text: 'Time' }
                }
            },
            plugins: {
                legend: { display: false }
            }
        }
    });

    fanChart = new Chart(document.getElementById('fanChart'), {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Fan PWM',
                data: [],
                borderColor: '#36A2EB',
                backgroundColor: 'rgba(54, 162, 235, 0.1)',
                tension: 0.4,
                fill: true
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: {
                    beginAtZero: true,
                    max: 255,
                    title: { display: true, text: 'PWM Value (0-255)' }
                },
                x: {
                    title: { display: true, text: 'Time' }
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
        const response = await fetch('/api/latest');
        const data = await response.json();
        
        if (data.id) {
            // Update status cards with values
            document.getElementById('current-temp').textContent = data.temperature.toFixed(1);
            document.getElementById('current-moisture').textContent = data.moisture.toFixed(1);
            document.getElementById('current-fan').textContent = data.fan_pwm;
            
            // Add fuzzy state labels
            const tempState = getTempErrorState(data.temp_error);
            const moistureState = getMoistureState(data.moisture);
            const fanState = getFanLevel(data.fan_pwm);
            
            // Update temperature label
            let tempLabel = document.getElementById('temp-state');
            if (!tempLabel) {
                tempLabel = document.createElement('div');
                tempLabel.id = 'temp-state';
                tempLabel.className = 'state-label';
                document.getElementById('current-temp').parentElement.appendChild(tempLabel);
            }
            tempLabel.textContent = tempState.label;
            tempLabel.style.color = tempState.color;
            
            // Update moisture label
            let moistureLabel = document.getElementById('moisture-state');
            if (!moistureLabel) {
                moistureLabel = document.createElement('div');
                moistureLabel.id = 'moisture-state';
                moistureLabel.className = 'state-label';
                document.getElementById('current-moisture').parentElement.appendChild(moistureLabel);
            }
            moistureLabel.textContent = moistureState.label;
            moistureLabel.style.color = moistureState.color;
            
            // Update fan label
            let fanLabel = document.getElementById('fan-state');
            if (!fanLabel) {
                fanLabel = document.createElement('div');
                fanLabel.id = 'fan-state';
                fanLabel.className = 'state-label';
                document.getElementById('current-fan').parentElement.appendChild(fanLabel);
            }
            fanLabel.textContent = fanState.label;
            fanLabel.style.color = fanState.color;
            
            // Update pump status with color
            const pumpElement = document.getElementById('current-pump');
            pumpElement.textContent = data.pump_active ? 'ON' : 'OFF';
            pumpElement.className = data.pump_active ? 'value pump-on' : 'value pump-off';
            
            // Update last update timestamp
            document.getElementById('last-update').textContent = new Date(data.timestamp).toLocaleString();
        }
    } catch (error) {
        console.error('Error fetching latest data:', error);
        document.getElementById('last-update').textContent = 'Error loading data';
    }
}

// Fetch history and update charts + table
async function fetchHistory() {
    try {
        const response = await fetch('/api/history?limit=100');
        const data = await response.json();
        
        if (data.length === 0) {
            console.log('No historical data available yet');
            return;
        }
        
        // Prepare data for charts
        const labels = data.map(d => {
            const date = new Date(d.timestamp);
            return date.toLocaleTimeString();
        });
        const temps = data.map(d => d.temperature);
        const fans = data.map(d => d.fan_pwm);
        
        // Update temperature chart
        tempChart.data.labels = labels;
        tempChart.data.datasets[0].data = temps;
        tempChart.update('none'); // 'none' = no animation for smooth updates
        
        // Update fan PWM chart
        fanChart.data.labels = labels;
        fanChart.data.datasets[0].data = fans;
        fanChart.update('none');
        
        // Update data log table (last 20 entries, newest first)
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
        tableBody.innerHTML = '<tr><td colspan="6" style="text-align: center; color: #999;">No data available</td></tr>';
        return;
    }
    
    data.forEach(row => {
        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td>${new Date(row.timestamp).toLocaleString()}</td>
            <td>${row.temperature.toFixed(1)}</td>
            <td>${row.moisture.toFixed(1)}</td>
            <td>${row.temp_error.toFixed(2)}</td>
            <td>${row.fan_pwm}</td>
            <td class="${row.pump_active ? 'pump-on' : 'pump-off'}">
                ${row.pump_active ? 'ON' : 'OFF'}
            </td>
        `;
        tableBody.appendChild(tr);
    });
}

// Initialize dashboard
function init() {
    console.log('BioLoop Monitor Dashboard initialized');
    
    // Initialize charts
    initCharts();
    
    // Initial data load
    fetchLatest();
    fetchHistory();
    
    // Set up auto-refresh intervals
    setInterval(fetchLatest, 3000);  // Update status every 3 seconds
    setInterval(fetchHistory, 10000); // Update charts/table every 10 seconds
}

// Start when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
} else {
    init();
}
