import * as d3 from "https://cdn.jsdelivr.net/npm/d3@7/+esm";

const csvFile = "data/world_data_v3.csv";
const jsonFile = "data/world_data.json";

let currentAttribute1 = "birth rate per 1000";
let currentAttribute2 = "gdp_per_capita";

const markersMap = new Map(); //save mapmaker

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
        }).catch(err => console.error("update charts1 error", err));
    });

    document.getElementById("prop_selection2").addEventListener("change", (event) => {
        currentAttribute2 = event.target.value;
        d3.json(jsonFile).then(dataJSON => {
            dataJSON = cleanData(dataJSON);
            updateBarChart(dataJSON, d3.select("#bar2"), currentAttribute2);
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

            d3.selectAll(".bar").filter(bar => bar.name === d.name)
                .attr("fill", "blue");

            if (markersMap.has(d.name)) {
                markersMap.get(d.name).setIcon(L.icon({
                    iconUrl: 'data/location-pin-hover.png',
                    iconSize: [35, 35],
                    iconAnchor: [17, 35],
                    popupAnchor: [0, -35]
                }));
            }
        })
        .on("mouseout", (event, d) => {
            d3.select(event.currentTarget).attr("fill", "grey");

            d3.selectAll(".bar").filter(bar => bar.name === d.name)
                .attr("fill", "grey");

            if (markersMap.has(d.name)) {
                markersMap.get(d.name).setIcon(L.icon({
                    iconUrl: 'data/location-pin.png',
                    iconSize: [30, 30],
                    iconAnchor: [15, 30],
                    popupAnchor: [0, -30]
                }));
            }
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

            marker.bindPopup(`<b>${country.name}</b>`);

            marker.on("mouseover", () => {
                marker.setIcon(L.icon({
                    iconUrl: 'data/location-pin-hover.png',
                    iconSize: [35, 35],
                    iconAnchor: [17, 35],
                    popupAnchor: [0, -35]
                }));

                d3.selectAll(".bar").filter(bar => bar.name === country.name)
                    .attr("fill", "blue");
            });

            marker.on("mouseout", () => {
                marker.setIcon(customIcon);

                d3.selectAll(".bar").filter(bar => bar.name === country.name)
                    .attr("fill", "grey");
            });
        }
    });
}
