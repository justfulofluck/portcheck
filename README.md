Here is the updated **README.md** with your specific instructions:

---

# PortCheck

**PortCheck** is a Linux CLI tool written in C to check if a specific TCP port is open on a given hostname or IP address.

---

## 📦 Install PortCheck on Debian/Ubuntu

I have provided a **`.deb` package** for easy installation on **Debian** or **Ubuntu-based** distributions.

### 🔽 Step 1: Download the `.deb` package

```bash
wget https://github.com/yourusername/portcheck/releases/latest/download/portcheck.deb
```

### ⚙️ Step 2: Install the package

```bash
sudo dpkg -i portcheck.deb
```

If you see any dependency errors, fix them using:

```bash
sudo apt --fix-broken install
```

---

## 🚀 How to Use

Once installed, you can use `portcheck` directly from the terminal:

```bash
portcheck <hostname_or_ip> <port>
```

### Example

```bash
portcheck google.com 443
```

**Output Example:**

```
Scanning host: google.com port: 443
Port 443 is OPEN
```

If the port is closed:

```
Port 443 is CLOSED
```

---

## 🛠️ Build from Source (Optional)

If you'd rather build it from the source code:

1. **Clone the repository**

```bash
git clone https://github.com/yourusername/portcheck.git
cd portcheck
```

2. **Compile**

```bash
gcc -o portcheck portcheck.c
```

3. **Run**

```bash
./portcheck <hostname_or_ip> <port>
```

---

## 🐧 Supported OS

* Debian / Ubuntu / Other Debian-based systems (via `.deb` package)
* Any Linux distribution (via source compilation)

---

## 📄 License

MIT License

---

## 🤝 Contributing

Contributions and suggestions are welcome! Feel free to fork the repository and submit pull requests.

