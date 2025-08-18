# IP Geolocation Lookup Script

A simple Bash script that displays your public IP address and its geographical location using two different IP geolocation services for comparison.

## What it does

This script performs the following actions:
1. **Fetches your public IP address** using `ifconfig.me`
2. **Queries IP geolocation data** from two services:
   - **ipwhois.app** - Shows country, region, and city
   - **ipapi.co** - Shows country, region, city, IP, and organization/ISP

## Features

- Quick public IP lookup
- Dual geolocation service comparison
- Clean, formatted output using `jq`
- No API keys required

## Requirements

- `curl` - for making HTTP requests
- `jq` - for parsing JSON responses

### Installing dependencies

**Ubuntu/Debian:**
```bash
sudo apt install curl jq
```

**CentOS/RHEL/Fedora:**
```bash
sudo yum install curl jq
# or for newer versions:
sudo dnf install curl jq
```

**macOS:**
```bash
brew install curl jq
```

## Usage

1. Save the script to a file (e.g., `ip-lookup.sh`)

2. Make it executable:
   ```bash
   chmod +x ip-lookup.sh
   ```

3. Run the script:
   ```bash
   ./ip-lookup.sh
   ```

## Example Output

```
Public IP is 45.56.133.196
Country: United States
Region: Texas
City: Dallas

IPAPI says

"US"
"Texas"
"Dallas"
"45.56.133.196"
"WEB2OBJECTS"
```

## How it works

1. **Public IP Detection**: Uses `ifconfig.me` to determine your external/public IP address
2. **First Lookup**: Queries `ipwhois.app` with your IP and extracts country, region, and city
3. **Second Lookup**: Queries `ipapi.co` for additional details including ISP/organization info

## Privacy Note

This script makes requests to external services that will log your IP address. Both services used are reputable IP geolocation providers, but be aware that your IP and timestamp will be recorded in their logs.

## License

This script is provided as-is under the MIT License. Feel free to modify and distribute.