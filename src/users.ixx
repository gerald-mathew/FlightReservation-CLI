export module users;
import std;
import flights;
import sqlite3db;
import crypto;

export struct NextOfKin {
	std::string name{};
	std::string relationship{};
	std::string phoneNumber{};
	std::string address{};
};

export std::chrono::year_month_day parseDate(const std::string& dateStr);

class User {
protected:
	User();
    User(const std::string& aName, const std::string& aPassword, const std::string& aPhoneNumber = "", char aGender = '\0', const std::string& anAddress = "", const std::string& anEmail = "", const NextOfKin& aNextOfKin = {});

	// Protected members for derived classes to access hashes
	std::string passwordHash{};
	std::string passwordSalt{};
private:
	std::string name{};
	std::string phoneNumber{};
	char gender{};
	std::string address{};
	std::string email{};
	NextOfKin nextOfKin{};
public:
	// Setters
    void setName(const std::string& aName);
	bool setPhoneNumber(std::string aPhoneNumber);
	bool setGender(char aGender);
	void setAddress(const std::string& anAddress);
	bool setPassword(const std::string& aPassword);
	bool setEmail(const std::string& anEmail);
	void setNextOfKin(NextOfKin aNextOfKin);
	bool validatePassword(const std::string& aPassword) const;
	// Internal use only: set pre-computed hash and salt (for loading from storage)
    void setPasswordHashAndSalt(const std::string& hash, const std::string& salt) {
		passwordHash = hash;
		passwordSalt = salt;
	}

	// Getters
	std::string getName() const;
	std::string getPhoneNumber() const;
	char getGender() const;
	std::string getAddress() const;
	std::string getPasswordHash() const;
	std::string getPasswordSalt() const;
	std::string getEmail() const;
	NextOfKin getNextOfKin() const;

	~User();
};

export class Admin : public User {
private:
	std::string department{};
	std::string adminLevel{};
	std::chrono::year_month_day hireDate{};
public:
	Admin();
    Admin(const std::string& aName, const std::string& aPassword);
	Admin(const std::string& aName, const std::string& aPassword, const std::string& aPhoneNumber, char aGender, const std::string& anAddress, const std::string& anEmail, NextOfKin aNextOfKin, const std::string& aDepartment, const std::string& anAdminLevel, const std::string& aHireDate);
	Admin(const Admin& other);

	// Setters for Admin-specific attributes
	void setDepartment(const std::string& aDepartment);
	void setAdminLevel(const std::string& anAdminLevel);
	bool setHireDate(const std::string& aHireDate);

	// Getters for Admin-specific attributes
	std::string getDepartment() const;
	std::string getAdminLevel() const;
	std::string getHireDate() const;
};

export class Customer : public User {
private:
	int totalBookings{};
	double loyaltyPoints{};
	std::string accountCreationDate{};
	double accountBalance{};
	std::vector <std::string> transactionHistory{};
	std::vector <Flight> bookedFlights{};

public:
    Customer();
	Customer(const std::string& aName, const std::string& aPassword);
	Customer(const std::string& aName, const std::string& aPassword, const std::string& aPhoneNumber, char aGender, const std::string& anAddress, const std::string& anEmail, NextOfKin aNextOfKin, int aTotalBookings, double aLoyaltyPoints, const std::string& anAccountCreationDate, double anAccountBalance, std::vector<std::string> aTransactionHistory = {}, std::vector <Flight> aBookedFlight = {});
	Customer(const Customer& other);

	// Setters for Customer-specific attributes
	void setTotalBookings(int aBookings);
	void setLoyaltyPoints(double aPoints);
    void setAccountCreationDate(const std::string& aDate);
	void setAccountBalance(double aBalance);

	// Getters for Customer-specific attributes
	int getTotalBookings() const;
	double getLoyaltyPoints() const;
	std::string getAccountCreationDate() const;
	double getAccountBalance() const;
	std::vector <std::string> getTransactionHistory() const;
	std::vector <Flight> getBookedFlights() const;

	// Utility methods
	std::string getCurrentDate() const;
	void addBooking(Seat::Cabin cabin);
	bool addLoyaltyPoints(double aPoints);
	bool deductBalance(double anAmount);
	bool creditBalance(double anAmount);
    void addTransaction(const std::string& aTransaction);
	void addBookedFlight(const Flight& aFlight);
    bool removeBookedFlight(const std::string& flightId);
	bool removeBookedFlight(const Flight& aFlight);
};

export inline std::unordered_map<std::string, Admin> adminUsers;
export inline std::unordered_map<std::string, Customer> customerUsers;

export std::string readPassword();

export template <typename U> std::string validateUser(const std::unordered_map<std::string, U>& userCategory);

export bool createAdminUser(const std::string& username, const std::string& password);
export bool createCustomerUser(const std::string& username, const std::string& password);

export template <typename U> inline bool deleteUser(const std::string& username, std::unordered_map<std::string, U>& userCategory) {
	if (userCategory.contains(username)) {
		userCategory.erase(username);
		return true;
	}
	else {
		std::cout << "User with username '" << username << "' does not exist." << std::endl;
		return false;
	}
}

export template <typename U> void saveAccounts(const std::unordered_map<std::string, U>& userCategory);
export template <typename U> void loadAccounts(std::unordered_map<std::string, U>& userCategory);
export void loadAccounts();

export void saveAccounts();

// Payment reference tracking functions
export bool isPaymentReferenceUsed(const std::string& reference);

export bool markPaymentReferenceAsUsed(const std::string& reference);