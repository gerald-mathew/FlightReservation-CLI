module;
#include <cstddef>
#include <string>
#include <nlohmann/json.hpp>
export module payment;

// Helper function to load secret key from file
std::string loadSecretKey();

// Paystack API Configuration - defined in payment.cpp
extern const std::string PAYSTACK_SECRET_KEY;

// Helper callback for libcurl to write response data
static std::size_t WriteCallback(void* contents, std::size_t size, std::size_t nmemb, std::string* userp);

export struct PaystackInitResult {
	bool ok{};

	std::string reference{};
	std::string authorization_url{};
	std::string message{};
	double amount{}; // in kobo
};

export struct PaystackVerifyResult {
	bool ok{};
	bool isSuccess{}; // true if status == "success"
	std::string status{};
	std::string message{};
	nlohmann::json data{}; // full transaction data
	double amount{}; // amount in kobo from Paystack
};

export double koboToNaira(long long kobo);

export long long nairaTOKobo(double naira);

// Helper function to make HTTP POST requests with authentication
static nlohmann::json makePaystackPostRequest(const std::string& endpoint, const nlohmann::json& payload);

// Helper function to make HTTP GET requests with authentication
static nlohmann::json makePaystackGetRequest(const std::string& endpoint);
// Paystack Initialize - Creates a payment transaction
export PaystackInitResult paystackInitialize(const std::string& email, double nairaAmount, const std::string& metadataJson = "{}");

export PaystackVerifyResult paystackVerify(const std::string& reference);

export void displayPaystackAuthorizationUrl(const std::string& url);