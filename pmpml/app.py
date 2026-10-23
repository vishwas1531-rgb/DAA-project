import os
import csv
import json
import subprocess
from datetime import datetime
from flask import Flask, render_template, request, jsonify

app = Flask(__name__)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_DIR = os.path.join(BASE_DIR, "data")
CPP_EXE = os.path.join(BASE_DIR, "engine.exe" if os.name == "nt" else "engine")


def get_all_stops():
    stops = []
    stops_file = os.path.join(DATA_DIR, "stops.csv")
    if os.path.exists(stops_file):
        with open(stops_file, mode="r", encoding="utf-8") as f:
            reader = csv.DictReader(f)
            for row in reader:
                try:
                    stops.append({
                        "stop_id": int(row["stop_id"]),
                        "stop_name": row["stop_name"]
                    })
                except (KeyError, ValueError):
                    continue
    return sorted(stops, key=lambda x: x["stop_name"])


def get_all_routes():
    routes = []
    routes_file = os.path.join(DATA_DIR, "routes.csv")
    if os.path.exists(routes_file):
        with open(routes_file, mode="r", encoding="utf-8") as f:
            reader = csv.DictReader(f)
            for row in reader:
                routes.append(row)
    return routes


@app.route("/")
def index():
    stops = get_all_stops()
    routes = get_all_routes()
    return render_template("index.html", stops=stops, routes=routes)


@app.route("/api/stops", methods=["GET"])
def api_stops():
    return jsonify(get_all_stops())


@app.route("/api/search", methods=["POST"])
def api_search():
    data = request.get_json() or {}
    origin_id = data.get("origin_id")
    destination_id = data.get("destination_id")
    priority = data.get("priority", "balanced")

    if not origin_id or not destination_id:
        return jsonify({"error": "Please select valid origin and destination stops."}), 400

    if origin_id == destination_id:
        return jsonify({"error": "Origin and destination stops cannot be identical."}), 400

    if not os.path.exists(CPP_EXE):
        return jsonify({"error": f"C++ engine binary not found at '{CPP_EXE}'. Please run run.bat to compile."}), 500

    cmd = [CPP_EXE, DATA_DIR, str(origin_id), str(destination_id), str(priority)]

    try:
        proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=10)
        if proc.returncode != 0 and not proc.stdout:
            return jsonify({"error": f"C++ engine error: {proc.stderr.strip()}"}), 500

        try:
            result = json.loads(proc.stdout)
            return jsonify(result)
        except json.JSONDecodeError:
            return jsonify({"error": f"Invalid JSON returned from engine: {proc.stdout.strip()}"}), 500

    except Exception as e:
        return jsonify({"error": f"Execution failed: {str(e)}"}), 500


@app.route("/api/report", methods=["POST"])
def api_report():
    data = request.get_json() or {}
    route_id = data.get("route_id")
    stop_id = data.get("stop_id")
    crowd_level = data.get("crowd_level")
    passenger_count = data.get("passenger_count", 30)

    if not route_id or not stop_id or not crowd_level:
        return jsonify({"error": "Missing required fields for crowd report."}), 400

    try:
        crowd_level = int(crowd_level)
        stop_id = int(stop_id)
        passenger_count = int(passenger_count)
        if crowd_level < 1 or crowd_level > 5:
            raise ValueError()
    except ValueError:
        return jsonify({"error": "Crowd level must be an integer between 1 and 5."}), 400

    reports_file = os.path.join(DATA_DIR, "crowd_reports.csv")
    new_id = 1

    if os.path.exists(reports_file):
        with open(reports_file, mode="r", encoding="utf-8") as f:
            reader = list(csv.DictReader(f))
            if reader:
                try:
                    new_id = max(int(row["report_id"]) for row in reader if row["report_id"].isdigit()) + 1
                except ValueError:
                    new_id = len(reader) + 1

    now_iso = datetime.now().strftime("%Y-%m-%dT%H:%M:%S")

    file_exists = os.path.exists(reports_file)
    with open(reports_file, mode="a", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        if not file_exists:
            writer.writerow(["report_id", "route_id", "stop_id", "crowd_level", "passenger_count", "report_time", "source", "confidence"])
        writer.writerow([new_id, route_id, stop_id, crowd_level, passenger_count, now_iso, "User_App", 0.85])

    return jsonify({"status": "success", "message": "Crowd report submitted successfully!", "report_id": new_id})


@app.route("/results")
def results_page():
    return render_template("results.html")


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000, debug=True)