import bodyParser from 'body-parser';
import csv from 'csvtojson';
import express from 'express';
import path from 'path';
import { fileURLToPath } from 'url';

// get the directory path of the current file
const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Init Express
const app = express();

// configure body-parser to parse JSON data
app.use(bodyParser.json());

// configure static file directory
app.use(express.static(path.join(__dirname, 'public')));

// the default route loads index.html
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// bind server to port
app.listen(3000, () => {
    console.log(`REST API server listening at http://localhost:3000`);
});

/* ******************************************************************
** LOAD CSV
** ****************************************************************** */
let records = [];
const csvFilePath = './world_data.csv';
records = await csv({delimiter: ';',}).fromFile(csvFilePath);

// console.log(records);

/* ******************************************************************
** HANDLE REQUESTS
** ****************************************************************** */

// "/api/items": GET all items as json
app.get('/api/items', (req, res) => {
    res.status(200).json(records);
});

// "/api/items": GET single item by id
app.get('/api/items/:id', (req, res) => {
    const id = req.params.id.toString();
    const item = records.find(r => r.id === id);
    if (item) {
        res.status(200).json(item);
    } else {
        res.status(205).send('Item not found');
    }
});

// "/api/items": GET items based on range (two ids)
app.get('/api/items/:id1/:id2', (req, res) => {
    let { id1, id2 } = req.params;
    id1 = parseInt(id1);
    id2 = parseInt(id2);

    if (id1 > id2) [id1, id2] = [id2, id1];
    const items = records.filter(r => r.id >= id1 && r.id <= id2);
    
    if (items.length > 0) {
        res.status(200).json(items);
    } else {
        res.status(205).send('No items in the given range');
    }
});

// "/api/properties": GET all properties
app.get('/api/properties', (req, res) => {
    if (records.length > 0) {
        const keys = Object.keys(records[0]);
        res.status(200).json(keys);
    } else {
        res.status(205).send('No properties found');
    }
});

// "/api/items": DELETE last item
app.post('/api/items', (req, res) => {
    const newItem = req.body;
    if (!newItem.name) {
        return res.status(400).send('Invalid input: name is required');
    }
    const lastId = records.length > 0
        ? Math.max(...records.map(r => parseInt(r.id || '0', 10)))
        : 0;
    const newId = (lastId + 1).toString().padStart(3, '0');
    const item = { id: newId, ...newItem };
    records.push(item);
    res.status(201).send(`Added country ${item.name} with ID ${item.id} to the list!`);
});

// "/api/items": POST add new item
app.delete('/api/items', (req, res) => {
    if (records.length > 0) {
        const deleted = records.pop();
        res.status(200).send(`Deleted last country: ${deleted.name}!`);
    } else {
        res.status(205).send('No items to delete');
    }
});