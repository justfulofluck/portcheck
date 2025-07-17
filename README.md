Here is a complete **README.md** suitable for your GitHub repository:

---

# PortCheck

**PortCheck** is a lightweight CLI tool written in C to check if a specific port is open on a given hostname or IP address.
You can easily build it or download the prebuilt `.deb` package to install and run on your Linux system.

---

## 📦 Download & Install via `.deb` package

1. **Download the latest `.deb` package:**

```bash
wget https://github.com/yourusername/portcheck/releases/latest/download/portcheck.deb
```

2. **Install the package:**

```bash
sudo dpkg -i portcheck.deb
```

If any dependencies are missing, fix them with:

```bash
sudo apt --fix-broken install
```

---

## ⚙️ Usage

```bash
portcheck <hostname_or_ip> <port>
```

### Example

```bash
portcheck example.com 80
```

**Output:**

```
Scanning host: example.com port: 80
Port 80 is OPEN
```

If the port is closed, the output will be:

```
Port 80 is CLOSED
```

---

## 🔨 Build from Source

If you'd like to build from source:

1. **Clone the repository:**

```bash
git clone https://github.com/yourusername/portcheck.git
cd portcheck
```

2. **Compile:**

```bash
gcc -o portcheck portcheck.c
```

3. **Run:**

```bash
./portcheck <hostname_or_ip> <port>
```

---

## 🐧 Supported Platforms

* Ubuntu/Debian-based systems (via `.deb` package)
* Any Linux distribution with `gcc` to compile from source

---

## 📜 License

This project is licensed under the MIT License.

---

## 🤝 Contributions

Contributions, issues, and feature requests are welcome! Feel free to submit a PR or open an issue.

---

Let me know if you want me to write the **control file** or **deb packaging instructions** too!
