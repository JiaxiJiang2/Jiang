/* ******************************************************************
** General functions
** ****************************************************************** */

/**
 * Shows a bootstrap toast with the message msg
 * and removes it from the DOM when it autohides.
 * 
 * @param {*} msg The message to display
 */

function showToast(msg) {
	const toasts = document.querySelector("#toasts");

	const newT = document.createElement("div");
	newT.classList.add("toast");
	newT.setAttribute("role", "alert");
	newT.setAttribute("aria-live", "assertive");
	newT.setAttribute("aria-atomic", "true");
	let inner = '<div class="d-flex">';
	inner += '<div class="toast-body">';
	inner += msg;
	inner += '</div>';
	inner += '<button type="button" class="btn-close me-2 m-auto" data-bs-dismiss="toast" aria-label="Close"></button>';
	inner += '</div>';
	newT.innerHTML = inner;

	newT.addEventListener('hidden.bs.toast', (e) => {
		e.target.remove();
	});

	toasts.append(newT);
	const bootstrapToast = bootstrap.Toast.getOrCreateInstance(newT);
	bootstrapToast.show();
}

/**
 * Updates the specified table with data.
 * 
 * @param {string} tableId The ID of the table body element to update.
 * @param {Array} data The data to display in the table.
 */
function updateTable(tableId, data) {
    const tbody = document.querySelector(`#${tableId}`);
    tbody.innerHTML = data.map(item => `
        <tr>
            <td>${item.id || '-'}</td>
            <td>${item.name || '-'}</td>
            <td>${item.birth_rate_per_1000 || '-'}</td>
            <td>${item.cell_phones_per_100 || '-'}</td>
            <td>${item.children_per_woman || '-'}</td>
            <td>${item.electricity_consumption_per_capita || '-'}</td>
            <td>${item.internet_user_per_100 || '-'}</td>
        </tr>
    `).join('');
}

/* ******************************************************************
** Initialization
** ****************************************************************** */
let initialData = [];
document.addEventListener('DOMContentLoaded', async () => {
    try {
        const response = await fetch('/api/items');
        const data = await response.json();

        if (Array.isArray(data) && data.length > 0) {
            initialData = data;
            updateTable('all_countries_tablebody', data);
        } else {
            showToast('No data available.');
        }
    } catch (error) {
        showToast('Failed to load initial data.');
    }
});

/* ******************************************************************
** ALL DATA ITEMS
** ****************************************************************** */
document.querySelector('#all_countries_reset').addEventListener('click', async () => {
    const response = await fetch('/api/reset', { method: 'POST' });
    const message = await response.text();

    if (response.status === 200) {
        const dataResponse = await fetch('/api/items');
        const data = await dataResponse.json();
        initialData = data;
        updateTable('all_countries_tablebody', data);
        updateTable('delete_country_tablebody', data);
        updateTable('add_country_tablebody', data);
        showToast(message);
    } else {
        showToast(message);
    }
});

/* ******************************************************************
** FILTERED DATA ITEMS (single)
** ****************************************************************** */
document.querySelector('#filter_single_submit').addEventListener('click', async () => {
    const id = document.querySelector('#country_filter_id').value;
    const response = await fetch(`/api/items/${id}`);
    const data = await response.json();
    updateTable('filtered_countries_tablebody', [data]);
});

document.querySelector('#filter_single_reset').addEventListener('click', () => {
    updateTable('filtered_countries_tablebody', []);
});

/* ******************************************************************
** FILTERED DATA ITEMS (range)
** ****************************************************************** */
document.querySelector('#filter_range_submit').addEventListener('click', async () => {
    const range = document.querySelector('#country_filter_range').value;
    const [id1, id2] = range.split('-');
    const response = await fetch(`/api/items/${id1}/${id2}`);
    const data = await response.json();
    updateTable('filtered_countries_tablebody', data);
});

document.querySelector('#filter_range_reset').addEventListener('click', () => {
    updateTable('filtered_countries_tablebody', []);
});

/* ******************************************************************
** ALL PROPERTIES
** ****************************************************************** */
document.querySelector('#properties_submit').addEventListener('click', async () => {
    const response = await fetch('/api/properties');
    const data = await response.json();
    const list = document.querySelector('#properties_list');
    list.innerHTML = data.map(prop => `<li class="list-group-item">${prop}</li>`).join('');
});

document.querySelector('#properties_reset').addEventListener('click', () => {
    document.querySelector('#properties_list').innerHTML = '';
});

/* ******************************************************************
** DELETE
** ****************************************************************** */
document.querySelector('#delete_submit').addEventListener('click', async () => {
    const response = await fetch('/api/items', { method: 'DELETE' });
    const message = await response.text();
    showToast(message);

	const updatedData = await fetch('/api/items').then(res => res.json());
    updateTable('all_countries_tablebody', updatedData);
    updateTable('delete_country_tablebody', updatedData);
});

/* ******************************************************************
** ADD
** ****************************************************************** */
document.querySelector('#add_country_submit').addEventListener('click', async () => {
    const name = document.querySelector('#add_country_name').value;
    const response = await fetch('/api/items', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ name })
    });
    const message = await response.text();
    showToast(message);

	const updatedData = await fetch('/api/items').then(res => res.json());
    updateTable('all_countries_tablebody', updatedData);
    updateTable('add_country_tablebody', updatedData);
});
