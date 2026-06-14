module;
#include <nlohmann/json.hpp>
export module crypto;
import std;

// Argon2 password hashing utility (installed via vcpkg)
export namespace Crypto {
	constexpr std::size_t HASH_LENGTH = 128;
	constexpr std::size_t SALT_LENGTH = 16;
	constexpr int ARGON2_TIME_COST = 2;
	constexpr int ARGON2_MEMORY_COST = 131072;  // 128 MiB
	constexpr int ARGON2_PARALLELISM = 1;

	std::string generateSalt(); // Generate a random salt for password hashing
	nlohmann::json hashPassword(const std::string& password); // Returns JSON object with hash and salt
	bool verifyPassword(const std::string& password, const std::string& storedHash, const std::string& salt); // Verify a password against a stored Argon2 hash
}