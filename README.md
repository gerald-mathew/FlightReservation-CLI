# Nexora Airways — Flight Reservation CLI

A command-line flight reservation system for a fictional Nigerian airline, written in
**C++20 named modules**. It pairs an interactive console interface with SQLite
persistence, Argon2id password hashing, a built-in account wallet, and live Paystack
payment processing.

---

## Features

- **Two account roles** — Customers and Admins, plus a protected super-admin account.
- **Flight management** (admin) — create and edit flights between 21 Nigerian airports, set ticket prices, manage seat availability, and review every booking.
- **Booking** (customer) — search and book flights, view booked flights, and cancel within a 30-minute cancellation window.
- **Seat model** — Economy, Business, and First-Class cabins with window/aisle awareness and per-seat booking ownership.
- **Flight status** — Scheduled / Boarding / Departed / Delayed / Cancelled, updated automatically from the system clock.
- **Account wallet** — deposit, withdraw, and transfer funds between Nexora accounts, with loyalty points and a per-account transaction history.
- **Paystack payments** — initialize a transaction, open the checkout page in the default browser, then verify the result; payment references are tracked so they cannot be reused.
- **Secure authentication** — Argon2id hashing with per-password salts and constant-time verification.
- **Persistence** — all flights, seats, accounts, transactions, and bookings are stored in a single SQLite database.

---

## Requirements

- **OS**: Windows (primary). The fallback build path also targets MSYS2 on Windows.
- **Compiler** (any one): MSVC with C++20 module support (v143/v145 toolset,
  Visual Studio 2022 or newer) · Clang with libc++ · GCC 15+ — all need `import std;`.
- **Dependencies**: `argon2`, `curl`, `sqlite3` (and `zlib`). `nlohmann/json` is
  header-only and vendored under `third_party/` for the Clang/GCC fallbacks.

The MSVC build resolves dependencies through vcpkg (`x64-windows-static` triplet);
the Clang/GCC fallbacks link against MSYS2 UCRT64 libraries.

---

## Build

The build script tries three toolchains in order — **MSVC → Clang → GCC** — and
stops at the first that succeeds. The executable is emitted as
`build/NexoraAirways.exe`.

```powershell
cd build
./build.bat
```

In Visual Studio: open `FlightReservation-CLI.slnx`, select the **x64** configuration,
and build (Ctrl+Shift+B).

### Debug symbols (`.pdb`)

PDB generation is **disabled** by default to keep the output directory clean. To
produce a `.pdb` for debugging, edit `FlightReservation-CLI.vcxproj` and, for your
configuration, set:

```xml
<ClCompile>
  <DebugInformationFormat>ProgramDatabase</DebugInformationFormat>
</ClCompile>
<Link>
  <GenerateDebugInformation>true</GenerateDebugInformation>
</Link>
```

Both values currently read `None` / `false`. Revert them to disable PDBs again.

---

## Configuration

Credentials live in `config/` (excluded from git — never commit real keys):

- **`config/paystack_secret.txt`** — your Paystack secret key (e.g. `sk_test_...`).
  Required for the payment flow.
- **`config/super_admin.txt`** — optional override for the super-admin password.

If `super_admin.txt` is absent, the default super-admin credentials are:

| Field | Value |
|-------|-------|
| Username | `Shadow as admin.` |
| Password | `Kamar-Taj` |

To set a custom password:

```powershell
"YourSecurePassword123" | Out-File -Encoding utf8 config/super_admin.txt
```

---

## Running

```powershell
./build/NexoraAirways.exe
```

On first launch the app creates the `data/` directory, opens
`data/flight_reservation.db`, and provisions the schema and super-admin account
automatically. From the main menu you can create an account, log in as a customer or
admin, and proceed to the role-specific menus.

---

## Project Structure

```
src/            C++20 module interfaces (*.ixx) and implementations (*.cpp)
third_party/    Vendored nlohmann/json for the Clang/GCC fallback builds
build/          build.bat and the compiled NexoraAirways.exe
data/           flight_reservation.db (created/updated at runtime)
config/         paystack_secret.txt, super_admin.txt (not committed)
icon/           Windows resource script and application icon
```

### Modules

| Module | Responsibility |
|--------|----------------|
| `crypto` | Argon2id password hashing and verification |
| `seats` | Seat hierarchy (Economy/Business/First) and cabin logic |
| `flights` | Flight data, status transitions, and seat allocation |
| `users` | Customer/Admin models, wallet, and account persistence |
| `payment` | Paystack initialize/verify, Naira↔Kobo conversion |
| `sqlite3db` | Singleton SQLite connection and schema management |
| `menus` | Interactive CLI menus and user flows |

---

## Security Notes

- Passwords are hashed with **Argon2id** (2 iterations, 128 MiB memory, single
  thread, 128-byte output) using a unique salt per password.
- Paystack requests use Bearer-token auth over TLS with a 30-second timeout; the
  secret key is read from `config/` and never stored in the database or source.
- The super-admin account is protected from deletion and excluded from bulk account
  listings.

---

## License

MIT License — see `LICENSE`.
