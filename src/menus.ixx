export module menus;
import std;
import payment;
export import users;
export import flights;

export int getValidInt();
export double getValidDouble();
export void runCustomerSession(Customer& customer, const std::string& username);
export void runAdminSession(Admin& admin, const std::string& username);
export void runMainMenu();