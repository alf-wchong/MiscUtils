To use the **BFG Repo-Cleaner** on Fedora to redact sensitive data based on a list, you'll need to use the `--replace-text` flag. Unlike standard `git filter-branch`, BFG is significantly faster and easier to use for this specific task.

Here is the step-by-step process:

---

### 1. Install BFG on Fedora

BFG is a Java application. First, ensure you have the OpenJDK installed, then download the BFG `.jar` file.

```bash
# Install Java if you don't have it
sudo dnf install java-latest-openjdk -y

# Download the BFG jar (check for the latest version, currently 1.14.0)
curl -O https://repo1.maven.org/maven2/com/madgag/bfg/1.14.0/bfg-1.14.0.jar

```

### 2. Prepare your Redact List

Create a plain text file (e.g., `passwords.txt`). Each line in this file will be searched and replaced with the string `***REMOVED***` in your git history.

```text
# passwords.txt
my_secret_password
api_key_12345
db_password_production

```

### 3. Clone a "Fresh" Mirror of your Repo

**Warning:** BFG is a destructive operation for your git history. Always work on a fresh "mirror" clone to protect your local working directory.

```bash
git clone --mirror git@github.com:username/your-repo.git

```

### 4. Run the Redaction

Execute the jar file against your mirror directory using the `--replace-text` flag.

```bash
java -jar bfg-1.14.0.jar --replace-text passwords.txt your-repo.git

```

### 5. Cleanup and Force Push

After BFG runs, it strips the dirty data from the commits, but the "old" data still exists in Git's internal storage until you run the garbage collector.

```bash
cd your-repo.git

# Strip out the old reflog and run the garbage collector
git reflog expire --expire=now --all && git gc --prune=now --aggressive

# Force push the clean history back to your server
git push --force

```

---

### Important Considerations

* **Case Sensitivity:** By default, BFG is case-sensitive.
* **The "Latest" Commit:** BFG **will not** modify the contents of your very last (latest) commit. It only cleans the *history*. Make sure you have manually removed the secrets from your current files and committed that change before running BFG.
* **Collaboration:** Because you are rewriting history, everyone else on your team will need to delete their local copies and re-clone the repository after you force push.
