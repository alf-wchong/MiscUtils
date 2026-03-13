# Interrogating Certinia Timecards via Salesforce CLI

This guide provides a streamlined workflow for authenticating via SAML and interrogating **Certinia (formerly FinancialForce)** Timecard data using the Salesforce CLI within a **Fedora/WSL2** environment.

## 📋 Prerequisites

* **OS:** Fedora (WSL2)
* **Shell:** Bash
* **Environment:** Node.js installed (`sudo dnf install nodejs`)
* **Salesforce Access:** Standard credentials with SAML SSO.

---

## 🛠️ 1. Environment Setup

Install the Salesforce CLI and ensure it is available in your `$PATH`.

```bash
# Install Salesforce CLI globally via npm
sudo npm install -g @salesforce/cli

# Verify installation
sf --version

```

---

## 🔐 2. Authentication (SAML SSO)

Because your organization uses SAML, you must use the **Web Server Flow**. This leverages your Windows browser to complete the SSO handshake.

### Step 1: Authorize the Org

Run the following command in your Fedora terminal:

```bash
sf org login web --instance-url https://<yourOrgSfdcTenant>.my.salesforce.com --alias <yourOrg>

```

1. A browser window will open on your host machine.
2. Log in through your standard corporate SSO portal.
3. Once the "Authentication Successful" message appears, you can close the browser.

### Step 2: Set as Default

To avoid typing the alias every time, set this org as your default:

```bash
sf config set target-org <yourOrg> --global

```

---

## 🔍 3. Interrogating Timecards

We use the `data query` command to pull Certinia data. Certinia uses the `pse__` namespace for its objects.

### Basic Interrogation (Last 10 Submitted Cards)

```bash
sf data query --query \
"SELECT Name, pse__Resource__r.Name, pse__Project__r.Name, pse__Total_Hours__c, pse__Status__c \
 FROM pse__Timecard_Header__c \
 WHERE pse__Status__c = 'Submitted' \
 ORDER BY CreatedDate DESC \
 LIMIT 10"

```

---

## 💾 4. Exporting Data

### Export to Markdown Table

The CLI can output directly in markdown format, which is perfect for pasting into GitHub issues:

```bash
sf data query --query "SELECT Name, pse__Total_Hours__c FROM pse__Timecard_Header__c LIMIT 5" --result-format table > timecards.md

```

---

## ⚡ 5. Automation: "Query The Most Recent TimeCards" Bash Function

Instead of a basic alias, add this function to your `~/.bashrc`. It allows you to query the most recent cards by default or filter by a specific week's start date.

1. Open your configuration: `nano ~/.bashrc`
2. Add the following:

```bash
# Certinia Timecard Interrogator
get-timecards() {
    local start_date=$1
    if [ -z "$start_date" ]; then
        # Default: Show last 10 submitted cards
        sf data query --query "SELECT Name, pse__Resource__r.Name, pse__Project__r.Name, pse__Total_Hours__c, pse__Status__c FROM pse__Timecard_Header__c WHERE pse__Status__c = 'Submitted' ORDER BY CreatedDate DESC LIMIT 10"
    else
        # Filtered: Show cards for a specific week (YYYY-MM-DD)
        sf data query --query "SELECT Name, pse__Resource__r.Name, pse__Total_Hours__c, pse__Status__c FROM pse__Timecard_Header__c WHERE pse__Start_Date__c = $start_date"
    fi
}

```

3. Reload: `source ~/.bashrc`

**Usage:**

* `get-timecards` (Returns the latest 10)
* `get-timecards 2026-03-08` (Returns cards for that specific week)

---

## 💡 Key Schema Reference

| Object API Name | Description |
| --- | --- |
| `pse__Timecard_Header__c` | The main timecard record. |
| `pse__Timecard_Line_Item__c` | Detailed daily entries (related to the header). |
| `pse__Assignment__c` | The link between a Resource and a Project. |

---
