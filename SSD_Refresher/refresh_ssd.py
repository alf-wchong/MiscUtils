#!/usr/bin/env python3
import os
import sys
import time

def format_time(seconds):
    hrs = int(seconds // 3600)
    mins = int((seconds % 3600) // 60)
    secs = int(seconds % 60)
    if hrs > 0:
        return f"{hrs}h {mins}m {secs}s"
    elif mins > 0:
        return f"{mins}m {secs}s"
    else:
        return f"{secs}s"

def count_files(directory):
    count = 0
    for _, _, files in os.walk(directory):
        count += len(files)
    return count

def read_files_with_progress(directory, total):
    count = 0
    start_time = time.time()

    for root, _, files in os.walk(directory):
        for name in files:
            file_path = os.path.join(root, name)
            try:
                with open(file_path, 'rb') as f:
                    while f.read(1024 * 1024):
                        pass
            except Exception:
                pass  # Ignore unreadable files silently
            count += 1

            elapsed = time.time() - start_time
            percent = (count / total) * 100
            eta_seconds = (elapsed / count) * (total - count) if count else 0
            eta = format_time(eta_seconds)

            print(f"\rProgress: {percent:6.2f}% | Elapsed: {format_time(elapsed)} | ETA: {eta}     ", end='', flush=True)

    print("\nDone.")

def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <directory>")
        sys.exit(1)

    directory = sys.argv[1]

    if not os.path.isdir(directory):
        print(f"Error: Directory '{directory}' does not exist.")
        sys.exit(1)

    print("Counting files. Please wait...")
    total_files = count_files(directory)
    if total_files == 0:
        print("No files found.")
        sys.exit(0)

    read_files_with_progress(directory, total_files)

if __name__ == "__main__":
    main()

