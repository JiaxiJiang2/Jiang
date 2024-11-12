function toggleColumn(columnClass, toggleId) {
    const cells = document.querySelectorAll(`.${columnClass}`);
    const toggleElements = document.querySelectorAll(`#${toggleId}`);
    
    cells.forEach(cell => {
        cell.style.display = cell.style.display === 'none' ? '' : 'none';
    });

    toggleElements.forEach(toggleElement => {
        toggleElement.classList.toggle('strikethrough');
    });
}

let originalOrder = [];
let isOriginalOrder = true;

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
        originalOrder.push(tr);
    });

    document.querySelector(".sort-icons").addEventListener("click", toggleSortOrder);
}

function toggleMenu() {
    const navMenu = document.querySelector('nav');
    navMenu.classList.toggle('show-menu');

    const mainContent = document.querySelector('main.container');
    mainContent.style.marginTop = navMenu.classList.contains('show-menu');
}

function toggleSortOrder() {
    const tbody = document.querySelector("#data-table tbody");

    if (isOriginalOrder) {
        const rows = Array.from(tbody.querySelectorAll("tr"));
        rows.sort((a, b) => {
            const numberA = parseInt(a.cells[0].textContent.trim(), 10);
            const numberB = parseInt(b.cells[0].textContent.trim(), 10);
            return numberB - numberA;
        });
        tbody.innerHTML = "";
        rows.forEach(row => tbody.appendChild(row));
    } else {
        tbody.innerHTML = "";
        originalOrder.forEach(row => tbody.appendChild(row.cloneNode(true)));
    }

    isOriginalOrder = !isOriginalOrder;

    document.querySelector(".asc-icon").classList.toggle("active", isOriginalOrder);
    document.querySelector(".desc-icon").classList.toggle("active", !isOriginalOrder);
}

document.addEventListener('DOMContentLoaded', loadCSVData);
