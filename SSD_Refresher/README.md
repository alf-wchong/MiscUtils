# SSD Data Retention Refresher

This Python script is designed to **refresh the data retention** of files stored on an SSD by reading every file in a specified directory. This process can help prevent data loss caused by long-term unpowered storage, as regular reads ensure the SSD's internal error correction and charge refresh mechanisms are triggered.

## Features

- Recursively reads all files in a directory tree to `/dev/null`
- Shows a clean, continuously updating progress line:
  - Percentage complete
  - Elapsed time
  - Estimated time remaining (ETA) in human-friendly format
- Handles unreadable files gracefully

## Why Use This Script?

Consumer SSDs can lose data over long periods if left unpowered. Powering up your SSD and reading all its data annually helps keep your data safe. This script is ideal for "data hoarders" and anyone storing archives or backups on SSDs.

## Requirements

- Python 3.6 or newer

## Usage

1. Copy the script to your computer and make it executable:

    ```bash
    chmod +x refresh_ssd_progress.py
    ```

2. Run the script with the directory you want to refresh:

    ```bash
    ./refresh_ssd_progress.py /path/to/your/ssd/mount
    ```

    For example:

    ```bash
    ./refresh_ssd_progress.py /run/media/youruser/yourssd/
    ```

3. The script will:
    - First, count all files (this may take some time for large directories).
    - Then, read each file in the tree, updating the progress bar on one line.
    - Show overall percentage, elapsed time, and ETA.
    - Print "Done." when complete.

## Example Output

```
Counting files. Please wait...
Progress:  12.50% | Elapsed: 1m 15s | ETA: 3m 28s
Done.
```

## Notes

- The script only reads files; it does not modify, move, or delete anything.
- ETA is an estimate based on progress so far.
- Reading files may take significant time if your directory is large or your drive is slow.
- For very large directories, running the script may require ample memory and time.

## License

This script is free software under the [GNU GPL v3](https://www.gnu.org/licenses/gpl.html). See LICENSE for details.
