import * as d3 from "https://cdn.jsdelivr.net/npm/d3@7/+esm";

const csvFile = "data/world_data_v3.csv";
const jsonFile = "data/world_data.json";

let currentAttribute1 = "birth rate per 1000";
let currentAttribute2 = "gdp_per_capita";

document.addEventListener("DOMContentLoaded", () => {
    // Load CSV data for the first chart
    d3.csv(csvFile).then(dataCSV => {
        // Normalize keys to ensure proper mapping
        dataCSV.forEach(d => {
            for (const key in d) {
                const cleanKey = key.trim();
                d[cleanKey] = isNaN(+d[key]) ? d[key] : +d[key]; // Convert numeric fields
                if (key !== cleanKey) delete d[key];
            }
        });

        if (!dataCSV || dataCSV.length === 0) {
            console.error("CSV data is empty or not loaded correctly.");
            return;
        }

        // Load JSON data for the second chart
        d3.json(jsonFile).then(dataJSON => {
            if (!dataJSON || dataJSON.length === 0) {
                console.error("JSON data is empty or not loaded correctly.");
                return;
            }

            populateSelectBoxes(dataCSV, dataJSON); // Populate dropdown options
            createBarCharts(dataCSV, dataJSON);    // Create bar charts
            createMap(dataCSV);                   // Create map using CSV data
        }).catch(err => console.error("Error loading JSON data:", err));
    }).catch(err => console.error("Error loading CSV data:", err));

    // Update the first chart when the first dropdown changes
    document.getElementById("prop_selection1").addEventListener("change", (event) => {
        currentAttribute1 = event.target.value;
        d3.csv(csvFile).then(dataCSV => {
            updateCharts(dataCSV, d3.select("#bar1"), null, currentAttribute1);
        }).catch(err => console.error("Error updating first chart:", err));
    });

    // Update the second chart when the second dropdown changes
    document.getElementById("prop_selection2").addEventListener("change", (event) => {
        currentAttribute2 = event.target.value;
        d3.json(jsonFile).then(dataJSON => {
            updateCharts(dataJSON, null, d3.select("#bar2"), currentAttribute2);
        }).catch(err => console.error("Error updating second chart:", err));
    });
});

// Populate dropdowns with data attributes
function populateSelectBoxes(dataCSV, dataJSON) {
    const attributesCSV = Object.keys(dataCSV[0]).filter(attr => {
        return !["id", "name", "gps_lat", "gps_long"].includes(attr.trim());
    });

    const attributesJSON = Object.keys(dataJSON[0]).filter(attr => {
        return !["id", "name", "gps_lat", "gps_long"].includes(attr.trim());
    });

    const select1 = document.getElementById("prop_selection1");
    const select2 = document.getElementById("prop_selection2");
    select1.innerHTML = "";
    select2.innerHTML = "";

    attributesCSV.forEach(attr => {
        const option = document.createElement("option");
        option.value = attr;
        option.textContent = attr;
        select1.appendChild(option);
    });

    attributesJSON.forEach(attr => {
        const option = document.createElement("option");
        option.value = attr;
        option.textContent = attr;
        select2.appendChild(option);
    });

    select1.value = currentAttribute1;
    select2.value = currentAttribute2;
}

// Create bar charts
function createBarCharts(dataCSV, dataJSON) {
    updateCharts(dataCSV, d3.select("#bar1"), null, currentAttribute1);
    updateCharts(dataJSON, null, d3.select("#bar2"), currentAttribute2);
}

// Update bar charts
function updateCharts(data, chart1 = null, chart2 = null, attribute = "") {
    const width = 500;
    const height = 230;
    const margin = { top: 5, right: 5, bottom: 120, left: 30 };

    const filteredData = data.map(d => ({
        ...d,
        [attribute]: +d[attribute] || 0 // Ensure valid numeric data
    }));

    if (chart1) {
        const xScale1 = d3.scaleBand()
            .domain(filteredData.map(d => d.name.trim()))
            .range([margin.left, width - margin.right])
            .padding(0.2);

        const yScale1 = d3.scaleLinear()
            .domain([0, d3.max(filteredData, d => d[attribute])])
            .nice()
            .range([height - margin.bottom, margin.top]);

        chart1.selectAll("*").remove();

        chart1.append("g")
            .selectAll("rect")
            .data(filteredData)
            .join("rect")
            .attr("class", "bar")
            .attr("x", d => xScale1(d.name))
            .attr("y", d => yScale1(d[attribute]))
            .attr("width", xScale1.bandwidth())
            .attr("height", d => yScale1(0) - yScale1(d[attribute]));

        chart1.append("g")
            .attr("transform", `translate(0,${height - margin.bottom})`)
            .call(d3.axisBottom(xScale1))
            .selectAll("text")
            .attr("transform", "rotate(-90)")
            .attr("x", -5)
            .attr("y", -5)
            .style("text-anchor", "end");

        chart1.append("g")
            .attr("transform", `translate(${margin.left},0)`)
            .call(d3.axisLeft(yScale1).tickFormat(d3.format("~s")));
    }

    if (chart2) {
        const xScale2 = d3.scaleBand()
            .domain(filteredData.map(d => d.name.trim()))
            .range([margin.left, width - margin.right])
            .padding(0.2);

        const yScale2 = d3.scaleLinear()
            .domain([0, d3.max(filteredData, d => d[attribute])])
            .nice()
            .range([height - margin.bottom, margin.top]);

        chart2.selectAll("*").remove();

        chart2.append("g")
            .selectAll("rect")
            .data(filteredData)
            .join("rect")
            .attr("class", "bar")
            .attr("x", d => xScale2(d.name))
            .attr("y", d => yScale2(d[attribute]))
            .attr("width", xScale2.bandwidth())
            .attr("height", d => yScale2(0) - yScale2(d[attribute]));

        chart2.append("g")
            .attr("transform", `translate(0,${height - margin.bottom})`)
            .call(d3.axisBottom(xScale2))
            .selectAll("text")
            .attr("transform", "rotate(-90)")
            .attr("x", -5)
            .attr("y", -5)
            .style("text-anchor", "end");

        chart2.append("g")
            .attr("transform", `translate(${margin.left},0)`)
            .call(d3.axisLeft(yScale2).tickFormat(d3.format("~s")));
    }
}

// Create map
function createMap(data) {
    const map = L.map("map").setView([20, 0], 2);

    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; OpenStreetMap contributors'
    }).addTo(map);

    data.forEach(country => {
        const lat = +country.gps_lat;
        const lon = +country.gps_long;
        const value = country[currentAttribute1] || "N/A";

        if (!isNaN(lat) && !isNaN(lon)) {
            const marker = L.marker([lat, lon]).addTo(map);
            marker.bindPopup(`
                <b>${country.name}</b><br>
                ${currentAttribute1}: ${value}
            `);
        }
    });
}
