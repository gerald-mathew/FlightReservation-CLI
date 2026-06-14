module menus;

// Reads and validates integer input, returns -1 on invalid input
int getValidInt() {
	int choice;
	std::cin >> choice;

	if (std::cin.fail()) {
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		return -1;
	}

	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	return choice;
}

// Reads and validates double input, returns -1.0 on invalid input
double getValidDouble() {
	double value{};
	std::cin >> value;

	if (std::cin.fail()) {
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		return -1.0;
	}

	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	return value;
}

void runCustomerSession(Customer& customer, const std::string& username) {
	char customerRunAgain = 'N';
	do {
		std::cout << "1. Book Flight\n2. View Booked flights\n3. Cancel Booking\n4. Deposit to Nexora Account\n5. Withdraw from Nexora account\n6. Transfer to another Nexora account\n7. View Account Info\n8. Update account information\n9. Logout" << std::endl;
		std::cout << "Please select an option: ";
		int customerChoice = getValidInt();
		if (customerChoice == 1) {
			if (flights.empty()) {
				std::cout << "No flights available at the moment." << std::endl;
			}
			else {
				std::cout << "\nAvailable flights:" << std::endl;
				for (const auto& [id, f] : flights) {
					std::cout << "ID: " << id << " | " << f.getAirline() << " | " << f.getOrigin() << " -> " << f.getDestination() << " | Dep: " << f.getDepartureTime() << " | Base Price: NGN" << std::fixed << std::setprecision(2) << f.getTicketPrice() << "  | Seats: " << f.getAvailableSeats() << "/" << f.getTotalSeats() << std::endl;
				}

				std::cout << "\nEnter the flight ID you want to book: ";
				std::string flightId;
				std::getline(std::cin, flightId);
				if (flightId.empty()) {
					std::cout << "Flight ID cannot be empty." << std::endl;
				}
				else {
					auto flightIt = flights.find(flightId);
					if (flightIt == flights.end()) {
						std::string upper = flightId;
						std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
						flightIt = flights.find(upper);
					}

					if (flightIt == flights.end()) {
						std::cout << "Flight '" << flightId << "' not found." << std::endl;
					}
					else {
						Flight& f = flightIt->second;
						if (!f.hasAvailableSeats()) {
							std::cout << "This flight has no available seats." << std::endl;
						}
						else {
							bool alreadyBooked = false;
							for (const auto& bookedF : customer.getBookedFlights()) {
								if (bookedF.getId() == f.getId()) {
									alreadyBooked = true;
									break;
								}
							}
							if (alreadyBooked) {
								std::cout << "You have already booked flight '" << f.getId() << "'." << std::endl;
							}
							else {
								std::cout << "Enter seat number to book (e.g., 1A): ";
								std::string seatNumber;
								std::getline(std::cin, seatNumber);
								Seat* seat = f.findSeat(seatNumber);
								if (!seat) {
									std::cout << "Seat '" << seatNumber << "' does not exist on this flight." << std::endl;
								}
								else if (seat->getIsBooked()) {
									std::cout << "Seat '" << seat->getSeatNumber() << "' is already booked." << std::endl;
								}
								else {
									// Pricing multiplier based on cabin type: FirstClass (2x), Business (1.5x), Economy (1x)
									double multiplier = 1.0;
									std::string bookingLabel = "Standard flight";
									switch (seat->getCabin()) {
									case Seat::Cabin::FirstClass:
										multiplier = 2.0;
										bookingLabel = "First class flight";
										break;
									case Seat::Cabin::Business:
										multiplier = 1.5;
										bookingLabel = "Business class flight";
										break;
									default:
										multiplier = 1.0;
										bookingLabel = "Standard flight";
										break;
									}

									double price = f.getTicketPrice() * multiplier;
									if (customer.getAccountBalance() < price) {
										std::cout << "Insufficient balance. Your balance is NGN" << std::fixed << std::setprecision(2) << customer.getAccountBalance() << " , but this booking costs NGN" << std::fixed << std::setprecision(2) << price << " ." << std::endl;
										std::cout << "Please deposit funds first (option 4)." << std::endl;
									}
                                    else if (f.bookSeat(std::string(seat->getSeatNumber()))) {
										customer.deductBalance(price);
										customer.addBookedFlight(f);
										customer.addBooking(seat->getCabin());

										std::ostringstream oss;
										oss << "Flight booking: " << f.getId() << " seat " << seat->getSeatNumber() << " (NGN" << std::fixed << std::setprecision(2) << price << ")";
										customer.addTransaction(oss.str());

										std::cout << "Booking successful! Flight '" << f.getId() << "' seat '" << seat->getSeatNumber() << "' booked." << std::endl;
										saveFlights();
										saveAccounts(customerUsers);
									}
									else {
										std::cout << "Failed to book seat '" << seat->getSeatNumber() << "'." << std::endl;
									}
								}
							}
						}
					}
				}
			}
		}
		else if (customerChoice == 2) {
			const auto booked = customer.getBookedFlights();
			if (booked.empty()) {
				std::cout << "You have no booked flights." << std::endl;
			}
			else {
				std::cout << "\nYour booked flights:" << std::endl;
				for (const auto& f : booked) {
					std::cout << "ID: " << f.getId() << " | " << f.getAirline() << " | " << f.getOrigin() << " -> " << f.getDestination() << " | Dep: " << f.getDepartureTime() << std::endl;
				}
				std::cout << "\nNote: Seat selection is stored in flight seat map, not per-user booking list." << std::endl;
			}
		}
		else if (customerChoice == 3) {
			const auto booked = customer.getBookedFlights();
			if (booked.empty()) {
				std::cout << "You have no bookings to cancel." << std::endl;
			}
			else {
				std::cout << "\nBooked flights:" << std::endl;
				for (const auto& f : booked) {
					std::cout << "- " << f.getId() << " (" << f.getOrigin() << " -> " << f.getDestination() << " | Dep: " << f.getDepartureTime() << ")" << std::endl;
				}

				std::cout << "\nEnter the flight ID to cancel: ";
				std::string flightId;
				std::getline(std::cin, flightId);
				if (flightId.empty()) {
					std::cout << "Flight ID cannot be empty." << std::endl;
				}
				else {
					auto flightIt = flights.find(flightId);
					if (flightIt == flights.end()) {
						std::string upper = flightId;
						std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
						flightIt = flights.find(upper);
					}
					if (flightIt == flights.end()) {
						std::cout << "Flight '" << flightId << "' not found." << std::endl;
					}
					else {
						Flight& f = flightIt->second;
						std::cout << "Enter seat number to cancel (e.g., 1A): ";
						std::string seatNumber;
						std::getline(std::cin, seatNumber);
						Seat* seat = f.findSeat(seatNumber);
						if (!seat) {
							std::cout << "Seat '" << seatNumber << "' does not exist on this flight." << std::endl;
						}
						else {
							double multiplier = 1.0;
							switch (seat->getCabin()) {
							case Seat::Cabin::FirstClass: multiplier = 2.0; break;
							case Seat::Cabin::Business: multiplier = 1.5; break;
							default: multiplier = 1.0; break;
							}
							double refundAmount = f.getTicketPrice() * multiplier;

							if (!f.cancelSeat(std::string(seat->getSeatNumber()), seat->getBookedTime())) {
								std::cout << "Failed to cancel booking for seat '" << seat->getSeatNumber() << "'. Make sure it is currently booked." << std::endl;
							}
							else {
								customer.creditBalance(refundAmount);
								customer.removeBookedFlight(std::string(f.getId()));

								std::ostringstream oss;
								oss << "Booking cancelled: " << f.getId() << " seat " << seat->getSeatNumber() << " (Refund NGN" << std::fixed << std::setprecision(2) << refundAmount << ")";
								customer.addTransaction(oss.str());

								std::cout << "Cancelled booking for flight '" << f.getId() << "' seat '" << seat->getSeatNumber() << "'. Refund credited: NGN" << std::fixed << std::setprecision(2) << refundAmount << " " << std::endl;
								saveFlights();
								saveAccounts(customerUsers);
							}
						}
					}
				}
			}
		}
		else if (customerChoice == 4) {
			std::cout << "\nDeposit to Nexora Account (via Paystack)" << std::endl;
			std::cout << "Enter deposit amount (NGN): ";
			double amount = getValidDouble();
			if (amount <= 0) {
				std::cout << "Invalid amount." << std::endl;
			}
			else {
				std::string email = customer.getEmail();
				bool haveEmail = true;
				if (email.empty()) {
					std::cout << "Enter your email for Paystack (required): ";
					std::getline(std::cin, email);
					if (email.empty()) {
						std::cout << "Email cannot be empty." << std::endl;
						haveEmail = false;
					}
					else {
						(void)customer.setEmail(email);
					}
				}
				if (!haveEmail) {
					std::cout << "Deposit cancelled." << std::endl;
				}
				else {
					auto init = paystackInitialize(email, amount);
					if (!init.ok || init.authorization_url.empty() || init.reference.empty()) {
						std::cout << "Paystack initialization failed: " << init.message << std::endl;
					}
					else {
						displayPaystackAuthorizationUrl(init.authorization_url);
						std::cout << "Reference: " << init.reference << std::endl;
						std::cout << "Enter reference to verify (press Enter to use the above reference): ";
						std::string ref;
						std::getline(std::cin, ref);
						if (ref.empty()) ref = init.reference;

						auto verify = paystackVerify(ref);
						if (!verify.ok) {
							std::cout << "Verification failed: " << verify.message << std::endl;
						}
						else if (!verify.isSuccess) {
							std::cout << "Payment not successful yet. Status: " << verify.status << std::endl;
							std::cout << "You can try again after completing payment in your browser." << std::endl;
						}
						else if (isPaymentReferenceUsed(ref)) {
							std::cout << "Error: This payment reference has already been used for a deposit. " << std::endl;
							std::cout << "A reference can only be used once. Please contact support if you believe this is an error." << std::endl;
						}
						else {
							double credited = koboToNaira(static_cast<long long>(verify.amount));
							customer.creditBalance(credited);

							std::ostringstream oss;
							oss << "Deposit (Paystack): NGN" << std::fixed << std::setprecision(2) << credited << " | Ref: " << ref;
							customer.addTransaction(oss.str());

							// Mark the reference as used to prevent duplicate deposits
							if (markPaymentReferenceAsUsed(ref)) {
								std::cout << "Deposit successful. Amount credited: NGN" << std::fixed << std::setprecision(2) << credited << std::endl;
								saveAccounts(customerUsers);
							}
							else {
								std::cout << "Deposit processed but failed to register reference. Please contact support." << std::endl;
								saveAccounts(customerUsers);
							}
						}
					}
				}
			}
		}
		else if (customerChoice == 5) {
			std::cout << "\nWithdraw from Nexora Account" << std::endl;
			std::cout << "Current balance: NGN" << std::fixed << std::setprecision(2) << customer.getAccountBalance() << " " << std::endl;
			std::cout << "Enter withdrawal amount (NGN): ";
			double amount = getValidDouble();
			if (amount <= 0) {
				std::cout << "Invalid amount." << std::endl;
			}
			else if (amount > customer.getAccountBalance()) {
				std::cout << "Insufficient balance." << std::endl;
			}
			else {
				customer.deductBalance(amount);
				std::ostringstream oss;
				oss << "Withdrawal: - NGN" << std::fixed << std::setprecision(2) << amount;
				customer.addTransaction(oss.str());
				std::cout << "Withdrawal successful. New balance: NGN" << std::fixed << std::setprecision(2) << customer.getAccountBalance() << std::endl;
				saveAccounts(customerUsers);
			}
		}
		else if (customerChoice == 6) {
			std::cout << "\nTransfer to another Nexora account" << std::endl;
			std::cout << "Enter recipient username: ";
			std::string recipient;
			std::getline(std::cin, recipient);
			if (recipient.empty()) {
				std::cout << "Recipient username cannot be empty." << std::endl;
			}
			else if (recipient == username) {
				std::cout << "You cannot transfer to yourself." << std::endl;
			}
			else if (!customerUsers.contains(recipient)) {
				std::cout << "Recipient '" << recipient << "' does not exist." << std::endl;
			}
			else {
				std::cout << "Your balance: NGN" << std::fixed << std::setprecision(2) << customer.getAccountBalance() << std::endl;
				std::cout << "Enter transfer amount (NGN): ";
				double amount = getValidDouble();
				if (amount <= 0) {
					std::cout << "Invalid amount." << std::endl;
				}
				else if (amount > customer.getAccountBalance()) {
					std::cout << "Insufficient balance." << std::endl;
				}
				else {
					Customer& recipientCustomer = customerUsers.at(recipient);
					if (customer.deductBalance(amount)) recipientCustomer.creditBalance(amount);

					{
						std::ostringstream oss;
						oss << "Transfer to " << recipient << ": - NGN" << std::fixed << std::setprecision(2) << amount;
						customer.addTransaction(oss.str());
					}
					{
						std::ostringstream oss;
						oss << "Transfer from " << username << ": + NGN" << std::fixed << std::setprecision(2) << amount;
						recipientCustomer.addTransaction(oss.str());
					}

					std::cout << "Transfer successful. New balance: NGN" << std::fixed << std::setprecision(2) << customer.getAccountBalance() << std::endl;
					saveAccounts(customerUsers);
				}
			}
		}
		else if (customerChoice == 7) {
			std::cout << "\n--- Account Info ---" << std::endl;
			std::cout << "Username: " << username << std::endl;
			std::cout << "Name: " << customer.getName() << std::endl;
			std::cout << "Email: " << customer.getEmail() << std::endl;
			std::cout << "Phone: " << customer.getPhoneNumber() << std::endl;
			std::cout << "Gender: " << customer.getGender() << std::endl;
			std::cout << "Address: " << customer.getAddress() << std::endl;
			std::cout << "Balance: NGN" << std::fixed << std::setprecision(2) << customer.getAccountBalance() << std::endl;
			std::cout << "Total bookings: " << customer.getTotalBookings() << std::endl;
			std::cout << "Loyalty points: " << customer.getLoyaltyPoints() << std::endl;
			const auto booked = customer.getBookedFlights();
			std::cout << "Booked flights: " << booked.size() << std::endl;
			if (!booked.empty()) {
				for (const auto& f : booked) {
					std::cout << "- " << f.getId() << " (" << f.getOrigin() << " -> " << f.getDestination() << " | Dep: " << f.getDepartureTime() << ")" << std::endl;
				}
			}

			const auto history = customer.getTransactionHistory();
			if (!history.empty()) {
				std::cout << "\nRecent transactions:" << std::endl;
				const std::size_t start = history.size() > 15 ? history.size() - 15 : 0;
				for (std::size_t i = start; i < history.size(); ++i) {
					std::cout << history[i] << std::endl;
				}
			}
			else {
				std::cout << "\nNo transactions yet." << std::endl;
			}
		}
		else if (customerChoice == 8) {
			std::cout << "\n--- Update Account Information ---" << std::endl;
			std::cout << "1. Update Name\n2. Update Phone Number\n3. Update Gender\n4. Update Address\n5. Update Email\n6. Update Next of Kin\n7. Change Password\n8. Back to Main Menu" << std::endl;
			std::cout << "Select option: ";
			int updateChoice = getValidInt();

			if (updateChoice == 1) {
				std::cout << "Enter new name: ";
				std::string newName;
				std::getline(std::cin, newName);
				if (!newName.empty() && newName.length() <= 100) {
					customer.setName(newName);
					std::cout << "Name updated successfully!" << std::endl;
					saveAccounts(customerUsers);
				}
				else if (newName.empty()) {
					std::cout << "Name cannot be empty." << std::endl;
				}
				else {
					std::cout << "Name is too long (max 100 characters)." << std::endl;
				}
			}
			else if (updateChoice == 2) {
				std::cout << "Enter new phone number (10-20 digits): ";
				std::string newPhone;
				std::getline(std::cin, newPhone);
				if (customer.setPhoneNumber(newPhone)) {
					std::cout << "Phone number updated successfully!" << std::endl;
					saveAccounts(customerUsers);
				}
				else {
					std::cout << "Invalid phone number. Must be 10-20 digits." << std::endl;
				}
			}
			else if (updateChoice == 3) {
				std::cout << "Enter gender (M/F): ";
				char newGender;
				std::cin >> newGender;
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

				if (customer.setGender(newGender)) {
					std::cout << "Gender updated successfully!" << std::endl;
					saveAccounts(customerUsers);
				}
				else {
					std::cout << "Invalid gender. Please enter M or F." << std::endl;
				}
			}
			else if (updateChoice == 4) {
				std::cout << "Enter new address: ";
				std::string newAddress;
				std::getline(std::cin, newAddress);
				if (!newAddress.empty() && newAddress.length() <= 200) {
					customer.setAddress(newAddress);
					std::cout << "Address updated successfully!" << std::endl;
					saveAccounts(customerUsers);
				}
				else if (newAddress.empty()) {
					std::cout << "Address cannot be empty." << std::endl;
				}
				else {
					std::cout << "Address is too long (max 200 characters)." << std::endl;
				}
			}
			else if (updateChoice == 5) {
				std::cout << "Enter new email: ";
				std::string newEmail;
				std::getline(std::cin, newEmail);
				if (customer.setEmail(newEmail)) {
					std::cout << "Email updated successfully!" << std::endl;
					saveAccounts(customerUsers);
				}
				else {
					std::cout << "Invalid email format. Email must contain '@'." << std::endl;
				}
			}
			else if (updateChoice == 6) {
				std::cout << "\n--- Update Next of Kin Information ---" << std::endl;
				std::cout << "Enter next of kin name: ";
				std::string nokName;
				std::getline(std::cin, nokName);
				std::cout << "Enter relationship to Next of Kin: ";
				std::string nokRelationship;
				std::getline(std::cin, nokRelationship);
				std::cout << "Enter next of kin phone number: ";
				std::string nokPhoneNumber;
				std::getline(std::cin, nokPhoneNumber);
				std::cout << "Enter next of kin address: ";
				std::string nokAddress;
				std::getline(std::cin, nokAddress);

				if (!nokName.empty() && !nokRelationship.empty()) {
					NextOfKin nok{ nokName, nokRelationship, nokPhoneNumber, nokAddress };
					customer.setNextOfKin(nok);
					std::cout << "Next of kin information updated successfully!" << std::endl;
					saveAccounts(customerUsers);
				}
				else {
					std::cout << "Next of kin name and relationship cannot be empty." << std::endl;
				}
			}
			else if (updateChoice == 7) {
				std::cout << "Enter current password: ";
				std::string currentPassword;
				currentPassword = readPassword();
				if (customer.validatePassword(currentPassword)) {
					std::cout << "Enter new password (minimum 8 characters): ";
					std::string newPassword;
					newPassword = readPassword();
					if (customer.setPassword(newPassword)) {
						std::cout << "Password updated successfully!" << std::endl;
						saveAccounts(customerUsers);
					}
					else {
						std::cout << "Password must be at least 8 characters long." << std::endl;
					}
				}
				else {
					std::cout << "Current password is incorrect." << std::endl;
				}
			}
			else if (updateChoice == 8) {
				std::cout << "Returning to main menu..." << std::endl;
			}
			else {
				std::cout << "Invalid choice." << std::endl;
			}
		}
		else if (customerChoice == 9) {
			std::cout << "Logging out..." << std::endl;
			break;
		}
		else {
			std::cout << "Invalid choice selected." << std::endl;
		}
		std::cout << "Do you want to perform another transaction? (Y/N): ";
		std::cin >> customerRunAgain;
		customerRunAgain = std::toupper(customerRunAgain);
		std::cout << std::endl << std::endl;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	while (customerRunAgain == 'Y');
}

void runAdminSession(Admin& admin, const std::string& username) {
	char adminRunAgain = 'N';
	do {
		const std::string adminLevel = admin.getAdminLevel();

		std::cout << "\n=== ADMIN MENU ===" << std::endl;
		std::cout << "1. Add / Edit Flights\n2. Set Flight Prices\n3. Manage Seats (Availability)\n4. View All Bookings\n5. Confirm Payments\n6. Update My Account Information" << std::endl;
		if (adminLevel == "MAX") {
			std::cout << "7. Create Admin Account\n8. Delete Account\n9. Edit Admin Level\n10. View Accounts Info\n11. View Customer Transactions\n12. Logout" << std::endl;
		}
		else {
			std::cout << "7. Logout" << std::endl;
		}

		std::cout << "Please select an option: ";
		int adminChoice = getValidInt();

		// 1) Add / Edit Flights (with 3 tries for invalid input)
		if (adminChoice == 1) {
			std::cout << "\n--- Add / Edit Flights ---" << std::endl;
			std::cout << "Enter flight ID (e.g., NX101): ";
			std::string flightId;
			std::getline(std::cin, flightId);

			if (flightId.empty()) {
				std::cout << "Flight ID cannot be empty." << std::endl;
			}
			else {
				std::transform(flightId.begin(), flightId.end(), flightId.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
				const bool isExisting = flights.contains(flightId);
				Flight* flightPtr = isExisting ? &flights.at(flightId) : nullptr;

				if (isExisting) {
					std::cout << "Flight '" << flightId << "' already exists. Edit mode." << std::endl;
				}
				else {
					std::cout << "Creating new flight '" << flightId << "'" << std::endl;
				}

				Flight newFlight;
				if (!isExisting) {
					(void)newFlight.setId(flightId);
				}
				Flight& f = isExisting ? *flightPtr : newFlight;

				bool cancelled = false;

				// Airline
				for (int attempt = 0; attempt < 3; ++attempt) {
					if (isExisting) std::cout << "Current airline: " << f.getAirline() << std::endl;
					std::cout << "Enter airline name" << (isExisting ? " (press Enter to keep current)" : "") << ": ";
					std::string airline;
					std::getline(std::cin, airline);
					if (isExisting && airline.empty()) break;
					if (f.setAirline(airline)) break;
					std::cout << "Invalid airline name." << std::endl;
					if (attempt == 2) cancelled = true;
				}

				// Origin
				if (!cancelled) {
					for (int attempt = 0; attempt < 3; ++attempt) {
						if (isExisting) std::cout << "Current origin: " << f.getOrigin() << std::endl;
						std::cout << "Enter origin (airport code, e.g., LOS)" << (isExisting ? " (press Enter to keep current)" : "") << ": ";
						std::string origin;
						std::getline(std::cin, origin);
						if (isExisting && origin.empty()) break;
						if (f.setOrigin(origin)) break;
						std::cout << "Invalid origin. Use a supported airport code and ensure origin != destination." << std::endl;
						if (attempt == 2) cancelled = true;
					}
				}

				// Destination
				if (!cancelled) {
					for (int attempt = 0; attempt < 3; ++attempt) {
						if (isExisting) std::cout << "Current destination: " << f.getDestination() << std::endl;
						std::cout << "Enter destination (airport code, e.g., ABV)" << (isExisting ? " (press Enter to keep current)" : "") << ": ";
						std::string destination;
						std::getline(std::cin, destination);
						if (isExisting && destination.empty()) break;
						if (f.setDestination(destination)) break;
						std::cout << "Invalid destination. Use a supported airport code and ensure destination != origin." << std::endl;
						if (attempt == 2) cancelled = true;
					}
				}

				// Departure time
				if (!cancelled) {
					for (int attempt = 0; attempt < 3; ++attempt) {
						if (isExisting) std::cout << "Current departure time: " << f.getDepartureTime() << std::endl;
						std::cout << "Enter departure time (YYYY-MM-DD HH:MM)" << (isExisting ? " (press Enter to keep current)" : "") << ": ";
						std::string dep;
						std::getline(std::cin, dep);
						if (isExisting && dep.empty()) break;
						if (f.setDepartureTimeFromString(dep)) break;
						if (attempt == 2) cancelled = true;
					}
				}

				// Arrival time
				if (!cancelled) {
					for (int attempt = 0; attempt < 3; ++attempt) {
						if (isExisting) std::cout << "Current arrival time: " << f.getArrivalTime() << std::endl;
						std::cout << "Enter arrival time (YYYY-MM-DD HH:MM)" << (isExisting ? " (press Enter to keep current)" : "") << ": ";
						std::string arr;
						std::getline(std::cin, arr);
						if (isExisting && arr.empty()) break;
						if (f.setArrivalTimeFromString(arr)) break;
						if (attempt == 2) cancelled = true;
					}
				}

				// Seats
				if (!cancelled) {
					for (int attempt = 0; attempt < 3; ++attempt) {
						if (isExisting) std::cout << "Current total seats: " << f.getTotalSeats() << std::endl;
						std::cout << "Enter total seats (1-500)" << (isExisting ? " (press Enter to keep current)" : "") << ": ";
						std::string seatsLine;
						std::getline(std::cin, seatsLine);
						if (isExisting && seatsLine.empty()) break;
						std::istringstream iss(seatsLine);
						int seats = 0;
						if (!(iss >> seats)) {
							std::cout << "Invalid number." << std::endl;
							if (attempt == 2) cancelled = true;
							continue;
						}
						if (f.setTotalSeats(seats)) break;
						std::cout << "Invalid seat count. Must be 1-500 (and seats cannot be resized if any are booked)." << std::endl;
						if (attempt == 2) cancelled = true;
					}
				}

				// Ticket price
				if (!cancelled) {
					for (int attempt = 0; attempt < 3; ++attempt) {
						if (isExisting) std::cout << "Current base ticket price: NGN" << std::fixed << std::setprecision(2) << f.getTicketPrice() << std::endl;
						std::cout << "Enter base ticket price (NGN)" << (isExisting ? " (press Enter to keep current)" : "") << ": ";
						std::string priceLine;
						std::getline(std::cin, priceLine);
						if (isExisting && priceLine.empty()) break;
						std::istringstream iss(priceLine);
						double price = 0.0;
						if (!(iss >> price)) {
							std::cout << "Invalid number." << std::endl;
							if (attempt == 2) cancelled = true;
							continue;
						}
						if (f.setTicketPrice(price)) break;
						std::cout << "Invalid ticket price." << std::endl;
						if (attempt == 2) cancelled = true;
					}
				}

				if (cancelled) {
					std::cout << "Flight operation cancelled (too many invalid inputs)." << std::endl;
				}
				else {
					if (!isExisting) {
						flights[flightId] = newFlight;
						std::cout << "Flight '" << flightId << "' created successfully!" << std::endl;
					}
					else {
						std::cout << "Flight '" << flightId << "' updated successfully!" << std::endl;
					}
					saveFlights();
				}
			}
		}

		// 2) Set Flight Prices
		else if (adminChoice == 2) {
			std::cout << "\n--- Set Flight Prices ---" << std::endl;

			if (flights.empty()) {
				std::cout << "No flights available to update." << std::endl;
			}
			else {
				std::cout << "Available flights:" << std::endl;
				for (const auto& [id, f] : flights) {
					std::cout << "ID: " << id << " | " << f.getAirline() << " | " << f.getOrigin() << " -> " << f.getDestination() << " | Current Price: NGN" << std::fixed << std::setprecision(2) << f.getTicketPrice() << std::endl;
				}

				std::cout << "\nEnter flight ID to update price: ";
				std::string flightId;
				std::getline(std::cin, flightId);

				if (flightId.empty()) {
					std::cout << "Flight ID cannot be empty." << std::endl;
				}
				else {
					std::transform(flightId.begin(), flightId.end(), flightId.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

					if (!flights.contains(flightId)) {
						std::cout << "Flight '" << flightId << "' not found." << std::endl;
					}
					else {
						Flight& f = flights.at(flightId);
						std::cout << "Current price for flight '" << flightId << "': NGN" << std::fixed << std::setprecision(2) << f.getTicketPrice() << std::endl;
						std::cout << "Enter new price (NGN): ";
						double newPrice = getValidDouble();

						if (newPrice <= 0) {
							std::cout << "Price must be greater than 0." << std::endl;
						}
						else {
							f.setTicketPrice(newPrice);
							std::cout << "Price updated successfully! New price: NGN" << std::fixed << std::setprecision(2) << newPrice << std::endl;
							saveFlights();
						}
					}
				}
			}
		}

		// Option 3: Manage Seats (Availability)
		else if (adminChoice == 3) {
			std::cout << "\n--- Manage Seats (Availability) ---" << std::endl;

			if (flights.empty()) {
				std::cout << "No flights available." << std::endl;
			}
			else {
				std::cout << "Available flights:" << std::endl;
				for (const auto& [id, f] : flights) {
					std::cout << "ID: " << id << " | " << f.getAirline() << " | " << f.getOrigin() << " -> " << f.getDestination() << " | Available: " << f.getAvailableSeats() << "/" << f.getTotalSeats() << std::endl;
				}

				std::cout << "\nEnter flight ID: ";
				std::string flightId;
				std::getline(std::cin, flightId);

				if (flightId.empty()) {
					std::cout << "Flight ID cannot be empty." << std::endl;
				}
				else {
					std::transform(flightId.begin(), flightId.end(), flightId.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

					if (!flights.contains(flightId)) {
						std::cout << "Flight '" << flightId << "' not found." << std::endl;
					}
					else {
						Flight& f = flights.at(flightId);

						std::cout << "\nFlight '" << flightId << "' seat information:" << std::endl;
						std::cout << "Total seats: " << f.getTotalSeats() << std::endl;
						std::cout << "Available seats: " << f.getAvailableSeats() << std::endl;
						std::cout << "Booked seats: " << f.getBookedSeats() << std::endl;

						std::cout << "\nSeat Details by Cabin:" << std::endl;
						int economyCount = 0, businessCount = 0, firstClassCount = 0;
						int economyBooked = 0, businessBooked = 0, firstClassBooked = 0;

						for (const auto& seat : f.getSeats()) {
							if (!seat) continue;

							if (seat->getCabin() == Seat::Cabin::Economy) {
								economyCount++;
								if (seat->getIsBooked()) economyBooked++;
							}
							else if (seat->getCabin() == Seat::Cabin::Business) {
								businessCount++;
								if (seat->getIsBooked()) businessBooked++;
							}
							else if (seat->getCabin() == Seat::Cabin::FirstClass) {
								firstClassCount++;
								if (seat->getIsBooked()) firstClassBooked++;
							}
						}

						std::cout << "  Economy: " << economyBooked << " booked / " << economyCount << " total" << std::endl;
						std::cout << "  Business: " << businessBooked << " booked / " << businessCount << " total" << std::endl;
						std::cout << "  First Class: " << firstClassBooked << " booked / " << firstClassCount << " total" << std::endl;
					}
				}
			}
		}

		// Option 4: View All Bookings
		else if (adminChoice == 4) {
			std::cout << "\n--- View All Bookings ---" << std::endl;

			if (flights.empty()) {
				std::cout << "No flights in system." << std::endl;
			}
			else {
				int totalBookings = 0;

				for (const auto& [flightId, flight] : flights) {
					int bookedCount = 0;
					std::vector<std::string> bookedSeats;

					for (const auto& seat : flight.getSeats()) {
						if (seat && seat->getIsBooked()) {
							bookedCount++;
							std::string seatInfo = seat->getSeatNumber() + " (" + Seat::cabinToString(seat->getCabin()) + ")";
							bookedSeats.push_back(seatInfo);
						}
					}

					if (bookedCount > 0) {
						std::cout << "\nFlight " << flightId << ": " << flight.getAirline() << " | " << flight.getOrigin() << " -> " << flight.getDestination() << std::endl;
						std::cout << "  Booked seats: " << bookedCount << std::endl;
						for (const auto& bs : bookedSeats) {
							std::cout << "    - " << bs << std::endl;
						}
						totalBookings += bookedCount;
					}
				}

				if (totalBookings == 0) {
					std::cout << "No bookings in the system." << std::endl;
				}
				else {
					std::cout << "\nTotal bookings across all flights: " << totalBookings << std::endl;
				}
			}
		}

		// Option 5: Confirm Payments
		else if (adminChoice == 5) {
			std::cout << "\n--- Confirm Payments (Paystack) ---" << std::endl;
			std::cout << "Enter Paystack reference to verify: ";
			std::string ref;
			std::getline(std::cin, ref);

			if (ref.empty()) {
				std::cout << "Reference cannot be empty." << std::endl;
			}
			else {
				auto verify = paystackVerify(ref);

				if (!verify.ok) {
					std::cout << "Verification failed: " << verify.message << std::endl;
				}
				else {
					std::cout << "Reference: " << ref << std::endl;
					std::cout << "Amount: NGN" << std::fixed << std::setprecision(2) << koboToNaira(static_cast<long long>(verify.amount)) << std::endl;
					std::cout << "Status: " << verify.status << std::endl;
					std::cout << "Success: " << (verify.isSuccess ? "Yes" : "No") << std::endl;

					// Extract customer email from the transaction data
					std::string customerEmail = verify.data.contains("customer") && verify.data["customer"].is_object() && verify.data["customer"].contains("email") ? verify.data["customer"]["email"].get<std::string>() : "N/A";
					std::cout << "Customer Email: " << customerEmail << std::endl;

					if (verify.isSuccess) {
						std::cout << "Payment confirmed and successful!" << std::endl;
					}
					else {
						std::cout << "Payment not completed. Status: " << verify.status << std::endl;
					}
				}
			}
		}

		// 6) Update My Account (Admin)
		else if (adminChoice == 6) {
			std::cout << "\n--- Update My Admin Account ---" << std::endl;
			std::cout << "1. Update Name\n2. Update Phone Number\n3. Update Gender\n4. Update Address\n5. Update Email\n6. Update Department\n7. Change Password\n8. Back" << std::endl;
			std::cout << "Select option: ";
			int updateChoice = getValidInt();

			if (updateChoice == 1) {
				for (int attempt = 0; attempt < 3; ++attempt) {
					std::cout << "Enter new name: ";
					std::string v;
					std::getline(std::cin, v);
					if (!v.empty()) {
						admin.setName(v);
						saveAccounts(adminUsers);
						std::cout << "Name updated." << std::endl;
						break;
					}
					std::cout << "Name cannot be empty." << std::endl;
				}
			}
			else if (updateChoice == 2) {
				for (int attempt = 0; attempt < 3; ++attempt) {
					std::cout << "Enter new phone number (10-20 digits): ";
					std::string v;
					std::getline(std::cin, v);
					if (admin.setPhoneNumber(v)) {
						saveAccounts(adminUsers);
						std::cout << "Phone number updated." << std::endl;
						break;
					}
					std::cout << "Invalid phone number." << std::endl;
				}
			}
			else if (updateChoice == 3) {
				for (int attempt = 0; attempt < 3; ++attempt) {
					std::cout << "Enter gender (M/F): ";
					std::string v;
					std::getline(std::cin, v);
					if (!v.empty() && admin.setGender(v[0])) {
						saveAccounts(adminUsers);
						std::cout << "Gender updated." << std::endl;
						break;
					}
					std::cout << "Invalid gender." << std::endl;
				}
			}
			else if (updateChoice == 4) {
				for (int attempt = 0; attempt < 3; ++attempt) {
					std::cout << "Enter new address: ";
					std::string v;
					std::getline(std::cin, v);
					if (!v.empty()) {
						admin.setAddress(v);
						saveAccounts(adminUsers);
						std::cout << "Address updated." << std::endl;
						break;
					}
					std::cout << "Address cannot be empty." << std::endl;
				}
			}
			else if (updateChoice == 5) {
				for (int attempt = 0; attempt < 3; ++attempt) {
					std::cout << "Enter new email: ";
					std::string v;
					std::getline(std::cin, v);
					if (admin.setEmail(v)) {
						saveAccounts(adminUsers);
						std::cout << "Email updated." << std::endl;
						break;
					}
					std::cout << "Invalid email." << std::endl;
				}
			}
			else if (updateChoice == 6) {
				for (int attempt = 0; attempt < 3; ++attempt) {
					std::cout << "Enter new department: ";
					std::string v;
					std::getline(std::cin, v);
					if (!v.empty()) {
						admin.setDepartment(v);
						saveAccounts(adminUsers);
						std::cout << "Department updated." << std::endl;
						break;
					}
					std::cout << "Department cannot be empty." << std::endl;
				}
			}
			else if (updateChoice == 7) {
				for (int attempt = 0; attempt < 3; ++attempt) {
					std::cout << "Enter new password (min 8 chars): ";
					std::string v;
					v = readPassword();
					if (admin.setPassword(v)) {
						saveAccounts(adminUsers);
						std::cout << "Password updated." << std::endl;
						break;
					}
				}
			}
		}

		// MAX-only options
		else if (adminLevel == "MAX" && adminChoice == 7) {
			std::cout << "\n--- Create Admin Account ---" << std::endl;
			std::cout << "Enter a unique username for the new admin account: ";
			std::string newUsername;
			std::getline(std::cin, newUsername);
			std::cout << "Enter a password for the new admin account: ";
			std::string password = readPassword();
			if (createAdminUser(newUsername, password)) {
				std::cout << "Admin account '" << newUsername << "' created successfully!" << std::endl;
				saveAccounts(adminUsers);
			}
			else {
				std::cout << "Failed to create admin account." << std::endl;
			}
		}
		else if (adminLevel == "MAX" && adminChoice == 8) {
			std::cout << "\n--- Delete Account ---" << std::endl;
			std::cout << "1. Delete Customer Account\n2. Delete Admin Account\n3. Back" << std::endl;
			std::cout << "Select option: ";
			int delChoice = getValidInt();
			if (delChoice == 1) {
				std::cout << "Enter customer username to delete: ";
				std::string customerName;
				std::getline(std::cin, customerName);
				if (customerName.empty()) {
					std::cout << "Username cannot be empty." << std::endl;
				}
				else if (deleteUser(customerName, customerUsers)) {
					std::cout << "Customer account '" << customerName << "' deleted." << std::endl;
					saveAccounts(customerUsers);
				}
				else {
					std::cout << "Customer '" << customerName << "' not found." << std::endl;
				}
			}
			else if (delChoice == 2) {
				std::cout << "Enter admin username to delete: ";
				std::string adminToDelete;
				std::getline(std::cin, adminToDelete);
				if (adminToDelete.empty()) {
					std::cout << "Username cannot be empty." << std::endl;
				}
				else if (adminToDelete == username) {
					std::cout << "You cannot delete your own admin account." << std::endl;
				}
				else if (adminToDelete == "Shadow as admin.") {
					std::cout << "This is a reserved account and cannot be deleted." << std::endl;
				}
				else if (deleteUser(adminToDelete, adminUsers)) {
					std::cout << "Admin account '" << adminToDelete << "' deleted." << std::endl;
					saveAccounts(adminUsers);
				}
				else {
					std::cout << "Admin '" << adminToDelete << "' not found." << std::endl;
				}
			}
			else if (delChoice == 3) {
				std::cout << "Returning to admin menu..." << std::endl;
			}
			else {
				std::cout << "Invalid choice." << std::endl;
			}
		}
		else if (adminLevel == "MAX" && adminChoice == 9) {
			std::cout << "\n--- Edit Admin Level ---" << std::endl;
			std::cout << "Enter admin username: ";
			std::string target;
			std::getline(std::cin, target);
			if (!adminUsers.contains(target)) {
				std::cout << "Admin '" << target << "' not found." << std::endl;
			}
			else {
				std::cout << "Enter new admin level (e.g., Level 1, MAX): ";
				std::string newLevel;
				std::getline(std::cin, newLevel);
				if (newLevel.empty()) {
					std::cout << "Admin level cannot be empty." << std::endl;
				}
				else {
					adminUsers.at(target).setAdminLevel(newLevel);
					saveAccounts(adminUsers);
					std::cout << "Updated '" << target << "' level to '" << newLevel << "'." << std::endl;
				}
			}
		}
		else if (adminLevel == "MAX" && adminChoice == 10) {
			std::cout << "\n--- View Accounts Info ---" << std::endl;
			std::cout << "1. View Admin Accounts\n2. View Customer Accounts\n3. Back" << std::endl;
			std::cout << "Select option: ";
			int viewChoice = getValidInt();
			std::vector<std::string> lines;

			if (viewChoice == 1) {
				for (const auto& [uname, adm] : adminUsers) {
					std::ostringstream oss;
					oss << uname
						<< " | Name: " << adm.getName()
						<< " | Level: " << adm.getAdminLevel()
						<< " | Dept: " << adm.getDepartment();
					lines.push_back(oss.str());
				}
			}
			else if (viewChoice == 2) {
				for (const auto& [uname, cust] : customerUsers) {
					std::ostringstream oss;
					oss << uname
						<< " | Name: " << cust.getName()
						<< " | Balance: NGN" << std::fixed << std::setprecision(2) << cust.getAccountBalance()
						<< " | Transactions: " << cust.getTransactionHistory().size();
					lines.push_back(oss.str());
				}
			}

			// Print 20 at a time
			std::size_t index = 0;
			while (index < lines.size()) {
				std::size_t end = std::min(index + static_cast<std::size_t>(20), lines.size());
				for (std::size_t i = index; i < end; ++i) {
					std::cout << lines[i] << std::endl;
				}
				index = end;
				if (index >= lines.size()) break;
				std::cout << "Show next 20? (Y/N): ";
				char more = 'N';
				std::cin >> more;
				more = static_cast<char>(std::toupper(static_cast<unsigned char>(more)));
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				if (more != 'Y') break;
			}
		}
		else if (adminLevel == "MAX" && adminChoice == 11) {
			std::cout << "\n--- View Customer Transactions ---" << std::endl;
			std::cout << "1. View One Customer\n2. View All Customers\n3. Back" << std::endl;
			std::cout << "Select option: ";
			int txChoice = getValidInt();

			std::vector<std::string> lines;
			if (txChoice == 1) {
				std::cout << "Enter customer username: ";
				std::string custName;
				std::getline(std::cin, custName);
				if (!customerUsers.contains(custName)) {
					std::cout << "Customer '" << custName << "' not found." << std::endl;
				}
				else {
					for (const auto& tx : customerUsers.at(custName).getTransactionHistory()) {
						lines.push_back(tx);
					}
				}
			}
			else if (txChoice == 2) {
				for (const auto& [uname, cust] : customerUsers) {
					for (const auto& tx : cust.getTransactionHistory()) {
						std::ostringstream oss;
						oss << uname << ": " << tx;
						lines.push_back(oss.str());
					}
				}
			}

			// Print 20 at a time
			std::size_t index = 0;
			while (index < lines.size()) {
				std::size_t end = std::min(index + static_cast<std::size_t>(20), lines.size());
				for (std::size_t i = index; i < end; ++i) {
					std::cout << lines[i] << std::endl;
				}
				index = end;
				if (index >= lines.size()) break;
				std::cout << "Show next 20? (Y/N): ";
				char more = 'N';
				std::cin >> more;
				more = static_cast<char>(std::toupper(static_cast<unsigned char>(more)));
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				if (more != 'Y') break;
			}
		}
		else if ((adminLevel == "MAX" && adminChoice == 12) || (adminLevel != "MAX" && adminChoice == 7)) {
			std::cout << "Logging out..." << std::endl;
			break;
		}
		else {
			std::cout << "Invalid choice selected." << std::endl;
		}

		std::cout << "\nDo you want to perform another transaction? (Y/N): ";
		std::cin >> adminRunAgain;
		adminRunAgain = std::toupper(adminRunAgain);
		std::cout << std::endl << std::endl;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	while (adminRunAgain == 'Y');
}

void runMainMenu() {
	std::cout << "Welcome to Nexora Airways" << std::endl;
	char runAgain = 'Y';
	do {
		std::string username, password;
		std::cout << "1. Create an account\n2. Login\n3. Exit" << std::endl;
		std::cout << "Please select an option: ";
		int choice = getValidInt();
		if (choice == 1) {
			std::cout << std::endl;
			std::cout << "Enter a unique username for your account: ";
			std::getline(std::cin, username);
			std::cout << "Enter a password for your account: ";
			password = readPassword();

			if (createCustomerUser(username, password)) {
				std::cout << "Customer account '" << username << "' created successfully!" << std::endl;
				saveAccounts();
			}
		}
		else if (choice == 2) {
			std::cout << std::endl;
			std::cout << "1. Login as Customer\n2. Login as Admin" << std::endl;
			std::cout << "Please select an option: ";
			int loginType = getValidInt();
			if (loginType == 1) {
				username = validateUser(customerUsers);
				if (!username.empty()) {
					std::cout << "Welcome, " << username << "!" << std::endl;
					std::cout << std::endl;
					Customer& currentCustomer = customerUsers.at(username);
					runCustomerSession(currentCustomer, username);
				}
			}
			else if (loginType == 2) {
				username = validateUser(adminUsers);
				if (!username.empty()) {
					std::cout << "Welcome, admin '" << username << "'!" << std::endl;
					std::cout << std::endl;
					Admin& currentAdmin = adminUsers.at(username);
					runAdminSession(currentAdmin, username);
				}
			}
			else {
				std::cout << "Invalid option. Please try again." << std::endl;
			}
		}
		else if (choice == 3) {
			break;
		}
		else {
			std::cout << "Invalid option. Please try again." << std::endl << std::endl;
			continue;
		}
		std::cout << "Do you want to perform another action? (Y/N): ";
		std::cin >> runAgain;
		runAgain = std::toupper(runAgain);
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cout << std::endl;
	}
	while (runAgain == 'Y');
	std::cout << "Thank you for using Nexora Airways. Have a great day!" << std::endl;
}