// script.js

function toggleColumn(columnClass, toggleId) {
    const cells = document.querySelectorAll(`.${columnClass}`);
    const toggleElement = document.getElementById(toggleId);
    
    cells.forEach(cell => {
        cell.style.display = cell.style.display === 'none' ? '' : 'none';
    });

    if (toggleElement.classList.contains('strikethrough')) {
        toggleElement.classList.remove('strikethrough');
    } else {
        toggleElement.classList.add('strikethrough');
    }
}

async function loadCSVData() {
    const response = await fetch('data/data.csv');
    const data = await response.text();
    const rows = data.split('\n').slice(1);
    
    const tableBody = document.querySelector('#data-table tbody');
    rows.forEach(row => {
        const cols = row.split(',');
        const tr = document.createElement('tr');
        cols.forEach((col, index) => {
            const td = document.createElement('td');
            td.textContent = col.trim();
            
            if (index === 2) td.classList.add('birth_rate');
            else if (index === 3) td.classList.add('children_per_woman');
            else if (index === 4) td.classList.add('cellphones');
            else if (index === 5) td.classList.add('electric_usage');
            else if (index === 6) td.classList.add('internet_usage');
            
            tr.appendChild(td);
        });
        tableBody.appendChild(tr);
    });
}

document.addEventListener('DOMContentLoaded', loadCSVData);
