// Function to load data from CSV file and populate the table
document.addEventListener("DOMContentLoaded", function() {
    loadCSVData("data/data.csv");
});

function loadCSVData(csvFilePath) {
    fetch(csvFilePath)
        .then(response => {
            if (!response.ok) {
                throw new Error("Network response was not ok " + response.statusText);
            }
            return response.text();
        })
        .then(data => {
            populateTable(data);
        })
        .catch(error => console.error("Error loading CSV file:", error));
}

// Function to populate the table with CSV data
function populateTable(data) {
    const rows = data.split("\n").slice(1); // Split data into rows, skipping the header
    const tableBody = document.querySelector("#data-table tbody");
    
    rows.forEach(row => {
        const cells = row.split(",");
        const tr = document.createElement("tr");

        // Create table cells for each data item
        cells.forEach((cellData, index) => {
            const td = document.createElement("td");
            td.textContent = cellData.trim();
            
            // Add class for toggle functionality based on column index
            if (index === 1) td.classList.add("population");
            if (index === 2) td.classList.add("gdp");

            tr.appendChild(td);
        });

        tableBody.appendChild(tr);
    });
}

// Function to toggle the visibility of columns
function toggleColumn(columnClass) {
    const cells = document.querySelectorAll(`.${columnClass}`);
    cells.forEach(cell => {
        cell.style.display = cell.style.display === 'none' ? '' : 'none';
    });
}
