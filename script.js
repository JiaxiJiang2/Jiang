import * as d3 from "https://cdn.jsdelivr.net/npm/d3@7/+esm";

const csvFile = "data/world_data_v3.csv";
const jsonFile = "data/world_data.json";

let currentAttribute1 = "birth rate per 1000";
let currentAttribute2 = "gdp_per_capita";

const markersMap = new Map(); // 保存地图标记

document.addEventListener("DOMContentLoaded", () => {
    // load csv data
    d3.csv(csvFile).then(dataCSV => {
        dataCSV = cleanData(dataCSV);
        if (!dataCSV || dataCSV.length === 0) {
            console.error("CSV error");
            return;
        }

        // load json data
        d3.json(jsonFile).then(dataJSON => {
            if (!dataJSON || dataJSON.length === 0) {
                console.error("JSON error");
                return;
            }

            dataJSON = cleanData(dataJSON);

            populateSelectBoxes(dataCSV, dataJSON);
            createVisualizations(dataCSV, dataJSON);
        }).catch(err => console.error("load json error", err));
    }).catch(err => console.error("load csv error", err));

    document.getElementById("prop_selection1").addEventListener("change", (event) => {
        currentAttribute1 = event.target.value;
        d3.csv(csvFile).then(dataCSV => {
            dataCSV = cleanData(dataCSV);
            updateBarChart(dataCSV, d3.select("#bar1"), currentAttribute1);
            updateMapPopups(dataCSV);
        }).catch(err => console.error("update charts1 error", err));
    });

    document.getElementById("prop_selection2").addEventListener("change", (event) => {
        currentAttribute2 = event.target.value;
        d3.json(jsonFile).then(dataJSON => {
            dataJSON = cleanData(dataJSON);
            updateBarChart(dataJSON, d3.select("#bar2"), currentAttribute2);
            updateMapPopups(dataJSON);
        }).catch(err => console.error("update charts2 error", err));
    });
});

//clean data
function cleanData(data) {
    return data.map(d => {
        const cleaned = {};
        for (const key in d) {
            const cleanKey = key.trim();
            cleaned[cleanKey] = isNaN(+d[key]) ? d[key].trim() : +d[key];
        }
        return cleaned;
    });
}

//pop up window
function populateSelectBoxes(dataCSV, dataJSON) {
    const attributesCSV = Object.keys(dataCSV[0]).filter(attr => !["id", "name", "gps_lat", "gps_long"].includes(attr));
    const attributesJSON = Object.keys(dataJSON[0]).filter(attr => !["id", "name", "gps_lat", "gps_long"].includes(attr));

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

// visualisierung
function createVisualizations(dataCSV, dataJSON) {
    updateBarChart(dataCSV, d3.select("#bar1"), currentAttribute1);
    updateBarChart(dataJSON, d3.select("#bar2"), currentAttribute2);
    createMap(dataCSV);
}

// update charts
function updateBarChart(data, svg, attribute) {
    const width = 500;
    const height = 250;
    const margin = { top: 20, right: 20, bottom: 130, left: 50 };

    const filteredData = data.filter(d => d[attribute] !== undefined && d[attribute] !== null);

    const xScale = d3.scaleBand()
        .domain(filteredData.map(d => d.name))
        .range([margin.left, width - margin.right])
        .padding(0.2);

    const yScale = d3.scaleLinear()
        .domain([0, d3.max(filteredData, d => d[attribute])])
        .nice()
        .range([height - margin.bottom, margin.top]);

    svg.attr("width", width).attr("height", height).selectAll("*").remove();

    svg.append("g")
        .selectAll("rect")
        .data(filteredData)
        .join("rect")
        .attr("class", "bar")
        .attr("x", d => xScale(d.name))
        .attr("y", d => yScale(d[attribute]))
        .attr("height", d => yScale(0) - yScale(d[attribute]))
        .attr("width", xScale.bandwidth())
        .attr("fill", "grey")
        .on("mouseover", (event, d) => {
            d3.select(event.currentTarget).attr("fill", "blue");
            highlightMarker(d.name, true);
        })
        .on("mouseout", (event, d) => {
            d3.select(event.currentTarget).attr("fill", "grey");
            highlightMarker(d.name, false);
        });

    svg.append("g")
        .attr("transform", `translate(0,${height - margin.bottom})`)
        .call(d3.axisBottom(xScale).tickSizeOuter(0))
        .selectAll("text")
        .attr("transform", "rotate(-90)")
        .attr("x", -5)
        .attr("y", -5)
        .style("text-anchor", "end");

    svg.append("g")
        .attr("transform", `translate(${margin.left},0)`)
        .call(d3.axisLeft(yScale));
}

//create map
function createMap(data) {
    const map = L.map("map").setView([20, 0], 2);

    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; OpenStreetMap contributors'
    }).addTo(map);

    const customIcon = L.icon({
        iconUrl: 'data/location-pin.png',
        iconSize: [30, 30],
        iconAnchor: [15, 30],
        popupAnchor: [0, -30]
    });

    data.forEach(country => {
        const lat = +country.gps_lat;
        const lon = +country.gps_long;

        if (!isNaN(lat) && !isNaN(lon)) {
            const marker = L.marker([lat, lon], { icon: customIcon }).addTo(map);
            markersMap.set(country.name, marker);

            marker.bindPopup(() => {
                const value1 = country[currentAttribute1] || "N/A";
                const value2 = country[currentAttribute2] || "N/A";

                return `
                    <div>
                        <b>${country.name}</b><br>
                        ${currentAttribute1}: <b>${value1}</b><br>
                        ${currentAttribute2}: <b>${value2}</b>
                    </div>
                `;
            });

            marker.on("mouseover", () => {
                highlightBar(country.name, true);
                highlightMarker(country.name, true);
            });

            marker.on("mouseout", () => {
                highlightBar(country.name, false);
                highlightMarker(country.name, false);
            });
        }
    });
}

function highlightMarker(countryName, highlight) {
    const marker = markersMap.get(countryName);
    if (marker) {
        marker.setIcon(L.icon({
            iconUrl: highlight ? 'data/location-pin-hover.png' : 'data/location-pin.png',
            iconSize: [30, 30],
            iconAnchor: [15, 30],
            popupAnchor: [0, -30]
        }));
    }
}

function highlightBar(countryName, highlight) {
    d3.selectAll(".bar").filter(d => d.name === countryName)
        .attr("fill", highlight ? "blue" : "grey");
}

//map popup window
function updateMapPopups(data) {
    markersMap.forEach((marker, countryName) => {
        const country = data.find(d => d.name === countryName);
        const value1 = country ? country[currentAttribute1] : "N/A";
        const value2 = country ? country[currentAttribute2] : "N/A";

        marker.setPopupContent(`
            <div>
                <b>${countryName}</b><br>
                ${currentAttribute1}: <b>${value1}</b><br>
                ${currentAttribute2}: <b>${value2}</b>
            </div>
        `);
    });
}
