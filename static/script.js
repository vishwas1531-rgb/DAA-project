document.addEventListener('DOMContentLoaded', () => {
    const searchForm = document.getElementById('search-form');
    const reportForm = document.getElementById('report-form');
    const reportStatus = document.getElementById('report-status');

    if (searchForm) {
        searchForm.addEventListener('submit', async (e) => {
            e.preventDefault();

            const origin_id = document.getElementById('origin_id').value;
            const destination_id = document.getElementById('destination_id').value;
            const priority = document.getElementById('priority').value;

            if (origin_id === destination_id) {
                alert("Origin and Destination cannot be the same stop.");
                return;
            }

            const searchBtn = document.getElementById('search-btn');
            searchBtn.disabled = true;
            searchBtn.textContent = "Processing Algorithm...";

            try {
                const response = await fetch('/api/search', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ origin_id, destination_id, priority })
                });

                const data = await response.json();
                sessionStorage.setItem('journey_result', JSON.stringify(data));
                window.location.href = '/results';

            } catch (err) {
                alert("Failed to communicate with backend server: " + err.message);
                searchBtn.disabled = false;
                searchBtn.textContent = "Search Optimal Journey";
            }
        });
    }

    if (reportForm) {
        reportForm.addEventListener('submit', async (e) => {
            e.preventDefault();

            const route_id = document.getElementById('rep_route_id').value;
            const stop_id = document.getElementById('rep_stop_id').value;
            const crowd_level = document.getElementById('rep_crowd_level').value;

            const reportBtn = document.getElementById('report-btn');
            reportBtn.disabled = true;

            try {
                const response = await fetch('/api/report', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ route_id, stop_id, crowd_level })
                });

                const data = await response.json();
                if (response.ok) {
                    reportStatus.style.color = "green";
                    reportStatus.textContent = data.message;
                    reportForm.reset();
                } else {
                    reportStatus.style.color = "red";
                    reportStatus.textContent = data.error || "Failed to submit report.";
                }
            } catch (err) {
                reportStatus.style.color = "red";
                reportStatus.textContent = "Network error: " + err.message;
            } finally {
                reportBtn.disabled = false;
            }
        });
    }
});