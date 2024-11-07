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

function toggleMenu() {
    const navMenu = document.querySelector('nav');
    const mainContent = document.querySelector('main'); // 假设正文部分用 <main> 标签包裹

    navMenu.classList.toggle('show-menu');
    
    if (navMenu.classList.contains('show-menu')) {
        mainContent.style.marginTop = navMenu.offsetHeight + "px"; // 将正文推到菜单下方
    } else {
        mainContent.style.marginTop = "60px"; // 恢复默认的 margin-top，与 header 对齐
    }
}


let ascending = true;

function sortTableByCountry() {
    const table = document.getElementById("data-table");
    const tbody = table.querySelector("tbody");
    const rows = Array.from(tbody.querySelectorAll("tr"));

    rows.sort((a, b) => {
        const countryA = a.cells[1].textContent.trim().toLowerCase();
        const countryB = b.cells[1].textContent.trim().toLowerCase();
        
        if (ascending) {
            return countryA.localeCompare(countryB);
        } else {
            return countryB.localeCompare(countryA);
        }
    });

    tbody.innerHTML = "";
    rows.forEach(row => tbody.appendChild(row));

    ascending = !ascending;

    document.querySelector(".asc-icon").classList.toggle("active", ascending);
    document.querySelector(".desc-icon").classList.toggle("active", !ascending);
}


document.addEventListener('DOMContentLoaded', loadCSVData);
