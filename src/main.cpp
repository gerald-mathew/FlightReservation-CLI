import std;
import menus;
import sqlite3db;

int main() {
	if (!std::filesystem::is_directory("data")) {
		std::filesystem::create_directory("data");
	}
	// Initialize database connection
	SQLite3DB* db = SQLite3DB::getInstance();
	db->connect("data/flight_reservation.db");

	loadFlights();
	loadAccounts();
	runMainMenu();

	db->disconnect();

	return 0;
}