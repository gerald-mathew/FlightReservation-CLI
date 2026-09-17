<p align="center">
  <picture>
    <img src="assets/banner.svg" alt="Nexora Airways CLI" width="820">
  </picture>
</p>

<p align="center">
  <strong>A command-line flight reservation system for a fictional Nigerian airline, written in C++20 named modules.</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20%20modules-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++20 modules">
  <img src="https://img.shields.io/badge/interface-CLI-818cf8?style=for-the-badge&logo=gnubash&logoColor=white" alt="CLI">
  <img src="https://img.shields.io/badge/SQLite-3-003B57?style=for-the-badge&logo=sqlite&logoColor=white" alt="SQLite 3">
</p>
<p align="center">
  <img src="https://img.shields.io/badge/Argon2-password%20hashing-818cf8?style=for-the-badge" alt="Argon2">
  <img src="https://img.shields.io/badge/Paystack-payments-011B33?style=for-the-badge" alt="Paystack">
  <img src="https://img.shields.io/badge/License-MIT-818cf8?style=for-the-badge" alt="MIT license">
</p>

Nexora Airways pairs an interactive console interface with SQLite persistence, Argon2id password hashing, a built-in account wallet and live Paystack payment processing.

## Features

| Feature | Description |
| --- | --- |
| Account roles | Customers and Admins, plus a protected super-admin account |
| Flight management (admin) | Create and edit flights between 21 Nigerian airports, set ticket prices, manage seat availability and review every booking |
| Booking (customer) | Search and book flights, view booked flights and cancel within a 30-minute cancellation window |
| Seat model | Economy, Business and First-Class cabins with window/aisle awareness and per-seat booking ownership |
| Flight status | Scheduled / Boarding / Departed / Delayed / Cancelled, updated automatically from the system clock |
| Account wallet | Deposit, withdraw and transfer funds between Nexora accounts, with loyalty points and a per-account transaction history |
| Paystack payments | Initialize a transaction, open the checkout page in the default browser, then verify the result; payment references are tracked so they cannot be reused |
| Secure authentication | Argon2id hashing with per-password salts and constant-time verification |
| Persistence | All flights, seats, accounts, transactions and bookings live in a single SQLite database |

## Requirements

- **OS:** Windows (primary). The fallback build path also targets MSYS2 on Windows.
- **Compiler** (any one): MSVC with C++20 module support (v143/v145 toolset, Visual Studio 2022 or newer), Clang with libc++, or GCC 15+. All need `import std;`.
- **Dependencies:** `argon2`, `curl`, `sqlite3` and `zlib`. `nlohmann/json` is header-only and vendored under `third_party/` for the Clang/GCC fallbacks.

The MSVC build resolves dependencies through vcpkg (`x64-windows-static` triplet); the Clang/GCC fallbacks link against MSYS2 UCRT64 libraries.

## Build

The build script tries three toolchains in order (**MSVC → Clang → GCC**) and stops at the first that succeeds. The executable is emitted as `build/NexoraAirways.exe`.

```powershell
cd build
./build.bat
```

In Visual Studio: open `FlightReservation-CLI.slnx`, select the **x64** configuration and build (Ctrl+Shift+B).

### Debug symbols (`.pdb`)

PDB generation is disabled by default to keep the output directory clean. To produce a `.pdb` for debugging, edit `FlightReservation-CLI.vcxproj` and, for your configuration, set:

```xml
<ClCompile>
  <DebugInformationFormat>ProgramDatabase</DebugInformationFormat>
</ClCompile>
<Link>
  <GenerateDebugInformation>true</GenerateDebugInformation>
</Link>
```

Both values currently read `None` / `false`. Revert them to disable PDBs again.

## Configuration

Configuration is not committed. Copy the examples and fill in your own values:

```powershell
Copy-Item config/paystack_secret.example.txt config/paystack_secret.txt
Copy-Item config/super_admin.example.txt config/super_admin.txt
```

| File | Purpose |
| --- | --- |
| `config/paystack_secret.txt` | Paystack secret key (for example `sk_test_...`). Required for the payment flow |
| `config/super_admin.txt` | Optional override for the super-admin password |

If `super_admin.txt` is absent, the default super-admin credentials are:

| Field | Value |
| --- | --- |
| Username | `Shadow as admin.` |
| Password | `Kamar-Taj` |

To set a custom password:

```powershell
"YourSecurePassword123" | Out-File -Encoding utf8 config/super_admin.txt
```

## Running

```powershell
./build/NexoraAirways.exe
```

On first launch the app creates the `data/` directory, opens `data/flight_reservation.db`, and provisions the schema and super-admin account automatically. From the main menu you can create an account, log in as a customer or admin, and proceed to the role-specific menus.

## Project structure

```text
src/            C++20 module interfaces (*.ixx) and implementations (*.cpp)
third_party/    Vendored nlohmann/json for the Clang/GCC fallback builds
build/          build.bat and (after building) the NexoraAirways.exe output
data/           flight_reservation.db, created and updated at runtime
config/         paystack_secret.txt and super_admin.txt, created from *.example.txt
icon/           Windows resource script and application icon
```

### Modules

| Module | Responsibility |
| --- | --- |
| `crypto` | Argon2id password hashing and verification |
| `seats` | Seat hierarchy (Economy/Business/First) and cabin logic |
| `flights` | Flight data, status transitions and seat allocation |
| `users` | Customer/Admin models, wallet and account persistence |
| `payment` | Paystack initialize/verify and Naira to Kobo conversion |
| `sqlite3db` | Singleton SQLite connection and schema management |
| `menus` | Interactive CLI menus and user flows |

## Security notes

- Passwords are hashed with **Argon2id** (2 iterations, 128 MiB memory, single thread, 128-byte output) using a unique salt per password.
- Password verification compares hashes in constant time.
- Paystack requests use Bearer-token auth over TLS with a 30-second timeout; the secret key is read from `config/` and never stored in the database or source.
- The super-admin account is protected from deletion and excluded from bulk account listings.

## License

Released under the [MIT License](LICENSE).

<p align="center"><sub>Built and maintained by <a href="https://github.com/Gerald-Mathew">Gerald-Mathew</a></sub></p>
